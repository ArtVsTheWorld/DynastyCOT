using System;
using System.IO;
using OfflineDynastySettings;
namespace DynastyPlayClock {
    public sealed partial class DynastyPlayClockAction {
        private static void SeedDynastyClocks(byte[] data) {
            var profiles=DynastyClockProfiles.Load(Path.GetDirectoryName(typeof(DynastyPlayClockAction).Assembly.Location));
            byte[] table=DynastyClockProfiles.Encode(profiles);
            Array.Copy(table,0,data,DynastyClockProfiles.TableOffset,table.Length);
        }
        // Inline, callback-local lookup. Preserve flags and all scratch registers;
        // EAX is the only output. Each record is one aligned atomic 64-bit pair.
        private static void AppendDynastyClockRead(Code c,long image,long data,bool defense,string p,bool excludePractice=true) {
            c.Hex("9c51524150415141524153");
            c.Hex("41b9");c.Int(-1);
            if(excludePractice){
                c.MovR10(image+0xd9eb808);c.Hex("41833a0e");c.Branch("0f85",p+"done");
                c.MovR10(image+0xf3fddbc);c.Hex("41833a03");c.Branch("0f85",p+"done");
            }else AppendDynastyClockMenuScope(c,image,p);
            c.MovR10(image+0xf55c998);c.Hex("418b1285d2");c.Branch("0f84",p+"done");
            c.Hex("83faff");c.Branch("0f84",p+"done");
            if(excludePractice){
            c.MovR10(image+0xf63f9f0);c.Hex("4d8b124d85d2");c.Branch("0f84",p+"scopeReady");
            c.Hex("4180ba8400000000");c.Branch("0f85",p+"done");
            c.Label(p+"scopeReady");
            }
            c.Hex("41b9");c.Int(defense?15:1);
            c.MovR10(data+DynastyClockProfiles.TableOffset);c.Hex("41b8");c.Int(DynastyClockProfiles.Capacity);
            c.Label(p+"loop");c.Hex("4d8b1a4139d3");c.Branch("0f84",p+"found");
            c.Hex("4983c20841ffc8");c.Branch("0f85",p+"loop");c.Branch("e9",p+"done");
            c.Label(p+"found");c.Hex(defense?"49c1eb28":"49c1eb20");c.Hex("4589d94181e1ff000000");
            c.Label(p+"done");c.Hex("4489c8415b415a415941585a599d");
        }
        private static void AppendDynastyClockWrite(Code c,long image,long data,bool defense,string p,bool validated=false) {
            c.Hex("9c5051524150415141524153");c.Hex("4189c1");
            if(!validated&&defense) {
                foreach(int seconds in new int[]{0,3,5,15,20,30,45,60}) {
                    c.Hex("83f8");c.Bytes(new byte[]{(byte)seconds});c.Branch("0f84",p+"valid");
                }
                c.Branch("e9",p+"done");c.Label(p+"valid");
            } else if(!validated) {c.Hex("83f803");c.Branch("0f87",p+"done");}
            // Called only after the native settings adapter's offline check.
            // The Dynasty hub has no match mode; do not require gameplay here.
            AppendDynastyClockMenuScope(c,image,p);
            c.Hex("41833aff");c.Branch("0f84",p+"menuIdentity");
            c.MovR10(image+0xf3fddbc);c.Hex("41833a03");c.Branch("0f85",p+"done");
            c.Label(p+"menuIdentity");
            c.MovR10(image+0xf55c998);c.Hex("418b1285d2");c.Branch("0f84",p+"done");
            c.Hex("83faff");c.Branch("0f84",p+"done");
            c.MovR10(image+0xd9eb808);c.Hex("41833aff");c.Branch("0f84",p+"scopeReady");
            c.MovR10(image+0xf63f9f0);c.Hex("4d8b124d85d2");c.Branch("0f84",p+"scopeReady");
            c.Hex("4180ba8400000000");c.Branch("0f85",p+"done");
            c.Label(p+"scopeReady");
            // Try once: do not block the UI or spin over a concurrent writer.
            c.MovR10(data+DynastyClockProfiles.LockOffset);c.Hex("33c041bb01000000f0450fb11a");c.Branch("0f85",p+"done");
            c.MovR10(data+DynastyClockProfiles.TableOffset);c.Hex("b9");c.Int(DynastyClockProfiles.Capacity);
            c.Label(p+"loop");c.Hex("4d8b1a4139d3");c.Branch("0f84",p+"record");
            c.Hex("4d85db");c.Branch("0f84",p+"new");
            c.Hex("4983c208ffc9");c.Branch("0f85",p+"loop");c.Branch("e9",p+"release");
            c.Label(p+"new");c.Hex("4189d348b8");c.Long(0x00000f0100000000L);c.Hex("4909c3");
            c.Label(p+"record");
            c.Hex("48b8");c.Long(defense?unchecked((long)0xffff00ffffffffffUL):unchecked((long)0xffffff00ffffffffUL));
            c.Hex("4921c3");c.Hex(defense?"49c1e128":"49c1e120");c.Hex("4d09cb4d89d0");
            // Recheck identity immediately before publication.
            c.MovR10(image+0xf55c998);c.Hex("413912");c.Branch("0f85",p+"release");
            c.MovR10(data+DynastyClockProfiles.SequenceOffset);c.Hex("f041ff024d8918f041ff02");
            c.Label(p+"release");c.MovR10(data+DynastyClockProfiles.LockOffset);c.Hex("41c70200000000");
            c.Label(p+"done");c.Hex("415b415a415941585a59589d");
        }
        internal static byte[] ExportDynastyRead(long image,long data,bool defense) {
            Code c=new Code();AppendDynastyClockRead(c,image,data,defense,"test");c.Hex("c3");return c.Finish();
        }
        private static void AppendDynastyClockMenuScope(Code c,long image,string p) {
            c.MovR10(image+0xd9eb808);c.Hex("41833a0e");c.Branch("0f84",p+"menuScopeReady");
            c.Hex("41833aff");c.Branch("0f85",p+"done");
            c.Label(p+"menuScopeReady");
        }
        internal static byte[] ExportDynastyWrite(long image,long data,bool defense) {
            Code c=new Code();c.Hex("8bc1");AppendDynastyClockWrite(c,image,data,defense,"test");c.Hex("c3");return c.Finish();
        }
    }
}
