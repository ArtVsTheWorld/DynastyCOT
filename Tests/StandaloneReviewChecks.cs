using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Security.AccessControl;
using System.Security.Principal;
using System.Windows.Forms;
using System.Web.Script.Serialization;
namespace DynastyCOT.Setup {
 internal static class StandaloneReviewChecks {
  static int checks;
  static void Check(bool ok,string message){if(!ok)throw new Exception(message);checks++;}
  static Dictionary<string,string> Snapshot(string root){return Directory.GetFiles(root,"*",SearchOption.AllDirectories).Where(p=>!p.Contains("\\Backups\\")).ToDictionary(p=>p.Substring(root.Length),p=>{using(var f=File.OpenRead(p))return InstallerEngine.Hash(f);});}
  static bool Same(Dictionary<string,string> a,Dictionary<string,string> b){return a.Count==b.Count&&a.All(x=>b.ContainsKey(x.Key)&&b[x.Key]==x.Value);}
  static Dictionary<string,bool> Modules(){return InstallerEngine.ModuleIds.ToDictionary(x=>x,x=>true);}
  [STAThread]static int Main(string[] args){try{
   var fixture=new Fixture(args[0],args[1]);string root=fixture.Root;var before=Snapshot(root);
   // Exercise the actual old released window's summary method, then the new one.
   string old="Unrecognized active legacy payload kept; no installation changes made: Plugins/DynamicGamedayAtmosphere.dll. Close MMC/game and remove or verify that standalone mod before retrying.";
   var oldForm=Assembly.LoadFile(Path.GetFullPath(args[2])).GetType("DynastyCOT.Setup.SetupForm");
   string oldSummary=(string)oldForm.GetMethod("FriendlyError",BindingFlags.Static|BindingFlags.NonPublic).Invoke(null,new object[]{old});
   Check(oldSummary.StartsWith("Close MMC"),"Original symptom was not reproduced");
   Check(SetupForm.FriendlyError(old).StartsWith("Older standalone"),"Legacy error still mislabeled");
   Check(SetupForm.FriendlyError("COT_RUNNING_PROCESSES: Close MMC and the game").StartsWith("Close MMC"),"True process error lost");
   Check(!SetupForm.FriendlyError("unrelated failure mentioning Close MMC").StartsWith("Close MMC"),"Broad substring detection remains");
   using(var engine=new InstallerEngine()){
    var blocked=engine.Run(root,Modules(),null,fixture,s=>{});
    Check(!blocked.Success&&blocked.Conflicts.Count==3,"Expected all three legacy conflicts");Check(Same(before,Snapshot(root)),"Unreviewed files changed");
    Check(blocked.Conflicts.All(x=>x.Path.StartsWith("Plugins/")&&x.Sha256.Length==64),"Conflict identity missing");
    // A changed DLL requires fresh review, and unrelated file paths are refused.
    string changed=Path.Combine(root,blocked.Conflicts[0].Path.Replace('/','\\'));byte[] original=File.ReadAllBytes(changed);File.AppendAllText(changed,"changed");var modified=Snapshot(root);
    var stale=engine.Run(root,Modules(),null,fixture,s=>{},"Install",null,blocked.Conflicts);
    Check(!stale.Success&&stale.Conflicts.Count==1&&Same(modified,Snapshot(root)),"Stale review was accepted or incorrectly rejected: "+stale.Details+"; conflicts="+(stale.Conflicts==null?-1:stale.Conflicts.Count)+"; same="+Same(modified,Snapshot(root)));File.WriteAllBytes(changed,original);
    var forged=new List<StandaloneConflict>{new StandaloneConflict{Path="Plugins/OtherOverhaul.dll",Module="GamedayAtmosphere",Sha256=new string('0',64)}};
    var bad=engine.Run(root,Modules(),null,fixture,s=>{},"Install",null,forged);Check(!bad.Success&&Same(before,Snapshot(root)),"Unrelated DLL approval accepted");
    // A file occupying the runtime directory is diagnosed before a transaction.
    string runtime=Path.Combine(root,"COT-Runtime");File.WriteAllText(runtime,"blocking file");var collision=Snapshot(root);
    var denied=engine.Run(root,Modules(),null,fixture,s=>{},"Install",null,blocked.Conflicts);
    Check(!denied.Success&&denied.Details.Contains("COT_WRITE_ACCESS:")&&denied.Details.Contains("COT-Runtime"),"Directory collision was not diagnosed");Check(Same(collision,Snapshot(root)),"Directory collision changed files");File.Delete(runtime);
    // Genuine NTFS deny-write permission is confined to this marked fixture.
    Directory.CreateDirectory(runtime);var directory=new DirectoryInfo(runtime);var acl=directory.GetAccessControl(AccessControlSections.Access);byte[] descriptor=acl.GetSecurityDescriptorBinaryForm();
    try{
     var rule=new FileSystemAccessRule(WindowsIdentity.GetCurrent().User,FileSystemRights.Write,InheritanceFlags.ContainerInherit|InheritanceFlags.ObjectInherit,PropagationFlags.None,AccessControlType.Deny);acl.AddAccessRule(rule);directory.SetAccessControl(acl);
     var aclBefore=Snapshot(root);denied=engine.Run(root,Modules(),null,fixture,s=>{},"Install",null,blocked.Conflicts);
     Check(!denied.Success&&denied.Details.Contains("COT_WRITE_ACCESS:")&&denied.Details.Contains(runtime),"NTFS denial was not diagnosed");Check(Same(aclBefore,Snapshot(root)),"Permission preflight changed files");
    }finally{var restored=new DirectorySecurity();restored.SetSecurityDescriptorBinaryForm(descriptor,AccessControlSections.Access);directory.SetAccessControl(restored);}
   }
   Application.EnableVisualStyles();Application.SetCompatibleTextRenderingDefault(false);
   using(var form=new SetupForm(fixture))using(var poll=new Timer{Interval=50}){
    int phase=0,reviews=0;Exception failure=null;var deadline=System.Diagnostics.Stopwatch.StartNew();form.ShowInTaskbar=false;form.WindowState=FormWindowState.Minimized;
    form.ReviewStandalone=list=>{Check(list.Count==3,"UI review lost a file");return ++reviews>1;};
    form.Shown+=(s,e)=>{form.SetModule(1,false);form.SetModule(3,false);form.ClickInstall();phase=1;poll.Start();};
    poll.Tick+=(s,e)=>{try{
     if(deadline.Elapsed.TotalSeconds>180)throw new TimeoutException("Standalone UI test timed out");if(form.Busy)return;
     if(phase==1){Check(!form.LastSuccess&&reviews==1,"Decline did not stop");Check(!form.StatusText.StartsWith("Close MMC")&&form.StatusText.Contains("standalone"),"UI gave a phantom process warning");Check(Same(before,Snapshot(root)),"Decline changed files");Check(!form.Selection()["GamedayAtmosphere"]&&!form.Selection()["DrivePractice"],"Review reset module choices");phase=2;form.ClickInstall();}
     else if(phase==2){Check(form.LastSuccess&&reviews==2,"Reviewed UI installation failed: "+form.DetailsText);Check(!InstallerEngine.LoadModules(root)["GamedayAtmosphere"]&&!InstallerEngine.LoadModules(root)["DrivePractice"],"Retry lost module choices");Check(!File.Exists(Path.Combine(root,"Plugins","DynamicGamedayAtmosphere.dll"))&&!File.Exists(Path.Combine(root,"Plugins","OfflineDynastySettings.dll")),"Legacy DLLs remain active");Check(File.ReadAllText(Path.Combine(root,"Plugins","DynamicGamedayAtmosphere","settings.ini"))=="user crowd settings","Settings lost");Check(File.ReadAllText(Path.Combine(root,"Plugins","DynastyEnhancer","RuntimeHistory","77","posts.json"))=="published words","History lost");phase=3;form.ClickRestore();}
     else{Check(form.LastSuccess,"Review rollback failed");Check(Same(before,Snapshot(root)),"Review rollback did not restore exact originals");poll.Stop();form.Close();}
    }catch(Exception error){failure=error;poll.Stop();form.Close();}};
    Application.Run(form);if(failure!=null)throw failure;
   }
   File.WriteAllText(args[3],new JavaScriptSerializer().Serialize(new{checks=checks,oldFalseWarningReproduced=true,reviewDeclineAndAccept=true,staleHashRejected=true,unrelatedFilesRejected=true,writeAccessAndCollision=true,settingsHistoryPreserved=true,rollback=true,liveInstall=false}));Console.WriteLine("PASS: "+checks+" actual standalone conflict/error/review/permission/rollback checks; fixture only.");return 0;
  }catch(Exception e){Console.Error.WriteLine(e);return 1;}}
 }
}
