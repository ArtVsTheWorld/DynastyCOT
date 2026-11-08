#pragma once
#include <cstdint>
// Native offsets for the inspected CFB27 layout. Signature checks are required;
// compatibility is not tied to an executable hash. Keep future binding changes
// here, separately from the configurable atmosphere rules.
namespace attendance::bindings {
// Read-only live carrier/animation tag layout; guarded type getter and asset identities.
inline constexpr uintptr_t ShowboatPlayerType=0x2ec64e0,ShowboatPlayerVtable=0xb32aca8,ShowboatAudioTagVtable=0xadaad40,ShowboatEnumValueVtable=0xae4c558;
inline constexpr uintptr_t StartingAttendanceFormula=0x856a4b0,StartingAttendanceLaunch=0x90a18b0,StartingAttendanceBoundary=0x7f182c8;
// Native authored replay lifecycle: complete prologues are checked before hooking.
inline constexpr uintptr_t ReplayEventNodeUpdate=0x7dcb0c0,ReplayEventNodeVtable=0xbc8a938;
inline constexpr uintptr_t ReplayNodeUpdate=0x7dc27f0,ReplayNodeVtable=0xbc89e68;
inline constexpr uintptr_t PresentationClient=0x7d6a7b0,PresentationEvent=0x7d6a7e0;
// Final native LegacySoundEntity event receiver and read-only Int32 variant accessor.
inline constexpr uintptr_t LegacySoundVtable=0xaf02898,LegacySoundReceive=0x10fe6a0,VariantValue=0x5b2d1c0,Int32Type=0xf640ea0;
inline constexpr uintptr_t AudioUpdate=0x28492a0,CrowdQueue=0x3d42450;
inline constexpr uintptr_t AttendanceUpdate=0x3635190;
inline constexpr uintptr_t CrowdVisibility=0x64b3930,PeriodChange=0x5d05f90,CrowdPartResolve=0x6508340;
// Read-only placement lookup verified against the native agent*16 position path.
inline constexpr uintptr_t SceneCrowdVtable=0xb7e1ba8,CrowdRendererVtable=0xb7e1988,CrowdPositionKey=0xb228418;
// GameStates post-update dispatches every active phase, including PreGame.
inline constexpr uintptr_t PresentationUpdate=0x5d11000;
inline constexpr uintptr_t MixerCreate=0x10ebf30,MixerRelease=0x10eddb0,MixerManager=0xdc4e2f0;
inline constexpr uintptr_t MatchWrapper=0xf85f700,ClockManager=0xf85f308,GameStates=0xf85f5f0;
inline constexpr uintptr_t CurrentDownGetter=0x61fe1a0,CurrentOffenseGetter=0x61ff620;
inline constexpr uintptr_t TimeoutGetter=0x5d7f010,FirstDownGetter=0x61fef30;
inline constexpr uintptr_t FirstDownLine=0x5d52870,TurnoverGetter=0x2e92610;
inline constexpr uintptr_t FumblePredicate=0x6200100;
inline constexpr uintptr_t SackPredicate=0x62007c0,ForcedFumblePredicate=0x6200ef0;
inline constexpr uintptr_t SackPlaySelectorVtable=0xb47d9d0,SackPlayIndexGetter=0x35b58d0;
inline constexpr uintptr_t FieldGoalBlockedPredicate=0x61ff420; // Broadcast key 32: match flags bit 8.
inline constexpr uintptr_t LiveFootballEvent=0xe357548,LiveFootballEventVtable=0xc299a20,LiveFootballEventType=0x275c9e0;
inline constexpr uintptr_t PlayStats=0xf3d9fe0,SelectedPlayer=0xd9d6380,PlayerContexts=0xe3485a0;
inline constexpr uintptr_t SkillCounters=0xfad4c78,SkillCountersVtable=0xbcd52d8,HurdleCountGetter=0x7d696d0;
// Native catch-drop observer: +0x175 is copied from the knockout payload,
// also used for the native "Pass Dropped - KNOCKOUT!" event label.
inline constexpr uintptr_t CatchSummary=0xe357b90,CatchSummaryVtable=0xb1dfa48,CatchKnockoutStore=0x27917c8,CatchSummaryReset=0x2798712;
// WasPassTipped queries event 39 in the native per-play event cache. The
// captured third-and-20 swat credited this event to the home defender.
inline constexpr uintptr_t PassTippedQuery=0x7d7c83e,PlayEventLookup=0x7dfb120,PlayEventReceive=0x7dfaf90;
// Same stadium DB capacity reader and attendance option used by
// FootballMatchConditionsConstData. Reads only; no attendance/HFA mutation.
inline constexpr uintptr_t StadiumCapacity=0x38c2760,OptionQuery=0x9099440;
inline constexpr unsigned AttendanceOption=0x9e1d2500;
inline constexpr uintptr_t MatchVtable=0xb47e908,RulesVtable=0xb47de10;
// Native IsNeutralStadium, also used by HFA suppression at 0x2e203a4.
inline constexpr uintptr_t NeutralGetter=0x61fea30;
// Crowd slider in MixerBridge's cached user-volume array.
inline constexpr uintptr_t MasterSlider=0x1e8,CrowdSlider=0x1f4;
// Scoped crowd-event query and owned temporary event lifecycle.
inline constexpr uintptr_t SwellQuery=0x2077980,SwellQueryGet=0x207d780,SwellQueryDestroy=0x20785b0,SwellWeakRelease=0x149caf0;
inline constexpr uintptr_t SwellAllocator=0xdc54670,SwellQueryType=0xf5c23a8,SwellManagerGetter=0x3d3fe20;
inline constexpr uintptr_t SwellLookup=0x3d57740,SwellConstruct=0x3d4fab0,SwellDestroy=0x3d50aa0,SwellSetInt=0x3d535f0,SwellSend=0x3d5dc50;
inline constexpr uintptr_t SwellArenaInit=0x113b060,SwellArenaDestroy=0x113b420,SwellArenaVtable=0xad95eb8;
// Output-only native excitement; never writes the native envelope or HFA.
inline constexpr uintptr_t IntensityUpdate=0x28422c0,IntensityVtable=0xb227a30;
inline constexpr uintptr_t IntensityLock=0x114e5d0,IntensityUnlock=0x114f430,IntensityPublish=0x5b30870;
// Presentation camera interfaces and read-only accessibility settings.
inline constexpr uintptr_t CameraViewCopy=0x80a00f0,CameraViewSlot=0xbd10f60;
inline constexpr uintptr_t PlayCallService=0xe2d3918,PlayCallServiceVtable=0xb332530,PlayCallTeamVtable=0xb2e86f0,PlayCallSelectedVtable=0xb2e8340,PlayCallVtable=0xb1e32f0;
inline constexpr uintptr_t OffensivePlayType=0x61ff6c0,PlayCallTeamGet=0x2f24d80,PlayCallSelectedGet=0x2b10360,PlayCallGet=0x2b10200,PlayCallType=0x27b5d80,PlayCallFallbackType=0x27b5c10;
inline constexpr uintptr_t BallControl=0xf85f648,BallStateGetter=0x5d23920,CatchPosition=0x620c720;
inline constexpr uintptr_t CameraOwner=0xfcdaf00,CameraGet=0xa181320,CameraFind=0x809e0f0,CameraSet=0x80aa670,CameraVtable=0xbd10e50;
inline constexpr uintptr_t CameraEffectsGetter=0x6201ae0,CameraSettingGetter=0x5cf9c50;
inline constexpr uintptr_t CameraSettings=0xe2cf3f0,CameraSettingsVtable=0xb7d98e0;
// Read-only controller/team ownership used to select offense vs defense camera.
inline constexpr uintptr_t TeamInputManager=0xf85f120,TeamInputManagerVtable=0xb32a410;
inline constexpr uintptr_t MatchVisualManager=0xe37dd90,MatchVisualGetter=0x29f1820,MatchVisualVtable=0xb26bc28;
inline constexpr uintptr_t HfaSelection=0xf3cfaf8,HfaMeter=0xf3cffd8,HfaMeterVtable=0xb300ba8,HfaDisabledGetter=0x2576cc0;
inline constexpr uintptr_t HfaSelectionVtable=0xb3003f0;
inline constexpr uintptr_t ReactionUpdate=0x2842520,ReactionReceive=0x2841b70,ReactionVtable=0xb227be0;
// IsPaused UI accessor reads this same flag; only read it after verification.
inline constexpr uintptr_t PauseCheck=0x7baa8f0,PauseFlag=0xf54fa00;
// Named pause-tile text output; separate from the shared UI string writer.
inline constexpr uintptr_t PauseMenuText=0x7a1bf6b,MenuStringWriter=0x25a3e60,MenuTileVtable=0xb64cd00;
// Replay manager: verified leaf getters expose playback and recording times.
// These fields are read only; live game clocks do not rewind with highlights.
inline constexpr uintptr_t ReplayManager=0xe375bf8,ReplayVtable=0xc75c0f8;
inline constexpr uintptr_t ReplayTimeGetter=0xa14a450,RecordingTimeGetter=0xa14a460;
inline constexpr uintptr_t ReplayActiveGetter=0xa14a540,ReplayStartGetter=0xa14a580;
inline constexpr uintptr_t ReplayUpdate=0x288cad0,ReplayEntityVtable=0xb236390;
// CFB post-play expression evaluation. Match complete authored selectors;
// expression objects are discovered by evaluation, never by scanning the heap.
inline constexpr uintptr_t MuseLogicEvaluate=0x913a4c0,MuseComparisonValue=0x9138030;
inline constexpr uintptr_t MuseLogicVtable=0xbf22a88,MuseComparisonVtable=0xbf22a00;
inline constexpr uintptr_t MuseConstantVtable=0xb64ca30,MuseFunctionVtable=0xbc828b0,MuseActorFunctionVtable=0xbc829e0;
inline constexpr uintptr_t GameStatesVtable=0xb47f790;
// Read only. UI IsInSuperSimGameplayMode checks this controller's mode +8.
inline constexpr uintptr_t SuperSimController=0xf8d65b8,SuperSimVtable=0xb7df0e0,SuperSimModeGetter=0x3757420;
// Native APT IsInRealtimeSim / IsPlayCallOpen, independent of game phase.
inline constexpr uintptr_t ReplaySimApi=0xfa5fb10,ReplaySimApiVtable=0xbae6e70,ReplayRealtimeQuery=0x92141f0;
inline constexpr uintptr_t ReplayFlowApi=0xfa60810,ReplayFlowApiVtable=0xbae6600,ReplayPlaycallQuery=0x7baa920;
}
