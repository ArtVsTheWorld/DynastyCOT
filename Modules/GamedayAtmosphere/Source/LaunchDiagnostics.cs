using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;

namespace DynamicGamedayAtmosphere {
    internal sealed class LaunchOperationException : Exception {
        internal readonly string Operation;
        internal LaunchOperationException(string operation,Exception error):base(error.Message,error){Operation=operation;}
    }
    internal interface ILaunchProcess {
        bool Exited {get;}
        IntPtr Window {get;}
        DateTime Started {get;}
    }
    internal sealed class LaunchProcess : ILaunchProcess {
        readonly Process process;
        internal LaunchProcess(Process value){process=value;}
        public bool Exited {get{return process.HasExited;}}
        public IntPtr Window {get{return process.MainWindowHandle;}}
        public DateTime Started {get{return process.StartTime.ToUniversalTime();}}
    }
    internal enum LaunchWait { Exited, Window, Startup, Ready }
    internal static class LaunchReadiness {
        internal static bool Read(ILaunchProcess process,DateTime utc,out long started){
            LaunchWait reason;return Read(process,utc,out started,out reason);
        }
        internal static bool Read(ILaunchProcess process,DateTime utc,out long started,out LaunchWait reason){
            reason=LaunchWait.Exited;
            started=0;string operation="check whether the game process has exited";
            try {
                if(process.Exited)return false;
                reason=LaunchWait.Window;
                operation="read the game window handle";if(process.Window==IntPtr.Zero)return false;
                reason=LaunchWait.Startup;
                operation="read the game process start time";DateTime time=process.Started;
                started=time.Ticks;if((utc-time).TotalSeconds<20)return false;
                reason=LaunchWait.Ready;return true;
            }catch(Exception error){throw new LaunchOperationException(operation,error);}
        }
        internal static string Description(LaunchWait reason){
            switch(reason){
                case LaunchWait.Window:return "Waiting for the game's main window. CollegeFB27.exe was found, but Windows reports no main window; a new runtime load cannot start in this state. If the game is already at its main menu, send this log.";
                case LaunchWait.Startup:return "Game window found; waiting until the game process is 20 seconds old before loading DGA.";
                case LaunchWait.Ready:return "Game window and startup wait are ready.";
                default:return "Game exited before this readiness check completed.";
            }
        }
    }
    // State changes are immediate; unchanged waiting/loaded states get one
    // reminder per minute. This is reporting only, never an activation gate.
    internal sealed class LaunchProgress {
        sealed class Entry {internal string State;internal long Logged;}
        readonly Dictionary<int,Entry> entries=new Dictionary<int,Entry>();
        internal bool Report(int pid,string state,long now){
            Entry entry;if(!entries.TryGetValue(pid,out entry)){entries[pid]=new Entry{State=state,Logged=now};return true;}
            if(entry.State==state&&now>=entry.Logged&&now-entry.Logged<60000)return false;
            entry.State=state;entry.Logged=now;return true;
        }
        internal string[] Retain(HashSet<int> present){
            var notes=new List<string>();var removed=new List<int>();
            foreach(var pair in entries){
                if(pair.Key==0||present.Contains(pair.Key))continue;
                notes.Add("Game process no longer found | PID "+pair.Key+" | Last launcher state: "+pair.Value.State+". This does not establish whether the game closed normally or crashed.");removed.Add(pair.Key);
            }
            foreach(int pid in removed)entries.Remove(pid);return notes.ToArray();
        }
        internal static string CandidateNames(string[] names){
            var selected=new SortedSet<string>(StringComparer.OrdinalIgnoreCase);
            foreach(string name in names){
                if(String.IsNullOrEmpty(name)||name.Length>80)continue;
                if(name.StartsWith("CollegeFB",StringComparison.OrdinalIgnoreCase)||String.Equals(name,"CFB27",StringComparison.OrdinalIgnoreCase))selected.Add(name+".exe");
            }
            var bounded=new List<string>();foreach(string name in selected){if(bounded.Count==8)break;bounded.Add(name);}
            return bounded.Count==0?"none":String.Join(", ",bounded.ToArray());
        }
    }
    internal static class LaunchDiagnostics {
        [DllImport("kernel32.dll",SetLastError=true)] static extern IntPtr OpenProcess(uint rights,bool inherit,int pid);
        [DllImport("advapi32.dll",SetLastError=true)] static extern bool OpenProcessToken(IntPtr process,uint access,out IntPtr token);
        [DllImport("advapi32.dll",SetLastError=true)] static extern bool GetTokenInformation(IntPtr token,int kind,out int value,int size,out int needed);
        [DllImport("kernel32.dll")] static extern bool CloseHandle(IntPtr handle);
        internal static string OneLine(string value){return (value??"").Replace("\r"," ").Replace("\n"," ").Trim();}
        internal static string Elevation(int pid){
            IntPtr process=IntPtr.Zero,token=IntPtr.Zero;
            try {
                process=OpenProcess(0x1000,false,pid);
                if(process==IntPtr.Zero)return "unknown (process query: Windows error "+Marshal.GetLastWin32Error()+")";
                if(!OpenProcessToken(process,8,out token))return "unknown (token query: Windows error "+Marshal.GetLastWin32Error()+")";
                int value,needed;if(!GetTokenInformation(token,20,out value,4,out needed))return "unknown (elevation query: Windows error "+Marshal.GetLastWin32Error()+")";
                return value!=0?"administrator":"standard";
            }catch(Exception error){return "unknown ("+OneLine(error.Message)+")";}
            finally{if(token!=IntPtr.Zero)CloseHandle(token);if(process!=IntPtr.Zero)CloseHandle(process);}
        }
        internal static Exception Root(Exception error){while(error.InnerException!=null)error=error.InnerException;return error;}
        internal static string Failure(int pid,string operation,string path,Exception error){
            var wrapped=error as LaunchOperationException;if(wrapped!=null)operation=wrapped.Operation;
            Exception root=Root(error);var win32=root as Win32Exception;
            int hr=Marshal.GetHRForException(root);int code=win32!=null?win32.NativeErrorCode:(unchecked((uint)hr)&0xffff0000u)==0x80070000u?hr&65535:0;
            string text="Activation check failed | PID "+pid+" | Step: "+OneLine(operation)+" | "+root.GetType().Name;
            if(code!=0)text+=" | Windows error "+code;
            text+=" | HRESULT 0x"+unchecked((uint)hr).ToString("X8")+" | "+OneLine(root.Message);
            if(!String.IsNullOrEmpty(path))text+=" | File: "+OneLine(path);
            foreach(Exception detail in Chain(error)){
                foreach(string key in new[]{"DgaSettingsOperation","DgaSettingsSource","DgaSettingsDestination","DgaSettingsBackup","DgaSettingsHint","DgaSettingsCleanup"}){
                    if(!detail.Data.Contains(key))continue;
                    string value=Convert.ToString(detail.Data[key]);
                    string label=key.Substring("DgaSettings".Length);
                    text+=" | Settings "+label+": "+OneLine(value);
                    if(label=="Source"||label=="Destination"||label=="Backup")text+=" ("+value.Length+" characters)";
                }
            }
            return text;
        }
        static IEnumerable<Exception> Chain(Exception error){for(;error!=null;error=error.InnerException)yield return error;}
        internal static string Signature(string operation,Exception error){
            var wrapped=error as LaunchOperationException;if(wrapped!=null)operation=wrapped.Operation;
            Exception root=Root(error);var win32=root as Win32Exception;
            return operation+"|"+root.GetType().FullName+"|"+(win32==null?Marshal.GetHRForException(root):win32.NativeErrorCode)+"|"+root.Message;
        }
    }
    // Only read-only process checks retry. Runtime loading stays once per
    // process/start-time identity because a failed load can be uncertain.
    internal sealed class LaunchBackoff {
        sealed class Entry {internal string Signature,Description;internal long Due,Logged,Seen;internal int Count,Hidden;}
        readonly Dictionary<int,Entry> failures=new Dictionary<int,Entry>();
        static readonly long[] waits={5000,15000,30000,60000};
        internal bool Due(int pid,long now){Entry e;if(!failures.TryGetValue(pid,out e))return true;if(now<e.Seen){e.Due=now;e.Logged=now-60000;}return now>=e.Due;}
        internal string Failed(int pid,long now,string operation,string path,Exception error){
            string signature=LaunchDiagnostics.Signature(operation,error);Entry e;bool fresh=!failures.TryGetValue(pid,out e)||e.Signature!=signature;
            if(fresh){e=new Entry{Signature=signature,Description=LaunchDiagnostics.Failure(pid,operation,path,error),Logged=now};failures[pid]=e;}
            ++e.Count;e.Seen=now;long delay=waits[Math.Min(e.Count-1,waits.Length-1)];e.Due=now+delay;
            if(!fresh&&now-e.Logged<60000){++e.Hidden;return null;}
            string note=e.Description+(e.Hidden>0?" | "+e.Hidden+" identical checks suppressed since the previous report":"")+" | Next process check in "+delay/1000+" seconds.";
            e.Hidden=0;e.Logged=now;return note;
        }
        internal string Recovered(int pid){Entry e;if(!failures.TryGetValue(pid,out e))return null;failures.Remove(pid);return "Game process checks recovered for PID "+pid+" after "+e.Count+" failure(s). Window and startup readiness are checked separately.";}
        internal void Retain(HashSet<int> present){var remove=new List<int>();foreach(int pid in failures.Keys)if(!present.Contains(pid))remove.Add(pid);foreach(int pid in remove)failures.Remove(pid);}
    }
    internal sealed class LaunchClaims {
        readonly Dictionary<int,long> attempted=new Dictionary<int,long>();
        internal bool Take(int pid,long started){long prior;if(attempted.TryGetValue(pid,out prior)&&prior==started)return false;attempted[pid]=started;return true;}
        internal void Retain(HashSet<int> present){var remove=new List<int>();foreach(int pid in attempted.Keys)if(!present.Contains(pid))remove.Add(pid);foreach(int pid in remove)attempted.Remove(pid);}
    }
    internal sealed class LaunchCapture {
        readonly object gate=new object();readonly StringBuilder buffer=new StringBuilder();int omitted;
        internal void Add(string text){if(text==null)return;lock(gate){int space=8192-buffer.Length;if(space>0)buffer.AppendLine(text.Substring(0,Math.Min(text.Length,space)));else ++omitted;}}
        internal string Text {get{lock(gate)return buffer.ToString()+(omitted>0?"[additional loader output omitted]":"");}}
    }
}
