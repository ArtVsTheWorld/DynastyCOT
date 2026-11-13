// Shared by all four standalone DLLs and the optional collection installer.
// File migration only: no game access, hook changes or permission changes.
using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Security.Cryptography;
using System.Collections.Generic;
using System.Threading;
using System.Reflection;
namespace Ace.DynastyCOT {
 public sealed class UserStore {
  public readonly string Root;
  public UserStore(string root){if(String.IsNullOrWhiteSpace(root)||!Path.IsPathRooted(root))throw new IOException("A full user-data path is required.");Root=Path.GetFullPath(root).TrimEnd('\\');}
  public static string Module(string id){switch(id){case "DynastyEnhancer":case "BeyondTheField":return "BeyondTheField";case "ExtraDynastySettings":case "ExtraSettings":return "ExtraSettings";case "DrivePractice":case "ExpandedPractice":return "ExpandedPractice";case "GamedayAtmosphere":return id;default:throw new IOException("Unknown module storage identity.");}}
  public string Folder(string id){return Resolve(Module(id));}
  public string Resolve(string relative){
   if(String.IsNullOrWhiteSpace(relative)||Path.IsPathRooted(relative)||relative.IndexOfAny(new[]{':','\0'})>=0||relative.Replace('\\','/').Split('/').Any(x=>x.Length==0||x=="."||x==".."||x.EndsWith(".")||x.EndsWith(" ")))throw new IOException("Unsafe user-data path.");
   string path=Path.GetFullPath(Path.Combine(Root,relative.Replace('/',Path.DirectorySeparatorChar)));
   if(!path.StartsWith(Root+"\\",StringComparison.OrdinalIgnoreCase))throw new IOException("User-data path escaped its root.");NoLinks(Root,path);return path;
  }
  public static void NoLinks(string root,string path){
   string prefix=Path.GetFullPath(root).TrimEnd('\\');
   // Validate every ancestor too: a redirected AppData/module path must not
   // silently redirect migration or extraction into an installation folder.
   for(string current=Path.GetFullPath(path);current!=null;current=Path.GetDirectoryName(current))
    if((Directory.Exists(current)||File.Exists(current))&&(File.GetAttributes(current)&FileAttributes.ReparsePoint)!=0)throw new IOException("Linked data paths need an explicit supported location: "+current);
   if(!path.Equals(prefix,StringComparison.OrdinalIgnoreCase)&&!path.StartsWith(prefix+"\\",StringComparison.OrdinalIgnoreCase))throw new IOException("Data path left its root.");
  }
  public string Payload(string module,string relative,string version=null){
   // Retain embedded resource IDs/manifest keys; remap only their destination.
   string id=Module(module),p=relative.Replace('\\','/');
   if(version!=null&&!System.Text.RegularExpressions.Regex.IsMatch(version,@"^\d+[.]\d+[.]\d+$"))throw new IOException("Invalid cached runtime version.");
   string cache=id+(version==null?"":"/Runtime-"+version);
   string prefix=id=="BeyondTheField"?"COT-Runtime/DynastyEnhancer/":id=="ExtraSettings"?"Plugins/OfflineDynastySettings/":id=="ExpandedPractice"?"Plugins/SituationalPractice/":null;
   if(prefix!=null&&p.StartsWith(prefix,StringComparison.Ordinal))return Resolve((id=="BeyondTheField"&&p.Substring(prefix.Length)=="beyond-the-field.json"?id:cache)+"/"+p.Substring(prefix.Length));
   if(id=="BeyondTheField"&&p.StartsWith("COT-Runtime/Shared/",StringComparison.Ordinal))return Resolve(cache+"/Shared/"+p.Substring("COT-Runtime/Shared/".Length));
   if(p.StartsWith("LICENSES/",StringComparison.Ordinal))return Resolve(cache+"/Licenses/"+p.Substring(9));
   if(p.StartsWith("Standalone-Runtime/"+module+"/",StringComparison.Ordinal))return Resolve(id+"/Package/"+p.Substring(("Standalone-Runtime/"+module+"/").Length));
   throw new IOException("Unrecognized embedded destination: "+relative);
  }
  static string Hash(string value){using(var sha=SHA256.Create())return BitConverter.ToString(sha.ComputeHash(Encoding.UTF8.GetBytes(value))).Replace("-","").ToLowerInvariant();}
  public static string HashFile(string path){using(var f=File.OpenRead(path))using(var sha=SHA256.Create())return BitConverter.ToString(sha.ComputeHash(f)).Replace("-","").ToLowerInvariant();}
  public static void Atomic(string file,byte[] bytes){
   Directory.CreateDirectory(Path.GetDirectoryName(file));string temp=Path.Combine(Path.GetDirectoryName(file),"cot-"+Guid.NewGuid().ToString("N").Substring(0,16)+".tmp");
   try{using(var stream=new FileStream(temp,FileMode.CreateNew,FileAccess.Write,FileShare.None)){stream.Write(bytes,0,bytes.Length);stream.Flush(true);}if(File.Exists(file))File.Replace(temp,file,null,true);else File.Move(temp,file);}finally{if(File.Exists(temp))File.Delete(temp);}
  }
  static IEnumerable<string> Files(string source,bool skipPrograms=false){
   if(!Directory.Exists(source))yield break;NoLinks(source,source);
   foreach(string file in Directory.GetFiles(source).OrderBy(x=>Path.GetFileName(x)=="beyond-the-field.json"?0:1).ThenBy(x=>x,StringComparer.OrdinalIgnoreCase)){NoLinks(source,file);yield return file;}
   foreach(string child in Directory.GetDirectories(source)){string name=Path.GetFileName(child);if(skipPrograms&&(new[]{"Tools","Content","Runtime","Runtime-v103"}.Contains(name)||name.StartsWith("Runtime-",StringComparison.OrdinalIgnoreCase)))continue;NoLinks(source,child);foreach(string file in Files(child))yield return file;}
  }
  static bool UserFile(string id,string relative){
   string p=relative.Replace('\\','/'),name=Path.GetFileName(p),top=p.Split('/')[0],ext=Path.GetExtension(name).ToLowerInvariant();
   if(name.StartsWith("game-",StringComparison.OrdinalIgnoreCase)||name.IndexOf("control",StringComparison.OrdinalIgnoreCase)>=0||name.IndexOf("request-",StringComparison.OrdinalIgnoreCase)>=0||name.EndsWith(".tmp",StringComparison.OrdinalIgnoreCase)||name.EndsWith(".pending",StringComparison.OrdinalIgnoreCase)||name.EndsWith(".stop",StringComparison.OrdinalIgnoreCase))return false;
   if(new[]{"Tools","Content","Runtime","Runtime-v103"}.Contains(top)||top.StartsWith("Runtime-",StringComparison.OrdinalIgnoreCase))return false;
   if(id=="GamedayAtmosphere")return p=="DynamicGamedayAtmosphere.ini"||new[]{"History","Logs","Backups"}.Contains(top);
   if(id=="BeyondTheField")return new[]{"RuntimeHistory","RecruitResumeHistory","DynastyEnhancementSettings","Logs","AnalyticsHistory"}.Contains(top)||(!p.Contains("/")&&(new[]{".json",".jsonl",".log",".ini"}.Contains(ext)));
   if(id=="ExtraSettings")return new[]{".ini",".json",".log",".txt",".zip"}.Contains(ext)||new[]{"Diagnostics","Legacy","Backups"}.Contains(top);
   return new[]{".ini",".log",".txt",".zip",".json"}.Contains(ext)||top=="Diagnostics";
  }
  public bool NeedsMigration(string module,string plugins){
   string id=Module(module),legacy=Path.GetFullPath(plugins),mmc=Path.GetDirectoryName(legacy);
   var sources=id=="BeyondTheField"?new[]{Path.Combine(mmc,"COT-Runtime","DynastyEnhancer"),Path.Combine(legacy,"DynastyEnhancer","Runtime-v103"),Path.Combine(legacy,"DynastyEnhancer")}:new[]{Path.Combine(legacy,id=="GamedayAtmosphere"?"DynamicGamedayAtmosphere":id=="ExtraSettings"?"OfflineDynastySettings":"SituationalPractice")};
   if(id=="ExtraSettings")sources=sources.Concat(new[]{Path.Combine(legacy,"OfflinePlaycallSettings")}).ToArray();
   foreach(string source in sources)if(Directory.Exists(source)&&!File.Exists(Resolve("Migration/"+id+"/"+Hash(source.ToUpperInvariant())+".done")))return true;
   foreach(string name in id=="GamedayAtmosphere"?new[]{"DynamicGamedayAtmosphere.ini","DynamicAttendance.ini"}:id=="ExtraSettings"?new[]{"DynastyPlayClock.ini","OfflineDynastySettings.ini","DynastyClocks.json","OfflineDynastySettings.log","OfflineDynastySettings.log.1","OfflineDynastySettings.Persistence.log","DynastyPlayClock.log","OfflinePlaycall.log"}:new string[0]){
    string source=Path.Combine(legacy,name);if(File.Exists(source)&&!File.Exists(Resolve("Migration/"+id+"/"+Hash(source.ToUpperInvariant())+".done")))return true;
   }return false;
  }
  public string CollectionFolder(string mmc){return Resolve("Collection/"+Hash(Path.GetFullPath(mmc).TrimEnd('\\').ToUpperInvariant()).Substring(0,24));}
  public void MigrateCollection(string mmc){
   string source=Path.Combine(Path.GetFullPath(mmc),"COT-Collection");if(!Directory.Exists(source))return;
   string token=Hash(Path.GetFullPath(mmc).TrimEnd('\\').ToUpperInvariant()),id="Collection/"+token.Substring(0,24),marker=Resolve("Migration/Collection/"+token+".done");if(File.Exists(marker))return;
   using(var mutex=new Mutex(false,"Local\\Ace.DynastyCOT.Storage.Collection."+Hash(Root.ToUpperInvariant()+token))){
    bool held=false;try{try{held=mutex.WaitOne(15000);}catch(AbandonedMutexException){held=true;}if(!held)throw new IOException("Another collection-data migration is active.");if(File.Exists(marker))return;
     foreach(string file in Files(source)){string relative=file.Substring(source.TrimEnd('\\').Length+1).Replace('\\','/');if(relative.EndsWith(".tmp",StringComparison.OrdinalIgnoreCase))continue;Copy(id,file,relative,token);}
     Atomic(marker,Encoding.UTF8.GetBytes(source+"\nCollection receipts/backups migrated; originals retained.\n"));
    }finally{if(held)mutex.ReleaseMutex();}
   }
  }
  public void WithModule(string module,Action action){
   string id=Module(module);using(var mutex=new Mutex(false,"Local\\Ace.DynastyCOT.Storage."+Hash(Root.ToUpperInvariant()+"/"+id))){
    bool held=false;try{try{held=mutex.WaitOne(15000);}catch(AbandonedMutexException){held=true;}if(!held)throw new IOException("Another process is preparing "+id+". Retry shortly.");action();}finally{if(held)mutex.ReleaseMutex();}
   }
  }
  public void Migrate(string module,string plugins){
   string id=Module(module),legacy=Path.GetFullPath(plugins),mmc=Path.GetDirectoryName(legacy);
   NoLinks(legacy,legacy);NoLinks(Root,Root);
   using(var mutex=new Mutex(false,"Local\\Ace.DynastyCOT.Storage."+Hash(Root.ToUpperInvariant()+"/"+id))){
    bool held=false;try{try{held=mutex.WaitOne(15000);}catch(AbandonedMutexException){held=true;}if(!held)throw new IOException("Another process is migrating "+id+" data. Retry shortly.");
     Directory.CreateDirectory(Folder(id));
     var sources=new List<string>();
     if(id=="BeyondTheField"){sources.Add(Path.Combine(mmc,"COT-Runtime","DynastyEnhancer"));sources.Add(Path.Combine(legacy,"DynastyEnhancer","Runtime-v103"));sources.Add(Path.Combine(legacy,"DynastyEnhancer"));}
     else sources.Add(Path.Combine(legacy,id=="GamedayAtmosphere"?"DynamicGamedayAtmosphere":id=="ExtraSettings"?"OfflineDynastySettings":"SituationalPractice"));
     foreach(string source in sources)Import(id,source,"");
     if(id=="ExtraSettings")Import(id,Path.Combine(legacy,"OfflinePlaycallSettings"),"OfflinePlaycallSettings/");
     // Exact earlier flat settings only, never delete/move their originals.
     if(id=="GamedayAtmosphere")foreach(string name in new[]{"DynamicGamedayAtmosphere.ini","DynamicAttendance.ini"})ImportFlat(id,legacy,name,"DynamicGamedayAtmosphere.ini");
     if(id=="ExtraSettings")foreach(string name in new[]{"DynastyPlayClock.ini","OfflineDynastySettings.ini","DynastyClocks.json","OfflineDynastySettings.log","OfflineDynastySettings.log.1","OfflineDynastySettings.Persistence.log","DynastyPlayClock.log","OfflinePlaycall.log"})ImportFlat(id,legacy,name,name);
    }finally{if(held)mutex.ReleaseMutex();}
   }
  }
  void ImportFlat(string id,string directory,string name,string target){
   string source=Path.Combine(directory,name);if(!File.Exists(source))return;NoLinks(directory,source);
   string marker=Resolve("Migration/"+id+"/"+Hash(source.ToUpperInvariant())+".done");if(File.Exists(marker))return;
   Copy(id,source,target,Hash(directory.ToUpperInvariant()));Atomic(marker,Encoding.UTF8.GetBytes(source+"\nOriginal retained.\n"));
  }
  void Import(string id,string source,string prefix){
   if(!Directory.Exists(source)||source.Equals(Folder(id),StringComparison.OrdinalIgnoreCase))return;
   string token=Hash(Path.GetFullPath(source).ToUpperInvariant()),marker=Resolve("Migration/"+id+"/"+token+".done");if(File.Exists(marker))return;
   foreach(string file in Files(source,true)){
    string relative=file.Substring(source.TrimEnd('\\').Length+1).Replace('\\','/');if(!UserFile(id,relative))continue;
    string destination=id=="BeyondTheField"&&relative=="dynasty-enhancer.json"?"beyond-the-field.json":relative;
    Copy(id,file,prefix+destination,token);
   }
   Atomic(marker,Encoding.UTF8.GetBytes(source+"\nComplete; original data retained. Existing destination data was never overwritten.\n"));
  }
  void Copy(string id,string source,string relative,string token){
   string destination=Resolve(id+"/"+relative);
   if(File.Exists(destination)){
    if(HashFile(source)==HashFile(destination))return;
    // The log records the full original name. Short content-addressed archive
    // names avoid multiplying already-long history paths on .NET Framework.
    destination=Resolve("Migration/"+id+"/Conflicts/"+token.Substring(0,16)+"/"+Hash(relative).Substring(0,24)+"-"+HashFile(source).Substring(0,16)+".bak");
    if(File.Exists(destination)){if(HashFile(source)==HashFile(destination))return;throw new IOException("Migration archive collision; originals retained.");}
   }
   Directory.CreateDirectory(Path.GetDirectoryName(destination));string temp=Path.Combine(Path.GetDirectoryName(destination),"cot-"+Guid.NewGuid().ToString("N").Substring(0,16)+".tmp");
   try{using(var input=new FileStream(source,FileMode.Open,FileAccess.Read,FileShare.Read))using(var output=new FileStream(temp,FileMode.CreateNew,FileAccess.Write,FileShare.None)){input.CopyTo(output);output.Flush(true);}if(HashFile(source)!=HashFile(temp))throw new IOException("Migration source changed during copy: "+source);File.Move(temp,destination);}finally{if(File.Exists(temp))File.Delete(temp);}
   // Record conflicts where users can find them; no private data goes to MMC.
   string report=Resolve("Migration/"+id+"/migration.log");Directory.CreateDirectory(Path.GetDirectoryName(report));File.AppendAllText(report,DateTimeOffset.Now.ToString("O")+" | copied "+source+" -> "+destination+"; original retained\n");
  }
 }
 public static class UserStorage {
  static UserStore current;
  public static UserStore Current {get{if(current==null){
   string own=typeof(UserStorage).Assembly.Location,dir=Path.GetDirectoryName(own);
   string moduleDir=Path.GetFileName(dir).StartsWith("Runtime-",StringComparison.Ordinal)?Path.GetDirectoryName(dir):dir,parent=Path.GetDirectoryName(moduleDir);
   bool helper=Path.GetExtension(own).Equals(".exe",StringComparison.OrdinalIgnoreCase)&&
     ((Path.GetFileName(own)=="ExtraSettings.Persistence.exe"&&Path.GetFileName(moduleDir)=="ExtraSettings")||
      (Path.GetFileName(own)=="ExpandedPractice.Runtime.exe"&&Path.GetFileName(moduleDir)=="ExpandedPractice"))&&
     Path.GetFileName(parent)=="DynastyCOT"&&Path.GetFileName(Path.GetDirectoryName(parent))=="Ace";
   if(helper)Interlocked.CompareExchange(ref current,new UserStore(parent),null);
   string local=Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);if(String.IsNullOrWhiteSpace(local))throw new IOException("Windows did not provide a Local AppData folder.");Interlocked.CompareExchange(ref current,new UserStore(Path.Combine(local,"Ace","DynastyCOT")),null);}return current;}}
  public static string Folder(string id){return Current.Folder(id);}
  public static void Migrate(string id,string plugins){Current.Migrate(id,plugins);}
 }
}
