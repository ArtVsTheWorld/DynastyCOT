using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
class ClockHelperChecks {
 static int checks;static void Check(bool ok,string text){if(!ok)throw new Exception(text);checks++;}
 static int Main(string[] args){
  IntPtr data=Marshal.AllocHGlobal(16384);
  try{
   Marshal.Copy(new byte[16384],0,data,16384);
   string folder=Path.Combine(Path.GetFullPath(args[1]),"Helper-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(folder);
   Assembly a=Assembly.LoadFrom(Path.GetFullPath(args[0]));
   Type h=a.GetType("OfflineDynastySettings.PersistenceHelper",true),c=a.GetType("OfflineDynastySettings.PersistenceContext",true);
   object context=Activator.CreateInstance(c);
   c.GetProperty("ClockData").SetValue(context,data.ToInt64(),null);c.GetProperty("PluginFolder").SetValue(context,folder,null);
   h.GetField("context",BindingFlags.Static|BindingFlags.NonPublic).SetValue(null,context);
   MethodInfo save=h.GetMethod("SaveDynastyClocks",BindingFlags.Static|BindingFlags.NonPublic);
   object[] call={GetCurrentProcess(),(uint)0};
   string path=Path.Combine(folder,"OfflineDynastySettings","DynastyClocks.json");
   save.Invoke(null,call);Check(!File.Exists(path),"unchanged revision wrote defaults");
   Marshal.WriteInt64(data,0x1100,unchecked((long)(101UL|(3UL<<32)|(5UL<<40))));
   Marshal.WriteInt64(data,0x1108,unchecked((long)(202UL|(2UL<<32))));
   Marshal.WriteInt32(data,0x1000,1);save.Invoke(null,call);Check(!File.Exists(path)&&(uint)call[1]==0,"odd revision persisted");
   Marshal.WriteInt32(data,0x1000,2);save.Invoke(null,call);Check(File.Exists(path)&&(uint)call[1]==2,"stable pair not acknowledged");
   string first=File.ReadAllText(path);Check(first.Contains("\"101\"")&&first.Contains("\"202\"")&&first.Contains("\"defensiveSeconds\":5"),"two Dynasty profiles not saved");
   Marshal.WriteInt64(data,0x1100,unchecked((long)(101UL|(1UL<<32)|(60UL<<40))));Marshal.WriteInt32(data,0x1000,4);
   bool rejected=false;
   using(var locked=new FileStream(path,FileMode.Open,FileAccess.ReadWrite,FileShare.None)){
    try{save.Invoke(null,call);}catch(TargetInvocationException e){rejected=e.InnerException is IOException;}
    Check(rejected&&(uint)call[1]==2,"save failure acknowledged revision");
   }
   Check(File.ReadAllText(path)==first,"failed save changed previous file");
   save.Invoke(null,call);Check((uint)call[1]==4&&File.ReadAllText(path).Contains("\"defensiveSeconds\":60"),"retry did not save pending pair");
   Check(File.Exists(path+".bak")&&File.ReadAllText(path+".bak")==first,"atomic backup differs");
   string second=File.ReadAllText(path);Marshal.WriteInt64(data,0x1108,unchecked((long)(101UL|(2UL<<32))));Marshal.WriteInt32(data,0x1000,6);
   rejected=false;try{save.Invoke(null,call);}catch(TargetInvocationException e){rejected=e.InnerException is InvalidDataException;}
   Check(rejected&&(uint)call[1]==4&&File.ReadAllText(path)==second,"invalid table changed storage or acknowledgement");
   Check(!File.Exists(Path.Combine(folder,"DynastyPlayClock.ini"))&&!File.Exists(Path.Combine(folder,"DefensivePlayClock.ini")),"helper wrote old global clocks");
   Console.WriteLine("PASS: "+checks+" detached clock-helper checks; stable snapshots, failed-save retry, backups and invalid-table rejection. Self-process only.");return 0;
  }catch(Exception e){Console.Error.WriteLine(e);return 1;}finally{Marshal.FreeHGlobal(data);}
 }
 [DllImport("kernel32.dll")]static extern IntPtr GetCurrentProcess();
}
