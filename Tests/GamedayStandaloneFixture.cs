// File-only/OS-mutex tests of the actual production assembly. Never calls Start,
// execution callbacks, native LoadLibrary or any game-process API.
using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Diagnostics;
using System.Threading;
class GamedayStandaloneFixture {
 static int count;
 static void Check(bool value,string message){if(!value)throw new Exception(message);++count;}
 static object Call(Type type,string name,params object[] args){return type.GetMethod(name,BindingFlags.Public|BindingFlags.NonPublic|BindingFlags.Static).Invoke(null,args);}
 static byte[] Bytes(string file){return File.ReadAllBytes(file);}
 static void Same(string file,byte[] expected){Check(Bytes(file).SequenceEqual(expected),"Changed bytes: "+file);}
 static int Main(string[] args){try{
  string root=Path.GetFullPath(args[1]),parent=Path.GetFullPath(args[2]).TrimEnd('\\')+"\\",sdk=args[3];
  Check(root.StartsWith(parent,StringComparison.OrdinalIgnoreCase)&&File.Exists(Path.Combine(root,".cot-fixture")),"Marked isolated fixture required");
  AppDomain.CurrentDomain.AssemblyResolve+=(s,e)=>{string path=Path.Combine(sdk,new AssemblyName(e.Name).Name+".dll");return File.Exists(path)?Assembly.LoadFrom(path):null;};
  string runtime=args.Length>5?args[5]:"2.1.9";
  string plugins=Path.Combine(root,"Plugins"),dll=Path.Combine(plugins,"GamedayAtmosphere.dll");
  var assembly=Assembly.LoadFile(dll);var user=assembly.GetType("Ace.DynastyCOT.UserStorage",true);user.GetField("current",BindingFlags.NonPublic|BindingFlags.Static).SetValue(null,Activator.CreateInstance(assembly.GetType("Ace.DynastyCOT.UserStore",true),new object[]{Path.Combine(Path.GetDirectoryName(root),Path.GetFileName(root)+"-Profile","Ace","DynastyCOT")}));var action=assembly.GetType("DynamicGamedayAtmosphere.AttendanceAction",true);var storage=assembly.GetType("DynamicGamedayAtmosphere.PluginStorage",true);
  string mutex="Local\\Ace.DynamicGamedayAtmosphere.MMC."+Process.GetCurrentProcess().Id;
  if(args[0]=="cot-first"){
   bool first;using(var owner=new Mutex(false,mutex,out first)){
    Check(first,"Fresh COT-equivalent guard");object[] parameters={false};Check(!(bool)Call(action,"TryClaimWatcher",parameters)&&(bool)parameters[0],"Standalone must yield to bundled guard");
   }
  }else if(args[0]=="standalone-first"){
   object[] parameters={false};Check((bool)Call(action,"TryClaimWatcher",parameters)&&!(bool)parameters[0],"Standalone claims guard");
   bool first;using(var other=new Mutex(false,mutex,out first))Check(!first,"Bundled guard sees existing standalone owner");
   Check(!(bool)Call(action,"TryClaimWatcher",new object[]{false}),"Repeated callback does not restart");
   string copy=Path.Combine(plugins,"Duplicate","GamedayAtmosphere.dll");Directory.CreateDirectory(Path.GetDirectoryName(copy));File.Copy(dll,copy);
   var duplicate=Assembly.LoadFile(copy).GetType(action.FullName,true);object[] duplicateArgs={false};
   Check(!(bool)Call(duplicate,"TryClaimWatcher",duplicateArgs)&&(bool)duplicateArgs[0],"Separate adapter instance cannot start a second watcher");
  }else if(args[0]=="files"){
   Check(assembly.GetName().Name=="GamedayAtmosphere","Branded assembly name");
   Check(assembly.GetName().Version==new Version(args.Length>4?args[4].Split('-')[0]+".0":"1.0.2.0"),"Distribution assembly version");
   var attributes=assembly.GetCustomAttributesData();
   foreach(string name in new[]{"PluginAuthorAttribute","PluginDisplayNameAttribute","PluginVersionAttribute"}){
    var attr=attributes.Single(x=>x.AttributeType.Name==name);string value=(string)attr.ConstructorArguments[0].Value;
    Check(value==(name=="PluginAuthorAttribute"?"Ace":name=="PluginDisplayNameAttribute"?"Ace\u2019s Gameday Atmosphere":(args.Length>4?args[4]:"1.0.2-standalone-test")),"MMC branding "+name);
   }
   var names=assembly.GetManifestResourceNames();Check((names.Length==3||names.Length==2)&&!names.Any(x=>x.Contains("LoadDynamic")),"Exactly engine/defaults/diagnostics resources; no loader EXE");
   if(!names.Any(x=>x.EndsWith(".dll"))){string leaf="GamedayAtmosphereNative-"+runtime+".dll",external=Path.Combine(root,"COT-Support","GamedayAtmosphere",runtime,leaf);byte[] original=Bytes(external);Call(action,"Extract",leaf,false);string cached=Path.Combine((string)Call(storage,"DataFolder",plugins),"Runtime-"+runtime,leaf);Same(cached,original);File.WriteAllText(external,"tampered");bool rejected=false;try{Call(action,"Extract",leaf,false);}catch{rejected=true;}Check(rejected,"External native tampering is rejected before loading");Same(cached,original);File.WriteAllBytes(external,original);}
   Check(Call(action,"LegacyStandaloneIssue")==null,"No false legacy conflict");
   string legacy=Path.Combine(plugins,"DynamicGamedayAtmosphere.dll");File.WriteAllText(legacy,"old adapter marker");
   Check(((string)Call(action,"LegacyStandaloneIssue")).Contains("Keep its settings/history"),"Clear old DLL conflict guidance");Same(legacy,System.Text.Encoding.UTF8.GetBytes("old adapter marker"));File.Delete(legacy);
   string data=(string)Call(storage,"DataFolder",plugins),ini=(string)Call(storage,"SettingsFile",plugins);Directory.CreateDirectory(data);
   string[] histories={"attendance-history.txt","audience-foundations.txt","visitor-plans.txt"};
   foreach(string name in histories){string path=Path.Combine(data,"History",name);Directory.CreateDirectory(Path.GetDirectoryName(path));File.WriteAllText(path,"keep existing "+name);}
   string defaults;using(var reader=new StreamReader(assembly.GetManifestResourceStream("Attendance.DynamicGamedayAtmosphere.ini")))defaults=reader.ReadToEnd();
   // Current, old reset-version, and future-schema choices must all remain exact.
   foreach(string text in new[]{"[General]\r\nConfigVersion=2\r\nSettingsResetVersion=4\r\n[Broadcast]\r\nCameraShake=0\r\n","[General]\nConfigVersion=1\nSettingsResetVersion=0\nCustom=keep\n","[General]\nConfigVersion=999\nFutureOption=keep\n"}){
    File.WriteAllText(ini,text,System.Text.Encoding.Unicode);byte[] before=Bytes(ini);Call(storage,"EnsurePreserving",plugins,defaults);Same(ini,before);Call(storage,"EnsurePreserving",plugins,defaults);Same(ini,before);
   }
   File.Delete(ini);
   foreach(string oldName in new[]{"DynamicGamedayAtmosphere.ini","DynamicAttendance.ini"}){
    string old=Path.Combine(plugins,oldName);byte[] original=System.Text.Encoding.Unicode.GetBytes("[General]\r\nCustomProfile=unchanged "+oldName);File.WriteAllBytes(old,original);
    Call(storage,"EnsurePreserving",plugins,defaults);Same(old,original);Same(ini,original);File.Delete(old);File.Delete(ini);
   }
   Call(storage,"EnsurePreserving",plugins,defaults);Check(File.ReadAllText(ini)==defaults,"Fresh install gets released defaults");
   string[] oldHelpers={"DynamicGamedayAtmosphereNative-2.0.35.dll","GamedayAtmosphereNative-2.1.9.dll","LoadDynamicGamedayAtmosphere.exe","DgaDiagnostics.exe"};
   foreach(string name in oldHelpers)File.WriteAllText(Path.Combine(plugins,name),"old helper "+name);
   string unrelated=Path.Combine(plugins,"AnotherPlugin.dll");File.WriteAllText(unrelated,"unrelated plugin");
   Check((int)Call(storage,"RetireLooseRuntimeFiles",plugins)==4,"Archive exact loose native/helper names");
   foreach(string name in oldHelpers)Check(File.Exists(Path.Combine(plugins,name)),"Read-only legacy helper retained: "+name);
   Check((int)Call(storage,"RetireLooseRuntimeFiles",plugins)==4,"Legacy files remain read-only migration inputs");
   Check(File.ReadAllText(unrelated)=="unrelated plugin","Unrelated plugin unchanged");
   string[] archived=Directory.GetFiles(Path.Combine(data,"Retired","Loose"));
   foreach(string name in oldHelpers)Check(archived.Any(x=>File.ReadAllText(x)=="old helper "+name),"Exact backup bytes retained");
   byte[] chosen=Bytes(ini);Call(action,"Extract","DynamicGamedayAtmosphere.ini",true);Same(ini,chosen);
   foreach(string name in names.Where(x=>!x.EndsWith(".ini"))){
    string leaf=name.Substring("Attendance.".Length);Call(action,"Extract",leaf,false);
    string target=leaf.EndsWith(".dll")?Path.Combine(data,"Runtime-"+runtime,leaf):Path.Combine(data,"Tools",leaf);
    byte[] expected;using(var input=assembly.GetManifestResourceStream(name))using(var buffer=new MemoryStream()){input.CopyTo(buffer);expected=buffer.ToArray();}
    Same(target,expected);DateTime modified=File.GetLastWriteTimeUtc(target);Call(action,"Extract",leaf,false);Check(File.GetLastWriteTimeUtc(target)==modified,"Identical runtime not rewritten");
    File.WriteAllText(target,"damaged cache");Call(action,"Extract",leaf,false);Same(target,expected);
   }
   foreach(string name in oldHelpers)File.Delete(Path.Combine(plugins,name));
   Check(Directory.GetFiles(plugins,"*Native*.dll",SearchOption.AllDirectories).Length==0,"Extracted native engine outside recursive Plugins scan");
   foreach(string name in histories)Check(File.ReadAllText(Path.Combine(data,"History",name))=="keep existing "+name,"History preserved: "+name);
   Same(ini,chosen);
  }else if(args[0]=="logs"){
   CheckPrivateLogging(action);
  }else if(args[0]=="extract"){
   Call(action,"Extract","GamedayAtmosphereNative-"+runtime+".dll",false);Call(action,"Extract","DgaDiagnostics.exe",false);
  }else throw new Exception("Unknown case");
  Console.WriteLine("PASS "+args[0]+" "+count);return 0;
 }catch(Exception error){Console.Error.WriteLine(error);return 1;}}
 [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
 static void CheckPrivateLogging(Type action){
  var logger=new CapturedGdaLogger();string folder=(string)action.GetProperty("LogsFolder").GetValue(null,null);Directory.CreateDirectory(folder);
  string path=Path.Combine(folder,"launcher.log");
  Call(action,"Note",logger,"Private-only state check");
  Check(logger.Calls==0&&File.ReadAllText(path).Contains("Private-only state check"),"GDA writes AppData diagnostics without host messages");
  using(var locked=new FileStream(path,FileMode.Open,FileAccess.Read,FileShare.Read))Call(action,"Note",logger,"Locked private log check");
  Check(logger.Calls==0,"Even log failure does not hijack MMC progress or errors");
 }
}
sealed class CapturedGdaLogger:FrostySdk.Interfaces.ILogger {
 public int Calls;public void Log(string text,params object[] args){++Calls;}public void LogWarning(string text,params object[] args){++Calls;}public void LogError(string text,params object[] args){++Calls;}
}
