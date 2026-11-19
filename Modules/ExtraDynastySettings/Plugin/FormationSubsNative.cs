using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace DynastyPlayClock
{
    public sealed partial class DynastyPlayClockAction
    {
        private static long formationSubsStart,formationSubsUnavailableStart;
        private static List<OfflineDynastySettings.MemoryCheck> formationSubsChecks;

        internal static void InstallFormationSubs(int pid)
        {
            lock(Sync) using(Process game=Process.GetProcessById(pid)){
                long start=game.StartTime.ToUniversalTime().Ticks;
                if(installedPid!=pid||installedStart!=start)throw new InvalidOperationException("Settings installation not ready.");
                if(formationSubsStart==start||formationSubsUnavailableStart==start)return;
                IntPtr h=IntPtr.Zero,allocation=IntPtr.Zero;bool published=false;
                try{
                    h=OpenProcess(0x1038,false,pid);
                    if(h==IntPtr.Zero)throw new InvalidOperationException("Could not open Formation Subs installation (Windows error "+Marshal.GetLastWin32Error()+").");
                    long b=installedBase;
                    List<OfflineDynastySettings.MemoryCheck> originals=new List<OfflineDynastySettings.MemoryCheck>();
                    foreach(Region region in FormationSubsRegions){
                        byte[] data=Read(h,b+region.Rva,region.Length);
                        if(data==null||Hash(data)!=region.Hash)throw new InvalidOperationException(OfflineDynastySettings.DiagnosticLog.Mismatch(region.Name,region.Rva,region.Hash,data));
                        originals.Add(new OfflineDynastySettings.MemoryCheck(b+region.Rva,data));
                    }
                    foreach(Region region in FormationPreviewRegions){
                        byte[] data=Read(h,b+region.Rva,region.Length);
                        if(data==null||Hash(data)!=region.Hash)throw new InvalidOperationException(OfflineDynastySettings.DiagnosticLog.Mismatch(region.Name,region.Rva,region.Hash,data));
                        originals.Add(new OfflineDynastySettings.MemoryCheck(b+region.Rva,data));
                    }
                    VerifyBytes(h,b+0xb2e82e0+0xa0,BitConverter.GetBytes(b+0x2b10120),"Formation set name owner");
                    VerifyBytes(h,b+0xbae21b8+0x1a8,BitConverter.GetBytes(b+0x7b5f6a0),"Formation preview owner");
                    VerifyBytes(h,b+0xda1dda0,BitConverter.GetBytes(b+0xb5757a8),"Formation preview output owner");
                    long[] slots={0x318,0x340,0x350,0x370,0x390};int[] kinds={0,1,3,4,5};
                    for(int i=0;i<slots.Length;i++)VerifyBytes(h,b+0xb3324d0+slots[i],BitConverter.GetBytes(b+FormationSubsOriginals[kinds[i]]),"Formation Subs library owner");
                    VerifyBytes(h,b+0xb3324d0+0xc0,BitConverter.GetBytes(b+0x2f24d50),"Formation Subs default-type owner");
                    VerifyBytes(h,b+0xb3324d0+0x520,BitConverter.GetBytes(b+0x2f25700),"Formation Subs count-filter owner");
                    VerifyBytes(h,b+0x3760774,FormationLookupCall,"Formation Subs typed lookup owner");
                    // All external transfers are absolute. Let Windows choose free
                    // memory rather than requiring a free arena near the game.
                    allocation=VirtualAllocEx(h,IntPtr.Zero,new UIntPtr(0x8000),0x3000,4);
                    if(allocation==IntPtr.Zero)throw new InvalidOperationException("Formation Subs code allocation failed (Windows error "+Marshal.GetLastWin32Error()+"); native menu retained.");
                    Log(null,"Formation Subs code space reserved without proximity requirement; address=0x"+allocation.ToInt64().ToString("x"));
                    long remote=allocation.ToInt64();List<OfflineDynastySettings.MemoryCheck> checks=new List<OfflineDynastySettings.MemoryCheck>();
                    for(int kind=0;kind<6;kind++){
                        byte[] code=BuildFormationSubsBridge(kind,b,remote);if(code.Length>0x800)throw new InvalidOperationException("Formation Subs bridge exceeds its bound.");
                        Write(h,remote+kind*0x800,code);checks.Add(new OfflineDynastySettings.MemoryCheck(remote+kind*0x800,code));
                    }
                    byte[] native=Read(h,b+0x3488850,215);
                    foreach(int type in new int[]{2,12}){
                        long address=remote+(type==2?0x3000:0x3400);byte[] clone=CloneTypedFormationItem(native,b,address,type);
                        Write(h,address,clone);checks.Add(new OfflineDynastySettings.MemoryCheck(address,clone));
                    }
                    byte[] find=BuildFormationSubsFind();Write(h,remote+0x3800,find);checks.Add(new OfflineDynastySettings.MemoryCheck(remote+0x3800,find));
                    byte[] lookupNative=Read(h,b+0x3479750,0xa0),lookupClone=CloneFormationLookup(lookupNative,b);
                    Write(h,remote+0x3d00,lookupClone);checks.Add(new OfflineDynastySettings.MemoryCheck(remote+0x3d00,lookupClone));
                    byte[][] previewCode={BuildFormationPreview(b,remote),BuildReturnSetMembership(b,remote),BuildPreviewTreeFind(),BuildPreviewFloat(b),BuildPreviewString(),BuildFormationGroup(b,remote),BuildGroupedFormationSet(b,remote),BuildFormationSetName(b,remote),BuildTypedSetMembership(b,remote)};
                    int[] previewOffsets={0x5000,0x5800,0x5b00,0x5e00,0x5f00,0x6100,0x6500,0x7000,0x7400};
                    int[] previewLimits={0x800,0x300,0x300,0x100,0x100,0x400,0xb00,0x400,0x400};
                    for(int i=0;i<previewCode.Length;i++){
                        if(previewCode[i].Length>previewLimits[i])throw new InvalidOperationException("Formation preview code exceeds its bound.");
                        Write(h,remote+previewOffsets[i],previewCode[i]);checks.Add(new OfflineDynastySettings.MemoryCheck(remote+previewOffsets[i],previewCode[i]));
                    }
                    foreach(var label in new[]{new {Address=remote+0x7800,Text="Kickoff"},new {Address=remote+0x7820,Text="Kickoff Return"}}){
                        byte[] bytes=System.Text.Encoding.ASCII.GetBytes(label.Text+"\0");Write(h,label.Address,bytes);checks.Add(new OfflineDynastySettings.MemoryCheck(label.Address,bytes));
                    }
                    byte[] previewVtable=new byte[64];
                    Array.Copy(BitConverter.GetBytes(remote+0x5e00),0,previewVtable,0x28,8);
                    Array.Copy(BitConverter.GetBytes(remote+0x5f00),0,previewVtable,0x30,8);
                    Write(h,remote+0x6000,previewVtable);checks.Add(new OfflineDynastySettings.MemoryCheck(remote+0x6000,previewVtable));
                    foreach(var check in checks)VerifyBytes(h,check.Address,Convert.FromBase64String(check.Bytes),"Formation Subs bridge readback");
                    uint previous;if(!VirtualProtectEx(h,allocation,new UIntPtr(0x4000),0x20,out previous)||!FlushInstructionCache(h,allocation,new UIntPtr(0x4000)))throw new InvalidOperationException("Formation Subs code protection failed.");
                    if(!VirtualProtectEx(h,new IntPtr(remote+0x5000),new UIntPtr(0x3000),0x20,out previous)||!FlushInstructionCache(h,new IntPtr(remote+0x5000),new UIntPtr(0x3000)))throw new InvalidOperationException("Formation preview code protection failed.");
                    List<ControllerChange> changes=new List<ControllerChange>();
                    changes.Add(new ControllerChange(b+0xb2e82e0+0xa0,BitConverter.GetBytes(b+0x2b10120),BitConverter.GetBytes(remote+0x7000)));
                    changes.Add(new ControllerChange(b+0xbae21b8+0x1a8,BitConverter.GetBytes(b+0x7b5f6a0),BitConverter.GetBytes(remote+0x5000)));
                    for(int i=0;i<slots.Length;i++)changes.Add(new ControllerChange(b+0xb3324d0+slots[i],BitConverter.GetBytes(b+FormationSubsOriginals[kinds[i]]),BitConverter.GetBytes(remote+kinds[i]*0x800)));
                    // Twenty bytes spanning complete SUB/CMP/JE/ADD instructions.
                    // Every other caller runs the exact native lookup clone. The
                    // original menu CALL and its caller/stack guard stay intact.
                    byte[] lookupEntry=Pad(Jump(remote+0x1000),20);
                    changes.Add(new ControllerChange(b+0x3479750,Sub(lookupNative,0,20),lookupEntry));
                    using(new ControllerFrozen(game,changes)){
                        foreach(var check in originals)VerifyBytes(h,check.Address,Convert.FromBase64String(check.Bytes),"Formation Subs pre-commit contract");
                        foreach(var change in changes)VerifyBytes(h,change.Address,change.Before,"Formation Subs pre-commit owner");
                        int applied=0;
                        try{foreach(var change in changes){published=true;applied++;Patch(h,change.Address,change.After);}}
                        catch(Exception failure){
                            Exception rollback=null;
                            for(int i=applied-1;i>=0;i--){try{Patch(h,changes[i].Address,changes[i].Before);}catch(Exception ex){rollback=ex;}}
                            if(rollback!=null)throw new InvalidOperationException("Formation Subs installation failed and rollback was incomplete. Close the game before retrying. "+rollback.Message,failure);
                            throw;
                        }
                    }
                    foreach(var change in changes)checks.Add(new OfflineDynastySettings.MemoryCheck(change.Address,change.After));
                    // Keep the original dependencies verified, substituting only
                    // the owned lookup entry. Its full native clone is also checked.
                    foreach(var check in originals){
                        byte[] data=Convert.FromBase64String(check.Bytes);
                        if(check.Address==b+0x3479750)Array.Copy(lookupEntry,0,data,0,lookupEntry.Length);
                        checks.Add(new OfflineDynastySettings.MemoryCheck(check.Address,data));
                    }
                    checks.Add(new OfflineDynastySettings.MemoryCheck(b+0xb3324d0+0xc0,BitConverter.GetBytes(b+0x2f24d50)));
                    checks.Add(new OfflineDynastySettings.MemoryCheck(b+0xb3324d0+0x520,BitConverter.GetBytes(b+0x2f25700)));
                    checks.Add(new OfflineDynastySettings.MemoryCheck(b+0xda1dda0,BitConverter.GetBytes(b+0xb5757a8)));
                    formationSubsChecks=checks;formationSubsStart=start;
                    Log(null,"Kickoff and Kickoff Return Formation Subs ready for the supported offline Dynasty hub. Code=0x"+remote.ToString("x"));
                }catch(Exception ex){formationSubsUnavailableStart=start;Log(null,"Kickoff/return Formation Subs unavailable; other settings remain active. "+ex.ToString());}
                finally{
                    // Never free code that another thread might have observed,
                    // even after successful rollback. Published blocks live to exit.
                    if(allocation!=IntPtr.Zero&&!published)FormationSubsFree(h,allocation,UIntPtr.Zero,0x8000);
                    if(h!=IntPtr.Zero)CloseHandle(h);
                }
            }
        }
        internal static List<OfflineDynastySettings.MemoryCheck> FormationSubsChecks(){lock(Sync)return formationSubsStart==installedStart&&formationSubsChecks!=null?new List<OfflineDynastySettings.MemoryCheck>(formationSubsChecks):new List<OfflineDynastySettings.MemoryCheck>();}
        internal static void VerifyFormationSubs(int pid){
            lock(Sync){if(formationSubsStart!=installedStart||formationSubsChecks==null)return;
                IntPtr h=OpenProcess(0x1010,false,pid);if(h==IntPtr.Zero)throw new InvalidOperationException("Could not verify Formation Subs.");
                try{foreach(var check in formationSubsChecks)VerifyBytes(h,check.Address,Convert.FromBase64String(check.Bytes),"Formation Subs ownership");}finally{CloseHandle(h);}
            }
        }
        [DllImport("kernel32.dll",EntryPoint="VirtualFreeEx",SetLastError=true)]private static extern bool FormationSubsFree(IntPtr process,IntPtr address,UIntPtr size,uint type);
    }
}
