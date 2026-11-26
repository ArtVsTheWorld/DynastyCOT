// Reviewed November-update native practice-only injury contracts.
#pragma warning disable 1591
namespace DynastyPlayClock { public sealed partial class DynastyPlayClockAction {
 private const long PracticeRollSite=0x828f0e3,PracticeRollCaller=0x8152926,PracticeNoInjury=0x828f2ef;
 private const long PracticeRiskSite=0x828ddb0,PracticeRiskCaller=0x887b700;
 private static readonly byte[] PracticeRollOriginal=Hex("c5f82ff0c5f828b424000100000f86f9010000");
 private static readonly byte[] PracticeRiskOriginal=Hex("4053555657415641574881ecc8000000");
 private static readonly Region[] PracticeInjuryRegions={
  new Region("Practice injury generation",0x828edf0,0x56a,"1261140c4aabf4e937e10e9cd4087ff787aa6bf86771866c13db570df794ef2f"),
  new Region("Practice injury risk preview",0x828ddb0,0x239,"d0280c525251389c76af405c6e2dacacc73846e8576af09a3026c2a9732774d1"),
  new Region("Weekly practice application",0x8152320,0x880,"115187e03df8a22d3cb3f12a71d5b33b219045929165eaccbcb06ace95c8cec6"),
  new Region("Practice risk player-position capture",0x887b329,0x2a,"febd3167587024ff3e75f12ce4c7c4510d6fa42486e4cda9c6d4d89a0038bbce"),
  new Region("Practice risk call",0x887b6bd,0x71,"b6fabb381c710eb2d8745acc2745d8826439f81cda2212e5b97b2662b06dc82d"),
 };
} }
