using System;
using System.IO;
using System.IO.Compression;
using System.Reflection;
using System.Security.Cryptography;
class VerifyPackageResources {
 static string Validate(Assembly assembly,string expectedVersion){
  string version=assembly.GetName().Version.ToString(3);
  if(!String.IsNullOrEmpty(expectedVersion)&&expectedVersion!=version)throw new Exception("Package and plugin versions differ");
  string[] resources=assembly.GetManifestResourceNames();
  if(Array.IndexOf(resources,"Attendance.LoadDynamicGamedayAtmosphere.exe")>=0)throw new Exception("Release still embeds the loader EXE");
  int native=0;foreach(string name in resources)if(name.StartsWith("Attendance.DynamicGamedayAtmosphereNative-")){
   ++native;if(name!="Attendance.DynamicGamedayAtmosphereNative-"+version+".dll")throw new Exception("Native runtime and plugin versions differ");
  }
  if(native!=1)throw new Exception("Expected one matching native runtime resource");
  if(assembly.GetType("DynamicGamedayAtmosphere.ManagedRuntimeLoader",false)!=null)return "direct";
  if(assembly.GetType("DynamicGamedayAtmosphere.ExternalLoaderInfo",false)!=null)return "external";
  throw new Exception("No supported runtime loader in adapter");
 }
 static int Main(string[] args){
  try{
   if(args[0]=="--adapter"){
    var adapter=Assembly.ReflectionOnlyLoad(File.ReadAllBytes(args[1]));Console.WriteLine(Validate(adapter,null));return 0;
   }
   using(var zip=ZipFile.OpenRead(args[0])){
    var dll=zip.GetEntry("DynamicGamedayAtmosphere.dll");if(dll==null)throw new Exception("Plugin DLL missing");
    Assembly assembly;using(var stream=dll.Open())using(var buffer=new MemoryStream()){stream.CopyTo(buffer);assembly=Assembly.ReflectionOnlyLoad(buffer.ToArray());}
    string mode=Validate(assembly,args.Length>1?args[1]:null),version=assembly.GetName().Version.ToString(3);
    if(mode=="direct"){
     foreach(var entry in zip.Entries)if(entry.FullName.EndsWith(".exe",StringComparison.OrdinalIgnoreCase))throw new Exception("Direct-loading package contains an external EXE");
     Console.WriteLine("PASS: reflection-only direct-loader inspection; one matching embedded native runtime; no loader EXE or required runtime folder");return 0;
    }
    var identity=assembly.GetType("DynamicGamedayAtmosphere.ExternalLoaderInfo",true);
    string expected=(string)identity.GetField("Sha256",BindingFlags.Static|BindingFlags.NonPublic).GetRawConstantValue();
    var loader=zip.GetEntry("DynamicGamedayAtmosphere/Runtime-"+version+"/LoadDynamicGamedayAtmosphere.exe");if(loader==null)throw new Exception("Separate runtime loader missing");
    using(var stream=loader.Open())using(var hash=SHA256.Create())if(!String.Equals(BitConverter.ToString(hash.ComputeHash(stream)).Replace("-",""),expected,StringComparison.OrdinalIgnoreCase))throw new Exception("Separate runtime loader hash differs from the plugin expectation");
   }
   Console.WriteLine("PASS: reflection-only resource inspection, matching runtime version and separate-loader SHA256");return 0;
  }catch(Exception error){Console.Error.WriteLine(error.Message);return 1;}
 }
}
