// Reviewed old-build native contracts. No whole-EXE allowlist.
namespace DynastyPlayClock { public sealed partial class DynastyPlayClockAction {
 private static readonly Region[] FormationSubsRegions = {
  new Region("Formation list and fixed frame",0x3760960,0x4a8,"002e01540fa59457d5747e46824821fd9c017a9dd34e7bd37249e398d4ce94a5"),
  new Region("Set list and fixed frame",0x37606d0,0x290,"9571fa45df401d989f1354204249af4de00da85b8547c1fc9c532b2ffe95bde4"),
  new Region("Formation Subs owning refresh",0x7fdd1d0,0x128,"d7ddc895c3208e975c5067afb6228c5aa7dfcce428d8920a1adcd098bd144326"),
  new Region("Formation Subs initial list getter",0x7fc9d00,0x5a,"30561c1b32a2c6aabfa6de23e8330f78d3c17ed7d6ef3673e6b1174b9c18edb8"),
  new Region("Formation Subs lazy list owner",0x7fe30c0,0xe5,"0f0e07884a3e6665a35f69ec4ea3ab62e158e04b3d24e6521f6b2a263133955c"),
  new Region("Formation Subs owning set refresh",0x7fc9d60,0x61,"93f97eab87b23b22eecbed6d147ecd498fc5973052ad3f7b369b6a8f80ccd535"),
  new Region("Native formation count",0x3489030,0x45,"385933cf506533851161cce1dd151cf910117defced6196ba53c44d34f443f5d"),
  new Region("Native formation item",0x3488850,0xd7,"3c45e629c8f0a7ad135dc25c219ddddec7426d5480c444ee7966914c5c072553"),
  new Region("Native type getter",0x2f24d50,0x12,"9face4739169090ef60c4f7c082ca79bf4ec00271fc081ff8fbd39d0314e7ad9"),
  new Region("Native ordinal to formation ID",0x34797f0,0x94,"e721f25fcc9cb19fe05e38aaadf0b294c39300d6a0ef27bd37f02c0bea7d749f"),
  new Region("Native formation lookup",0x3479750,0xa0,"a507b9199506fc4bb54cf724a6f8b4b4c0d32fc076680a960b2a8dcf9ec718b9"),
  new Region("Native set count",0x34776b0,0x80,"fa9fcf0e169c5a49612f25d8ba243a5bc0a0c45a22ae458edf13e84088022f2b"),
  new Region("Native set lookup",0x3479b80,0xc2,"e663d1e017fdc2a5a1fec361dcc9ba0b7ce47f7edd894691f555c7a642bafe36"),
  new Region("Native play count",0x3476e60,0xbb,"1403ce9f421f890cdd12ab925d2ff8c8a6bf704a90a48e1c730d4c0df35654ad"),
  new Region("Native metadata vector copy",0x347e1a0,0x34c,"f1e859971fc9654048d53f38b62740b159dba26037b24bec1062fc09a7e03140"),
  new Region("Native formation copy",0x347f1e0,0x90,"c4d9f611f9f63d8db51fa40d3305d282b8c91ae551dfbbdc7d0616e308d334f2"),
  new Region("Native empty set result",0x347e620,0x8,"66dcb28d08f68c6d078c746fa169024ed29ee8edf628da994924f22fa5826ee5"),
  new Region("Native metadata constructor",0x35eb6c0,0x165,"386b46464a5d85526a98bb49a9ed31b62b89cfc238cc368104c359617c8e9250"),
  new Region("Formation Subs native commands",0x7fcfd50,0x698,"7806ea147ce1687bcece8557d0c433728bf773c329ed10f3b8bb44ad52f8587e"),
  new Region("Native formation-count core",0x3476cc0,0x1a0,"6e6e21b968ef8adfd2e80d2ea247777cc7ca6186e38c60aaccc5644f86ef9c30"),
  new Region("Native count-filter selection",0x2f25700,0x2c,"884cef0f530cf4eceace3c30d82aaf3e81dbac8074e26d39f0d47670fc8793fc"),
 };
 private static readonly Region[] FormationPreviewRegions = {
  new Region("Native set display name",0x2b10120,0x1e,"8e8b6b35cdfc93760102d4f4c4cdf7df364a7bf937f110cfc719ccad8ee1b922"),
  new Region("Formation preview producer",0x7b5f6a0,0x215,"3dc6596fe3b5196367f418d96c7b93adcd97f02b1505996b301e12ebb9fa77dc"),
  new Region("Formation preview binding",0x7b431c0,0xac,"b23f22daf40ba22ddfe2aeefa9d7dd102dd49997927f1fb6855d2e01af57afd0"),
  new Region("Formation preview registration",0x7b42200,0x34,"1e477eb1226883ce1b4b1cda55b245cce519d6ca0e6db6c460c91e1da6bdec5e"),
 };
 private static readonly byte[] FormationLookupCall=Hex("e8d78fd1ff");
} }
