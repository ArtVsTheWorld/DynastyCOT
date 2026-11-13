using System;
using System.IO;
using System.Security.Cryptography;
namespace SituationalPractice {
 internal static class RuntimeFiles {
  internal static string PathIn(string pluginFolder){return Path.Combine(Ace.DynastyCOT.UserStorage.Folder("ExpandedPractice"),"Runtime-"+BuildIdentity.Version,
#if COT_BRANDED_RUNTIME
   "ExpandedPractice.Runtime.exe"
#else
   "SituationalPractice.Runtime.exe"
#endif
  );}
  // Hold this read-only lease until process creation completes. Windows denies
  // replacement/deletion between verification and launch. Never extract or repair.
  internal static FileStream OpenVerified(string path) {
   if(!File.Exists(path))throw new FileNotFoundException("Missing runtime helper: "+path+". Unzip the release and copy BOTH the plugin DLL and its SituationalPractice folder into MMC's Plugins folder. Merge folders and replace matching files.",path);
   FileStream file=new FileStream(path,FileMode.Open,FileAccess.Read,FileShare.Read);
   try { string actual;using(var sha=SHA256.Create())actual=BitConverter.ToString(sha.ComputeHash(file)).Replace("-","").ToLowerInvariant();
    if(!String.Equals(actual,ExpectedRuntime.Hash,StringComparison.OrdinalIgnoreCase))throw new InvalidDataException("Runtime helper hash mismatch: "+path+". Expected SHA-256 "+ExpectedRuntime.Hash+"; actual "+actual+". Re-copy the DLL and SituationalPractice folder from the SAME release ZIP. No helper launched.");
    file.Position=0;return file;
   }catch{file.Dispose();throw;}
  }
  internal static string Resolve(string pluginFolder){string path=PathIn(pluginFolder);using(OpenVerified(path))return path;}
 }
}
