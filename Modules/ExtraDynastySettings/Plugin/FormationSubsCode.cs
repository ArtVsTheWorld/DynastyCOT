using System;

namespace DynastyPlayClock
{
    public sealed partial class DynastyPlayClockAction
    {
        // A transient lookup index, never a FormationSetID, PlayerRef or saved value.
        // The provider publishes the original formation/set IDs after these lookups.
        private const int SpecialFormationIndex = 0x10000000;
        private static readonly long[] FormationSubsOriginals = {0x3489030,0x3488850,0x3479750,0x3489720,0x348c720,0x3489380};

        private static byte[] BuildFormationSubsBridge(int kind,long image,long remote)
        {
            if(kind<0||kind>=6)throw new ArgumentOutOfRangeException("kind");
            Code c=new Code();
            // Exact immediate AND enclosing callers. These library methods also serve
            // other menus. Never change those other callers, including practice.
            long[] callers;int[] frames;
            if(kind==0){callers=new long[]{0x37609f4};frames=new int[]{0x3e0};}
            else if(kind==1){callers=new long[]{0x3760a63};frames=new int[]{0x3e0};}
            else if(kind==2){callers=new long[]{0x3760779};frames=new int[]{0x3a0};}
            else if(kind==3){callers=new long[]{0x3760abf,0x3760799};frames=new int[]{0x3e0,0x3a0};}
            else if(kind==4){callers=new long[]{0x3760afd,0x37607c8};frames=new int[]{0x3e0,0x3a0};}
            else {callers=new long[]{0x3760b52,0x37607f5};frames=new int[]{0x3e0,0x3a0};}
            for(int i=0;i<callers.Length;i++){
                c.MovR10(image+callers[i]);c.Hex("4c391424");c.Branch("0f85","callerNext"+i);
                c.MovR10(image+(frames[i]==0x3e0?0x7fdd27f:0x7fc9d92));
                c.Hex("4c399424");c.Int(frames[i]);c.Branch("0f84","scope");
                if(frames[i]==0x3e0){
                    // The initial lazy-list getter and side-change refresh are
                    // distinct native owners of this same builder/frame.
                    c.MovR10(image+0x7fc9d2b);c.Hex("4c399424");c.Int(frames[i]);c.Branch("0f84","scope");
                }
                c.Branch("e9","native");c.Label("callerNext"+i);
            }
            c.Branch("e9","native");c.Label("scope");
            FormationSubsCount(c,remote,kind,0); // Verified menu caller reached.
            // Menu scope only: an active, offline Dynasty, no active match mode.
            c.MovR10(image+0xd9eb808);c.Hex("41833aff");c.Branch("0f85","native");
            c.MovR10(image+0xf55c998);c.Hex("41833a00");c.Branch("0f84","native");
            c.Hex("41833aff");c.Branch("0f84","native");
            c.MovR10(image+0xf3fddbc);c.Hex("41833a03");c.Branch("0f85","native");
            // The match configuration is normally absent in the hub. Verify the
            // active career's owned local database binding instead of inferring
            // a Dynasty from a stale LeagueID or an absent online flag.
            c.MovR10(image+0xfa519b8);c.Hex("4d8b124d85d2");c.Branch("0f84","native");
            c.Hex("49bb");c.Long(image+0xbaa8938);c.Hex("4d391a");c.Branch("0f85","native");
            c.Hex("4180bab107000000");c.Branch("0f84","native");
            c.Hex("498b82680400004885c0");c.Branch("0f84","native");
            c.MovR10(image+0xbce9a78);c.Hex("4c3910");c.Branch("0f85","native");
            c.Hex("4c8b90300200004d85d2");c.Branch("0f84","native");
            c.Hex("49bb");c.Long(image+0xb4dbab8);c.Hex("4d391a");c.Branch("0f85","native");
            c.Hex("4c8b98400200004d85db");c.Branch("0f84","native");
            c.Hex("4d3913");c.Branch("0f85","native");
            c.Hex("4d8b5b084d85db");c.Branch("0f84","native");
            c.MovR10(image+0xbd0d2f8);c.Hex("4d3913");c.Branch("0f85","native");
            c.Hex("4c8b90380200004d85d2");c.Branch("0f84","native");
            c.Hex("4d3993a8010000");c.Branch("0f85","native");
            c.Hex("49bb");c.Long(image+0xbd0d258);c.Hex("4d391a");c.Branch("0f85","native");
            c.MovR10(image+0xf63f9f0);c.Hex("4d8b124d85d2");c.Branch("0f84","menuScopeReady");
            c.Hex("4180ba8400000000");c.Branch("0f85","native");c.Label("menuScopeReady");
            c.MovR10(image+0xe2d3918);c.Hex("4d8b124d85d2");c.Branch("0f84","native");
            if(kind==2)c.Hex("4981c2181d0000");
            c.Hex("4c39d1");c.Branch("0f85","native");
            // All supported provider paths own library slot zero. Do not decode
            // other/custom slots using a different object's indexing convention.
            c.Hex("85d2");c.Branch("0f85","native");
            if(kind==5){c.Hex("837c2430ff");c.Branch("0f85","native");}
            FormationSubsCount(c,remote,kind,4); // Ownership/context checks passed.

            // Seven nonvolatile registers plus 64 bytes; aligned before every call.
            c.Hex("53565741544155415641574883ec40488bd9488bf2498bf84d8be1");
            if(kind==2){c.Hex("448bef");} // Caller supplies verified ordinary type.
            else {
                c.Hex("488bcb8bd6");c.Call(image+0x2f24d50);c.Hex("448be8");
            }
            c.Hex("4183fd01");c.Branch("0f84","typeReady");
            c.Hex("4183fd0b");c.Branch("0f85","savedNative");c.Label("typeReady");
            if(kind==0){c.Hex("4439ef");c.Branch("0f85","savedNative");}
            if(kind==0||kind==1){
                // Match the native formation-list count's offense/defense flag.
                c.Hex("488bcb488bd64183fd0b");c.Branch("0f85","countArgs");
                c.Hex("48b800000000010000004809c2");c.Label("countArgs");
                c.Hex("458bc5");c.Call(image+0x3489030);c.Hex("448bf0");
                c.Hex("83f801");c.Branch("0f8c","savedNative");
                c.Hex("3d00010000");c.Branch("0f87","savedNative");
                if(kind==0){
                    c.Hex("488bcb488bd6458d45014183fd0b");c.Branch("0f85","extraCountArgs");
                    c.Hex("48b800000000010000004809c2");c.Label("extraCountArgs");
                    c.Call(image+0x3489030);c.Hex("448bf8");
                    c.Hex("83f808");c.Branch("0f87","ordinaryCount");
                    c.Hex("4585ff");c.Branch("0f84","ordinaryCount");
                    // Reject the entire extension if IDs would be ambiguous.
                    c.Hex("4533e4");c.Label("checkId");
                    c.Hex("488d8b181d00008bd6458d4501458bcc488d4424304889442420");
                    c.Call(image+0x34797f0);c.Hex("84c0");c.Branch("0f84","ordinaryCount");
                    c.Hex("488d8b181d0000418d5501448b442430");c.Call(remote+0x3800);
                    c.Hex("4885c0");c.Branch("0f84","ordinaryCount");
                    c.Hex("4439a014030000");c.Branch("0f85","ordinaryCount");
                    c.Hex("488d8b181d0000418bd5448b442430");c.Call(remote+0x3800);
                    c.Hex("4885c0");c.Branch("0f85","ordinaryCount");
                    c.Hex("41ffc4453be7");c.Branch("0f82","checkId");
                    FormationSubsCount(c,remote,kind,8);
                    c.Hex("488d8b181d0000418bd5");c.Call(remote+0x6100);
                    c.Hex("4885c0");c.Branch("0f84","separateCount");
                    c.Hex("418bc6");c.Branch("e9","return");
                    c.Label("separateCount");c.Hex("438d043e");c.Branch("e9","return");
                    c.Label("ordinaryCount");c.Hex("418bc6");c.Branch("e9","return");
                }else{
                    c.Hex("453be6");c.Branch("0f83","extraItem");
                    c.Hex("488d8b181d0000418bd5");c.Call(remote+0x6100);
                    c.Hex("4885c0");c.Branch("0f84","savedNative");
                    c.Hex("4439a014030000");c.Branch("0f85","savedNative");
                    c.Hex("488bcb488bd64c8bc7458bcc488b8424a00000004889442420");c.Call(image+0x3488850);
                    c.Hex("818f1403000000000020");c.Branch("e9","return");
                    c.Label("extraItem");c.Hex("452be64183fc08");c.Branch("0f83","savedNative");
                    c.Hex("488bcb8bd64c8bc74d8bccc7442420000000004183fd01");c.Branch("0f85","copyDefense");
                    c.Call(remote+0x3000);c.Branch("e9","copied");
                    c.Label("copyDefense");c.Call(remote+0x3400);c.Label("copied");
                    FormationSubsCount(c,remote,kind,8);
                    c.Hex("818f1403000000000010"); // OR only transient index +0x314.
                    c.Branch("e9","return");
                }
            }else if(kind==2){
                // First preserve the exact ordinary lookup. Only a missing ID can
                // be resolved from the matching native kickoff/return type.
                c.Hex("488bcb8bd6448bc74d8bcc488b8424a00000004889442420");
                c.Call(remote+0x3d00);c.Hex("84c0");c.Branch("0f85","ordinaryLookup");
                c.Hex("488bcb8bd6448d47014d8bcc488b8424a00000004889442420");
                c.Call(remote+0x3d00);c.Hex("84c0");c.Branch("0f84","return");
                FormationSubsCount(c,remote,kind,8);
                c.Hex("488b9424a0000000818a1403000000000010");c.Branch("e9","return");
                c.Label("ordinaryLookup");
                c.Hex("488bcb418bd5");c.Call(remote+0x6100);
                c.Hex("4885c0");c.Branch("0f84","ordinarySuccess");
                c.Hex("4439a010030000");c.Branch("0f85","ordinarySuccess");
                c.Hex("488b9424a0000000818a1403000000000020");
                c.Label("ordinarySuccess");c.Hex("b801000000");c.Branch("e9","return");
            }else{
                GroupedFormationDispatch(c,kind,image,remote);
                c.Hex("81ff00000010");c.Branch("0f82","savedNative");
                c.Hex("81ff08000010");c.Branch("0f83","savedNative");
                c.Hex("81e7ffffff0f");
                FormationSubsCount(c,remote,kind,8);
                if(kind==3){
                    c.Hex("488d8b181d00008bd6448bc7458d4d01");c.Call(image+0x34776b0);
                }else if(kind==4){
                    c.Hex("4489642420488d8b181d00008bd6458d4501448bcf");c.Call(image+0x3479b80);
                }else{
                    c.Hex("418d450189442420488d8b181d00008bd6448bc7458bcc");c.Call(image+0x3476e60);
                }
                c.Branch("e9","return");
            }
            c.Label("savedNative");c.Hex("488bcb488bd64c8bc74d8bcc");FormationSubsPop(c);
            c.Label("native");c.Bytes(Jump(kind==2?remote+0x3d00:image+FormationSubsOriginals[kind]));
            c.Label("return");FormationSubsPop(c);c.Hex("c3");return c.Finish();
        }
        private static void FormationSubsPop(Code c){c.Hex("4883c440415f415e415d415c5f5e5b");}
        // Private diagnostic counters live on a separate RW page. No scans,
        // allocations, disk work or text formatting run inside these callbacks.
        private static void FormationSubsCount(Code c,long remote,int kind,int field){c.MovR10(remote+0x4000+kind*0x20+field);c.Hex("f041ff02");}

