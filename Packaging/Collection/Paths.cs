using File = Ace.DynastyCOT.StorageFile;
using Directory = Ace.DynastyCOT.StorageDirectory;
using Path = Ace.DynastyCOT.StoragePath;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;

namespace DynastyCOT.Collection {
 internal sealed class Fixture {
  internal readonly string Root,Parent;
  internal Fixture(string root,string parent){Root=Path.GetFullPath(root).TrimEnd('\\');Parent=Path.GetFullPath(parent).TrimEnd('\\');if(!Root.StartsWith(Parent+"\\",StringComparison.OrdinalIgnoreCase)||!File.Exists(Path.Combine(Root,".cot-fixture")))throw new IOException("Tests must use a marked isolated fixture.");Paths.NoLinks(Root,Root);}
 }
 internal static class Paths {
  internal const string State="COT-Collection/installed.json",Backups="COT-Collection/Backups",Log="COT-Collection/Logs/setup.log";
  internal static string Hash(byte[] data){using(var sha=SHA256.Create())return BitConverter.ToString(sha.ComputeHash(data)).Replace("-","").ToLowerInvariant();}
  internal static string HashFile(string file){using(var stream=File.OpenRead(file))using(var sha=SHA256.Create())return BitConverter.ToString(sha.ComputeHash(stream)).Replace("-","").ToLowerInvariant();}
  internal static string Existing(string file){return File.Exists(file)?HashFile(file):null;}
  internal static void Relative(string relative){if(String.IsNullOrWhiteSpace(relative)||Path.IsPathRooted(relative)||relative.IndexOfAny(new[]{':','\0'})>=0||relative.Replace('\\','/').Split('/').Any(x=>x==".."||x=="."||x.Length==0||x.EndsWith(" ")||x.EndsWith(".")))throw new IOException("Unsafe managed path.");}
  internal static string Resolve(string root,string relative){Relative(relative);
   if(relative.Replace('\\','/').StartsWith("COT-Collection/",StringComparison.Ordinal)){
    Ace.DynastyCOT.UserStorage.Current.MigrateCollection(root);
    string folder=Ace.DynastyCOT.UserStorage.Current.CollectionFolder(root),tail=relative.Replace('\\','/').Substring("COT-Collection/".Length);
    string target=Path.GetFullPath(Path.Combine(folder,tail.Replace('/',Path.DirectorySeparatorChar)));Ace.DynastyCOT.UserStore.NoLinks(folder,target);return target;
   }
string prefix=Path.GetFullPath(root).TrimEnd('\\')+"\\",path=Path.GetFullPath(Path.Combine(prefix,relative.Replace('/','\\')));if(!path.StartsWith(prefix,StringComparison.OrdinalIgnoreCase))throw new IOException("Managed path leaves its MMC folder.");NoLinks(root,path);return path;}
  internal static void NoLinks(string root,string path){
   string full=Path.GetFullPath(path),prefix=Path.GetFullPath(root).TrimEnd('\\');
   if(!full.Equals(prefix,StringComparison.OrdinalIgnoreCase)&&!full.StartsWith(prefix+"\\",StringComparison.OrdinalIgnoreCase)){
    string user=Ace.DynastyCOT.UserStorage.Current.CollectionFolder(root);
    if(!full.Equals(user,StringComparison.OrdinalIgnoreCase)&&!full.StartsWith(user+"\\",StringComparison.OrdinalIgnoreCase))throw new IOException("Path leaves its authorized installation/data store.");prefix=user;
   }
   Ace.DynastyCOT.UserStore.NoLinks(prefix,full);
  }
  internal static string ValidateRoot(string value){
   if(String.IsNullOrWhiteSpace(value))throw new IOException("Choose MMCModManager.exe.");string path=Path.GetFullPath(value.Trim()).TrimEnd('\\');if(File.Exists(path)&&Path.GetFileName(path).Equals("MMCModManager.exe",StringComparison.OrdinalIgnoreCase))path=Path.GetDirectoryName(path);
   if(File.Exists(Path.Combine(path,"MMCModManager.exe"))){NoLinks(path,path);return path;}
   var children=Directory.Exists(path)?Directory.GetDirectories(path,"MMC_ModManager*").Where(x=>File.Exists(Path.Combine(x,"MMCModManager.exe"))).ToArray():new string[0];
   if(children.Length>1)throw new IOException("Several Mod Managers found. Choose the one you use.");if(children.Length==1)path=children[0];if(!File.Exists(Path.Combine(path,"MMCModManager.exe")))throw new IOException("Choose the folder containing MMCModManager.exe.");NoLinks(path,path);return path;
  }
  internal static void Closed(Fixture fixture,string root){if(fixture!=null){if(!fixture.Root.Equals(root,StringComparison.OrdinalIgnoreCase))throw new IOException("Fixture root changed.");return;}RequireClosed(Process.GetProcessesByName);}
  internal static void RequireClosed(Func<string,Process[]> find){foreach(string name in new[]{"MMCModManager","CollegeFB27"})foreach(var process in find(name))using(process)throw new IOException("Close "+(name=="MMCModManager"?"MMC Mod Manager":"the game")+" before changing files. Process ID: "+process.Id+". Installed plugin files alone do not trigger this warning.");}
  static int OneWindowsCode(Exception cause){var native=cause as System.ComponentModel.Win32Exception;return native!=null?native.NativeErrorCode:((uint)cause.HResult>>16)==0x8007?cause.HResult&65535:0;}
  internal static int WindowsCode(Exception failure){for(var cause=failure;cause!=null;cause=cause.InnerException){int code=OneWindowsCode(cause);if(code!=0)return code;}return 0;}
  internal static int AntivirusCode(Exception failure){for(var cause=failure;cause!=null;cause=cause.InnerException){int code=OneWindowsCode(cause);if(code==225||code==226)return code;}return 0;}
  internal static string Explain(Exception failure){
   int antivirus=AntivirusCode(failure);if(antivirus!=0)return failure.Message+"\r\n\r\nWindows reported an antivirus block (error "+antivirus+"). Review your antivirus Protection History for the named file and detection. Keep the original plugin, history and backup details for investigation. Setup did not change antivirus settings. Administrator permission does not resolve this detection.";
   for(Exception cause=failure;cause!=null;cause=cause.InnerException){
    if(cause is PathTooLongException)return failure.Message+"\r\n\r\nWindows rejected this path or a filename component. Long COT data paths are supported; check that no individual filename exceeds Windows limits. This is not evidence that MMC is running. No permissions or antivirus settings were changed.";
    if(cause is DirectoryNotFoundException||cause is FileNotFoundException)return failure.Message+"\r\n\r\nA required file or folder is missing. Refresh Check & Repair and check the named path. This is not evidence that MMC is running. Keep your settings/history and any repair backup.";
    if(cause is UnauthorizedAccessException)return failure.Message+"\r\n\r\nWindows denied access to a file or folder. Use a writable MMC folder, or run setup as administrator if required. This does not mean MMC is running. No Windows permissions were changed.";
    int code=cause.HResult&65535;if(cause is IOException&&(code==32||code==33))return failure.Message+"\r\n\r\nWindows reports a locked file. Another app, background helper or security scanner may be using it. Close the app holding that file and retry. This does not prove MMC is running; do not delete your settings or history.";
   }return failure.Message;
  }
  internal static void Atomic(string file,byte[] bytes){Directory.CreateDirectory(Path.GetDirectoryName(file));string temp=file+"."+Guid.NewGuid().ToString("N")+".tmp";try{using(var stream=Ace.DynastyCOT.StorageIO.OpenFile(temp,FileMode.CreateNew,FileAccess.Write,FileShare.None)){stream.Write(bytes,0,bytes.Length);stream.Flush(true);}if(File.Exists(file))File.Replace(temp,file,null);else File.Move(temp,file);}finally{if(File.Exists(temp))File.Delete(temp);}}
  internal static void Writable(string root,IEnumerable<string> paths){foreach(string relative in paths){string target=Resolve(root,relative);if(Directory.Exists(target))throw new IOException("A folder blocks the required file: "+target+". Move that folder outside Plugins or choose another MMC folder; no files were changed.");if(File.Exists(target)&&(File.GetAttributes(target)&FileAttributes.ReadOnly)!=0)throw new IOException("Read-only installation file: "+target+". Remove its Read-only attribute and retry; no files were changed.");}foreach(string folder in paths.Select(p=>Path.GetDirectoryName(Resolve(root,p))).Distinct(StringComparer.OrdinalIgnoreCase)){string existing=folder;while(!Directory.Exists(existing)){if(File.Exists(existing))throw new IOException("A file blocks the required folder: "+existing);existing=Path.GetDirectoryName(existing);}string probe=Path.Combine(existing,".cot-write-"+Guid.NewGuid().ToString("N"));try{using(var file=Ace.DynastyCOT.StorageIO.OpenFile(probe,FileMode.CreateNew,FileAccess.Write,FileShare.None,4096,FileOptions.DeleteOnClose)){} }catch(UnauthorizedAccessException e){throw new IOException("Windows denied access to "+existing+". Use a writable Mod Manager folder or run setup as administrator if required. No permissions were changed.",e);}}}
 }
}
