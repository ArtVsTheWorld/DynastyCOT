using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace DynastyPlayClock {
    public sealed partial class DynastyPlayClockAction {
        internal const int PracticeDescriptor=0x3200,PracticeMenuPointer=0x3780,PracticeWriterPointer=0x3788;
        private static long practiceStart,practiceUnavailableStart;
        private static List<OfflineDynastySettings.MemoryCheck> practiceChecks;

        private static void BuildPracticeInjuryData(byte[] bytes,long data) {
            Array.Copy(bytes,0,bytes,PracticeDescriptor,0xb0);
            Put(bytes,PracticeDescriptor+0x10,data+0x3580);
            Put(bytes,PracticeDescriptor+0x28,data+0x35b0);
            Put(bytes,PracticeDescriptor+0x5c,0);
            Put(bytes,PracticeDescriptor+0x78,data+0x3300);
            Put(bytes,PracticeDescriptor+0x80,data+0x3318);
            string[] labels={"Default","Off for QBs","Off"};
            for(int i=0;i<labels.Length;i++) {
                int option=0x3380+i*0x80,label=0x36c0+i*0x40;
                Put(bytes,0x3300+i*8,data+option);Put(bytes,option+0x28,i);
                Put(bytes,option+0x30,data+label);PutString(bytes,label,labels[i]);
            }
            PutString(bytes,0x3580,"Practice Injuries");
            PutString(bytes,0x35b0,"Default keeps normal practice injury rules. Off for QBs protects quarterbacks on every team. Off prevents new practice injuries. Game injuries, reps and fatigue stay normal. Saved for this Dynasty.");
        }

        // Exact local-career database ownership, also used by Formation Subs.
        // A stale LeagueID or a menu mode alone does not authorize an override.
        private static void AppendPracticeScope(Code c,long image,string native) {
            c.MovR10(image+0xd9eb808);c.Hex("41833aff");c.Branch("0f84","practiceModeReady");
            c.Hex("41833a0e");c.Branch("0f85",native);c.Label("practiceModeReady");
            c.MovR10(image+0xf3fddbc);c.Hex("41833a03");c.Branch("0f85",native);
            c.MovR10(image+0xfa519b8);c.Hex("4d8b124d85d2");c.Branch("0f84",native);
            c.Hex("49bb");c.Long(image+0xbaa8938);c.Hex("4d391a");c.Branch("0f85",native);
            c.Hex("4180bab107000000");c.Branch("0f84",native);
            c.Hex("498b82680400004885c0");c.Branch("0f84",native);
            c.MovR10(image+0xbce9a78);c.Hex("4c3910");c.Branch("0f85",native);
            c.Hex("4c8b90300200004d85d2");c.Branch("0f84",native);
            c.Hex("49bb");c.Long(image+0xb4dbab8);c.Hex("4d391a");c.Branch("0f85",native);
            c.Hex("4c8b98400200004d85db");c.Branch("0f84",native);
            c.Hex("4d3913");c.Branch("0f85",native);
            c.Hex("4d8b5b084d85db");c.Branch("0f84",native);
        }

        internal static byte[] BuildPracticeInjuryBridge(int kind,long image,long data) {
            if(kind<0||kind>1)throw new ArgumentOutOfRangeException("kind");
            Code c=new Code();
            // Flags and all volatile integer inputs survive both paths. No
            // allocation, managed call, lock, RNG call or broad scan in the hook.
            c.Hex("9c5051524150415141524153"); // 0x40 bytes
            c.MovR10(image+(kind==0?PracticeRollCaller:PracticeRiskCaller));
            c.Hex("4c399424");c.Int(kind==0?0x198:0x40);c.Branch("0f85","native");
            AppendPracticeScope(c,image,"native");
            AppendDynastyClockRead(c,image,data,false,"practiceChoice",false,true);
            c.Hex("83f802");c.Branch("0f84","suppress");
            c.Hex("83f801");c.Branch("0f85","native");
            if(kind==0) {
                // The verified weekly caller already resolved Player.Position
                // (schema field 3) and saved position*4 at [parent RBP-0x19].
                // Native generation has saved that parent's RBP at RSP+0x150.
                c.Hex("4c8b94249001000049837ae700");c.Branch("0f85","native");
            } else {
                // The preview caller saves that same native Player.Position at
                // its RSP+0x70: +8 for CALL, +0x40 for our saved registers.
                c.Hex("83bc24b800000000");c.Branch("0f85","native");
            }
            c.Label("suppress");c.Hex("415b415a415941585a59589d");
            if(kind==0) {
                // Keep the existing probability computation and first random
                // draw. Skip only the new-injury branch before native writes.
                c.Hex("c5f82ff0c5f828b42400010000");c.Bytes(Jump(image+PracticeNoInjury));
            } else {c.Hex("33c0c3");} // Same preview result as native practice injuries Off.
            c.Label("native");c.Hex("415b415a415941585a59589d");
            if(kind==0) {
                c.Hex("c5f82ff0c5f828b42400010000");c.Branch("0f86","nativeNoInjury");
                c.Bytes(Jump(image+PracticeRollSite+PracticeRollOriginal.Length));
                c.Label("nativeNoInjury");c.Bytes(Jump(image+PracticeNoInjury));
            } else {c.Bytes(PracticeRiskOriginal);c.Bytes(Jump(image+PracticeRiskSite+PracticeRiskOriginal.Length));}
            return c.Finish();
        }

        private static byte[] BuildPracticeMenuRow(long image,long data) {
            Code c=new Code();c.Hex("4883ec28");
            AppendDynastyClockRead(c,image,data,false,"practiceMenu",false,true);
            c.Hex("83f802");c.Branch("0f87","done");
            c.MovR10(data+PracticeDescriptor);c.Hex("4189425c");
            c.Hex("895c242041b80310000049b9");c.Long(data+PracticeDescriptor);
            c.Hex("488bd6488bcf");c.Call(image+0x799faa0);c.Hex("ffc3");
            c.Label("done");c.Hex("4883c428c3");return c.Finish();
        }
        internal static byte[] BuildPracticeMenuDispatch(long data) {
            Code c=new Code();c.Hex("4883ec28");c.MovR10(data+PracticeMenuPointer);
            c.Hex("498b024885c0");c.Branch("0f84","done");c.Hex("ffd0");
            c.Label("done");c.Hex("4883c428c3");return c.Finish();
        }
        private static byte[] BuildPracticeWriter(long image,long data) {
            Code c=new Code();
            c.Hex("498b8e18040000488b01ff90100a000084c0");c.Branch("0f85","done");
            c.Hex("488b06418bd749b8");c.Long(image+0xad960f0);c.Hex("488bceff5018");
            AppendDynastyClockWrite(c,image,data,false,"practiceWrite",false,true);
            c.Label("done");c.Bytes(Jump(image+0x79a850d));return c.Finish();
        }
        internal static byte[] ExportPracticeRead(long image,long data) {
            Code c=new Code();AppendDynastyClockRead(c,image,data,false,"testPracticeRead",false,true);c.Hex("c3");return c.Finish();
        }
        internal static byte[] ExportPracticeWrite(long image,long data) {
            Code c=new Code();c.Hex("8bc1");AppendDynastyClockWrite(c,image,data,false,"testPracticeWrite",false,true);c.Hex("c3");return c.Finish();
        }

        internal static void InstallPracticeInjuries(int pid) {
            lock(Sync) using(Process game=Process.GetProcessById(pid)) {
                long start=game.StartTime.ToUniversalTime().Ticks;
                if(installedPid!=pid||installedStart!=start)throw new InvalidOperationException("Settings installation not ready.");
                if(practiceStart==start||practiceUnavailableStart==start)return;
                IntPtr h=IntPtr.Zero,allocation=IntPtr.Zero;bool published=false;
                try {
                    h=OpenProcess(0x1038,false,pid);if(h==IntPtr.Zero)throw new InvalidOperationException("Could not open practice-injury installation.");
                    long image=installedBase,data=dataAddress;
                    var originals=new List<OfflineDynastySettings.MemoryCheck>();
                    foreach(Region region in PracticeInjuryRegions) {
                        byte[] bytes=Read(h,image+region.Rva,region.Length);
                        if(bytes==null||Hash(bytes)!=region.Hash)throw new InvalidOperationException(OfflineDynastySettings.DiagnosticLog.Mismatch(region.Name,region.Rva,region.Hash,bytes));
                        originals.Add(new OfflineDynastySettings.MemoryCheck(image+region.Rva,bytes));
                    }
                    VerifyBytes(h,image+PracticeRollSite,PracticeRollOriginal,"Practice injury decision owner");
                    VerifyBytes(h,image+PracticeRiskSite,PracticeRiskOriginal,"Practice risk owner");
                    allocation=VirtualAllocEx(h,IntPtr.Zero,new UIntPtr(0x2000),0x3000,4);
                    if(allocation==IntPtr.Zero)throw new InvalidOperationException("Practice-injury code allocation failed.");
                    long remote=allocation.ToInt64();var checks=new List<OfflineDynastySettings.MemoryCheck>();
                    byte[][] bodies={BuildPracticeInjuryBridge(0,image,data),BuildPracticeInjuryBridge(1,image,data),BuildPracticeMenuRow(image,data),BuildPracticeWriter(image,data)};
                    int[] offsets={0,0x800,0x1000,0x1400};
                    for(int i=0;i<bodies.Length;i++) {
                        if(bodies[i].Length>(i==2?0x400:0x800))throw new InvalidOperationException("Practice-injury bridge exceeds its bound.");
                        Write(h,remote+offsets[i],bodies[i]);VerifyBytes(h,remote+offsets[i],bodies[i],"Practice-injury code readback");
                        checks.Add(new OfflineDynastySettings.MemoryCheck(remote+offsets[i],bodies[i]));
                    }
                    uint previous;
                    if(!VirtualProtectEx(h,allocation,new UIntPtr(0x2000),0x20,out previous)||!FlushInstructionCache(h,allocation,new UIntPtr(0x2000)))throw new InvalidOperationException("Practice-injury code protection failed.");
                    byte[] roll=Pad(Jump(remote),PracticeRollOriginal.Length),risk=Pad(Jump(remote+0x800),PracticeRiskOriginal.Length);
                    var changes=new List<ControllerChange>{
                        new ControllerChange(image+PracticeRollSite,PracticeRollOriginal,roll),
                        new ControllerChange(image+PracticeRiskSite,PracticeRiskOriginal,risk),
                        new ControllerChange(data+PracticeMenuPointer,new byte[8],BitConverter.GetBytes(remote+0x1000)),
                        new ControllerChange(data+PracticeWriterPointer,new byte[8],BitConverter.GetBytes(remote+0x1400))};
                    using(new ControllerFrozen(game,changes)) {
                        foreach(var check in originals)VerifyBytes(h,check.Address,Convert.FromBase64String(check.Bytes),"Practice-injury pre-commit contract");
                        foreach(var change in changes)VerifyBytes(h,change.Address,change.Before,"Practice-injury pre-commit owner");
                        CommitPracticeChanges(changes,(change,restore)=>{
                            published=true;byte[] bytes=restore?change.Before:change.After;
                            Patch(h,change.Address,bytes);VerifyBytes(h,change.Address,bytes,"Practice-injury transactional readback");
                        });
                    }
                    foreach(var change in changes)checks.Add(new OfflineDynastySettings.MemoryCheck(change.Address,change.After));
                    foreach(var original in originals) {
                        byte[] bytes=Convert.FromBase64String(original.Bytes);
                        foreach(var change in changes)if(change.Address>=original.Address&&change.Address+change.After.Length<=original.Address+bytes.Length)
                            Array.Copy(change.After,0,bytes,(int)(change.Address-original.Address),change.After.Length);
                        checks.Add(new OfflineDynastySettings.MemoryCheck(original.Address,bytes));
                    }
                    practiceChecks=checks;practiceStart=start;
                    Log(null,"Practice Injuries ready: Default / Off for QBs / Off, per Dynasty. Existing injuries and game injuries preserved. Code=0x"+remote.ToString("x"));
                } catch(OfflineDynastySettings.HookOwnershipException) {throw;}
                catch(Exception error) {practiceUnavailableStart=start;Log(null,"Practice-injury options unavailable; other Extra Settings remain active. "+error.ToString());}
                finally {if(allocation!=IntPtr.Zero&&!published)FormationSubsFree(h,allocation,UIntPtr.Zero,0x8000);if(h!=IntPtr.Zero)CloseHandle(h);}
            }
        }
        private static void CommitPracticeChanges(List<ControllerChange> changes,Action<ControllerChange,bool> apply) {
            int applied=0;
            try{foreach(var change in changes){applied++;apply(change,false);}}
            catch(Exception failure){
                Exception rollback=null;
                for(int i=applied-1;i>=0;i--)try{apply(changes[i],true);}catch(Exception error){rollback=error;}
                if(rollback!=null)throw new OfflineDynastySettings.HookOwnershipException("Practice-injury installation failed and rollback was incomplete. Close the game before retrying. "+failure.Message+"; rollback: "+rollback.Message);
                throw;
            }
        }
        internal static List<OfflineDynastySettings.MemoryCheck> PracticeInjuryChecks() {
            lock(Sync)return practiceStart==installedStart&&practiceChecks!=null?new List<OfflineDynastySettings.MemoryCheck>(practiceChecks):new List<OfflineDynastySettings.MemoryCheck>();
        }
        internal static void VerifyPracticeInjuries(int pid) {
            lock(Sync) {
                if(practiceStart!=installedStart||practiceChecks==null)return;
                IntPtr h=OpenProcess(0x1010,false,pid);if(h==IntPtr.Zero)throw new InvalidOperationException("Could not verify practice-injury hooks.");
                try{foreach(var check in practiceChecks)VerifyBytes(h,check.Address,Convert.FromBase64String(check.Bytes),"Practice-injury ownership");}finally{CloseHandle(h);}
            }
        }
    }
}
