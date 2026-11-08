// Tests release-DLL validation only. Never starts a helper, MMC or the game.
using System;using System.IO;using System.Reflection;
class RuntimePackageChecks {
 static int Main(string[] args){try {
  string sdk=args[0],dll=Path.GetFullPath(args[1]),fixture=Path.GetFullPath(args[2]),ns=args[3];
  AppDomain.CurrentDomain.AssemblyResolve+=(s,e)=>{string p=Path.Combine(sdk,new AssemblyName(e.Name).Name+".dll");return File.Exists(p)?Assembly.LoadFrom(p):null;};
  Assembly a=Assembly.LoadFrom(dll);Type t=a.GetType(ns+".RuntimeFiles",true);var flags=BindingFlags.NonPublic|BindingFlags.Static;
  var resolve=t.GetMethod("Resolve",flags);var open=t.GetMethod("OpenVerified",flags);
  string source=Path.Combine(Path.GetDirectoryName(dll),ns,args[4]),dest=Path.Combine(fixture,ns,args[4]);
  byte[] payload=File.ReadAllBytes(source);Directory.CreateDirectory(Path.GetDirectoryName(dest));File.WriteAllBytes(dest,payload);
  foreach(string r in a.GetManifestResourceNames())if(r.EndsWith(".exe")||r.EndsWith(".pdb"))throw new Exception("Helper/symbols still embedded");
  if((string)resolve.Invoke(null,new object[]{fixture})!=dest)throw new Exception("Wrong runtime location");
  using((IDisposable)open.Invoke(null,new object[]{dest})){
   bool denied=false;try{using(File.Open(dest,FileMode.Open,FileAccess.Write,FileShare.ReadWrite)){} }catch(IOException){denied=true;}
   if(!denied)throw new Exception("Helper can change during verified launch lease");
  }
  string absent=Path.Combine(fixture,"absent");Expect(resolve,absent,"Missing runtime helper",args[4]);if(Directory.Exists(absent))throw new Exception("Missing runtime folder auto-created");
  File.Delete(dest);Expect(resolve,fixture,"Missing runtime helper",args[4]);if(File.Exists(dest))throw new Exception("Missing helper auto-created");
  File.WriteAllBytes(dest,new byte[]{1,2,3});Expect(resolve,fixture,"hash mismatch",args[4]);if(File.ReadAllBytes(dest).Length!=3)throw new Exception("Wrong helper auto-overwritten");
  File.WriteAllBytes(dest,payload);resolve.Invoke(null,new object[]{fixture});
  Console.WriteLine("PASS external runtime path/hash, no embedded EXE/PDB, missing and wrong files fail closed with actionable paths, no auto-repair, launch lease denies replacement; no processes launched.");return 0;
 }catch(Exception e){Console.WriteLine(e);return 1;}}
 static void Expect(MethodInfo method,string folder,string message,string name){try{method.Invoke(null,new object[]{folder});throw new Exception("Bad helper accepted");}catch(TargetInvocationException e){if(!e.InnerException.Message.Contains(message)||!e.InnerException.Message.Contains(name))throw;}}
}
