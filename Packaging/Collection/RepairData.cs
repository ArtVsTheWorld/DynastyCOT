using File = Ace.DynastyCOT.StorageFile;
using Directory = Ace.DynastyCOT.StorageDirectory;
using Path = Ace.DynastyCOT.StoragePath;
using FileInfo = Ace.DynastyCOT.StorageFileInfo;
// Additive, verified legacy-data copies made only by a confirmed repair.
// Originals and current AppData values are never deleted or overwritten.
using System;using System.Collections.Generic;using System.IO;using System.Linq;using System.Text;using System.Security.Cryptography;using System.Threading;
namespace DynastyCOT.Collection {
 internal static class RepairData {
  internal static bool UserFile(string id,string relative){
   string p=relative.Replace('\\','/'),name=Path.GetFileName(p),top=p.Split('/')[0],ext=Path.GetExtension(p).ToLowerInvariant();
   if(name.StartsWith("game-",StringComparison.OrdinalIgnoreCase)||name.IndexOf("control",StringComparison.OrdinalIgnoreCase)>=0||name.IndexOf("request-",StringComparison.OrdinalIgnoreCase)>=0||new[]{".tmp",".pending",".stop",".dll",".exe"}.Contains(ext))return false;
   if(new[]{"Tools","Content","Runtime","Runtime-v103"}.Contains(top)||top.StartsWith("Runtime-",StringComparison.OrdinalIgnoreCase))return false;
   if(id=="BeyondTheField")return new[]{"RuntimeHistory","RecruitResumeHistory","ScoutingReports","DynastyEnhancementSettings","Logs","AnalyticsHistory"}.Contains(top)||!p.Contains("/")&&new[]{".json",".jsonl",".log",".ini"}.Contains(ext);
   if(id=="GamedayAtmosphere")return p=="DynamicGamedayAtmosphere.ini"||new[]{"History","Logs","Backups"}.Contains(top);
   if(id=="ExtraSettings")return new[]{".ini",".json",".log",".txt",".zip"}.Contains(ext)||new[]{"Diagnostics","Legacy","Backups"}.Contains(top);
   return id=="ExpandedPractice"&&(new[]{".ini",".log",".txt",".zip",".json"}.Contains(ext)||top=="Diagnostics");
  }
  static string Token(string value){return Paths.Hash(Encoding.UTF8.GetBytes(value));}
  // Bound metadata, not the number of history bytes. Contents are always streamed.
  internal const int MaximumFiles=250000,MaximumFolders=250000;
  internal const long SpaceReserve=64L*1024*1024;
  static string Hash(string file,CancellationToken cancel){
   using(var input=Ace.DynastyCOT.StorageIO.OpenFile(file,FileMode.Open,FileAccess.Read,FileShare.Read,65536,FileOptions.SequentialScan))using(var hash=SHA256.Create()){
    var buffer=new byte[65536];int length;while((length=input.Read(buffer,0,buffer.Length))!=0){cancel.ThrowIfCancellationRequested();hash.TransformBlock(buffer,0,length,buffer,0);}cancel.ThrowIfCancellationRequested();hash.TransformFinalBlock(buffer,0,0);return BitConverter.ToString(hash.Hash).Replace("-","").ToLowerInvariant();
   }
  }
  internal static List<MigrationFile> Plan(string root,Inventory inventory,IEnumerable<string> modules,CancellationToken cancel=default(CancellationToken),Action<string> progress=null){
   var result=new List<MigrationFile>();var destinations=new Dictionary<string,string>(StringComparer.OrdinalIgnoreCase);var seen=new HashSet<string>(StringComparer.OrdinalIgnoreCase);long bytes=0;int scanned=0;string plugins=Paths.Resolve(root,"Plugins");var store=Ace.DynastyCOT.UserStorage.Current;
   Action<string,string,string> file=(id,source,relative)=>{
    cancel.ThrowIfCancellationRequested();if(!seen.Add(id+"|"+source)||!File.Exists(source))return;if(++scanned>MaximumFiles)throw new IOException("Repair reached its "+MaximumFiles.ToString("N0")+"-file inventory limit. Nothing was retired. Select fewer modules for this pass; all history remains in place. Path: "+source);Paths.NoLinks(root,source);string hash=Hash(source,cancel);long length=new FileInfo(source).Length;bytes=checked(bytes+length);
    if(id=="BeyondTheField"&&relative=="dynasty-enhancer.json")relative="beyond-the-field.json";
    string destination=id+"/"+relative,prior;string current=store.Resolve(destination);
    if(!destinations.TryGetValue(destination,out prior))prior=File.Exists(current)?Hash(current,cancel):null;
    if(prior!=null&&prior!=hash)destination="Migration/"+id+"/RepairConflicts/"+Token(source.ToUpperInvariant()).Substring(0,24)+"-"+hash.Substring(0,16)+".bak";
    destinations[destination]=hash;result.Add(new MigrationFile{Source=source,Destination=destination,Hash=hash,Bytes=length});if(progress!=null&&(scanned==1||scanned%128==0))progress("Reviewing legacy data: "+scanned.ToString("N0")+" files, "+Size(bytes)+". Originals are unchanged.");
   };
   foreach(string id in modules.Distinct()){
    var scopes=new HashSet<string>(StringComparer.OrdinalIgnoreCase){plugins};
    // A verified COT component establishes a legacy Plugins scope even when
    // another module's DLL is missing from that partial backup. Only explicit
    // module-data paths below those scopes are eligible for additive copying.
    foreach(var component in inventory.Plugins)for(string dir=Path.GetDirectoryName(Paths.Resolve(root,component.Path));dir!=null&&(dir.Equals(plugins,StringComparison.OrdinalIgnoreCase)||dir.StartsWith(plugins+"\\",StringComparison.OrdinalIgnoreCase));dir=Path.GetDirectoryName(dir)){scopes.Add(dir);if(scopes.Count>10000)throw new IOException("Legacy repair scope exceeds its safe limit.");}
    var sources=new Dictionary<string,string>(StringComparer.OrdinalIgnoreCase);
    foreach(string scope in scopes){
     if(id=="BeyondTheField"){sources[Path.Combine(Path.GetDirectoryName(scope),"COT-Runtime","DynastyEnhancer")]="";sources[Path.Combine(scope,"DynastyEnhancer","Runtime-v103")]="";sources[Path.Combine(scope,"DynastyEnhancer")]="";if(Path.GetFileName(scope)=="DynastyEnhancer"||Path.GetFileName(scope)=="Runtime-v103"&&Path.GetFileName(Path.GetDirectoryName(scope))=="DynastyEnhancer")sources[scope]="";}
     else{string folder=id=="GamedayAtmosphere"?"DynamicGamedayAtmosphere":id=="ExtraSettings"?"OfflineDynastySettings":"SituationalPractice";sources[Path.Combine(scope,folder)]="";if(Path.GetFileName(scope)==folder)sources[scope]="";if(id=="ExtraSettings"){sources[Path.Combine(scope,"OfflinePlaycallSettings")]="OfflinePlaycallSettings/";if(Path.GetFileName(scope)=="OfflinePlaycallSettings")sources[scope]="OfflinePlaycallSettings/";}}
     string[] flat=id=="GamedayAtmosphere"?new[]{"DynamicGamedayAtmosphere.ini","DynamicAttendance.ini"}:id=="ExtraSettings"?new[]{"DynastyPlayClock.ini","OfflineDynastySettings.ini","DynastyClocks.json","OfflineDynastySettings.log","OfflineDynastySettings.log.1","OfflineDynastySettings.Persistence.log","DynastyPlayClock.log","OfflinePlaycall.log"}:new string[0];
     foreach(string leaf in flat)file(id,Path.Combine(scope,leaf),id=="GamedayAtmosphere"?"DynamicGamedayAtmosphere.ini":leaf);
    }
    foreach(var source in sources)if(Directory.Exists(source.Key)){
     Paths.NoLinks(root,source.Key);var todo=new Stack<string>();todo.Push(source.Key);int folders=0;
     while(todo.Count>0){cancel.ThrowIfCancellationRequested();string folder=todo.Pop();if(++folders>MaximumFolders)throw new IOException("Repair reached its folder inventory limit. Nothing was retired; all history remains in place. Path: "+folder);Paths.NoLinks(root,folder);foreach(string child in Directory.EnumerateDirectories(folder)){cancel.ThrowIfCancellationRequested();string rel=child.Substring(source.Key.Length+1).Replace('\\','/'),top=rel.Split('/')[0];if(new[]{"Tools","Content","Runtime","Runtime-v103"}.Contains(top)||top.StartsWith("Runtime-",StringComparison.OrdinalIgnoreCase))continue;if(todo.Count+folders>=MaximumFolders)throw new IOException("Repair reached its folder inventory limit. No program files were retired. Path: "+child);todo.Push(child);}
      foreach(string path in Directory.EnumerateFiles(folder)){cancel.ThrowIfCancellationRequested();string rel=path.Substring(source.Key.Length+1).Replace('\\','/');if(UserFile(id,rel))file(id,path,source.Value+rel);}
     }
    }
   }if(progress!=null)progress("Legacy review ready: "+result.Count.ToString("N0")+" files, "+Size(bytes)+". No program files changed.");return result;
  }
  internal static string Size(long bytes){return (bytes/(1024d*1024*1024)).ToString("0.00",System.Globalization.CultureInfo.InvariantCulture)+" GiB";}
  internal static string Summary(IEnumerable<MigrationFile> files){return "Legacy data: "+files.Count().ToString("N0")+" files, "+Size(files.Sum(f=>f.Bytes))+". Matching AppData copies are reused. Originals and current data are kept; conflicting versions are archived. Space is checked before copying.";}
  // Conservative allocation/metadata allowance; identical destinations count once.
  internal static long RequiredSpace(IEnumerable<MigrationFile> files,long programBytes){
   long bytes=checked(SpaceReserve+programBytes);var seen=new HashSet<string>(StringComparer.OrdinalIgnoreCase);var store=Ace.DynastyCOT.UserStorage.Current;
   // Existing targets are never overwritten: Copy must verify them or stop.
   // They therefore need no extra allocation here. Content verification stays
   // in Plan/Copy; space estimation must not repeatedly hash whole histories.
   foreach(var item in files)if(seen.Add(item.Destination)&&!File.Exists(store.Resolve(item.Destination)))bytes=checked(bytes+4096+((item.Bytes+4095)/4096)*4096);return bytes;
  }
  internal static void RequireSpace(string destination,long required,long available){if(available<required)throw new IOException("Not enough free space to preserve legacy data and repair backups at "+destination+". Need about "+Size(required)+"; available "+Size(available)+". Free space on that drive and retry. No conflicting program was retired; original history is kept.");}
  internal static void CheckSpace(IEnumerable<MigrationFile> files,long programBytes){
   var store=Ace.DynastyCOT.UserStorage.Current;long required=RequiredSpace(files,programBytes);long available;
   try{available=new DriveInfo(Path.GetPathRoot(store.Root)).AvailableFreeSpace;}catch(Exception error){if(!(error is IOException)&&!(error is ArgumentException)&&!(error is UnauthorizedAccessException))throw;throw new IOException("Could not check free space for the repair data folder: "+store.Root+". No conflicting program was retired. Check that drive and retry.",error);}RequireSpace(store.Root,required,available);
  }
  internal static void Copy(string root,IEnumerable<MigrationFile> files,Action<string> progress){
   var store=Ace.DynastyCOT.UserStorage.Current;CheckSpace(files,0);
   foreach(var item in files){Paths.NoLinks(root,item.Source);if(!File.Exists(item.Source)||Paths.HashFile(item.Source)!=item.Hash)throw new IOException("Legacy data changed after review: "+item.Source+". Refresh Check & Repair; no conflicting program was retired.");string destination=store.Resolve(item.Destination),existing=Paths.Existing(destination);if(existing!=null&&existing!=item.Hash)throw new IOException("The user-data destination changed after review: "+destination+". Refresh Check & Repair; current data was kept.");}
   int completed=0;long copied=0;foreach(var item in files){string destination=store.Resolve(item.Destination);if(Paths.Existing(destination)==item.Hash)continue;Directory.CreateDirectory(Path.GetDirectoryName(destination));string temporary=destination+"."+Guid.NewGuid().ToString("N")+".tmp";
    try{using(var input=Ace.DynastyCOT.StorageIO.OpenFile(item.Source,FileMode.Open,FileAccess.Read,FileShare.Read,65536,FileOptions.SequentialScan))using(var output=Ace.DynastyCOT.StorageIO.OpenFile(temporary,FileMode.CreateNew,FileAccess.Write,FileShare.None,65536))using(var hash=SHA256.Create()){
      var buffer=new byte[65536];int n;while((n=input.Read(buffer,0,buffer.Length))>0){output.Write(buffer,0,n);hash.TransformBlock(buffer,0,n,buffer,0);}hash.TransformFinalBlock(buffer,0,0);output.Flush(true);if(BitConverter.ToString(hash.Hash).Replace("-","").ToLowerInvariant()!=item.Hash||output.Length!=item.Bytes)throw new IOException("Legacy data changed during copy: "+item.Source);
     }if(Paths.HashFile(temporary)!=item.Hash)throw new IOException("Legacy data copy verification failed: "+item.Source);File.Move(temporary,destination);copied=checked(copied+item.Bytes);if(++completed==1||completed%128==0)progress("Preserving legacy data: "+completed.ToString("N0")+" copied, "+Size(copied)+". Originals are unchanged.");}catch(IOException error){throw new IOException("Could not preserve legacy data from "+item.Source+" to "+destination+". Original history and current AppData were kept. "+error.Message,error);}finally{if(File.Exists(temporary))File.Delete(temporary);}
   }
   foreach(var item in files)if(Paths.Existing(store.Resolve(item.Destination))!=item.Hash)throw new IOException("Legacy data verification failed: "+item.Source);progress("Legacy data preserved and verified. Copied "+completed.ToString("N0")+" files ("+Size(copied)+"); matching copies were reused.");
  }
 }
}
