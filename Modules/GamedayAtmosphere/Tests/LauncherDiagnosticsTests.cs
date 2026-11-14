using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Threading;
using DynamicGamedayAtmosphere;
class LauncherDiagnosticsTests {
    sealed class Probe : ILaunchProcess {
        internal int Fail,Reads;internal DateTime Start;internal bool Closed;internal IntPtr Handle=new IntPtr(1);
        public bool Exited{get{if(Fail==1)throw new Win32Exception(5);return Closed;}}
        public IntPtr Window{get{if(Fail==2)throw new Win32Exception(5);return Handle;}}
        public DateTime Started{get{++Reads;if(Fail==3)throw new Win32Exception(5);return Start;}}
    }
    static void Check(bool value,string why){if(!value)throw new Exception(why);}
    static int Main(){try{
        DateTime now=new DateTime(2026,10,26,12,0,0,DateTimeKind.Utc);long identity;
        var p=new Probe{Start=now.AddSeconds(-19)};Check(!LaunchReadiness.Read(p,now,out identity),"startup grace");
        p.Start=now.AddSeconds(-20);Check(LaunchReadiness.Read(p,now,out identity)&&identity==p.Start.Ticks,"twenty-second ready boundary");
        p.Reads=0;p.Closed=true;Check(!LaunchReadiness.Read(p,now,out identity)&&p.Reads==0,"exit short circuit");
        p.Closed=false;p.Handle=IntPtr.Zero;Check(!LaunchReadiness.Read(p,now,out identity)&&p.Reads==0,"window short circuit");p.Handle=new IntPtr(1);
        LaunchWait reason;p.Closed=true;Check(!LaunchReadiness.Read(p,now,out identity,out reason)&&reason==LaunchWait.Exited,"explicit exited state");
        p.Closed=false;p.Handle=IntPtr.Zero;Check(!LaunchReadiness.Read(p,now,out identity,out reason)&&reason==LaunchWait.Window,"explicit missing-window state");
        p.Handle=new IntPtr(1);p.Start=now.AddSeconds(-19);Check(!LaunchReadiness.Read(p,now,out identity,out reason)&&reason==LaunchWait.Startup,"explicit startup-wait state");
        p.Start=now.AddSeconds(-20);Check(LaunchReadiness.Read(p,now,out identity,out reason)&&reason==LaunchWait.Ready,"ready state at unchanged boundary");
        p.Start=now.AddSeconds(1);Check(!LaunchReadiness.Read(p,now,out identity,out reason)&&reason==LaunchWait.Startup,"future start time does not bypass wait");p.Start=now.AddSeconds(-20);
        Check(LaunchReadiness.Description(LaunchWait.Window).Contains("no main window")&&LaunchReadiness.Description(LaunchWait.Startup).Contains("20 seconds"),"human readable wait descriptions");
        var progress=new LaunchProgress();
        Check(progress.Report(0,"no game found",0)&&!progress.Report(0,"no game found",1000)&&!progress.Report(0,"no game found",59999)&&!progress.Report(0,"no game found",60000),"no-game transition once; no healthy heartbeat");
        Check(progress.Report(0,"game found",60001)&&progress.Report(123,"Window",60001)&&!progress.Report(123,"Window",61000)&&progress.Report(123,"Startup",61001)&&progress.Report(123,"loading",61002),"independent PID states and immediate transitions");
        Check(progress.Report(123,"loaded",61003)&&!progress.Report(123,"loaded",62000)&&!progress.Report(123,"loaded",121003),"loaded state once without a reload claim");
        Check(progress.Retain(new HashSet<int>{123}).Length==0,"active PID retained");
        string[] ended=progress.Retain(new HashSet<int>());Check(ended.Length==1&&ended[0].Contains("123")&&ended[0].Contains("loaded")&&ended[0].Contains("does not establish")&&progress.Retain(new HashSet<int>()).Length==0,"one disappearance report with no crash assumption");
        Check(progress.Report(123,"Window",121004)&&progress.Report(123,"Window",100),"fresh process and monotonic rollback report safely");
        Check(LaunchProgress.CandidateNames(new[]{"chrome","MMCModManager","CollegeFB27","collegefb27","CollegeFB27_modded","CFB27"})=="CFB27.exe, CollegeFB27.exe, CollegeFB27_modded.exe","name-only hints are filtered sorted and deduplicated");
        var candidates=new List<string>();for(int i=0;i<20;++i)candidates.Add("CollegeFB"+i);
        Check(LaunchProgress.CandidateNames(candidates.ToArray()).Split(',').Length==8&&LaunchProgress.CandidateNames(new[]{"chrome",null,""})=="none","candidate hints bounded and unrelated apps excluded");
        string[] stages={"","check whether the game process has exited","read the game window handle","read the game process start time"};
        for(int stage=1;stage<=3;++stage){p.Fail=stage;try{LaunchReadiness.Read(p,now,out identity);throw new Exception("missing failure");}catch(LaunchOperationException error){
            string message=LaunchDiagnostics.Failure(123,"wrong outer label",null,error);
            Check(message.Contains(stages[stage])&&message.Contains("Windows error 5")&&message.Contains("HRESULT 0x")&&message.Contains("PID 123"),"specific denied process stage/code");
        }}
        string file=LaunchDiagnostics.Failure(123,"start runtime loader executable",@"C:\Mods\DGA\loader.exe",new Win32Exception(5));
        Check(file.Contains("loader.exe")&&file.Contains("start runtime loader executable"),"denied loader path");
        Check(LaunchDiagnostics.Failure(123,"extract settings",null,new UnauthorizedAccessException("denied")).Contains("Windows error 5"),"managed access HRESULT");
        var storageError=new DirectoryNotFoundException("Could not find a part of the path.");
        storageError.Data["DgaSettingsOperation"]="replace settings with Update 3 defaults";
        storageError.Data["DgaSettingsSource"]=@"X:\MMC\reset.tmp";
        storageError.Data["DgaSettingsDestination"]=@"X:\MMC\settings.ini";
        storageError.Data["DgaSettingsBackup"]=@"X:\MMC\Backups\settings.bak";
        storageError.Data["DgaSettingsCleanup"]="Temporary cleanup also failed";
        string storage=LaunchDiagnostics.Failure(123,"prepare or reset DGA settings",null,new IOException("outer context",storageError));
        Check(storage.Contains("Windows error 3")&&storage.Contains("replace settings with Update 3 defaults")&&storage.Contains("Settings Source:")&&storage.Contains("Settings Destination:")&&storage.Contains("Settings Backup:")&&storage.Contains("characters)")&&storage.Contains("Temporary cleanup also failed"),"storage sub-operation, all paths/lengths, root error and cleanup retained");
        Check(!LaunchDiagnostics.OneLine("a\r\nb").Contains("\n"),"one-line bounded log");
        var backoff=new LaunchBackoff();int checks=0,reports=0;string summary="";
        for(long ms=0;ms<=300000;ms+=1000){if(!backoff.Due(123,ms))continue;++checks;
            string note=backoff.Failed(123,ms,"read start time",null,new Win32Exception(5));if(note!=null){++reports;summary+=note;}}
        Check(checks==8&&reports==5&&summary.Contains("3 identical checks suppressed"),"5/15/30/60-second retry schedule with coalesced reports");
        Check(backoff.Failed(123,301000,"read window",null,new Win32Exception(299))!=null,"changed error reports immediately at next check");
        Check(backoff.Recovered(123)!=null&&backoff.Recovered(123)==null&&backoff.Due(123,301001),"one recovery notice and reset");
        backoff.Failed(55,1000,"read window",null,new Win32Exception(5));Check(backoff.Due(55,500),"monotonic-clock rollback safely resets wait");
        backoff.Retain(new HashSet<int>());Check(backoff.Due(55,1000),"ended process retires denied state");
        var claims=new LaunchClaims();Check(claims.Take(1,100)&&!claims.Take(1,100)&&claims.Take(1,200),"one loader attempt per PID/start time");
        claims.Retain(new HashSet<int>());Check(claims.Take(1,200),"retire finished PID claims");
        var capture=new LaunchCapture();var a=new Thread(()=>{for(int i=0;i<1000;++i)capture.Add("stdout");});var b=new Thread(()=>{for(int i=0;i<1000;++i)capture.Add("stderr");});a.Start();b.Start();a.Join();b.Join();
        Check(capture.Text.Length<8300&&capture.Text.Contains("omitted"),"bounded concurrent loader output");
        Check(!String.IsNullOrEmpty(LaunchDiagnostics.Elevation(Process.GetCurrentProcess().Id)),"read-only own-process elevation probe");
        Console.WriteLine("PASS: explicit readiness states and unchanged 20-second/window gates; immediate/60-second diagnostics, PID retirement, filtered bounded process hints; exact errors, backoff/recovery, PID reuse/once-only loads and bounded output. No game opened.");return 0;
    }catch(Exception error){Console.Error.WriteLine(error);return 1;}}
}
