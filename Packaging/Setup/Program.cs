using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Web.Script.Serialization;
using System.Windows.Forms;

namespace DynastyCOT.Setup {
 internal static class Program {
  [STAThread]private static int Main(string[] args){
   try{
    Application.EnableVisualStyles();Application.SetCompatibleTextRenderingDefault(false);
    if(args.Length==0){Application.Run(new SetupForm(null));return 0;}
    if(args.Length==3&&args[0]=="--preview-fixture"){
     Application.Run(new SetupForm(new Fixture(args[1],args[2])));return 0;
    }
    if(args.Length==4&&args[0]=="--smoke-test")return Smoke(new Fixture(args[1],args[2]),args[3]);
    if(args.Length==4&&args[0]=="--layout-test"){
     var fixture=new Fixture(args[1],args[2]);string output=Path.GetFullPath(args[3]);
     if(!output.StartsWith(fixture.Parent+"\\",StringComparison.OrdinalIgnoreCase))throw new IOException("Layout captures must stay inside the fixture parent.");Directory.CreateDirectory(output);
     var layouts=new List<Dictionary<string,object>>();
     foreach(float scale in new[]{1f,1.25f,1.5f})using(var form=new SetupForm(fixture)){form.ShowInTaskbar=false;layouts.Add(form.CaptureLayout(output,scale));form.Close();}
     foreach(int height in new[]{1080,1152})using(var form=new SetupForm(fixture)){form.ShowInTaskbar=false;layouts.Add(form.CaptureLayout(output,1.5f,new System.Drawing.Rectangle(0,0,1920,height)));form.Close();}
     File.WriteAllText(Path.Combine(output,"layout-report.json"),new JavaScriptSerializer().Serialize(layouts));
     Console.WriteLine("PASS: actual WinForms layout captured at 100/125/150 percent on all four pages; 150 percent bounded to 1080/1152 working areas, footer/update actions visible. Fixture only.");return 0;
    }
    if(args.Length==1&&args[0]=="--verify-payload"){
     using(var engine=new InstallerEngine())engine.Extract(Console.WriteLine);
     Console.WriteLine("PASS bundled package SHA-256, safe extraction and cleanup");return 0;
    }
    throw new ArgumentException("Unknown setup argument.");
   }catch(Exception error){
    if(args.Length==0)MessageBox.Show(error.Message,"Dynasty COT Setup",MessageBoxButtons.OK,MessageBoxIcon.Error);
    else Console.Error.WriteLine(error.ToString());
    return 1;
   }
  }
  private static int Smoke(Fixture fixture,string report){
   int checks=0;Action<bool,string> check=(ok,name)=>{if(!ok)throw new Exception(name);checks++;};
   using(var stream=typeof(Program).Assembly.GetManifestResourceStream("DynastyCOT.Crown.ico"))using(var icon=new System.Drawing.Icon(stream))check(icon.Width>0,"Windows crown icon is missing or invalid");
   using(var stream=typeof(Program).Assembly.GetManifestResourceStream("DynastyCOT.Crown.png"))using(var image=System.Drawing.Image.FromStream(stream))check(image.Width==image.Height&&image.Width>=256,"crown branding resource is missing or invalid");
   string outside=Path.Combine(fixture.Parent,"outside");bool rejected=false;
   try{InstallerEngine.Resolve(fixture.Root,"../outside");}catch(IOException){rejected=true;}check(rejected,"unsafe extraction accepted");
   check(InstallerEngine.Quote("MMC's $folder; & end")=="'MMC''s $folder; & end'","PowerShell literal quoting changed");
   rejected=false;try{new Fixture(outside,fixture.Parent);}catch(IOException){rejected=true;}check(rejected,"fixture marker not required");
   string parent=Path.Combine(fixture.Parent,"MMC"),manager=Path.Combine(parent,"MMC_ModManager_v1.1.0.6");
   Directory.CreateDirectory(manager);File.WriteAllText(Path.Combine(parent,"MMCModManager.exe"),"outer launcher fixture");File.WriteAllText(Path.Combine(manager,"MMCModManager.exe"),"manager fixture");
   check(InstallerEngine.ValidateRoot(parent)==manager,"outer MMC launcher was selected instead of Mod Manager");
   check(InstallerEngine.FindMmc(parent)==manager,"Mod Manager discovery failed");
   check(MmcLocator.FromExe(Path.Combine(manager,"MMCModManager.exe"))==manager,"exe picker did not resolve manager folder");
   check(MmcLocator.Candidates(new[]{parent,manager})==manager,"duplicate auto-locate candidates were not merged");
   rejected=false;try{MmcLocator.FromExe(Path.Combine(manager,"MMC.exe"));}catch(IOException){rejected=true;}check(rejected,"picker accepted wrong executable");
   string second=Path.Combine(parent,"MMC_ModManager_v1.2.0.0");Directory.CreateDirectory(second);File.WriteAllText(Path.Combine(second,"MMCModManager.exe"),"second manager fixture");
   rejected=false;try{InstallerEngine.ValidateRoot(parent);}catch(IOException){rejected=true;}check(rejected&&InstallerEngine.FindMmc(parent)==null&&MmcLocator.Candidates(new[]{manager,second})==null,"multiple versions were guessed");
   using(var form=new SetupForm(fixture))using(var poll=new System.Windows.Forms.Timer{Interval=50}){
    form.ShowInTaskbar=false;form.WindowState=FormWindowState.Minimized;
    Exception failure=null;int phase=0;var deadline=Stopwatch.StartNew();
    form.Shown+=(s,e)=>{
     try{
      var initial=form.Selection();check(!initial["DrivePractice"],"existing module choices not loaded");
      form.SetModule(0,true);form.SetModule(1,false);form.SetModule(2,true);form.SetModule(3,true);
      form.ClickInstall();check(form.Busy,"install button did not start worker");phase=1;poll.Start();
     }catch(Exception error){failure=error;form.Close();}
    };
    poll.Tick+=(s,e)=>{
     try{
      if(deadline.Elapsed.TotalSeconds>210)throw new TimeoutException("Fixture setup did not finish.");
      if(form.Busy)return;
      if(!form.LastSuccess)throw new IOException(form.StatusText+Environment.NewLine+form.DetailsText);
      if(phase==1){
       check(form.LastSuccess,"install result not shown");
       var selected=InstallerEngine.LoadModules(fixture.Root);check(!selected["GamedayAtmosphere"]&&selected["DrivePractice"],"checkboxes not saved transactionally");
       check(File.Exists(Path.Combine(fixture.Root,"Plugins","DynastyCOT.dll")),"host not installed");
       check(InstallerEngine.Backups(fixture.Root).Count>0,"backup picker was not populated");
       form.ClickRestore();check(form.Busy,"restore button did not start worker");phase=2;
      }else if(phase==2){
       check(form.StatusText.StartsWith("Backup restored"),"rollback result not shown");
       check(!InstallerEngine.LoadModules(fixture.Root)["DrivePractice"],"rollback did not restore prior module choices");
       form.SetOperation(0);for(int i=0;i<4;i++)form.SetModule(i,true);form.ClickInstall();check(form.Busy,"second install did not start");phase=3;
      }else if(phase==3){
       form.SetOperation(1);for(int i=0;i<4;i++)form.SetModule(i,i==3);form.ClickInstall();check(form.Busy,"selected uninstall did not start");phase=4;
      }else if(phase==4){
       check(!File.Exists(Path.Combine(fixture.Root,"Plugins","SituationalPractice","SituationalPractice.Runtime.exe")),"selected practice helper retained");
       check(File.Exists(Path.Combine(fixture.Root,"Plugins","DynastyCOT.dll")),"selected removal removed host");
       check(!InstallerEngine.LoadModules(fixture.Root)["DrivePractice"]&&InstallerEngine.LoadModules(fixture.Root)["DynastyEnhancer"],"selected removal disabled other modules");
       form.ClickRestore();check(form.Busy,"selected removal undo did not start");phase=5;
      }else if(phase==5){
       check(File.Exists(Path.Combine(fixture.Root,"Plugins","SituationalPractice","SituationalPractice.Runtime.exe")),"selected removal undo lost helper");
       form.SetOperation(2);form.ClickInstall();check(form.Busy,"uninstall all did not start");phase=6;
      }else if(phase==6){
       check(!File.Exists(Path.Combine(fixture.Root,"Plugins","DynastyCOT.dll")),"all removal retained host");
       check(File.Exists(Path.Combine(fixture.Root,"COT-Runtime","DynastyEnhancer","RuntimeHistory","101","news.json")),"all removal lost news");
       form.ClickRestore();check(form.Busy,"all removal undo did not start");phase=7;
      }else if(phase==7){
       check(File.Exists(Path.Combine(fixture.Root,"Plugins","DynastyCOT.dll")),"all removal undo lost host");
       form.ClickRestore();check(form.Busy,"fixture cleanup restore did not start");phase=8;
      }else{
       check(!InstallerEngine.LoadModules(fixture.Root)["DrivePractice"],"fixture cleanup did not restore initial settings");poll.Stop();form.Close();
      }
     }catch(Exception error){failure=error;poll.Stop();if(form.Busy)Application.ExitThread();else form.Close();}
    };
    Application.Run(form);if(failure!=null)throw failure;
   }
   var doc=new Dictionary<string,object>{{"checks",checks},{"fixture",fixture.Root},{"version",SetupInfo.Version},{"payloadSha256",SetupInfo.PayloadSha256},{"liveInstall",false}};
   File.WriteAllText(report,new JavaScriptSerializer().Serialize(doc));Console.WriteLine("PASS "+checks+" setup UI/engine checks; actual Install, selected/all Uninstall and Restore buttons, fixture only.");return 0;
  }
 }
}
