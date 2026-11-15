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
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;
using System.ComponentModel;
namespace Ace.DynastyCOT {
 // Keep path normalization/security logical; use extended Unicode paths only
 // at our file-IO boundary. Never change MMC AppContext switches or registry.
 internal static partial class StorageIO {
  [StructLayout(LayoutKind.Sequential,CharSet=CharSet.Unicode)]struct FindData {
   internal uint Attributes,CreatedLow,CreatedHigh,AccessLow,AccessHigh,WrittenLow,WrittenHigh,SizeHigh,SizeLow,Reserved0,Reserved1;
   [MarshalAs(UnmanagedType.ByValTStr,SizeConst=260)]internal string Name;
   [MarshalAs(UnmanagedType.ByValTStr,SizeConst=14)]internal string Alternate;
  }
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,ExactSpelling=true,SetLastError=true)]static extern uint GetFullPathNameW(string path,uint capacity,StringBuilder value,IntPtr part);
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,ExactSpelling=true,SetLastError=true)]static extern uint GetFileAttributesW(string path);
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,ExactSpelling=true,SetLastError=true)]static extern SafeFileHandle CreateFileW(string path,uint access,uint share,IntPtr security,uint creation,uint flags,IntPtr template);
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,ExactSpelling=true,SetLastError=true)]static extern bool CreateDirectoryW(string path,IntPtr security);
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,ExactSpelling=true,SetLastError=true)]static extern IntPtr FindFirstFileW(string pattern,out FindData data);
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,ExactSpelling=true,SetLastError=true)]static extern bool FindNextFileW(IntPtr handle,out FindData data);
  [DllImport("kernel32.dll",ExactSpelling=true,SetLastError=true)]static extern bool FindClose(IntPtr handle);
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,ExactSpelling=true,SetLastError=true)]static extern bool MoveFileExW(string source,string destination,uint flags);
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,ExactSpelling=true,SetLastError=true)]static extern bool ReplaceFileW(string destination,string source,string backup,uint flags,IntPtr exclude,IntPtr reserved);
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,ExactSpelling=true,SetLastError=true)]static extern bool DeleteFileW(string path);
  sealed class StorageException:IOException {internal StorageException(string message,int code):base(message,new Win32Exception(code)){HResult=unchecked((int)(0x80070000u|(uint)code));}}
  static Exception Error(string action,string path,int code){string message=action+": "+path+" (Windows "+code+": "+new Win32Exception(code).Message+")";if(code==2)return new FileNotFoundException(message,path);if(code==3)return new DirectoryNotFoundException(message);if(code==206)return new PathTooLongException(message);if(code==5)return new UnauthorizedAccessException(message,new Win32Exception(code));return new StorageException(message,code);}
  static bool Long(string p){return p!=null&&p.Length>=248;}
  internal static string FullPath(string path){
   if(String.IsNullOrWhiteSpace(path)||path.IndexOf('\0')>=0)throw new IOException("Invalid storage path.");
   if(path.StartsWith(@"\\?\UNC\",StringComparison.OrdinalIgnoreCase))path=@"\\"+path.Substring(8);else if(path.StartsWith(@"\\?\",StringComparison.Ordinal)){if(path.Length<7||!Char.IsLetter(path[4])||path[5]!=':'||path[6]!='\\')throw new IOException("Unsupported extended storage namespace.");path=path.Substring(4);}
   if(path.StartsWith(@"\\.\",StringComparison.Ordinal))throw new IOException("Device storage paths are not supported.");
   path=path.Replace('/','\\');if(!Long(path))return Path.GetFullPath(path);
   var value=new StringBuilder(32768);uint length=GetFullPathNameW(path,(uint)value.Capacity,value,IntPtr.Zero);if(length==0)throw Error("Cannot normalize storage path",path,Marshal.GetLastWin32Error());if(length>=value.Capacity)throw new PathTooLongException("Storage path exceeds the Unicode path limit: "+path);return value.ToString();
  }
  internal static string Extended(string path){path=FullPath(path);return path.StartsWith(@"\\",StringComparison.Ordinal)?@"\\?\UNC\"+path.Substring(2):@"\\?\"+path;}
  internal static string Parent(string path){
   if(path==null)return null;if(!Long(path))return Path.GetDirectoryName(path);
   path=path.Replace('/','\\');int root=StoragePath.GetPathRoot(path).Length;
   if(path.Length<=root)return null;int end=path.LastIndexOf('\\');
   if(end<root)return path.Substring(0,root);return path.Substring(0,end);
  }
  internal static string Name(string path){int at=path.LastIndexOfAny(new[]{'\\','/'});return path.Substring(at+1);}
  static uint AttributesValue(string path){uint value=GetFileAttributesW(Extended(path));if(value!=UInt32.MaxValue)return value;int error=Marshal.GetLastWin32Error();if(error==2||error==3)return UInt32.MaxValue;throw Error("Cannot inspect storage path",path,error);}
  internal static bool FileExists(string path){if(!Long(path))return File.Exists(path);uint v=AttributesValue(path);return v!=UInt32.MaxValue&&(v&16)==0;}
  internal static bool DirectoryExists(string path){if(!Long(path))return Directory.Exists(path);uint v=AttributesValue(path);return v!=UInt32.MaxValue&&(v&16)!=0;}
  internal static FileAttributes Attributes(string path){if(!Long(path))return File.GetAttributes(path);uint v=AttributesValue(path);if(v==UInt32.MaxValue)throw new FileNotFoundException("Storage path vanished: "+path,path);return (FileAttributes)v;}
  static FileStream Open(string path,bool write,uint creation){var handle=CreateFileW(Extended(path),write?0x40000000u:0x80000000u,write?0u:1u,IntPtr.Zero,creation,0x08000000,IntPtr.Zero);if(handle.IsInvalid){int error=Marshal.GetLastWin32Error();handle.Dispose();throw Error(write?"Cannot write storage file":"Cannot read storage file",path,error);}try{return new FileStream(handle,write?FileAccess.Write:FileAccess.Read,65536,false);}catch{handle.Dispose();throw;}}
  internal static FileStream Read(string path){return Long(path)?Open(path,false,3):File.OpenRead(path);}
  internal static FileStream CreateNew(string path){return Long(path)?Open(path,true,1):new FileStream(path,FileMode.CreateNew,FileAccess.Write,FileShare.None);}
  internal static void CreateDirectory(string path){if(!Long(path)){Directory.CreateDirectory(path);return;}if(DirectoryExists(path))return;string parent=Parent(path);if(parent==null)throw new IOException("Invalid storage directory: "+path);CreateDirectory(parent);if(!CreateDirectoryW(Extended(path),IntPtr.Zero)){int e=Marshal.GetLastWin32Error();if(e!=183||!DirectoryExists(path))throw Error("Cannot create storage directory",path,e);}}
  static string[] Entries(string path,bool directories){
   if(!Long(path))return directories?Directory.GetDirectories(path):Directory.GetFiles(path);
   FindData data;IntPtr handle=FindFirstFileW(Extended(path).TrimEnd('\\')+@"\*",out data);if(handle==new IntPtr(-1)){int e=Marshal.GetLastWin32Error();if(e==2&&DirectoryExists(path))return new string[0];throw Error("Cannot enumerate storage directory",path,e);}var result=new List<string>();
   try{while(true){if(data.Name!="."&&data.Name!=".."&&((data.Attributes&16)!=0)==directories)result.Add(path.TrimEnd('\\')+"\\"+data.Name);if(!FindNextFileW(handle,out data)){int e=Marshal.GetLastWin32Error();if(e!=18)throw Error("Storage enumeration failed",path,e);break;}}}finally{FindClose(handle);}return result.ToArray();
  }
  internal static string[] Files(string path){return Entries(path,false);}
  internal static string[] Directories(string path){return Entries(path,true);}
  internal static void Move(string source,string destination){if(!Long(source)&&!Long(destination)){File.Move(source,destination);return;}if(!MoveFileExW(Extended(source),Extended(destination),0))throw Error("Cannot move verified storage file",destination,Marshal.GetLastWin32Error());}
  internal static void Replace(string source,string destination,string backup,bool ignoreMetadata){if(!Long(source)&&!Long(destination)&&(backup==null||!Long(backup))){File.Replace(source,destination,backup,ignoreMetadata);return;}if(!ReplaceFileW(Extended(destination),Extended(source),backup==null?null:Extended(backup),ignoreMetadata?2u:0u,IntPtr.Zero,IntPtr.Zero))throw Error("Cannot replace storage file",destination,Marshal.GetLastWin32Error());}
  internal static void Delete(string path){if(!Long(path)){File.Delete(path);return;}if(!DeleteFileW(Extended(path))){int e=Marshal.GetLastWin32Error();if(e!=2&&e!=3)throw Error("Cannot clean storage temporary file",path,e);}}
  internal static void AppendAllText(string path,string text){if(!Long(path)){File.AppendAllText(path,text);return;}using(var file=Open(path,true,4)){file.Seek(0,SeekOrigin.End);using(var writer=new StreamWriter(file,new UTF8Encoding(false)))writer.Write(text);}}
 }
 public sealed class UserStore {
  public readonly string Root;
  public UserStore(string root){if(String.IsNullOrWhiteSpace(root)||!Path.IsPathRooted(root))throw new IOException("A full user-data path is required.");Root=StorageIO.FullPath(root).TrimEnd('\\');}
  public static string Module(string id){switch(id){case "DynastyEnhancer":case "BeyondTheField":return "BeyondTheField";case "ExtraDynastySettings":case "ExtraSettings":return "ExtraSettings";case "DrivePractice":case "ExpandedPractice":return "ExpandedPractice";case "GamedayAtmosphere":return id;default:throw new IOException("Unknown module storage identity.");}}
  public string Folder(string id){return Resolve(Module(id));}
  public string Resolve(string relative){
   if(String.IsNullOrWhiteSpace(relative)||Path.IsPathRooted(relative)||relative.IndexOfAny(new[]{':','\0'})>=0||relative.Replace('\\','/').Split('/').Any(x=>x.Length==0||x=="."||x==".."||x.EndsWith(".")||x.EndsWith(" ")))throw new IOException("Unsafe user-data path.");
   string path=StorageIO.FullPath(Path.Combine(Root,relative.Replace('/',Path.DirectorySeparatorChar)));
   if(!path.StartsWith(Root+"\\",StringComparison.OrdinalIgnoreCase))throw new IOException("User-data path escaped its root.");NoLinks(Root,path);return path;
  }
  public static void NoLinks(string root,string path){
   string prefix=StorageIO.FullPath(root).TrimEnd('\\');
   // Validate every ancestor too: a redirected AppData/module path must not
   // silently redirect migration or extraction into an installation folder.
   for(string current=StorageIO.FullPath(path);current!=null;current=StorageIO.Parent(current))
    if((StorageIO.DirectoryExists(current)||StorageIO.FileExists(current))&&(StorageIO.Attributes(current)&FileAttributes.ReparsePoint)!=0)throw new IOException("Linked data paths need an explicit supported location: "+current);
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
  public static string HashFile(string path){using(var f=StorageIO.Read(path))using(var sha=SHA256.Create())return BitConverter.ToString(sha.ComputeHash(f)).Replace("-","").ToLowerInvariant();}
  public static void Atomic(string file,byte[] bytes){
   StorageIO.CreateDirectory(StorageIO.Parent(file));string temp=Path.Combine(StorageIO.Parent(file),"cot-"+Guid.NewGuid().ToString("N").Substring(0,16)+".tmp");
   try{using(var stream=StorageIO.CreateNew(temp)){stream.Write(bytes,0,bytes.Length);stream.Flush(true);}if(StorageIO.FileExists(file))StorageIO.Replace(temp,file,null,true);else StorageIO.Move(temp,file);}finally{if(StorageIO.FileExists(temp))StorageIO.Delete(temp);}
  }
  static IEnumerable<string> Files(string source,bool skipPrograms=false){
   if(!StorageIO.DirectoryExists(source))yield break;NoLinks(source,source);
   foreach(string file in StorageIO.Files(source).OrderBy(x=>StorageIO.Name(x)=="beyond-the-field.json"?0:1).ThenBy(x=>x,StringComparer.OrdinalIgnoreCase)){NoLinks(source,file);yield return file;}
   foreach(string child in StorageIO.Directories(source)){string name=StorageIO.Name(child);if(skipPrograms&&(new[]{"Tools","Content","Runtime","Runtime-v103"}.Contains(name)||name.StartsWith("Runtime-",StringComparison.OrdinalIgnoreCase)))continue;NoLinks(source,child);foreach(string file in Files(child))yield return file;}
  }
  static bool UserFile(string id,string relative){
   string p=relative.Replace('\\','/'),name=StorageIO.Name(p),top=p.Split('/')[0],ext=Path.GetExtension(name).ToLowerInvariant();
   if(name.StartsWith("game-",StringComparison.OrdinalIgnoreCase)||name.IndexOf("control",StringComparison.OrdinalIgnoreCase)>=0||name.IndexOf("request-",StringComparison.OrdinalIgnoreCase)>=0||name.EndsWith(".tmp",StringComparison.OrdinalIgnoreCase)||name.EndsWith(".pending",StringComparison.OrdinalIgnoreCase)||name.EndsWith(".stop",StringComparison.OrdinalIgnoreCase))return false;
   if(new[]{"Tools","Content","Runtime","Runtime-v103"}.Contains(top)||top.StartsWith("Runtime-",StringComparison.OrdinalIgnoreCase))return false;
   if(id=="GamedayAtmosphere")return p=="DynamicGamedayAtmosphere.ini"||new[]{"History","Logs","Backups"}.Contains(top);
   if(id=="BeyondTheField")return new[]{"RuntimeHistory","RecruitResumeHistory","ScoutingReports","DynastyEnhancementSettings","Logs","AnalyticsHistory"}.Contains(top)||(!p.Contains("/")&&(new[]{".json",".jsonl",".log",".ini"}.Contains(ext)));
   if(id=="ExtraSettings")return new[]{".ini",".json",".log",".txt",".zip"}.Contains(ext)||new[]{"Diagnostics","Legacy","Backups"}.Contains(top);
   return new[]{".ini",".log",".txt",".zip",".json"}.Contains(ext)||top=="Diagnostics";
  }
  public bool NeedsMigration(string module,string plugins){
   string id=Module(module),legacy=StorageIO.FullPath(plugins),mmc=StorageIO.Parent(legacy);
   var sources=id=="BeyondTheField"?new[]{Path.Combine(mmc,"COT-Runtime","DynastyEnhancer"),Path.Combine(legacy,"DynastyEnhancer","Runtime-v103"),Path.Combine(legacy,"DynastyEnhancer")}:new[]{Path.Combine(legacy,id=="GamedayAtmosphere"?"DynamicGamedayAtmosphere":id=="ExtraSettings"?"OfflineDynastySettings":"SituationalPractice")};
   if(id=="ExtraSettings")sources=sources.Concat(new[]{Path.Combine(legacy,"OfflinePlaycallSettings")}).ToArray();
   foreach(string source in sources)if(StorageIO.DirectoryExists(source)&&!StorageIO.FileExists(Resolve("Migration/"+id+"/"+Hash(source.ToUpperInvariant())+".done")))return true;
   foreach(string name in id=="GamedayAtmosphere"?new[]{"DynamicGamedayAtmosphere.ini","DynamicAttendance.ini"}:id=="ExtraSettings"?new[]{"DynastyPlayClock.ini","OfflineDynastySettings.ini","DynastyClocks.json","OfflineDynastySettings.log","OfflineDynastySettings.log.1","OfflineDynastySettings.Persistence.log","DynastyPlayClock.log","OfflinePlaycall.log"}:new string[0]){
    string source=Path.Combine(legacy,name);if(StorageIO.FileExists(source)&&!StorageIO.FileExists(Resolve("Migration/"+id+"/"+Hash(source.ToUpperInvariant())+".done")))return true;
   }return false;
  }
  public string CollectionFolder(string mmc){return Resolve("Collection/"+Hash(StorageIO.FullPath(mmc).TrimEnd('\\').ToUpperInvariant()).Substring(0,24));}
  public void MigrateCollection(string mmc){
   string source=Path.Combine(StorageIO.FullPath(mmc),"COT-Collection");if(!StorageIO.DirectoryExists(source))return;
   string token=Hash(StorageIO.FullPath(mmc).TrimEnd('\\').ToUpperInvariant()),id="Collection/"+token.Substring(0,24),marker=Resolve("Migration/Collection/"+token+".done");if(StorageIO.FileExists(marker))return;
   using(var mutex=new Mutex(false,"Local\\Ace.DynastyCOT.Storage.Collection."+Hash(Root.ToUpperInvariant()+token))){
    bool held=false;try{try{held=mutex.WaitOne(15000);}catch(AbandonedMutexException){held=true;}if(!held)throw new IOException("Another collection-data migration is active.");if(StorageIO.FileExists(marker))return;
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
   string id=Module(module),legacy=StorageIO.FullPath(plugins),mmc=StorageIO.Parent(legacy);
   NoLinks(legacy,legacy);NoLinks(Root,Root);
   using(var mutex=new Mutex(false,"Local\\Ace.DynastyCOT.Storage."+Hash(Root.ToUpperInvariant()+"/"+id))){
    bool held=false;try{try{held=mutex.WaitOne(15000);}catch(AbandonedMutexException){held=true;}if(!held)throw new IOException("Another process is migrating "+id+" data. Retry shortly.");
     StorageIO.CreateDirectory(Folder(id));
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
   string source=Path.Combine(directory,name);if(!StorageIO.FileExists(source))return;NoLinks(directory,source);
   string marker=Resolve("Migration/"+id+"/"+Hash(source.ToUpperInvariant())+".done");if(StorageIO.FileExists(marker))return;
   Copy(id,source,target,Hash(directory.ToUpperInvariant()));Atomic(marker,Encoding.UTF8.GetBytes(source+"\nOriginal retained.\n"));
  }
  void Import(string id,string source,string prefix){
   if(!StorageIO.DirectoryExists(source)||source.Equals(Folder(id),StringComparison.OrdinalIgnoreCase))return;
   string token=Hash(StorageIO.FullPath(source).ToUpperInvariant()),marker=Resolve("Migration/"+id+"/"+token+".done");if(StorageIO.FileExists(marker))return;
   foreach(string file in Files(source,true)){
    string relative=file.Substring(source.TrimEnd('\\').Length+1).Replace('\\','/');if(!UserFile(id,relative))continue;
    string destination=id=="BeyondTheField"&&relative=="dynasty-enhancer.json"?"beyond-the-field.json":relative;
    Copy(id,file,prefix+destination,token);
   }
   Atomic(marker,Encoding.UTF8.GetBytes(source+"\nComplete; original data retained. Existing destination data was never overwritten.\n"));
  }
  void Copy(string id,string source,string relative,string token){
   string destination=Resolve(id+"/"+relative);
   if(StorageIO.FileExists(destination)){
    if(HashFile(source)==HashFile(destination))return;
    // The log records the full original name. Short content-addressed archive
    // names avoid multiplying already-long history paths on .NET Framework.
    destination=Resolve("Migration/"+id+"/Conflicts/"+token.Substring(0,16)+"/"+Hash(relative).Substring(0,24)+"-"+HashFile(source).Substring(0,16)+".bak");
    if(StorageIO.FileExists(destination)){if(HashFile(source)==HashFile(destination))return;throw new IOException("Migration archive collision; originals retained.");}
   }
   StorageIO.CreateDirectory(StorageIO.Parent(destination));string temp=Path.Combine(StorageIO.Parent(destination),"cot-"+Guid.NewGuid().ToString("N").Substring(0,16)+".tmp");
   try{using(var input=StorageIO.Read(source))using(var output=StorageIO.CreateNew(temp)){input.CopyTo(output);output.Flush(true);}if(HashFile(source)!=HashFile(temp))throw new IOException("Migration source changed during copy: "+source);StorageIO.Move(temp,destination);}finally{if(StorageIO.FileExists(temp))StorageIO.Delete(temp);}
   // Record conflicts where users can find them; no private data goes to MMC.
   string report=Resolve("Migration/"+id+"/migration.log");StorageIO.CreateDirectory(StorageIO.Parent(report));StorageIO.AppendAllText(report,DateTimeOffset.Now.ToString("O")+" | copied "+source+" -> "+destination+"; original retained\n");
  }
 }
 public static class UserStorage {
  static UserStore current;
  public static UserStore Current {get{if(current==null){
   string own=typeof(UserStorage).Assembly.Location,dir=StorageIO.Parent(own);
   string moduleDir=StorageIO.Name(dir).StartsWith("Runtime-",StringComparison.Ordinal)?StorageIO.Parent(dir):dir,parent=StorageIO.Parent(moduleDir);
   bool helper=Path.GetExtension(own).Equals(".exe",StringComparison.OrdinalIgnoreCase)&&
     ((StorageIO.Name(own)=="ExtraSettings.Persistence.exe"&&StorageIO.Name(moduleDir)=="ExtraSettings")||
      (StorageIO.Name(own)=="ExpandedPractice.Runtime.exe"&&StorageIO.Name(moduleDir)=="ExpandedPractice"))&&
     StorageIO.Name(parent)=="DynastyCOT"&&StorageIO.Name(StorageIO.Parent(parent))=="Ace";
   if(helper)Interlocked.CompareExchange(ref current,new UserStore(parent),null);
   string local=Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);if(String.IsNullOrWhiteSpace(local))throw new IOException("Windows did not provide a Local AppData folder.");Interlocked.CompareExchange(ref current,new UserStore(Path.Combine(local,"Ace","DynastyCOT")),null);}return current;}}
  public static string Folder(string id){return Current.Folder(id);}
  public static void Migrate(string id,string plugins){Current.Migrate(id,plugins);}
 }
}
// These facades are private to each COT assembly. Logical paths stay unprefixed;
// only the actual Unicode Windows I/O call receives an extended path. MMC's
// process-wide AppContext switches and Windows configuration are never changed.
namespace Ace.DynastyCOT {
 internal static partial class StorageIO {
  [StructLayout(LayoutKind.Sequential)] internal struct FileData {internal uint Attributes,CreatedLow,CreatedHigh,AccessLow,AccessHigh,WrittenLow,WrittenHigh,SizeHigh,SizeLow;}
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,ExactSpelling=true,SetLastError=true)]static extern bool GetFileAttributesExW(string path,int level,out FileData data);
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,ExactSpelling=true,SetLastError=true)]static extern bool CopyFileW(string source,string target,bool failIfExists);
  [DllImport("kernel32.dll",CharSet=CharSet.Unicode,ExactSpelling=true,SetLastError=true)]static extern bool RemoveDirectoryW(string path);
  internal static void RequireExecutablePath(string file){
   // CLR executable startup happens before our helper code can normalize paths.
   // Do not launch a process that can only fail or show a Windows error dialog.
   if(FullPath(file).Length>=248)throw new PathTooLongException("Windows cannot start this .NET helper from a path this long: "+file+". COT data/history can use long paths, but the Windows Local AppData runtime location must be shorter. No helper was started and no history was changed.");
  }
  internal static string IoPath(string path){return Long(path)?Extended(path):path;}
  internal static FileData Metadata(string path){FileData data;if(!GetFileAttributesExW(Extended(path),0,out data))throw Error("Cannot inspect file metadata",path,Marshal.GetLastWin32Error());return data;}
  internal static FileStream OpenFile(string path,FileMode mode,FileAccess access=FileAccess.ReadWrite,FileShare share=FileShare.None,int buffer=4096,FileOptions options=FileOptions.None){
   if(!Long(path))return new FileStream(path,mode,access,share,buffer,options);
   if(buffer<=0)throw new ArgumentOutOfRangeException("buffer");
   if(mode<FileMode.CreateNew||mode>FileMode.Append||access<FileAccess.Read||access>FileAccess.ReadWrite||(mode==FileMode.Append&&access!=FileAccess.Write))throw new ArgumentException("Invalid file access/mode.");
   uint creation=mode==FileMode.CreateNew?1u:mode==FileMode.Create?2u:mode==FileMode.Open?3u:mode==FileMode.Truncate?5u:4u;
   uint rights=(access==FileAccess.Read||access==FileAccess.ReadWrite?0x80000000u:0u)|(access==FileAccess.Write||access==FileAccess.ReadWrite?0x40000000u:0u);
   if((share&FileShare.Inheritable)!=0)throw new NotSupportedException("Inheritable file handles are not supported.");
   var h=CreateFileW(Extended(path),rights,(uint)share,IntPtr.Zero,creation,(uint)options|0x08000000u,IntPtr.Zero);
   if(h.IsInvalid){int e=Marshal.GetLastWin32Error();h.Dispose();throw Error("Cannot open storage file",path,e);}
   try{var f=new FileStream(h,access,buffer,(options&FileOptions.Asynchronous)!=0);if(mode==FileMode.Append)f.Seek(0,SeekOrigin.End);return f;}catch{h.Dispose();throw;}
  }
  internal static string[] Enumerate(string path,string pattern,bool directories,SearchOption option){
   if(pattern==null||pattern.IndexOfAny(new[]{'\\','/',':','\0'})>=0||pattern=="..")throw new ArgumentException("Invalid search pattern.");
   var result=new List<string>();EnumerateInto(path,pattern,directories,option,result);return result.ToArray();
  }
  static void EnumerateInto(string path,string pattern,bool directories,SearchOption option,List<string> result){
   FindData data;IntPtr handle=FindFirstFileW(Extended(path).TrimEnd('\\')+"\\"+pattern,out data);
   if(handle==new IntPtr(-1)){int e=Marshal.GetLastWin32Error();if(e!=2||!DirectoryExists(path))throw Error("Cannot enumerate storage directory",path,e);}
   else try{while(true){if(data.Name!="."&&data.Name!=".."&&((data.Attributes&16)!=0)==directories)result.Add(path.TrimEnd('\\','/')+"\\"+data.Name);if(!FindNextFileW(handle,out data)){int e=Marshal.GetLastWin32Error();if(e!=18)throw Error("Storage enumeration failed",path,e);break;}}}finally{FindClose(handle);}
   if(option==SearchOption.AllDirectories)foreach(string child in Enumerate(path,"*",true,SearchOption.TopDirectoryOnly)){
    // Recursive callers may not follow a junction into another installation.
    if((Attributes(child)&FileAttributes.ReparsePoint)!=0)throw new IOException("Linked recursive storage path requires review: "+child);
    EnumerateInto(child,pattern,directories,option,result);
   }else if(option!=SearchOption.TopDirectoryOnly)throw new ArgumentOutOfRangeException("option");
  }
  internal static IEnumerable<string> EnumerateLazy(string path,string pattern,bool directories,SearchOption option){
   if(pattern==null||pattern.IndexOfAny(new[]{'\\','/',':','\0'})>=0||pattern=="..")throw new ArgumentException("Invalid search pattern.");
   if(option!=SearchOption.TopDirectoryOnly&&option!=SearchOption.AllDirectories)throw new ArgumentOutOfRangeException("option");
   FindData data;IntPtr handle=FindFirstFileW(Extended(path).TrimEnd('\\')+"\\"+pattern,out data);
   if(handle==new IntPtr(-1)){int e=Marshal.GetLastWin32Error();if(e!=2||!DirectoryExists(path))throw Error("Cannot enumerate storage directory",path,e);}
   else try{while(true){if(data.Name!="."&&data.Name!=".."&&((data.Attributes&16)!=0)==directories)yield return path.TrimEnd('\\','/')+"\\"+data.Name;if(!FindNextFileW(handle,out data)){int e=Marshal.GetLastWin32Error();if(e!=18)throw Error("Storage enumeration failed",path,e);break;}}}finally{FindClose(handle);}
   if(option==SearchOption.AllDirectories)foreach(var child in EnumerateLazy(path,"*",true,SearchOption.TopDirectoryOnly)){if((Attributes(child)&FileAttributes.ReparsePoint)!=0)throw new IOException("Linked recursive storage path requires review: "+child);foreach(var file in EnumerateLazy(child,pattern,directories,option))yield return file;}
  }
  internal static void Copy(string source,string target,bool overwrite){if(!Long(source)&&!Long(target)){File.Copy(source,target,overwrite);return;}if(!CopyFileW(Extended(source),Extended(target),!overwrite))throw Error("Cannot copy storage file",source+" -> "+target,Marshal.GetLastWin32Error());}
  internal static void RemoveDirectory(string path,bool recursive){
   if(recursive&&(Attributes(path)&FileAttributes.ReparsePoint)==0){foreach(var f in Enumerate(path,"*",false,SearchOption.TopDirectoryOnly))Delete(f);foreach(var d in Enumerate(path,"*",true,SearchOption.TopDirectoryOnly))RemoveDirectory(d,true);}
   if(!RemoveDirectoryW(Extended(path)))throw Error("Cannot remove storage directory",path,Marshal.GetLastWin32Error());
  }
 }
 internal static class StorageFile {
  internal static bool Exists(string p){try{return StorageIO.FileExists(p);}catch(ArgumentException){return false;}catch(IOException){return false;}catch(UnauthorizedAccessException){return false;}}
  internal static FileAttributes GetAttributes(string p){return StorageIO.Attributes(p);}
  internal static FileStream OpenRead(string p){return StorageIO.OpenFile(p,FileMode.Open,FileAccess.Read,FileShare.Read);}
  internal static FileStream Create(string p){return StorageIO.OpenFile(p,FileMode.Create);}
  internal static string ReadAllText(string p){return ReadAllText(p,Encoding.UTF8);}
  internal static string ReadAllText(string p,Encoding e){using(var f=OpenRead(p))using(var r=new StreamReader(f,e,true))return r.ReadToEnd();}
  internal static byte[] ReadAllBytes(string p){using(var f=OpenRead(p))using(var m=new MemoryStream()){f.CopyTo(m);return m.ToArray();}}
  internal static void WriteAllBytes(string p,byte[] b){using(var f=Create(p))f.Write(b,0,b.Length);}
  internal static void WriteAllText(string p,string t){WriteAllText(p,t,new UTF8Encoding(false));}
  internal static void WriteAllText(string p,string t,Encoding e){using(var f=StorageIO.OpenFile(p,FileMode.Create,FileAccess.Write,FileShare.Read))using(var w=new StreamWriter(f,e))w.Write(t);}
  internal static void AppendAllText(string p,string t){AppendAllText(p,t,new UTF8Encoding(false));}
  internal static void AppendAllText(string p,string t,Encoding e){using(var f=StorageIO.OpenFile(p,FileMode.Append,FileAccess.Write,FileShare.Read))using(var w=new StreamWriter(f,e))w.Write(t);}
  internal static void Move(string a,string b){StorageIO.Move(a,b);}
  internal static void Copy(string a,string b,bool overwrite){StorageIO.Copy(a,b,overwrite);}
  internal static void Copy(string a,string b){Copy(a,b,false);}
  internal static void Replace(string a,string b,string backup){StorageIO.Replace(a,b,backup,false);}
  internal static void Replace(string a,string b,string backup,bool ignore){StorageIO.Replace(a,b,backup,ignore);}
  internal static void Delete(string p){StorageIO.Delete(p);}
  internal static DateTime GetLastWriteTimeUtc(string p){if(!Exists(p))return DateTime.FromFileTimeUtc(0);var d=StorageIO.Metadata(p);return DateTime.FromFileTimeUtc(((long)d.WrittenHigh<<32)|d.WrittenLow);}
 }
 internal sealed class StorageFileInfo {
  internal readonly string FullName;internal StorageFileInfo(string p){FullName=StorageIO.FullPath(p);}
  internal long Length{get{var d=StorageIO.Metadata(FullName);if((d.Attributes&16)!=0)throw new FileNotFoundException("Expected a file.",FullName);return ((long)d.SizeHigh<<32)|d.SizeLow;}}
 }
 internal sealed class StorageDirectoryInfo {
  internal readonly string FullName;internal StorageDirectoryInfo(string p){FullName=StorageIO.FullPath(p).TrimEnd('\\');if(FullName.Length==2&&FullName[1]==':')FullName+="\\";}
  internal string Name{get{return StoragePath.GetFileName(FullName.TrimEnd('\\'));}}
  internal StorageDirectoryInfo Parent{get{string p=StorageIO.Parent(FullName);return p==null?null:new StorageDirectoryInfo(p);}}
 }
 internal static class StorageDirectory {
  internal static bool Exists(string p){try{return StorageIO.DirectoryExists(p);}catch(ArgumentException){return false;}catch(IOException){return false;}catch(UnauthorizedAccessException){return false;}}
  internal static void CreateDirectory(string p){StorageIO.CreateDirectory(p);}
  internal static string[] GetFiles(string p){return GetFiles(p,"*");}
  internal static string[] GetFiles(string p,string pattern){return GetFiles(p,pattern,SearchOption.TopDirectoryOnly);}
  internal static string[] GetFiles(string p,string pattern,SearchOption option){return StorageIO.Enumerate(p,pattern,false,option);}
  internal static string[] GetDirectories(string p){return GetDirectories(p,"*");}
  internal static string[] GetDirectories(string p,string pattern){return GetDirectories(p,pattern,SearchOption.TopDirectoryOnly);}
  internal static string[] GetDirectories(string p,string pattern,SearchOption option){return StorageIO.Enumerate(p,pattern,true,option);}
  internal static IEnumerable<string> EnumerateFiles(string p){return StorageIO.EnumerateLazy(p,"*",false,SearchOption.TopDirectoryOnly);}
  internal static IEnumerable<string> EnumerateFiles(string p,string pattern,SearchOption option){return StorageIO.EnumerateLazy(p,pattern,false,option);}
  internal static IEnumerable<string> EnumerateDirectories(string p){return StorageIO.EnumerateLazy(p,"*",true,SearchOption.TopDirectoryOnly);}
  internal static void Delete(string p){Delete(p,false);}
  internal static void Delete(string p,bool recursive){if(p.Length<248&&!recursive){Directory.Delete(p,false);return;}StorageIO.RemoveDirectory(p,recursive);}
  internal static DateTime GetCreationTime(string p){var d=StorageIO.Metadata(p);return DateTime.FromFileTimeUtc(((long)d.CreatedHigh<<32)|d.CreatedLow).ToLocalTime();}
 }
 internal static class StoragePath {
  internal const char DirectorySeparatorChar='\\';
  internal static string Combine(params string[] p){return Path.Combine(p);}
  internal static string GetFullPath(string p){return StorageIO.FullPath(p);}
  internal static string GetDirectoryName(string p){return StorageIO.Parent(p);}
  internal static string GetFileName(string p){return p==null?null:StorageIO.Name(p);}
  internal static string GetExtension(string p){if(p==null)return null;p=GetFileName(p);int at=p.LastIndexOf('.');return at<0||at==p.Length-1?"":p.Substring(at);}
  internal static string GetFileNameWithoutExtension(string p){string n=GetFileName(p);if(n==null)return null;int at=n.LastIndexOf('.');return at<0?n:n.Substring(0,at);}
  internal static bool IsPathRooted(string p){return Path.IsPathRooted(p);}
  internal static string GetTempPath(){return Path.GetTempPath();}
  internal static string GetPathRoot(string p){if(p==null)return null;p=p.Replace('/','\\');if(p.StartsWith("\\\\",StringComparison.Ordinal)){int server=p.IndexOf('\\',2);if(server<0)return p;int share=p.IndexOf('\\',server+1);return share<0?p:p.Substring(0,share);}if(p.Length>=2&&p[1]==':')return p.Substring(0,p.Length>=3&&p[2]=='\\'?3:2);return p.StartsWith("\\",StringComparison.Ordinal)?"\\":"";}
 }
}
namespace Ace.DynastyCOT {
 // Read only the bounded ECMA-335 Assembly metadata row. No assembly loading,
 // dependency resolution, constructors or persistent reflection-only cache.
 internal static class StorageAssembly {
  internal static AssemblyName GetAssemblyName(string file){
   try{return ReadName(file);}catch(EndOfStreamException e){throw new BadImageFormatException("Truncated assembly metadata.",e);}catch(OverflowException e){throw new BadImageFormatException("Invalid assembly metadata size.",e);}catch(ArgumentException e){throw new BadImageFormatException("Invalid assembly metadata value.",e);}
  }
  static AssemblyName ReadName(string file){
   using(var stream=StorageFile.OpenRead(file))using(var b=new BinaryReader(stream)){
    Func<long,int,long> range=(at,n)=>{if(at<0||n<0||at>stream.Length-n)throw new BadImageFormatException("Truncated PE metadata.");return at;};
    Func<long,uint> u32=at=>{stream.Position=range(at,4);return b.ReadUInt32();};
    Func<long,ushort> u16=at=>{stream.Position=range(at,2);return b.ReadUInt16();};
    if(u16(0)!=0x5a4d)throw new BadImageFormatException("Not a PE image.");long pe=u32(60);if(u32(pe)!=0x4550)throw new BadImageFormatException("Invalid PE header.");
    int sections=u16(pe+6),optional=u16(pe+20);if(sections<1||sections>96)throw new BadImageFormatException("Invalid PE sections.");long opt=pe+24;int magic=u16(opt),directory=magic==0x20b?112:magic==0x10b?96:0;if(directory==0||optional<directory+120)throw new BadImageFormatException("No CLI header.");
    Func<uint,long> rva=value=>{for(int i=0;i<sections;i++){long at=opt+optional+i*40;uint address=u32(at+12),bytes=u32(at+16);if(value>=address&&(ulong)value-address<bytes)return range((long)u32(at+20)+value-address,1);}throw new BadImageFormatException("Metadata RVA is outside file sections.");};
    uint cliRva=u32(opt+directory+112);if(cliRva==0)throw new BadImageFormatException("Not a managed assembly.");long cli=rva(cliRva),metadata=rva(u32(cli+8));long metadataEnd=range(metadata,(int)u32(cli+12))+(long)u32(cli+12);if(u32(metadata)!=0x424a5342)throw new BadImageFormatException("Invalid metadata signature.");
    uint version=u32(metadata+12);if(version>4096)throw new BadImageFormatException("Invalid metadata version.");long header=metadata+16+((version+3)&~3u);int streams=u16(header+2);if(streams<1||streams>16)throw new BadImageFormatException("Invalid metadata streams.");long cursor=header+4,tables=0,strings=0;uint stringBytes=0;
    for(int i=0;i<streams;i++){uint offset=u32(cursor),size=u32(cursor+4);cursor+=8;var name=new StringBuilder();for(int k=0;k<32;k++){stream.Position=range(cursor++,1);byte c=b.ReadByte();if(c==0)break;name.Append((char)c);if(k==31)throw new BadImageFormatException("Unterminated metadata stream.");}cursor=(cursor+3)&~3L;long at=metadata+offset;if(at<metadata||at>metadataEnd-size)throw new BadImageFormatException("Metadata stream out of bounds.");if(name.ToString()=="#~"||name.ToString()=="#-")tables=at;if(name.ToString()=="#Strings"){strings=at;stringBytes=size;}}
    if(tables==0||strings==0)throw new BadImageFormatException("Missing metadata tables.");stream.Position=range(tables+6,18);byte heaps=b.ReadByte();b.ReadByte();ulong valid=b.ReadUInt64();b.ReadUInt64();uint[] rows=new uint[64];for(int i=0;i<64;i++)if((valid&(1ul<<i))!=0)rows[i]=b.ReadUInt32();long data=stream.Position;
    int str=(heaps&1)!=0?4:2,guid=(heaps&2)!=0?4:2,blob=(heaps&4)!=0?4:2;Func<int,int> idx=t=>rows[t]>=65536?4:2;
    Func<int,int[],int> coded=(bits,types)=>types.Any(t=>rows[t]>=(1u<<(16-bits)))?4:2;
    int td=coded(2,new[]{2,1,27}),member=coded(1,new[]{6,10});int[] sizes={
     2+str+3*guid,coded(2,new[]{0,26,35,1})+2*str,4+2*str+td+idx(4)+idx(6),idx(4),2+str+blob,idx(6),8+str+blob+idx(8),idx(8),4+str,idx(2)+td,
     coded(3,new[]{2,1,26,6,27})+str+blob,2+coded(2,new[]{4,8,23})+blob,coded(5,new[]{6,4,1,2,8,9,10,0,14,23,20,17,26,27,32,35,38,39,40,42,44,43})+coded(3,new[]{6,10})+blob,coded(1,new[]{4,8})+blob,2+coded(2,new[]{2,6,32})+blob,6+idx(2),4+idx(4),blob,idx(2)+idx(20),idx(20),2+str+td,idx(2)+idx(23),idx(23),2+str+blob,2+idx(6)+coded(1,new[]{20,23}),idx(2)+2*member,str,blob,2+coded(1,new[]{4,6})+str+idx(26),4+idx(4),8,4};
    for(int i=0;i<32;i++)data=checked(data+(long)sizes[i]*rows[i]);if(rows[32]!=1)throw new BadImageFormatException("Missing Assembly identity row.");range(data,16+blob+2*str);
    ushort major=u16(data+4),minor=u16(data+6),build=u16(data+8),revision=u16(data+10);uint nameIndex=str==2?u16(data+16+blob):u32(data+16+blob);
    if(nameIndex>=stringBytes)throw new BadImageFormatException("Assembly name outside string heap.");stream.Position=strings+nameIndex;var utf8=new List<byte>();bool terminated=false;for(uint i=nameIndex;i<stringBytes&&utf8.Count<=1024;i++){byte c=b.ReadByte();if(c==0){terminated=true;break;}utf8.Add(c);}if(!terminated||utf8.Count==0)throw new BadImageFormatException("Invalid assembly name.");
    return new AssemblyName{ Name=new UTF8Encoding(false,true).GetString(utf8.ToArray()),Version=new Version(major,minor,build,revision) };
   }
  }
 }
 internal sealed class StorageVersionInfo {
  internal string FileVersion,ProductVersion,ProductName;
  [DllImport("version.dll",CharSet=CharSet.Unicode,ExactSpelling=true,SetLastError=true)]static extern uint GetFileVersionInfoSizeW(string file,out uint ignored);
  [DllImport("version.dll",CharSet=CharSet.Unicode,ExactSpelling=true,SetLastError=true)]static extern bool GetFileVersionInfoW(string file,uint ignored,uint size,byte[] data);
  [DllImport("version.dll",CharSet=CharSet.Unicode,ExactSpelling=true)]static extern bool VerQueryValueW(byte[] data,string block,out IntPtr value,out uint length);
  static string Text(byte[] data,string key){IntPtr translation;uint length;if(!VerQueryValueW(data,@"\VarFileInfo\Translation",out translation,out length)||length<4)return null;int lang=(ushort)Marshal.ReadInt16(translation),page=(ushort)Marshal.ReadInt16(translation,2);IntPtr text;return VerQueryValueW(data,"\\StringFileInfo\\"+lang.ToString("x4")+page.ToString("x4")+"\\"+key,out text,out length)&&length>0?Marshal.PtrToStringUni(text):null;}
  internal static StorageVersionInfo GetVersionInfo(string file){
   var result=new StorageVersionInfo();uint ignored;string path=StorageIO.IoPath(file);uint size=GetFileVersionInfoSizeW(path,out ignored);if(size==0){if(!StorageFile.Exists(file))throw new FileNotFoundException("Version file is missing.",file);return result;}if(size>16*1024*1024)throw new IOException("File version resource exceeds its limit.");var data=new byte[size];if(!GetFileVersionInfoW(path,0,size,data))throw new Win32Exception(Marshal.GetLastWin32Error());// VerQueryValue returns interior pointers into this buffer. Keep it
   // pinned across every query/string allocation, not only each P/Invoke.
   var pin=GCHandle.Alloc(data,GCHandleType.Pinned);try{result.FileVersion=Text(data,"FileVersion");result.ProductVersion=Text(data,"ProductVersion");result.ProductName=Text(data,"ProductName");return result;}finally{pin.Free();}
  }
 }
}
