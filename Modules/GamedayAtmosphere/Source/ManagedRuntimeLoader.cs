using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace DynamicGamedayAtmosphere {
    internal sealed class RuntimeModule {
        internal readonly string Name,Path;internal readonly long Base,Size;
        internal RuntimeModule(string name,string path,long address,long size){Name=name;Path=path;Base=address;Size=size;}
    }
    // Separates the load policy from Windows calls. Tests never attach to a game.
    internal interface IRuntimeLoadApi {
        IntPtr Open(int pid);
        string Image(IntPtr process);
        IList<RuntimeModule> Modules(int pid);
        long LocalLoadLibrary(out RuntimeModule owner);
        IntPtr Allocate(IntPtr process,int size);
        void Write(IntPtr process,IntPtr destination,byte[] bytes);
        IntPtr Start(IntPtr process,long function,IntPtr argument);
        void Wait(IntPtr thread);
        uint Result(IntPtr thread);
        void Free(IntPtr process,IntPtr allocation);
        void Close(IntPtr handle);
    }
    internal static class ManagedRuntimeLoader {
        static void Notice(Action<string> note,string text){if(note!=null)try{note(text);}catch{}}
        internal static void Load(int pid,string file,Action<string> note){Load(pid,file,new WindowsRuntimeLoadApi(),note);}
        internal static void Load(int pid,string file,IRuntimeLoadApi api,Action<string> note){
            string step="validate direct runtime load request";
            IntPtr process=IntPtr.Zero,thread=IntPtr.Zero,allocation=IntPtr.Zero;
            bool started=false,finished=false;
            try {
                if(IntPtr.Size!=8||pid<=0)throw new InvalidOperationException("An x64 host and valid game process ID are required.");
                file=Path.GetFullPath(file);
                if(!File.Exists(file))throw new FileNotFoundException("Native runtime DLL is missing.",file);
                if(!Path.GetFileName(file).StartsWith("DynamicGamedayAtmosphereNative-",StringComparison.OrdinalIgnoreCase)||!file.EndsWith(".dll",StringComparison.OrdinalIgnoreCase))
                    throw new InvalidOperationException("Only the versioned DGA runtime DLL can be loaded.");
                step="open game for direct runtime loading";process=api.Open(pid);
                step="validate direct-load target process";
                if(!String.Equals(Path.GetFileName(api.Image(process)),"CollegeFB27.exe",StringComparison.OrdinalIgnoreCase))throw new InvalidOperationException("Target is not CollegeFB27.exe.");
                step="inspect existing game runtime modules";
                var modules=api.Modules(pid);bool same=false;
                foreach(var module in modules){
                    if(String.Equals(module.Name,Path.GetFileName(file),StringComparison.OrdinalIgnoreCase)){
                        if(!String.Equals(Path.GetFullPath(module.Path),file,StringComparison.OrdinalIgnoreCase))throw new InvalidOperationException("The same runtime filename is already loaded from another folder. Restart the game and MMC.");
                        same=true;continue;
                    }
                    if(module.Name.StartsWith("DynamicAttendanceNative",StringComparison.OrdinalIgnoreCase)||module.Name.StartsWith("DynamicAttendanceProbe",StringComparison.OrdinalIgnoreCase)||module.Name.StartsWith("DynamicGamedayAtmosphereNative",StringComparison.OrdinalIgnoreCase))
                        throw new InvalidOperationException("Another attendance/atmosphere runtime is already loaded. Restart the game and MMC before updating.");
                }
                if(same){Notice(note,"Requested runtime is already loaded from its expected folder.");return;}
                step="resolve LoadLibraryW in its Windows module";
                RuntimeModule localOwner;long local=api.LocalLoadLibrary(out localOwner),offset=local-localOwner.Base;
                if(offset<0||offset>=localOwner.Size)throw new InvalidOperationException("LoadLibraryW is outside its reported Windows module.");
                RuntimeModule remoteOwner=null;
                foreach(var module in modules)if(String.Equals(module.Name,localOwner.Name,StringComparison.OrdinalIgnoreCase)&&String.Equals(Path.GetFullPath(module.Path),Path.GetFullPath(localOwner.Path),StringComparison.OrdinalIgnoreCase)){remoteOwner=module;break;}
                if(remoteOwner==null||offset>=remoteOwner.Size)throw new InvalidOperationException("Matching Windows loader module is unavailable in the game.");
                long entry=checked(remoteOwner.Base+offset);
                byte[] path=Encoding.Unicode.GetBytes(file+"\0");
                step="allocate game runtime-path buffer";allocation=api.Allocate(process,path.Length);
                step="write native runtime path";api.Write(process,allocation,path);
                step="start direct native runtime load";thread=api.Start(process,entry,allocation);started=true;
                step="wait for direct native runtime load";api.Wait(thread);finished=true;
                step="read direct native load result";uint result=api.Result(thread);
                step="verify native runtime module and path";bool verified=false;
                foreach(var module in api.Modules(pid))if(String.Equals(Path.GetFileName(file),module.Name,StringComparison.OrdinalIgnoreCase)&&String.Equals(Path.GetFullPath(module.Path),file,StringComparison.OrdinalIgnoreCase))verified=true;
                if(!verified)throw new InvalidOperationException("Load operation finished but the requested DLL is absent. Thread result: "+result+". Check security history, dependencies and runtime initialization.");
            }catch(Exception error){throw new LaunchOperationException(step,error);}
            finally {
                // An unfinished remote operation may still read the path. Do not
                // free it or retry. The game's exit releases that allocation.
                if(allocation!=IntPtr.Zero&&(!started||finished))try{api.Free(process,allocation);}catch(Exception error){Notice(note,"Direct loader cleanup warning: "+LaunchDiagnostics.OneLine(error.Message));}
                else if(allocation!=IntPtr.Zero)Notice(note,"Direct load completion is unknown; path buffer retained until game exit. No retry in this game process.");
                if(thread!=IntPtr.Zero)api.Close(thread);
                if(process!=IntPtr.Zero)api.Close(process);
            }
        }
    }
    internal sealed class WindowsRuntimeLoadApi : IRuntimeLoadApi {
        [StructLayout(LayoutKind.Sequential,CharSet=CharSet.Unicode)]
        struct ModuleEntry {
            public uint Size,ModuleId,ProcessId,GlobalUsage,ProcessUsage;public IntPtr Base;public uint Bytes;public IntPtr Module;
            [MarshalAs(UnmanagedType.ByValTStr,SizeConst=256)] public string Name;
            [MarshalAs(UnmanagedType.ByValTStr,SizeConst=260)] public string Path;
        }
        [StructLayout(LayoutKind.Sequential)]
        struct MemoryInfo {public IntPtr Base,AllocationBase;public uint AllocationProtect;public UIntPtr Size;public uint State,Protect,Type;}
        [StructLayout(LayoutKind.Sequential)]
        struct ModuleInfo {public IntPtr Base;public uint Size;public IntPtr Entry;}
        [DllImport("kernel32.dll",SetLastError=true)] static extern IntPtr OpenProcess(uint rights,bool inherit,int pid);
        [DllImport("kernel32.dll",CharSet=CharSet.Unicode,SetLastError=true)] static extern bool QueryFullProcessImageName(IntPtr process,int flags,StringBuilder value,ref int size);
        [DllImport("kernel32.dll",SetLastError=true)] static extern IntPtr CreateToolhelp32Snapshot(uint flags,int pid);
        [DllImport("kernel32.dll",CharSet=CharSet.Unicode,SetLastError=true)] static extern bool Module32FirstW(IntPtr snapshot,ref ModuleEntry entry);
        [DllImport("kernel32.dll",CharSet=CharSet.Unicode,SetLastError=true)] static extern bool Module32NextW(IntPtr snapshot,ref ModuleEntry entry);
        [DllImport("kernel32.dll",CharSet=CharSet.Unicode,SetLastError=true)] static extern IntPtr GetModuleHandle(string name);
        [DllImport("kernel32.dll",CharSet=CharSet.Ansi,ExactSpelling=true,SetLastError=true)] static extern IntPtr GetProcAddress(IntPtr module,string name);
        [DllImport("kernel32.dll",SetLastError=true)] static extern UIntPtr VirtualQuery(IntPtr address,out MemoryInfo info,UIntPtr size);
        [DllImport("kernel32.dll",CharSet=CharSet.Unicode,SetLastError=true)] static extern uint GetModuleFileName(IntPtr module,StringBuilder value,int size);
        [DllImport("psapi.dll",SetLastError=true)] static extern bool GetModuleInformation(IntPtr process,IntPtr module,out ModuleInfo info,uint size);
        [DllImport("kernel32.dll")] static extern IntPtr GetCurrentProcess();
        [DllImport("kernel32.dll",SetLastError=true)] static extern IntPtr VirtualAllocEx(IntPtr process,IntPtr address,UIntPtr size,uint allocation,uint protect);
        [DllImport("kernel32.dll",SetLastError=true)] static extern bool WriteProcessMemory(IntPtr process,IntPtr address,byte[] value,UIntPtr size,out UIntPtr written);
        [DllImport("kernel32.dll",SetLastError=true)] static extern IntPtr CreateRemoteThread(IntPtr process,IntPtr attributes,UIntPtr stack,IntPtr start,IntPtr argument,uint flags,IntPtr id);
        [DllImport("kernel32.dll",SetLastError=true)] static extern uint WaitForSingleObject(IntPtr handle,uint timeout);
        [DllImport("kernel32.dll",SetLastError=true)] static extern bool GetExitCodeThread(IntPtr thread,out uint result);
        [DllImport("kernel32.dll",SetLastError=true)] static extern bool VirtualFreeEx(IntPtr process,IntPtr address,UIntPtr size,uint kind);
        [DllImport("kernel32.dll")] static extern bool CloseHandle(IntPtr handle);
        static Exception Error(){return new Win32Exception(Marshal.GetLastWin32Error());}
        public IntPtr Open(int pid){var p=OpenProcess(0x103a,false,pid);if(p==IntPtr.Zero)throw Error();return p;}
        public string Image(IntPtr process){var text=new StringBuilder(32768);int size=text.Capacity;if(!QueryFullProcessImageName(process,0,text,ref size))throw Error();return text.ToString();}
        public IList<RuntimeModule> Modules(int pid){
            IntPtr snapshot=CreateToolhelp32Snapshot(0x18,pid);if(snapshot==new IntPtr(-1))throw Error();
            try{
                var entry=new ModuleEntry();entry.Size=(uint)Marshal.SizeOf(typeof(ModuleEntry));
                var list=new List<RuntimeModule>();bool found=Module32FirstW(snapshot,ref entry);
                if(!found)throw Error();
                do{list.Add(new RuntimeModule(entry.Name,entry.Path,entry.Base.ToInt64(),entry.Bytes));found=Module32NextW(snapshot,ref entry);}while(found);
                int error=Marshal.GetLastWin32Error();if(error!=18)throw new Win32Exception(error);
                return list;
            }finally{CloseHandle(snapshot);}
        }
        public long LocalLoadLibrary(out RuntimeModule owner){
            IntPtr module=GetModuleHandle("kernel32.dll");if(module==IntPtr.Zero)throw Error();
            IntPtr entry=GetProcAddress(module,"LoadLibraryW");if(entry==IntPtr.Zero)throw Error();
            MemoryInfo memory;if(VirtualQuery(entry,out memory,(UIntPtr)(uint)Marshal.SizeOf(typeof(MemoryInfo)))==UIntPtr.Zero)throw Error();
            var text=new StringBuilder(32768);uint n=GetModuleFileName(memory.AllocationBase,text,text.Capacity);if(n==0)throw Error();if(n>=text.Capacity)throw new Win32Exception(122);
            ModuleInfo info;if(!GetModuleInformation(GetCurrentProcess(),memory.AllocationBase,out info,(uint)Marshal.SizeOf(typeof(ModuleInfo))))throw Error();
            owner=new RuntimeModule(Path.GetFileName(text.ToString()),text.ToString(),info.Base.ToInt64(),info.Size);return entry.ToInt64();
        }
        public IntPtr Allocate(IntPtr p,int size){IntPtr value=VirtualAllocEx(p,IntPtr.Zero,(UIntPtr)(uint)size,0x3000,4);if(value==IntPtr.Zero)throw Error();return value;}
        public void Write(IntPtr p,IntPtr address,byte[] value){UIntPtr n;bool okay=WriteProcessMemory(p,address,value,(UIntPtr)(uint)value.Length,out n);if(!okay)throw Error();if(n.ToUInt64()!=(ulong)value.Length)throw new Win32Exception(299);}
        public IntPtr Start(IntPtr p,long entry,IntPtr argument){var thread=CreateRemoteThread(p,IntPtr.Zero,UIntPtr.Zero,new IntPtr(entry),argument,0,IntPtr.Zero);if(thread==IntPtr.Zero)throw Error();return thread;}
        public void Wait(IntPtr thread){uint result=WaitForSingleObject(thread,15000);if(result==0)return;if(result==0xffffffff)throw Error();throw new TimeoutException("Native load did not finish within 15 seconds. It may still be pending; restart the game and MMC before retrying.");}
        public uint Result(IntPtr thread){uint value;if(!GetExitCodeThread(thread,out value))throw Error();return value;}
        public void Free(IntPtr p,IntPtr address){if(!VirtualFreeEx(p,address,UIntPtr.Zero,0x8000))throw Error();}
        public void Close(IntPtr handle){CloseHandle(handle);}
    }
}
