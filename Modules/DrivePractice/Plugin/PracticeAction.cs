using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Collections.Generic;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using Frosty.Core;
using FrostySdk.Interfaces;

namespace SituationalPractice
{
    public sealed class PracticeAction : ExecutionAction
    {
        private static int watching;
        public override Action<ILogger,PluginManagerType,CancellationToken> PreLaunchAction {get{return (log,type,token)=>Start(log);}}
        public override Action<ILogger,PluginManagerType,CancellationToken> PostLaunchAction {get{return (log,type,token)=>Start(log);}}
        private static void Start(ILogger log)
        {
            if(Interlocked.CompareExchange(ref watching,1,0)!=0)return;
            var diagnostics=new SafeFileLog(Path.Combine(BuildIdentity.Folder,"launcher.log"));
            new Thread(()=>{try{
                diagnostics.Write("INFO","MMC discovered the plugin. Waiting for a game process; plugin="+BuildIdentity.Version+"; MMC PID="+Process.GetCurrentProcess().Id);
                var launched=new HashSet<string>();var attempts=new Dictionary<string,DateTime>();var lastFailure=new Dictionary<string,string>();
                while(true){
                    try{foreach(var p in Process.GetProcessesByName("CollegeFB27"))using(p){try{
                        if(p.HasExited)continue;
                        string key=p.Id+":"+p.StartTime.ToUniversalTime().Ticks;
                        if(launched.Contains(key))continue;
                        DateTime next;if(attempts.TryGetValue(key,out next)&&DateTime.UtcNow<next)continue;
                        bool firstAttempt=!attempts.ContainsKey(key);attempts[key]=DateTime.UtcNow.AddSeconds(10);
                        string folder=BuildIdentity.Folder;Directory.CreateDirectory(folder);
                        if(firstAttempt)WriteLaunchContext(folder,p,diagnostics);
                        string helper=RuntimeFiles.PathIn(Path.GetDirectoryName(typeof(PracticeAction).Assembly.Location));
                        int gamePid=p.Id;long gameStart=p.StartTime.ToUniversalTime().Ticks;
                        Process child;
                        using(RuntimeFiles.OpenVerified(helper))
                            child=Process.Start(new ProcessStartInfo(helper,gamePid.ToString()){UseShellExecute=false,CreateNoWindow=true,WindowStyle=ProcessWindowStyle.Hidden,WorkingDirectory=folder});
                        if(child==null)throw new InvalidOperationException("Helper launch returned no process.");
                        diagnostics.Write("INFO","Matching helper verified and started. Practice features wait for an eligible offline practice: game="+key+" helper PID="+child.Id+" path="+helper);
                        launched.Add(key);
                        child.Exited+=(sender,eventArgs)=>ObserveHelperExit(child,gamePid,gameStart,diagnostics);
                        child.EnableRaisingEvents=true;
                        if(log!=null)try{log.Log("[Expanded Practice] Helper started. Enter offline Dynasty or main-menu practice to activate the practice modes.",new object[0]);}catch(Exception error){diagnostics.Error("MMC readiness message",error);}
                    }catch(Exception error){
                        diagnostics.Error("Start practice helper for PID "+p.Id,error);
                        string failedKey=p.Id.ToString();string previous;
                        if(!lastFailure.TryGetValue(failedKey,out previous)||previous!=error.Message){
                            lastFailure[failedKey]=error.Message;
                            if(log!=null)try{log.Log("[Expanded Practice] Could not start the helper: "+error.Message,new object[0]);}catch{}
                        }
                    }}}
                    catch(Exception error){diagnostics.Error("Enumerate game processes",error);}
                    Thread.Sleep(1000);
                }
            }catch(Exception error){diagnostics.Error("Launch watcher failed",error);}
            finally{Interlocked.Exchange(ref watching,0);}}){IsBackground=true,Name="Expanded Practice launch watcher"}.Start();
        }
        private static void ObserveHelperExit(Process child,int gamePid,long gameStart,SafeFileLog diagnostics)
        {
            try{
                bool gameAlive=false;
                try{using(var game=Process.GetProcessById(gamePid))gameAlive=!game.HasExited && game.StartTime.ToUniversalTime().Ticks==gameStart;}catch(ArgumentException){}
                diagnostics.Write(child.ExitCode==0?"INFO":"ERROR","Helper exited: helper PID="+child.Id+" game PID="+gamePid+" helper exit="+DiagnosticSession.ExitStatus(child.ExitCode)+" original game still running="+gameAlive+". Check this session's diagnostic report. An exited helper is not automatically restarted over existing native hooks.");
                if(child.ExitCode!=0 || gameAlive && !File.Exists(Path.Combine(BuildIdentity.Folder,"runtime.stop"))){
                    Thread.Sleep(1500);
                    DiagnosticSession.ObserveHelperFailure(BuildIdentity.Folder,gamePid,child.Id,child.ExitCode,gameAlive);
                }
            }catch(Exception error){diagnostics.Error("Observe helper exit",error);}
            finally{child.Dispose();}
        }
        private static void WriteLaunchContext(string folder,Process game,SafeFileLog diagnostics)
        {
            try{
                var text=new StringBuilder("Expanded Practice "+BuildIdentity.Version+Environment.NewLine);
                text.AppendLine("Captured: "+DateTimeOffset.Now.ToString("O"));
                text.AppendLine("Game PID/start UTC: "+game.Id+" / "+game.StartTime.ToUniversalTime().ToString("O"));
                using(var mmc=Process.GetCurrentProcess())text.AppendLine("MMC: "+mmc.MainModule.FileName+" / "+FileVersionInfo.GetVersionInfo(mmc.MainModule.FileName).FileVersion);
                text.AppendLine("DLL files in MMC's plugin directory (presence does not establish that a plugin is loaded):");
                string directory=Path.GetDirectoryName(typeof(PracticeAction).Assembly.Location);
                string[] files=Directory.GetFiles(directory,"*.dll");Array.Sort(files,StringComparer.OrdinalIgnoreCase);
                foreach(string path in files)try{
                    text.AppendLine(Path.GetFileName(path)+" | version="+FileVersionInfo.GetVersionInfo(path).FileVersion+" | bytes="+new FileInfo(path).Length);
                }catch(Exception error){text.AppendLine(Path.GetFileName(path)+" | metadata unavailable: "+error.Message);}
                DiagnosticSession.AtomicText(Path.Combine(folder,"launcher-context.txt"),text.ToString());
            }catch(Exception error){diagnostics.Error("Record MMC/plugin context",error);}
        }
    }
}

