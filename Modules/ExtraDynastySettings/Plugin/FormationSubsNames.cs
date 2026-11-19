// Display aliases for native kickoff sets; source names, IDs and saves stay native.
namespace DynastyPlayClock { public sealed partial class DynastyPlayClockAction {
    private static byte[] BuildFormationSetName(long image,long remote)
    {
        Code c=new Code();
        c.MovR10(image+0x3760895);c.Hex("4c391424");c.Branch("0f85","native");
        c.MovR10(image+0x7fc9d92);c.Hex("4c399424a0030000");c.Branch("0f85","native");
        FormationPreviewScope(c,image);
        c.MovR10(image+0xb2e82e0);c.Hex("4c3911");c.Branch("0f85","native");
        c.Hex("534883ec20488bd9"); // Preserve native set receiver and align calls.
        c.Call(image+0x2b10120); // Preserve any existing meaningful native name.
        c.Hex("48ba6e6f206e616d6500483910");c.Branch("0f85","existingName");
        foreach(int type in new int[]{2,12}){
            c.MovR10(image+0xe2d3918);c.Hex("498b0a4881c1181d0000488bd3");
            c.Hex("41b8");c.Int(type);c.Call(remote+0x7400);c.Hex("84c0");c.Branch("0f85","type"+type);
        }
        c.Hex("488bcb4883c4205b");c.Branch("e9","native");
        foreach(int type in new int[]{2,12}){
            c.Label("type"+type);FormationSubsCount(c,remote,7,8);
            c.Hex("48b8");c.Long(remote+(type==2?0x7800:0x7820));c.Hex("4883c4205bc3");
        }
        c.Label("existingName");c.Hex("4883c4205bc3");
        c.Label("native");c.Bytes(Jump(image+0x2b10120));return c.Finish();
    }
    // RCX current core, RDX exact native set pointer, R8D required type.
    // A name or coincident numeric set ID alone never grants an alias.
    private static byte[] BuildTypedSetMembership(long image,long remote)
    {
        Code c=new Code();c.Hex("53555741544155415641574883ec20488bd9488bea");
        c.Hex("488b8b200300004489c2");c.Call(remote+0x5b00);
        c.Hex("4885c0");c.Branch("0f84","missing");
        c.Hex("83784808");c.Branch("0f87","missing");
        c.Hex("8b584885db");c.Branch("0f84","missing");
        c.Hex("4c8b70384533e4");c.Label("form");
        c.Hex("498bce418bd4");c.Call(remote+0x5b00);c.Hex("4885c0");c.Branch("0f84","nextForm");
        c.Hex("83784820");c.Branch("0f87","missing");
        c.Hex("8b784885ff");c.Branch("0f84","nextForm");
        c.Hex("4c8b78384533ed");c.Label("set");
        c.Hex("498bcf418bd5");c.Call(remote+0x5b00);c.Hex("4885c0");c.Branch("0f84","nextSet");
        c.MovR10(image+0xb2e82e0);c.Hex("4c395028");c.Branch("0f85","missing");
        c.Hex("488d50284839ea");c.Branch("0f84","found");
        c.Label("nextSet");c.Hex("41ffc5443bef");c.Branch("0f82","set");
        c.Label("nextForm");c.Hex("41ffc4443be3");c.Branch("0f82","form");
        c.Label("missing");c.Hex("33c0");c.Branch("e9","done");
        c.Label("found");c.Hex("b801000000");c.Label("done");
        c.Hex("4883c420415f415e415d415c5f5d5bc3");return c.Finish();
    }
} }
