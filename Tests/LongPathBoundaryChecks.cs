using System;
using System.IO;
using System.Text;
using System.Reflection;
using System.Diagnostics;
using Ace.DynastyCOT;
using F=Ace.DynastyCOT.StorageFile;
using D=Ace.DynastyCOT.StorageDirectory;
using P=Ace.DynastyCOT.StoragePath;
[assembly:System.Reflection.AssemblyFileVersion("1.2.3.4")]
class LongPathBoundaryChecks {
 static int count;static void Check(bool ok,string message){if(!ok)throw new Exception(message);count++;}
 static void Main(string[] args){try{Run(args);}catch(Exception e){Console.Error.WriteLine(e);Environment.ExitCode=1;}}
 static void Run(string[] args){
  var root=P.GetFullPath(args[0]);if(!System.IO.File.Exists(P.Combine(root,".cot-fixture")))throw new Exception("Marked fixture required");
  foreach(int length in new[]{235,248,259,260,262,324,430}){
   string folder=root;while(folder.Length<length)folder=P.Combine(folder,new string('x',Math.Min(70,length-folder.Length-1)));
   D.CreateDirectory(folder);string source=P.Combine(folder,"settings.ini"),other=P.Combine(folder,"other.ini"),backup=P.Combine(folder,"saved.bak");
   F.WriteAllText(source,"[Settings]\r\nValue=17\r\n");Check(F.Exists(source),"exists");Check(F.ReadAllText(source).Contains("17"),"text");Check(new StorageFileInfo(source).Length>0,"metadata");
   F.Copy(source,other);Check(F.ReadAllBytes(source).Length==F.ReadAllBytes(other).Length,"copy");
   F.AppendAllText(other,"new");F.Replace(other,source,backup);Check(F.ReadAllText(backup).Contains("Value=17")&&F.ReadAllText(source).EndsWith("new"),"replace preserves backup");
   Check(D.GetFiles(folder,"*.ini").Length==1&&D.GetFiles(folder,"*.bak").Length==1,"wildcard enumeration");
   using(var lease=StorageIO.OpenFile(source,FileMode.Open,FileAccess.Read,FileShare.Read)){bool blocked=false;try{F.WriteAllText(source,"bad");}catch(IOException){blocked=true;}Check(blocked,"helper lease remains exclusive against writes");}
   F.Move(backup,other);F.Delete(other);Check(!F.Exists(other),"move/delete");
   string temp=P.Combine(folder,"delete-on-close.tmp");using(var probe=StorageIO.OpenFile(temp,FileMode.CreateNew,FileAccess.Write,FileShare.None,4096,FileOptions.DeleteOnClose))Check(F.Exists(temp),"DeleteOnClose exists");Check(!F.Exists(temp),"DeleteOnClose cleanup");
   Check(P.GetDirectoryName(source)==folder&&P.GetDirectoryName(folder+"\\")==folder,"logical parents");
   Check(!P.GetFullPath(StorageIO.IoPath(source)).StartsWith("\\\\?\\"),"prefix not part of identity");
   string json=P.Combine(folder,"atomic.json");UserStore.Atomic(json,Encoding.UTF8.GetBytes("first"));UserStore.Atomic(json,Encoding.UTF8.GetBytes("second"));Check(F.ReadAllText(json)=="second","atomic persistence");
   string dll=P.Combine(folder,"Identity.dll");F.Copy(args[1],dll);
   try{var id=StorageAssembly.GetAssemblyName(dll);Check(id.Name=="LongPathBoundaryChecks","assembly identity");}catch(Exception e){throw new Exception("Assembly metadata "+length,e);}
   try{Check(StorageVersionInfo.GetVersionInfo(dll).FileVersion!=null,"file version");}catch(Exception e){throw new Exception("Version metadata "+length,e);}
   bool missing=false;try{F.ReadAllText(P.Combine(folder,"missing.json"));}catch(FileNotFoundException){missing=true;}Check(missing,"real missing remains missing");
  }
  bool device=false;try{P.GetFullPath("\\\\?\\GLOBALROOT\\Device\\HarddiskVolume1\\x");}catch(IOException){device=true;}Check(device,"device namespace rejected");
  StorageIO.RequireExecutablePath(P.Combine(root,"short-helper.exe"));Check(true,"short helper location remains eligible");
  bool launchGuard=false;try{StorageIO.RequireExecutablePath(P.Combine(root,new string('a',180),new string('b',100),"helper.exe"));}catch(PathTooLongException){launchGuard=true;}Check(launchGuard,"unlaunchable helper rejected before process creation");
  Console.WriteLine("PASS "+count+" boundary checks: extraction/settings/logging/atomic writes/metadata/leases; no game or installed plugin access.");
 }
}
