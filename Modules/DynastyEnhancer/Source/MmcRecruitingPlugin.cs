using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Security.Cryptography;
using System.Threading;
using System.Web.Script.Serialization;
using Frosty.Core;
using Frosty.Core.Attributes;
using FrostySdk.Interfaces;
[assembly: AssemblyTitle("Beyond the Field")]
[assembly: AssemblyProduct("Beyond the Field")]
[assembly: AssemblyDescription("Native Dynasty Social Media and recruiting Fog of War")]
[assembly: AssemblyVersion("0.1.105.0")]
[assembly: AssemblyFileVersion("0.1.105.0")]
[assembly: PluginVersion("0.1.105-prototype")]
[assembly: PluginAuthor("ArtVsTheWorld")]
[assembly: PluginDisplayName("Ace\u2019s Beyond the Field")]
[assembly: RegisterExecutionAction(typeof(DynastyEnhancer.LaunchAction))]
namespace DynastyEnhancer {
 public sealed class LaunchAction : ExecutionAction {
  public const string NativeFile="BeyondTheFieldNative-v103.dll";
  public const string RecruitingNativeFile="BeyondTheFieldRecruitFog-v1.dll";
  public static string[] NativeFiles {get{return new[]{NativeFile,RecruitingNativeFile};}}
  private static int watching;
  private static Mutex watcherMutex;
  public static string PluginsFolder {get{return Path.GetDirectoryName(typeof(LaunchAction).Assembly.Location);}}
  public static string RuntimeFolder {get{return Path.Combine(Ace.DynastyCOT.UserStorage.Folder("BeyondTheField"),"Runtime-"+typeof(LaunchAction).Assembly.GetName().Version.ToString(3));}}
  public LaunchAction(){using(var host=Process.GetCurrentProcess())if(String.Equals(host.ProcessName,"MMCModManager",StringComparison.OrdinalIgnoreCase))Start();}
  public override Action<ILogger,PluginManagerType,CancellationToken> PreLaunchAction {get{return (log,type,token)=>Start();}}
  public override Action<ILogger,PluginManagerType,CancellationToken> PostLaunchAction {get{return (log,type,token)=>Start();}}
  public static string PayloadPath(string relative){
   if(String.IsNullOrWhiteSpace(relative)||Path.IsPathRooted(relative)||relative.Contains(":"))throw new IOException("Invalid payload path");
   var root=Path.GetFullPath(RuntimeFolder)+Path.DirectorySeparatorChar;
   var path=Path.GetFullPath(Path.Combine(root,relative.Replace('/',Path.DirectorySeparatorChar)));
   if(!path.StartsWith(root,StringComparison.OrdinalIgnoreCase))throw new IOException("Payload path escapes runtime folder");
   return path;
  }
  public static void Prepare(){
   using(var stream=typeof(LaunchAction).Assembly.GetManifestResourceStream("DynastyEnhancer.payload-sha256.txt")){
    if(stream==null)throw new IOException("Missing payload manifest");
    using(var reader=new StreamReader(stream))for(string line;(line=reader.ReadLine())!=null;){
     if(line.Length==0)continue;var item=line.Split('|');if(item.Length!=2||item[0].Length!=64)throw new IOException("Invalid payload manifest");
     var path=PayloadPath(item[1]);if(!File.Exists(path))throw new IOException("Missing runtime file "+path+". Restore the matching Beyond the Field release DLL and prepare its runtime before launching the game.");
     using(var hash=SHA256.Create())using(var input=File.OpenRead(path))if(BitConverter.ToString(hash.ComputeHash(input)).Replace("-","").ToLowerInvariant()!=item[0])throw new IOException("Integrity check failed for "+path+". Restore the matching Beyond the Field release DLL and prepare its runtime before launching the game.");
    }
   }
  }
  public static string ConfigurationIssue(){
   try{
    var config=new JavaScriptSerializer().Deserialize<Dictionary<string,object>>(File.ReadAllText(Path.Combine(Ace.DynastyCOT.UserStorage.Folder("BeyondTheField"),"dynasty-enhancer.json")));
    foreach(var key in new[]{"node","exporter"}){object value;if(!config.TryGetValue(key,out value)||!(value is string))return "Missing configuration field: "+key;
     var path=(string)value;if(key=="node"&&path=="../Shared/node.exe")path=Path.Combine(RuntimeFolder,"Shared","node.exe");else if(key=="exporter"&&path=="Tools/export_runtime.mjs")path=Path.Combine(RuntimeFolder,"Tools","export_runtime.mjs");else if(!Path.IsPathRooted(path))path=Path.Combine(Ace.DynastyCOT.UserStorage.Folder("BeyondTheField"),path);if(!File.Exists(path))return "Social Media is waiting for its configured "+key+" at "+path+". Check the path in the Beyond the Field configuration or repair the matching release.";}
    return "";
   }catch(Exception error){return "Runtime configuration unavailable: "+error.Message;}
  }
  private static void Note(string message){
   try{var folder=Path.Combine(Ace.DynastyCOT.UserStorage.Folder("BeyondTheField"),"Logs");Directory.CreateDirectory(folder);
   var path=Path.Combine(folder,"BeyondTheField-launcher.log");
   if(File.Exists(path)&&new FileInfo(path).Length>1024*1024){if(File.Exists(path+".1"))File.Delete(path+".1");File.Move(path,path+".1");}
   File.AppendAllText(path,DateTimeOffset.Now.ToString("O")+" | "+message+Environment.NewLine);
   }catch(Exception error){Debug.WriteLine("Beyond the Field diagnostics unavailable: "+error.Message);}
  }
  private static void Start(){
   if(Interlocked.CompareExchange(ref watching,1,0)!=0)return;
   bool first;watcherMutex=new Mutex(false,"Local\\DynastyEnhancer.MMC."+Process.GetCurrentProcess().Id,out first);if(!first)return;
   new Thread(()=>{
    try{
     Prepare();Note("MMC discovered Beyond the Field. Runtime files passed integrity checks. Waiting for the game window before loading Social Media and recruiting.");
     var loaded=new HashSet<string>();var retryAt=new Dictionary<string,DateTime>();var attempts=new Dictionary<string,int>();string previousIssue=null;
     while(true){
      var issue=ConfigurationIssue();if(issue.Length!=0){if(issue!=previousIssue)Note(issue);previousIssue=issue;Thread.Sleep(1000);continue;}previousIssue=null;
      foreach(var game in Process.GetProcessesByName("CollegeFB27"))using(game)try{
       if(game.HasExited||game.MainWindowHandle==IntPtr.Zero)continue;
       foreach(var nativeFile in NativeFiles){
       var key=game.Id+":"+game.StartTime.ToUniversalTime().Ticks+":"+nativeFile;DateTime next;int count;attempts.TryGetValue(key,out count);
       if(loaded.Contains(key)||count>=3||retryAt.TryGetValue(key,out next)&&DateTime.UtcNow<next)continue;
       attempts[key]=count+1;retryAt[key]=DateTime.UtcNow.AddSeconds(10);
       // An unknown remote completion must never start a second load request.
       // Payload hashes are verified by Prepare before the watcher starts.
       attempts[key]=3;
       DynamicGamedayAtmosphere.ManagedRuntimeLoader.Load(game.Id,Path.Combine(RuntimeFolder,nativeFile),Note);
       loaded.Add(key);Note(nativeFile+" loaded for game PID "+game.Id+". Native hook and feature checks follow. See "+Path.Combine(Ace.DynastyCOT.UserStorage.Folder("BeyondTheField"),nativeFile==NativeFile?"social-media-status.log":"recruiting-fog-status.log")+".");
      }
      }catch(Exception error){Note("Runtime loading failed for game PID "+game.Id+": "+error.Message+". Check the named component and its AppData diagnostics. After resolving the cause, start a fresh game.");}
      Thread.Sleep(1000);
     }
    }catch(Exception error){try{Note("Watcher stopped: "+error.Message);}catch{}}
    finally{Interlocked.Exchange(ref watching,0);}
   }){IsBackground=true,Name="Beyond the Field launch watcher"}.Start();
  }
 }
}