        // Bounded read-only lookup in the native type -> formation-ID trees. Used
        // solely to reject ambiguous IDs, not as an alternative assignment store.
        private static byte[] BuildFormationSubsFind()
        {
            Code c=new Code();c.Hex("803900");c.Branch("0f84","missing");
            c.Hex("4881c1d00000004c8b491041ba40000000");
            c.Label("typeLoop");c.Hex("4d85c9");c.Branch("0f84","missing");
            c.Hex("41ffca");c.Branch("0f84","missing");
            c.Hex("413b5120");c.Branch("0f84","foundType");c.Branch("0f8c","typeLeft");
            c.Hex("4d8b09");c.Branch("e9","typeLoop");
            c.Label("typeLeft");c.Hex("4d8b4908");c.Branch("e9","typeLoop");
            c.Label("foundType");c.Hex("4d8b493841ba40000000");
            c.Label("idLoop");c.Hex("4d85c9");c.Branch("0f84","missing");
            c.Hex("41ffca");c.Branch("0f84","missing");
            c.Hex("453b4120");c.Branch("0f84","found");c.Branch("0f8c","idLeft");
            c.Hex("4d8b09");c.Branch("e9","idLoop");
            c.Label("idLeft");c.Hex("4d8b4908");c.Branch("e9","idLoop");
            c.Label("found");c.Hex("498d4128c3");c.Label("missing");c.Hex("33c0c3");return c.Finish();
        }

