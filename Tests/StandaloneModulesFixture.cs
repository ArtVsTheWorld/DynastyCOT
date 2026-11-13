// No Entry constructors, execution callbacks, native loads or game access.
using System;using System.IO;using System.Linq;using System.Reflection;using System.Collections.Generic;using System.Security.Cryptography;using System.Web.Script.Serialization;
class StandaloneModulesFixture {
 static int checks;static void Check(bool value,string message){if(!value)throw new Exception(message);++checks;}
 static object Call(Type type,string name,params object[] args){try{return type.GetMethod(name,BindingFlags.Static|BindingFlags.Public|BindingFlags.NonPublic).Invoke(null,args);}catch(TargetInvocationException e){throw e.InnerException;}}
 static void Profile(Assembly assembly,string root){var type=assembly.GetType("Ace.DynastyCOT.UserStorage",true);var store=Activator.CreateInstance(assembly.GetType("Ace.DynastyCOT.UserStore",true),new object[]{Path.Combine(Path.GetDirectoryName(root),Path.GetFileName(root)+"-Profile","Ace","DynastyCOT")});type.GetField("current",BindingFlags.Static|BindingFlags.NonPublic).SetValue(null,store);}
 static string Hash(string file){using(var stream=File.OpenRead(file))using(var hash=SHA256.Create())return BitConverter.ToString(hash.ComputeHash(stream)).Replace("-","").ToLowerInvariant();}
 static int Main(string[] args){try{
  string mode=args[0],root=Path.GetFullPath(args[1]),parent=Path.GetFullPath(args[2]).TrimEnd('\\')+"\\",sdk=args[3];
  Check(root.StartsWith(parent,StringComparison.OrdinalIgnoreCase)&&File.Exists(Path.Combine(root,".cot-fixture")),"Isolated fixture required");
  AppDomain.CurrentDomain.AssemblyResolve+=(s,e)=>{string file=Path.Combine(sdk,new AssemblyName(e.Name).Name+".dll");return File.Exists(file)?Assembly.LoadFrom(file):null;};
  foreach(string name in new[]{"BeyondTheField","ExtraSettings","ExpandedPractice"}){
   string plugins=Path.Combine(root,"Plugins"),dll=Path.Combine(plugins,name+".dll");var assembly=Assembly.LoadFile(dll);Profile(assembly,root);var runtime=assembly.GetType("DynastyCOTStandalone.Runtime",true);var identity=assembly.GetType("DynastyCOTStandalone.Identity",true);
   string module=(string)identity.GetField("Module",BindingFlags.Static|BindingFlags.NonPublic).GetRawConstantValue();
   Check(assembly.GetName().Name==name,"Branded assembly "+name);
   if(mode=="claim"){
    Check((bool)Call(runtime,"Claim"),"Independent module guard "+module);Check(!(bool)Call(runtime,"Claim"),"Duplicate module guard "+module);continue;
   }
   if(mode=="conflict"){
    string cot=Path.Combine(plugins,"DynastyCOT.dll"),config=Path.Combine(plugins,"DynastyCOT","settings.json");Directory.CreateDirectory(Path.GetDirectoryName(config));File.WriteAllText(cot,"presence marker, never loaded");
    Check(((string)Call(runtime,"Conflict")).Contains("COT already provides"),"Missing config defaults to bundled feature");
    File.WriteAllText(config,"{\"format\":1,\"modules\":{\""+module+"\":false}}");Check(Call(runtime,"Conflict")==null,"Other COT modules can stay on");
    File.WriteAllText(config,"{bad");bool rejected=false;try{Call(runtime,"Conflict");}catch{rejected=true;}Check(rejected,"Unreadable COT choices rejected");File.Delete(config);File.Delete(cot);
    var legacy=(string[])identity.GetField("LegacyDlls",BindingFlags.Static|BindingFlags.NonPublic).GetValue(null);
    foreach(string old in legacy){string file=Path.Combine(plugins,old);File.WriteAllText(file,"legacy marker");Check(((string)Call(runtime,"Conflict")).Contains("Move the old"),"Legacy DLL rejected: "+old);Check(File.ReadAllText(file)=="legacy marker","Legacy file retained");File.Delete(file);}continue;
   }
   Check(Call(runtime,"Conflict")==null,"Independent module is eligible");
   string[] bad={"../outside.txt","C:/outside.txt","/outside.txt","COT-Runtime/../../outside.txt"};
   foreach(string path in bad){bool rejected=false;try{Call(runtime,"Resolve",path);}catch(IOException){rejected=true;}Check(rejected,"Path escape rejected");}
   Dictionary<string,string> manifest;using(var reader=new StreamReader(assembly.GetManifestResourceStream("Standalone.payload.json")))manifest=new JavaScriptSerializer().Deserialize<Dictionary<string,string>>(reader.ReadToEnd());
   Check(!manifest.Keys.Any(x=>x.Contains("RuntimeHistory/")||x.Contains("DynastyEnhancementSettings/")||x.Contains("RecruitResumeHistory/")||x.EndsWith("DynastyClocks.json")||x.EndsWith("settings.json")),"No user data in payload");
   Call(assembly.GetType("Ace.DynastyCOT.UserStorage",true),"Migrate",module,plugins);Call(runtime,"Prepare");
   foreach(var item in manifest)Check(Hash((string)Call(runtime,"Resolve",item.Key))==item.Value,"Exact materialized payload: "+item.Key);
   var stamps=manifest.Keys.ToDictionary(x=>x,x=>File.GetLastWriteTimeUtc((string)Call(runtime,"Resolve",x)));Call(runtime,"Prepare");
   Check(stamps.All(x=>File.GetLastWriteTimeUtc((string)Call(runtime,"Resolve",x.Key))==x.Value),"Matching payloads not rewritten");
   string targetKey=manifest.Keys.First(x=>x.EndsWith(".dll")||x.EndsWith(".exe")),target=(string)Call(runtime,"Resolve",targetKey);File.WriteAllText(target,"damaged owned runtime");Call(runtime,"Prepare");Check(Hash(target)==manifest[targetKey],"Integrity repair");
   string backups=(string)Call(runtime,"Resolve","Standalone-Runtime/"+module+"/Backups");Check(Directory.GetFiles(backups).Any(x=>File.ReadAllText(x)=="damaged owned runtime"),"Replaced runtime backed up");
   foreach(string file in Directory.GetFiles(plugins,"*.dll",SearchOption.AllDirectories))Check(!Path.GetFileName(file).Contains("Native"),"No recursive native DLL scan collision");
   if(name=="BeyondTheField"){
    Check(!manifest.Keys.Any(x=>Path.GetFileName(x).IndexOf("dynastyenhancer",StringComparison.OrdinalIgnoreCase)>=0||Path.GetFileName(x)=="dynasty-enhancer.json"),"New runtime filenames use new branding");
    Check(manifest.Keys.Any(x=>x.EndsWith("beyond-the-field.json")),"Renamed runtime configuration");
    var action=assembly.GetType("DynastyEnhancer.LaunchAction",true);Call(action,"Prepare");Check((string)Call(action,"ConfigurationIssue")=="","Self-contained generator resolves without global Node install");
   }else{
    string ns=name=="ExtraSettings"?"OfflineDynastySettings":"SituationalPractice";var runtimeFiles=assembly.GetType(ns+".RuntimeFiles",true);string file=(string)Call(runtimeFiles,"Resolve",plugins);Check(File.Exists(file),"Original helper identity check passes");
    using(var held=(FileStream)Call(runtimeFiles,"OpenVerified",file)){bool locked=false;try{File.WriteAllText(file,"replace while leased");}catch(IOException){locked=true;}Check(locked,"Helper cannot be replaced between verify and launch");}
   }
  }
  var gameday=Assembly.LoadFile(Path.Combine(root,"Plugins","GamedayAtmosphere.dll"));Profile(gameday,root);var atmosphere=gameday.GetType("DynamicGamedayAtmosphere.AttendanceAction",true);
  if(mode=="claim"){object[] parameters={false};Check((bool)Call(atmosphere,"TryClaimWatcher",parameters),"Gameday can independently claim alongside the other three");}
  if(mode=="files"){
   string resource=gameday.GetManifestResourceNames().SingleOrDefault(x=>x.StartsWith("Attendance.GamedayAtmosphereNative-",StringComparison.Ordinal)&&x.EndsWith(".dll",StringComparison.Ordinal));
   string file=resource!=null?resource.Substring("Attendance.".Length):(string)gameday.GetType("DynamicGamedayAtmosphere.BuildInfo",true).GetField("NativeFile",BindingFlags.Static|BindingFlags.Public).GetValue(null);
   string folder=(string)atmosphere.GetProperty("RuntimeFolder",BindingFlags.Static|BindingFlags.Public|BindingFlags.NonPublic).GetValue(null,null);
   Check(Path.GetFullPath(folder).StartsWith(Path.Combine(Path.GetDirectoryName(root),Path.GetFileName(root)+"-Profile")+"\\",StringComparison.OrdinalIgnoreCase)&&!Path.GetFullPath(folder).StartsWith(Path.Combine(root,"Plugins")+"\\",StringComparison.OrdinalIgnoreCase),"Gameday extraction stays in isolated fixture outside Plugins");
   Call(atmosphere,"Extract",file,false);string native=Path.Combine(folder,file);
   Check(File.Exists(native),"Gameday runtime coexists outside Plugins");
   if(resource!=null){using(var stream=gameday.GetManifestResourceStream(resource))using(var hash=SHA256.Create())Check(Hash(native)==BitConverter.ToString(hash.ComputeHash(stream)).Replace("-","").ToLowerInvariant(),"Exact current Gameday embedded runtime");}else Check(Hash(native)==Hash(Path.Combine(root,"COT-Support","GamedayAtmosphere",gameday.GetName().Version.ToString(3),file)),"Exact separately packaged Gameday native runtime");
  }
  Console.WriteLine("PASS "+checks+" "+mode);return 0;
 }catch(Exception error){Console.Error.WriteLine(error);return 1;}}
}
