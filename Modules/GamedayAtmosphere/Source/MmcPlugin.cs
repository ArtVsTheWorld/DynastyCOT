using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Security.Cryptography;
using System.Threading;
using System.Text;
using Frosty.Core;
using Frosty.Core.Attributes;
using FrostySdk.Interfaces;

[assembly: AssemblyTitle("Gameday Atmosphere")]
[assembly: AssemblyCompany("Ace")]
[assembly: AssemblyProduct("Gameday Atmosphere")]
[assembly: AssemblyDescription("Dynamic attendance, layered crowd sound, play reactions and broadcast effects")]
[assembly: AssemblyVersion(DynamicGamedayAtmosphere.BuildInfo.AssemblyVersion)]
[assembly: AssemblyFileVersion(DynamicGamedayAtmosphere.BuildInfo.AssemblyVersion)]
[assembly: PluginVersion(DynamicGamedayAtmosphere.BuildInfo.Version)]
[assembly: PluginAuthor("Ace")]
[assembly: PluginDisplayName("Gameday Atmosphere")]
[assembly: RegisterExecutionAction(typeof(DynamicGamedayAtmosphere.AttendanceAction))]

namespace DynamicGamedayAtmosphere
{
#if DIRECT_RUNTIME_LOADING && EXTERNAL_RUNTIME_LOADER
#error Select only one private loader variant
#endif
    public sealed class AttendanceAction : ExecutionAction
    {
#if DIRECT_RUNTIME_LOADING
        internal const bool DirectRuntimeLoading=true;
#else
        internal const bool DirectRuntimeLoading=false;
#endif
#if EXTERNAL_RUNTIME_LOADER
        internal const bool ExternalRuntimeLoading=true;
        private static void VerifyExternalLoader(string file)
        {
            if(!File.Exists(file))throw new FileNotFoundException("The separately packaged runtime loader is missing. Extract the complete private test ZIP into MMC.",file);
            using(var input=File.OpenRead(file))using(var hash=SHA256.Create())
                if(!String.Equals(BitConverter.ToString(hash.ComputeHash(input)).Replace("-",""),ExternalLoaderInfo.Sha256,StringComparison.OrdinalIgnoreCase))
                    throw new InvalidDataException("The separately packaged runtime loader does not match this plugin build. Restore the matching file from the private test ZIP.");
        }
#else
        internal const bool ExternalRuntimeLoading=false;
#endif
        private static int watching;
        private static Mutex watcherMutex;
        private static ILogger hostLogger;
        private static readonly object noteLock=new object();
        public AttendanceAction()
        {
            // Some MMC launch paths skip execution callbacks. Registration still runs.
            using(var host=Process.GetCurrentProcess())
                if(String.Equals(host.ProcessName,"MMCModManager",StringComparison.OrdinalIgnoreCase))Start(null);
        }
        public static string DataFolder { get { return PluginStorage.DataFolder(PluginsFolder); } }
        public static string RuntimeFolder { get { return PluginStorage.RuntimeFolder(PluginsFolder,BuildInfo.Version); } }
        public override Action<ILogger,PluginManagerType,CancellationToken> PreLaunchAction { get { return (log,type,token)=>Start(log); } }
        public override Action<ILogger,PluginManagerType,CancellationToken> PostLaunchAction { get { return (log,type,token)=>Start(log); } }
        public static string PluginsFolder { get { return PluginStorage.FindPluginsFolder(typeof(AttendanceAction).Assembly.Location); } }
        public static string LogsFolder { get { return Path.Combine(DataFolder,"Logs"); } }
        private static void Note(ILogger logger,string text)
        {
            logger=logger??Interlocked.CompareExchange(ref hostLogger,null,null);
            string failure=null;
            try {Directory.CreateDirectory(LogsFolder);
            lock(noteLock){
                string path=Path.Combine(LogsFolder,"launcher.log");
                byte[] entry=Encoding.UTF8.GetBytes(DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss")+" | "+text+Environment.NewLine);
                const int limit=2*1024*1024;
                int count=Math.Min(limit,entry.Length);long size=File.Exists(path)?new FileInfo(path).Length:0;
                using(var file=new FileStream(path,size>=limit||size+count>limit?FileMode.Create:FileMode.Append,FileAccess.Write,FileShare.Read))file.Write(entry,0,count);
            }}catch(Exception error){failure=" Log file unavailable: "+error.Message;}
            if(logger!=null)try{logger.Log("[Gameday] "+text+(failure??""),new object[0]);}catch{}
            else if(failure!=null)using(var diagnostic=new DefaultTraceListener())diagnostic.WriteLine("[Gameday] "+text+failure);
        }
        private static void Extract(string name,bool preserve)
        {
            string output=name=="DgaDiagnostics.exe"?Path.Combine(DataFolder,"Tools"):name.EndsWith(".ini",StringComparison.OrdinalIgnoreCase)?DataFolder:RuntimeFolder;
            Directory.CreateDirectory(output);string destination=Path.Combine(output,name);
            if(preserve&&File.Exists(destination))return;
            byte[] bytes;
            using(var resource=typeof(AttendanceAction).Assembly.GetManifestResourceStream("Attendance."+name))
            using(var buffer=new MemoryStream())
            { if(resource==null)throw new InvalidOperationException("Missing embedded resource: "+name);resource.CopyTo(buffer);bytes=buffer.ToArray(); }
            using(var hash=SHA256.Create())
            {
                byte[] expected=hash.ComputeHash(bytes);
                if(File.Exists(destination))using(var old=File.OpenRead(destination))
                    if(Convert.ToBase64String(hash.ComputeHash(old))==Convert.ToBase64String(expected))return;
                File.WriteAllBytes(destination,bytes);
                using(var saved=File.OpenRead(destination))if(Convert.ToBase64String(hash.ComputeHash(saved))!=Convert.ToBase64String(expected))throw new IOException("Extracted runtime hash differs");
            }
        }
        private static string RelatedGameProcesses()
        {
            // A bounded name-only hint for renamed executables. Never load a
            // process based on this hint, and do not log unrelated applications.
            try {
                var names=new List<string>();int unreadable=0;
                foreach(var process in Process.GetProcesses())using(process){try{names.Add(process.ProcessName);}catch{++unreadable;}}
                return LaunchProgress.CandidateNames(names.ToArray())+(unreadable>0?" | Process names unreadable: "+unreadable:"");
            }catch(Exception error){return "name scan unavailable ("+LaunchDiagnostics.OneLine(error.Message)+")";}
        }
        private static void Start(ILogger logger)
        {
            // Registration can start the watcher before MMC supplies its logger.
            // Later launch callbacks must still make that logger available.
            if(logger!=null)Interlocked.Exchange(ref hostLogger,logger);
            if(Interlocked.CompareExchange(ref watching,1,0)!=0)return;
            bool first=false;watcherMutex=new Mutex(false,"Local\\Ace.DynamicGamedayAtmosphere.MMC."+Process.GetCurrentProcess().Id,out first);
            if(!first)return;
            new Thread(()=>
            {
                var attempted=new LaunchClaims();var errors=new LaunchBackoff();var progress=new LaunchProgress();var clock=Stopwatch.StartNew();
                var outcomes=new Dictionary<int,string>();
                string watcherStep="create runtime folder",watcherFile=RuntimeFolder;
                string mmcElevation=LaunchDiagnostics.Elevation(Process.GetCurrentProcess().Id);
                try
                {
                    Directory.CreateDirectory(RuntimeFolder);
                    watcherStep="extract native runtime";watcherFile=Path.Combine(RuntimeFolder,BuildInfo.NativeFile);Extract(BuildInfo.NativeFile,false);
#if EXTERNAL_RUNTIME_LOADER
                    watcherStep="verify separately packaged runtime loader";watcherFile=Path.Combine(RuntimeFolder,"LoadDynamicGamedayAtmosphere.exe");VerifyExternalLoader(watcherFile);
#elif !DIRECT_RUNTIME_LOADING
                    watcherStep="extract runtime loader";watcherFile=Path.Combine(RuntimeFolder,"LoadDynamicGamedayAtmosphere.exe");Extract("LoadDynamicGamedayAtmosphere.exe",false);
#endif
                    watcherStep="read embedded default settings";watcherFile=null;
                    string defaults;using(var input=typeof(AttendanceAction).Assembly.GetManifestResourceStream("Attendance.DynamicGamedayAtmosphere.ini"))
                    using(var reader=new StreamReader(input))defaults=reader.ReadToEnd();
                    // An Update 2 runtime still uses the loose settings file.
                    // Never move its settings out from under a running game.
                    bool moving=File.Exists(Path.Combine(PluginsFolder,"DynamicGamedayAtmosphere.ini"))||File.Exists(Path.Combine(PluginsFolder,"DynamicAttendance.ini"))||PluginStorage.NeedsReset(PluginsFolder,defaults)||PluginStorage.NeedsRelocation(PluginsFolder)||PluginStorage.NeedsRuntimeRetirement(PluginsFolder);
                    bool noted=false;
                    while(moving){
                        watcherStep="wait for the running game before migrating settings";
                        var active=Process.GetProcessesByName("CollegeFB27");bool running=active.Length>0;foreach(var process in active)process.Dispose();
                        if(!running)break;
                        if(!noted){Note(logger,"Close the game once to prepare settings and retire previous runtime files. The running game's files are preserved until then.");noted=true;}
                        Thread.Sleep(1000);
                    }
                    watcherStep="prepare or reset DGA settings";watcherFile=PluginStorage.SettingsFile(PluginsFolder);string resetBackup=PluginStorage.Ensure(PluginsFolder,defaults);
                    watcherStep="archive previous runtime files outside MMC's plugin scan";watcherFile=PluginStorage.DataFolder(PluginsFolder);
                    try{
                        int retiredRuntimeFiles=PluginStorage.RetirePreviousRuntimes(PluginsFolder);
                        if(retiredRuntimeFiles>0)Note(logger,"Archived "+retiredRuntimeFiles+" previous runtime/helper files outside MMC's plugin scan. Settings, logs and crowd history stay in "+DataFolder+". A scan warning from this first upgrade startup clears after reopening MMC.");
                    }catch(Exception cleanupError){
                        Note(logger,"Previous runtime cleanup incomplete; direct loading will continue. "+LaunchDiagnostics.Failure(0,watcherStep,watcherFile,cleanupError)+" | Close MMC and the game before retrying cleanup. Old DLL scan warnings may remain until those files can be archived.");
                    }
                    if(resetBackup!=null)Note(logger,"Update 3 settings reset: all previous DGA values replaced with current defaults. Original INI saved at "+resetBackup+". This reset runs once; later custom settings are preserved.");
                    Note(logger,"Settings: "+PluginStorage.SettingsFile(PluginsFolder)+". Former loose DGA files, if present, are preserved in its Backups folder.");
                    watcherStep="extract diagnostics tool";watcherFile=Path.Combine(DataFolder,"Tools","DgaDiagnostics.exe");Extract("DgaDiagnostics.exe",false);
                    Note(logger,"DGA "+BuildInfo.Version+" launch watcher started | MMC PID "+Process.GetCurrentProcess().Id+" | MMC permissions: "+mmcElevation+". Automatic crowd departures are restricted to the verified offline Dynasty game layout.");
#if DIRECT_RUNTIME_LOADING
                    Note(logger,"Single-DLL install: runtime loading is handled directly by the MMC plugin. No loader EXE is needed. Generated native runtime: "+RuntimeFolder+"; settings/history/logs: "+DataFolder+".");
#elif EXTERNAL_RUNTIME_LOADER
                    Note(logger,"Separate loader verified: loader EXE is packaged beside the native runtime, outside the plugin DLL; its SHA256 matches this build.");
#endif
                    Note(logger,"Watching for CollegeFB27.exe once per second | Plugin: "+typeof(AttendanceAction).Assembly.Location+" | Logs: "+LogsFolder+". Loading requires a game main window and a process age of at least 20 seconds. Unchanged launcher states repeat once per minute.");
                    while(true)
                    {
                        watcherStep="enumerate game processes";watcherFile=null;var present=new HashSet<int>();
                        foreach(var game in Process.GetProcessesByName("CollegeFB27"))using(game)
                        {
                            int pid=game.Id;present.Add(pid);
                            if(progress.Report(0,"game found",clock.ElapsedMilliseconds))Note(logger,"CollegeFB27.exe detected; checking its window and startup wait.");
                            if(!errors.Due(pid,clock.ElapsedMilliseconds))continue;
                            string operation="check game readiness",file=null;bool loadAttempted=false;
                            try
                            {
                                long started;LaunchWait waiting;
                                bool ready=LaunchReadiness.Read(new LaunchProcess(game),DateTime.UtcNow,out started,out waiting);
                                string recovered=errors.Recovered(pid);if(recovered!=null)Note(logger,recovered);
                                if(!ready){
                                    if(progress.Report(pid,waiting.ToString(),clock.ElapsedMilliseconds)){
                                        string prior;string outcome=outcomes.TryGetValue(pid,out prior)?" | Previous runtime outcome: "+prior:"";
                                        Note(logger,"Game readiness | PID "+pid+" | "+LaunchReadiness.Description(waiting)+outcome);
                                    }
                                    continue;
                                }
                                if(!attempted.Take(pid,started)){
                                    string outcome;if(!outcomes.TryGetValue(pid,out outcome))outcome="Runtime load already attempted; no duplicate attempt will be made in this game process.";
                                    if(progress.Report(pid,outcome,clock.ElapsedMilliseconds))Note(logger,"Launcher check | PID "+pid+" | "+outcome);
                                    continue;
                                }
                                loadAttempted=true;outcomes[pid]="Runtime load started";
                                if(progress.Report(pid,"loading",clock.ElapsedMilliseconds))Note(logger,"Game ready | PID "+pid+" | Process started UTC "+new DateTime(started,DateTimeKind.Utc).ToString("yyyy-MM-dd HH:mm:ss")+" | Starting runtime load.");
#if DIRECT_RUNTIME_LOADING
                                file=Path.Combine(RuntimeFolder,BuildInfo.NativeFile);
                                operation="load native runtime directly from MMC plugin";
                                Note(logger,"Starting direct runtime load | PID "+pid+" | MMC permissions: "+mmcElevation+" | Game permissions: "+LaunchDiagnostics.Elevation(pid)+" | Runtime: "+file);
                                ManagedRuntimeLoader.Load(pid,file,text=>Note(logger,text));
                                Note(logger,"Runtime loaded directly for PID "+pid+". Activation status is in "+Path.Combine(LogsFolder,"DynamicGamedayAtmosphere.log")+".");
#else
                                file=Path.Combine(RuntimeFolder,"LoadDynamicGamedayAtmosphere.exe");
#if EXTERNAL_RUNTIME_LOADER
                                operation="verify separately packaged runtime loader before launch";VerifyExternalLoader(file);
#endif
                                string arguments=pid+" \""+Path.Combine(RuntimeFolder,BuildInfo.NativeFile)+"\"";
                                Note(logger,"Starting runtime loader | PID "+pid+" | MMC permissions: "+mmcElevation+" | Game permissions: "+LaunchDiagnostics.Elevation(pid)+" | Loader: "+file+" | Runtime: "+Path.Combine(RuntimeFolder,BuildInfo.NativeFile));
                                operation="start runtime loader executable";
                                using(var child=new Process())
                                {
                                    child.StartInfo=new ProcessStartInfo(file,arguments){WorkingDirectory=RuntimeFolder,UseShellExecute=false,CreateNoWindow=true,WindowStyle=ProcessWindowStyle.Hidden,RedirectStandardOutput=true,RedirectStandardError=true,StandardOutputEncoding=Encoding.UTF8,StandardErrorEncoding=Encoding.UTF8};
                                    var output=new LaunchCapture();var stderr=new LaunchCapture();
                                    child.OutputDataReceived+=(sender,line)=>output.Add(line.Data);child.ErrorDataReceived+=(sender,line)=>stderr.Add(line.Data);
                                    if(!child.Start())throw new IOException("Loader did not start");
                                    operation="read runtime loader output";child.BeginOutputReadLine();child.BeginErrorReadLine();
                                    operation="wait for runtime loader to finish";
                                    if(!child.WaitForExit(30000))throw new TimeoutException("Loader has not finished after 30 seconds. Loading may still be pending; another load will not be attempted in this game process.");
                                    child.WaitForExit();operation="check runtime loader result";
                                    if(child.ExitCode!=0)throw new IOException("Loader exit "+child.ExitCode+": "+LaunchDiagnostics.OneLine(stderr.Text+" "+output.Text));
                                    Note(logger,"Runtime loaded for PID "+pid+". Activation status is in "+Path.Combine(LogsFolder,"DynamicGamedayAtmosphere.log")+".");
                                    if(!String.IsNullOrWhiteSpace(stderr.Text))Note(logger,"Loader diagnostic | PID "+pid+" | "+LaunchDiagnostics.OneLine(stderr.Text));
                                }
#endif
                                outcomes[pid]="Runtime loaded; check DynamicGamedayAtmosphere.log and status.txt for in-game activation.";
                                progress.Report(pid,outcomes[pid],clock.ElapsedMilliseconds);
                            }
                            catch(Exception error){
                                if(loadAttempted){outcomes[pid]="Runtime load failed or timed out; no retry in this game process. Resolve the reported error, then restart MMC and the game.";progress.Report(pid,outcomes[pid],clock.ElapsedMilliseconds);Note(logger,LaunchDiagnostics.Failure(pid,operation,file,error)+" | No further runtime load will be attempted in this game process. Restart both MMC and the game after resolving the reported restriction.");}
                                else {progress.Report(pid,"readiness check failed",clock.ElapsedMilliseconds);string note=errors.Failed(pid,clock.ElapsedMilliseconds,operation,file,error);if(note!=null)Note(logger,note+" | MMC permissions: "+mmcElevation+" | Game permissions: "+LaunchDiagnostics.Elevation(pid)+". Check administrator-level differences or a reported security block.");}
                            }
                        }
                        if(present.Count==0&&progress.Report(0,"no game found",clock.ElapsedMilliseconds))Note(logger,"Waiting for CollegeFB27.exe: no process with that exact name was found. MMC's watcher is running; runtime loading has not been attempted for a current game. Keep this MMC open and launch the game. If already at the main menu, confirm the executable name in Task Manager > Details and that this is the MMC you launched. Related process names: "+RelatedGameProcesses());
                        foreach(string note in progress.Retain(present))Note(logger,note);
                        var retired=new List<int>();foreach(int pid in outcomes.Keys)if(!present.Contains(pid))retired.Add(pid);foreach(int pid in retired)outcomes.Remove(pid);
                        errors.Retain(present);attempted.Retain(present);
                        Thread.Sleep(1000);
                    }
                }
                catch(Exception error){try{Note(logger,"Watcher stopped | "+LaunchDiagnostics.Failure(Process.GetCurrentProcess().Id,watcherStep,watcherFile,error));}catch{}}
                finally{Interlocked.Exchange(ref watching,0);}
            }){IsBackground=true,Name="Gameday Atmosphere launch watcher"}.Start();
        }
    }
}
