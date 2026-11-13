// Embedded distribution files only. This class never patches game memory or
// starts helpers. Native scope/ownership and helper lease checks stay in modules.
using System;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Reflection;
using System.Security.Cryptography;
using System.Threading;
using System.Diagnostics;
using System.Collections.Generic;
using System.Web.Script.Serialization;
namespace DynastyCOTStandalone {
 internal static class Runtime {
  static Mutex owner;static bool ready;static readonly object sync=new object();
  internal static string Plugins {get{
   string path=Path.GetDirectoryName(typeof(Runtime).Assembly.Location);
   if(!String.Equals(Path.GetFileName(path),"Plugins",StringComparison.OrdinalIgnoreCase))throw new IOException("Put the plugin DLL directly in MMC's Plugins folder.");return path;
  }}
  internal static string Mmc {get{return Path.GetDirectoryName(Plugins);}}
  internal static string Resolve(string relative){
   if(String.IsNullOrWhiteSpace(relative)||Path.IsPathRooted(relative)||relative.Contains(":")||relative.Split('/','\\').Any(x=>x==".."))throw new IOException("Unsafe embedded path");
   string root=Ace.DynastyCOT.UserStorage.Current.Root.TrimEnd('\\')+"\\",path=Ace.DynastyCOT.UserStorage.Current.Payload(Identity.Module,relative,Identity.Version);
   if(!path.StartsWith(root,StringComparison.OrdinalIgnoreCase))throw new IOException("Embedded path escapes user storage");
   for(string current=path;current!=null&&current.StartsWith(root,StringComparison.OrdinalIgnoreCase);current=Path.GetDirectoryName(current))
    if((Directory.Exists(current)||File.Exists(current))&&(File.GetAttributes(current)&FileAttributes.ReparsePoint)!=0)throw new IOException("Linked payload paths are not supported: "+current);
   return path;
  }
  internal static string Conflict(){
   foreach(string old in Identity.LegacyDlls)if(File.Exists(Path.Combine(Plugins,old)))return "Move the old "+old+" DLL outside Plugins, then restart MMC. Keep its data/settings folders.";
   foreach(Assembly assembly in AppDomain.CurrentDomain.GetAssemblies())if(Identity.LegacyDlls.Any(x=>Path.GetFileNameWithoutExtension(x)==assembly.GetName().Name))return "An old standalone copy is loaded. Close MMC/game and remove its DLL; keep settings/history.";
   if(File.Exists(Path.Combine(Plugins,"DynastyCOT.dll"))||AppDomain.CurrentDomain.GetAssemblies().Any(x=>x.GetName().Name=="DynastyCOT")){
    string file=Path.Combine(Plugins,"DynastyCOT","settings.json");bool enabled=true;
    if(File.Exists(file)){
     var document=new JavaScriptSerializer().DeserializeObject(File.ReadAllText(file)) as Dictionary<string,object>;object format,modules,value;
     if(document==null||!document.TryGetValue("format",out format)||!(format is int)||(int)format!=1||!document.TryGetValue("modules",out modules)||!(modules is Dictionary<string,object>))throw new IOException("Unreadable COT module choices; leave files unchanged and repair that configuration first.");
     if(((Dictionary<string,object>)modules).TryGetValue(Identity.Module,out value)){
      if(!(value is bool))throw new IOException("Invalid COT module choice");enabled=(bool)value;
     }
    }
    if(enabled)return "COT already provides "+Identity.Title+". Disable that module in COT before using its standalone DLL. Other COT modules may stay enabled. Restart MMC/game after changes.";
   }
   return null;
  }
  internal static bool Claim(){bool first;var claim=new Mutex(false,"Local\\Ace.DynastyCOT.Standalone."+Identity.Module+".MMC."+Process.GetCurrentProcess().Id,out first);if(!first){claim.Dispose();return false;}owner=claim;return true;}
  static string Hash(Stream stream){using(var hash=SHA256.Create())return BitConverter.ToString(hash.ComputeHash(stream)).Replace("-","").ToLowerInvariant();}
  static Dictionary<string,string> Manifest(){
   using(var input=typeof(Runtime).Assembly.GetManifestResourceStream("Standalone.payload.json"))using(var reader=new StreamReader(input)){
    var result=new JavaScriptSerializer().Deserialize<Dictionary<string,string>>(reader.ReadToEnd());
    foreach(var item in result){Resolve(item.Key);if(item.Value.Length!=64||item.Value.Any(x=>!Uri.IsHexDigit(x)))throw new IOException("Invalid payload hash");}
    return result;
   }
  }
  internal static bool Mutable(string key){return key=="COT-Runtime/DynastyEnhancer/beyond-the-field.json";}
  internal static bool PayloadDiffers(){foreach(var item in Manifest()){string file=Resolve(item.Key);if(Mutable(item.Key)&&File.Exists(file))continue;if(!File.Exists(file))return true;using(var input=File.OpenRead(file))if(Hash(input)!=item.Value)return true;}return false;}
  // Pure file operation; invoked in tests only beneath marked isolated fixtures.
  internal static void Prepare(){Ace.DynastyCOT.UserStorage.Current.WithModule(Identity.Module,PrepareCore);}
  static void PrepareCore(){
   var manifest=Manifest();
   using(var input=typeof(Runtime).Assembly.GetManifestResourceStream("Standalone.payload.zip"))
   using(var archive=new ZipArchive(input,ZipArchiveMode.Read)){
    if(archive.Entries.Count!=manifest.Count||archive.Entries.Select(x=>x.FullName).Distinct(StringComparer.OrdinalIgnoreCase).Count()!=manifest.Count)throw new IOException("Duplicate or unexpected embedded entries");
    foreach(var entry in archive.Entries){string expected;if(!manifest.TryGetValue(entry.FullName,out expected))throw new IOException("Unmanifested embedded file");using(var source=entry.Open())if(Hash(source)!=expected)throw new IOException("Embedded payload integrity failure");}
    foreach(var entry in archive.Entries){
     string file=Resolve(entry.FullName),expected=manifest[entry.FullName];
     if(Mutable(entry.FullName)&&File.Exists(file))continue;
     if(File.Exists(file))using(var old=File.OpenRead(file))if(Hash(old)==expected)continue;
     Directory.CreateDirectory(Path.GetDirectoryName(file));string temporary=file+"."+Guid.NewGuid().ToString("N")+".tmp";
     try{
      using(var source=entry.Open())using(var output=File.Create(temporary))source.CopyTo(output);
      using(var saved=File.OpenRead(temporary))if(Hash(saved)!=expected)throw new IOException("Extracted payload integrity failure");
      if(File.Exists(file)){
       string backups=Resolve("Standalone-Runtime/"+Identity.Module+"/Backups");Directory.CreateDirectory(backups);
       string backup=Path.Combine(backups,Guid.NewGuid().ToString("N")+"-"+Path.GetFileName(file)+".bak");File.Replace(temporary,file,backup,true);
      }else File.Move(temporary,file);
     }finally{if(File.Exists(temporary))File.Delete(temporary);}
    }
   }
  }
  internal static bool Initialize(){lock(sync){
   if(ready)return true;
   string conflict=Conflict();if(conflict!=null)throw new IOException(conflict);
   if(PayloadDiffers()||Ace.DynastyCOT.UserStorage.Current.NeedsMigration(Identity.Module,Plugins)){
    var games=Process.GetProcessesByName("CollegeFB27");bool running=games.Length!=0;foreach(var game in games)game.Dispose();
    if(running)throw new IOException("Close the game once so "+Identity.Title+" can prepare its matching runtime files. Existing settings/history are kept.");
   }
   if(owner==null&&!Claim())throw new IOException("Another copy of "+Identity.Title+" already provides this module in this MMC process.");
   Ace.DynastyCOT.UserStorage.Migrate(Identity.Module,Plugins);Prepare();ready=true;return true;
  }}
  internal static void Note(string message){try{
   string folder=Resolve("Standalone-Runtime/"+Identity.Module+"/Logs");Directory.CreateDirectory(folder);string file=Path.Combine(folder,"package.log");
   if(File.Exists(file)&&new FileInfo(file).Length>1024*1024)File.WriteAllText(file,"");File.AppendAllText(file,DateTimeOffset.Now.ToString("O")+" | "+message+Environment.NewLine);
  }catch{}}
 }
}
