using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
class PackageHostChecks {
 static string sdk;static int count;
 static void Check(bool value,string reason){if(!value)throw new Exception(reason);count++;}
 static Assembly Resolve(object s,ResolveEventArgs e){string file=Path.Combine(sdk,new AssemblyName(e.Name).Name+".dll");return File.Exists(file)?Assembly.LoadFrom(file):null;}
 static int Main(string[] args){
  try{
   sdk=args[1];AppDomain.CurrentDomain.AssemblyResolve+=Resolve;
   string root=Path.GetFullPath(args[0]);Assembly dll=Assembly.LoadFrom(Path.Combine(root,"Plugins","DynastyCOT.dll"));
   var attrs=dll.GetCustomAttributesData();
   var registered=attrs.Where(x=>x.AttributeType.Name.StartsWith("RegisterExecutionAction")).ToArray();
   Check(registered.Length==1,"Expected exactly one execution registration");
   Check(((Type)registered[0].ConstructorArguments[0].Value).FullName=="DynastyCOT.LaunchAction","Wrong registered host");
   Check(attrs.Count(x=>x.AttributeType.Name.StartsWith("PluginDisplayName"))==1,"Duplicate MMC entries");
   Check(Directory.GetFiles(Path.Combine(root,"Plugins"),"*.dll",SearchOption.AllDirectories).Length==1,"Internal native DLL would be scanned as a plugin");
   Type host=dll.GetType("DynastyCOT.LaunchAction",true);MethodInfo verify=host.GetMethod("VerifyModule",BindingFlags.Public|BindingFlags.Static);
   foreach(string id in new[]{"DynastyEnhancer","GamedayAtmosphere","ExtraDynastySettings","DrivePractice"}){verify.Invoke(null,new object[]{id});count++;}
   string helper=Path.Combine(root,"Plugins","OfflineDynastySettings","OfflineDynastySettings.Persistence.exe");
   byte[] saved=File.ReadAllBytes(helper);
   try{
    byte[] changed=(byte[])saved.Clone();changed[changed.Length-1]^=1;File.WriteAllBytes(helper,changed);bool rejected=false;
    try{verify.Invoke(null,new object[]{"ExtraDynastySettings"});}catch(TargetInvocationException e){rejected=e.InnerException is IOException;}
    Check(rejected,"Changed helper was accepted");
    // Other modules must still verify when one component is damaged.
    verify.Invoke(null,new object[]{"DrivePractice"});count++;
   }finally{File.WriteAllBytes(helper,saved);}
   foreach(string name in new[]{"OfflineDynastySettings.RuntimeFiles","SituationalPractice.RuntimeFiles"}){
    Type t=dll.GetType(name,true);MethodInfo open=t.GetMethod("OpenVerified",BindingFlags.NonPublic|BindingFlags.Static);
    string file=name.StartsWith("Offline")?helper:Path.Combine(root,"Plugins","SituationalPractice","SituationalPractice.Runtime.exe");
    using((IDisposable)open.Invoke(null,new object[]{file})){count++;}
   }
   Type de=dll.GetType("DynastyEnhancer.LaunchAction",true);
   de.GetMethod("Prepare").Invoke(null,null);count++;
   string issue=(string)de.GetMethod("ConfigurationIssue").Invoke(null,null);
   Check(issue=="","Bundled news generator configuration is invalid: "+issue);
   Type paths=dll.GetType("DynastyCOT.CotPaths",true);
   MethodInfo resolve=paths.GetMethod("Resolve");
   foreach(string invalid in new[]{"../escape","C:/escape","a/../../escape"}){
    bool rejected=false;try{resolve.Invoke(null,new object[]{root,invalid});}catch(TargetInvocationException e){rejected=e.InnerException is IOException;}
    Check(rejected,"Accepted unsafe path");
   }
   Console.WriteLine("PASS: "+count+" actual COT assembly registration/resource/helper/path checks. No module engines started.");return 0;
  }catch(Exception error){Console.Error.WriteLine(error);return 1;}
 }
}
