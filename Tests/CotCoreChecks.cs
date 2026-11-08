using System;
using System.Collections.Generic;
using System.IO;
using DynastyCOT;
class CotCoreChecks {
 static int count;
 static void Check(bool value,string name){if(!value)throw new Exception(name);count++;}
 static void Reject(Action action,string name){try{action();}catch(InvalidDataException){count++;return;}catch(IOException){count++;return;}throw new Exception("Accepted "+name);}
 static int Main(){
  foreach(int mask in new[]{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}){
   string[] entries=new string[4];var expected=new HashSet<string>();
   for(int i=0;i<4;i++){bool enabled=(mask&(1<<i))!=0;entries[i]="\""+Configuration.ModuleIds[i]+"\":"+(enabled?"true":"false");if(enabled)expected.Add(Configuration.ModuleIds[i]);}
   var config=Configuration.Parse("{\"format\":1,\"modules\":{"+String.Join(",",entries)+"}}");
   var verified=new HashSet<string>();var started=new HashSet<string>();
   var result=ModulePlanner.StartSelected(config,id=>verified.Add(id),id=>started.Add(id));
   Check(verified.SetEquals(expected),"disabled module verified");Check(started.SetEquals(expected),"disabled module instantiated");
   foreach(string id in Configuration.ModuleIds)Check(result[id]==(expected.Contains(id)?"startup_requested":"disabled"),"module result");
  }
  var statuses=ModulePlanner.StartSelected(new Configuration(),id=>{if(id=="DynastyEnhancer")throw new IOException("tamper");},id=>{});
  Check(statuses["DynastyEnhancer"].StartsWith("unavailable"),"failure hidden");
  Check(statuses["DrivePractice"]=="startup_requested","one module failure blocked others");
  Reject(()=>Configuration.Parse("{\"format\":2,\"modules\":{}}"),"future config");
  Reject(()=>Configuration.Parse("{\"format\":1,\"modules\":{\"DrivePractice\":1}}"),"numeric switch");
  Reject(()=>Configuration.Parse("{\"format\":1,\"modules\":{\"Unknown\":true}}"),"unknown module");
  foreach(string path in new[]{"../escape","/escape","C:/escape","a/../../escape"})Reject(()=>CotPaths.Resolve(Path.GetTempPath(),path),"path "+path);
  Check(CotPaths.Resolve(Path.GetTempPath(),"owned/file").StartsWith(Path.GetFullPath(Path.GetTempPath())),"owned path");
  Console.WriteLine("PASS: "+count+" COT configuration/planner/path checks, including all 16 selections. No component engines started.");return 0;
 }
}
