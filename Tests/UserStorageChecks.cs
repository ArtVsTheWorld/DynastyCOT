using System;using System.IO;using System.Linq;using System.Security.AccessControl;using System.Security.Principal;using System.Collections.Generic;using Ace.DynastyCOT;
class UserStorageChecks {
 static int checks;static void Check(bool yes,string text){if(!yes)throw new Exception(text);++checks;}
 static string Put(string root,string relative,string text){string p=Path.Combine(root,relative);Directory.CreateDirectory(Path.GetDirectoryName(p));File.WriteAllText(p,text);return p;}
 static int Main(string[] args){try{
  string parent=Path.GetFullPath(args[0]),fixture=Path.GetFullPath(args[1]);Check(fixture.StartsWith(parent.TrimEnd('\\')+"\\",StringComparison.OrdinalIgnoreCase)&&File.Exists(Path.Combine(fixture,".cot-fixture")),"Marked isolated fixture required");
  string mmc=Path.Combine(fixture,"Program Files","MMC with spaces"),plugins=Path.Combine(mmc,"Plugins"),profile=Path.Combine(fixture,"LocalAppData","Ace","DynastyCOT");Directory.CreateDirectory(plugins);var store=new UserStore(profile);
  string ini=Put(plugins,@"DynamicGamedayAtmosphere\DynamicGamedayAtmosphere.ini","[General]\r\nConfigVersion=999\r\nSecretChoice=42\r\n");
  Put(plugins,@"DynamicGamedayAtmosphere\History\visitor-plans-v3.txt","precious crowd history");
  Put(plugins,@"OfflineDynastySettings\DynastyClocks.json","{\"format\":1,\"clocks\":{\"123\":{\"offense\":1,\"defense\":25}}}");
  Put(plugins,@"OfflinePlaycallSettings\dynasties.json","per-league playcall choices");
  Put(plugins,@"SituationalPractice\launcher.log","old practice report");
  string rt=Path.Combine(mmc,"COT-Runtime","DynastyEnhancer");
  Put(rt,@"RuntimeHistory\season-2026\published.json","unaltered published wording and account IDs");
  Put(rt,@"DynastyEnhancementSettings\123.json","unaltered dynasty settings");
  Put(rt,@"RecruitResumeHistory\123.json","recruit scouting history");
  Put(rt,"beyond-the-field.json","{\"node\":\"../Shared/node.exe\",\"exporter\":\"Tools/export_runtime.mjs\"}");
  Put(rt,"BeyondTheFieldNative-v103.dll","old code must not migrate");Put(rt,@"Content\Runtime\pack.json","old content must not migrate");
  Put(plugins,@"SituationalPractice\runtime.stop","never replay old control commands");
  var before=Directory.GetFiles(mmc,"*",SearchOption.AllDirectories).ToDictionary(x=>x,UserStore.HashFile);
  var acl=Directory.GetAccessControl(mmc,AccessControlSections.Access);var denied=new DirectorySecurity();denied.SetSecurityDescriptorBinaryForm(acl.GetSecurityDescriptorBinaryForm(),AccessControlSections.Access);
  var rule=new FileSystemAccessRule(WindowsIdentity.GetCurrent().User,FileSystemRights.Write|FileSystemRights.Delete|FileSystemRights.DeleteSubdirectoriesAndFiles,InheritanceFlags.ContainerInherit|InheritanceFlags.ObjectInherit,PropagationFlags.None,AccessControlType.Deny);denied.AddAccessRule(rule);
  Directory.SetAccessControl(mmc,denied);
  try{
   bool refused=false;try{File.WriteAllText(Path.Combine(mmc,"permission-probe"),"must fail");}catch(UnauthorizedAccessException){refused=true;}Check(refused,"Fixture really denies installation writes");
   foreach(string id in new[]{"GamedayAtmosphere","DynastyEnhancer","ExtraDynastySettings","DrivePractice"})store.Migrate(id,plugins);
   Check(File.ReadAllBytes(ini).SequenceEqual(File.ReadAllBytes(Path.Combine(store.Folder("GamedayAtmosphere"),Path.GetFileName(ini)))),"Exact future-version settings preserved");
   Check(File.ReadAllText(store.Resolve("BeyondTheField/RuntimeHistory/season-2026/published.json"))=="unaltered published wording and account IDs","Published history/identities intact");
   Check(File.ReadAllText(store.Resolve("BeyondTheField/DynastyEnhancementSettings/123.json"))=="unaltered dynasty settings","Dynasty options intact");
   Check(File.ReadAllText(store.Resolve("ExtraSettings/OfflinePlaycallSettings/dynasties.json"))=="per-league playcall choices","Separate legacy playcall store migrated");
   Check(!File.Exists(store.Resolve("BeyondTheField/BeyondTheFieldNative-v103.dll"))&&!File.Exists(store.Resolve("BeyondTheField/Content/Runtime/pack.json")),"Never import legacy code/content as a trusted payload");
   Check(!File.Exists(store.Resolve("ExpandedPractice/runtime.stop")),"Old commands not replayed");
   foreach(var item in before)Check(File.Exists(item.Key)&&UserStore.HashFile(item.Key)==item.Value,"Original retained: "+item.Key);
   Check(Directory.GetFiles(mmc,"*",SearchOption.AllDirectories).Length==before.Count,"No installation writes/extraneous files");
   string data=store.Resolve("GamedayAtmosphere/DynamicGamedayAtmosphere.ini");File.WriteAllText(data,"new user choice");store.Migrate("GamedayAtmosphere",plugins);Check(File.ReadAllText(data)=="new user choice","Later launches never restore stale legacy defaults");
  }finally{Directory.SetAccessControl(mmc,acl);}
  string second=Path.Combine(fixture,"Downloads","MMC2","Plugins");Put(second,@"DynamicGamedayAtmosphere\DynamicGamedayAtmosphere.ini","other MMC config");Put(second,@"DynamicGamedayAtmosphere\History\another-fixture.txt","new nonconflicting history");
  store.Migrate("GamedayAtmosphere",second);Check(File.ReadAllText(store.Resolve("GamedayAtmosphere/DynamicGamedayAtmosphere.ini"))=="new user choice","All MMC copies share canonical existing choices");
  Check(Directory.GetFiles(store.Resolve("Migration/GamedayAtmosphere/Conflicts"),"*",SearchOption.AllDirectories).Any(x=>File.ReadAllText(x)=="other MMC config"),"Conflicting source archived for review");
  Check(File.Exists(store.Resolve("GamedayAtmosphere/History/another-fixture.txt")),"Nonconflicting old history imported");
  foreach(string bad in new[]{"../outside",@"C:\outside","/outside","foo:bar","foo//bar","foo/./bar","foo/../bar"}){bool rejected=false;try{store.Resolve(bad);}catch(IOException){rejected=true;}Check(rejected,"Unsafe path rejected: "+bad);}
  Check(store.Payload("DynastyEnhancer","COT-Runtime/DynastyEnhancer/Tools/export_runtime.mjs")==store.Resolve("BeyondTheField/Tools/export_runtime.mjs"),"Old resource IDs resolve into branded user folder");
  Check(store.Payload("DrivePractice","Plugins/SituationalPractice/ExpandedPractice.Runtime.exe")==store.Resolve("ExpandedPractice/ExpandedPractice.Runtime.exe"),"Practice helper lives outside Plugins");
  Check(store.Payload("ExtraSettings","Plugins/OfflineDynastySettings/ExtraSettings.Persistence.exe")==store.Resolve("ExtraSettings/ExtraSettings.Persistence.exe"),"Settings helper lives outside Plugins");
  string v1=store.Payload("DynastyEnhancer","COT-Runtime/DynastyEnhancer/Tools/export_runtime.mjs","1.0.8"),v2=store.Payload("DynastyEnhancer","COT-Runtime/DynastyEnhancer/Tools/export_runtime.mjs","1.0.9");
  Check(v1!=v2,"Different plugin versions cannot replace each other's runtime");UserStore.Atomic(v1,new byte[]{1});UserStore.Atomic(v2,new byte[]{2});Check(File.ReadAllBytes(v1)[0]==1,"Version coexistence retains old runtime");
  Check(store.Payload("DynastyEnhancer","COT-Runtime/DynastyEnhancer/beyond-the-field.json","1.0.8")==store.Payload("DynastyEnhancer","COT-Runtime/DynastyEnhancer/beyond-the-field.json","1.0.9"),"User config is shared across versions");
  Check(!store.NeedsMigration("GamedayAtmosphere",plugins),"Completed migration is not repeated");
  string retry=Path.Combine(fixture,"RetryMMC","Plugins");string locked=Put(retry,@"OfflineDynastySettings\DynastyClocks.json","retry-preserves-exact-data");
  using(var hold=new FileStream(locked,FileMode.Open,FileAccess.ReadWrite,FileShare.None)){bool failed=false;try{store.Migrate("ExtraSettings",retry);}catch(IOException){failed=true;}Check(failed&&store.NeedsMigration("ExtraSettings",retry),"Failed copy never marks migration complete");}
  store.Migrate("ExtraSettings",retry);Check(!store.NeedsMigration("ExtraSettings",retry),"Unlocked source resumes migration without loss");Check(File.ReadAllText(locked)=="retry-preserves-exact-data","Retry retains original");
  string c1=Path.Combine(fixture,"Collections","One"),c2=Path.Combine(fixture,"Collections","Two");Put(c1,@"COT-Collection\installed.json","receipt-one");Put(c1,@"COT-Collection\Backups\first\original.dll","rollback-byte-stream");store.MigrateCollection(c1);
  Check(File.ReadAllText(Path.Combine(store.CollectionFolder(c1),"installed.json"))=="receipt-one","Installer receipt moved to user store");Check(File.ReadAllText(Path.Combine(store.CollectionFolder(c1),"Backups","first","original.dll"))=="rollback-byte-stream","Old transactional backups retained");Check(store.CollectionFolder(c1)!=store.CollectionFolder(c2),"Installer inventories isolated across MMC copies");
  bool relativeRefused=false;try{new UserStore("relative");}catch(IOException){relativeRefused=true;}Check(relativeRefused,"User root must be absolute");
  Console.WriteLine("PASS "+checks+" storage checks; real installation-write denial, migration, conflicts, shared profiles, user-data preservation and path safety; no live install/game access");return 0;
 }catch(Exception e){Console.Error.WriteLine(e);return 1;}}
}
