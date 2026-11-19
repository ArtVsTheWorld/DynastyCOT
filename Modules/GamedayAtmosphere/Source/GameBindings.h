#pragma once
#include <cstdint>
// Native offsets for the inspected CFB27 layout. Signature checks are required;
// compatibility is not tied to an executable hash. Keep future binding changes
// here, separately from the configurable atmosphere rules.
namespace attendance::bindings {
// Read-only live carrier/animation tag layout; guarded type getter and asset identities.
inline constexpr uintptr_t ShowboatPlayerType=0x2ec6500,ShowboatPlayerVtable=0xb32ac48,ShowboatAudioTagVtable=0xadaad60,ShowboatEnumValueVtable=0xae4c578;
inline constexpr uintptr_t StartingAttendanceFormula=0x856a510,StartingAttendanceLaunch=0x90a1910,StartingAttendanceBoundary=0x7f18328;
// Native authored replay lifecycle: complete prologues are checked before hooking.
inline constexpr uintptr_t ReplayEventNodeUpdate=0x7dcb150,ReplayEventNodeVtable=0xbc8a8d8;
inline constexpr uintptr_t ReplayNodeUpdate=0x7dc2880,ReplayNodeVtable=0xbc89e08;
inline constexpr uintptr_t PresentationClient=0x7d6a840,PresentationEvent=0x7d6a870;
// Final native LegacySoundEntity event receiver and read-only Int32 variant accessor.
inline constexpr uintptr_t LegacySoundVtable=0xaf028b8,LegacySoundReceive=0x10fe6a0,VariantValue=0x5b2d250,Int32Type=0xf640ea0;
inline constexpr uintptr_t AudioUpdate=0x28492a0,CrowdQueue=0x3d424e0;
inline constexpr uintptr_t AttendanceUpdate=0x3635220;
inline constexpr uintptr_t CrowdVisibility=0x64b39c0,PeriodChange=0x5d06020,CrowdPartResolve=0x65083d0;
// Read-only placement lookup verified against the native agent*16 position path.
inline constexpr uintptr_t SceneCrowdVtable=0xb7e1b48,CrowdRendererVtable=0xb7e1928,CrowdPositionKey=0xb2283b8;
// GameStates post-update dispatches every active phase, including PreGame.
inline constexpr uintptr_t PresentationUpdate=0x5d11090;
inline constexpr uintptr_t MixerCreate=0x10ebf30,MixerRelease=0x10eddb0,MixerManager=0xdc4e2f0;
inline constexpr uintptr_t MatchWrapper=0xf85f700,ClockManager=0xf85f308,GameStates=0xf85f5f0;
inline constexpr uintptr_t CurrentDownGetter=0x61fe230,CurrentOffenseGetter=0x61ff6b0;
inline constexpr uintptr_t TimeoutGetter=0x5d7f0a0,FirstDownGetter=0x61fefc0;
inline constexpr uintptr_t FirstDownLine=0x5d52900,TurnoverGetter=0x2e92610;
inline constexpr uintptr_t FumblePredicate=0x6200190;
inline constexpr uintptr_t SackPredicate=0x6200850,ForcedFumblePredicate=0x6200f80;
inline constexpr uintptr_t SackPlaySelectorVtable=0xb47d970,SackPlayIndexGetter=0x35b5960;
inline constexpr uintptr_t FieldGoalBlockedPredicate=0x61ff4b0; // Broadcast key 32: match flags bit 8.
inline constexpr uintptr_t LiveFootballEvent=0xe357548,LiveFootballEventVtable=0xc2999a0,LiveFootballEventType=0x275c9e0;
inline constexpr uintptr_t PlayStats=0xf3d9fe0,SelectedPlayer=0xd9d6380,PlayerContexts=0xe3485a0;
inline constexpr uintptr_t SkillCounters=0xfad4c78,SkillCountersVtable=0xbcd5278,HurdleCountGetter=0x7d69760;
// Native catch-drop observer: +0x175 is copied from the knockout payload,
// also used for the native "Pass Dropped - KNOCKOUT!" event label.
inline constexpr uintptr_t CatchSummary=0xe357b90,CatchSummaryVtable=0xb1df9e8,CatchKnockoutStore=0x27917c8,CatchSummaryReset=0x2798712;
// WasPassTipped queries event 39 in the native per-play event cache. The
// captured third-and-20 swat credited this event to the home defender.
inline constexpr uintptr_t PassTippedQuery=0x7d7c8ce,PlayEventLookup=0x7dfb1b0,PlayEventReceive=0x7dfb020;
// Same stadium DB capacity reader and attendance option used by
// FootballMatchConditionsConstData. Reads only; no attendance/HFA mutation.
inline constexpr uintptr_t StadiumCapacity=0x38c27f0,OptionQuery=0x90994a0;
inline constexpr unsigned AttendanceOption=0x9e1d2500;
inline constexpr uintptr_t MatchVtable=0xb47e8a8,RulesVtable=0xb47ddb0;
// Native IsNeutralStadium, also used by HFA suppression at 0x2e203a4.
inline constexpr uintptr_t NeutralGetter=0x61feac0;
// Crowd slider in MixerBridge's cached user-volume array.
inline constexpr uintptr_t MasterSlider=0x1e8,CrowdSlider=0x1f4;
// Scoped crowd-event query and owned temporary event lifecycle.
inline constexpr uintptr_t SwellQuery=0x2077980,SwellQueryGet=0x207d780,SwellQueryDestroy=0x20785b0,SwellWeakRelease=0x149caf0;
inline constexpr uintptr_t SwellAllocator=0xdc54670,SwellQueryType=0xf5c23a8,SwellManagerGetter=0x3d3feb0;
inline constexpr uintptr_t SwellLookup=0x3d577d0,SwellConstruct=0x3d4fb40,SwellDestroy=0x3d50b30,SwellSetInt=0x3d53680,SwellSend=0x3d5dce0;
inline constexpr uintptr_t SwellArenaInit=0x113b060,SwellArenaDestroy=0x113b420,SwellArenaVtable=0xad95ec8;
// Output-only native excitement; never writes the native envelope or HFA.
inline constexpr uintptr_t IntensityUpdate=0x28422c0,IntensityVtable=0xb2279d0;
inline constexpr uintptr_t IntensityLock=0x114e5d0,IntensityUnlock=0x114f430,IntensityPublish=0x5b30900;
// Presentation camera interfaces and read-only accessibility settings.
inline constexpr uintptr_t CameraViewCopy=0x80a0150,CameraViewSlot=0xbd10ee0;
inline constexpr uintptr_t PlayCallService=0xe2d3918,PlayCallServiceVtable=0xb3324d0,PlayCallTeamVtable=0xb2e8690,PlayCallSelectedVtable=0xb2e82e0,PlayCallVtable=0xb1e3290;
inline constexpr uintptr_t OffensivePlayType=0x61ff750,PlayCallTeamGet=0x2f24da0,PlayCallSelectedGet=0x2b10360,PlayCallGet=0x2b10200,PlayCallType=0x27b5d80,PlayCallFallbackType=0x27b5c10;
inline constexpr uintptr_t BallControl=0xf85f648,BallStateGetter=0x5d239b0,CatchPosition=0x620c7b0;
inline constexpr uintptr_t CameraOwner=0xfcdaec0,CameraGet=0xa181380,CameraFind=0x809e150,CameraSet=0x80aa6d0,CameraVtable=0xbd10dd0;
inline constexpr uintptr_t CameraEffectsGetter=0x6201b70,CameraSettingGetter=0x5cf9ce0;
inline constexpr uintptr_t CameraSettings=0xe2cf3f0,CameraSettingsVtable=0xb7d9880;
// Read-only controller/team ownership used to select offense vs defense camera.
inline constexpr uintptr_t TeamInputManager=0xf85f120,TeamInputManagerVtable=0xb32a3b0;
inline constexpr uintptr_t MatchVisualManager=0xe37dd90,MatchVisualGetter=0x29f1820,MatchVisualVtable=0xb26bbc8;
inline constexpr uintptr_t HfaSelection=0xf3cfaf8,HfaMeter=0xf3cffd8,HfaMeterVtable=0xb300b48,HfaDisabledGetter=0x2576cc0;
inline constexpr uintptr_t HfaSelectionVtable=0xb300390;
inline constexpr uintptr_t ReactionUpdate=0x2842520,ReactionReceive=0x2841b70,ReactionVtable=0xb227b80;
// IsPaused UI accessor reads this same flag; only read it after verification.
inline constexpr uintptr_t PauseCheck=0x7baa980,PauseFlag=0xf54fa00;
// Named pause-tile text output; separate from the shared UI string writer.
inline constexpr uintptr_t PauseMenuText=0x7a1bffb,MenuStringWriter=0x25a3e60,MenuTileVtable=0xb64cca0;
// Replay manager: verified leaf getters expose playback and recording times.
// These fields are read only; live game clocks do not rewind with highlights.
inline constexpr uintptr_t ReplayManager=0xe375bf8,ReplayVtable=0xc75c0f8;
inline constexpr uintptr_t ReplayTimeGetter=0xa14a4b0,RecordingTimeGetter=0xa14a4c0;
inline constexpr uintptr_t ReplayActiveGetter=0xa14a5a0,ReplayStartGetter=0xa14a5e0;
inline constexpr uintptr_t ReplayUpdate=0x288cad0,ReplayEntityVtable=0xb236330;
// CFB post-play expression evaluation. Match complete authored selectors;
// expression objects are discovered by evaluation, never by scanning the heap.
inline constexpr uintptr_t MuseLogicEvaluate=0x913a520,MuseComparisonValue=0x9138090;
inline constexpr uintptr_t MuseLogicVtable=0xbf22a08,MuseComparisonVtable=0xbf22980;
inline constexpr uintptr_t MuseConstantVtable=0xb64c9d0,MuseFunctionVtable=0xbc82850,MuseActorFunctionVtable=0xbc82980;
inline constexpr uintptr_t GameStatesVtable=0xb47f730;
// Read only. UI IsInSuperSimGameplayMode checks this controller's mode +8.
inline constexpr uintptr_t SuperSimController=0xf8d65b8,SuperSimVtable=0xb7df080,SuperSimModeGetter=0x37574b0;
// Native APT IsInRealtimeSim / IsPlayCallOpen, independent of game phase.
inline constexpr uintptr_t ReplaySimApi=0xfa5fb10,ReplaySimApiVtable=0xbae6e10,ReplayRealtimeQuery=0x9214250;
inline constexpr uintptr_t ReplayFlowApi=0xfa60810,ReplayFlowApiVtable=0xbae65a0,ReplayPlaycallQuery=0x7baa9b0;
}