        private static byte[] CloneTypedFormationItem(byte[] native,long image,long remote,int type)
        {
            if(native==null||native.Length!=215||(type!=2&&type!=12))throw new ArgumentException("Typed formation clone contract");
            byte[] code=Pad(native,0x140);
            // Exact instruction sites in the fully fingerprinted native body.
            foreach(int at in new int[]{0x2e,0x5e,0x85}){
                if(!Equal(Sub(code,at,6),Hex("ff90c0000000")))throw new InvalidOperationException("Formation type call changed");
                code[at]=0xb8;Array.Copy(BitConverter.GetBytes(type),0,code,at+1,4);code[at+5]=0x90;
            }
            int stub=0x100;
            foreach(int at in new int[]{0x4d,0x78,0xa2,0xb9}){
                if(code[at]!=0xe8)throw new InvalidOperationException("Formation helper call changed");
                long target=image+0x3488850+at+5+BitConverter.ToInt32(code,at+1);
                // A local absolute-jump island preserves the original CALL/RET
                // stack and every register. No native-relative placement needed.
                if(target==image+0x3479750)target=remote-(type==2?0x3000:0x3400)+0x3d00;
                Array.Copy(BitConverter.GetBytes(stub-at-5),0,code,at+1,4);
                Array.Copy(Jump(target),0,code,stub,14);stub+=0x10;
            }
            return code;
        }
        private static byte[] CloneFormationLookup(byte[] native,long image){
            if(native==null||native.Length!=0xa0||native[0x8d]!=0xe8||image+0x3479750+0x92+BitConverter.ToInt32(native,0x8e)!=image+0x347f1e0)
                throw new ArgumentException("Formation lookup clone contract");
            // The fully verified method has no RIP-relative data and one external
            // call. Internal branch distances and original instruction positions
            // remain unchanged. Its local island leaves native argument/state intact.
            byte[] code=Pad(native,0xae);
            Array.Copy(BitConverter.GetBytes(0xa0-0x92),0,code,0x8e,4);
            Array.Copy(Jump(image+0x347f1e0),0,code,0xa0,14);return code;
        }
        private static byte[] Sub(byte[] source,int at,int count){byte[] result=new byte[count];Array.Copy(source,at,result,0,count);return result;}
    }
}
