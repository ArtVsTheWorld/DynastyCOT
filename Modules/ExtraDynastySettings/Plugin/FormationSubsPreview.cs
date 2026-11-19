using System;

namespace DynastyPlayClock { public sealed partial class DynastyPlayClockAction {
    // This is an invocation-local presentation proxy, never a native formation
    // edit. The verified producer writes only float (+28) and string (+30)
    // fields; the consumer reads its completed map after the producer returns.
    // Delay only its eleven y fields so an over-deep return can be fitted as a
    // group. Preserve x, slot order, labels, and all native submission data.
    private static byte[] BuildFormationPreview(long image,long remote)
    {
        Code c=new Code();
        c.MovR10(image+0x7b43251);c.Hex("4c391424");c.Branch("0f85","native");
        c.Hex("4584c9");c.Branch("0f85","native"); // Defense presentation only.
        c.MovR10(image+0xda1dda0);c.Hex("4c39d2");c.Branch("0f85","native");
        c.MovR10(image+0xfa5ea78);c.Hex("49390a");c.Branch("0f85","native");
        c.MovR10(image+0xbae21b8);c.Hex("4c3911");c.Branch("0f85","native");
        FormationPreviewScope(c,image);
        FormationSubsCount(c,remote,6,0);
        // Seven saved registers, then 256 bytes: aligned at every native call.
        c.Hex("53565741544155415641574881ec00010000488bd9488bf2418bf8458be14c8bac2460010000");
        c.MovR10(image+0xe2d3918);c.Hex("498b0a4881c1181d00008bd7");
        c.Call(remote+0x5800);c.Hex("84c0");c.Branch("0f84","savedNative");
        FormationSubsCount(c,remote,6,4);
        c.MovR10(remote+0x6000);c.Hex("4c895424404889742448c744245800000000");
        // Original fifth argument is forwarded unchanged, including flip state.
        c.Hex("4c896c2420488bcb488d542440448bc7458bcc");c.Call(image+0x7b5f6a0);
        c.Hex("48898424f0000000817c2458ff070000");c.Branch("0f85","flush");
        c.Hex("f30f10442460f30f10c833ff");c.Label("bounds");
        c.Hex("8b44bc6025ffffff7f3d00008044");c.Branch("0f87","flush"); // Nonfinite/implausible map coordinates: retain native values.
        c.Hex("f30f1054bc60f30f5dc2f30f5fcaffc783ff0b");c.Branch("0f82","bounds");
        c.Hex("0f28e9f30f5cc8b800002042660f6ee00f2fcc");c.Branch("0f86","translate"); // 40 map units of depth.
        c.Hex("f30f5ee133ff");c.Label("fit");
        c.Hex("f30f1054bc60f30f5cd5f30f59d4f30f58d5f30f1154bc60ffc783ff0b");c.Branch("0f82","fit");
        c.Label("translate");c.Hex("b800000041660f6ee033ff");c.Label("offset");
        c.Hex("f30f1054bc60f30f58d4f30f1154bc60ffc783ff0b");c.Branch("0f82","offset");
        FormationSubsCount(c,remote,6,8);
        c.Label("flush");c.Hex("33ff");c.Label("field");
        c.Hex("0fa37c2458");c.Branch("0f83","next");
        c.Hex("488bce8bd7");c.Hex("49b8");c.Long(image+0xad964e4);
        c.Hex("f30f105cbc60488b06ff5028");
        c.Label("next");c.Hex("ffc783ff0b");c.Branch("0f82","field");
        c.Hex("488b8424f0000000");PreviewPop(c);c.Hex("c3");
        c.Label("savedNative");c.Hex("488bcb488bd6448bc7458bcc4c89ac2460010000");PreviewPop(c);
        c.Label("native");c.Bytes(Jump(image+0x7b5f6a0));return c.Finish();
    }
    private static void PreviewPop(Code c){c.Hex("4881c400010000415f415e415d415c5f5e5b");}

    private static void FormationPreviewScope(Code c,long image)
    {
        c.MovR10(image+0xd9eb808);c.Hex("41833aff");c.Branch("0f85","native");
        c.MovR10(image+0xf55c998);c.Hex("41833a00");c.Branch("0f84","native");
        c.Hex("41833aff");c.Branch("0f84","native");
        c.MovR10(image+0xf3fddbc);c.Hex("41833a03");c.Branch("0f85","native");
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
        c.MovR10(image+0xf63f9f0);c.Hex("4d8b124d85d2");c.Branch("0f84","scopeReady");
        c.Hex("4180ba8400000000");c.Branch("0f85","native");c.Label("scopeReady");
        c.MovR10(image+0xe2d3918);c.Hex("4d8b124d85d2");c.Branch("0f84","native");
        c.Hex("49bb");c.Long(image+0xb3324d0);c.Hex("4d391a");c.Branch("0f85","native");
    }

    // Bounded searches of the current library, not a hardcoded set-ID allowlist.
    // Native type 12 must own the set in the selected playbook.
    private static byte[] BuildReturnSetMembership(long image,long remote)
    {
        Code c=new Code();c.Hex("53555741544155415641574883ec20488bd98bea");
        c.Hex("488b8b20030000ba0c000000");c.Call(remote+0x5b00);
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
        c.Hex("39a848020000");c.Branch("0f84","found");
        c.Label("nextSet");c.Hex("41ffc5443bef");c.Branch("0f82","set");
        c.Label("nextForm");c.Hex("41ffc4443be3");c.Branch("0f82","form");
        c.Label("missing");c.Hex("33c0");c.Branch("e9","done");
        c.Label("found");c.Hex("b801000000");c.Label("done");
        c.Hex("4883c420415f415e415d415c5f5d5bc3");return c.Finish();
    }
    private static byte[] BuildPreviewTreeFind()
    {
        Code c=new Code();c.Hex("41ba40000000");c.Label("loop");c.Hex("4885c9");c.Branch("0f84","missing");
        c.Hex("41ffca");c.Branch("0f84","missing");
        c.Hex("3b5120");c.Branch("0f84","found");c.Branch("0f8c","left");
        c.Hex("488b09");c.Branch("e9","loop");c.Label("left");c.Hex("488b4908");c.Branch("e9","loop");
        c.Label("found");c.Hex("488bc1c3");c.Label("missing");c.Hex("33c0c3");return c.Finish();
    }
    private static byte[] BuildPreviewFloat(long image)
    {
        Code c=new Code();c.MovR10(image+0xad964e4);c.Hex("4d39d0");c.Branch("0f85","native");
        c.Hex("83fa0a");c.Branch("0f87","native");
        c.Hex("f30f115c91200fab5118c3"); // y[index], present bit; no shared scratch memory.
        c.Label("native");c.Hex("488b4908488b01ff6028");return c.Finish();
    }
    private static byte[] BuildPreviewString(){return Hex("488b4908488b01ff6030");}
} }
