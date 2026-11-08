using System;using System.Collections.Generic;using System.Diagnostics;using System.IO;
namespace DynastyCOT.Setup {
 internal static class MmcLocator {
  static string Preference{get{return Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),"DynastyCOT","last-manager.txt");}}
  internal static string FromExe(string path){if(!String.Equals(Path.GetFileName(path),"MMCModManager.exe",StringComparison.OrdinalIgnoreCase))throw new IOException("Choose MMCModManager.exe.");return InstallerEngine.ValidateRoot(Path.GetDirectoryName(Path.GetFullPath(path)));}
  internal static string Candidates(IEnumerable<string> starts){var found=new HashSet<string>(StringComparer.OrdinalIgnoreCase);foreach(string start in starts){if(String.IsNullOrWhiteSpace(start)||!Directory.Exists(start))continue;try{var path=InstallerEngine.FindMmc(start);if(path!=null)found.Add(path);}catch(IOException){}catch(UnauthorizedAccessException){}}if(found.Count==1)foreach(string path in found)return path;return null;}
  internal static string Find(string start){
   try{if(File.Exists(Preference)){var old=File.ReadAllText(Preference).Trim();if(File.Exists(Path.Combine(old,"MMCModManager.exe")))return InstallerEngine.ValidateRoot(old);}}catch(IOException){}catch(UnauthorizedAccessException){}
   foreach(var process in Process.GetProcessesByName("MMCModManager"))using(process)try{return FromExe(process.MainModule.FileName);}catch(System.ComponentModel.Win32Exception){}catch(InvalidOperationException){}catch(IOException){}catch(UnauthorizedAccessException){}
   var nearby=Candidates(new[]{start,Environment.CurrentDirectory});if(nearby!=null)return nearby;
   var roots=new List<string>{Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.UserProfile),"Downloads"),Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory)};
   foreach(var drive in DriveInfo.GetDrives())if(drive.DriveType==DriveType.Fixed)foreach(string relative in new[]{@"Apps\CFB 27\MMC",@"Games\CFB 27\MMC",@"MMC",@"Tools\MMC"})roots.Add(Path.Combine(drive.Name,relative));
   return Candidates(roots);
  }
  internal static void Remember(string root){if(String.IsNullOrWhiteSpace(root))return;try{Directory.CreateDirectory(Path.GetDirectoryName(Preference));File.WriteAllText(Preference,root);}catch(IOException){}catch(UnauthorizedAccessException){}}
 }
}
