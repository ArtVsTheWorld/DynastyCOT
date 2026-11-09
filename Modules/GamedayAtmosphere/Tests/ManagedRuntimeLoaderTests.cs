using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Text;
using DynamicGamedayAtmosphere;
class ManagedRuntimeLoaderTests {
    sealed class Fake : IRuntimeLoadApi {
        internal string File,Fail,Target="CollegeFB27.exe";internal bool Existing,Conflict,WrongExistingPath,MissingOwner,AbsentAfter,FreeFails;
        internal string OtherModule;
        internal int Starts,Frees,Closes,ModuleCalls;internal byte[] Bytes;internal long Entry;
        internal string Owner=Path.Combine(Environment.SystemDirectory,"kernelbase.dll");
        void Check(string step){if(Fail==step)throw new Win32Exception(5,"Test denied "+step);}
        public IntPtr Open(int pid){Check("open");return new IntPtr(1);}
        public string Image(IntPtr p){Check("image");return Path.Combine(Environment.SystemDirectory,Target);}
        public IList<RuntimeModule> Modules(int pid){
            ++ModuleCalls;Check(ModuleCalls==1?"modules":"verify");var list=new List<RuntimeModule>();
            if(!MissingOwner)list.Add(new RuntimeModule("kernelbase.dll",Owner,0x60000000,4096));
            if(Existing||(Starts>0&&!AbsentAfter))list.Add(new RuntimeModule(Path.GetFileName(File),WrongExistingPath?Path.Combine(Path.GetTempPath(),Path.GetFileName(File)):File,0x70000000,4096));
            if(Conflict)list.Add(new RuntimeModule("DynamicGamedayAtmosphereNative-1.0.0.dll","other",0x71000000,4096));
            if(OtherModule!=null)list.Add(new RuntimeModule(OtherModule,"other",0x72000000,4096));
            return list;
        }
        public long LocalLoadLibrary(out RuntimeModule owner){Check("local");owner=new RuntimeModule("kernelbase.dll",Owner,0x10000000,4096);return 0x10000040;}
        public IntPtr Allocate(IntPtr p,int size){Check("allocate");return new IntPtr(2);}
        public void Write(IntPtr p,IntPtr address,byte[] bytes){Check("write");Bytes=bytes;}
        public IntPtr Start(IntPtr p,long function,IntPtr argument){Check("start");++Starts;Entry=function;return new IntPtr(3);}
        public void Wait(IntPtr thread){Check("wait");}
        public uint Result(IntPtr thread){Check("result");return 0;} // Do not truncate x64 HMODULE into a success test.
        public void Free(IntPtr p,IntPtr value){++Frees;if(FreeFails)throw new Win32Exception(5,"Free denied");}
        public void Close(IntPtr p){++Closes;}
    }
    static void Check(bool value,string message){if(!value)throw new Exception(message);}
    static Exception Failure(Fake api){try{ManagedRuntimeLoader.Load(42,api.File,api,s=>{});}catch(Exception e){return e;}throw new Exception("Expected failure");}
    static int Main(){string folder=Path.Combine(Path.GetTempPath(),"DgaDirectTests-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(folder);
        try{
            string file=Path.Combine(folder,"DynamicGamedayAtmosphereNative-2.0.24.dll");File.WriteAllText(file,"Fixture only - never loaded");
            var okay=new Fake{File=file};ManagedRuntimeLoader.Load(42,file,okay,s=>{});
            Check(okay.Starts==1&&okay.Frees==1&&okay.Closes==2&&okay.Entry==0x60000040,"Successful load/relocation/cleanup");
            Check(Encoding.Unicode.GetString(okay.Bytes)==file+"\0","Exact UTF-16 path and null terminator");
            var existing=new Fake{File=file,Existing=true};ManagedRuntimeLoader.Load(42,file,existing,s=>{});Check(existing.Starts==0&&existing.Frees==0&&existing.Closes==1,"Existing runtime must not reload");
            foreach(string name in new[]{"BeyondTheFieldNative-v103.dll","BeyondTheFieldRecruitFog-v1.dll","GamedayAtmosphereNative-2.1.9.dll"}){
                string branded=Path.Combine(folder,name);File.WriteAllText(branded,"Fixture only");
                var accepted=new Fake{File=branded};ManagedRuntimeLoader.Load(42,branded,accepted,s=>{});Check(accepted.Starts==1,"Branded runtime rejected");
                string old=name.StartsWith("BeyondTheFieldRecruit")?"DynastyEnhancerRecruitFog-v1.dll":name.StartsWith("BeyondTheFieldNative")?"DynastyEnhancerNative-v103.dll":"DynamicGamedayAtmosphereNative-2.0.35.dll";
                var conflict=new Fake{File=branded,OtherModule=old};Failure(conflict);Check(conflict.Starts==0,"Legacy same-role runtime allowed");
            }
            var sibling=new Fake{File=Path.Combine(folder,"BeyondTheFieldNative-v103.dll"),OtherModule="BeyondTheFieldRecruitFog-v1.dll"};ManagedRuntimeLoader.Load(42,sibling.File,sibling,s=>{});Check(sibling.Starts==1,"Legitimate social/fog siblings conflict");
            foreach(string invalid in new[]{"BeyondTheFieldNative-other.dll","other.dll","GamedayAtmosphereNative-whatever.dll"})Check(!ManagedRuntimeLoader.AllowedRuntime(invalid),"Unknown runtime name accepted");
            foreach(string stage in new[]{"open","image","modules","local","allocate","write","start","wait","result","verify"}){
                var api=new Fake{File=file,Fail=stage};Exception error=Failure(api);
                Check(error is LaunchOperationException&&LaunchDiagnostics.Root(error) is Win32Exception,"Failure operation and Windows code: "+stage);
                string text=LaunchDiagnostics.Failure(42,"generic",file,error);Check(text.Contains("Windows error 5")&&text.Contains("Step:"),"Useful diagnostics: "+stage);
                Check(api.Starts==(stage=="wait"||stage=="result"||stage=="verify"?1:0),"Single or no start: "+stage);
                Check(api.Frees==(stage=="write"||stage=="start"||stage=="result"||stage=="verify"?1:0),"Path lifetime: "+stage);
                Check(api.Closes==(stage=="open"?0:stage=="wait"||stage=="result"||stage=="verify"?2:1),"Handle lifetime: "+stage);
            }
            foreach(var api in new[]{new Fake{File=file,Target="other.exe"},new Fake{File=file,Conflict=true},new Fake{File=file,Existing=true,Conflict=true},new Fake{File=file,Existing=true,WrongExistingPath=true},new Fake{File=file,MissingOwner=true}}){Failure(api);Check(api.Starts==0&&api.Frees==0,"Target/module guards before allocation");}
            var absent=new Fake{File=file,AbsentAfter=true};Failure(absent);Check(absent.Starts==1&&absent.Frees==1&&absent.Closes==2,"Missing runtime is not load success");
            var cleanup=new Fake{File=file,Fail="write",FreeFails=true};var first=Failure(cleanup);Check(LaunchDiagnostics.Root(first).Message.Contains("write"),"Cleanup cannot hide first failure");
            var noteFails=new Fake{File=file,FreeFails=true};ManagedRuntimeLoader.Load(42,file,noteFails,s=>{throw new Exception("Logger failed");});Check(noteFails.Closes==2,"Logger cannot prevent handle cleanup");
            var missing=new Fake{File=file+".missing"};Failure(missing);Check(missing.Closes==0,"Missing file fails before native calls");
            // Native marshalling and read-only queries against this test itself.
            // No allocation, write, remote thread or game process is used.
            using(var self=Process.GetCurrentProcess()){
                var native=new WindowsRuntimeLoadApi();IntPtr handle=native.Open(self.Id);
                try{Check(Path.GetFileName(native.Image(handle)).Equals(Path.GetFileName(self.MainModule.FileName),StringComparison.OrdinalIgnoreCase),"Image query ABI");}finally{native.Close(handle);}
                var modules=native.Modules(self.Id);Check(modules.Count>0,"Module enumeration ABI");RuntimeModule owner;long address=native.LocalLoadLibrary(out owner);
                Check(address>=owner.Base&&address<owner.Base+owner.Size,"Forwarded LoadLibrary owner lookup");
            }
            Console.WriteLine("PASS: direct loader target/path/conflict guards, UTF-16 path, forwarded Windows-module relocation, exact load verification, all failure stages, pending-operation allocation retention, cleanup/error ownership and read-only self-process ABI. No game opened or loaded.");return 0;
        }catch(Exception e){Console.Error.WriteLine(e);return 1;}
        finally{Directory.Delete(folder,true);}
    }
}
