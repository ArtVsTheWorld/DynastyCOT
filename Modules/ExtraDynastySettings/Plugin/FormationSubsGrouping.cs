namespace DynastyPlayClock { public sealed partial class DynastyPlayClockAction {
    // Native enum IDs, not team/book identities. Require the current book's
    // matching typed metadata and canonical name before merging any group.
    private const int GroupedSpecialTeamsIndex=0x20000000;
    private static byte[] BuildFormationGroup(long image,long remote)
    {
        Code c=new Code();c.Hex("4883ec2883fa01");c.Branch("0f84","offense");
        c.Hex("83fa0b");c.Branch("0f85","missing");c.Hex("41b814000000");c.Branch("e9","find");
        c.Label("offense");c.Hex("41b80c000000");c.Label("find");c.Call(remote+0x3800);
        c.Hex("4885c0");c.Branch("0f84","missing");
        c.Hex("44398010030000");c.Branch("0f85","missing");
        c.Hex("399018030000");c.Branch("0f85","missing");
        c.Hex("81b81403000000010000");c.Branch("0f83","missing");
        c.Hex("4c8d90f802000041807a0f00");c.Branch("0f8c","heap");
        c.Hex("41807a0f02");c.Branch("0f85","missing");c.Branch("e9","name");
        c.Label("heap");c.Hex("41837a080d");c.Branch("0f85","missing");c.Hex("4d8b124d85d2");c.Branch("0f84","missing");
        c.Label("name");c.Hex("49bb5370656369616c204d391a");c.Branch("0f85","missing");
        c.Hex("41817a085465616d");c.Branch("0f85","missing");
        c.Hex("41807a0c73");c.Branch("0f85","missing");c.Branch("e9","done");
        c.Label("missing");c.Hex("33c0");c.Label("done");c.Hex("4883c428c3");return c.Finish();
    }

    // Resolve a UI-only concatenation of ordinary ST sets and real kickoff/
    // return sets. Out = {type, formation ordinal, set ordinal, total count}.
    // Nothing in the native library, set metadata or persisted keys is changed.
    private static byte[] BuildGroupedFormationSet(long image,long remote)
    {
        Code c=new Code();
        c.Hex("5355565741544155415641574883ec48488bd98bf2418bf8458be1488bac24b0000000");
        c.Hex("488d8b181d00008bd6");c.Call(remote+0x6100);c.Hex("4885c0");c.Branch("0f84","missing");
        c.Hex("39b814030000");c.Branch("0f85","missing");
        c.Hex("488d8b181d000033d2448bc7448bce");c.Call(image+0x34776b0);
        c.Hex("83f820");c.Branch("0f87","missing");
        c.Hex("89450c897500897d0444896508");
        c.Hex("4183fcff");c.Branch("0f84","countMode");
        c.Hex("443be0");c.Branch("0f82","found");
        c.Hex("458bfc442bf8");c.Branch("e9","extraCount");
        c.Label("countMode");c.Hex("41bfffffffff");
        c.Label("extraCount");c.Hex("488bcb33d2448d460183fe0b");c.Branch("0f85","countArgs");
        c.Hex("48ba0000000001000000");c.Label("countArgs");c.Call(image+0x3489030);c.Hex("448be8");
        c.Hex("83f808");c.Branch("0f87","missing");c.Hex("4585ed");c.Branch("0f84","finish");
        c.Hex("4533f6");c.Label("extra");
        c.Hex("488d8b181d000033d2448d4601458bce488d4424304889442420");c.Call(image+0x34797f0);
        c.Hex("84c0");c.Branch("0f84","missing");
        c.Hex("488d8b181d00008d5601448b442430");c.Call(remote+0x3800);
        c.Hex("4885c0");c.Branch("0f84","missing");
        c.Hex("4439b014030000");c.Branch("0f85","missing");
        c.Hex("488d8b181d00008bd6448b442430");c.Call(remote+0x3800);
        c.Hex("4885c0");c.Branch("0f85","missing");
        c.Hex("488d8b181d000033d2458bc6448d4e01");c.Call(image+0x34776b0);
        c.Hex("83f820");c.Branch("0f87","missing");c.Hex("01450c4585ff");c.Branch("0f88","next");
        c.Hex("443bf8");c.Branch("0f83","skip");
        c.Hex("8d46018945004489750444897d0841bffeffffff");c.Branch("e9","next");
        c.Label("skip");c.Hex("442bf8");c.Label("next");c.Hex("41ffc6453bf5");c.Branch("0f82","extra");
        c.Label("finish");c.Hex("4183fcff");c.Branch("0f84","found");
        c.Hex("4183fffe");c.Branch("0f85","missing");
        c.Label("found");c.Hex("b801000000");c.Branch("e9","done");
        c.Label("missing");c.Hex("33c0");c.Label("done");
        c.Hex("4883c448415f415e415d415c5f5e5d5bc3");return c.Finish();
    }

    private static void GroupedFormationDispatch(Code c,int kind,long image,long remote)
    {
        c.Hex("81ff00000020");c.Branch("0f82","singleGroup");
        c.Hex("81ff00010020");c.Branch("0f83","savedNative");
        c.Hex("81e7ffffff0f488bcb418bd5448bc7");
        if(kind==3)c.Hex("41b9ffffffff");else c.Hex("458bcc");
        c.Hex("488d4424304889442420");c.Call(remote+0x6500);
        c.Hex("84c0");c.Branch("0f84","savedNative");
        FormationSubsCount(c,remote,kind,8);
        if(kind==3)c.Hex("8b44243c");
        else if(kind==4){
            c.Hex("8b44243889442420488d8b181d00008bd6448b442430448b4c2434");c.Call(image+0x3479b80);
        }else{
            c.Hex("8b44243089442420488d8b181d00008bd6448b442434448b4c2438");c.Call(image+0x3476e60);
        }
        c.Branch("e9","return");c.Label("singleGroup");
    }
} }
