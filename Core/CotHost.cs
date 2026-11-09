using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Security.Cryptography;
using System.Threading;
using Frosty.Core;
using Frosty.Core.Attributes;
using FrostySdk.Interfaces;
[assembly: AssemblyTitle("Dynasty COT")]
[assembly: AssemblyProduct("Dynasty COT")]
[assembly: AssemblyDescription("Dynasty additions by Ace")]
[assembly: AssemblyVersion(DynastyCOT.ProductInfo.AssemblyVersion)]
[assembly: AssemblyFileVersion(DynastyCOT.ProductInfo.AssemblyVersion)]
[assembly: PluginVersion(DynastyCOT.ProductInfo.Version)]
[assembly: PluginAuthor("Ace")]
[assembly: PluginDisplayName("Dynasty COT")]
[assembly: RegisterExecutionAction(typeof(DynastyCOT.LaunchAction))]
namespace DynastyCOT {
 public sealed class LaunchAction : ExecutionAction {
  private static int started;
  private static readonly object sync=new object();
  private static Dictionary<string,string> status=new Dictionary<string,string>();
  public LaunchAction(){using(var host=Process.GetCurrentProcess())if(String.Equals(host.ProcessName,"MMCModManager",StringComparison.OrdinalIgnoreCase))Start(null,PluginManagerType.ModManager,CancellationToken.None);}
  public override Action<ILogger,PluginManagerType,CancellationToken> PreLaunchAction {get{return Start;}}
  public override Action<ILogger,PluginManagerType,CancellationToken> PostLaunchAction {get{return Start;}}
  public static void VerifyModule(string module) {
   bool found=false;
   using(var source=typeof(LaunchAction).Assembly.GetManifestResourceStream("DynastyCOT.payload.txt")){
    if(source==null)throw new IOException("Missing COT payload manifest.");
    using(var reader=new StreamReader(source))for(string line;(line=reader.ReadLine())!=null;){
     if(line.Length==0)continue;string[] entry=line.Split('|');
     if(entry.Length!=3 || entry[1].Length!=64)throw new IOException("Invalid COT payload manifest.");
     if(entry[0]!=module)continue;found=true;
     using(var hash=SHA256.Create()){
      string actual;
      if(entry[2].StartsWith("resource:",StringComparison.Ordinal)){
       using(var input=typeof(LaunchAction).Assembly.GetManifestResourceStream(entry[2].Substring(9))){
        if(input==null)throw new IOException("Missing embedded COT payload.");actual=BitConverter.ToString(hash.ComputeHash(input)).Replace("-","").ToLowerInvariant();
       }
      }else{
       string file=CotPaths.Resolve(CotPaths.Mmc,entry[2]);
       using(var input=File.OpenRead(file))actual=BitConverter.ToString(hash.ComputeHash(input)).Replace("-","").ToLowerInvariant();
      }
      if(actual!=entry[1])throw new IOException("COT payload differs: "+entry[2]);
     }
    }
   }
   if(!found)throw new IOException("Module has no verified payload: "+module);
  }
  private static void Start(ILogger logger,PluginManagerType type,CancellationToken token){
   if(Interlocked.CompareExchange(ref started,1,0)!=0)return;
   try {
    Directory.CreateDirectory(CotPaths.Data);
    Configuration config=Configuration.Load(Path.Combine(CotPaths.Data,"settings.json"));
    var states=ModulePlanner.StartSelected(config,id=>{
     string legacy=id=="GamedayAtmosphere"?"DynamicGamedayAtmosphere":id=="ExtraDynastySettings"?"OfflineDynastySettings":id=="DrivePractice"?"SituationalPractice":"DynastyEnhancer";
     foreach(Assembly assembly in AppDomain.CurrentDomain.GetAssemblies())if(assembly.GetName().Name==legacy)throw new IOException("Standalone "+legacy+" is loaded; close MMC and install COT with its installer.");
     VerifyModule(id);
    },id=>{
     ExecutionAction action;
     switch(id){
      case "DynastyEnhancer":action=new DynastyEnhancer.LaunchAction();break;
      case "GamedayAtmosphere":action=new DynamicGamedayAtmosphere.AttendanceAction();break;
      case "ExtraDynastySettings":action=new OfflineDynastySettings.CombinedSettingsAction();break;
      case "DrivePractice":action=new SituationalPractice.PracticeAction();break;
      default:throw new InvalidOperationException("Unknown module.");
     }
     action.PreLaunchAction(logger,type,token);
    });
    lock(sync)status=states;
    using(var writer=new StreamWriter(Path.Combine(CotPaths.Data,"launcher-status.txt"),false)){
     writer.WriteLine("Dynasty COT "+ProductInfo.Version+" | "+DateTimeOffset.UtcNow.ToString("O"));
     writer.WriteLine("Startup status only. See each component log for native activation.");
     foreach(var item in states)writer.WriteLine(item.Key+": "+item.Value);
    }
   }catch(Exception error){try{Directory.CreateDirectory(CotPaths.Data);File.WriteAllText(Path.Combine(CotPaths.Data,"launcher-error.txt"),error.ToString());}catch{}}
  }
 }
}
