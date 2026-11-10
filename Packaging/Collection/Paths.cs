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
  internal static string Resolve(string root,string relative){Relative(relative);string prefix=Path.GetFullPath(root).TrimEnd('\\')+"\\",path=Path.GetFullPath(Path.Combine(prefix,relative.Replace('/','\\')));if(!path.StartsWith(prefix,StringComparison.OrdinalIgnoreCase))throw new IOException("Managed path leaves its MMC folder.");NoLinks(root,path);return path;}
  internal static void NoLinks(string root,string path){string prefix=Path.GetFullPath(root).TrimEnd('\\');for(string current=path;current!=null&&(current.Equals(prefix,StringComparison.OrdinalIgnoreCase)||current.StartsWith(prefix+"\\",StringComparison.OrdinalIgnoreCase));current=Path.GetDirectoryName(current))if((Directory.Exists(current)||File.Exists(current))&&(File.GetAttributes(current)&FileAttributes.ReparsePoint)!=0)throw new IOException("Linked installation paths are not supported: "+current);}
  internal static string ValidateRoot(string value){
   if(String.IsNullOrWhiteSpace(value))throw new IOException("Choose MMCModManager.exe.");string path=Path.GetFullPath(value.Trim()).TrimEnd('\\');if(File.Exists(path)&&Path.GetFileName(path).Equals("MMCModManager.exe",StringComparison.OrdinalIgnoreCase))path=Path.GetDirectoryName(path);
   var children=Directory.Exists(path)?Directory.GetDirectories(path,"MMC_ModManager*").Where(x=>File.Exists(Path.Combine(x,"MMCModManager.exe"))).ToArray():new string[0];
   if(children.Length>1)throw new IOException("Several Mod Managers found. Choose the one you use.");if(children.Length==1)path=children[0];if(!File.Exists(Path.Combine(path,"MMCModManager.exe")))throw new IOException("Choose the folder containing MMCModManager.exe.");NoLinks(path,path);return path;
  }
  internal static void Closed(Fixture fixture,string root){if(fixture!=null){if(!fixture.Root.Equals(root,StringComparison.OrdinalIgnoreCase))throw new IOException("Fixture root changed.");return;}RequireClosed(Process.GetProcessesByName);}
  internal static void RequireClosed(Func<string,Process[]> find){foreach(string name in new[]{"MMCModManager","CollegeFB27"})foreach(var process in find(name))using(process)throw new IOException("Close "+(name=="MMCModManager"?"MMC Mod Manager":"the game")+" before changing files. Process ID: "+process.Id+". Installed plugin files alone do not trigger this warning.");}
  internal static string Explain(Exception failure){
   for(Exception cause=failure;cause!=null;cause=cause.InnerException){
    if(cause is UnauthorizedAccessException)return failure.Message+"\r\n\r\nWindows denied access to a file or folder. Use a writable MMC folder, or run setup as administrator if required. This does not mean MMC is running. No Windows permissions were changed.";
    int code=cause.HResult&65535;if(cause is IOException&&(code==32||code==33))return failure.Message+"\r\n\r\nWindows reports a locked file. Another app, background helper or security scanner may be using it. Close the app holding that file and retry. This does not prove MMC is running; do not delete your settings or history.";
   }return failure.Message;
  }
  internal static void Atomic(string file,byte[] bytes){Directory.CreateDirectory(Path.GetDirectoryName(file));string temp=file+"."+Guid.NewGuid().ToString("N")+".tmp";try{using(var stream=new FileStream(temp,FileMode.CreateNew,FileAccess.Write,FileShare.None)){stream.Write(bytes,0,bytes.Length);stream.Flush(true);}if(File.Exists(file))File.Replace(temp,file,null);else File.Move(temp,file);}finally{if(File.Exists(temp))File.Delete(temp);}}
  internal static void Writable(string root,IEnumerable<string> paths){foreach(string folder in paths.Select(p=>Path.GetDirectoryName(Resolve(root,p))).Distinct(StringComparer.OrdinalIgnoreCase)){string existing=folder;while(!Directory.Exists(existing)){if(File.Exists(existing))throw new IOException("A file blocks the required folder: "+existing);existing=Path.GetDirectoryName(existing);}string probe=Path.Combine(existing,".cot-write-"+Guid.NewGuid().ToString("N"));try{using(var file=new FileStream(probe,FileMode.CreateNew,FileAccess.Write,FileShare.None,4096,FileOptions.DeleteOnClose)){} }catch(UnauthorizedAccessException e){throw new IOException("Windows denied access to "+existing+". Use a writable Mod Manager folder or run setup as administrator if required. No permissions were changed.",e);}}}
 }
}
