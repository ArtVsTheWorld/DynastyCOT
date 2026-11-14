using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using OfflineDynastySettings;
// Pass an isolated scratch directory. Never opens a game process or writes live settings.
class SnapshotChecks {
 static void Main(string[] args){
  var root=Path.GetFullPath(args[0]);
  typeof(Ace.DynastyCOT.UserStorage).GetField("current",BindingFlags.Static|BindingFlags.NonPublic).SetValue(null,new Ace.DynastyCOT.UserStore(root));
  var c=new PersistenceContext {Pid=433,ImageBase=0x100000000,ClockData=0x10000,SplitData=0x20000,PluginFolder=root};
  var dir=Path.Combine(Ace.DynastyCOT.UserStorage.Folder("ExtraSettings"),"Diagnostics"); Directory.CreateDirectory(dir);
  string path=Path.Combine(dir,"runtime.json");int mode=1;
  Func<long,int,byte[]> read=(address,count)=>address==c.ImageBase+0xd9eb808?BitConverter.GetBytes(mode):address==c.ImageBase+0xf3fddbc?BitConverter.GetBytes(3):new byte[count];
  DiagnosticLog.Runtime(c,read); string initial=File.ReadAllText(path);
  int changes=0;var seconds=Stopwatch.StartNew();
  for(int i=0;i<128;i++){File.SetLastWriteTimeUtc(path,new DateTime(2000,1,1));DiagnosticLog.Runtime(c,read);if(File.GetLastWriteTimeUtc(path).Year!=2000)++changes;}
  seconds.Stop();
  {
   if(changes!=0||!initial.Contains("inactive outside"))throw new Exception("Repeated or false match activation");
   mode=14;DiagnosticLog.Runtime(c,read);string active=File.ReadAllText(path);
   if(!active.Contains("Timer counters")||active==initial)throw new Exception("Changed context not persisted");
   mode=5;using(var locked=File.Open(path,FileMode.Open,FileAccess.ReadWrite,FileShare.None))DiagnosticLog.Runtime(c,read);
   DiagnosticLog.Runtime(c,read);if(!File.ReadAllText(path).Contains("\"mode\":5"))throw new Exception("Locked report not retried");
  }
  Console.WriteLine("Extra snapshot comparison (128 identical polls): files rewritten="+changes+" elapsed ms="+seconds.Elapsed.TotalMilliseconds);
 }
}
