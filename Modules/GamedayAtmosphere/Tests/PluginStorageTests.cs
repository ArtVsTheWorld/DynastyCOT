using System;using System.IO;using System.Linq;
using DynamicGamedayAtmosphere;
class PluginStorageTests {
 static void Check(bool okay){if(!okay)throw new Exception("Storage migration regression");}
 static void ResetCase(string root,string name,string relative,string old,string defaults){
  string plugins=Path.Combine(root,name);Directory.CreateDirectory(plugins);
  string original=Path.Combine(plugins,relative);Directory.CreateDirectory(Path.GetDirectoryName(original));
  File.WriteAllText(original,old);byte[] bytes=File.ReadAllBytes(original);
  Check(PluginStorage.NeedsReset(plugins,defaults));
  string saved=PluginStorage.Ensure(plugins,defaults);Check(saved!=null&&File.ReadAllBytes(saved).SequenceEqual(bytes));
  string ini=PluginStorage.SettingsFile(plugins),text=File.ReadAllText(ini);
  Check(text.Contains("SettingsResetVersion=4")&&text.Contains("CannonShakeStrengthPercent=18")&&!text.Contains("Obsolete=99")&&!text.Contains("CannonShakeStrengthPercent=29"));
  Check(!PluginStorage.NeedsReset(plugins,defaults));
  // User edits after the reset survive launch and a routine newer-default update.
  text=text.Replace("CannonShakeStrengthPercent=18","CannonShakeStrengthPercent=7.5");File.WriteAllText(ini,text);
  int count=Directory.GetFiles(Path.Combine(PluginStorage.DataFolder(plugins),"Backups")).Length;
  Check(PluginStorage.Ensure(plugins,defaults)==null&&File.ReadAllText(ini)==text);
  Check(PluginStorage.Ensure(plugins,defaults.Replace("CannonShakeStrengthPercent=18","CannonShakeStrengthPercent=21"))==null&&File.ReadAllText(ini)==text);
  Check(Directory.GetFiles(Path.Combine(PluginStorage.DataFolder(plugins),"Backups")).Length==count);
 }
 static int Main(){
  string root=Path.Combine(Path.GetTempPath(),"DGA-storage-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(root);
  string defaults=File.ReadAllText(Path.Combine(AppDomain.CurrentDomain.BaseDirectory,"..","..","Source","DynamicGamedayAtmosphere.ini"));
  try{
   string modern=Path.Combine(root,"DynamicGamedayAtmosphere.ini"),legacy=Path.Combine(root,"DynamicAttendance.ini"),nested=PluginStorage.SettingsFile(root);
   File.WriteAllText(modern,"[General]\nConfigVersion=2\nSettingsResetVersion=4\nAudioBalanceVersion=39\n[Dynamic Gameday Attendance]\nStartPercent=47\n[Custom]\nMyValue=keep\n");
   File.WriteAllText(legacy,"old legacy content");File.WriteAllText(Path.Combine(root,"AnotherPlugin.ini"),"untouched");
   File.WriteAllText(Path.Combine(root,"DynamicGamedayAtmosphere.log"),"old log");File.WriteAllText(Path.Combine(root,"DynamicGamedayAtmosphere-control.txt"),"empty");
   PluginStorage.Ensure(root,defaults);
   string text=File.ReadAllText(nested);Check(text.Contains("StartPercent=47")&&text.Contains("MyValue=keep")&&!File.Exists(modern)&&!File.Exists(legacy));
   string backups=Path.Combine(PluginStorage.DataFolder(root),"Backups");Check(Directory.GetFiles(backups).Length==4&&File.ReadAllText(Path.Combine(root,"AnotherPlugin.ini"))=="untouched");
   PluginStorage.Ensure(root,defaults);Check(File.ReadAllText(nested)==text&&Directory.GetFiles(backups).Length==4);
   File.WriteAllText(modern,"[Custom]\nMyValue=stale\n");PluginStorage.Ensure(root,defaults);Check(File.ReadAllText(nested).Contains("MyValue=keep")&&Directory.GetFiles(backups).Length==5);
   File.WriteAllText(modern+".previous","previous settings");File.WriteAllText(modern+".tmp","interrupted migration");PluginStorage.Ensure(root,defaults);Check(Directory.GetFiles(backups).Length==7&&!File.Exists(modern+".previous")&&!File.Exists(modern+".tmp"));
   string future=Path.Combine(root,"Future");Directory.CreateDirectory(future);File.WriteAllText(Path.Combine(future,"DynamicGamedayAtmosphere.ini"),"[General]\nConfigVersion=999\nKeepExactly=yes\n");
   PluginStorage.Ensure(future,defaults);Check(File.ReadAllText(PluginStorage.SettingsFile(future)).Contains("ConfigVersion=999")&&!File.Exists(Path.Combine(future,"DynamicGamedayAtmosphere.ini")));
   string failure=Path.Combine(root,"Failure");Directory.CreateDirectory(failure);string old=Path.Combine(failure,"DynamicGamedayAtmosphere.ini");File.WriteAllText(old,"precious old settings");File.WriteAllText(PluginStorage.DataFolder(failure),"not a directory");
   bool failed=false;try{PluginStorage.Ensure(failure,defaults);}catch(IOException){failed=true;}Check(failed&&File.ReadAllText(old)=="precious old settings");
   string stale="[General]\nConfigVersion=2\nAudioBalanceVersion=42\n[Dynamic Gameday Broadcast]\nCannonShakeStrengthPercent=29\n[Custom]\nObsolete=99\n";
   ResetCase(root,"Update2","DynamicGamedayAtmosphere.ini",stale,defaults);
   ResetCase(root,"Legacy","DynamicAttendance.ini","[Attendance]\nEnabled=0\n[Custom]\nObsolete=99\n",defaults);
   ResetCase(root,"WorkingBuild",Path.Combine("DynamicGamedayAtmosphere","DynamicGamedayAtmosphere.ini"),stale,defaults);
   ResetCase(root,"PreReleaseMarker3",Path.Combine("DynamicGamedayAtmosphere","DynamicGamedayAtmosphere.ini"),"[General]\nConfigVersion=2\nSettingsResetVersion=3\nAudioBalanceVersion=50\n[Dynamic Gameday Broadcast]\nCannonShakeStrengthPercent=29\n",defaults);
   ResetCase(root,"Malformed",Path.Combine("DynamicGamedayAtmosphere","DynamicGamedayAtmosphere.ini"),"Obsolete=99\n",defaults);
   ResetCase(root,"BadMarker",Path.Combine("DynamicGamedayAtmosphere","DynamicGamedayAtmosphere.ini"),"[General]\nSettingsResetVersion=invalid\nObsolete=99\n",defaults);
   string fresh=Path.Combine(root,"Fresh");Check(!PluginStorage.NeedsReset(fresh,defaults)&&PluginStorage.Ensure(fresh,defaults)==null);
   Check(File.ReadAllText(PluginStorage.SettingsFile(fresh)).Contains("SettingsResetVersion=4")&&!Directory.Exists(Path.Combine(PluginStorage.DataFolder(fresh),"Backups")));
   // Do not let a stale loose copy overwrite an already-reset nested INI.
   File.WriteAllText(Path.Combine(fresh,"DynamicGamedayAtmosphere.ini"),stale);
   string current=File.ReadAllText(PluginStorage.SettingsFile(fresh));Check(!PluginStorage.NeedsReset(fresh,defaults));
   Check(PluginStorage.Ensure(fresh,defaults)==null&&File.ReadAllText(PluginStorage.SettingsFile(fresh))==current);
   string newer=Path.Combine(root,"NewerReset");Directory.CreateDirectory(PluginStorage.DataFolder(newer));
   File.WriteAllText(PluginStorage.SettingsFile(newer),"[General]\nConfigVersion=2\nSettingsResetVersion=5\n[Custom]\nObsolete=99\n");
   Check(!PluginStorage.NeedsReset(newer,defaults)&&PluginStorage.Ensure(newer,defaults)==null&&File.ReadAllText(PluginStorage.SettingsFile(newer)).Contains("SettingsResetVersion=5"));
   string helpers=Path.Combine(root,"LooseHelpers");Directory.CreateDirectory(helpers);
   string[] owned={"LoadDynamicGamedayAtmosphere.exe","LoadDynamicAttendance.exe","DGADIAGNOSTICS.EXE","DynamicGamedayAtmosphereNative-2.0.10.dll","DynamicAttendanceNative-1.2.3.dll"};
   foreach(string name in owned)File.WriteAllText(Path.Combine(helpers,name),name);
   foreach(string name in new[]{"DynamicGamedayAtmosphere.dll","AnotherPlugin.dll","OtherTool.exe","DynamicGamedayAtmosphereNative-user.dll"})File.WriteAllText(Path.Combine(helpers,name),"untouched");
   Check(PluginStorage.NeedsRelocation(helpers));PluginStorage.Ensure(helpers,defaults);Check(!PluginStorage.NeedsRelocation(helpers));
   foreach(string name in owned)Check(!File.Exists(Path.Combine(helpers,name)));
   foreach(string name in new[]{"DynamicGamedayAtmosphere.dll","AnotherPlugin.dll","OtherTool.exe","DynamicGamedayAtmosphereNative-user.dll"})Check(File.ReadAllText(Path.Combine(helpers,name))=="untouched");
   Check(Directory.GetFiles(Path.Combine(PluginStorage.DataFolder(helpers),"Backups")).Length==owned.Length);
   string blocked=Path.Combine(root,"Locked");Directory.CreateDirectory(PluginStorage.DataFolder(blocked));string blockedIni=PluginStorage.SettingsFile(blocked);File.WriteAllText(blockedIni,stale);
   using(var held=new FileStream(blockedIni,FileMode.Open,FileAccess.Read,FileShare.Read)){
    failed=false;try{PluginStorage.Ensure(blocked,defaults);}catch(IOException){failed=true;}
    Check(failed&&File.ReadAllText(blockedIni)==stale&&PluginStorage.NeedsReset(blocked,defaults));
   }
   Check(Directory.GetFiles(PluginStorage.DataFolder(blocked),"*.tmp").Length==0);
   Check(PluginStorage.Ensure(blocked,defaults)!=null&&!PluginStorage.NeedsReset(blocked,defaults));
   string noBackup=Path.Combine(root,"BlockedBackup");Directory.CreateDirectory(PluginStorage.DataFolder(noBackup));string noBackupIni=PluginStorage.SettingsFile(noBackup);File.WriteAllText(noBackupIni,stale);File.WriteAllText(Path.Combine(PluginStorage.DataFolder(noBackup),"Backups"),"blocked");
   failed=false;try{PluginStorage.Ensure(noBackup,defaults);}catch(IOException){failed=true;}Check(failed&&File.ReadAllText(noBackupIni)==stale);
   foreach(int length in new[]{181,190}){
    string prefix=Path.Combine(root,"Long"+length);int padding=length-PluginStorage.SettingsFile(prefix).Length-1;
    Check(padding>0);string longPlugins=Path.Combine(prefix,new string('p',padding));Directory.CreateDirectory(longPlugins);
    string longIni=PluginStorage.SettingsFile(longPlugins);Check(longIni.Length==length);
    File.WriteAllText(Path.Combine(longPlugins,"DynamicGamedayAtmosphere.ini"),stale);
    byte[] before=File.ReadAllBytes(Path.Combine(longPlugins,"DynamicGamedayAtmosphere.ini"));
    foreach(string helper in owned)File.WriteAllText(Path.Combine(longPlugins,helper),helper);
    string oldBackup=Path.Combine(PluginStorage.DataFolder(longPlugins),"Backups","existing-user-backup.bak");Directory.CreateDirectory(Path.GetDirectoryName(oldBackup));File.WriteAllText(oldBackup,"keep backup");
    string saved=PluginStorage.Ensure(longPlugins,defaults);Check(saved!=null&&saved.Length<260&&File.ReadAllBytes(saved).SequenceEqual(before));
    Check(File.ReadAllText(oldBackup)=="keep backup"&&!PluginStorage.NeedsRelocation(longPlugins));
    Check(Directory.GetFiles(PluginStorage.DataFolder(longPlugins),"*.tmp").Length==0);
    string active=File.ReadAllText(longIni);Check(PluginStorage.Ensure(longPlugins,defaults)==null&&File.ReadAllText(longIni)==active);
   }
   string tooLong=new string('x',260);failed=false;
   try{SettingsIo.Run("test long-path validation",null,tooLong,null,()=>{throw new Exception("Must validate before mutation");});}
   catch(PathTooLongException error){failed=true;Check((string)error.Data["DgaSettingsOperation"]=="test long-path validation"&&(string)error.Data["DgaSettingsDestination"]==tooLong&&error.Data.Contains("DgaSettingsHint"));}Check(failed);
   // A failed cleanup must not hide the operation that originally failed.
   string cleanupFile=Path.Combine(root,"cleanup.tmp");File.WriteAllText(cleanupFile,"temporary");var primary=new IOException("original replacement failed");
   using(var held=new FileStream(cleanupFile,FileMode.Open,FileAccess.Read,FileShare.Read)){SettingsIo.Cleanup(cleanupFile,primary);Check(primary.Message=="original replacement failed"&&primary.Data.Contains("DgaSettingsCleanup"));}
   SettingsIo.Cleanup(cleanupFile,null);Check(!File.Exists(cleanupFile));
   Console.WriteLine("PASS: settings reset/migration and exact backups; 181/190-character INI paths, helper retirement, existing-backup preservation, idempotence, path diagnostics, cleanup-error preservation and unrelated-file isolation.");return 0;
  }catch(Exception e){Console.Error.WriteLine(e);return 1;}
 }
}
