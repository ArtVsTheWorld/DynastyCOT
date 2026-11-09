#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <intrin.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include "CallbackTiming.h"
#include "OfflineMatchScope.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <unordered_map>
#include <string>
#include <vector>
#include "AtmosphereRules.h"
#include "ExcitementRules.h"
#include "ArrivalRules.h"
#include "DepartureModel.h"
#include "LateTrafficEvents.h"
#include "MatchLifecycle.h"
#include "ProductVersion.h"
#include "RuntimePaths.h"
#include "GameBindings.h"
#include "LogFiles.h"
#include "LogPolicy.h"
#include "SettingsPoll.h"
#include "StatusReport.h"
#include "HumanDiagnostics.h"
#include "CrowdVisibility.h"
#include "CrowdAgents.h"
#include "AwayAppearance.h"
#include "AwayCrowdAudio.h"
#include "VisitorPlans.h"
#include "SeatMapping.h"
#include "SeatBaseline.h"
#include "CrowdDispatch.h"
#include "CrowdPartResolver.h"
#include "AudioIntent.h"
#include "ChantMixerRuntime.h"
#include "CrowdIntensityRuntime.h"
#include "NativeReactionTrace.h"
#include "MotionSoundPairing.h"
#include "CannonSoundRuntime.h"
#include "ScoringSoundHandoff.h"
#include "CallbackGate.h"
#include "CrowdChangeWindow.h"
#include "SuperSimState.h"
#include "CoreAtmosphere.h"
#include "LineupAtmosphere.h"
#include "SoundOnlyReactions.h"
#include "AwaySoundReactions.h"
#include "OvertimeAtmosphere.h"
#include "OpeningKickoffAtmosphere.h"
#include "PresentationAtmosphere.h"
#include "OpeningWelcome.h"
#include "SituationalReactions.h"
#include "HailMaryPass.h"
#include "LiveFumble.h"
#include "ReturnAtmosphere.h"
#include "BackgroundRoar.h"
#include "PlayReactionTiming.h"
#include "ConfigValue.h"
#include "NativeRead.h"
#include "HalftimeExitConversion.h"
#include "SparseHfaCamera.h"
#include "NativeCameraAdmission.h"
#include "DirectorShakeRuntime.h"
#include "FinalReactionCarry.h"
#include "DiagnosticPreview.h"
#include "GameplayCamera.h"
#include "ConfigSections.h"
#include "LiveReactions.h"
#include "NativeSkillSignals.h"
#include "CatchKnockout.h"
#include "DefensiveAccents.h"
#include "DefensiveMotion.h"
#include "CrowdPopulation.h"
#include "Showboating.h"
#include "SkillSwell.h"
#include "ContextReactions.h"
#include "NativePlayCall.h"
#include "NativeSackFacts.h"
#include "AttendanceMixer.h"
#include "StadiumSilenceRuntime.h"
#include "NativeAtmosphereContext.h"
#include "NativeCapacity.h"
#include "NativeStartingAttendance.h"
#include "NativeAudience.h"
#include "StartingAttendanceLaunchHook.h"
#include "HostileOpeningResult.h"
#include "RenderedOccupancy.h"
#include "ReactionAudioRuntime.h"
#include "ResultFreshness.h"
#include "ResumeBridge.h"
#include "AttendanceTimeline.h"
#include "ReplayUpdateHook.h"
#include "ReplayPresentation.h"
#include "ReplaySelectionTrace.h"
#include "MajorReplay.h"
#include "ReplayTouchdownPolicy.h"
#include "PauseMenu.h"
#include "NativeSettingsMenu.h"

using A=uintptr_t;
extern "C" void CrowdTick();
namespace {
A image=0; HMODULE self=nullptr; std::filesystem::path folder,pluginsFolder,dataFolder,logsFolder;
std::mutex logMutex;
std::atomic<int> logLimitMB{2};
std::atomic<uint64_t> logWriteFailures{0};
std::atomic<bool> debugLog{false};
std::mutex historyMutex;attendance::History gameHistory;attendance::MatchLifecycle matchLifecycle;
std::atomic<bool> shownComeback{false},shownUpset{false},contextReady{false};
std::atomic<int> contextGameMode{-1};
std::atomic<int> busMixStrength{100};std::atomic<float> busMaximumBoost{30};
std::mutex atmosphereMutex;
attendance::energy::Tracker playTracker;
attendance::resultfreshness::Window resultFreshness;
attendance::core::Model coreAtmosphere;
attendance::finalcarry::Model finalReactionCarry;
attendance::lineup::Model lineupAtmosphere;
attendance::openingresult::Model hostileOpeningResult;
attendance::soundonly::Model soundOnlyReactions;
attendance::kickoff::Model openingKickoffAtmosphere;
attendance::presentationaudio::Model presentationAtmosphere;
attendance::openingwelcome::Model openingWelcome;
std::atomic<bool> shownLineup{false};
std::atomic<attendance::lineup::OpeningAdmission> shownOpeningDrive{attendance::lineup::OpeningAdmission::Inactive};
std::atomic<bool> shownHostileOpening{false},shownHostileSeries{false};
std::atomic<int> shownUpsetStage{0};std::atomic<bool> shownUpsetSound{false};std::atomic<float> shownUpsetMinimum{1.01f};
std::shared_ptr<const attendance::occupancy::Snapshot> stadiumOccupancy=std::make_shared<attendance::occupancy::Snapshot>();
std::shared_ptr<const attendance::occupancy::Rendered> renderedOccupancy=std::make_shared<attendance::occupancy::Rendered>();
std::mutex capacityMutex;uint64_t lastCapacityAttempt=0;
std::atomic<bool> capacityBinding{false},shownPackedHouse{false};std::atomic<float> shownDrive{0};
using InitialFormula=int(__fastcall*)(A,A);
using InitialOptionSet=bool(__fastcall*)(A,unsigned,A);
InitialFormula originalInitialFormula=nullptr;InitialOptionSet originalInitialOptionSet=nullptr;
void* initialLaunchGate=nullptr;void* initialLaunchOriginal=nullptr;
struct SessionCrowdPlan {A match=0;attendance::State state;};
std::shared_ptr<const SessionCrowdPlan> sessionCrowdPlan=std::make_shared<SessionCrowdPlan>();
void applySessionPlan(attendance::State& s,A match);
void publishSessionPlan(A match,const attendance::State& state){std::shared_ptr<const SessionCrowdPlan> next=std::make_shared<SessionCrowdPlan>(SessionCrowdPlan{match,state});std::atomic_store(&sessionCrowdPlan,next);}
std::mutex audienceMutex;attendance::audience::Book audienceBook;
std::atomic<bool> audienceStorageReady{true},audienceBindings{false};
struct AudienceCaptureStatus {int home=-1,away=-1;bool okay=false;std::string detail="no fresh regular-season launch read yet";};
std::shared_ptr<const AudienceCaptureStatus> audienceCaptureStatus=std::make_shared<AudienceCaptureStatus>();
std::mutex initialMutex;attendance::turnout::Seeds initialSeeds;
attendance::turnout::ModeWitness initialModeWitness;
std::atomic<int> initialObservedSetupMode{-999};
std::shared_ptr<const attendance::turnout::Applied> initialApplied=std::make_shared<attendance::turnout::Applied>();
// School crowd identity is useful even when revised turnout is switched off,
// unavailable, or deliberately bypassed for a resumed/postseason game.
std::shared_ptr<const attendance::turnout::Seed> crowdIdentity=std::make_shared<attendance::turnout::Seed>();
std::shared_ptr<const attendance::turnout::CrowdFixture> crowdFixture=std::make_shared<attendance::turnout::CrowdFixture>();
std::atomic<bool> initialBinding{false},initialSetting{true},initialCaptureFault{false};
using AppearanceGenerate=A(__fastcall*)(A);
AppearanceGenerate originalAppearanceGenerate=nullptr;
std::atomic<bool> appearanceBinding{false},dynamicAwayFans{true};
std::atomic<int> visitorCrowdInfluence{100};
std::atomic<unsigned> appearanceRuns{0},appearanceChanged{0},appearanceBand{0};
std::atomic<float> appearanceShare{0};
std::atomic<double> appearanceMiles{-1};
std::atomic<int> appearanceTravel{0};
std::mutex visitorPlanMutex;attendance::visitors::PlanBook visitorPlans;
std::vector<uint64_t> savedAttendanceGames; // Worker-owned disk checkpoint, never a live game pointer.
std::atomic<bool> visitorPlanWritable{true};std::atomic<int> appearancePlanSource{0};
std::mutex generatedCrowdMutex;
std::array<std::shared_ptr<const attendance::visitors::GeneratedAppearance>,32> generatedCrowds;
size_t nextGeneratedCrowd=0;std::atomic<bool> appearanceRestoreFailed{false};
std::atomic<int> shownKickoffMinutes{-1};
std::shared_ptr<const attendance::core::Settings> coreSettings=std::make_shared<attendance::core::Settings>();
std::shared_ptr<const attendance::energy::Settings> retentionEventSettings=std::make_shared<attendance::energy::Settings>();
std::atomic<float> crowdBoost{0};
std::atomic<bool> shownPostgameCarry{false},shownEruption{false};
std::atomic<int> liveDown{-2},liveOffense{-1},liveHomeTimeouts{-1};
std::atomic<unsigned> attendanceSeed{0};
std::atomic<bool> playBindingsAvailable{false};
std::atomic<bool> fumbleBinding{false};
bool fieldGoalBlockBinding=false;
int pendingLateExits=0;A atmosphereMatch=0;unsigned long long lastAtmosphereTick=0;int lastAtmospherePhase=-1;
attendance::traffic::TurnoverExits lateTurnoverExits;
std::atomic<int> lateTurnoverMaximum{10};
attendance::arrivals::Model arrivalModel;
attendance::arrivals::CompletionNotice arrivalCompletion;
attendance::CrowdChangeWindow crowdChangeWindow;
attendance::arrivals::Halftime halftimeModel;
attendance::departures::Model departureModel;
std::atomic<int> arrivalEnabled{1},arrivalStart{55},arrivalLateStart{45},arrivalFinish{50},arrivalLateFinish{75},arrivalMissing{0},halftimeExtra{100},shownHalftime{0},shownArriving{0};
attendance::halftime::ExitConversion halftimeExitConversion;
std::atomic<int> halftimeConvertPercent{25},shownHalftimeConverted{0};
std::atomic<int> arrivalBigBonus{3},arrivalWeatherPenalty{5};
std::atomic<int> attendanceProfile{1}; // 0 Fuller Crowds, 1 Default; only new launches change peak.
std::atomic<bool> arrivalReconcilePending{false};
std::atomic<bool> visibilityRetry{false},captureArrivalBaseline{false};
std::atomic<bool> pregameRendererRetry{false};
std::atomic<bool> postgameRendererRetry{false};
std::atomic<bool> superSimBinding{false},shownSuperSim{false};
double simulationRecording=-1;
unsigned visibilityGeneration=0;
std::atomic<float> shownAmbienceLift{0},shownOpeningWeight{0},shownAudioLimit{4};
std::atomic<unsigned long long> shownDecisiveUntil{0},shownHighUntil{0};
std::atomic<int> request{0};
std::atomic<unsigned long long> ticks{0};
std::vector<A> entities;
std::mutex entityMutex;
std::atomic<unsigned> entityGeneration{0};
std::atomic<bool> refreshEntities{false};
std::shared_ptr<const attendance::Settings> settings=std::make_shared<attendance::Settings>();
std::atomic<int> mode{0}; // 0 automatic; 1 manual hold; 2 restored/disabled.
std::atomic<int> shownQuarter{0},shownScore0{0},shownScore1{0},shownPhase{0};
std::atomic<int> shownProfile{1},shownRank0{0},shownRank1{0},shownBowl{127},shownPrecipitation{0};
std::atomic<bool> shownRivalry{false},shownPlayoff{false},shownFcs{false};
std::atomic<bool> shownNeutral{false},shownConference{false},shownHfa{false},shownConferenceKnown{false},shownBigAtmosphere{false};
std::atomic<int> shownTeam0{-1},shownTeam1{-1},shownRivalryEvent{-1};
std::atomic<float> shownSeconds{0},shownCurrent{0},shownTarget{0};
float currentLoss=0,highestTarget=0;
int lastApplied=-1,lastQuarter=0,lastScore0=0,lastScore1=0;
float lastClock=0;
A lastMatch=0;
unsigned lastGeneration=0;
unsigned long long lastDecision=0,lastFrame=0;
std::atomic<bool> enabled{false};
thread_local bool applying=false;
thread_local attendance::visibility::PassParts passParts;
thread_local bool thinning=false;
thread_local bool reconcilingArrivals=false,capturingBaseline=false;
thread_local unsigned int removalLimit=0;
std::atomic<int> activeLoss{0};
std::atomic<bool> collectingStartingCrowd{true};
struct StartingCrowd {
    A masks=0;unsigned short parts=0;std::vector<unsigned short> hidden,temporary,nativeSeen,nativeHidden;
    A scene=0;unsigned captureToken=0;bool baselineComplete=false;attendance::seats::Pattern selection;
    attendance::agents::Tags agents;
    attendance::visitors::Groups supporters;
    attendance::visitors::Losses exits;
    A geometry=0;uint64_t layoutKey=0;bool exitsSeeded=false;
    unsigned homeGone=0,awayGone=0,homeAbsent=0,awayAbsent=0;
};
std::mutex crowdMutex;
inline bool selectedSupporter(const StartingCrowd& base,unsigned agent,int purpose,int fallbackLoss){
    const auto& group=base.supporters;
    if(!group.verified||agent>=group.rank.size())return base.selection.departed(agent,fallbackLoss);
    if(group.rank[agent]==UINT32_MAX)return false;
    bool away=group.side[agent]!=0;
    unsigned permanent=away?base.awayGone:base.homeGone,absent=away?base.awayAbsent:base.homeAbsent;
    return group.rank[agent]<(purpose==0?permanent:purpose==1?absent:(std::max)(permanent,absent));
}
std::unordered_map<A,StartingCrowd> startingCrowds;
attendance::visitors::Losses sharedFanExits;attendance::fanbudget::Counts sharedFanBasis;bool sharedFanExitsSeeded=false;
std::atomic<bool> sharedFanBudgetApplied{false},resumeLegacyBudget{false},replaySharedBudget{true};
std::atomic<uint64_t> sharedFanTarget{0},sharedFanProtected{0};
std::atomic<int> replayPermanentLoss{0};
// References in an unordered_map survive rehash, but not erasure/replacement.
// This revision is read and changed only under crowdMutex. The per-pass cache
// never outlives its forced native traversal and never bypasses that lock.
uint64_t crowdRevision=0;
thread_local A passRenderer=0;
thread_local StartingCrowd* passBaseline=nullptr;
thread_local uint64_t passRevision=0;
attendance::seats::Baselines originalSeatLayouts;
std::atomic<bool> seatPatternPending{false};
std::atomic<unsigned> shownSeatPatterns{0},shownSeatFallbacks{0};
unsigned captureSequence=0;thread_local unsigned activeCaptureToken=0;
std::atomic<bool> replayBinding{false},exactVisibility{false},replaySuspendsAtmosphere{false};
std::atomic<int> replayLoss{-1};
std::atomic<float> replayHomeLoss{-1},replayAwayLoss{-1},resumeHomeLoss{-1},resumeAwayLoss{-1};
std::atomic<int> replayVisitorArrivals{-1};
std::shared_ptr<const attendance::visitors::Population> visitorPopulation=std::make_shared<attendance::visitors::Population>();
bool replayWasActive=false,replayRestorePending=false;
std::mutex timelineMutex;attendance::timeline::Archive attendanceArchive;
std::atomic<bool> attendanceHistoryWritable{true};
std::shared_ptr<const attendance::resume::Signal> resumeSignal=std::make_shared<attendance::resume::Signal>();
attendance::resume::Tracker resumeTracker;
bool bootstrapPending=false;double lastRecordingTime=-1;
std::atomic<int> shownResumeSource{0}; // 0 ordinary, 1 estimated, 2 saved history.
thread_local unsigned int calls=0,changed=0,verifiedCalls=0;
std::atomic<unsigned> shownVisibilityCalls{0},shownVisibilityControllers{0};
std::atomic<unsigned long long> shownOriginalFans{0},shownVisibleFans{0},shownExpectedFans{0},shownNativeExtra{0},shownOtherExtra{0},shownExtraVisible{0};
std::atomic<bool> visibilityMeasured{false};
thread_local std::set<A> observed;
using SetVisibility=void(__fastcall*)(A,unsigned int,unsigned int,bool);
using SetPeriod=void(__fastcall*)(A,int);
using AttendanceUpdate=void(__fastcall*)(A);
AttendanceUpdate originalAttendanceUpdate=nullptr;
using PresentationUpdate=void(__fastcall*)(A,float);
PresentationUpdate originalPresentationUpdate=nullptr;
using ReplayUpdate=void(__fastcall*)(A);
ReplayUpdate originalReplayUpdate=nullptr;
std::mutex presentationMutex;
std::atomic<unsigned long long> replayCallbacks{0};
void __fastcall replayUpdate(A entity);
bool guardedDecision();
using MuseLogicEvaluate=bool(__fastcall*)(A,A);
using MuseComparisonValue=A(__fastcall*)(A,A);
MuseLogicEvaluate originalMuseLogic=nullptr;
MuseComparisonValue originalMuseComparison=nullptr;
std::atomic<bool> replayPresentationInstalled{false},replayPresentationFailed{false};
std::atomic<bool> extraReplays{true},routineInsets{true},bigPlayReplays{true},defensiveReplays{true};
std::atomic<int> replayMinimumYards{15};
std::atomic<unsigned long long> insetEligibilityChanges{0},fullEligibilityChanges{0},defensiveRouteChanges{0};
thread_local attendance::replaypresentation::ThreadState replaySelection;
using ReplayEventNodeUpdate=void(__fastcall*)(A);
using NativeReplayNodeUpdate=void(__fastcall*)(A,float);
ReplayEventNodeUpdate originalReplayEventNode=nullptr;NativeReplayNodeUpdate originalNativeReplayNode=nullptr;
std::atomic<bool> majorReplayBindings{false},majorReplayFailed{false};std::atomic<int> replaySnapType{-1};
bool replayModeBinding=false;
attendance::replaymode::Tracker replayModeTracker;
std::atomic<attendance::replaymode::Mode> replayMode{attendance::replaymode::Mode::Unknown};
std::mutex majorReplayMutex;attendance::majorreplay::Model majorReplay;attendance::energy::Tracker replayPlayTracker;
attendance::replaypolicy::TouchdownExclusion replayTouchdown;
bool extraReplayForCurrentPlay(){std::lock_guard<std::mutex> guard(majorReplayMutex);return replayTouchdown.allowed();}
A replayTrackerMatch=0;unsigned replayTrackerEpoch=0;attendance::energy::Play replayObserved;
double replayRecordingStart=-1;uint64_t replayRecordingSequence=0;
std::mutex replayTraceMutex;attendance::replaytrace::Buffer replayTrace;
void observeMajorReplay(bool dispatch);bool guardedMajorReplay(bool dispatch);
void __fastcall replayEventNode(A node);void __fastcall nativeReplayNode(A node,float delta);

SetPeriod originalPeriod=nullptr;
SetVisibility originalSet=nullptr;
unsigned long long restoreAt=0;
std::atomic<uint64_t> nativeAudioCallbacks{0};
std::atomic<bool> emptyMode2020{false},emptyCrowdActive{false},emptyModePending{false};
struct EmptyWorld {A match=0;int phase=-1,home=-1,away=-1;};
bool emptyWorld(EmptyWorld& world);bool guardedEmptyService();
attendance::silence::Runtime stadiumSilence;
std::atomic<bool> silenceBindingsReady{false};
attendance::timing::Counts audioMixerTimings,intensityCallbackTimings,reactionCallbackTimings,cameraCallbackTimings,sparseCameraTimings,cannonCallbackTimings;
std::atomic<uint64_t> audioContextSnapshots{0},audioWorldSnapshots{0};
using NativeAudioUpdate=A(__fastcall*)(A,A,A,A);NativeAudioUpdate originalAudioUpdate=nullptr;
std::atomic<bool> audioHookInstalled{false},audioSetting{true};
std::atomic<int> nativeCrowdSlider{-1};std::atomic<bool> notedAudioState{false};

attendance::balance::Runtime attendanceMixer;
attendance::mix::Continuity audioContinuity;
std::atomic<bool> chantAvailable{false};
std::atomic<A> audioMatch{0};
std::atomic<unsigned> audioEpoch{1};
void applySessionPlan(attendance::State& s,A match){auto density=std::atomic_load(&renderedOccupancy);s.mappedPeak=density->owns(match,audioEpoch.load())?density->peak():-1;auto plan=std::atomic_load(&sessionCrowdPlan);if(plan->match==match&&s.team0==plan->state.team0&&s.team1==plan->state.team1){attendance::inheritCrowdPlan(plan->state,s);if(s.plannedStadium<0){s.plannedStadium=plan->state.plannedStadium;s.night=plan->state.night;}}}
attendance::sustained::Runtime sustainedAudio,supportingSwell;
attendance::cheer::Runtime positiveCheer,extraCheer,opponentCheer;
uint64_t extraReactionId=0,extraReactionUntil=0,extraReactionSerial=0,extraReactionSequence=0;int extraReactionType=83;
attendance::nativevoices::Trace nativeVoiceTrace;std::mutex nativeVoiceTraceMutex;
std::atomic<bool> cheerAvailable{false};
using ReactionUpdate=void(__fastcall*)(A,A);ReactionUpdate originalReactionUpdate=nullptr;
attendance::sustained::IntensityRuntime crowdIntensity;
attendance::motion::Runtime presentationCamera;
attendance::motion::PointerHook cameraViewHook,cannonReceiverHook;
using LegacySoundReceive=void(__fastcall*)(A,A);LegacySoundReceive originalLegacySoundReceive=nullptr;
std::shared_ptr<const attendance::cannon::Blast> cannonBlast=std::make_shared<attendance::cannon::Blast>();
std::mutex cannonMutex;std::atomic<bool> cannonAvailable{false};std::atomic<unsigned> cannonStarts{0};
std::atomic<bool> nativeMasterAudible{false};
attendance::sparsecamera::Runtime sparseNativeCamera;
attendance::sparsecamera::Api::Set originalNativeCameraSet=nullptr;
std::atomic<bool> sparseCameraAvailable{false},sparseCameraFaultReported{false};std::atomic<int> nativeShakeMinimum{50};
std::shared_ptr<const attendance::sparsecamera::Lease> sparseCameraLease=std::make_shared<attendance::sparsecamera::Lease>();
std::shared_ptr<const attendance::nativecamera::Admission> nativeCameraAdmission=std::make_shared<attendance::nativecamera::Admission>();
attendance::motion::Model cameraMotion;
attendance::live::Model liveReactions;
attendance::skills::Model skillReactions;
attendance::knockout::Model catchKnockouts;
attendance::defense::Model defensiveAccents;
attendance::defensivemotion::Model defensiveMotion;
attendance::showboating::Model showboating;
std::atomic<bool> passBreakupBinding{false};
attendance::skillswell::Model skillSwell;
attendance::contextreaction::Model contextReactions;
std::atomic<bool> skillBindings{false};
std::atomic<bool> hurdleCountBinding{false};
std::atomic<bool> catchKnockoutBinding{false};
attendance::situational::Model situationalReactions;
attendance::awaysound::Model awaySoundReactions;attendance::motion::Model awayCameraMotion;attendance::motion::Cue awayLastCue;
uint64_t awayHeardId=0,awayHeardAt=0,awayHeardSequence=0,awayExtraHeardId=0;
int celebrationReactionType=91;
attendance::hailmary::Pass hailMaryPass;
attendance::live::Fumbles liveFumbles;
attendance::fumblecontact::Model fumbleContacts;
std::atomic<bool> contactFumbleBinding{false};
attendance::live::ReturnAtmosphere returnAtmosphere;
attendance::soundhandoff::Model scoringSoundHandoff;
attendance::timing::Distance reactionDistance;
std::atomic<unsigned> reactionDistanceMaximum{0},shownDistanceExtra{0};std::atomic<float> shownReactionDistance{0};
std::atomic<bool> liveFumbleBinding{false};
std::atomic<bool> situationalReactionsEnabled{true};
attendance::live::PassFlight passFlight;
std::atomic<unsigned> shownPassFlights{0};std::atomic<const char*> shownPassGate{"waiting for an eligible snap"};
attendance::live::RunnerCoordinates runnerCoordinates;
attendance::live::Returns returnReactions;
std::shared_ptr<const attendance::live::Settings> liveReactionSettings=std::make_shared<attendance::live::Settings>();
bool liveReactionBindings=false,runnerPositionBinding=false,playCallBinding=false;
std::atomic<bool> defensiveReactionBindings{false};
uint64_t liveReactionId=0,liveReactionUntil=0,liveSourceSequence=0,liveMotionSequence=0;int liveReactionType=83;
bool liveReactionMajor=false,liveReactionMilestone=false;
attendance::live::Event liveMotionEvent=attendance::live::Event::None;
uint64_t liveAuditId=0,liveAuditUntil=0;
uint64_t playAuditSequence=0,playAuditEndSequence=0;
bool liveReactionPost=false;
std::atomic<unsigned> shownLiveCatches{0},shownLivePicks{0};
using CameraViewCopy=void(__fastcall*)(A,float*);CameraViewCopy originalCameraView=nullptr;
bool cameraBindings=false;
std::shared_ptr<const attendance::motion::Settings> cameraSettings=std::make_shared<attendance::motion::Settings>();
std::shared_ptr<const attendance::motion::Cue> cameraCue=std::make_shared<attendance::motion::Cue>();
std::atomic<bool> cameraAvailable{false},cameraFaultReported{false},conferenceBinding{false},hfaBinding{false};
std::shared_ptr<const attendance::sustained::Intent> sustainedIntent=std::make_shared<attendance::sustained::Intent>();
attendance::diagnostic::Model diagnosticModel;
uint64_t diagnosticProcessed=0;
std::shared_ptr<const attendance::diagnostic::Request> diagnosticRequest=std::make_shared<attendance::diagnostic::Request>();
std::shared_ptr<const attendance::diagnostic::Preview> diagnosticPreview=std::make_shared<attendance::diagnostic::Preview>();
std::shared_ptr<const std::string> diagnosticStatus=std::make_shared<std::string>("idle");
std::atomic<bool> swellAvailable{false},intensityAvailable{false},audioAudible{false};
std::atomic<bool> pauseBinding{false};
std::atomic<bool> pauseMenuSetting{true},pauseMenuAvailable{false};
std::atomic<int> pauseMenuLastPercent{-2};std::atomic<uint64_t> pauseMenuDisplays{0};
attendance::pausemenu::Labels pauseMenuLabels;
attendance::settingsmenu::native::Menu settingsMenu;
void* settingsMenuOriginal=nullptr;bool settingsMenuAvailable=false;
attendance::settingsmenu::native::PauseMenu pauseSettingsMenu;
void* pauseSettingsMenuOriginal=nullptr;bool pauseSettingsMenuAvailable=false;
void* pauseMenuGate=nullptr;void* pauseMenuOriginal=nullptr;
std::atomic<unsigned long long> lastAudibleUpdate{0};
using IntensityUpdate=void(__fastcall*)(A);
IntensityUpdate originalIntensityUpdate=nullptr;
attendance::sustained::Api::Send originalCrowdSend=nullptr;
attendance::audio::Wanted audioWanted();
attendance::balance::Mix audioBusMix();

void audioNote(const char* message);
void log(const char* event,const std::string& message);
void sustainedNote(const char* message);
bool nativeUnpaused(){
    A pause=0;unsigned char paused=1;SIZE_T copied=0;
    return pauseBinding&&ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(image+attendance::bindings::PauseFlag),&pause,sizeof(pause),&copied)&&copied==sizeof(pause)&&pause>=0x10000&&
        ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(pause),&paused,1,&copied)&&copied==1&&paused==0;
}
int audioPresentationPhase(){
    int phase=attendance::sustained::at<int>(attendance::sustained::at<A>(image+attendance::bindings::GameStates)+0xa0);
    if(phase==8&&attendance::sustained::at<int>(attendance::sustained::at<A>(image+attendance::bindings::ClockManager)+8)==6)phase=7;
    return phase;
}
float audibleFullness(const attendance::audio::Wanted& wanted){
    auto rendered=std::atomic_load(&renderedOccupancy);int historical=replayLoss.load();
    if(historical>=0&&!replaySuspendsAtmosphere&&rendered->owns(wanted.match,wanted.epoch))
        return rendered->peak()*(100-(std::clamp)(historical,0,100))*.01f;
    return (std::min)(wanted.remaining,wanted.arrivalFraction);
}
attendance::sustained::Intent wantedSwell(const attendance::audio::Wanted& wanted){
    auto intent=*std::atomic_load(&sustainedIntent);auto c=std::atomic_load(&coreSettings);
    auto now=GetTickCount64(),audibleAt=lastAudibleUpdate.load();
    auto preview=std::atomic_load(&diagnosticPreview);
    if(preview->sound&&preview->fresh(wanted.match,wanted.epoch,now)){
        if(preview->playing(wanted.match,wanted.epoch,now))intent=preview->intent;
        else if(preview->cooling)intent.active=false;
    }
    int phase=audioPresentationPhase();
    if(intent.diagnosticToken&&phase!=1)intent.active=false;
    // Verified intros can run their audio while the gameplay world is paused.
    if(!wanted.active||mode!=0||!c->enabled||!audioAudible||now<audibleAt||now-audibleAt>750||
       (!nativeUnpaused()&&phase!=6)||!attendance::sustained::fresh(intent,wanted.match,wanted.epoch,now))intent.active=false;
    // Stop pre-snap shake and scheduled cheers as soon as the native snap is
    // visible. The model keeps audio continuous through the result handoff.
    intent.cameraPrime=phase==5&&intent.shake;
    if(phase!=1&&phase!=5){intent.shake=false;intent.shakeTier=0;}
    if(phase!=1&&intent.reason==attendance::sustained::Reason::Lineup)intent.cheers=false;
    return intent;
}
attendance::sustained::Intent wantedSwell(){return wantedSwell(audioWanted());}
void __fastcall intensityUpdate(A entity){
    originalIntensityUpdate(entity);
    attendance::timing::Scope measured(intensityCallbackTimings);
    auto wanted=audioWanted();
    try {
        crowdIntensity.tick(entity,wantedSwell(wanted),wanted.match,wanted.epoch,GetTickCount64());
    }
    catch(...){crowdIntensity.fail();audioNote("intensity-fault: temporary excitement stopped; native output will be restored on its next update");}
}
void __fastcall reactionUpdate(A entity,A frame){
    originalReactionUpdate(entity,frame);
    attendance::timing::Scope measured(reactionCallbackTimings);
    // Observation only, using this callback's borrowed live entity. Debug=0
    // adds no reads, scanning, allocation or locks to the normal path.
    try {if(debugLog&&attendance::sustained::at<A>(entity)==image+attendance::bindings::ReactionVtable){
        auto wanted=audioWanted();A begin=attendance::sustained::at<A>(entity+0x120),end=attendance::sustained::at<A>(entity+0x128);
        if(wanted.active&&end>=begin&&(end-begin)%16==0&&end-begin<=16*16){
            attendance::nativevoices::Snapshot voices;voices.count=static_cast<unsigned>((end-begin)/16);
            for(unsigned i=0;i<voices.count;++i)voices.voices[i]={attendance::sustained::at<int>(begin+i*16),attendance::sustained::at<int>(begin+i*16+4)};
            bool voicesChanged=false;{std::lock_guard<std::mutex> guard(nativeVoiceTraceMutex);voicesChanged=nativeVoiceTrace.observe(entity,attendance::sustained::at<A>(entity+0x38),wanted.match,wanted.epoch,GetTickCount64(),voices);}
            if(voicesChanged){std::ostringstream text;text<<"Engine crowd voices: "<<voices.count<<"; phase "<<audioPresentationPhase()<<"; world paused "<<(nativeUnpaused()?"no":"yes")<<"; types/handles";
                for(unsigned i=0;i<voices.count;++i)text<<" "<<voices.voices[i].type<<"/"<<voices.voices[i].handle;
                text<<". Includes game and DGA requests; compare reaction-audio entries. A listed instance does not prove audible playback.";log("native-reactions",text.str());}
        }
    }}catch(...){/* Optional observation must never interrupt the native or added cheer callback. */}
    try {auto wanted=wantedSwell();auto now=GetTickCount64();positiveCheer.tick(entity,wanted,now);
        if(wanted.extraCheer&&!positiveCheer.priorityPending()&&!wanted.scoringReaction&&!wanted.celebration)try{auto extra=wanted;extra.celebration=extra.resultCheer=extra.lineupCheer=0;extra.cheers=extra.staggered=extra.majorLive=extra.deepCheers=false;
         extra.liveCheer=extra.extraCheer;extra.liveCheerUntil=extra.extraCheerUntil;extra.liveType=extra.extraType;extraCheer.tick(entity,extra,now);}catch(...){extraCheer.fail();audioNote("skill-cheer-fault: independent skill cheer stopped; other reactions unchanged");}
        if(wanted.opponentCheer&&!positiveCheer.priorityPending())try{auto other=wanted;other.celebration=other.resultCheer=other.lineupCheer=0;
         other.cheers=other.staggered=other.majorLive=other.deepCheers=false;other.supporterSize=0;
         other.liveCheer=wanted.opponentCheer;other.liveCheerUntil=wanted.opponentCheerUntil;other.liveType=wanted.opponentType;
         other.populationSize=wanted.opponentSize;opponentCheer.tick(entity,other,now);}catch(...){opponentCheer.fail();audioNote("visitor-cheer-fault: independent visitor response stopped; other reactions unchanged");}
    }
    catch(...){positiveCheer.fail();audioNote("cheer-fault: further added cheers disabled");}
}
void __fastcall nativeCameraSet(A camera,int code,bool flag){
    int output=code;
    auto lease=std::atomic_load(&sparseCameraLease);
    A actual=attendance::sustained::at<A>(attendance::sustained::at<A>(image+attendance::bindings::MatchWrapper)+16);
    if(actual==lease->match)try{output=sparseNativeCamera.filter(camera,code,*lease,GetTickCount64());}catch(...){sparseNativeCamera.fail();if(!sparseCameraFaultReported.exchange(true))audioNote("camera-fault: sparse native HFA filtering stopped; restoring native camera control.");}
    originalNativeCameraSet(camera,output,flag);
}
void sparseCameraTick(){
    attendance::timing::Scope measured(sparseCameraTimings);
    if(!sparseCameraAvailable)return;
    auto now=GetTickCount64();auto wanted=audioWanted();auto density=std::atomic_load(&renderedOccupancy);
    attendance::sparsecamera::Lease lease;lease.epoch=wanted.epoch;lease.updated=now;
    // Keep a valid match identity for restoration even after Soundscape is disabled.
    A actual=attendance::sustained::at<A>(attendance::sustained::at<A>(image+attendance::bindings::MatchWrapper)+16);
    A config=attendance::sustained::at<A>(image+0xf63f9f0);
    bool offline=actual&&actual==wanted.match&&attendance::sustained::at<A>(actual)==image+0xb47e908&&config&&
        attendance::sustained::at<unsigned char>(config+0x84)==0&&attendance::scope::matchMode(attendance::sustained::at<int>(image+0xd9eb808))&&attendance::sustained::at<int>(image+0xf3fddbc)==3;
    lease.match=offline?actual:0;
    EmptyWorld empty;bool silence=emptyMode2020&&emptyWorld(empty);
    if(silence)lease.match=empty.match;
    auto admission=std::atomic_load(&nativeCameraAdmission);
    auto motion=std::atomic_load(&cameraCue);
    bool dgaPlaying=motion->match==wanted.match&&motion->epoch==wanted.epoch&&
        motion->kind!=attendance::motion::Kind::Cannon&&presentationCamera.playing(*motion,now);
    lease.suppress=sparseNativeCamera.healthy()&&wanted.active&&enabled&&mode==0&&density->owns(wanted.match,wanted.epoch)&&
        attendance::nativecamera::suppress(*admission,wanted.match,wanted.epoch,now,audibleFullness(wanted),nativeShakeMinimum.load(),dgaPlaying)&&
        attendance::balance::audiblePhase(audioPresentationPhase(),nativeUnpaused(),replaySuspendsAtmosphere.load());
    if(silence&&sparseNativeCamera.healthy())lease.suppress=true;
    std::shared_ptr<const attendance::sparsecamera::Lease> published=std::make_shared<attendance::sparsecamera::Lease>(lease);std::atomic_store(&sparseCameraLease,published);
    // Idle admitted contexts add no camera query/ref traffic. An active DGA cue
    // owns motion; native tiers are remembered and restored when it stops. A suppressed view
    // gets a final borrowed-camera pass to restore the game's latest HFA event.
    if(lease.suppress||sparseNativeCamera.active())sparseNativeCamera.tick(attendance::sustained::at<A>(image+attendance::bindings::CameraOwner),lease,now);
}
bool guardedSparseCameraTick(){__try{sparseCameraTick();return true;}__except(EXCEPTION_EXECUTE_HANDLER){return false;}}
bool cannonRead(A p,void* value,size_t size){SIZE_T copied=0;return p>=0x10000&&ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(p),value,size,&copied)&&copied==size;}
bool cannonInteger(A variant,int& value){
    using Get=A(__fastcall*)(A,A);
    A data=0;return attendance::sustained::obtain(data,reinterpret_cast<Get>(image+attendance::bindings::VariantValue),variant,image+attendance::bindings::Int32Type)&&cannonRead(data,&value,sizeof(value));
}
bool cannonPlaying(A instance){
    using Playing=bool(__fastcall*)(A);
    __try{
        A table=attendance::sustained::at<A>(instance);if(table<image+0xac00000||table>=image+0xdc00000)return false;
        A function=attendance::sustained::at<A>(table+0x58);if(function<image||function>=image+0xac00000)return false;
        return reinterpret_cast<Playing>(function)(instance);
    }
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
void observeCannon(A entity,A event){
    if(emptyMode2020||!cannonAvailable||!enabled||mode!=0||replayLoss.load()>=0||replaySuspendsAtmosphere||!audioSetting||!cameraAvailable||!nativeMasterAudible)return;
    // The receiver sees many parameter/stop events too. They cannot start a
    // cannon, so do not sample the world or resolve sound assets for them.
    unsigned eventHash=0;if(!cannonRead(event+8,&eventHash,sizeof(eventHash))||eventHash!=attendance::cannon::PlayEvent)return;
    auto config=std::atomic_load(&coreSettings);A match=audioMatch.load();unsigned epoch=audioEpoch.load();auto now=GetTickCount64();
    A configuration=attendance::sustained::at<A>(image+0xf63f9f0);
    if(!cannonAvailable||!enabled||mode!=0||replayLoss.load()>=0||replaySuspendsAtmosphere||!audioSetting||!config->enabled||!config->cameraShake||!cameraAvailable||!match||!nativeUnpaused()||!configuration||attendance::sustained::at<unsigned char>(configuration+0x84)!=0||!nativeMasterAudible)return;
    // Match/world/offline and accessibility are checked again at camera output.
    int effect=attendance::cannon::started(image,entity,event,cannonRead,cannonInteger,cannonPlaying);
    if(effect<0)return;
    std::lock_guard<std::mutex> guard(cannonMutex);auto old=std::atomic_load(&cannonBlast);
    if(old->match==match&&old->epoch==epoch&&now>=old->began&&now-old->began<120)return;
    std::shared_ptr<const attendance::cannon::Blast> next=std::make_shared<attendance::cannon::Blast>(attendance::cannon::Blast{match,epoch,now,effect});
    std::atomic_store(&cannonBlast,next);++cannonStarts;
}
void __fastcall legacySoundReceive(A entity,A event){
    originalLegacySoundReceive(entity,event);
    attendance::timing::Scope measured(cannonCallbackTimings);
    try{observeCannon(entity,event);}catch(...){cannonAvailable=false;}
}
bool read(A p,void* out,size_t n);
void __fastcall cameraView(A view,float* output){
    originalCameraView(view,output);
    attendance::timing::Scope measured(cameraCallbackTimings);
    if(emptyMode2020||!enabled||mode!=0||!cameraAvailable||replayLoss.load()>=0)return; // Recorded clips, including play-selection insets, stay steady.
    auto published=std::atomic_load(&cameraCue);auto cue=*published;auto preview=std::atomic_load(&diagnosticPreview);
    if(preview->fresh(audioMatch.load(),audioEpoch.load(),GetTickCount64())&&(preview->active||preview->cooling)){
        auto config=std::atomic_load(&coreSettings);
        if(!preview->playing(audioMatch.load(),audioEpoch.load(),GetTickCount64())||!preview->camera||!preview->sound||!config->enabled||!config->cameraShake||!nativeUnpaused()||audioPresentationPhase()!=1)return;
        cue=preview->cue;
    }
    bool cannonSound=false;
    if(!preview->active&&!preview->cooling){
        auto config=std::atomic_load(&coreSettings);auto blast=std::atomic_load(&cannonBlast);
        cue=attendance::cannon::merge(cue,*blast,GetTickCount64(),std::atomic_load(&cameraSettings)->cannon,
            enabled&&mode==0&&audioSetting&&config->enabled&&config->cameraShake&&blast->match==audioMatch.load()&&blast->epoch==audioEpoch.load());
        auto now=GetTickCount64();cannonSound=std::atomic_load(&cameraSettings)->cannon>0&&nativeMasterAudible&&blast->match==audioMatch.load()&&blast->epoch==audioEpoch.load()&&attendance::cannon::variant(blast->effect)&&blast->began&&now>=blast->began&&now-blast->began<900;
    }
    if(cue.kind==attendance::motion::Kind::None)return;
    try{
        auto now=GetTickCount64();
        auto cameraConfig=std::atomic_load(&cameraSettings);float presetScale=1;
        // At the common 100% preferences no named-preset lookup is needed.
        // Projection normalization already follows the actual output view.
        if(!shownSuperSim&&replayMode!=attendance::replaymode::Mode::RealtimeSim)
            presetScale=attendance::gamecamera::preference(cameraConfig->cameraPercent,image,audioMatch.load(),read);
        if(presetScale==0)return;
        // The native audio callback supplies the companion sound from this
        // envelope. Voice-start latency must not suppress or shorten motion.
        // User mute, world ownership and the original camera envelope still apply.
        auto coreConfig=std::atomic_load(&coreSettings);
        if(!cannonSound&&(!audioAudible||!coreConfig->enabled||!coreConfig->cameraShake||
           !nativeUnpaused()||cue.match!=audioMatch.load()||cue.epoch!=audioEpoch.load()))return;
        presentationCamera.tick(view,output,cue,audioMatch.load(),audioEpoch.load(),now,cameraConfig->widePlayerCompensation,presetScale);
    }
    catch(...){presentationCamera.fail();}
}
A __fastcall crowdSend(A manager,A result,A event){
    A returned=originalCrowdSend(manager,result,event);
    sustainedAudio.observeNativeStop(manager,event);supportingSwell.observeNativeStop(manager,event);
    return returned;
}
void logAudioState(int volume);
A __fastcall audioUpdate(A owner,A a,A b,A c){
    A result=originalAudioUpdate(owner,a,b,c);++nativeAudioCallbacks;
    attendance::timing::Scope measured(audioMixerTimings);
    int volume=0;SIZE_T count=0;
    bool audible=owner>=0x10040&&ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(owner-0x40+attendance::bindings::CrowdSlider),&volume,4,&count)&&count==4&&volume>0&&volume<=100;
    nativeCrowdSlider=count==4&&volume>=0&&volume<=100?volume:-1;
    int master=0;SIZE_T masterCount=0;nativeMasterAudible=owner>=0x10040&&ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(owner-0x40+attendance::bindings::MasterSlider),&master,4,&masterCount)&&masterCount==4&&master>0&&master<=100;
    audible=audible&&nativeMasterAudible;
    if(!notedAudioState.exchange(true))logAudioState(nativeCrowdSlider.load());
    audioAudible=audible;lastAudibleUpdate=GetTickCount64();
    if(silenceBindingsReady&&(emptyMode2020||stadiumSilence.active())){
        EmptyWorld world;bool emptyAudio=emptyMode2020&&emptyWorld(world);
        stadiumSilence.tick(owner,emptyAudio,nativeUnpaused()&&(world.phase==1||world.phase==3||world.phase==4||world.phase==5));
    }
    if(swellAvailable)try {
        auto wanted=wantedSwell();const auto companionNow=GetTickCount64();auto companion=std::atomic_load(&cameraCue);
        if(nativeUnpaused()){
            const auto fullness=audibleFullness(audioWanted());
            if(!companion->visitor)wanted=attendance::pairing::merge(wanted,*companion,companionNow,fullness,wanted.playSequence,audioPresentationPhase()==1);
            else{
                // A visitor reaction gets a visitor-sized sustained companion,
                // independent of the short home/away one-shot delivery proof.
                auto supporters=std::atomic_load(&visitorPopulation);auto rendered=std::atomic_load(&renderedOccupancy);
                attendance::sustained::Intent visitor;visitor.active=wanted.active;visitor.match=wanted.match;visitor.epoch=wanted.epoch;visitor.updated=wanted.updated;
                visitor=attendance::pairing::merge(visitor,*companion,companionNow,fullness,wanted.playSequence);
                wanted=attendance::visitors::layer(wanted,visitor,*supporters,rendered->seats,fullness);
            }
        }
        auto primary=wanted;auto preview=std::atomic_load(&diagnosticPreview);
        bool diagnostic=preview->sound&&preview->playing(wanted.match,wanted.epoch,GetTickCount64())&&wanted.active;
        if((diagnostic&&!preview->primary)||(!diagnostic&&wanted.supportingOnly&&supportingSwell.healthy()))primary.active=false;
        sustainedAudio.tick(owner,primary,GetTickCount64());
        if(diagnostic)wanted.supporting=preview->supporting;
        supportingSwell.tick(owner,attendance::sustained::supportingIntent(wanted),GetTickCount64());
    }catch(...){sustainedAudio.fail();supportingSwell.fail();audioNote("swell-fault: new starts stopped; cleanup will retry on the next audio update");}
    if(chantAvailable)try {
        auto mix=audioBusMix();attendanceMixer.tick(owner,mix.valid,mix);
    }catch(...){attendanceMixer.fail();audioNote("audio-fault: new crowd mix adjustments stopped; owned preset cleanup will retry on the next audio update");}

    return result;
}

bool signatures=false;
bool read(A p,void* out,size_t n);
void __fastcall periodChanged(A manager,int next);
std::vector<unsigned char> cleanEntry(A rva){
    if(rva==attendance::turnout::native::Formula)return {attendance::turnout::native::FormulaPrefix.begin(),attendance::turnout::native::FormulaPrefix.end()};
    if(rva==attendance::turnout::native::OptionSet)return {attendance::turnout::native::SetterPrefix.begin(),attendance::turnout::native::SetterPrefix.end()};
    if(rva==attendance::bindings::ReplayEventNodeUpdate)return {0x40,0x53,0x48,0x83,0xec,0x30,0x48,0x63,0x41,0x50,0x48,0x8b,0xd9,0x66,0xc7,0x41,0x1a,0x02,0x00};
    if(rva==attendance::bindings::ReplayNodeUpdate)return {0x48,0x89,0x5c,0x24,0x10,0x48,0x89,0x7c,0x24,0x18,0x55,0x48,0x8b,0xec,0x48,0x83,0xec,0x50};
    if(rva==attendance::bindings::CrowdVisibility)return {0x48,0x89,0x5c,0x24,0x08,0x48,0x89,0x6c,0x24,0x10,0x48,0x89,0x74,0x24,0x18};
    if(rva==attendance::bindings::PeriodChange)return {0x40,0x53,0x48,0x83,0xec,0x20,0x48,0x8b,0x99,0x30,0x01,0,0,0x48,0x89,0x6c,0x24,0x30};
    if(rva==attendance::bindings::AudioUpdate)return {0x40,0x53,0x48,0x83,0xec,0x20,0x48,0x8b,0xd9,0x48,0x8b,0x89,0x38,0x02,0,0};
    if(rva==attendance::bindings::IntensityUpdate)return {0x48,0x89,0x5c,0x24,0x10,0x48,0x89,0x6c,0x24,0x18,0x56,0x57,0x41,0x56,0x48,0x83,0xec,0x20};
    if(rva==attendance::bindings::SwellSend)return {0x4c,0x8b,0xdc,0x55,0x41,0x56,0x49,0x8d,0x6b,0xd8,0x48,0x81,0xec,0x18,1,0,0};
    if(rva==attendance::bindings::MuseLogicEvaluate)return {0x48,0x8b,0xc4,0x55,0x48,0x83,0xec,0x50,0x44,0x8b,0x41,0x48,0x40,0xb5,0x01};
    if(rva==attendance::bindings::MuseComparisonValue)return {0x48,0x89,0x5c,0x24,0x10,0x48,0x89,0x7c,0x24,0x18,0x4c,0x89,0x74,0x24,0x20};
    return {};
}
struct Patch {A site=0;void* trampoline=nullptr;std::vector<unsigned char> original,replacement;};
std::vector<Patch> patches;
void jump(unsigned char* p,A target){p[0]=0xff;p[1]=0x25;memset(p+2,0,4);memcpy(p+6,&target,8);}
bool makePatch(A rva,void* detour,void** original,size_t length){
    if(length<14||length>32||!detour||!original)return false;
    auto clean=cleanEntry(rva);
    if(!clean.empty()&&clean.size()!=length)return false;
    Patch p;p.site=image+rva;p.original.resize(length);p.replacement.assign(length,0x90);
    if(!read(p.site,p.original.data(),length))return false;
    std::vector<unsigned char> code;
    // Copy only the complete, verified prologue for this hook.
    if(rva==attendance::bindings::ReplayUpdate){
        if(length!=attendance::replayhook::SourceBytes||std::memcmp(p.original.data(),attendance::replayhook::Prefix.data(),length))return false;
        auto bytes=attendance::replayhook::trampoline(image+attendance::bindings::ReplayManager,p.site+length);code.assign(bytes.begin(),bytes.end());
    }else if(rva==attendance::turnout::launchhook::Site){
        const auto& prefix=attendance::turnout::launchhook::Prefix;
        if(length!=prefix.size()||std::memcmp(p.original.data(),prefix.data(),length))return false;
        auto bytes=attendance::turnout::launchhook::trampoline(image+attendance::turnout::launchhook::NativeTeamSetup,p.site+length);code.assign(bytes.begin(),bytes.end());
    }else {code.resize(length+14);memcpy(code.data(),clean.empty()?p.original.data():clean.data(),length);jump(code.data()+length,p.site+length);}
    size_t allocated=code.size();
    p.trampoline=VirtualAlloc(nullptr,allocated,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);if(!p.trampoline)return false;
    std::memcpy(p.trampoline,code.data(),allocated);
    DWORD old=0;if(!VirtualProtect(p.trampoline,allocated,PAGE_EXECUTE_READ,&old)){VirtualFree(p.trampoline,0,MEM_RELEASE);return false;}
    FlushInstructionCache(GetCurrentProcess(),p.trampoline,allocated);jump(p.replacement.data(),reinterpret_cast<A>(detour));
    *original=p.trampoline;patches.push_back(std::move(p));return true;
}
bool commitPatches(){
    // Complete prologues; the replay updater expands its verified RIP-relative load.
    std::vector<HANDLE> frozen;frozen.reserve(4096);bool okay=true;
    std::vector<DWORD> protection(patches.size());
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);if(snapshot==INVALID_HANDLE_VALUE)return false;
    THREADENTRY32 e{};e.dwSize=sizeof(e);
    if(Thread32First(snapshot,&e))do{
        if(e.th32OwnerProcessID!=GetCurrentProcessId()||e.th32ThreadID==GetCurrentThreadId())continue;
        HANDLE t=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT|THREAD_SET_CONTEXT,FALSE,e.th32ThreadID);
        if(!t){if(GetLastError()==ERROR_INVALID_PARAMETER)continue;okay=false;break;}
        if(SuspendThread(t)==DWORD(-1)){CloseHandle(t);okay=false;break;}frozen.push_back(t);
    }while(Thread32Next(snapshot,&e));
    CloseHandle(snapshot);
    if(okay)for(HANDLE t:frozen){
        CONTEXT c{};c.ContextFlags=CONTEXT_CONTROL;if(!GetThreadContext(t,&c)){okay=false;break;}
        for(const auto& p:patches)if(c.Rip>=p.site&&c.Rip<p.site+p.original.size()){
            c.Rip=reinterpret_cast<A>(p.trampoline)+c.Rip-p.site;if(!SetThreadContext(t,&c))okay=false;
        }
    }
    if(okay)for(const auto& p:patches){unsigned char actual[32]{};if(p.original.size()>sizeof(actual)||!read(p.site,actual,p.original.size())||memcmp(actual,p.original.data(),p.original.size()))okay=false;}
    // Make every target writable before any code is changed; roll back protections on failure.
    size_t prepared=0;
    if(okay)for(size_t i=0;i<patches.size();++i){if(!VirtualProtect(reinterpret_cast<void*>(patches[i].site),patches[i].original.size(),PAGE_EXECUTE_READWRITE,&protection[i])){okay=false;break;}++prepared;}
    if(okay)for(const auto& p:patches){memcpy(reinterpret_cast<void*>(p.site),p.replacement.data(),p.replacement.size());FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(p.site),p.replacement.size());}
    for(size_t i=0;i<prepared;++i){DWORD old=0;VirtualProtect(reinterpret_cast<void*>(patches[i].site),patches[i].original.size(),protection[i],&old);}
    for(HANDLE t:frozen){ResumeThread(t);CloseHandle(t);}return okay;
}

attendance::LogQueue queuedLogs;
attendance::timing::Counts atmosphereTimings,decisionTimings,crowdApplyTimings,replayCrowdTimings,nativeCrowdTimings,naturalCrowdTimings;
attendance::timing::Counts seatPreparationTimings,crowdMeasurementTimings;
attendance::timing::Counts replaySelectorTimings,replaySelectorIntervals,replayOriginalTimings,majorReplayTimings;
attendance::timing::Counts replayEventNativeTimings,replayNodeNativeTimings;
std::atomic<uint64_t> replaySelectorWorkNs{0},replayMajorSelectorSamples{0};
void flushReplaySelectorTiming(){auto ns=replaySelectorWorkNs.exchange(0);if(ns)replaySelectorIntervals.record(ns/1000);}
std::atomic<uint64_t> forcedCrowdPasses{0},unchangedReplaySkips{0};
std::atomic<int> appliedVisibilityTarget{-1},appliedVisibilityMode{-1};
void drainLogs(){
    std::vector<std::string> lines;
    {std::lock_guard<std::mutex> guard(logMutex);lines=queuedLogs.take();}
    if(lines.empty())return;
    auto limit=static_cast<uintmax_t>(logLimitMB.load())*1024*1024;
    if(!attendance::appendLogBatch(logsFolder/L"technical.log",lines,limit))++logWriteFailures;
    std::vector<std::string> readable;readable.reserve(lines.size());
    for(const auto& line:lines){auto summary=attendance::readable::event(line);if(!summary.empty())readable.push_back(std::move(summary));}
    if(!attendance::appendLogBatch(logsFolder/L"DynamicGamedayAtmosphere.log",readable,limit))++logWriteFailures;
}
void log(const char* event,const std::string& message) {
    std::lock_guard<std::mutex> guard(logMutex);
    const std::string kind=event;
    if(!debugLog&&!attendance::importantLogTag(kind,message))return;
    if(!debugLog&&kind=="arrivals"&&message.find("starting at")==std::string::npos&&message.find("complete")==std::string::npos&&message.find("target reached")==std::string::npos)return;
    static attendance::DuplicateLogFilter repeated;
    if(!repeated.accept(kind+"\n"+message,GetTickCount64()))return;
    SYSTEMTIME local{};GetLocalTime(&local);char timestamp[40]{};
    sprintf_s(timestamp,"%04u-%02u-%02u %02u:%02u:%02u",local.wYear,local.wMonth,local.wDay,local.wHour,local.wMinute,local.wSecond);
    queuedLogs.push(std::string(timestamp)+" | ["+kind+"] "+message+"\n");
}
std::string hex(A a){char s[32]{};sprintf_s(s,"0x%llx",static_cast<unsigned long long>(a));return s;}
bool read(A p,void* out,size_t n){SIZE_T done=0;return p>=0x10000&&ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(p),out,n,&done)&&done==n;}
template<class T>T at(A p){return attendance::nativeValue<T>(p,read);}
attendance::turnout::ModeWitness initialModeProof(){
    int observedMode=-2;bool readable=attendance::turnout::native::setupMode(image,observedMode);
    const auto now=GetTickCount64();attendance::turnout::ModeWitness proof;
    {std::lock_guard<std::mutex> guard(initialMutex);
        if(!initialModeWitness.observe(observedMode,readable,now))initialSeeds.clear();proof=initialModeWitness;
    }
    if(initialObservedSetupMode.exchange(readable?observedMode:-2)!=(readable?observedMode:-2))
        log("starting-attendance-state","Native setup mode "+std::to_string(observedMode)+"; readable "+std::to_string(readable)+"; Dynasty witness "+(proof.armed?"armed":"not armed")+"; generation "+std::to_string(proof.generation)+"; verified tick "+std::to_string(proof.verifiedAt)+"; current tick "+std::to_string(now)+".");
    return proof;
}
bool initialTransitionProof(const attendance::turnout::Seed& seed,const attendance::turnout::Launch& launch){
    auto proof=initialModeProof();return proof.armed&&attendance::turnout::transitionProof(seed,launch,GetTickCount64(),proof.generation);
}
bool crowdTransitionProof(const attendance::turnout::Seed& seed,const attendance::turnout::Launch& launch){
    auto proof=initialModeProof();return proof.armed&&attendance::turnout::capturedLaunchProof(seed,launch,GetTickCount64(),proof.generation);
}
int __fastcall initialFormula(A manager,A record){
    A caller=reinterpret_cast<A>(_ReturnAddress());
    bool observe=caller==image+attendance::turnout::native::PlayableFormulaReturn&&enabled&&std::atomic_load(&settings)->enabled;
    if(observe)initialModeProof();
    int native=originalInitialFormula(manager,record);
    // Observation only: simulated/saved attendance always receives EA's result.
    if(observe)try{
        std::shared_ptr<const attendance::turnout::Seed> empty=std::make_shared<attendance::turnout::Seed>();std::atomic_store(&crowdIdentity,empty);
        auto proof=initialModeProof();
        if(!proof.armed||!attendance::turnout::native::dynastySetup(image,true)){
            int observedMode=-2;bool modeReadable=attendance::turnout::native::setupMode(image,observedMode);
            log("starting-attendance-inactive","Playable input observation has no recent verified Dynasty setup; EA turnout preserved. Native setup mode "+std::to_string(observedMode)+"; readable "+std::to_string(modeReadable)+"; witness armed "+std::to_string(proof.armed)+"; generation "+std::to_string(proof.generation)+"; verified tick "+std::to_string(proof.verifiedAt)+"; current tick "+std::to_string(GetTickCount64())+".");return native;
        }
        attendance::turnout::Seed seed;
        attendance::turnout::Seed schools;
        if(attendance::turnout::native::crowdIdentity(image,manager,record,schools)){
            std::shared_ptr<const attendance::turnout::Seed> identity=std::make_shared<attendance::turnout::Seed>(schools);std::atomic_store(&crowdIdentity,identity);
        }
        if(attendance::turnout::native::capture(image,manager,record,native,seed)){
            if(audienceBindings)attendance::audiencenative::identity(image,manager,seed.dynasty);
            seed.dynastyVerifiedAt=proof.verifiedAt;seed.dynastyGeneration=proof.generation;
            std::shared_ptr<const attendance::turnout::Seed> identity=std::make_shared<attendance::turnout::Seed>(seed);std::atomic_store(&crowdIdentity,identity);
            {std::lock_guard<std::mutex> guard(initialMutex);initialSeeds.clear();
             if(initialModeWitness.armed&&initialModeWitness.generation==proof.generation)initialSeeds.remember(seed,GetTickCount64());}
            if(seed.regular&&!seed.resumed){
                attendance::audiencenative::CaptureTrace trace;
                bool captured=false;
                if(!audienceBindings)trace.fail("native audience field labels/ordinals not verified");
                else if(!audienceStorageReady)trace.fail("audience foundation storage unavailable");
                else captured=attendance::audiencenative::capture(image,manager,record,seed,nullptr,&trace);
                std::shared_ptr<const AudienceCaptureStatus> readStatus=std::make_shared<AudienceCaptureStatus>(AudienceCaptureStatus{seed.home.id,seed.away.id,captured,trace.describe()});
                std::atomic_store(&audienceCaptureStatus,readStatus);
                if(!captured){
                    log("starting-attendance-inactive","New audience read failed for "+std::to_string(seed.home.id)+" / "+std::to_string(seed.away.id)+": "+trace.describe()+". This launch keeps EA turnout; no inferred history or cross-dynasty audience is used.");return native;
                }
                log("audience-inputs","Native audience read verified for "+std::to_string(seed.home.id)+" / "+std::to_string(seed.away.id)+"; dynasty "+std::to_string(seed.dynasty)+"; source active statistics manager -> league history -> dynasty; owning league matched; "+trace.describe()+".");
                std::lock_guard<std::mutex> audienceGuard(audienceMutex);attendance::audience::Foundation home,away;
                if(!audienceBook.resolve(seed.dynasty,seed.home.program,home)||
                   (!attendance::turnout::fcs(seed.away.id)&&!audienceBook.resolve(seed.dynasty,seed.away.program,away))){
                    log("starting-attendance-inactive","Audience foundation could not be prepared; EA turnout preserved.");return native;
                }
                seed.home.audienceFans=attendance::audience::demand(home,seed.home.program);
                seed.home.opponentRetention=attendance::audience::opponentRetention(home,seed.home.program);
                if(!attendance::turnout::fcs(seed.away.id)){
                    seed.away.audienceFans=attendance::audience::demand(away,seed.away.program);
                    seed.away.opponentRetention=attendance::audience::opponentRetention(away,seed.away.program);
                }
            }
            seed.dynastyVerifiedAt=proof.verifiedAt;seed.dynastyGeneration=proof.generation;
            std::shared_ptr<const attendance::turnout::Seed> finalIdentity=std::make_shared<attendance::turnout::Seed>(seed);std::atomic_store(&crowdIdentity,finalIdentity);
            {std::lock_guard<std::mutex> guard(initialMutex);if(!initialModeWitness.armed||initialModeWitness.generation!=proof.generation)return native;initialSeeds.remember(seed,GetTickCount64());}
            log("starting-attendance","Fresh Dynasty launch inputs captured for "+std::to_string(seed.home.id)+" / "+std::to_string(seed.away.id)+"; native attendance "+std::to_string(native)+"; stadium "+std::to_string(seed.stadium)+"; captured capacity "+std::to_string(seed.capacity)+".");
        }else {
            {std::lock_guard<std::mutex> guard(initialMutex);initialSeeds.clear();}
            if(!initialCaptureFault.exchange(true))log("starting-attendance-inactive","Native matchup inputs unavailable; that launch keeps EA's attendance. No saved/simulated attendance changed.");
        }
    }catch(...){if(!initialCaptureFault.exchange(true))log("starting-attendance-inactive","Input observation failed; native attendance remains available.");}
    return native;
}
void initialLaunch(){
    using namespace attendance::turnout;
    Launch launch;
    if(!enabled||!std::atomic_load(&settings)->enabled||mode!=0)return;
    initialModeProof();
    Launch identity;if(!native::launchIdentity(image,identity)){log("starting-attendance-inactive","Launch identity descriptors unavailable; EA turnout preserved.");return;}
    Seed seed;{std::lock_guard<std::mutex> guard(initialMutex);if(!initialSeeds.find(identity,GetTickCount64(),seed)||!seed.dynastyVerifiedAt){
        log("starting-attendance","EA attendance preserved: no fresh matching Dynasty launch plan was captured. See preceding launch/audience warnings for the reason.");return;
    }}
    const char* failure=nullptr;
    bool transition=crowdTransitionProof(seed,identity);
    if(!native::launch(image,launch,seed,&failure,transition)){log("starting-attendance-inactive",std::string("Shared launch check failed: ")+(failure?failure:"unknown")+"; EA turnout preserved.");return;}
    if(launch.home!=identity.home||launch.away!=identity.away||launch.stadium!=identity.stadium||launch.ea!=identity.ea){log("starting-attendance-inactive","Launch identity changed during input validation; EA turnout preserved.");return;}
    if(seed.homeStadium<0){log("starting-attendance","EA attendance preserved: native home-venue inputs unavailable.");return;}
    launch.neutral=launch.stadium!=seed.homeStadium&&launch.stadium!=seed.homeAlternate;
    std::shared_ptr<const CrowdFixture> fixture=std::make_shared<CrowdFixture>(CrowdFixture{seed,launch,true});std::atomic_store(&crowdFixture,fixture);
    if(!initialSetting){log("starting-attendance","EA attendance preserved: revised turnout disabled; verified crowd identity remains available for visiting fans and protected sections.");return;}
    if(!seed.regular||seed.resumed){log("starting-attendance","EA attendance preserved: postseason, non-regular-season or resumed game.");return;}
    Result result=calculateAudience(seed,launch,attendanceProfile.load()==0);
    if(!result.applied){log("starting-attendance","EA attendance preserved: postseason, non-regular-season, resumed game, or incomplete inputs.");return;}
    result.kickoff=openingPercent(result.standing,seed.home.id,seed.home.rank,seed.away.rank,launch.neutral,result.night,launch.rivalry,seed.home.homeGames==0,launch.precipitation,seed.roof>=1&&seed.roof<=3,arrivalStart,arrivalLateStart,arrivalBigBonus,result.percent,launch.capacity,sameState(seed),result.fuller);
    result.exceptional=exceptional(result.percent,result.standing,seed.home.id,seed.home.rank,seed.away.rank,launch.neutral,result.night,launch.rivalry,seed.home.homeGames==0,arrivalBigBonus);
    result.fillPercent=!launch.neutral&&lateArrivalSchool(seed.home.id)?arrivalLateFinish.load():big(seed.home.rank,seed.away.rank,launch.rivalry)?30:arrivalFinish.load();
    applyShowcase(seed,launch,result);
    if(result.showcaseEvent>=0)log("starting-attendance",std::string("Showcase rivalry: ")+attendance::showcase::event(result.showcaseEvent).name+"; native rivalry event "+std::to_string(result.showcaseEvent)+"; full peak crowd, "+std::to_string(result.kickoff)+"% seated at kickoff. Team IDs do not select this rule.");
    std::shared_ptr<const Applied> publish=std::make_shared<Applied>(Applied{seed,launch,result});
    auto api=native::Api::engine(image);api.set=originalInitialOptionSet;
    if(!native::setAttendance(image,result.attendance,&api,initialTransitionProof(seed,launch))){
        native::setAttendance(image,launch.ea,&api,initialTransitionProof(seed,launch));
        log("starting-attendance-inactive","Launch count could not be verified; requested native attendance restoration.");return;
    }
    std::atomic_store(&initialApplied,publish);
    std::ostringstream message;message<<std::fixed<<std::setprecision(1)<<"Regular-season turnout ("<<(result.fuller?"Fuller Crowds":"Default")<<", peak bonus "<<result.profileBoost<<" points): EA "<<launch.ea<<" ("<<100.f*launch.ea/launch.capacity<<"%); DGA "<<result.attendance<<" ("<<result.percent<<"% of "<<launch.capacity<<"). Support standing "<<100*result.standing<<"%, weighted score "<<100*result.score<<" across "<<seed.schools<<" FBS schools; home grades atmosphere/tradition/brand/conference "<<seed.home.atmosphere<<"/"<<seed.home.tradition<<"/"<<seed.home.brand<<"/"<<seed.home.conference<<" (A+=0, F=12), prestige "<<seed.home.prestige<<"/10, AP/media rank "<<seed.home.apRank<<", general rank "<<seed.home.rank<<"; native school states "<<seed.home.state<<"/"<<seed.away.state<<". Demand "<<result.demand<<", season "<<result.momentum<<", matchup "<<result.appeal<<", night "<<result.nightBonus<<", modifier limit "<<result.adjustmentLimit<<", weather policy "<<(launch.weatherKnown?(launch.dynamicWeather?"dynamic forecast; no guaranteed-rain penalty":"verified static precipitation"):"unverified forecast; no launch penalty")<<", configured precipitation "<<launch.configuredPrecipitation<<", weather "<<result.weather<<", variation "<<result.variation<<", breakout "<<result.breakout<<", hard-to-fill deduction "<<result.hardToFillPenalty<<" points, event multiplier "<<result.eventMultiplier<<", opener/night/senior/local/academy bonuses "<<100*result.openerBoost<<"/"<<100*result.nightBoost<<"/"<<100*result.seniorBoost<<"/"<<100*result.stateBoost<<"/"<<100*result.academyBoost<<"%; schedule "<<(launch.seniorKnown?"native senior-day flag verified":"senior-day flag unavailable")<<"; access +"<<result.accessBonus<<" kickoff points; floor/cap/showcase "<<result.limits<<" points; local kickoff "<<launch.minutes/60<<":"<<std::setfill('0')<<std::setw(2)<<launch.minutes%60<<"; "<<(result.night?"night":"day")<<"; "<<result.kickoff<<"% of expected fans seated at kickoff ("<<(result.exceptional?"exceptional opening":"ordinary/marquee opening")<<"); arrivals finish "<<result.fillPercent<<"% through Q1. Saved/simulated attendance unchanged.";
    if(result.audienceModel){message<<" Audience model: "<<result.audienceFans<<" ordinary interested fans before matchup/weather; weights prestige40/atmosphere30/recent20/AP10; dynasty "<<seed.dynasty<<"; home program row "<<seed.home.program.row<<"; "<<(seed.home.program.custom?"TeamBuilder: pre-dynasty seasons excluded":"established program")<<"; recent completed-season win rate "<<attendance::audience::recent(seed.home.program)<<" (-1 means unavailable); evidence-based turnout retention home/away "<<100*seed.home.opponentRetention<<"/"<<100*seed.away.opponentRetention<<"%; weak-opponent relief "<<result.opponentRelief<<" points on fixed 75 reference. No annual growth bonus. Weather/variation act on fans; Fuller/showcase are explicit capacity overrides.";}
    log("starting-attendance",message.str());
}
bool __fastcall initialOptionSet(A service,unsigned key,A reference){
    A caller=reinterpret_cast<A>(_ReturnAddress());
    bool result=originalInitialOptionSet(service,key,reference);
    if(key==0x9f2e6900&&service==at<A>(image+attendance::turnout::native::OptionService))try{
        if(caller==image+attendance::turnout::native::LaunchResetReturn){
            std::shared_ptr<const attendance::turnout::Applied> empty=std::make_shared<attendance::turnout::Applied>();std::atomic_store(&initialApplied,empty);
            std::shared_ptr<const attendance::turnout::CrowdFixture> noFixture=std::make_shared<attendance::turnout::CrowdFixture>();std::atomic_store(&crowdFixture,noFixture);
            appearancePlanSource=0;appearanceShare=0;appearanceMiles=-1;appearanceTravel=0;
            {std::lock_guard<std::mutex> guard(generatedCrowdMutex);for(auto& entry:generatedCrowds)entry.reset();nextGeneratedCrowd=0;}
        }
    }catch(...){log("starting-attendance-inactive","Launch turnout unavailable; other atmosphere features remain active.");}
    return result;
}
void initialLaunchBoundary(){
    try{initialLaunch();}catch(...){log("starting-attendance-inactive","Shared launch boundary failed; native attendance preserved.");}
}
bool prepareInitialLaunch(){
    using namespace attendance::turnout::launchhook;
    initialLaunchGate=VirtualAlloc(nullptr,256,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);if(!initialLaunchGate)return false;
    if(!makePatch(Site,initialLaunchGate,&initialLaunchOriginal,SourceBytes)){VirtualFree(initialLaunchGate,0,MEM_RELEASE);initialLaunchGate=nullptr;return false;}
    auto code=gate(reinterpret_cast<A>(initialLaunchBoundary),reinterpret_cast<A>(initialLaunchOriginal));
    memcpy(initialLaunchGate,code.data(),code.size());DWORD old=0;
    if(!VirtualProtect(initialLaunchGate,256,PAGE_EXECUTE_READ,&old))return false;
    FlushInstructionCache(GetCurrentProcess(),initialLaunchGate,code.size());return true;
}
const char* __fastcall pauseMenuText(A tile,A key,A){
    try{
        if(!enabled||!pauseMenuSetting||!pauseMenuAvailable||mode!=0||!pauseBinding||!replayBinding)return nullptr;
        char action[16]{},field[16]{};
        if(at<A>(tile)!=image+attendance::pausemenu::TileVtable||!read(tile+0xd50,action,sizeof(action))||
           !read(key,field,sizeof(field)))return nullptr;
        action[15]=field[15]=0;
        if(!attendance::pausemenu::labelField(action,field))return nullptr;
        attendance::pausemenu::Context c;c.enabled=std::atomic_load(&settings)->enabled;
        A wrapper=at<A>(image+attendance::bindings::MatchWrapper),configuration=at<A>(image+0xf63f9f0);
        c.match=at<A>(wrapper+16);c.epoch=audioEpoch.load();
        unsigned char online=1,paused=0,replaying=1;A pause=at<A>(image+attendance::bindings::PauseFlag);
        A replay=at<A>(image+attendance::bindings::ReplayManager);
        c.offlineDynasty=configuration&&read(configuration+0x84,&online,1)&&online==0&&
            attendance::scope::matchMode(at<int>(image+0xd9eb808))&&at<int>(image+0xf3fddbc)==3&&
            c.match==audioMatch.load()&&at<A>(c.match)==image+attendance::bindings::MatchVtable;
        c.paused=read(pause,&paused,1)&&paused==1;
        c.replay=at<A>(replay)!=image+attendance::bindings::ReplayVtable||!read(replay+0x38,&replaying,1)||replaying!=0||replaySuspendsAtmosphere.load();
        {std::lock_guard<std::mutex> guard(historyMutex);c.leaving=matchLifecycle.leftWorld||matchLifecycle.match!=c.match;}
        auto density=std::atomic_load(&renderedOccupancy);c.mapped=density->valid();c.measuredMatch=density->match;c.measuredEpoch=density->epoch;c.peak=density->peak();
        c.loss=emptyCrowdActive?100:activeLoss.load();c.arrivalMissing=emptyCrowdActive?0:arrivalMissing.load();
        auto supporters=std::atomic_load(&visitorPopulation);
        if(!emptyCrowdActive&&density->owns(c.match,c.epoch)&&supporters->owns(c.match,c.epoch)){
            c.peak=static_cast<float>(static_cast<double>(supporters->home+supporters->away)/density->seats);c.loss=c.arrivalMissing=0;
        }
        EmptyWorld empty;bool emptyReadout=emptyCrowdActive&&emptyWorld(empty)&&empty.match==c.match;
        if(emptyReadout){c.enabled=c.offlineDynasty=c.mapped=true;c.leaving=false;c.measuredMatch=c.match;c.measuredEpoch=c.epoch;c.peak=1;}
        if(audioEpoch.load()!=c.epoch||(!emptyReadout&&audioMatch.load()!=c.match)||at<A>(wrapper+16)!=c.match)return nullptr;
        int value=attendance::pausemenu::percent(c);const char* label=pauseMenuLabels.get(value);
        if(label){pauseMenuLastPercent=value;++pauseMenuDisplays;}
        return label;
    }catch(...){return nullptr;} // A readout failure must leave the native menu usable.
}
bool bytes(A rva,const std::vector<unsigned char>& expected);
bool preparePauseMenu(){
    const auto& prefix=attendance::pausemenu::Prefix;
    if(!pauseBinding||!replayBinding||!bytes(attendance::pausemenu::Site,std::vector<unsigned char>(prefix.begin(),prefix.end())))return false;
    A call=image+attendance::pausemenu::Site+prefix.size();
    if(at<unsigned char>(call)!=0xe8||call+5+at<int32_t>(call+1)!=image+attendance::pausemenu::Writer)return false;
    auto code=attendance::pausemenu::trampoline(reinterpret_cast<A>(pauseMenuText),call);
    pauseMenuGate=VirtualAlloc(nullptr,code.size(),MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);if(!pauseMenuGate)return false;
    memcpy(pauseMenuGate,code.data(),code.size());DWORD old=0;
    if(!VirtualProtect(pauseMenuGate,code.size(),PAGE_EXECUTE_READ,&old)){
        VirtualFree(pauseMenuGate,0,MEM_RELEASE);pauseMenuGate=nullptr;return false;
    }
    FlushInstructionCache(GetCurrentProcess(),pauseMenuGate,code.size());
    if(!makePatch(attendance::pausemenu::Site,pauseMenuGate,&pauseMenuOriginal,prefix.size())){
        VirtualFree(pauseMenuGate,0,MEM_RELEASE);pauseMenuGate=nullptr;return false;
    }
    return true;
}
void observeWorldExit(){
    auto offline=[](){A configuration=at<A>(image+0xf63f9f0);return configuration&&
        attendance::scope::offlineMatch(at<int>(image+0xd9eb808),at<int>(image+0xf3fddbc),at<unsigned char>(configuration+0x84));};
    if(offline())return;
    std::lock_guard<std::mutex> guard(historyMutex);
    // Main-menu Practice shares mode 17 with Play Now. It must retire the
    // match lease even when the game retains or later reuses match memory.
    if(!offline())matchLifecycle.leave();
}
// These small expression objects belong to the synchronous native evaluator
// calling our hook, in THIS process. A bounded SEH-protected copy avoids a
// ReadProcessMemory system call for every field/shape test. Failed reads still
// reject the selector; object identities are never cached between evaluations.
bool readReplayObject(A p,void* out,size_t n){
    if(p<0x10000||!out||!n||n>256||p>UINTPTR_MAX-n)return false;
    __try{std::memcpy(out,reinterpret_cast<const void*>(p),n);return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
template<class T>T replayValue(A p){return attendance::nativeValue<T>(p,readReplayObject);}
attendance::replaypresentation::View replayPresentationView(){
    using namespace attendance::bindings;
    return {readReplayObject,{image+MuseLogicVtable,image+MuseComparisonVtable,image+MuseConstantVtable,image+MuseFunctionVtable,image+MuseActorFunctionVtable}};
}
bool replayPresentationAllowed(){
    if(attendance::energy::clockManagementType(replaySnapType.load()))return false;
    if(!enabled||mode==2||!extraReplays||!replayPresentationInstalled||replayPresentationFailed||!pauseBinding||!replayBinding)return false;
    if(replayValue<int>(image+0xd9eb808)!=14||replayValue<int>(image+0xf3fddbc)!=3)return false;
    A states=replayValue<A>(image+attendance::bindings::GameStates);
    if(replayValue<A>(states)!=image+attendance::bindings::GameStatesVtable||replayValue<int>(states+0xa0)!=4)return false;
    A configuration=replayValue<A>(image+0xf63f9f0),wrapper=replayValue<A>(image+attendance::bindings::MatchWrapper);
    A match=replayValue<A>(wrapper+16),manager=replayValue<A>(image+attendance::bindings::ReplayManager);
    A pause=replayValue<A>(image+attendance::bindings::PauseFlag);
    unsigned char online=1,active=1,paused=1;
    return configuration&&readReplayObject(configuration+0x84,&online,1)&&online==0&&
        replayValue<A>(match)==image+attendance::bindings::MatchVtable&&replayValue<A>(manager)==image+attendance::bindings::ReplayVtable&&
        readReplayObject(manager+0x38,&active,1)&&active==0&&readReplayObject(pause,&paused,1)&&paused==0;
}
bool writeReplayBoolean(A address,bool value=true){
    // Only an existing native eligibility result is written. A failed write
    // disables these additions without changing original presentation calls.
    __try{*reinterpret_cast<unsigned char*>(address)=static_cast<unsigned char>(value);return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){replayPresentationFailed=true;return false;}
}
void recordReplaySelection(A node,int family,int native,int output,bool allowed,int phase){
    if(!enabled||mode!=0||at<int>(image+0xd9eb808)!=14||at<int>(image+0xf3fddbc)!=3)return;
    A wrapper=at<A>(image+attendance::bindings::MatchWrapper),match=at<A>(wrapper+16);
    if(at<A>(match)!=image+attendance::bindings::MatchVtable)return;
    attendance::replaytrace::Record r;r.node=node;r.at=GetTickCount64();r.family=family;r.phase=phase;r.native=native;r.output=output;r.allowed=allowed;
    uint64_t sequence=0;unsigned epoch=audioEpoch.load();
    {
        std::lock_guard<std::mutex> lock(majorReplayMutex);
        if(replayTrackerMatch!=match||replayTrackerEpoch!=epoch)return;
        sequence=replayPlayTracker.serial;r.armed=static_cast<int>(majorReplay.kind());r.stage=static_cast<int>(majorReplay.stage());
        r.startHome=replayPlayTracker.start.game.homeScore;r.startAway=replayPlayTracker.start.game.awayScore;r.down=replayPlayTracker.start.down;
        r.result=replayObserved.resultStamp;r.startResult=replayPlayTracker.startStamp;
    }
    r.home=at<int>(match+0x1d8);r.away=at<int>(match+0x1dc);
    A replay=at<A>(image+attendance::bindings::ReplayManager);r.replaying=at<A>(replay)==image+attendance::bindings::ReplayVtable&&at<unsigned char>(replay+0x38)!=0;
    std::lock_guard<std::mutex> lock(replayTraceMutex);replayTrace.add(match,epoch,sequence,r);
}
void flushReplaySelection(){
    std::array<attendance::replaytrace::Record,128> rows{};std::array<uint64_t,128> sequences{};unsigned count=0;
    {std::lock_guard<std::mutex> lock(replayTraceMutex);replayTrace.flush([&](uint64_t sequence,const auto& r){if(count<rows.size()){rows[count]=r;sequences[count++]=sequence;}});}
    for(unsigned i=0;i<count;++i){const auto& r=rows[i];
        const char* family=r.family==1?"TD predicate":r.family==2?"takeaway predicate":r.family==3?"quick-presentation selector":r.family==4?"quick-TD selector":r.family==10?"native inset event":r.family==11?"native replay node entered":r.family==12?"native replay node completed":"other";
        std::ostringstream out;out<<"Play #"<<sequences[i]<<"; tick "<<r.at<<"; "<<family<<"; identity 0x"<<std::hex<<r.node<<std::dec
            <<"; phase "<<r.phase<<"; native/output "<<r.native<<"/"<<r.output<<"; modifications allowed "<<r.allowed
            <<"; armed kind/stage "<<r.armed<<"/"<<r.stage<<"; score "<<r.startHome<<"-"<<r.startAway<<" -> "<<r.home<<"-"<<r.away
            <<"; snap down "<<r.down<<"; result fresh "<<(r.result&&r.result!=r.startResult)<<"; playback active "<<r.replaying<<".";
        log("replay-select",out.str());
    }
}
bool __fastcall museLogicEvaluate(A node,A context){
    if(!enabled||mode==2||!extraReplays||!replayPresentationInstalled||replayPresentationFailed)return originalMuseLogic(node,context);
    attendance::timing::ExclusiveScope<> measured(replaySelectorTimings,&replaySelectorWorkNs);
    auto view=replayPresentationView();
    const bool root=replaySelection.depth==0;
    const bool contextAllowed=root&&replayPresentationAllowed();
    auto majorRoute=contextAllowed?view.major(node):attendance::replaypresentation::MajorRoute::None;
    auto identified=contextAllowed?view.identify(node):attendance::replaypresentation::Selection{};
    const bool recognized=majorRoute!=attendance::replaypresentation::MajorRoute::None||identified.fullComparison||identified.insetGate||identified.denyRoot;
    // Refresh only replay selectors, before any nested native comparison can
    // be changed. Missing/unfinished same-snap facts retain native behavior.
    if(recognized){++replayMajorSelectorSamples;guardedMajorReplay(false);}
    const bool allowed=contextAllowed&&recognized&&extraReplayForCurrentPlay();
    const bool fullRouting=majorReplayBindings&&!majorReplayFailed&&bigPlayReplays;
    attendance::majorreplay::Kind wantedMajor=attendance::majorreplay::Kind::None;
    if(majorRoute!=attendance::replaypresentation::MajorRoute::None&&!majorReplayFailed){
        std::lock_guard<std::mutex> guard(majorReplayMutex);
        if(majorReplay.routing(replayTrackerMatch,replayTrackerEpoch))wantedMajor=majorReplay.kind();
    }
    attendance::replaypresentation::Scope scope(replaySelection,view,node,
        allowed,bigPlayReplays.load(),routineInsets&&replayMode==attendance::replaymode::Mode::PlayCall,defensiveReplays.load());
    measured.pause();bool valid;
    if(root){attendance::timing::Scope nativeMeasured(replayOriginalTimings);valid=originalMuseLogic(node,context);}
    else valid=originalMuseLogic(node,context);
    measured.resume();
    int nativeValue=valid&&majorRoute!=attendance::replaypresentation::MajorRoute::None?(view.trueBoolean(node+0x38)?1:view.falseBoolean(node+0x38)?0:-1):-1;
    if(valid&&!majorReplayFailed&&majorRoute!=attendance::replaypresentation::MajorRoute::None){
        if(majorRoute==attendance::replaypresentation::MajorRoute::Touchdown&&view.trueBoolean(node+0x38)){
            std::lock_guard<std::mutex> guard(majorReplayMutex);
            replayTouchdown.nativeTouchdown();majorReplay.reset();wantedMajor=attendance::majorreplay::Kind::None;
        }
        if(fullRouting&&extraReplayForCurrentPlay()&&majorRoute==attendance::replaypresentation::MajorRoute::Takeaway&&view.trueBoolean(node+0x38)&&view.challengeExcluded(node)){
            std::lock_guard<std::mutex> guard(majorReplayMutex);
            const auto& start=replayPlayTracker.start;
            if(replayPlayTracker.serial&&attendance::energy::normalDown(start)&&!start.specialTeams&&
               replayObserved.offense==1-start.offense&&replayObserved.resultStamp&&replayObserved.resultStamp!=replayPlayTracker.startStamp){
                majorReplay.arm(replayTrackerMatch,replayTrackerEpoch,GetTickCount64(),replayPlayTracker.serial,attendance::majorreplay::Kind::Takeaway,replayMode.load());
                wantedMajor=majorReplay.kind();
            }
        }
        using R=attendance::replaypresentation::MajorRoute;using K=attendance::majorreplay::Kind;
        bool promote=fullRouting&&extraReplayForCurrentPlay()&&majorRoute==R::Takeaway&&wantedMajor==K::Takeaway&&view.challengeExcluded(node);
        bool decline=fullRouting&&extraReplayForCurrentPlay()&&wantedMajor==K::Takeaway&&majorRoute==R::QuickSkip;
        if(promote&&view.falseBoolean(node+0x38)&&writeReplayBoolean(node+0x38))++fullEligibilityChanges;
        if(decline&&view.trueBoolean(node+0x38)&&writeReplayBoolean(node+0x38,false))++fullEligibilityChanges;
    }
    if(majorRoute!=attendance::replaypresentation::MajorRoute::None)
        recordReplaySelection(node,static_cast<int>(majorRoute),nativeValue,valid?(view.trueBoolean(node+0x38)?1:view.falseBoolean(node+0x38)?0:-1):-1,allowed,at<int>(at<A>(image+attendance::bindings::GameStates)+0xa0));
    // A Logic Eval's return means its cached variant is valid, not that its
    // Boolean is true. The native parent reads this cache after Eval returns.
    if(valid&&node==replaySelection.selection.insetGate&&extraReplayForCurrentPlay()&&extraReplays&&routineInsets&&!replayPresentationFailed&&
       view.falseBoolean(node+0x38)&&writeReplayBoolean(node+0x38))++insetEligibilityChanges;
    // Decline only complete authored shortcut selectors, never the underlying
    // sack/tackle facts. Standard Flow then chooses the native sack replay, or
    // the routine run/pass inset instead of the replay-less TFL cutaway.
    if(valid&&node==replaySelection.selection.denyRoot&&extraReplayForCurrentPlay()&&extraReplays&&defensiveReplays&&!replayPresentationFailed&&
       view.trueBoolean(node+0x38)&&writeReplayBoolean(node+0x38,false))++defensiveRouteChanges;
    return valid;
}
A __fastcall museComparisonValue(A node,A output){
    if(!enabled||mode==2||!extraReplays||!replayPresentationInstalled||replayPresentationFailed)return originalMuseComparison(node,output);
    attendance::timing::ExclusiveScope<> measured(replaySelectorTimings,&replaySelectorWorkNs);
    measured.pause();A result=originalMuseComparison(node,output);measured.resume();
    if(node==replaySelection.selection.fullComparison&&extraReplayForCurrentPlay()&&extraReplays&&bigPlayReplays&&!replayPresentationFailed){
        auto view=replayPresentationView();
        if(result&&view.falseBoolean(result)&&view.fullEligible(node,replayMinimumYards.load())&&writeReplayBoolean(result))++fullEligibilityChanges;
    }
    return result;
}
attendance::audio::Wanted audioWanted(){
    ++audioContextSnapshots;
    attendance::audio::Wanted wanted;wanted.epoch=audioEpoch.load();wanted.match=audioMatch.load();
    // Replay may use different clocks. Ownership and match identity gate audio;
    // its target comes from the attendance checkpoint, not the replay's score.
    if(!emptyMode2020&&enabled&&audioSetting&&mode!=2&&std::atomic_load(&settings)->enabled&&wanted.match){
        ++audioWorldSnapshots;
        A wrapper=at<A>(image+attendance::bindings::MatchWrapper),actual=at<A>(wrapper+16),configuration=at<A>(image+0xf63f9f0);
        wanted.active=wanted.match==actual&&configuration&&at<unsigned char>(configuration+0x84)==0&&at<A>(actual)==image+0xb47e908&&attendance::scope::matchMode(at<int>(image+0xd9eb808))&&at<int>(image+0xf3fddbc)==3;
    }
    wanted.remaining=(100-(std::clamp)(activeLoss.load(),0,100))*0.01f;
    wanted.arrivalFraction=(100-(std::clamp)(arrivalMissing.load(),0,100))*0.01f;
    auto rendered=std::atomic_load(&renderedOccupancy);
    bool owned=rendered->owns(wanted.match,wanted.epoch);float peak=owned?rendered->peak():.75f;
    wanted.active=wanted.active&&owned;
    wanted.remaining=owned?wanted.remaining*peak:.75f;
    wanted.arrivalFraction=owned?wanted.arrivalFraction*peak:.75f;
    auto supporters=std::atomic_load(&visitorPopulation);
    if(owned&&supporters->owns(wanted.match,wanted.epoch))wanted.remaining=wanted.arrivalFraction=static_cast<float>(static_cast<double>(supporters->home+supporters->away)/rendered->seats);
    if(emptyMode2020){wanted.active=false;wanted.remaining=wanted.arrivalFraction=0;}
    return wanted;
}

attendance::balance::Mix audioBusMix(){
    auto wanted=audioWanted();auto density=std::atomic_load(&renderedOccupancy);
    auto now=GetTickCount64(),last=lastAudibleUpdate.load();
    if(!wanted.active||mode!=0||!audioAudible||now<last||now-last>750||!density->owns(wanted.match,wanted.epoch)||
       !attendance::balance::audiblePhase(audioPresentationPhase(),nativeUnpaused(),replaySuspendsAtmosphere.load()))return {};
    auto opening=std::atomic_load(&sustainedIntent);
    bool hostile=opening->active&&opening->match==wanted.match&&opening->epoch==wanted.epoch&&now>=opening->updated&&now-opening->updated<=750&&opening->hostileOpening&&opening->offense==1&&!shownNeutral;
    return attendance::balance::mix(audibleFullness(wanted),density->seats,busMaximumBoost.load(),static_cast<float>(busMixStrength.load()),hostile);
}

void audioNote(const char* message){log("audio",message);}
void settingsMenuNote(const char* message){log("settings-menu",message);}
void reactionNote(attendance::cheer::Source source,uint64_t id,attendance::cheer::Delivery delivery){
    using namespace attendance::cheer;
    const char* family=source==Source::Followup?"Staggered second cheer":source==Source::Live?"Live reaction":source==Source::Lineup?"Lineup cheer":source==Source::Result?"Play-result cheer":"Eruption cheer";
    const char* outcome=delivery==Delivery::Registered?"native cheer voice registered (audibility still depends on the native mix)":delivery==Delivery::Recovery?"native queue dropped the event selector; one same-size generic cheer/disappointment fallback sent":delivery==Delivery::Unregistered?"no native cheer voice registered within the short recovery window":delivery==Delivery::Sent?"native cheer event queued; playback not yet confirmed":
        delivery==Delivery::Expired?"scene/callback arrived after the cheer window; stale cheer skipped":"native cheer dispatch failed; sound control stopped";
    log("reaction-audio",std::string(family)+" #"+std::to_string(id)+": "+outcome+".");
}
void finishLiveAudit(const char* why){
    if(liveAuditId&&!positiveCheer.liveResolved(liveAuditId))log("reaction-audio","Live reaction #"+std::to_string(liveAuditId)+": "+why+"; no cheer dispatch confirmed.");
    liveAuditId=liveAuditUntil=0;
}
void sustainedNote(const char* message){
    std::string text=message;
    if(text=="big-game opening")text="Opening stadium atmosphere started.";
    else if(text=="overtime roar")text="Layered overtime atmosphere started.";
    else if(text=="winning celebration")text="Winning celebration roar started.";
    else if(text=="play-call atmosphere")text="Play-call atmosphere: sustained excitement, roar and swell; special lineup cheers only.";
    else if(text=="successful-play reaction")text="Successful-play reaction: fuller cheer and a sustained crowd response.";
    else if(text.find("fading out")!=std::string::npos)text="Added crowd roar fading out.";
    log("roar",text);
}

bool entityValid(A p){return at<A>(p)==image+0xb485368&&at<A>(at<A>(p+0x38)+8)==image+0xf50ab30;}
bool emptyWorld(EmptyWorld& world){
    // Empty stadiums use the verified shared layout in supported offline
    // matches. Dynasty starting-attendance formulas retain their own scope.
    int nativeMode=at<int>(image+0xd9eb808);
    if(nativeMode!=14){
        A configuration=at<A>(image+0xf63f9f0);
        if(!configuration||!attendance::scope::offlineMatch(nativeMode,at<int>(image+0xf3fddbc),at<unsigned char>(configuration+0x84)))return false;
    } // Preserve the existing mode-14 visual-only 2020 override scope.
    A states=at<A>(image+attendance::bindings::GameStates),wrapper=at<A>(image+attendance::bindings::MatchWrapper);
    world.match=at<A>(wrapper+16);world.phase=at<int>(states+0xa0);
    world.home=at<int>(image+0xd9fba20);world.away=at<int>(image+0xd9fba24);
    return at<A>(states)==image+attendance::bindings::GameStatesVtable&&at<A>(world.match)==image+attendance::bindings::MatchVtable&&
        world.phase>=0&&world.phase<=8&&world.home>0&&world.home<2047&&world.away>0&&world.away<2047;
}
void __fastcall presentationUpdate(A states,float delta){
    originalPresentationUpdate(states,delta);
    // Native post-update runs both presentation and gameplay. Using it alone
    // avoids the gameplay hook owned by Situational Practice and Call Sheet.
    if(enabled&&states==at<A>(image+attendance::bindings::GameStates)&&at<A>(states)==image+0xb47f790)CrowdTick();
    if(sparseCameraAvailable&&states==at<A>(image+attendance::bindings::GameStates)&&at<A>(states)==image+0xb47f790)
        if(!guardedSparseCameraTick()){sparseNativeCamera.fail();if(!sparseCameraFaultReported.exchange(true))audioNote("camera-fault: sparse native HFA update stopped; gameplay HFA was not changed.");}
    if(cameraAvailable&&!presentationCamera.healthy()&&!cameraFaultReported.exchange(true))audioNote("camera-fault: added motion stopped; the game's own camera is unchanged.");
}
void __fastcall attendanceUpdate(A entity){
    attendance::timing::Scope measured(naturalCrowdTimings);
    originalAttendanceUpdate(entity);
    // Observe controllers only after their native scene update has completed.
    // This discovers a freshly loaded crowd without waiting for a heap scan.
    if(!enabled||applying||!entityValid(entity))return;
    {std::lock_guard<std::mutex> guard(entityMutex);
        if(std::find(entities.begin(),entities.end(),entity)==entities.end()&&entities.size()<8){
            entities.push_back(entity);std::sort(entities.begin(),entities.end());++entityGeneration;
            visibilityRetry=true;
        }
    }
    A states=at<A>(image+attendance::bindings::GameStates);
    int phase=at<int>(states+0xa0);
    if(at<A>(states)==image+attendance::bindings::GameStatesVtable&&(visibilityRetry||seatPatternPending)){
        // The renderer can appear after the intro's one-shot arrival window.
        // Queue that SAME setup for presentation's existing post-update point.
        // Never re-enter the scene update while its native caller is unwinding.
        if(phase==6)pregameRendererRetry=true;
        else if(phase==7||(phase==8&&at<int>(at<A>(image+attendance::bindings::ClockManager)+8)==6))postgameRendererRetry=true;
    }
}
bool option(unsigned key,int width,int& value);
bool gameState(attendance::State& s,A& match){
    if(!attendance::scope::matchMode(at<int>(image+0xd9eb808))||at<int>(image+0xf3fddbc)!=3)return false;
    A configuration=at<A>(image+0xf63f9f0);
    if(!configuration||at<unsigned char>(configuration+0x84)!=0)return false;
    A states=at<A>(image+attendance::bindings::GameStates),wrapper=at<A>(image+attendance::bindings::MatchWrapper),clockManager=at<A>(image+attendance::bindings::ClockManager);
    match=at<A>(wrapper+16);
    if(at<A>(match)!=image+0xb47e908||at<A>(states)!=image+0xb47f790||at<A>(clockManager)!=image+0xb47eec8)return false;
    A gameClock=at<A>(clockManager+0x28);
    if(!read(clockManager+8,&s.quarter,4)||!read(states+0xa0,&s.phase,4)||!read(match+0x1d8,&s.homeScore,4)||!read(match+0x1dc,&s.awayScore,4))return false;
    bool clockReady=read(gameClock+0x28,&s.seconds,4)&&read(gameClock+0x2c,&s.quarterSeconds,4);
    attendance::normalizeFinalPresentation(s);
    if(s.phase==6){
        if(!clockReady||!std::isfinite(s.quarterSeconds)||s.quarterSeconds<60||s.quarterSeconds>1200){
            int minutes=0;if(!option(0x9f1b2500,2,minutes)||minutes<1||minutes>20)return false;
            s.quarterSeconds=static_cast<float>(minutes*60);
        }
        if(!attendance::normalizePregame(s))return false;
    }else if(!clockReady)return false;
    else if(attendance::untimedPeriod(s)){
        // Untimed OT can expose a zero-length clock. Normalize our snapshot
        // with the configured regulation length, never the game's clock.
        int minutes=0;if(!option(0x9f1b2500,2,minutes)||minutes<1||minutes>20)return false;
        if(!attendance::normalizeUntimedPeriod(s,static_cast<float>(minutes*60)))return false;
    }
    return attendance::valid(s);
}
bool offscreenSimulation(const attendance::State& s){
    return superSimBinding&&attendance::simulation::offscreen(read,image,s.phase);
}
void logAudioState(int volume){log("audio-state","Native audio callback running; native crowd volume "+std::to_string(volume)+"%; original game audio callback forwarded.");}
bool bytes(A rva,const std::vector<unsigned char>& expected){
    std::vector<unsigned char> got(expected.size());if(!read(image+rva,got.data(),got.size()))return false;
    if(got==expected)return true;
    return false; // A foreign detour is never silently replaced or chained.
}
// The native scene supplies this callback during creation/appearance refresh.
// No worker invokes it and no frame-by-frame clothing rewrite is performed.
A __fastcall appearanceGenerate(A component){
    attendance::visitors::AppearanceResult prepared;
    auto restore=[&](){bool okay=false;try{okay=attendance::visitors::restoreAppearance(read,image,prepared);}catch(...){}
        if(!okay)appearanceRestoreFailed=true;return okay;};
    std::shared_ptr<const attendance::visitors::GeneratedAppearance> previous;
    std::shared_ptr<attendance::visitors::GeneratedAppearance> generated;
    {std::lock_guard<std::mutex> guard(generatedCrowdMutex);for(auto& entry:generatedCrowds)if(entry&&entry->component==component){previous=entry;entry.reset();break;}}
    if(attendance::scope::dynasty(at<int>(image+0xd9eb808))&&enabled&&dynamicAwayFans&&!emptyMode2020&&!appearanceRestoreFailed)try{
        auto applied=std::atomic_load(&crowdFixture);
        int home=at<int>(image+0xd9fba20),away=at<int>(image+0xd9fba24);
        if(applied->owns(home,away)){
            const auto& s=applied->seed;
            auto plan=attendance::visitors::influence(attendance::visitors::plan(*applied,home,away),visitorCrowdInfluence.load());
            unsigned selectedSeed=attendance::turnout::fixtureHash(s);bool reused=false;
            if(!applied->launch.neutral){auto fixtureKey=attendance::visitors::key(s);attendance::visitors::Plan frozen;
                std::lock_guard<std::mutex> guard(visitorPlanMutex);
                if(visitorPlans.resolve(fixtureKey,plan,selectedSeed,frozen,selectedSeed,reused))plan=frozen;
            }
            // Only an original, completed native visibility snapshot can
            // define the occupied denominator. Current thinned/replay masks
            // must never be mistaken for the original audience.
            std::vector<unsigned char> occupied;
            {std::lock_guard<std::mutex> guard(crowdMutex);
                for(const auto& item:startingCrowds){const auto& base=item.second;
                    if(base.baselineComplete&&base.parts&&at<A>(base.scene)==image+attendance::bindings::SceneCrowdVtable&&
                       at<A>(base.scene+0x908)==component&&at<A>(base.scene+0x9f0)==item.first&&
                       at<A>(item.first)==image+attendance::bindings::CrowdRendererVtable&&at<A>(item.first+0x188)==base.masks&&
                       at<A>(item.first+0x190)==base.masks+base.hidden.size()*2){
                        occupied.resize(base.hidden.size());for(size_t i=0;i<occupied.size();++i)occupied[i]=(base.hidden[i]&base.parts)!=base.parts;break;
                    }
                }
            }
            prepared=attendance::visitors::appearance(read,image,component,plan,selectedSeed,s.home.militaryKnown&&s.home.military,s.away.militaryKnown&&s.away.military,occupied.empty()?nullptr:&occupied);
            log("away-appearance-budget","Native generation: component "+hex(component)+"; occupied snapshot "+std::to_string(prepared.occupiedVerified)+"; spectator / visitor count "+std::to_string(prepared.spectators)+" / "+std::to_string(prepared.occupiedVisitors)+"; authored positions "+std::to_string(prepared.positions)+"; target "+std::to_string(plan.share*100)+"% of spectators; "+(occupied.empty()?"original visibility not ready at this generation; appearance ratio requires live verification.":"original occupied denominator, bands excluded."));
            ++appearanceRuns;
            if(prepared.applied){
                // Allocate DGA's identity record before native generation. An
                // allocation failure can then restore the selectors first.
                generated=std::make_shared<attendance::visitors::GeneratedAppearance>();
                generated->component=component;generated->map=prepared.map;generated->data=prepared.stream.data;generated->coordinates=prepared.coordinates;
                generated->home=home;generated->away=away;generated->side=prepared.desired;
                ++appearanceChanged;appearanceBand=prepared.band;appearanceShare=plan.share;appearanceMiles=plan.travelKnown?plan.distance:-1;appearanceTravel=static_cast<int>(plan.travel);appearancePlanSource=reused?2:1;
            }
        }
    }catch(...){restore();prepared={};generated.reset();}
    A result=0;
    try{result=originalAppearanceGenerate(component);}
    catch(...){restore();throw;}
    restore();
    if(generated){std::lock_guard<std::mutex> guard(generatedCrowdMutex);size_t slot=generatedCrowds.size();
        for(size_t i=0;i<generatedCrowds.size();++i)if(!generatedCrowds[i]){slot=i;break;}
        if(slot==generatedCrowds.size())slot=nextGeneratedCrowd++%generatedCrowds.size();generatedCrowds[slot]=generated;
    }
    bool differentSupporters=static_cast<bool>(previous)!=static_cast<bool>(generated)||(previous&&generated&&previous->side!=generated->side);
    if(differentSupporters){std::lock_guard<std::mutex> guard(crowdMutex);for(auto& item:startingCrowds)if(at<A>(item.second.scene+0x908)==component){
        item.second.selection={};item.second.supporters={};item.second.exitsSeeded=false;seatPatternPending=true;visibilityRetry=true;
    }}
    return result;
}
bool requiredBytes(A rva,const std::vector<unsigned char>& expected){
    if(bytes(rva,expected))return true;
    std::vector<unsigned char> got(expected.size());bool readable=read(image+rva,got.data(),got.size());
    auto hex=[](const std::vector<unsigned char>& data){std::ostringstream o;o<<std::hex<<std::setfill('0');for(auto v:data)o<<std::setw(2)<<static_cast<unsigned>(v)<<' ';return o.str();};
    std::ostringstream message;message<<"Required game-code check failed at RVA 0x"<<std::hex<<rva<<"; expected ["<<hex(expected)<<"]; found ["<<(readable?hex(got):"unreadable")<<"].";
    A target=0;
    if(readable&&got.size()>=5&&got[0]==0xe9){int delta=0;std::memcpy(&delta,got.data()+1,4);target=image+rva+5+delta;}
    else if(readable&&got.size()>=6&&got[0]==0xff&&got[1]==0x25){int delta=0;std::memcpy(&delta,got.data()+2,4);target=at<A>(image+rva+6+delta);}
    if(target){HMODULE module=nullptr;std::vector<wchar_t> path(32768);DWORD length=0;if(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(target),&module))length=GetModuleFileNameW(module,path.data(),static_cast<DWORD>(path.size()));if(length>0&&length<path.size())message<<" Existing jump points into "<<std::filesystem::path(std::wstring(path.data(),length)).filename().u8string()<<".";else message<<" Existing jump target 0x"<<target<<" is outside a named module.";}
    message<<" No DGA hook was installed. Check game build and other injected plugins; this is not an EXE hash check.";
    log("binding-mismatch",message.str());return false;
}
struct ReplaySnapshot {bool valid=false,active=false;double playback=0,recording=0;};
ReplaySnapshot replaySnapshot(){
    ReplaySnapshot s;if(!replayBinding)return s;
    A manager=at<A>(image+attendance::bindings::ReplayManager);
    if(at<A>(manager)!=image+attendance::bindings::ReplayVtable)return s;
    unsigned char active=0;
    if(!read(manager+0x38,&active,1)||active>1||!read(manager+0x40,&s.playback,8)||!read(manager+0x1030,&s.recording,8)||
        !std::isfinite(s.playback)||!std::isfinite(s.recording)||s.playback<0||s.recording<0||s.playback>86400||s.recording>86400)return s;
    s.valid=true;s.active=active!=0;return s;
}
bool swellBindings(){
    using namespace attendance::bindings;
    struct Check {A rva;std::vector<unsigned char> code;};
    const Check checks[]={
        {SwellLookup,{0x48,0x89,0x5c,0x24,8,0x48,0x89,0x6c,0x24,0x10}},
        {SwellConstruct,{0x40,0x53,0x57,0x41,0x54,0x41,0x55,0x41,0x57}},
        {SwellDestroy,{0x41,0x56,0x48,0x83,0xec,0x20}},
        {SwellSetInt,{0x48,0x89,0x5c,0x24,0x10,0x57,0x48,0x83,0xec,0x20}},
        {SwellSend,{0x4c,0x8b,0xdc,0x55,0x41,0x56,0x49,0x8d,0x6b,0xd8,0x48,0x81,0xec,0x18,1,0,0}},
        {SwellQuery,{0x40,0x53,0x48,0x83,0xec,0x30}},
        {SwellQueryGet,{0x48,0x89,0x5c,0x24,8,0x57,0x48,0x83,0xec,0x30}},
        {SwellQueryDestroy,{0x48,0x89,0x5c,0x24,0x10,0x48,0x89,0x74,0x24,0x18}},
        {SwellWeakRelease,{0xb8,0xff,0xff,0xff,0xff,0xf0,0x0f,0xc1,0x41,8,0x83,0xf8,1}},
        {SwellArenaInit,{0x40,0x53,0x48,0x83,0xec,0x20}},
        {SwellArenaDestroy,{0x48,0x89,0x5c,0x24,8,0x48,0x89,0x74,0x24,0x10,0x57,0x48,0x83,0xec,0x20}},
        {SwellManagerGetter,{0x48,0x8b,0x41,0x50,0xc3}}
    };
    for(const auto& check:checks)if(!bytes(check.rva,check.code))return false;
    return true;
}

bool maskLayout(A crowd,A& renderer,A& begin,size_t& count,A& parts){
    __try {
        renderer=*reinterpret_cast<A*>(crowd+0x9f0);parts=*reinterpret_cast<A*>(crowd+0x908);
        if(*reinterpret_cast<A*>(renderer)!=image+0xb7e1988)return false;
        begin=*reinterpret_cast<A*>(renderer+0x188);A end=*reinterpret_cast<A*>(renderer+0x190);
        if(begin<0x10000||end<begin||(end-begin)%2||end-begin>0x200000)return false;
        count=(end-begin)/2;return count>0;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
bool currentMask(A begin,unsigned agent,unsigned short& mask){
    __try {mask=reinterpret_cast<const unsigned short*>(begin)[agent];return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
// Called only with crowdMutex held, on the serialized presentation path.
// Counts come from cached original seating masks, never camera/replay masks.
void publishRenderedOccupancyLocked(){
    A owner=audioMatch.load();unsigned epoch=audioEpoch.load();if(!owner)return;
    attendance::occupancy::RenderedBuilder builder;bool pending=false;
    for(const auto& item:startingCrowds){
        const auto& base=item.second;if(base.hidden.size()<512)continue;
        if(at<A>(base.scene)!=image+attendance::bindings::SceneCrowdVtable||at<A>(base.scene+0x9f0)!=item.first||
           at<A>(item.first)!=image+attendance::bindings::CrowdRendererVtable||at<A>(item.first+0x188)!=base.masks||
           at<A>(item.first+0x190)!=base.masks+base.hidden.size()*2)continue;
        if(!base.baselineComplete||base.selection.policy!=attendance::seats::Policy::Position){pending=true;continue;}
        attendance::seats::Mapping mapping;
        if(!attendance::seats::mapping(read,image,base.scene,item.first,base.masks,base.hidden.size(),mapping)){pending=true;continue;}
        builder.add(base.layoutKey?static_cast<A>(base.layoutKey):mapping.positions,mapping.count-base.agents.band,base.selection.original);
    }
    auto previous=std::atomic_load(&renderedOccupancy);auto value=builder.finish(owner,epoch,pending,*previous);
    if(!value.valid()||(previous->owns(owner,epoch)&&previous->seats==value.seats&&previous->original==value.original&&previous->groups==value.groups))return;
    std::shared_ptr<const attendance::occupancy::Rendered> next=std::make_shared<attendance::occupancy::Rendered>(value);std::atomic_store(&renderedOccupancy,next);
    log("seating-density","Audio seating baseline: "+std::to_string(value.original)+" occupied / "+std::to_string(value.seats)+" mapped positions across "+std::to_string(value.groups)+" seating layout(s). Live arrivals and departures scale this density; replay cameras do not.");
}
void refreshRenderedOccupancy(){
    auto known=std::atomic_load(&renderedOccupancy);if(known->owns(audioMatch.load(),audioEpoch.load()))return;
    std::lock_guard<std::mutex> guard(crowdMutex);publishRenderedOccupancyLocked();
}
// Runs after the native baseline pass, outside the per-fan callback. Build one
// immutable order per renderer, then share it across every attendance purpose.
void prepareSeatPatterns(){
    attendance::timing::Scope measured(seatPreparationTimings);
    std::lock_guard<std::mutex> guard(crowdMutex);bool pending=false;unsigned ready=0,fallback=0;
    for(auto cursor=startingCrowds.begin();cursor!=startingCrowds.end();){
        auto& item=*cursor;
        auto& base=item.second;
        if(at<A>(base.scene)!=image+attendance::bindings::SceneCrowdVtable||at<A>(base.scene+0x9f0)!=item.first||
           at<A>(item.first)!=image+attendance::bindings::CrowdRendererVtable||at<A>(item.first+0x188)!=base.masks||
           at<A>(item.first+0x190)!=base.masks+base.hidden.size()*2){++crowdRevision;cursor=startingCrowds.erase(cursor);continue;}
        ++cursor;
        if(base.selection.policy==attendance::seats::Policy::Pending){
            // Some native parts belong only to certain fan models. Requiring
            // every part on every fan would leave those scenes pending forever.
            if(!base.baselineComplete||!base.parts){
                // Tiny sideline draws may never be visited by this attendance
                // controller. Leave their unverified native seats alone, and
                // do not let them keep a ready seated stadium in retry mode.
                if(base.hidden.size()>=512)pending=true;
                continue;
            }
            std::vector<attendance::seats::Position> coordinates;
            try{
                base.agents=attendance::agents::readScene(read,image,base.scene,base.hidden.size());
                auto identity=std::atomic_load(&crowdIdentity);
                if(identity->home.id==shownTeam0.load()&&identity->away.id==shownTeam1.load())attendance::agents::protectStudents(base.agents,
                    identity->home.militaryKnown&&identity->home.military,identity->away.militaryKnown&&identity->away.military);
                {std::lock_guard<std::mutex> generatedGuard(generatedCrowdMutex);A component=at<A>(base.scene+0x908);
                 for(const auto& generated:generatedCrowds)if(generated&&generated->component==component){generated->apply(read,image,base.scene,shownTeam0,shownTeam1,base.agents);break;}}
                if(attendance::seats::positions(read,image,base.scene,item.first,base.masks,base.hidden.size(),coordinates)){
                    originalSeatLayouts.restore(coordinates,base.hidden,base.nativeSeen,base.parts);
                    base.layoutKey=attendance::seats::Baselines::fingerprint(coordinates);
                    auto fanSeats=base.hidden;
                    if(base.agents.verified)for(size_t i=0;i<fanSeats.size();++i)
                        if(base.agents.protectedBand(i))fanSeats[i]=base.parts;
                    base.selection=attendance::seats::build(coordinates,fanSeats,base.parts,attendanceSeed.load());
                    attendance::seats::Mapping geometry;
                    if(attendance::seats::mapping(read,image,base.scene,item.first,base.masks,base.hidden.size(),geometry))base.geometry=geometry.positions;
                    if(base.selection.policy==attendance::seats::Policy::Position)originalSeatLayouts.remember(coordinates,base.hidden,base.parts);
                }else base.selection.policy=attendance::seats::Policy::Scattered;
                base.supporters=attendance::visitors::groups(base.selection,base.hidden,base.parts,base.agents);
            }catch(const std::exception&){base.selection={};base.selection.policy=attendance::seats::Policy::Scattered;}
            if(base.selection.policy==attendance::seats::Policy::Position)
                log("seating","Crowd seating pattern ready: fuller near the field, uneven groups and scattered fans farther back.");
            else if(base.hidden.size()>=512)
                log("seating","Seat positions could not be verified for this crowd; using the standard scattered pattern.");
        }
        if(base.selection.policy==attendance::seats::Policy::Position)++ready;
        else if(base.selection.policy==attendance::seats::Policy::Scattered)++fallback;
    }
    seatPatternPending=pending;shownSeatPatterns=ready;shownSeatFallbacks=fallback;
    publishRenderedOccupancyLocked();
}
// Legacy filmed crowds retain their original temporary-seat accounting.
// Live budgets switch after a legacy save's current arrival/halftime interval ends.
void prepareLegacyVisitorLossesLocked(int historical,int arriving){
    for(auto& item:startingCrowds){auto& base=item.second;const auto& g=base.supporters;if(!g.verified)continue;
        if(historical>=0){
            float h=replayHomeLoss.load(),a=replayAwayLoss.load();bool split=h>=0&&a>=0;
            base.homeGone=static_cast<unsigned>(std::llround(g.home*(split?h:historical*.01)));
            base.awayGone=static_cast<unsigned>(std::llround(g.away*(split?a:historical*.01)));
            int missing=split?(std::max)(0,replayVisitorArrivals.load()):0;
            base.homeAbsent=static_cast<unsigned>((g.home-g.homeProtected)*static_cast<unsigned>(missing)/100);
            base.awayAbsent=static_cast<unsigned>((g.away-g.awayProtected)*static_cast<unsigned>(missing)/100);
            continue;
        }
        if(!base.exitsSeeded){
            float h=resumeHomeLoss.load(),a=resumeAwayLoss.load();
            base.exits.restore(g.home,g.away,g.homeProtected,g.awayProtected,h,a);
            base.exitsSeeded=true;
        }
        base.exits.advance(g.home,g.away,activeLoss.load(),shownScore0.load(),shownScore1.load(),shownQuarter.load(),g.homeProtected,g.awayProtected,shownNeutral.load());
        base.homeGone=static_cast<unsigned>(base.exits.home);base.awayGone=static_cast<unsigned>(base.exits.away);
        base.homeAbsent=static_cast<unsigned>((g.home-g.homeProtected)*static_cast<unsigned>((std::clamp)(arriving,0,100))/100);
        base.awayAbsent=static_cast<unsigned>((g.away-g.awayProtected)*static_cast<unsigned>((std::clamp)(arriving,0,100))/100);
    }
}
void prepareVisitorLosses(){
    std::lock_guard<std::mutex> guard(crowdMutex);
    int historical=replayLoss.load(),arriving=arrivalMissing.load();
    if(historical<0&&resumeLegacyBudget&&arriving<=activeLoss.load())resumeLegacyBudget=false;
    if((historical>=0&&!replaySharedBudget)||(historical<0&&resumeLegacyBudget)){
        sharedFanBudgetApplied=false;prepareLegacyVisitorLossesLocked(historical,arriving);return;
    }
    std::map<uint64_t,attendance::fanbudget::Layout> unique;bool unknown=false;
    for(const auto& item:startingCrowds){const auto& b=item.second;const auto& g=b.supporters;
        if(b.hidden.size()<512)continue;
        if(!g.verified||!b.geometry||!b.layoutKey){unknown=true;continue;}
        attendance::fanbudget::Counts c{g.home,g.away,g.homeProtected,g.awayProtected};
        auto inserted=unique.emplace(b.layoutKey,attendance::fanbudget::Layout{b.layoutKey,c});
        if(!inserted.second){const auto& prior=inserted.first->second.fans;
            if(prior.home!=c.home||prior.away!=c.away||prior.homeProtected!=c.homeProtected||prior.awayProtected!=c.awayProtected)unknown=true;
        }
    }
    std::vector<attendance::fanbudget::Layout> layouts;for(const auto& item:unique)layouts.push_back(item.second);
    attendance::fanbudget::Counts totals;
    if(unknown||!attendance::fanbudget::totals(layouts,totals)){
        sharedFanBudgetApplied=false;prepareLegacyVisitorLossesLocked(historical,arriving);return;
    }
    attendance::visitors::Losses exits;
    int missing=arriving;
    if(historical>=0){
        float h=replayHomeLoss.load(),a=replayAwayLoss.load();
        bool split=h>=0&&a>=0;float loss=replayPermanentLoss.load()*.01f;
        exits.restore(totals.home,totals.away,totals.homeProtected,totals.awayProtected,split?h:loss,split?a:loss);
        missing=(std::max)(0,replayVisitorArrivals.load());
    }else{
        if(!sharedFanExitsSeeded){
            sharedFanExits.restore(totals.home,totals.away,totals.homeProtected,totals.awayProtected,resumeHomeLoss.load(),resumeAwayLoss.load());
            // A legacy save may have progressed during its last temporary break.
            uint64_t home=0,away=0;std::set<uint64_t> seen;
            for(const auto& item:startingCrowds){const auto& b=item.second;if(b.exitsSeeded&&b.layoutKey&&seen.insert(b.layoutKey).second){home+=b.exits.home;away+=b.exits.away;}}
            sharedFanExits.home=(std::max)(sharedFanExits.home,home);sharedFanExits.away=(std::max)(sharedFanExits.away,away);
            sharedFanExitsSeeded=true;
        }else if(totals.home!=sharedFanBasis.home||totals.away!=sharedFanBasis.away||totals.homeProtected!=sharedFanBasis.homeProtected||totals.awayProtected!=sharedFanBasis.awayProtected){
            float h=sharedFanBasis.home?static_cast<float>(static_cast<double>(sharedFanExits.home)/sharedFanBasis.home):0;
            float a=sharedFanBasis.away?static_cast<float>(static_cast<double>(sharedFanExits.away)/sharedFanBasis.away):0;
            sharedFanExits.restore(totals.home,totals.away,totals.homeProtected,totals.awayProtected,h,a);
        }
        sharedFanBasis=totals;
        sharedFanExits.advance(totals.home,totals.away,activeLoss.load(),shownScore0.load(),shownScore1.load(),shownQuarter.load(),totals.homeProtected,totals.awayProtected,shownNeutral.load());
        exits=sharedFanExits;
    }
    const auto plan=attendance::fanbudget::build(layouts,{exits.home,exits.away},missing);
    if(!plan.valid){sharedFanBudgetApplied=false;prepareLegacyVisitorLossesLocked(historical,arriving);return;}
    for(auto& item:startingCrowds){auto& b=item.second;
        for(const auto& row:plan.layouts)if(row.key==b.layoutKey){
            b.homeGone=static_cast<unsigned>(row.gone.home);b.awayGone=static_cast<unsigned>(row.gone.away);
            b.homeAbsent=static_cast<unsigned>(row.missing.home);b.awayAbsent=static_cast<unsigned>(row.missing.away);
            if(historical<0){b.exits.home=row.gone.home;b.exits.away=row.gone.away;b.exitsSeeded=true;}break;
        }
    }
    sharedFanBudgetApplied=true;sharedFanTarget=totals.total()-plan.missing.total();sharedFanProtected=totals.homeProtected+totals.awayProtected;
}
void __fastcall visibility(A crowd,unsigned int part,unsigned int agent,bool hidden) {
    if(applying)++calls;
    A renderer=0,begin=0,parts=0;size_t count=0;
    if(enabled&&maskLayout(crowd,renderer,begin,count,parts)&&agent<count){
        // The backend stores a hidden bit per resolved part, for each fan.
        unsigned bit=16;
        if(passParts.resolve(reinterpret_cast<attendance::visibility::PartResolver>(image+attendance::bindings::CrowdPartResolve),parts,part,bit)){
            std::lock_guard<std::mutex> guard(crowdMutex);
            StartingCrowd* base=nullptr;
            if(applying&&passBaseline&&passRevision==crowdRevision&&passRenderer==renderer)base=passBaseline;
            else {auto found=startingCrowds.find(renderer);if(found!=startingCrowds.end())base=&found->second;}
            const bool collecting=(collectingStartingCrowd.load()&&!applying)||capturingBaseline;
            bool replacement=!base||base->scene!=crowd||base->masks!=begin||base->hidden.size()!=count;
            // A full live crowd may have no active visibility override. Still
            // invalidate replay's no-op cache when native callbacks reveal a
            // new seated renderer/buffer; tiny sideline draws stay optional.
            if(replacement&&count>=512){visibilityRetry=true;seatPatternPending=true;}
            if((applying||collecting||emptyCrowdActive||activeLoss>0||arrivalMissing>0||replayLoss>=0)&&replacement){
                StartingCrowd baseline;baseline.masks=begin;baseline.scene=crowd;baseline.hidden.resize(count);baseline.temporary.resize(count);baseline.nativeSeen.resize(count);
                if(!read(begin,baseline.hidden.data(),count*2)){log("baseline-unavailable","Crowd write skipped");return;}
                baseline.nativeHidden=baseline.hidden;
                std::vector<attendance::seats::Position> coordinates;
                if(attendance::seats::positions(read,image,crowd,renderer,begin,count,coordinates))
                    originalSeatLayouts.restore(coordinates,baseline.hidden,baseline.nativeSeen,baseline.parts);
                base=&startingCrowds.insert_or_assign(renderer,std::move(baseline)).first->second;++crowdRevision;
                seatPatternPending=true;
                if(!applying)visibilityRetry=true;
                log("starting-crowd","renderer="+hex(renderer)+" fans="+std::to_string(count)+" preserved existing empty seats");
            }
            if(applying){passRenderer=renderer;passBaseline=base;passRevision=crowdRevision;}
            if(base&&base->masks==begin&&agent<base->hidden.size()){
                if(capturingBaseline)base->captureToken=activeCaptureToken;
                base->parts|=static_cast<unsigned short>(1u<<bit);
                unsigned short flag=static_cast<unsigned short>(1u<<bit);
                auto& latest=base->nativeHidden[agent];
                latest=static_cast<unsigned short>((latest&~flag)|(hidden?flag:0));
                // The first native input for each part is authoritative even
                // if a newly loaded buffer initially contained empty defaults.
                bool firstNativeInput=!(base->nativeSeen[agent]&flag);
                if(firstNativeInput){
                    auto& original=base->hidden[agent];
                    original=static_cast<unsigned short>((original&~(1u<<bit))|(hidden?(1u<<bit):0u));
                    base->nativeSeen[agent]|=flag;
                }
                const bool restore=mode==2||(applying&&!thinning);
                const auto& selection=base->selection;
                if(!capturingBaseline&&selection.policy!=attendance::seats::Policy::Pending&&(!collecting||activeLoss>0||arrivalMissing>0||replayLoss>=0)&&(applying||!restore)){
                    unsigned short current=0;if(!currentMask(begin,agent,current)){if(applying)return;}
                    bool baseHidden=(base->hidden[agent]&(1u<<bit))!=0;
                    bool wasHidden=(current&(1u<<bit))!=0;
                    bool protectedBand=base->agents.protectedGroup(agent);
                    bool permanent=!protectedBand&&selectedSupporter(*base,agent,0,activeLoss.load());
                    bool withheld=!protectedBand&&selectedSupporter(*base,agent,1,arrivalMissing.load());
                    auto& owned=base->temporary[agent];bool wasTemporary=(owned&(1u<<bit))!=0;
                    bool result=attendance::hidePart(hidden,baseHidden,wasHidden,permanent||withheld,restore,reconcilingArrivals,wasTemporary,firstNativeInput);
                    int historical=replayLoss.load();
                    if(!restore&&(historical>=0||exactVisibility))result=attendance::historicalSelection(hidden,baseHidden,!protectedBand&&selectedSupporter(*base,agent,2,historical>=0?historical:(std::max)(activeLoss.load(),arrivalMissing.load())));
                    if(withheld&&!permanent&&!baseHidden&&!hidden)owned|=static_cast<unsigned short>(1u<<bit);
                    else if(reconcilingArrivals&&!withheld)owned&=static_cast<unsigned short>(~(1u<<bit));
                    if(applying&&result&&!hidden)++changed;
                    hidden=result;
                }
                // Preserve known original empty positions even before a newly
                // streamed renderer has its departure order prepared.
                if(base->hidden[agent]==0xffff)hidden=true;
                if(applying)++verifiedCalls;
            }
        }else if(applying)return;
    }else if(applying)return;
    if(emptyCrowdActive)hidden=true;
    originalSet(crowd,part,agent,hidden);
}
void resetMatch(){
    replayHomeLoss=replayAwayLoss=resumeHomeLoss=resumeAwayLoss=-1;replayVisitorArrivals=-1;replayPermanentLoss=0;replaySharedBudget=true;resumeLegacyBudget=false;sharedFanBudgetApplied=false;sharedFanTarget=sharedFanProtected=0;
    std::shared_ptr<const attendance::visitors::Population> noSupporters=std::make_shared<attendance::visitors::Population>();std::atomic_store(&visitorPopulation,noSupporters);
    appliedVisibilityTarget=appliedVisibilityMode=-1;
    replayLoss=-1;exactVisibility=false;replaySuspendsAtmosphere=false;replayWasActive=false;replayRestorePending=false;bootstrapPending=false;lastRecordingTime=-1;shownResumeSource=0;
    crowdChangeWindow.reset();simulationRecording=-1;shownSuperSim=false;
    attendanceSeed=0;publishSessionPlan(0,attendance::State{});
    {std::lock_guard<std::mutex> guard(timelineMutex);attendanceArchive.active=0;}
    {std::lock_guard<std::mutex> guard(historyMutex);gameHistory.reset();arrivalModel.reset();arrivalCompletion.reset();halftimeModel.reset();halftimeExitConversion.reset();shownHalftimeConverted=0;departureModel.reset();matchLifecycle.reset();}
    {std::lock_guard<std::mutex> guard(atmosphereMutex);playTracker.reset();soundOnlyReactions.reset();lateTurnoverExits.reset();coreAtmosphere.reset();lineupAtmosphere.reset();hostileOpeningResult.reset();openingKickoffAtmosphere.reset();presentationAtmosphere.reset();openingWelcome.reset();cameraMotion.reset();finalReactionCarry.reset();liveReactions.reset();skillReactions.reset();catchKnockouts.reset();defensiveAccents.reset();defensiveMotion.reset();showboating.reset();skillSwell.reset();contextReactions.reset();liveReactionMajor=liveReactionMilestone=false;liveMotionEvent=attendance::live::Event::None;awaySoundReactions.reset();awayCameraMotion.reset();awayLastCue={};awayHeardId=awayHeardAt=awayHeardSequence=awayExtraHeardId=0;extraReactionId=extraReactionUntil=0;celebrationReactionType=91;situationalReactions.reset();hailMaryPass.reset();liveFumbles.reset();fumbleContacts.reset();returnAtmosphere.reset();scoringSoundHandoff.reset();reactionDistance.reset();passFlight.reset();liveReactionId=liveReactionUntil=liveSourceSequence=liveMotionSequence=liveAuditId=liveAuditUntil=playAuditSequence=playAuditEndSequence=0;liveReactionPost=false;shownLiveCatches=shownLivePicks=0;audioContinuity.reset();pendingLateExits=0;atmosphereMatch=0;}
    shownLineup=shownPackedHouse=shownHostileOpening=shownHostileSeries=false;shownOpeningDrive=attendance::lineup::OpeningAdmission::Inactive;shownDrive=0;shownDistanceExtra=0;shownReactionDistance=0;
    shownUpsetStage=0;shownUpsetSound=false;shownUpsetMinimum=1.01f;
    shownPassFlights=0;shownPassGate="waiting for an eligible snap";
    crowdBoost=0;shownEruption=shownPostgameCarry=false;
    arrivalMissing=0;arrivalReconcilePending=false;visibilityRetry=false;captureArrivalBaseline=false;pregameRendererRetry=postgameRendererRetry=false;shownAmbienceLift=shownOpeningWeight=0;shownAudioLimit=attendance::core::NormalMaximumDb;shownDecisiveUntil=shownHighUntil=0;visibilityGeneration=0;
    shownHalftime=shownArriving=0;shownNeutral=false;shownConference=shownHfa=shownConferenceKnown=shownBigAtmosphere=false;shownTeam0=shownTeam1=shownRivalryEvent=-1;
    shownVisibilityCalls=shownVisibilityControllers=0;shownOriginalFans=shownVisibleFans=shownExpectedFans=shownNativeExtra=shownOtherExtra=shownExtraVisible=0;visibilityMeasured=false;
    shownComeback=shownUpset=false;contextReady=false;
    shownRank0=shownRank1=shownPrecipitation=0;shownRivalry=shownPlayoff=shownFcs=false;
    {std::lock_guard<std::mutex> guard(crowdMutex);++crowdRevision;startingCrowds.clear();originalSeatLayouts.reset();sharedFanExits={};sharedFanBasis={};sharedFanExitsSeeded=false;}
    seatPatternPending=false;shownSeatPatterns=shownSeatFallbacks=0;
    collectingStartingCrowd=true;activeLoss=0;currentLoss=highestTarget=0;lastApplied=0;lastMatch=0;lastQuarter=0;lastClock=0;
    shownCurrent=shownTarget=0;observed.clear();restoreAt=0;refreshEntities=true;
    audioMatch=0;++audioEpoch;
    std::shared_ptr<const attendance::occupancy::Snapshot> unknown=std::make_shared<attendance::occupancy::Snapshot>();std::atomic_store(&stadiumOccupancy,unknown);
    std::shared_ptr<const attendance::occupancy::Rendered> unmapped=std::make_shared<attendance::occupancy::Rendered>();std::atomic_store(&renderedOccupancy,unmapped);
    std::shared_ptr<const attendance::sustained::Intent> quiet=std::make_shared<attendance::sustained::Intent>();std::atomic_store(&sustainedIntent,quiet);
    std::shared_ptr<const attendance::motion::Cue> quietCamera=std::make_shared<attendance::motion::Cue>();std::atomic_store(&cameraCue,quietCamera);
}
void apply(bool thin,bool arrivals=false,bool captureOnly=false) {
    attendance::timing::Scope applyMeasured(crowdApplyTimings);
    collectingStartingCrowd=false;
    std::vector<A> snapshot;{std::lock_guard<std::mutex> guard(entityMutex);snapshot=entities;}
    bool needCapture=(captureArrivalBaseline&&activeLoss==0)||seatPatternPending||visibilityGeneration!=entityGeneration.load();
    {std::lock_guard<std::mutex> guard(crowdMutex);needCapture=needCapture||startingCrowds.empty();}
    bool prepared=false;
    auto result=attendance::visibility::dispatch(snapshot,captureOnly,needCapture,[&](A entity){
        if(entityValid(entity))return true;refreshEntities=true;return false;
    },[&](A entity,bool baseline){
        if(!baseline&&!prepared){prepareSeatPatterns();prepareVisitorLosses();prepared=true;}
        calls=changed=verifiedCalls=0;passBaseline=nullptr;passRenderer=0;applying=true;capturingBaseline=baseline;
        activeCaptureToken=baseline?++captureSequence:0;
        thinning=!baseline&&thin;reconcilingArrivals=!baseline&&arrivals;
        // The engine resolves controller -> scene crowds. Renderer readiness
        // is checked on each actual crowd in visibility(), before any write.
        {attendance::timing::Scope nativeMeasured(nativeCrowdTimings);++forcedCrowdPasses;passParts.begin();originalAttendanceUpdate(entity);passParts.end();}
        if(baseline){std::lock_guard<std::mutex> guard(crowdMutex);for(auto& item:startingCrowds)
            if(item.second.captureToken==activeCaptureToken)item.second.baselineComplete=true;
        }
        activeCaptureToken=0;passBaseline=nullptr;passRenderer=0;
        applying=false;thinning=false;reconcilingArrivals=capturingBaseline=false;
        return attendance::visibility::Pass{calls,verifiedCalls,changed};
    });
    shownVisibilityControllers=result.controllers;shownVisibilityCalls=result.verified;
    if(captureOnly)prepareSeatPatterns();
    visibilityRetry=result.verified==0||seatPatternPending;
    if(visibilityRetry)refreshEntities=true;
    else if(captureOnly||arrivalMissing>0)captureArrivalBaseline=false;
    attendance::VisibilityCount population;bool measured=false;
    attendance::visitors::Population supporters;std::set<A> supporterGeometry;bool supporterUnknown=false;
    uint64_t homeGone=0,awayGone=0;
    const int historical=replayLoss.load();
    const int measuredLoss=emptyCrowdActive?100:(!thin||mode==2)?0:historical>=0?historical:(std::max)(activeLoss.load(),arrivalMissing.load());
    if(!captureOnly&&result.verified){
        attendance::timing::Scope measuredCrowd(crowdMeasurementTimings);
        std::lock_guard<std::mutex> guard(crowdMutex);
        for(const auto& item:startingCrowds){
            A renderer=item.first;const auto& base=item.second;
            if(at<A>(base.scene)!=image+attendance::bindings::SceneCrowdVtable||at<A>(base.scene+0x9f0)!=renderer||at<A>(renderer)!=image+0xb7e1988||at<A>(renderer+0x188)!=base.masks||
               at<A>(renderer+0x190)!=base.masks+base.hidden.size()*2||!base.parts)continue;
            std::vector<unsigned short> current(base.hidden.size());if(!read(base.masks,current.data(),current.size()*2))continue;
            measured=true;
            bool unique=base.geometry&&supporterGeometry.insert(base.layoutKey?static_cast<A>(base.layoutKey):base.geometry).second;
            if(base.hidden.size()>=512&&(!base.supporters.verified||!base.geometry))supporterUnknown=true;
            if(unique&&base.supporters.verified){supporters.originalHome+=base.supporters.home;supporters.originalAway+=base.supporters.away;homeGone+=base.homeGone;awayGone+=base.awayGone;}
            for(size_t i=0;i<current.size();++i){
                if(base.selection.policy==attendance::seats::Policy::Pending)continue;
                if(base.agents.protectedBand(i))continue;
                bool selected=emptyCrowdActive||((thin&&mode!=2)&&selectedSupporter(base,static_cast<unsigned>(i),2,measuredLoss));
                attendance::countSelection(population,base.parts,base.hidden[i],current[i],base.nativeHidden[i],selected);
                if(unique&&base.supporters.verified&&(base.hidden[i]&base.parts)!=base.parts&&!selected){if(base.supporters.side[i])++supporters.away;else ++supporters.home;}
            }
        }
    }
    if(!captureOnly&&result.verified&&historical<0&&!supporterUnknown&&supporters.originalHome+supporters.originalAway){
        supporters.verified=true;supporters.sharedBudget=sharedFanBudgetApplied.load();supporters.appliedLoss=activeLoss.load();supporters.appliedTemporary=arrivalMissing.load();supporters.match=audioMatch.load();supporters.epoch=audioEpoch.load();
        supporters.homeLoss=supporters.originalHome?static_cast<float>(static_cast<double>(homeGone)/supporters.originalHome):0;
        supporters.awayLoss=supporters.originalAway?static_cast<float>(static_cast<double>(awayGone)/supporters.originalAway):0;
        std::shared_ptr<const attendance::visitors::Population> next=std::make_shared<attendance::visitors::Population>(supporters);std::atomic_store(&visitorPopulation,next);
    }
    if(!captureOnly){
        shownOriginalFans=population.original;shownVisibleFans=population.visible;shownExpectedFans=population.expected;
        shownNativeExtra=population.nativeExtra;shownOtherExtra=population.otherExtra;shownExtraVisible=population.extraVisible;
        visibilityMeasured=measured&&population.original>0;
        if(result.verified&&!visibilityRetry){appliedVisibilityTarget=measuredLoss;appliedVisibilityMode=mode.load();}
    }
    std::string detail="controllers="+std::to_string(result.controllers)+" verified visibility calls="+std::to_string(result.verified)+" hidden overrides="+std::to_string(result.changed);
    if(visibilityMeasured)detail+="; measured "+std::to_string(shownVisibleFans.load())+" / "+std::to_string(shownOriginalFans.load())+" original crowd agents visible";
    if(visibilityMeasured)detail+="; selection expected "+std::to_string(population.expected)+" visible, additionally hidden by game="+std::to_string(population.nativeExtra)+", other hidden="+std::to_string(population.otherExtra);
    log(visibilityRetry?"visibility-waiting":"attendance-applied",std::string(captureOnly?"Starting crowd capture: ":"Crowd visibility update: ")+detail+(visibilityRetry?"; waiting for the scene's actual crowd renderer.":"."));
}
bool guardedApply(bool thin,bool arrivals=false,bool captureOnly=false) {
    __try {apply(thin,arrivals,captureOnly);return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){passBaseline=nullptr;passRenderer=0;passParts.end();applying=false;thinning=false;reconcilingArrivals=capturingBaseline=false;return false;}
}
void serviceEmptyMode(){
    static EmptyWorld previous;static bool hadWorld=false;
    if(!emptyMode2020&&!emptyCrowdActive&&!emptyModePending)return;
    EmptyWorld world;if(!emptyWorld(world)){if(emptyCrowdActive)emptyModePending=true;emptyCrowdActive=false;hadWorld=false;return;}
    bool was=emptyCrowdActive.load(),wanted=emptyMode2020.load(),pending=emptyModePending.exchange(false);
    bool worldChanged=!hadWorld||world.match!=previous.match||world.home!=previous.home||world.away!=previous.away||(world.phase==6&&previous.phase!=6);
    bool owned=contextReady&&world.match==lastMatch&&world.home==shownTeam0&&world.away==shownTeam1;
    if(owned){std::lock_guard<std::mutex> guard(historyMutex);owned=!matchLifecycle.needsReset(world.match,world.phase==6);}
    if(worldChanged&&!owned){
        std::lock_guard<std::mutex> guard(crowdMutex);++crowdRevision;startingCrowds.clear();originalSeatLayouts.reset();sharedFanExits={};sharedFanBasis={};sharedFanExitsSeeded=false;seatPatternPending=false;visibilityRetry=true;
        appliedVisibilityTarget=appliedVisibilityMode=-1;
    }
    emptyCrowdActive=wanted;
    bool dirty=visibilityRetry||seatPatternPending||visibilityGeneration!=entityGeneration.load();
    if((pending||worldChanged||was!=wanted||dirty||(wanted&&appliedVisibilityTarget!=100))&&(wanted||was||pending)){
        bool thin=wanted||(owned&&(activeLoss>0||arrivalMissing>0));
        // The override hid additional seats without owning permanent exits.
        // Reconstruct the exact normal/replay selection on release rather than
        // letting monotonic departure logic preserve the forced empty masks.
        bool priorExact=exactVisibility.load();if(!wanted)exactVisibility=true;
        bool applied=guardedApply(thin,true);exactVisibility=priorExact;
        if(!applied){enabled=false;log("fault","2020 Mode crowd update failed; further visibility changes stopped.");return;}
        visibilityGeneration=entityGeneration.load();emptyModePending=visibilityRetry.load();
        if(was!=wanted)log("2020-mode",wanted?"Empty stadium override active; normal attendance history is preserved underneath.":"Empty stadium override released; restoring this game's normal crowd.");
    }
    previous=world;hadWorld=true;
}
bool guardedEmptyService(){__try{serviceEmptyMode();return true;}__except(EXCEPTION_EXECUTE_HANDLER){emptyModePending=true;return false;}}
// Replay timing changes do not imply a different seating picture. Avoid
// forcing another complete native fan pass when this exact target is already
// applied. Native renderer callbacks still discover streamed/replaced buffers
// and set visibilityRetry; generation, pending-layout and mode changes remain
// independent reasons to refresh.
bool refreshReplayVisibility(){
    int historical=replayLoss.load();int target=emptyCrowdActive?100:mode==2?0:historical>=0?historical:(std::max)(activeLoss.load(),arrivalMissing.load());
    bool dirty=visibilityRetry||seatPatternPending||visibilityGeneration!=entityGeneration.load();
    if(!dirty&&target==appliedVisibilityTarget.load()&&mode.load()==appliedVisibilityMode.load()){
        ++unchangedReplaySkips;return true;
    }
    if(!guardedApply(true,true))return false;
    visibilityGeneration=entityGeneration.load();return true;
}
void findEntities() {
    std::vector<A> found;
    const A needle=image+0xb485368;std::vector<unsigned char> buffer(0x100007);
    for(A addr=0x10000;addr<0x100000000;){
        MEMORY_BASIC_INFORMATION m{};if(!VirtualQuery(reinterpret_cast<void*>(addr),&m,sizeof(m)))break;
        A start=reinterpret_cast<A>(m.BaseAddress),end=start+m.RegionSize;if(end<=addr)break;addr=end;
        if(m.State!=MEM_COMMIT||m.Type!=MEM_PRIVATE||(m.Protect&(PAGE_GUARD|PAGE_NOACCESS))||!(m.Protect&0xee))continue;
        end=(std::min)(end,A(0x100000000));
        for(A p=start;p<end;p+=0x100000){
            size_t n=(std::min)(size_t(0x100007),size_t(end-p));if(!read(p,buffer.data(),n))continue;
            for(size_t i=0;i+8<=n;i+=8){A value=0;memcpy(&value,buffer.data()+i,8);if(value==needle&&entityValid(p+i))found.push_back(p+i);}
        }
    }
    std::sort(found.begin(),found.end());found.erase(std::unique(found.begin(),found.end()),found.end());
    if(found.size()>8){log("discovery-skipped","Too many attendance entities; no candidates published");return;}
    {std::lock_guard<std::mutex> guard(entityMutex);if(found==entities)return;entities=found;++entityGeneration;}
    for(A e:found)log("entity",hex(e)+" asset="+hex(at<A>(e+0x38)));
}
bool install() {
    conferenceBinding=bytes(attendance::bindings::MatchVisualGetter,{0x48,0x89,0x74,0x24,0x10,0x57,0x48,0x83,0xec,0x20,0x48,0x8b,0x3d,0x5f,0xc5,0x98,0x0b})&&
        bytes(attendance::bindings::MatchVisualGetter+0x2f,{0x48,0x8b,0x57,0x20});
    hfaBinding=bytes(attendance::bindings::HfaDisabledGetter+0x0f,{0x48,0x8b,0x05,0x22,0x8e,0xe5,0x0c})&&
        bytes(attendance::bindings::HfaDisabledGetter+0x1e,{0x0f,0xb6,0xb0,0xf0,0x01,0,0});
    if(at<unsigned int>(image+0x3c)>0x1000)return false;
    if(!requiredBytes(attendance::bindings::CrowdVisibility,cleanEntry(attendance::bindings::CrowdVisibility))||
       !requiredBytes(attendance::bindings::PeriodChange,{0x40,0x53,0x48,0x83,0xec,0x20,0x48,0x8b,0x99,0x30,0x01,0,0,0x48,0x89,0x6c,0x24,0x30})||
       !requiredBytes(attendance::bindings::CrowdPartResolve,{0x4c,0x8b,0x91,0xa0,0,0,0,0x44,0x8b,0xca})||
       !requiredBytes(0xa0b3000,{0x48,0x83,0xec,0x38,0x89,0x54,0x24,0x28})||
       !requiredBytes(attendance::bindings::NeutralGetter,{0x48,0x83,0xec,0x28,0x48,0x8b,0x0d,0x85,0xa7,0x36,0x09,0x48,0x85,0xc9,0x74,0x18})||
       !requiredBytes(0x9099440,{0x40,0x53,0x48,0x83,0xec,0x50})||
       !requiredBytes(attendance::bindings::AttendanceUpdate,{0x40,0x55,0x56,0x41,0x57,0x48,0x8d,0xac,0x24,0x20,0xe9,0xff,0xff,0xb8,0xe0,0x17,0x00,0x00})||
       !requiredBytes(attendance::bindings::PresentationUpdate,{0x40,0x53,0x48,0x83,0xec,0x40,0x8b,0x81,0xa0,0,0,0,0x48,0x8b,0xd9})||
       !requiredBytes(0x5d05640,{0x48,0x63,0xc2,0x48,0x8b,0x4c,0xc1,0x20,0xc5,0xfa,0x10,0x41,0x28,0xc3})||
       !requiredBytes(0x5d05650,{0x48,0x8b,0x41,0x28,0x8b,0x51,0x08}))return false;
    signatures=true;
    capacityBinding=bytes(attendance::bindings::StadiumCapacity,{0x40,0x53,0x48,0x83,0xec,0x30,0x33,0xc0,0x48,0x8b,0xda})&&bytes(attendance::bindings::OptionQuery,{0x40,0x53,0x48,0x83,0xec,0x50,0x48,0x8b,0x05,0x6b,0x7b,0xbf,0x06,0x49,0x8b,0xd8});
    superSimBinding=bytes(attendance::bindings::SuperSimModeGetter,{0x48,0x8b,0x05,0x91,0xf1,0x17,0x0c,0x48,0x85,0xc0,0x74,0x0d,0x83,0x78,0x08,0x01,0x75,0x07,0xb1,0x01});
    replayModeBinding=bytes(attendance::bindings::ReplayRealtimeQuery,{0x48,0x83,0xec,0x28,0x48,0x8b,0x0d,0x9d,0x68,0x1c,0x06,0x48,0x85,0xc9,0x74,0x10,0xe8,0x3b,0x23,0xcc,0xf9,0x84,0xc0,0x74,0x07,0xb0,0x01,0x48,0x83,0xc4,0x28,0xc3,0x32,0xc0,0x48,0x83,0xc4,0x28,0xc3})&&
        bytes(0x2ed6540,{0x40,0x53,0x48,0x83,0xec,0x20,0x48,0x8b,0xd9,0xe8,0xe2,0x15,0xea,0x00,0x48,0x8b,0xc8,0x48,0x8b,0x10,0xff,0x52,0x38,0x48,0x8b,0xc8,0x48,0x8b,0x10,0xff,0x52,0x50,0xf7,0x43,0x08,0xf7,0xff,0xff,0xff,0x0f,0x95,0xc1,0x84,0xc0,0x74,0x1e,0x83,0x7b,0x14,0x00,0x75,0x18,0x84,0xc9,0x75,0x0c,0x38,0x4b,0x2c,0x0f,0x95,0xc0,0x48,0x83,0xc4,0x20,0x5b,0xc3,0xb0,0x01,0x48,0x83,0xc4,0x20,0x5b,0xc3,0x32,0xc0,0x48,0x83,0xc4,0x20,0x5b,0xc3})&&
        bytes(attendance::bindings::ReplayPlaycallQuery,{0x0f,0xb6,0x81,0x49,0x04,0x00,0x00,0xc3})&&
        at<A>(image+attendance::bindings::ReplaySimApiVtable+0xb8)==image+0x7baa750&&
        at<A>(image+attendance::bindings::ReplayFlowApiVtable+0x5e0)==image+attendance::bindings::ReplayPlaycallQuery;
    if(!replayModeBinding)log("major-replay","Native replay-mode queries differ; added inset routing disabled, original presentation retained.");
    if(!superSimBinding)log("simulation-unavailable","Super Sim binding differs; crowd changes will wait for ordinary selection and breaks.");
    pauseBinding=bytes(attendance::bindings::PauseCheck,{0x48,0x8b,0x05,0x09,0x51,0x9a,0x07,0x48,0x85,0xc0,0x74,0x08,0x80,0x38,0x00,0x74,0x03,0xb0,0x01,0xc3,0x32,0xc0,0xc3});
    replayBinding=bytes(attendance::bindings::ReplayTimeGetter,{0xc5,0xfb,0x10,0x41,0x40,0xc3})&&
        bytes(attendance::bindings::RecordingTimeGetter,{0xc5,0xfb,0x10,0x81,0x30,0x10,0,0,0xc3})&&
        bytes(attendance::bindings::ReplayActiveGetter,{0x0f,0xb6,0x41,0x38,0xc3})&&
        bytes(attendance::bindings::ReplayStartGetter,{0xc5,0xfb,0x10,0x41,0x48,0xc3});
    if(!replayBinding)log("replay-unavailable","Replay time bindings differ; historical crowd playback is inactive.");
    if(replayBinding){
        const auto& prefix=attendance::replayhook::Prefix;
        if(!bytes(attendance::bindings::ReplayUpdate,std::vector<unsigned char>(prefix.begin(),prefix.end())))replayBinding=false;
        else if(!makePatch(attendance::bindings::ReplayUpdate,reinterpret_cast<void*>(replayUpdate),reinterpret_cast<void**>(&originalReplayUpdate),17))return false;
        if(!replayBinding)log("replay-unavailable","Replay update callback differs; historical crowd playback is inactive.");
    }
    if(!pauseBinding)log("anticipation-inactive","Pause-state binding differs; new roars and temporary excitement will remain inactive.");
    // Optional view-matrix copy hook. It never selects camera presets or retains
    // scene references, and uses the game's camera-effects accessibility gate.
    cameraBindings=pauseBinding&&replayBinding&&
       bytes(0x80a00b7,{0xc5,0xf8,0x10,0x81,0xb4,0x02,0,0})&&
       bytes(attendance::bindings::CameraViewCopy,{0x48,0x83,0xb9,0xc0,0x03,0,0,0,0x74,0x33,0xc5,0xf8,0x10,0x81,0x74,0x02,0,0})&&
       at<A>(image+attendance::bindings::CameraViewSlot)==image+attendance::bindings::CameraViewCopy&&
       bytes(attendance::bindings::CameraGet,{0x48,0x89,0x5c,0x24,0x08,0x48,0x89,0x74,0x24,0x10,0x57,0x48,0x83,0xec,0x20})&&
       bytes(attendance::bindings::CameraEffectsGetter,{0x48,0x83,0xec,0x28,0x8b,0xd1,0x41,0xb8,0x15,0x00,0x00,0x00,0x48,0x8b,0x0d,0xfd,0xd8,0x0c,0x08})&&
       bytes(attendance::bindings::CameraSettingGetter,{0x33,0xc0,0x41,0x83,0xf8,0x19,0x77,0x1a,0x83,0xfa,0x01,0x77,0x15,0x48,0x63,0xc2,0x48,0x6b,0xd0,0x1a,0x49,0x63,0xc0,0x48,0x03,0xd0,0x0f,0xb7,0x84,0x51,0xbc,0x03,0,0,0xc3});
    if(pauseBinding&&replayBinding&&
       bytes(attendance::bindings::MuseLogicEvaluate,cleanEntry(attendance::bindings::MuseLogicEvaluate))&&
       bytes(attendance::bindings::MuseComparisonValue,cleanEntry(attendance::bindings::MuseComparisonValue))){
        if(!makePatch(attendance::bindings::MuseLogicEvaluate,reinterpret_cast<void*>(museLogicEvaluate),reinterpret_cast<void**>(&originalMuseLogic),15)||
           !makePatch(attendance::bindings::MuseComparisonValue,reinterpret_cast<void*>(museComparisonValue),reinterpret_cast<void**>(&originalMuseComparison),15))return false;
        replayPresentationInstalled=true;
        if(bytes(attendance::bindings::ReplayEventNodeUpdate,cleanEntry(attendance::bindings::ReplayEventNodeUpdate))&&
           bytes(attendance::bindings::ReplayNodeUpdate,cleanEntry(attendance::bindings::ReplayNodeUpdate))&&
           at<A>(image+attendance::bindings::ReplayEventNodeVtable+0x60)==image+attendance::bindings::ReplayEventNodeUpdate&&
           at<A>(image+attendance::bindings::ReplayNodeVtable+0x60)==image+attendance::bindings::ReplayNodeUpdate&&
           bytes(attendance::bindings::PresentationClient,{0x48,0x8b,0x05,0x41,0x91,0xd6,0x07,0xc3})&&
           bytes(attendance::bindings::PresentationEvent,{0x48,0x83,0xec,0x28,0x45,0x33,0xc9,0x45,0x33,0xc0,0xe8,0x61,0xe0,0x3c,0x02,0xb0,0x01,0x48,0x83,0xc4,0x28,0xc3})){
            if(!makePatch(attendance::bindings::ReplayEventNodeUpdate,reinterpret_cast<void*>(replayEventNode),reinterpret_cast<void**>(&originalReplayEventNode),19)||
               !makePatch(attendance::bindings::ReplayNodeUpdate,reinterpret_cast<void*>(nativeReplayNode),reinterpret_cast<void**>(&originalNativeReplayNode),18))return false;
            majorReplayBindings=true;
        }else log("major-replay","Major-play replay lifecycle unavailable: native layout checks differ; original playback remains in charge.");
    }else log("replay-unavailable","Extra post-play replays are unavailable on this layout; normal replay selection remains active.");
    // These getters independently verify every new play-data field. A changed
    // layout disables energy/events while retaining the original attendance.
    playBindingsAvailable=bytes(attendance::bindings::CurrentDownGetter,{0x48,0x8b,0x05,0x59,0x15,0x66,0x09})&&
        bytes(attendance::bindings::CurrentDownGetter+0x15,{0x8b,0x80,0xd0,0x01,0x00,0x00})&&
        bytes(attendance::bindings::CurrentOffenseGetter,{0x48,0x8b,0x05,0xd9,0x00,0x66,0x09})&&
        bytes(attendance::bindings::CurrentOffenseGetter+0x15,{0x8b,0x80,0xc8,0x01,0x00,0x00})&&
        bytes(attendance::bindings::TimeoutGetter,{0x8b,0x41,0x4c,0x89,0x44,0x24,0x08,0x8b,0x41,0x50})&&
        bytes(attendance::bindings::FirstDownGetter,{0x48,0x8b,0x05,0xc9,0x07,0x66,0x09})&&
        bytes(attendance::bindings::FirstDownGetter+0x15,{0x48,0x81,0xc1,0x60,0x02,0x00,0x00})&&
        bytes(attendance::bindings::FirstDownGetter+0x2a,{0x8b,0x41,0x20,0xc1,0xe8,0x0a,0x24,0x01})&&
        bytes(attendance::bindings::FirstDownLine,{0x48,0x83,0xec,0x18,0x48,0x8b,0xd1})&&
        bytes(0x61fe135,{0xc4,0xe2,0x79,0x18,0x98,0x6c,0x01,0x00,0x00})&&
        bytes(0x2e913c0,{0x48,0x8b,0x05,0x19,0x8c,0x54,0x0c,0xc3})&&
        bytes(attendance::bindings::TurnoverGetter,{0x48,0x8b,0x81,0xd8,0x0c,0x00,0x00,0x8b,0x40,0x64,0xc3});
    // Broadcast key 14 tests ball state 5 with fumble substate 1 or 2.
    // Result kinds 3/4/5 belong to the incomplete-pass query (key 20).
    fumbleBinding=playBindingsAvailable&&bytes(attendance::bindings::FumblePredicate+0x2b,{0x83,0xf8,0x05,0x75,0x6b,0x8b,0x44,0x24,0x38,0xff,0xc8,0x83,0xf8,0x01})&&
        bytes(attendance::bindings::BallStateGetter,{0x8b,0x81,0x58,0x01,0,0,0x89,0x02,0x8b,0x81,0x68,0x01,0,0,0xc3});
    fieldGoalBlockBinding=playBindingsAvailable&&bytes(attendance::bindings::FieldGoalBlockedPredicate+0xc,{0x48,0x8b,0x40,0x10,0x48,0x05,0x60,0x02,0,0})&&
        bytes(attendance::bindings::FieldGoalBlockedPredicate+0x20,{0x8b,0x40,0x20,0xc1,0xe8,0x08,0xa8,0x01});
    liveReactionBindings=playBindingsAvailable&&bytes(attendance::bindings::BallStateGetter,{0x8b,0x81,0x58,0x01,0,0,0x89,0x02,0x8b,0x81,0x68,0x01,0,0,0xc3})&&
        bytes(attendance::bindings::CatchPosition+0x49,{0xc5,0xc8,0x57,0xf6,0xc5,0xca,0x2a,0x70,0x44});
    playCallBinding=bytes(attendance::bindings::OffensivePlayType,{0x48,0x83,0xec,0x28,0x48,0x8b,0x0d,0x4d,0x42,0x0d,0x08})&&
        bytes(attendance::bindings::PlayCallTeamGet,{0x48,0x83,0xec,0x28,0x48,0x63,0xc2,0x48,0x83,0xc1,0x10,0x48,0x69,0xd0,0xe8,0x0d,0,0})&&
        bytes(attendance::bindings::PlayCallSelectedGet,{0x48,0x8d,0x41,0x08,0xc3})&&bytes(attendance::bindings::PlayCallGet,{0x48,0x8d,0x41,0x08,0xc3})&&
        bytes(attendance::bindings::PlayCallType,{0x8b,0x41,0x2c,0xc3})&&bytes(attendance::bindings::PlayCallFallbackType,{0x8b,0x41,0x30,0xc3});
    runnerPositionBinding=bytes(0x2ea9c90,{0xc5,0xf8,0x10,0x51,0x40})&&at<A>(image+0xb47bb40)==image+0x2ea9c90;
    defensiveReactionBindings=bytes(attendance::bindings::SackPredicate,{0x48,0x89,0x5c,0x24,0x08})&&
        bytes(0x620083f,{0x81,0xff,0xb7,0x0b,0,0})&&bytes(0x2e9c590,{0x48,0x83,0xec,0x48})&&
        bytes(attendance::bindings::ForcedFumblePredicate,{0x40,0x53,0x48,0x83,0xec,0x20})&&bytes(0x6200f27,{0x83,0xfb,0x20,0x0f,0x94,0xc0});
    liveFumbleBinding=liveReactionBindings&&defensiveReactionBindings&&
        bytes(0x276b8d9,{0x48,0x8b,0x0d,0x68,0xbc,0xbe,0x0b})&&bytes(attendance::bindings::LiveFootballEventType,{0x8b,0x41,0x1c,0xc3})&&
        at<A>(image+attendance::bindings::LiveFootballEventVtable+0x40)==image+attendance::bindings::LiveFootballEventType;
    skillBindings=liveReactionBindings&&bytes(0x7dd9210,{0x48,0x83,0xec,0x38,0xba,0x4f,0,0,0})&&
        bytes(0x7dd922e,{0xba,0xe8,0xea,0x84,0x37})&&bytes(0x7dfa910,{0x48,0x8b,0x05,0xc9,0xa7,0xcd,0x07,0xc3})&&
        bytes(0x7dd9247,{0xc5,0xfb,0x10,0x88,0x50,0x16,0,0})&&bytes(0x7dd93c7,{0xc5,0xfb,0x10,0x88,0x40,0x16,0,0})&&
        bytes(0x7dd92a7,{0xc5,0xfb,0x10,0x88,0x68,0x16,0,0})&&bytes(0x7ddd522,{0x8b,0xd3,0x48,0x8b,0xc8});
    if(!skillBindings)log("skill-reactions-unavailable","Optional skill/hit event checks differ; these accents stay inactive. Other reactions remain available.");
    hurdleCountBinding=skillBindings&&bytes(attendance::bindings::HurdleCountGetter,{0x8b,0x81,0x60,0x08,0,0,0xc3});
    catchKnockoutBinding=liveReactionBindings&&bytes(attendance::bindings::CatchKnockoutStore,{0x88,0x83,0x75,0x01,0,0})&&
        bytes(attendance::bindings::CatchSummaryReset,{0x66,0x45,0x89,0x9a,0x74,0x01,0,0});
    passBreakupBinding=liveReactionBindings&&bytes(attendance::bindings::PassTippedQuery,{0xba,0x27,0,0,0,0x48,0x8b,0xc8})&&
        bytes(attendance::bindings::PlayEventLookup,{0x4c,0x8d,0x51,0x10,0x45,0x33,0xc9})&&
        bytes(attendance::bindings::PlayEventReceive+0x15,{0x41,0x81,0xf8,0xb5,0x01,0,0});
    contactFumbleBinding=passBreakupBinding&&bytes(0x7ddd522,{0x8b,0xd3,0x48,0x8b,0xc8});
    if(!passBreakupBinding)log("pass-breakups-unavailable","Optional native deflection checks differ; pass-breakup accents stay inactive. Catch knockouts and other reactions remain available.");
    if(!defensiveReactionBindings)log("defensive-reactions-unavailable","Native sack/forced-fumble checks differ; their flag-based reactions stay inactive. The measured five-yard loss rule does not require these checks.");
    if(!runnerPositionBinding)log("runner-position-unavailable","Live runner position differs; long catches and interceptions remain available.");
    if(!liveReactionBindings)log("live-reactions-unavailable","Live catch/interception bindings differ; completed-play reactions remain active.");
    findEntities();
    if(!makePatch(attendance::bindings::AttendanceUpdate,reinterpret_cast<void*>(attendanceUpdate),reinterpret_cast<void**>(&originalAttendanceUpdate),18)||
       !makePatch(attendance::bindings::PresentationUpdate,reinterpret_cast<void*>(presentationUpdate),reinterpret_cast<void**>(&originalPresentationUpdate),15)||
       !makePatch(attendance::bindings::CrowdVisibility,reinterpret_cast<void*>(visibility),reinterpret_cast<void**>(&originalSet),15)||
       !makePatch(attendance::bindings::PeriodChange,reinterpret_cast<void*>(periodChanged),reinterpret_cast<void**>(&originalPeriod),18))return false;
    if(bytes(attendance::bindings::AudioUpdate,cleanEntry(attendance::bindings::AudioUpdate))){
        if(!makePatch(attendance::bindings::AudioUpdate,reinterpret_cast<void*>(audioUpdate),reinterpret_cast<void**>(&originalAudioUpdate),16))return false;
        audioHookInstalled=true;stadiumSilence.configure(image,audioNote);
        if(swellBindings()){
            if(!makePatch(attendance::bindings::SwellSend,reinterpret_cast<void*>(crowdSend),reinterpret_cast<void**>(&originalCrowdSend),17))return false;
            auto api=attendance::sustained::Api::native(image);api.send=originalCrowdSend;
            sustainedAudio.configure(image,sustainedNote,&api);supportingSwell.configure(image,nullptr,&api,true);swellAvailable=true;
        }
        else log("anticipation-inactive","Sustained roar bindings differ; existing crowd audio remains available.");
        if(swellBindings()&&bytes(attendance::bindings::ReactionUpdate,{0x48,0x89,0x5c,0x24,0x10,0x48,0x89,0x6c,0x24,0x18,0x48,0x89,0x74,0x24,0x20})&&
           bytes(attendance::bindings::ReactionReceive,{0x40,0x57,0x48,0x83,0xec,0x70})){
            positiveCheer.configure(image,nullptr,nullptr,reactionNote);extraCheer.configure(image,nullptr,nullptr,reactionNote);opponentCheer.configure(image,nullptr,nullptr,reactionNote);
            if(!makePatch(attendance::bindings::ReactionUpdate,reinterpret_cast<void*>(reactionUpdate),reinterpret_cast<void**>(&originalReactionUpdate),15))return false;
            cheerAvailable=true;
        }else log("cheer-inactive","Native cheer bindings differ; celebration roars and volume remain available.");
        if(bytes(attendance::bindings::IntensityUpdate,cleanEntry(attendance::bindings::IntensityUpdate))&&
           bytes(attendance::bindings::IntensityPublish,{0x48,0x8b,0x09,0xe9,0x58,0x30,0,0})&&
           bytes(attendance::bindings::IntensityLock,{0x48,0xff,0x25,0x45,0xda,0x63,0x10})&&
           bytes(attendance::bindings::IntensityUnlock,{0x48,0xff,0x25,0xed,0xcb,0x63,0x10})){
            crowdIntensity.configure(image);
            if(!makePatch(attendance::bindings::IntensityUpdate,reinterpret_cast<void*>(intensityUpdate),reinterpret_cast<void**>(&originalIntensityUpdate),18))return false;
            intensityAvailable=true;
        }else log("intensity-inactive","Native excitement bindings differ; sustained roar playback and existing audio remain available.");
        if(bytes(attendance::bindings::MixerCreate,{0x4c,0x8b,0xdc,0x53,0x55,0x48,0x83,0xec,0x78})&&
           bytes(attendance::bindings::MixerRelease,{0x48,0x89,0x5c,0x24,0x10,0x48,0x89,0x74,0x24,0x18,0x57,0x48,0x83,0xec,0x20})){
            attendanceMixer.configure(image);chantAvailable=true;silenceBindingsReady=true;
        }else log("chants-inactive","Native crowd-bus mixer layout differs; native crowd audio remains unchanged");
    }else log("audio-inactive","Native audio layout differs; visible attendance remains available, crowd/band muting is unavailable.");
    if(pauseBinding&&replayBinding&&
       bytes(attendance::bindings::CameraSet,{0x48,0x89,0x5c,0x24,0x08,0x48,0x89,0x74,0x24,0x10,0x57,0x48,0x83,0xec,0x20})&&
       bytes(attendance::bindings::CameraFind,{0x40,0x53,0x48,0x83,0xec,0x20,0x48,0x8b,0xd9,0x48,0x8b,0xca})&&
       bytes(attendance::bindings::CameraGet,{0x48,0x89,0x5c,0x24,0x08,0x48,0x89,0x74,0x24,0x10,0x57,0x48,0x83,0xec,0x20})){
        sparseNativeCamera.configure(image);
        if(!makePatch(attendance::bindings::CameraSet,reinterpret_cast<void*>(nativeCameraSet),reinterpret_cast<void**>(&originalNativeCameraSet),15))return false;
        sparseNativeCamera.setOriginal(originalNativeCameraSet);sparseCameraAvailable=true;
    }else log("native-camera-unavailable","Sparse-crowd native HFA camera filter unavailable; gameplay HFA and native camera remain unchanged.");
    pauseMenuAvailable=preparePauseMenu();
    if(!pauseMenuAvailable)log("pause-menu-inactive","Native pause-label binding unavailable; the ordinary pause menu remains unchanged.");
    settingsMenu.configure(image,dataFolder/L"DynamicGamedayAtmosphere.ini",settingsMenuNote);
    const auto& menuPrefix=attendance::settingsmenu::native::ListPrefix;
    if(settingsMenu.bindings()&&bytes(attendance::settingsmenu::native::ListSite,{menuPrefix.begin(),menuPrefix.end()})){
        settingsMenuAvailable=makePatch(attendance::settingsmenu::native::ListSite,reinterpret_cast<void*>(attendance::settingsmenu::native::Menu::callback()),&settingsMenuOriginal,menuPrefix.size());
        if(settingsMenuAvailable)settingsMenu.original(reinterpret_cast<A>(settingsMenuOriginal));
    }
    if(!settingsMenuAvailable)log("settings-menu-inactive","In-game settings bindings differ; use the DGA INI for this game layout.");
    pauseSettingsMenu.configure(image,dataFolder/L"DynamicGamedayAtmosphere.ini",settingsMenuNote);
    const auto& pauseSettingsPrefix=attendance::settingsmenu::native::PauseListPrefix;
    if(pauseSettingsMenu.bindings()&&bytes(attendance::settingsmenu::native::PauseListSite,{pauseSettingsPrefix.begin(),pauseSettingsPrefix.end()})){
        pauseSettingsMenuAvailable=makePatch(attendance::settingsmenu::native::PauseListSite,reinterpret_cast<void*>(attendance::settingsmenu::native::PauseMenu::callback()),&pauseSettingsMenuOriginal,pauseSettingsPrefix.size());
        if(pauseSettingsMenuAvailable)pauseSettingsMenu.original(reinterpret_cast<A>(pauseSettingsMenuOriginal));
    }
    if(!pauseSettingsMenuAvailable)log("settings-menu-inactive","Pause settings provider differs; Dynasty/RTG User Settings and the DGA INI remain available.");
    if(capacityBinding&&bytes(attendance::turnout::native::Formula,cleanEntry(attendance::turnout::native::Formula))&&
       bytes(attendance::turnout::native::OptionSet,cleanEntry(attendance::turnout::native::OptionSet))&&
       bytes(attendance::turnout::native::LaunchCopy,{attendance::turnout::native::LaunchPrefix.begin(),attendance::turnout::native::LaunchPrefix.end()})&&
       bytes(0x7f182b8,{0xba,0x00,0x69,0x2e,0x9f,0x49,0x8b,0xcf,0xe8,0xeb,0x95,0x18,0x01})&&
       bytes(0x7f17415,{0xba,0x00,0x69,0x2e,0x9f,0x66,0x89,0x85,0x98,0x00,0x00,0x00,0x49,0x8b,0xcf,0xe8,0x87,0xa4,0x18,0x01})&&
       bytes(0x7f17ddb,{0xba,0x00,0x25,0x1d,0x9e,0x49,0x8b,0xcf,0xe8,0xc8,0x9a,0x18,0x01})&&
       bytes(0x37f40a0,{0x48,0x89,0x5c,0x24,0x10,0x48,0x89,0x74,0x24,0x18,0x57,0x48,0x83,0xec,0x20})&&
       bytes(0x8603b00,{0x40,0x55,0x56,0x41,0x54,0x41,0x55,0x41,0x56,0x41,0x57,0x48,0x8b,0xec})&&
       bytes(0x90a1670,{0x40,0x53,0x48,0x83,0xec,0x60,0x48,0x8b,0x05,0x3b,0xf9,0xbe,0x06,0x49,0x8b,0xd8})&&
       bytes(0x7dac4f6,{0x3d,0x74,0x04,0,0,0x48,0x8b,0xcb,0x0f,0x9d,0xc2})){
        size_t first=patches.size();
        initialBinding=makePatch(attendance::turnout::native::Formula,reinterpret_cast<void*>(initialFormula),reinterpret_cast<void**>(&originalInitialFormula),17)&&
            makePatch(attendance::turnout::native::OptionSet,reinterpret_cast<void*>(initialOptionSet),reinterpret_cast<void**>(&originalInitialOptionSet),15)&&prepareInitialLaunch();
        if(!initialBinding){
            while(patches.size()>first){VirtualFree(patches.back().trampoline,0,MEM_RELEASE);patches.pop_back();}
            if(initialLaunchGate){VirtualFree(initialLaunchGate,0,MEM_RELEASE);initialLaunchGate=nullptr;}initialLaunchOriginal=nullptr;
            originalInitialFormula=nullptr;originalInitialOptionSet=nullptr;
        }
    }
    if(!initialBinding)log("starting-attendance-inactive","Starting-turnout bindings unavailable; EA's native attendance and existing arrivals remain active.");
    if(bytes(attendance::visitors::Generation,attendance::visitors::GenerationPrefix))
        appearanceBinding=makePatch(attendance::visitors::Generation,reinterpret_cast<void*>(appearanceGenerate),reinterpret_cast<void**>(&originalAppearanceGenerate),attendance::visitors::GenerationPrefix.size());
    if(!appearanceBinding)log("away-crowd","Native appearance binding unavailable; EA crowd outfits preserved.");
    if(!commitPatches())return false;
    if(cameraBindings){
        originalCameraView=reinterpret_cast<CameraViewCopy>(image+attendance::bindings::CameraViewCopy);presentationCamera.configure(image);
        cameraAvailable=cameraViewHook.install(image+attendance::bindings::CameraViewSlot,reinterpret_cast<A>(originalCameraView),reinterpret_cast<A>(cameraView));
    }
    if(cameraAvailable&&audioSetting&&
       bytes(attendance::bindings::LegacySoundReceive,{0x48,0x89,0x5c,0x24,0x18,0x48,0x89,0x6c,0x24,0x20,0x41,0x56,0x48,0x83,0xec,0x20})&&
       bytes(attendance::bindings::VariantValue,{0x48,0x89,0x5c,0x24,0x10,0x57,0x48,0x83,0xec,0x20})){
        originalLegacySoundReceive=reinterpret_cast<LegacySoundReceive>(image+attendance::bindings::LegacySoundReceive);
        cannonAvailable=cannonReceiverHook.install(image+attendance::bindings::LegacySoundVtable+0x28,reinterpret_cast<A>(originalLegacySoundReceive),reinterpret_cast<A>(legacySoundReceive));
    }
    if(!cannonAvailable)log("cannon-inactive","Native stadium-sound observer unavailable; cannon audio remains under game control.");
    if(!cameraAvailable)log("anticipation-inactive","Camera motion binding unavailable; native camera presentation remains active.");
    enabled=true;return true;
}
// Read-only native context. The game uses these same options for its presentation data.
bool option(unsigned key,int width,int& value){
    A service=at<A>(image+0xe2e53e8),vt=at<A>(service);
    if(vt!=image+0xb29ee30||at<A>(vt+0x68)!=image+0x9099440)return false;
    struct Variant {A vt,type,data;} v{image+0xb135818,at<A>(image+0xfc90fb8),0};
    using Query=bool(__fastcall*)(A,unsigned,Variant*,A,A);
    if(!reinterpret_cast<Query>(image+0x9099440)(service,key,&v,0,0)||!v.data)return false;
    if(width==1){unsigned char x=0;if(!read(v.data,&x,1))return false;value=x;}
    else {unsigned short x=0;if(!read(v.data,&x,2))return false;value=x;}
    return true;
}
void refreshStadiumOccupancy(A match,unsigned epoch,uint64_t now){
    auto known=std::atomic_load(&stadiumOccupancy);if(known->owns(match,epoch))return;
    std::lock_guard<std::mutex> lock(capacityMutex);known=std::atomic_load(&stadiumOccupancy);
    if(known->owns(match,epoch))return;
    bool first=known->match!=match||known->epoch!=epoch;
    if(first){
        auto empty=std::make_shared<attendance::occupancy::Snapshot>();empty->match=match;empty->epoch=epoch;
        std::shared_ptr<const attendance::occupancy::Snapshot> next=empty;std::atomic_store(&stadiumOccupancy,next);lastCapacityAttempt=0;
    }
    if(lastCapacityAttempt&&now>=lastCapacityAttempt&&now-lastCapacityAttempt<1000)return;lastCapacityAttempt=now;
    attendance::occupancy::Snapshot snapshot;
    if(capacityBinding&&attendance::occupancy::nativeSnapshot(image,match,epoch,snapshot)&&audioMatch.load()==match&&audioEpoch.load()==epoch){
        std::shared_ptr<const attendance::occupancy::Snapshot> next=std::make_shared<attendance::occupancy::Snapshot>(snapshot);std::atomic_store(&stadiumOccupancy,next);
        log("stadium-fullness","Native crowd: "+std::to_string(snapshot.attendance)+" fans / "+std::to_string(snapshot.capacity)+" seats (stadium ID "+std::to_string(snapshot.stadium)+"). Reference values only; verified mapped seating takes priority for audio.");
    }else if(first)log("stadium-fullness","Seating measurement not ready; added soundscape waits for verified stadium fullness.");
}
bool context(attendance::State& s){
    A weather=at<A>(image+0xf506000);
    if(at<A>(weather)!=image+0xb32a290)return false;
    bool overrideWeather=at<unsigned char>(weather+0x388)!=0;
    float precipitation=at<float>(weather+(overrideWeather?0x39c:0xf0));
    int temperature=at<int>(weather+(overrideWeather?0x38c:0xe8));
    if(!std::isfinite(precipitation)||precipitation<0||temperature<-150||temperature>160)return false;
    s.precipitation=precipitation>0?(temperature<=32?2:1):0;
    int rivalry=1023;
    if(!option(0x9e2b2900,1,s.bowl)||!option(0x9f2e6900,2,rivalry))return false;
    // Same read-only predicate used by native HFA suppression and the UI's
    // IsNeutralStadium query. It includes the home program's alternate stadium.
    using Neutral=bool(__fastcall*)();
    s.neutral=reinterpret_cast<Neutral>(image+attendance::bindings::NeutralGetter)();
    s.rivalry=rivalry!=1023;s.rivalryEvent=s.rivalry?rivalry:-1;
    bool conference=false;bool dynasty=attendance::scope::dynasty(at<int>(image+0xd9eb808));shownConferenceKnown=dynasty&&conferenceBinding&&attendance::nativecontext::conference(image,s.neutral,s.rivalry,conference);
    attendance::postseason(s,dynasty&&attendance::playoffBowl(s.bowl),conference);
    int team0=at<int>(image+0xd9fba20),team1=at<int>(image+0xd9fba24);
    if(team0<0||team0>=2047||team1<0||team1>=2047)return false;
    s.team0=team0;s.team1=team1;
    int kickoff=-1;
    if(option(0x9f4be400,2,kickoff)&&kickoff>=0&&kickoff<1440){s.night=kickoff>=1140;shownKickoffMinutes=kickoff;}else shownKickoffMinutes=-1;
    s.plannedStadium=at<int>(weather+0x358);
    auto initial=std::atomic_load(&initialApplied);
    s.startingAttendance=dynasty&&!s.playoff&&!s.conferenceChampionship&&s.bowl==127&&initial->owns(team0,team1)&&initial->launch.stadium==at<int>(weather+0x358);
    if(s.startingAttendance){s.night=initial->result.night;s.plannedKickoff=initial->result.kickoff;s.plannedFill=initial->result.fillPercent;s.plannedPeak=initial->result.attendance;s.plannedCapacity=initial->launch.capacity;s.plannedStadium=initial->launch.stadium;s.plannedWeek=initial->seed.week;s.plannedYear=initial->seed.year;s.exceptionalOpening=initial->result.exceptional;}
    // This native reader supplies HomeTeamRank/AwayTeamRank to the game UI.
    using Rank=int(__fastcall*)(A,int);
    auto rank=reinterpret_cast<Rank>(image+0xa0b3000);
    s.rank0=rank(0,team0);s.rank1=rank(0,team1);
    if(s.rank0 < -1||s.rank0>255||s.rank1 < -1||s.rank1>255)return false;
    // Classification cannot change halfway through a match because a replay
    // or presentation asset briefly stops supplying its optional visual flag.
    A current=at<A>(at<A>(image+attendance::bindings::MatchWrapper)+16);
    if(contextReady&&shownConference&&audioMatch.load()==current&&shownTeam0==s.team0&&shownTeam1==s.team1&&shownBowl==s.bowl)attendance::postseason(s,attendance::playoffBowl(s.bowl),true);
    return true;
}
bool guardedContext(attendance::State& s){__try{return context(s);}__except(EXCEPTION_EXECUTE_HANDLER){return false;}}
void showContext(const attendance::Settings& c,const attendance::State& s){
    bool matchupChanged=!contextReady||shownRank0!=s.rank0||shownRank1!=s.rank1||shownTeam0!=s.team0||shownTeam1!=s.team1||shownRivalry!=s.rivalry||shownNeutral!=s.neutral||shownPlayoff!=s.playoff||shownConference!=s.conferenceChampionship||shownPrecipitation!=s.precipitation;
    contextReady=true;contextGameMode=at<int>(image+0xd9eb808);auto retained=s;applySessionPlan(retained,audioMatch.load());publishSessionPlan(audioMatch.load(),retained);
    if(matchupChanged){
        auto rankText=[](int rank){return rank>0?std::to_string(rank):std::string("unknown/unranked, raw ")+std::to_string(rank);};
        log("matchup","Matchup: home team ID "+std::to_string(s.team0)+" (rank "+rankText(s.rank0)+"); away team ID "+std::to_string(s.team1)+" (rank "+rankText(s.rank1)+"); rivalry "+(s.rivalry?"yes":"no")+"; neutral venue "+(s.neutral?"yes":"no")+"; conference championship "+(s.conferenceChampionship?"yes":"no")+"; postseason "+(s.playoff?"yes":"no")+"; weather "+(s.precipitation==1?"rain":s.precipitation==2?"snow":"dry")+".");
    }
    shownProfile=static_cast<int>(attendance::profileFor(c,s));shownRank0=s.rank0;shownRank1=s.rank1;
    shownRivalry=s.rivalry;shownRivalryEvent=s.rivalryEvent;shownPlayoff=s.playoff;shownConference=s.conferenceChampionship;shownBowl=s.bowl;shownPrecipitation=s.precipitation;
    shownNeutral=s.neutral;shownTeam0=s.team0;shownTeam1=s.team1;shownFcs=attendance::arrivals::fcsOpponent(s);
}
void readSettings(){
    static attendance::SettingsPoll poll;
    if(!poll.due(dataFolder/L"DynamicGamedayAtmosphere.ini",GetTickCount64()))return;
    auto c=std::make_shared<attendance::Settings>();auto path=dataFolder/L"DynamicGamedayAtmosphere.ini";
    auto location=[&](const wchar_t* section,const wchar_t* key){
        auto mapped=attendance::config::route(section,key);wchar_t found[4]{};
        GetPrivateProfileStringW(mapped.section,mapped.key,L"\x01",found,4,path.c_str());
        return found[0]==1?attendance::config::Route{section,key}:mapped;
    };
    auto number=[&](const wchar_t* section,const wchar_t* key,int fallback,int low,int high){
        auto field=location(section,key);
        wchar_t raw[128]{};GetPrivateProfileStringW(field.section,field.key,L"",raw,128,path.c_str());
        auto value=attendance::config::integer(raw,fallback,low,high);
        static std::map<std::wstring,std::wstring> warned;std::wstring id=std::wstring(field.section)+L"/"+field.key;
        if(value.status==attendance::config::Status::Invalid||value.status==attendance::config::Status::Clamped){if(warned[id]!=raw){warned[id]=raw;std::string name;for(auto letter:id)name.push_back(letter<=127?static_cast<char>(letter):'?');log("configuration",name+": invalid or out-of-range value; using "+std::to_string(value.value)+" (valid "+std::to_string(low)+" to "+std::to_string(high)+").");}}
        else warned.erase(id);
        return value.value;
    };
    auto percentage=[&](const wchar_t* section,const wchar_t* key,float fallback,float maximum,float minimum=0){
        wchar_t raw[128]{};auto field=location(section,key);GetPrivateProfileStringW(field.section,field.key,L"",raw,128,path.c_str());
        auto value=attendance::config::real(raw,fallback,minimum,maximum);
        static std::map<std::wstring,std::wstring> warned;std::wstring id=std::wstring(field.section)+L"/"+field.key;
        if(value.status==attendance::config::Status::Invalid||value.status==attendance::config::Status::Clamped){if(warned[id]!=raw){warned[id]=raw;std::string name;for(auto letter:id)name.push_back(letter<=127?static_cast<char>(letter):'?');log("configuration",name+": invalid or out-of-range value; using "+std::to_string(value.value)+" (valid "+std::to_string(minimum)+" to "+std::to_string(maximum)+").");}}
        else warned.erase(id);
        return value.value;
    };
    c->enabled=number(L"Attendance",L"Enabled",1,0,1)!=0;
    bool empty=number(L"Attendance",L"2020ModeEnabled",0,0,1)!=0;
    if(emptyMode2020.exchange(empty)!=empty)emptyModePending=true;
    initialSetting=number(L"Attendance",L"InitialAttendanceEnabled",1,0,1)!=0;
    pauseMenuSetting=number(L"Dynamic Gameday Broadcast",L"PauseMenuFullness",1,0,1)!=0;
    extraReplays=number(L"Replays",L"Enabled",1,0,1)!=0;
    routineInsets=number(L"Replays",L"InsetAfterRoutinePlays",1,0,1)!=0;
    bigPlayReplays=number(L"Replays",L"FullScreenBigPlays",1,0,1)!=0;
    defensiveReplays=number(L"Replays",L"DefensiveHighlights",1,0,1)!=0;
    replayMinimumYards=number(L"Replays",L"FullScreenMinimumYards",15,10,60);
    audioSetting=number(L"Audio",L"Enabled",1,0,1)!=0;
    busMixStrength=number(L"Dynamic Gameday Soundscape",L"CrowdMixStrengthPercent",100,0,100);
    attendanceProfile=number(L"Attendance",L"AttendanceProfile",1,0,1);
    arrivalEnabled=number(L"Arrivals",L"Enabled",1,0,1);arrivalStart=number(L"Arrivals",L"StartPercent",55,50,60);
    arrivalLateStart=number(L"Arrivals",L"LateSchoolStartPercent",45,40,50);
    arrivalFinish=number(L"Arrivals",L"FinishPercentOfQuarter",50,10,100);
    arrivalLateFinish=number(L"Arrivals",L"LateSchoolFinishPercentOfQuarter",75,10,100);
    halftimeExtra=number(L"Arrivals",L"HalftimeSeatBreakStrength",100,0,200);
    halftimeConvertPercent=number(L"Dynamic Gameday Attendance",L"HalftimeDepartureConversionPercent",25,0,25);
    arrivalBigBonus=number(L"Arrivals",L"BigGameBonus",3,0,3);
    arrivalWeatherPenalty=number(L"Arrivals",L"BadWeatherPenalty",5,0,8);

    logLimitMB=number(L"Logging",L"MaxFileMB",2,1,16);debugLog=number(L"Logging",L"Debug",0,0,1)!=0;
    c->comebackDeficit=number(L"Retention",L"ComebackOriginalDeficit",21,14,100);
    c->comebackRecovered=number(L"Retention",L"ComebackPointsRecovered",14,7,100);
    c->comebackDeteriorationSeconds=number(L"Retention",L"ComebackDeteriorationSeconds",60,0,600);
    c->warningEnabled=number(L"Retention",L"TwoMinuteWarning",1,0,1);
    c->quarterCap=number(L"Retention",L"MaximumDeparturePerQuarter",45,1,50);
    c->transitionCap=number(L"Retention",L"MaximumDeparturePerTransition",4,1,20);
    c->lateTraffic=number(L"Dynamic Gameday Attendance",L"LateTrafficEnabled",1,0,1);
    c->lateTrafficQuarterExtra=number(L"Dynamic Gameday Attendance",L"LateTrafficExtraQuarterAllowance",10,0,10);
    lateTurnoverMaximum=number(L"Dynamic Gameday Attendance",L"LateTurnoverExtraMax",10,0,15);
    c->formulaStrength=number(L"Retention",L"FormulaStrength",100,0,200);
    auto energy=std::make_shared<attendance::energy::Settings>();
    // Legacy event context is retained only for attendance protection; it no longer drives audio.
    auto core=std::make_shared<attendance::core::Settings>();
    core->enabled=number(L"Atmosphere",L"Enabled",1,0,1)!=0;
    core->opening=number(L"Atmosphere",L"OpeningAtmosphere",1,0,1)!=0;
    core->overtime=number(L"Atmosphere",L"OvertimeAtmosphere",1,0,1)!=0;
    core->lineups=number(L"Atmosphere",L"PlayCallAtmosphere",1,0,1)!=0;
    core->skillMoves=number(L"Atmosphere",L"SkillMoveReactions",1,0,1)!=0;
    core->staggeredCheers=number(L"Atmosphere",L"StaggeredMajorCheers",1,0,1)!=0;
    core->contextReactions=number(L"Atmosphere",L"PackedBigGameReactions",1,0,1)!=0;
    core->upsetWatch=number(L"Atmosphere",L"UpsetWatchAtmosphere",1,0,1)!=0;
    core->cameraShake=number(L"Atmosphere",L"CameraShake",1,0,1)!=0;
    nativeShakeMinimum=number(L"Dynamic Gameday Broadcast",L"NativeShakeMinimumFullnessPercent",50,0,100);
    auto motion=std::make_shared<attendance::motion::Settings>();
    motion->awayPercent=100.f*static_cast<float>(number(L"Dynamic Gameday Broadcast",L"AwayTeamReactionShakeEnabled",1,0,1));
    dynamicAwayFans=number(L"Dynamic Gameday Attendance",L"DynamicAwayCrowds",1,0,1)!=0;
    visitorCrowdInfluence=static_cast<int>(number(L"Dynamic Gameday Attendance",L"VisitorCrowdInfluencePercent",100,0,200));
    motion->neutralPercent=static_cast<float>(number(L"Dynamic Gameday Broadcast",L"NeutralSiteShakePercent",75,0,100));
    motion->widePlayerCompensation=number(L"Dynamic Gameday Broadcast",L"WidePlayerCameraCompensation",0,0,1)!=0;
    motion->cameraPercent[0]=static_cast<float>(number(L"Dynamic Gameday Broadcast",L"ZoomCameraShakePercent",100,0,200));
    motion->cameraPercent[1]=static_cast<float>(number(L"Dynamic Gameday Broadcast",L"StandardCameraShakePercent",100,0,200));
    motion->cameraPercent[2]=static_cast<float>(number(L"Dynamic Gameday Broadcast",L"WideCameraShakePercent",100,0,200));
    motion->cameraPercent[3]=static_cast<float>(number(L"Dynamic Gameday Broadcast",L"All22CameraShakePercent",100,0,200));
    motion->cameraPercent[4]=static_cast<float>(number(L"Dynamic Gameday Broadcast",L"BroadcastCameraShakePercent",100,0,200));
    motion->ordinary=static_cast<float>(number(L"Atmosphere",L"OrdinaryShakeStrengthPercent",5,0,100));
    motion->critical=static_cast<float>(number(L"Atmosphere",L"CriticalShakeStrengthPercent",10,0,100));
    motion->reactionFadeMs=static_cast<unsigned>(1000*percentage(L"Dynamic Gameday Broadcast",L"PlayReactionFadeSeconds",5.f,8,1));
    motion->impactExtraMs=static_cast<unsigned>(1000*percentage(L"Dynamic Gameday Broadcast",L"PlayImpactExtraSeconds",1,5));
    motion->catchImpact=percentage(L"Dynamic Gameday Broadcast",L"CatchImpactShakeStrengthPercent",12,30);
    motion->lineupCheer=percentage(L"Dynamic Gameday Broadcast",L"LineupCheerShakeStrengthPercent",6,100);
    motion->hostileOpeningMaximum=percentage(L"Dynamic Gameday Broadcast",L"HostileOpeningShakeMaximumPercent",10,30);
    motion->touchdown=static_cast<float>(number(L"Atmosphere",L"TouchdownShakeStrengthPercent",16,0,100));
    motion->eruption=percentage(L"Atmosphere",L"EruptionShakeStrengthPercent",22.5f,100);
    motion->touchdownImpact=static_cast<float>(number(L"Atmosphere",L"TouchdownShakeImpactPercent",20,0,100));
    motion->eruptionImpact=static_cast<float>(number(L"Atmosphere",L"EruptionShakeImpactPercent",30,0,100));
    motion->touchdownMs=1000u*number(L"Atmosphere",L"TouchdownShakeDurationSeconds",11,3,30);
    reactionDistanceMaximum=0; // Play length never extends the shared settling fade.
    motion->eruptionMs=1000u*number(L"Atmosphere",L"EruptionShakeDurationSeconds",16,5,60);
    motion->live=percentage(L"Dynamic Gameday Broadcast",L"DuringPlayShakeStrengthPercent",11,30);
    motion->cannon=percentage(L"Dynamic Gameday Broadcast",L"CannonShakeStrengthPercent",18,30);
    auto liveSettings=std::make_shared<attendance::live::Settings>();
    liveSettings->enabled=number(L"Dynamic Gameday Soundscape",L"DuringPlayReactionsEnabled",1,0,1)!=0;
    liveSettings->minimumCatchYards=static_cast<float>(number(L"Dynamic Gameday Soundscape",L"LongCompletionMinimumYards",20,10,60));
    liveSettings->minimumGainYards=static_cast<float>(number(L"Dynamic Gameday Soundscape",L"LongGainMinimumYards",15,10,60));
    liveSettings->passFlight=number(L"Dynamic Gameday Soundscape",L"PassFlightAnticipation",1,0,1)!=0;
    situationalReactionsEnabled=number(L"Dynamic Gameday Soundscape",L"SituationalCrowdReactions",1,0,1)!=0;
    std::shared_ptr<const attendance::live::Settings> nextLive=liveSettings;std::atomic_store(&liveReactionSettings,nextLive);
    std::shared_ptr<const attendance::motion::Settings> nextMotion=motion;std::atomic_store(&cameraSettings,nextMotion);
    core->chantMaximumPercent=percentage(L"Atmosphere",L"ChantMaximumBoostPercent",15,15);
    core->bigGameMinimum=number(L"Atmosphere",L"BigGameMinimumFullnessPercent",55,55,90)*.01f;
    core->eruptions=number(L"Atmosphere",L"WinningEruptions",1,0,1)!=0;
    core->packedHouse=number(L"Atmosphere",L"PackedHouseBuildup",1,0,1)!=0;
    core->driveEscalation=number(L"Atmosphere",L"DriveEscalation",1,0,1)!=0;
    busMaximumBoost=percentage(L"Atmosphere",L"MaximumBoostPercent",30,30);
    core->maximumPercent=(std::min)(busMaximumBoost.load(),attendance::core::NormalMaximumPercent);
    core->highLeveragePercent=percentage(L"Atmosphere",L"HighLeverageBoostPercent",attendance::core::HighLeverageMaximumPercent,attendance::core::HighLeverageMaximumPercent);
    core->eruptionPercent=percentage(L"Atmosphere",L"EruptionBoostPercent",attendance::core::EruptionMaximumPercent,attendance::core::EruptionMaximumPercent);
    std::shared_ptr<const attendance::core::Settings> nextCore=core;std::atomic_store(&coreSettings,nextCore);
    energy->lateScoreDepartures=number(L"Retention",L"LateScoreExtra",5,0,25);
    std::shared_ptr<const attendance::energy::Settings> nextEnergy=energy;std::atomic_store(&retentionEventSettings,nextEnergy);
    wchar_t profile[64]{};auto profileField=location(L"Attendance",L"Profile");GetPrivateProfileStringW(profileField.section,profileField.key,L"Auto",profile,64,path.c_str());
    if(!_wcsicmp(profile,L"Default"))c->profile=attendance::Profile::Default;
    else if(!_wcsicmp(profile,L"Reduced"))c->profile=attendance::Profile::Reduced;
    else if(!_wcsicmp(profile,L"BadWeather"))c->profile=attendance::Profile::BadWeather;
    static std::wstring invalidProfile;
    if(_wcsicmp(profile,L"Auto")&&_wcsicmp(profile,L"Default")&&_wcsicmp(profile,L"Reduced")&&_wcsicmp(profile,L"BadWeather")){if(invalidProfile!=profile){invalidProfile=profile;log("configuration","Unknown attendance Profile; using Auto. Valid choices: Auto, Default, Reduced, BadWeather.");}}
    else invalidProfile.clear();
    std::shared_ptr<const attendance::Settings> next=c;std::atomic_store(&settings,next);
    static bool missingAudioReported=false;
    bool missingAudio=enabled&&audioSetting&&!audioHookInstalled;
    if(missingAudio&&!missingAudioReported)log("configuration","Soundscape is requested but native audio hooks are unavailable; check the binding failures above.");
    missingAudioReported=missingAudio;
}
int sackPlayIdentity(A tracker){
    return attendance::sackfacts::selectedPlay(image,tracker,read);
}
bool defensiveFacts(bool& sack,bool& forcedFumble){
    // These are the game's presentation predicates. The sentinel requests
    // any sacker rather than filtering to a selected player; no gameplay writes.
    __try{
        sack=reinterpret_cast<bool(__fastcall*)(int)>(image+attendance::bindings::SackPredicate)(0xfffff);
        forcedFumble=reinterpret_cast<bool(__fastcall*)()>(image+attendance::bindings::ForcedFumblePredicate)();
        return true;
    }__except(EXCEPTION_EXECUTE_HANDLER){sack=forcedFumble=false;return false;}
}
bool playData(A match,attendance::energy::Play& p){
    p.sackSampled=false;
    p.sackPlay=-1;
    if(!playBindingsAvailable)return false;
    if(!read(match+0x1d0,&p.down,4)||!read(match+0x1c8,&p.offense,4))return false;
    if(p.down < -2||p.down>13||p.offense<0||p.offense>1)return false;
    A wrapper=at<A>(image+attendance::bindings::MatchWrapper),rules=at<A>(wrapper+0x18);
    if(at<A>(rules)==image+attendance::bindings::RulesVtable){
        int count=-1;if(read(rules+0x4c,&count,4)&&count>=0&&count<=3)p.homeTimeouts=count;
        count=-1;if(read(rules+0x50,&count,4)&&count>=0&&count<=3)p.awayTimeouts=count;
    }
    unsigned flags=0;if(read(match+0x280,&flags,4)){p.firstDown=(flags&(1u<<10))!=0;p.blockedFieldGoal=fieldGoalBlockBinding&&(flags&(1u<<8))!=0;}
    using FirstLine=float(__fastcall*)(A);
    float line=reinterpret_cast<FirstLine>(image+attendance::bindings::FirstDownLine)(match),spot=0;
    if(read(match+0x16c,&spot,4)&&std::isfinite(line)&&std::isfinite(spot)){
        float distance=std::fabs(line-spot);if(distance<=100)p.toFirst=distance;
        if(std::fabs(spot)<=50.01f&&std::fabs(line)<=50.01f){
            p.fieldSpot=spot;
            if(distance>0.01f)p.attackDirection=line>spot?1:-1;
            // Before the snap the verified first-down line identifies the attacking
            // end zone. This does not depend on which team the user controls.
            if(p.game.phase==1&&p.attackDirection)p.toGoal=std::fabs(p.attackDirection*50.0f-spot);
        }
        // A first-down line at an end-zone boundary is a verified goal-to-go
        // situation. This remains available without a selected-player context.
        if(std::fabs(std::fabs(line)-50)<0.01f&&distance<=10)p.toGoal=distance;
    }
    // Optional presentation field-position source. Do not use its default
    // value when the game has no active selected-player context.
    int index=at<int>(image+attendance::bindings::SelectedPlayer);
    if(index>=0&&index<2){
        A actor=at<A>(image+attendance::bindings::PlayerContexts+static_cast<A>(index)*8),field=at<A>(actor+0xa8);
        float goal=0,current=0;
        if(field&&read(field+0xc4,&goal,4)&&read(match+0x30,&current,4)&&std::isfinite(goal)&&std::isfinite(current)&&std::fabs(std::fabs(goal)-50)<0.01f){
            // Verify the first-down line points toward this end zone. This
            // avoids treating a team backed up at its own goal as attacking it.
            if((line-spot)*(goal-spot)>0&&std::fabs(line-spot)>0.01f){
                float distance=std::fabs(goal-spot);if(distance<=100)p.toGoal=distance;
            }
        }
    }
    A tracker=at<A>(image+attendance::bindings::PlayStats),record=at<A>(tracker+0xcd8);
    unsigned char result[48]{};
    if(record&&read(record+0x58,result,sizeof(result))){
        uint64_t stamp=1469598103934665603ull;
        for(unsigned char b:result){stamp^=b;stamp*=1099511628211ull;}
        // Include native result identity so equally shaped OT plays remain
        // distinguishable while the untimed game clock stays at zero.
        stamp^=static_cast<uint64_t>(record);stamp*=1099511628211ull;
        int kind=at<int>(record+0x64);p.resultStamp=stamp;p.interception=kind==2;
        if(liveReactionBindings&&p.game.phase==3&&(kind==3||kind==1)){
            A control=at<A>(at<A>(image+attendance::bindings::BallControl)+0x10);
            p.passReleased=at<A>(control)==image+0xb47bb10&&at<int>(control+0x168)==1&&at<A>(control+0x1a0)==0&&
                at<A>(tracker+0xcd8)==record&&at<int>(record+0x64)==kind;
        }
        if(fumbleBinding){A ball=at<A>(at<A>(image+attendance::bindings::BallControl)+0x10);int substate=at<int>(ball+0x158);
            p.fumble=at<A>(ball)==image+0xb47bb10&&attendance::live::isFumbleState(at<int>(ball+0x168),substate);}
        if(defensiveReactionBindings&&enabled&&audioSetting&&std::atomic_load(&coreSettings)->enabled&&(p.game.phase==4||p.game.phase==5||p.game.phase==1||p.game.phase==7)){
            // The predicate's completed-play selector has its own identity,
            // separate from the mutable current result hash.
            static thread_local A cachedMatch=0;static thread_local unsigned cachedEpoch=0;
            static thread_local uint64_t cachedStamp=0;static thread_local int cachedPhase=-1;
            static thread_local int cachedPlay=-1;
            static thread_local bool cachedSack=false,cachedForced=false;
            unsigned generation=audioEpoch.load();
            int selected=sackPlayIdentity(tracker);
            if(cachedMatch!=match||cachedEpoch!=generation||cachedStamp!=stamp||cachedPhase!=p.game.phase||cachedPlay!=selected){
                cachedMatch=match;cachedEpoch=generation;cachedStamp=stamp;cachedPhase=p.game.phase;
                cachedPlay=selected;
                if(!defensiveFacts(cachedSack,cachedForced)){defensiveReactionBindings=false;log("defensive-reactions-unavailable","Native result check failed; flag-based sack/fumble reactions disabled. Measured five-yard losses and other effects remain supported.");}
            }
            p.sackPlay=sackPlayIdentity(tracker);
            p.sackSampled=defensiveReactionBindings&&selected>=0&&p.sackPlay==selected;
            p.sack=cachedSack&&p.sackSampled;p.forcedFumble=cachedForced&&p.fumble;
        }
    }
    liveDown=p.down;liveOffense=p.offense;liveHomeTimeouts=p.homeTimeouts;return true;
}

bool majorReplayWorld(attendance::State& state,A& match){
    if(!enabled||mode==2||!extraReplays||!replayPresentationInstalled||replayPresentationFailed||!pauseBinding||!replayBinding||!playBindingsAvailable)return false;
    if(!gameState(state,match))return false;
    A config=at<A>(image+0xf63f9f0);unsigned char online=1;
    return config&&read(config+0x84,&online,1)&&online==0&&at<A>(match)==image+attendance::bindings::MatchVtable;
}
attendance::replaymode::Sample nativeReplayMode(){
    using namespace attendance::bindings;
    attendance::replaymode::Sample result;
    if(!replayModeBinding)return result;
    A sim=at<A>(image+ReplaySimApi),flow=at<A>(image+ReplayFlowApi);
    unsigned char open=0;
    if(at<A>(sim)!=image+ReplaySimApiVtable||at<A>(flow)!=image+ReplayFlowApiVtable||!read(flow+0x449,&open,1)||open>1)return result;
    __try{
        result.realtime=reinterpret_cast<bool(__fastcall*)()>(image+ReplayRealtimeQuery)();
        result.playcallOpen=open!=0;result.known=true;
    }__except(EXCEPTION_EXECUTE_HANDLER){result={};}
    return result;
}
bool enterNativeInset(){
    __try{
        reinterpret_cast<A(__fastcall*)()>(image+attendance::bindings::PresentationClient)();
        return reinterpret_cast<bool(__fastcall*)(int,int)>(image+attendance::bindings::PresentationEvent)(7,0x212);
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
bool finishReplayEvent(A node){
    __try{*reinterpret_cast<unsigned short*>(node+0x1a)=3;return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
void __fastcall replayEventNode(A node){
    if(!enabled||mode==2||!extraReplays||!replayPresentationInstalled||replayPresentationFailed||
       at<A>(node)!=image+attendance::bindings::ReplayEventNodeVtable||at<int>(node+0x50)!=12){originalReplayEventNode(node);return;}
    attendance::timing::ExclusiveScope<> measured(replaySelectorTimings,&replaySelectorWorkNs);
    bool handled=false;
    if(majorReplayBindings&&!majorReplayFailed)guardedMajorReplay(false);
    if(replayMode==attendance::replaymode::Mode::PlayCall&&majorReplayBindings&&!majorReplayFailed&&extraReplays&&bigPlayReplays&&routineInsets&&
       at<A>(node)==image+attendance::bindings::ReplayEventNodeVtable&&at<int>(node+0x50)==12&&nativeUnpaused()){
        attendance::State state;A match=0;
        if(majorReplayWorld(state,match)&&(state.phase==4||state.phase==5)){
            std::lock_guard<std::mutex> guard(majorReplayMutex);
            handled=replayTouchdown.allowed()&&majorReplay.suppressInset(match,audioEpoch.load());
        }
    }
    if(at<A>(node)==image+attendance::bindings::ReplayEventNodeVtable&&at<int>(node+0x50)==12)
        recordReplaySelection(node,10,0,handled?1:0,replayMode==attendance::replaymode::Mode::PlayCall,at<int>(at<A>(image+attendance::bindings::GameStates)+0xa0));
    if(handled){if(finishReplayEvent(node))return;majorReplayFailed=true;}
    measured.pause();{attendance::timing::Scope nativeMeasured(replayEventNativeTimings);originalReplayEventNode(node);}
}
void __fastcall nativeReplayNode(A node,float delta){
    if(!enabled||mode==2||!extraReplays||!replayPresentationInstalled||replayPresentationFailed){originalNativeReplayNode(node,delta);return;}
    attendance::timing::ExclusiveScope<> measured(replaySelectorTimings,&replaySelectorWorkNs);
    bool track=false;A match=0;unsigned epoch=audioEpoch.load();attendance::State state;
    if(majorReplayBindings&&!majorReplayFailed&&at<A>(node)==image+attendance::bindings::ReplayNodeVtable&&
       at<unsigned short>(node+0x1a)!=3&&majorReplayWorld(state,match)&&state.phase==4){
        std::lock_guard<std::mutex> guard(majorReplayMutex);
        track=majorReplay.owns(match,epoch);if(track)majorReplay.node(match,epoch,node,false);
    }
    bool traceNode=at<A>(node)==image+attendance::bindings::ReplayNodeVtable&&at<unsigned short>(node+0x1a)!=3;
    if(traceNode)recordReplaySelection(node,11,at<unsigned short>(node+0x1a),-1,track,at<int>(at<A>(image+attendance::bindings::GameStates)+0xa0));
    measured.pause();
    if(traceNode){attendance::timing::Scope nativeMeasured(replayNodeNativeTimings);originalNativeReplayNode(node,delta);}
    else originalNativeReplayNode(node,delta);
    measured.resume();
    if(traceNode&&at<A>(node)==image+attendance::bindings::ReplayNodeVtable&&at<unsigned short>(node+0x1a)==3)
        recordReplaySelection(node,12,3,3,track,at<int>(at<A>(image+attendance::bindings::GameStates)+0xa0));
    if(track&&at<A>(node)==image+attendance::bindings::ReplayNodeVtable&&at<unsigned short>(node+0x1a)==3){
        std::lock_guard<std::mutex> guard(majorReplayMutex);majorReplay.node(match,epoch,node,true);
    }
}
void observeMajorReplay(bool dispatch){
    static thread_local bool executing=false;if(executing)return;
    attendance::timing::Scope measured(majorReplayTimings);
    struct Guard{bool& value;Guard(bool& v):value(v){value=true;}~Guard(){value=false;}} guard(executing);
    attendance::State state;A match=0;bool world=majorReplayWorld(state,match);auto now=GetTickCount64();unsigned epoch=audioEpoch.load();
    A manager=at<A>(image+attendance::bindings::ReplayManager);bool replay=at<A>(manager)==image+attendance::bindings::ReplayVtable&&at<unsigned char>(manager+0x38)!=0;
    bool paused=!nativeUnpaused();attendance::energy::Play play;play.game=state;
    bool sampled=world&&!replay&&!paused&&playData(match,play);
    if(sampled){
        play.selectedType=playCallBinding?attendance::playcall::View(image,read).type(match,play.offense):-1;
        play.specialTeams=play.selectedType>0&&!attendance::lineup::offensivePlay(play.selectedType);play.simulated=offscreenSimulation(state);
    }
    auto modeSample=world?nativeReplayMode():attendance::replaymode::Sample{};
    bool modeChanged=false;attendance::replaymode::Mode presentationMode=attendance::replaymode::Mode::Unknown;
    bool send=false;attendance::majorreplay::Notice notice=attendance::majorreplay::Notice::None;uint64_t sequence=0;
    {
        std::lock_guard<std::mutex> lock(majorReplayMutex);
        if(!world||match!=replayTrackerMatch||epoch!=replayTrackerEpoch){majorReplay.reset();replayTouchdown.reset();replayPlayTracker.reset();replayTrackerMatch=match;replayTrackerEpoch=epoch;replaySnapType=-1;replayRecordingStart=-1;replayRecordingSequence=0;}
        presentationMode=replayModeTracker.update(world?match:0,epoch,modeSample);modeChanged=replayMode.exchange(presentationMode)!=presentationMode;
        double recording=-1;A recorder=at<A>(image+attendance::bindings::ReplayManager);
        if(world&&replayBinding&&at<A>(recorder)==image+attendance::bindings::ReplayVtable)read(recorder+0x1030,&recording,sizeof(recording));
        if(sampled){
            replayObserved=play;auto result=replayPlayTracker.observe(*std::atomic_load(&retentionEventSettings),play);
            if(play.game.phase==3&&!play.simulated&&replayPlayTracker.serial!=replayRecordingSequence){replayRecordingSequence=replayPlayTracker.serial;replayRecordingStart=recording;}
            if(replayPlayTracker.serial)replaySnapType=replayPlayTracker.start.selectedType;
            replayTouchdown.observe(match,epoch,replayPlayTracker.serial,
                replayPlayTracker.start.game.homeScore,replayPlayTracker.start.game.awayScore,
                play.game.homeScore,play.game.awayScore,result.accepted&&result.completed,play.simulated);
            if(!replayTouchdown.allowed())majorReplay.reset();
            if(majorReplayBindings&&!majorReplayFailed&&bigPlayReplays&&replayTouchdown.allowed()&&result.accepted&&result.completed&&result.takeaway)
                majorReplay.arm(match,epoch,now,result.sequence,attendance::majorreplay::Kind::Takeaway,routineInsets?presentationMode:attendance::replaymode::Mode::Unknown);
        }
        attendance::majorreplay::Input input{match,epoch,now,state.phase,world&&replayTouchdown.allowed()&&majorReplayBindings&&!majorReplayFailed&&bigPlayReplays,paused,replay,dispatch,routineInsets?presentationMode:attendance::replaymode::Mode::Unknown,modeSample.playcallOpen};
        input.recordingReady=replayRecordingSequence==majorReplay.sequence()&&attendance::majorreplay::recordingAvailable(replayRecordingStart,recording);
        send=majorReplay.tick(input);notice=majorReplay.takeNotice();sequence=majorReplay.sequence();
    }
    if(modeChanged&&world){
        using M=attendance::replaymode::Mode;
        log("major-replay",presentationMode==M::RealtimeSim?"Mode: real-time/Slow Sim; native full-screen playback retained, no added inset destination.":
            presentationMode==M::PlayCall?"Mode: play-call UI verified; separate inset routing available after full-screen playback.":
            "Mode: play-call destination not verified; original inset events retained, no added inset requests.");
    }
    const char* message=nullptr;using N=attendance::majorreplay::Notice;
    switch(notice){case N::Armed:message="major play confirmed; full-screen native sequence required before inset";break;
      case N::FullStarted:message="native playback observed in full-screen post-play stage";break;
      case N::FullFinished:message="required native full-screen clip(s) completed; waiting for separate inset";break;
      case N::InsetRequested:message="separate inset repeat requested after completed full-screen playback";break;
      case N::InsetStarted:message="native playback observed in play-selection inset stage";break;
      case N::Finished:message="native inset stage ended";break;
      case N::NativeOnly:message="full-screen native route retained; no added inset expected or requested without a verified play-call destination (Slow Sim, unavailable UI or inset option off)";break;
      case N::Unavailable:message="major sequence could not be completed with available native recording/presentation; original flow retained";break;
      case N::FallbackInset:message="native full-screen playback was not observed; returning the held native inset request at verified play selection (rendered playback still requires confirmation)";break;
      case N::RecordedFallback:message="no full-screen or authored inset reached playback; confirmed TD has fresh same-snap recording; one native inset recovery requested at stable play selection (rendered playback still requires confirmation)";break;
      default:break;}
    if(message)log("major-replay","Play #"+std::to_string(sequence)+": "+message+".");
    if(send){
        // The destination can disappear between the model tick and native
        // dispatch (e.g. opening Super Sim). Recheck without treating a normal
        // mode change as a fault or replaying this request on return.
        auto current=nativeReplayMode();
        A wrapper=at<A>(image+attendance::bindings::MatchWrapper);
        if(!current.known||current.realtime||at<A>(wrapper+16)!=match||audioEpoch.load()!=epoch){
            std::lock_guard<std::mutex> lock(majorReplayMutex);
            majorReplay.tick({match,epoch,now,state.phase,true,false,replay,false,attendance::replaymode::Mode::Unknown,false});
            return;
        }
    }
    if(send&&!extraReplayForCurrentPlay())return;
    if(send&&!enterNativeInset()){
        {std::lock_guard<std::mutex> lock(majorReplayMutex);majorReplay.dispatchFailed();}
        majorReplayFailed=true;log("major-replay","Native inset handoff failed; major-play additions stopped, original playback remains active.");
    }
}
bool guardedMajorReplay(bool dispatch){
    __try{observeMajorReplay(dispatch);return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){majorReplayFailed=true;return false;}
}

void updateDiagnostic(uint64_t now){
    auto queued=std::atomic_load(&diagnosticRequest);auto old=std::atomic_load(&diagnosticPreview);
    if(queued->token==diagnosticProcessed&&!old->active&&!old->cooling)return;
    diagnosticProcessed=queued->token;auto wanted=audioWanted();
    attendance::State state;A match=0;bool game=gameState(state,match);auto density=std::atomic_load(&renderedOccupancy);
    bool primary=!queued->sound||!attendance::diagnostic::primary(queued->preset)||(swellAvailable&&sustainedAudio.healthy());
    bool support=!queued->sound||!attendance::diagnostic::support(queued->preset)||(swellAvailable&&supportingSwell.healthy());
    bool cheers=!queued->sound||!attendance::diagnostic::cheer(queued->preset)||(cheerAvailable&&positiveCheer.healthy());
    bool sounds=primary&&support&&cheers;
    bool camera=!queued->camera||(cameraAvailable&&presentationCamera.healthy());
    attendance::diagnostic::Context context;context.match=game?match:0;context.epoch=audioEpoch.load();context.now=now;context.phase=game?state.phase:0;
    context.allowed=game&&contextReady&&wanted.active&&mode==0&&enabled&&audioSetting&&nativeUnpaused()&&audioAudible&&replayLoss<0&&!replaySuspendsAtmosphere&&!offscreenSimulation(state)&&density->owns(match,context.epoch)&&sounds&&camera;
    context.fullness=audibleFullness(wanted);
    if(!sounds)context.unavailable="not started: a requested native sound control is unavailable; check status/logs";
    else if(!camera)context.unavailable="not started: added camera control is unavailable; check status/logs";
    std::shared_ptr<const attendance::diagnostic::Preview> preview=std::make_shared<attendance::diagnostic::Preview>(diagnosticModel.update(*queued,context,*std::atomic_load(&coreSettings),*std::atomic_load(&cameraSettings)));
    std::atomic_store(&diagnosticPreview,preview);
    auto previous=std::atomic_load(&diagnosticStatus);
    if(*previous!=diagnosticModel.status()){
        std::shared_ptr<const std::string> text=std::make_shared<std::string>(diagnosticModel.status());std::atomic_store(&diagnosticStatus,text);log("diagnostics",*text);
    }
}
void atmosphereTick(){
    attendance::timing::Scope measured(atmosphereTimings);
    auto now=GetTickCount64();uint64_t interval=lastAtmospherePhase==3||lastAtmospherePhase==4?16:50;
    if(now-lastAtmosphereTick<interval)return;lastAtmosphereTick=now;
    flushReplaySelection();
    updateDiagnostic(now);
    static unsigned loggedCannons=0;auto fired=cannonStarts.load();if(fired!=loggedCannons){
        loggedCannons=fired;auto blast=std::atomic_load(&cannonBlast);float strength=std::atomic_load(&cameraSettings)->cannon;
        std::ostringstream message;message<<"Native cannon #"<<fired<<", variant "<<blast->effect-9<<" started; ";
        if(strength>0)message<<strength<<"% short impact requested alongside the existing cannon audio, with no extra sound or volume boost.";
        else message<<"cannon shake disabled (0%); existing cannon audio unchanged.";
        log("cannon",message.str());
    }
    auto c=std::atomic_load(&retentionEventSettings);
    auto gap=[&](){
        std::shared_ptr<const attendance::nativecamera::Admission> unknown=std::make_shared<attendance::nativecamera::Admission>();std::atomic_store(&nativeCameraAdmission,unknown);
        std::shared_ptr<const attendance::sustained::Intent> quiet=std::make_shared<attendance::sustained::Intent>();std::atomic_store(&sustainedIntent,quiet);
        auto wanted=audioWanted();std::lock_guard<std::mutex> guard(atmosphereMutex);
        finishLiveAudit("pause, replay or game-scene interruption");shownOpeningDrive=attendance::lineup::OpeningAdmission::Inactive;shownHostileOpening=shownHostileSeries=false;hostileOpeningResult.suspend();
        shownUpsetStage=0;shownUpsetSound=false;shownUpsetMinimum=1.01f;
        shownPassGate="paused, replaying or game scene unavailable";
        finalReactionCarry.suspend();lineupAtmosphere.suspend();openingKickoffAtmosphere.suspend();liveReactions.suspend();skillReactions.reset();catchKnockouts.reset();defensiveAccents.reset();defensiveMotion.suspend();showboating.suspend();skillSwell.reset();contextReactions.reset();liveReactionMajor=liveReactionMilestone=false;awaySoundReactions.reset();awayCameraMotion.reset();awayLastCue={};awayHeardId=awayHeardAt=awayHeardSequence=awayExtraHeardId=0;extraReactionId=extraReactionUntil=0;celebrationReactionType=91;situationalReactions.reset();hailMaryPass.reset();liveFumbles.reset();fumbleContacts.reset();returnAtmosphere.reset();scoringSoundHandoff.reset();reactionDistance.reset();passFlight.reset();liveReactionUntil=0;shownLineup=false;lastAtmospherePhase=-1;
        std::shared_ptr<const attendance::motion::Cue> quietCamera=std::make_shared<attendance::motion::Cue>(cameraMotion.suspend(now));std::atomic_store(&cameraCue,quietCamera);
        shownHighUntil=0;
        crowdBoost=audioContinuity.gap(wanted.match,now/1000.0,wanted.active&&mode==0&&std::atomic_load(&coreSettings)->enabled,attendance::core::normalLimit(*std::atomic_load(&coreSettings)));
        shownAudioLimit=attendance::core::normalLimit(*std::atomic_load(&coreSettings));
        if(crowdBoost<=0)shownAmbienceLift=shownOpeningWeight=0;
    };
    attendance::energy::Play p;A match=0;
    if(!gameState(p.game,match)||!contextReady){gap();return;}
    lastAtmospherePhase=p.game.phase;
    if(replaySuspendsAtmosphere){gap();return;}
    refreshStadiumOccupancy(match,audioEpoch.load(),now);refreshRenderedOccupancy();
    if(p.game.phase!=6&&pauseBinding&&!nativeUnpaused()){
        // The postgame stats/highlights menu retains phase 7 and pauses the
        // native world. End the carry once so highlight playback cannot restart it.
        if(p.game.phase==7){
            bool ended=false;{std::lock_guard<std::mutex> guard(atmosphereMutex);ended=coreAtmosphere.finishPostgame();}
            shownEruption=shownPostgameCarry=false;shownDecisiveUntil=0;
            if(ended)log("roar","Postgame menu opened; winning celebration roar released.");
        }
        gap();return;
    }
    // Matchup atmosphere uses the verified basic game context even if optional
    // play details are unavailable. Those details only gate specific events.
    if(p.game.phase!=6)playData(match,p);
    int selectedPlayType=playCallBinding?attendance::playcall::View(image,read).type(match,p.offense):-1;
    p.specialTeams=selectedPlayType>0&&!attendance::lineup::offensivePlay(selectedPlayType);
    p.selectedType=selectedPlayType;
    p.simulated=offscreenSimulation(p.game);p.resumed=shownResumeSource!=0;p.game.variationSeed=attendanceSeed;
    p.game.rank0=shownRank0;p.game.rank1=shownRank1;p.game.rivalry=shownRivalry;p.game.rivalryEvent=shownRivalryEvent;p.game.playoff=shownPlayoff;
    p.game.conferenceChampionship=shownConference;p.game.hfaActive=hfaBinding&&!shownNeutral&&attendance::nativecontext::hfa(image);shownHfa=p.game.hfaActive;
    p.game.neutral=shownNeutral;p.game.team0=shownTeam0;p.game.team1=shownTeam1;p.game.bowl=shownBowl;applySessionPlan(p.game,match);
    if(p.game.phase==3){std::lock_guard<std::mutex> guard(historyMutex);matchLifecycle.live();}
    std::lock_guard<std::mutex> guard(atmosphereMutex);
    if(atmosphereMatch!=match){shownPassFlights=0;playTracker.reset();soundOnlyReactions.reset();lateTurnoverExits.reset();coreAtmosphere.reset();lineupAtmosphere.reset();hostileOpeningResult.reset();openingKickoffAtmosphere.reset();presentationAtmosphere.reset();openingWelcome.reset();cameraMotion.reset();finalReactionCarry.reset();liveReactions.reset();skillReactions.reset();catchKnockouts.reset();defensiveAccents.reset();defensiveMotion.reset();showboating.reset();skillSwell.reset();contextReactions.reset();liveReactionMajor=liveReactionMilestone=false;liveMotionEvent=attendance::live::Event::None;awaySoundReactions.reset();awayCameraMotion.reset();awayLastCue={};awayHeardId=awayHeardAt=awayHeardSequence=awayExtraHeardId=0;extraReactionId=extraReactionUntil=0;celebrationReactionType=91;situationalReactions.reset();hailMaryPass.reset();liveFumbles.reset();fumbleContacts.reset();returnAtmosphere.reset();scoringSoundHandoff.reset();reactionDistance.reset();passFlight.reset();liveReactionId=liveReactionUntil=liveSourceSequence=liveMotionSequence=liveAuditId=liveAuditUntil=playAuditSequence=playAuditEndSequence=0;liveReactionPost=false;shownLiveCatches=shownLivePicks=0;audioContinuity.reset();pendingLateExits=0;atmosphereMatch=match;}
    if(liveAuditId&&now>=liveAuditUntil)finishLiveAudit(positiveCheer.healthy()?"no eligible crowd callback in the immediate reaction window":"crowd reaction control stopped after a fault");
    // The tracker rejects rewound/corrected scores and clocks for event
    // detection only. Verified live ambience still needs its normal handoff;
    // pause, interactive replay and world ownership were checked above.
    if(p.game.phase==6)playTracker.primeOpeningIntro(p);
    auto result=p.game.phase==6?attendance::energy::Outcome{}:playTracker.observe(*c,p);
    if(!resultFreshness.observe(match,audioEpoch.load(),playTracker.serial,p.game.phase,now,result)){
        log("play-summary","Play #"+std::to_string(result.sequence)+": late official result ("+attendance::energy::name(result.event)+", "+std::to_string(result.yards)+" yards, phase "+std::to_string(p.game.phase)+"); new added reactions expired after the presentation delay. Existing live reactions keep their original timing.");
        result={};
    }
    awaySoundReactions.track(*c,p,playTracker.serial);
    // Keep the verified call frozen through the whistle: the native selected
    // play can already describe the NEXT lineup while this result settles.
    bool clockPlay=attendance::energy::clockManagementContext(p,playTracker.start,playTracker.serial);
    auto rendered=std::atomic_load(&renderedOccupancy);unsigned epoch=audioEpoch.load();
    bool fullnessReady=rendered->owns(match,epoch);
    // Historical clip fullness may change the audible presentation, but must
    // never feed a past crowd into live event eligibility or cancel an earned
    // postgame celebration. Keep those models on the live owned crowd.
    float remaining=emptyCrowdActive?0:fullnessReady?rendered->peak()*(100-(std::max)(activeLoss.load(),arrivalMissing.load()))*.01f:.75f;
    auto liveSupporters=std::atomic_load(&visitorPopulation);
    if(!emptyCrowdActive&&fullnessReady&&liveSupporters->owns(match,epoch))remaining=static_cast<float>(static_cast<double>(liveSupporters->home+liveSupporters->away)/rendered->seats);
    bool hadLayers=lineupAtmosphere.match==match&&lineupAtmosphere.epoch==epoch&&lineupAtmosphere.playContextActive();
    float hadMinimum=hadLayers?lineupAtmosphere.qualifiedMinimum:1.01f;
    auto previousHostileCheer=lineupAtmosphere.match==match&&lineupAtmosphere.epoch==epoch?lineupAtmosphere.hostileCheer:0;
    lineupAtmosphere.observe(*std::atomic_load(&coreSettings),p,match,epoch,now,enabled&&audioSetting&&mode==0&&fullnessReady&&!clockPlay,remaining,fullnessReady,&result,fullnessReady?rendered->seats:0);
    const auto& upsetSound=lineupAtmosphere.upsetSound;
    if(shownUpsetStage!=upsetSound.stage||shownUpsetSound!=upsetSound.qualified){
        std::ostringstream message;message<<attendance::upsetsound::name(upsetSound.stage);
        if(upsetSound.stage)message<<"; "<<(upsetSound.qualified?"sound route qualified":"waiting for enough fans")<<" at "<<100*remaining<<"% fullness (needs "<<100*upsetSound.minimum<<"%). Packed-house and volume limits unchanged.";
        else message<<"; separate sound route settled or unavailable.";
        log("upset-watch",message.str());
    }
    shownUpsetStage=upsetSound.stage;shownUpsetSound=upsetSound.qualified;shownUpsetMinimum=upsetSound.minimum;
    bool hostile=lineupAtmosphere.hostileOpening;
    shownHostileOpening=hostile;shownHostileSeries=lineupAtmosphere.hostileSeriesActive;
    if(hostile&&lineupAtmosphere.hostileCheer&&lineupAtmosphere.hostileCheer!=previousHostileCheer)log("opening",p.game.neutral?"Neutral-site first offensive play for this team: shared crowd cheers, roar and swell; crowd shake at "+std::to_string(static_cast<int>(std::atomic_load(&cameraSettings)->neutralPercent))+"% of normal venue strength; existing volume limits apply.":"Visitors have the opening series: hostile defensive roar and swell until their first down or punt (no added lineup cheer); stadium-size camera strength and existing volume limits apply.");
    shownBigAtmosphere=attendance::lineup::bigGame(p.game);shownPackedHouse=lineupAtmosphere.packedHouse;shownDrive=lineupAtmosphere.drive.heat;
    shownOpeningDrive=attendance::lineup::openingAdmission(lineupAtmosphere.openingDriveQualified,p.game,fullnessReady?rendered->seats:0);
    float reactionMinimum=(std::min)(hadMinimum,lineupAtmosphere.playContextActive()?lineupAtmosphere.qualifiedMinimum:1.01f);
    bool overtimeQualified=!clockPlay&&attendance::overtimeeffects::qualified(*std::atomic_load(&coreSettings),p,remaining,fullnessReady,fullnessReady?rendered->seats:0,enabled&&audioSetting&&mode==0);
    if(overtimeQualified)reactionMinimum=(std::min)(reactionMinimum,attendance::nightRequirement(p.game,attendance::packed::specialMinimum(rendered->seats)));
    bool reactionEligible=!clockPlay&&attendance::lineup::reactionEligible(p.game,remaining,fullnessReady,hadLayers||lineupAtmosphere.playContextActive(),reactionMinimum);
    reactionEligible=reactionEligible||overtimeQualified;
    result=contextReactions.observe(result,playTracker.start,p,playTracker.serial,match,epoch,now,remaining,fullnessReady?rendered->seats:0,
        reactionEligible&&std::atomic_load(&coreSettings)->contextReactions);
    uintptr_t liveRecord=0;int liveKind=-1,ballState=-1,ballSubstate=-1;float runnerSpot=std::numeric_limits<float>::quiet_NaN();bool ballHeld=false;float catchSpot=std::numeric_limits<float>::quiet_NaN();
    if(liveReactionBindings){
        auto tracker=at<A>(image+attendance::bindings::PlayStats);liveRecord=at<A>(tracker+0xcd8);liveKind=at<int>(liveRecord+0x64);
        int position=0;if(liveKind==1&&read(liveRecord+0x44,&position,4)&&position>=-50&&position<=50)catchSpot=static_cast<float>(position);
        auto control=at<A>(at<A>(image+attendance::bindings::BallControl)+0x10);ballState=at<int>(control+0x168);ballSubstate=at<int>(control+0x158);
        if(runnerPositionBinding&&at<A>(control)==image+0xb47bb10){
            A carrier=at<A>(control+0x1a0);float runnerPosition=0;
            if(read(control+0x48,&runnerPosition,4)&&std::isfinite(runnerPosition)&&std::fabs(runnerPosition)<=65){runnerSpot=runnerPosition;ballHeld=carrier!=0;}
        }
        if(at<A>(tracker+0xcd8)!=liveRecord||at<int>(liveRecord+0x64)!=liveKind)liveRecord=0;
    }
    if(hostileOpeningResult.observe(p,playTracker.start,result,match,epoch,playTracker.serial,now,hostile,reactionEligible&&enabled&&audioSetting&&mode==0&&lineupAtmosphere.hostileSeriesActive,liveKind,playTracker.sawPassReleased,lineupAtmosphere.openingDrive.awayOpeningSeries()))
        log("opening","Hostile opening series: positive home defensive result; largest generic cheer requested once. Existing live/native sounds keep playing; no additional shake or volume bonus.");
    auto liveSettings=std::atomic_load(&liveReactionSettings);
    bool basicReactionEligible=!clockPlay&&attendance::soundonly::eligible(*std::atomic_load(&coreSettings),p,remaining,fullnessReady,enabled&&audioSetting&&mode==0);
    bool liveAllowed=basicReactionEligible&&liveReactionBindings;
    if(!liveAllowed||!liveSettings->enabled){finishLiveAudit("atmosphere became unavailable or stopped qualifying");liveReactionUntil=0;}
    // Normalize the read-only world position against a verified pre-snap
    // yard line. Raw world Z can be mirrored and is not itself field yardage.
    bool kickPlay=selectedPlayType==22||selectedPlayType==23||selectedPlayType==24;
    float rawRunnerSpot=runnerSpot;
    auto returnResult=returnReactions.observe(p,match,epoch,now,liveAllowed&&liveSettings->enabled,selectedPlayType,runnerSpot,ballHeld);
    runnerSpot=runnerCoordinates.observe(p,match,epoch,now,liveAllowed&&liveSettings->enabled&&!kickPlay,runnerSpot);
    bool offensiveFourth=p.down==4&&attendance::lineup::criticalCamera(p,selectedPlayType);
    auto previousFlight=passFlight.flightId();
    bool passAllowed=liveAllowed&&liveSettings->enabled&&liveSettings->passFlight&&attendance::lineup::offensivePlay(selectedPlayType);
    passFlight.observe(p,match,epoch,now,passAllowed,liveRecord,liveKind,ballState,ballHeld);
    shownPassGate=!liveSettings->passFlight||!liveSettings->enabled?"disabled by pass-flight/during-play setting":
        !liveReactionBindings?"native ball binding unavailable":!reactionEligible?"waiting for qualifying atmosphere/fullness":
        !attendance::lineup::offensivePlay(selectedPlayType)?"waiting for a verified offensive play":!passAllowed?"live soundscape unavailable":
        passFlight.airborne()?"verified airborne pass; fast supporting swell requested":passFlight.weight(now)>0?"pass ended; swell settling":"waiting for a fresh airborne pass";
    if(passFlight.flightId()&&passFlight.flightId()!=previousFlight){
        ++shownPassFlights;
        log("pass-flight","Verified airborne pass: kind "+std::to_string(liveKind)+", ball "+std::to_string(ballState)+", no holder; fullness "+std::to_string(static_cast<int>(std::lround(100*remaining)))+"%. Fast native swell requested; no pre-catch cheer/shake or extra volume bonus.");
    }
    if(p.fumble)liveReactions.cancelRunBuildup(true);
    auto liveResult=liveReactions.observe(*liveSettings,p,match,epoch,now,liveAllowed&&!kickPlay,liveRecord,liveKind,ballState,catchSpot,runnerSpot,ballHeld,offensiveFourth,result.sequence==playTracker.serial?&result:nullptr);
    attendance::live::ForcedEvent forcedEvent;
    if(liveFumbleBinding&&liveAllowed&&liveSettings->enabled&&(p.game.phase==1||p.game.phase==3)){
        A event=at<A>(image+attendance::bindings::LiveFootballEvent);
        forcedEvent={event,at<int>(event+0x1c),event&&at<A>(event)==image+attendance::bindings::LiveFootballEventVtable&&at<A>(image+attendance::bindings::LiveFootballEventVtable+0x40)==image+attendance::bindings::LiveFootballEventType};
        forcedEvent.valid=forcedEvent.valid&&at<A>(image+attendance::bindings::LiveFootballEvent)==event&&at<int>(event+0x1c)==forcedEvent.type;
    }
    auto contactSample=contactFumbleBinding?attendance::fumblecontact::snapshot(image,read,fumbleContacts.seen()):attendance::fumblecontact::Sample{};
    bool confirmedFumble=fumbleBinding&&attendance::live::isFumbleState(ballState,ballSubstate);
    auto contactProof=fumbleContacts.observe(p,match,epoch,now,liveAllowed&&liveSettings->enabled&&fumbleBinding&&contactFumbleBinding,contactSample,ballState,ballHeld,confirmedFumble);
    if(contactProof.id)log("live-play",std::string("Confirmed contact-caused fumble: ")+(contactProof.owner==0?"home":"away")+" carrier; opposing contact, loose ball and fresh native fumble state; "+(contactProof.owner==1||p.game.neutral?"defensive impact eligible.":"away defense; no positive home impact."));
    int holderSide=ballHeld?attendance::showboating::carrierSide(image,read):-1;
    auto forcedLive=liveFumbles.observe(p,match,epoch,now,liveAllowed&&liveSettings->enabled&&(liveFumbleBinding||fumbleBinding),forcedEvent,liveKind,ballState,ballHeld,confirmedFumble,contactProof,holderSide);
    attendance::live::applyRecoveryProof(playTracker,result,liveFumbles.fumbler(),liveFumbles.recoveredBy());
    if(forcedLive.id){liveResult=forcedLive;liveReactions.cancelRunBuildup(true);playTracker.sawFumble=true;
        if(forcedLive.event==attendance::live::Event::ForcedFumble){
            playTracker.sawForcedFumble=true;
            attendance::live::noteForcedResult(result,playTracker.serial,forcedLive);
        }}
    auto showboatSample=ballHeld?attendance::showboating::snapshot(image,read):attendance::showboating::Sample{};
    auto showboat=showboating.observe(playTracker.start,p,match,epoch,playTracker.serial,now,
        reactionEligible&&liveAllowed&&liveSettings->enabled,ballHeld,showboatSample);
    if(showboat.id)log("showboat",std::string(showboat.side==0?"Home":"Away")+" carrier: fresh native PChat_Loco_Celebration animation event during live possession; "+(showboat.side==0?"slow buildup admitted; stronger earned reactions retain priority.":"native GEN_BOO requested if the immediate sound slot is free; no positive home shake."));
    hailMaryPass.observe(playTracker.serial,liveReactions.catchAirYards());
    result.hailMary=hailMaryPass.scored(result,playTracker.start);
    if(!liveResult.id)liveResult=returnResult;
    attendance::skills::Sample skillSample;
    bool skillAllowed=liveAllowed&&liveSettings->enabled&&std::atomic_load(&coreSettings)->skillMoves&&skillBindings;
    if(skillAllowed&&(p.game.phase==1||p.game.phase==3||p.game.phase==4||p.game.phase==5))skillSample=attendance::skills::View(image,read).snapshot(skillReactions.eventTimes(),hurdleCountBinding);
    auto skillResult=skillReactions.observe(p,match,epoch,now,skillAllowed,ballHeld,skillSample);
    if(!liveResult.id)liveResult=skillResult;
    auto knockoutSample=catchKnockoutBinding?attendance::knockout::snapshot(image,read):attendance::knockout::Sample{};
    auto knockoutResult=catchKnockouts.observe(p,match,epoch,now,liveAllowed&&liveSettings->enabled&&catchKnockoutBinding,knockoutSample,liveKind,ballHeld);
    if(knockoutResult.id&&!liveResult.id)liveResult=knockoutResult;
    // Sound-only accents (for example a provisional TFL cheer) do not prove
    // this play already received camera motion. Keep that ownership separate
    // so a later confirmed five-yard loss/forced fumble can request its impact.
    bool supplementalPrevious=attendance::live::accent(liveMotionEvent)||liveMotionEvent==attendance::live::Event::FourthConversion;
    auto defensiveContact=liveResult;
    if(skillResult.id&&skillResult.event==attendance::live::Event::HitStick)defensiveContact=skillResult;
    auto confirmedImpact=attendance::live::completedImpact(result,p.game,liveAllowed&&liveSettings->enabled,supplementalPrevious?0:liveMotionSequence,59,liveReactions.fourthConverted()||(liveMotionSequence==result.sequence&&liveMotionEvent==attendance::live::Event::FourthConversion));
    bool defensiveImpact=confirmedImpact.id!=0;
    if(defensiveImpact&&(!liveResult.id||attendance::live::accent(liveResult.event)))liveResult=confirmedImpact;
    auto breakupSample=passBreakupBinding?attendance::defense::snapshot(image,read,defensiveAccents.lookupAfter()):attendance::defense::Sample{};
    auto defensiveAccent=defensiveAccents.observe(p,playTracker.start,result,match,epoch,playTracker.serial,now,
        liveAllowed&&liveSettings->enabled,breakupSample,liveKind,ballHeld,liveResult,liveMotionSequence);
    if(defensiveAccent.id&&(!liveResult.id||liveResult.event==attendance::live::Event::HitStick))liveResult=defensiveAccent;
    liveReactions.defensiveHandoff(liveResult);
    auto situation=situationalReactions.observe(p,playTracker.start,result,match,epoch,playTracker.serial,now,
        liveAllowed&&liveSettings->enabled&&situationalReactionsEnabled&&(attendance::lineup::offensivePlay(selectedPlayType)||selectedPlayType==21),liveKind,ballState,catchSpot,runnerSpot,ballHeld,selectedPlayType);
    if(showboat.id&&showboat.side==1&&!p.game.neutral&&!liveResult.id&&!situation.id)situation={showboat.id,103,attendance::situational::Event::AwayShowboat};
    if(liveResult.id||situation.id)awaySoundReactions.cover(playTracker.serial,
        result.accepted&&result.completed&&(result.homePoints>0||result.awayPoints>0));
    attendance::awaysound::Samples awaySamples;awaySamples.selectedType=selectedPlayType;awaySamples.worldZ=rawRunnerSpot;
    awaySamples.forced=forcedEvent;awaySamples.fumble=confirmedFumble;awaySamples.contact=contactProof;awaySamples.holderSide=holderSide;
    awaySamples.knockout=knockoutSample;awaySamples.breakup=breakupSample;awaySamples.showboat=showboatSample;
    auto awaySound=awaySoundReactions.observe(*liveSettings,p,match,epoch,playTracker.serial,now,
        liveAllowed&&liveSettings->enabled&&situationalReactionsEnabled,liveRecord,liveKind,ballState,catchSpot,runnerSpot,ballHeld,skillAllowed,skillSample,awaySamples);
    auto basicResult=soundOnlyReactions.observe(result,playTracker.start,p,match,epoch,basicReactionEligible,reactionEligible,
        liveSourceSequence==result.sequence&&liveReactionType>0&&!attendance::population::negative(liveReactionType),situationalReactionsEnabled);
    if(liveResult.id){
        if(reactionEligible&&attendance::live::cameraReaction(liveResult.event)&&!attendance::defensivemotion::routed(liveResult.event)){liveMotionSequence=playTracker.serial;liveMotionEvent=liveResult.event;}
        liveReactionMilestone=liveReactions.fourthConverted()&&(liveResult.event==attendance::live::Event::RunAfterCatch||liveResult.event==attendance::live::Event::LongRun||liveResult.event==attendance::live::Event::CompletedPass);
        liveReactionMajor=liveResult.yards>=40||liveResult.event==attendance::live::Event::Interception||liveResult.event==attendance::live::Event::FumbleRecovery||
            liveResult.event==attendance::live::Event::ForcedFumble||liveResult.event==attendance::live::Event::GoalLineStop||liveResult.event==attendance::live::Event::CriticalStop||
            liveResult.event==attendance::live::Event::BlockedFieldGoal||result.contextEruption||result.contextClincher;
        finishLiveAudit("superseded by the next verified reaction");
        ++liveReactionId;liveReactionUntil=now+500;liveSourceSequence=playTracker.serial;
        // The confirmed critical stop gets the full two-family result burst.
        // Use this event for its immediate impact without replacing that burst
        // with the smaller one-family early live-play accent.
        if(reactionEligible&&defensiveImpact&&result.highLeverageStop)liveReactionUntil=0;
        liveReactionPost=p.game.phase!=3;
        liveReactionType=attendance::palette::liveType(liveResult,playTracker.start,p.game,playTracker.serial,epoch);
        if(!liveReactionType)liveReactionUntil=0;
        if(liveResult.event==attendance::live::Event::Interception)++shownLivePicks;
        else if(liveResult.event==attendance::live::Event::LongCatch||liveResult.event==attendance::live::Event::CompletedPass||liveResult.event==attendance::live::Event::RunAfterCatch)++shownLiveCatches;
        if(liveReactionUntil>now){liveAuditId=liveReactionId;liveAuditUntil=liveReactionUntil;}
    }else if(situation.id){
        liveReactionMajor=liveReactionMilestone=false;
        finishLiveAudit("superseded by the next verified reaction");
        ++liveReactionId;liveReactionUntil=now+500;liveSourceSequence=playTracker.serial;liveReactionType=attendance::palette::situationType(situation,playTracker.start,p.game,playTracker.serial,epoch,&result,playTracker.sawPassReleased);liveReactionPost=p.game.phase!=3;
        liveAuditId=liveReactionId;liveAuditUntil=liveReactionUntil;
        log("live-play","Live reaction #"+std::to_string(liveReactionId)+": "+attendance::situational::name(situation.event)+"; native crowd reaction requested, no additional shake or volume boost.");
    }
    if(basicResult.sequence&&((!liveResult.id&&!situation.id)||result.safety||result.homePoints>=6||result.awayPoints>=6)){
        liveReactionMajor=liveReactionMilestone=false;finishLiveAudit("superseded by a sound-only play result");
        ++liveReactionId;liveReactionUntil=now+500;liveSourceSequence=playTracker.serial;liveReactionType=attendance::palette::resultType(result,playTracker.start,p.game,playTracker.sawPassReleased,playTracker.serial,epoch,attendance::population::negative(basicResult.type));liveReactionPost=true;
        liveAuditId=liveReactionId;liveAuditUntil=liveReactionUntil;
        log("live-play","Sound-only result #"+std::to_string(liveReactionId)+": "+attendance::energy::name(result.event)+"; native crowd reaction requested below layered-atmosphere gate; no added shake, roar, follow-up cheer or volume bonus.");
    }
    // Negative equivalents reuse verified live/result proofs. They own only
    // this one-shot sound slot, never the live outcome, camera or energy model.
    if(awaySound.type&&!situation.id&&!basicResult.sequence){
        liveReactionMajor=liveReactionMilestone=false;finishLiveAudit("superseded by an away-team sound-only counterpart");
        ++liveReactionId;liveReactionUntil=now+500;liveSourceSequence=playTracker.serial;liveReactionType=awaySound.type;liveReactionPost=p.game.phase!=3;
        liveAuditId=liveReactionId;liveAuditUntil=liveReactionUntil;
        log("live-play","Away-team sound-only counterpart #"+std::to_string(liveReactionId)+": family "+std::string(attendance::catalog::Families[liveReactionType/4].name)+"; no positive roar or volume boost; optional visitor shake is handled separately.");
    }
    int extraType=0;
    if(skillResult.id&&(skillResult.side==0||p.game.neutral)&&liveResult.id&&skillResult.event!=liveResult.event)
      extraType=attendance::palette::liveType(skillResult,playTracker.start,p.game,playTracker.serial,epoch);
    if(awaySoundReactions.extraType())extraType=awaySoundReactions.extraType();
    if(extraType){extraReactionType=extraType;extraReactionId=(uint64_t{1}<<60)|++extraReactionSerial;extraReactionUntil=now+500;extraReactionSequence=playTracker.serial;}
    skillSwell.observe(p,match,epoch,now,liveAllowed&&liveSettings->enabled,liveResult,liveReactionId);
    double time=now/1000.0;
    auto audio=std::atomic_load(&coreSettings);
    uint64_t previousCelebration=coreAtmosphere.celebration;
    reactionDistance.observe(p,match,epoch,playTracker.serial,now,liveAllowed&&liveSettings->enabled,liveResult,rawRunnerSpot,liveReactions.gain(),liveReactions.ranAfterCatch(),runnerCoordinates.axis()*p.attackDirection,ballHeld,liveReactions.runBuildup(),returnReactions.yards(),*liveSettings,reactionDistanceMaximum.load());
    unsigned scoreExtra=reactionDistance.forResult(result,playTracker.start,reactionDistanceMaximum.load());
    shownDistanceExtra=result.completed?scoreExtra:reactionDistance.extension();shownReactionDistance=reactionDistance.yards();
    if(reactionDistance.ownsDuration()&&(p.game.phase==4||p.game.phase==5))lineupAtmosphere.handOffResult();
    auto previousResultCheer=coreAtmosphere.resultCheer;
    if(fullnessReady)coreAtmosphere.react(*audio,result,time,p.game,hadLayers||lineupAtmosphere.playContextActive(),reactionEligible,reactionMinimum,scoreExtra,reactionDistance.ownsDuration());
    if(coreAtmosphere.resultCheer!=previousResultCheer)coreAtmosphere.resultType=attendance::palette::resultType(result,playTracker.start,p.game,playTracker.sawPassReleased,playTracker.serial,epoch);
    int turnoverExit=mode==0?lateTurnoverExits.observe(result,playTracker.start,p,lateTurnoverMaximum.load(),shownComeback.load()):0;
    int lateExit=(std::max)(result.lateDeparture?c->lateScoreDepartures:0,turnoverExit);
    if(lateExit){
        pendingLateExits=(std::min)(100,pendingLateExits+lateExit);
        if(turnoverExit)log("late-turnover","Costly home turnover: "+std::to_string(lateExit)+" percentage point(s) queued for the next hidden break; clock, timeout and recovery outlook checked. Combined with any same-play score exit, not added twice.");
    }
    if(result.lateDeparture){
        log("late-score","Late opponent score: home trails by "+std::to_string(p.game.awayScore-p.game.homeScore)+", with "+std::to_string(p.homeTimeouts)+(p.homeTimeouts==1?" timeout":" timeouts")+"; extra departures queued for the break before the next play.");
    }
    float previousOpening=coreAtmosphere.openingWeight;bool wasPostgameHeld=coreAtmosphere.postgameHeld;
    coreAtmosphere.update(*audio,p,time,remaining,remaining,lineupAtmosphere.momentWeight(),reactionEligible,reactionMinimum);
    if(!wasPostgameHeld&&coreAtmosphere.postgameHeld)log("roar","Winning eruption carried into postgame; roar held until the stats/highlights menu.");
    if(previousCelebration!=coreAtmosphere.celebration){
        celebrationReactionType=91; // Largest generic cheer for every admitted eruption.
        int seconds=static_cast<int>(p.game.seconds);char stamp[32]{};
        if(p.game.phase==7)sprintf_s(stamp,"Final");else sprintf_s(stamp,"Q%d %d:%02d",p.game.quarter,seconds/60,seconds%60);
        log("eruption",std::string(attendance::energy::name(coreAtmosphere.event))+" ("+(coreAtmosphere.winner==1?"away":"home")+") at "+stamp+"; "+std::to_string(static_cast<int>(coreAtmosphere.eruptionSeconds))+"-second celebration, sustained roar and native cheer; crowd bus gain stays within the occupied-seat mix and configured total cap; no extra eruption gain.");
    }
    if(previousOpening<=0&&coreAtmosphere.openingWeight>0)log("opening","Big-game opening atmosphere started.");
    if(previousOpening>0&&coreAtmosphere.openingWeight<=0)log("opening","Opening atmosphere settled into the normal crowd mix.");
    shownAmbienceLift=coreAtmosphere.beds;shownOpeningWeight=coreAtmosphere.openingWeight;shownAudioLimit=coreAtmosphere.limit;
    shownDecisiveUntil=coreAtmosphere.eruptionLease(now);
    shownHighUntil=coreAtmosphere.highLeverage?now+750:0;
    shownEruption=coreAtmosphere.erupting;
    shownPostgameCarry=!coreAtmosphere.postgameMenu&&(coreAtmosphere.postgameHeld||(coreAtmosphere.postgameAt>=0&&time-coreAtmosphere.postgameAt<10));

    bool live=p.game.phase==1||p.game.phase==3||p.game.phase==4||p.game.phase==5||p.game.phase==6||p.game.phase==7;
    bool active=mode==0&&fullnessReady&&remaining>0&&audio->enabled;
    auto wanted=coreAtmosphere.intent(*audio,p,match,audioEpoch.load(),now,active&&enabled&&audioSetting);
    bool openingKickoffProof=audio->opening&&attendance::lineup::kickoffMatchup(p.game)&&playTracker.openingReliable&&!playTracker.openingKickUsed&&p.game.quarter==1&&
        p.game.homeScore==0&&p.game.awayScore==0&&p.game.seconds>=p.game.quarterSeconds-1;
    bool kickoffStarted=openingKickoffAtmosphere.observe(p,match,epoch,now,active&&enabled&&audioSetting&&audio->opening&&audio->lineups&&reactionEligible&&!p.simulated,openingKickoffProof,selectedPlayType,remaining);
    if(kickoffStarted)log("opening-kickoff","Opening kickoff lineup: immediate cheer requested; roar and supporting swell follow the kick, then settle for four seconds after the whistle. Return/eruption reactions take priority.");
    if(!openingKickoffAtmosphere.replacesLineup())wanted=lineupAtmosphere.merge(wanted,*audio,p,now,remaining);
    wanted=passFlight.merge(wanted,now,remaining);
    auto gainBuildup=liveReactions.runBuildup();
    auto kickBuildup=returnReactions.buildup();if(kickBuildup.id)gainBuildup=kickBuildup;
    auto tauntBuildup=showboating.buildup(now);
    if(tauntBuildup.id){
        auto existingCamera=std::atomic_load(&cameraCue);
        bool earnedLive=existingCamera->kind==attendance::motion::Kind::LivePlay&&!existingCamera->runBuildup;
        if(gainBuildup.id)gainBuildup.progress=(std::max)(gainBuildup.progress,tauntBuildup.progress);
        else if(!earnedLive)gainBuildup=tauntBuildup;
    }
    returnAtmosphere.observe(p,match,epoch,now,liveAllowed&&liveSettings->enabled,liveResult,ballHeld,forcedLive.id&&forcedLive.event==attendance::live::Event::ForcedFumble,std::atomic_load(&cameraSettings)->reactionFadeMs,gainBuildup,reactionDistance.extension(),reactionDistance.ownsDuration());
    if(reactionEligible)wanted=returnAtmosphere.merge(wanted,now,remaining);
    if(wanted.reason==attendance::sustained::Reason::Result)wanted.reactionId=playTracker.serial;
    wanted=openingKickoffAtmosphere.merge(wanted,now);
    auto presentation=presentationAtmosphere.observe(*audio,p.game,match,epoch,now,active&&enabled&&audioSetting,
        fullnessReady,fullnessReady?audibleFullness(audioWanted()):remaining,fullnessReady?rendered->seats:0,coreAtmosphere.postgameMenu);
    if(presentation.active&&!wanted.celebration&&wanted.reason==attendance::sustained::Reason::None)wanted=presentation;
    wanted=attendance::overtimeeffects::fill(wanted,*audio,p,remaining,fullnessReady,fullnessReady?rendered->seats:0,active&&enabled&&audioSetting&&!clockPlay);
    if(wanted.active&&(p.game.phase==3||p.game.phase==4||p.game.phase==5||((p.game.phase==1||p.game.phase==7)&&liveReactionPost))&&basicReactionEligible&&liveReactionId&&now<liveReactionUntil){wanted.liveCheer=liveReactionId;wanted.liveCheerUntil=liveReactionUntil;wanted.liveType=liveReactionType;if(reactionEligible&&!attendance::population::negative(liveReactionType))wanted.intensity=(std::max)(wanted.intensity,950*std::sqrt(remaining));}
    // Keep the result roar/eruption, but do not add a second ordinary cheer
    // when this play's early accent was already delivered (or native-equivalent).
    // Distinct live and official-result sounds may overlap; their own IDs dedupe repeats.
    shownLineup=wanted.lineup!=0;
    auto stopMotion=defensiveMotion.observe(playTracker.start,p,result,
        defensiveContact.id&&defensiveContact.event==attendance::live::Event::HitStick?defensiveContact:liveResult,match,epoch,playTracker.serial,now,
        reactionEligible&&liveAllowed&&liveSettings->enabled,supplementalPrevious?0:liveMotionSequence);
    auto motionResult=stopMotion.id?stopMotion:liveResult;
    uint64_t motionId=attendance::defensivemotion::cameraId(liveResult,liveReactionId,stopMotion);
    if(stopMotion.id){liveMotionSequence=playTracker.serial;liveMotionEvent=stopMotion.event;
        log("defensive-motion","Play #"+std::to_string(playTracker.serial)+": "+attendance::live::name(stopMotion.event)+"; shared confirmed loss/stop route; one camera request.");}
    attendance::motion::Input motion;motion.match=match;motion.epoch=audioEpoch.load();motion.now=now;motion.phase=p.game.phase;motion.neutral=p.game.neutral;motion.venueScale=attendance::motion::occupiedScale(fullnessReady?rendered->seats:0,remaining);motion.overtime=attendance::overtime(p.game)&&audio->overtime;
    auto audioContext=audioWanted();
    // Camera cuts can briefly stop crowd-entity updates. The last verified
    // slider state is still authoritative for this owned match; cue freshness,
    // world ownership, pause/replay and the new-snap boundary remain separate.
    bool lostHomeMotion=liveReactions.homePossessionLost()||(!p.game.neutral&&(returnAtmosphere.lostPossession()||liveFumbles.homeBallLost()));
    motion.allowed=active&&reactionEligible&&!clockPlay&&!lostHomeMotion&&remaining>=attendance::core::ReactionMinimumFullness&&enabled&&audioSetting&&audioContext.active&&audio->cameraShake&&nativeUnpaused()&&audioAudible&&!p.simulated&&swellAvailable&&(sustainedAudio.healthy()||supportingSwell.healthy());
    bool openingKickoff=openingKickoffProof;
    auto cameraAdmission=attendance::lineup::cameraAdmission(*audio,p,remaining,fullnessReady,fullnessReady?rendered->seats:0,selectedPlayType,openingKickoff,lineupAtmosphere.openingDriveQualified);
    std::shared_ptr<const attendance::nativecamera::Admission> nativeAdmission=std::make_shared<attendance::nativecamera::Admission>(
        attendance::nativecamera::decide(*audio,p,match,epoch,now,remaining,fullnessReady,fullnessReady?rendered->seats:0,
            selectedPlayType,openingKickoff,lineupAtmosphere.openingDriveQualified,reactionEligible));
    std::atomic_store(&nativeCameraAdmission,nativeAdmission);
    motion.lineup=(p.game.phase==1||p.game.phase==5)&&lineupAtmosphere.shakeTier>0&&(p.game.phase==5||lineupAtmosphere.layeredActive())&&cameraAdmission.allowed;
    motion.lineupId=lineupAtmosphere.cameraLineupId();motion.critical=cameraAdmission.critical;
    motion.homeCritical=cameraAdmission.critical&&!p.game.neutral&&p.offense==0;
    motion.lineupStrengthScale=motion.homeCritical?1.f:cameraAdmission.strengthScale;
    motion.lineupScale=lineupAtmosphere.cameraChoice();motion.lineupCheer=lineupAtmosphere.cheerMoment;
    motion.hostileOpening=hostile?std::atomic_load(&cameraSettings)->hostileOpeningMaximum:0;
    motion.openingKickoff=openingKickoff&&(selectedPlayType==23||selectedPlayType==24);
    if(motion.openingKickoff)motion.hostileOpening=std::atomic_load(&cameraSettings)->hostileOpeningMaximum;
    motion.venueEligible=cameraAdmission.allowed;
    motion.duringAllowed=reactionEligible&&liveAllowed&&liveSettings->enabled;motion.duringPlay=reactionEligible?motionId:0;motion.duringImpact=stopMotion.id||(motionId&&(p.game.phase!=3||motionResult.event==attendance::live::Event::ForcedFumble));
    if(motionResult.id){
        bool accent=attendance::live::accent(motionResult.event);
        motion.duringImpact=motion.duringImpact||accent||motionResult.event==attendance::live::Event::FourthConversion;
        motion.supplementalImpact=accent;
        motion.profile=attendance::defense::shakeProfile(motionResult.event);
        if(attendance::live::softSkill(motionResult.event))motion.softAccent=true;
        attendance::defense::motion(motionResult,motion);
    }
    // Context chooses a point inside each event's range, never an additional
    // multiplier on top of it. Latch that choice with the cue at admission.
    motion.leverage=attendance::motion::importance((attendance::contextreaction::strength(playTracker.start,p.game,audio->contextReactions&&
        attendance::contextreaction::premium(playTracker.start.game,remaining,fullnessReady?rendered->seats:0))-1.f)/.3f);
    motion.cancelLive=returnAtmosphere.lostPossession()&&!liveResult.id;
    motion.duringCatch=motionId&&(motionResult.event==attendance::live::Event::LongCatch||motionResult.event==attendance::live::Event::CompletedPass);
    motion.distanceExtraMs=(std::max)(scoreExtra,reactionDistance.extension());
    int blockLead=result.side==1?playTracker.start.game.awayScore-playTracker.start.game.homeScore:playTracker.start.game.homeScore-playTracker.start.game.awayScore;
    if(reactionEligible&&result.accepted&&result.completed&&result.blockedFieldGoal&&(result.side==0||p.game.neutral)){
        motion.blockedFieldGoal=result.sequence;
        motion.gameSavingBlock=p.game.quarter>=4&&blockLead>=0&&blockLead<=3;
    }
    auto runBuildup=gainBuildup;motion.runBuildup=reactionEligible?runBuildup.id:0;motion.runProgress=runBuildup.progress;
    motion.smoothRunBuildup=motion.runBuildup&&!kickBuildup.id&&liveReactions.runBuildup().id==motion.runBuildup;
    if(tauntBuildup.id&&!liveReactions.runBuildup().id&&!kickBuildup.id)motion.buildupProfile=attendance::motion::Profile::Showboat;
    int scored=result.side==1?result.awayPoints:result.homePoints;
    if(reactionEligible&&result.accepted&&result.completed&&(result.side==0||p.game.neutral)&&scored>=6&&scored<=8)motion.touchdown=result.sequence;
    if(reactionEligible&&result.accepted&&result.completed&&result.safety&&(result.side==0||p.game.neutral))motion.safety=result.sequence;
    if(reactionEligible&&audio->contextReactions&&result.contextClincher)motion.clincher=result.sequence;
    motion.eruptionAlive=coreAtmosphere.erupting||coreAtmosphere.postgameHeld;
    if(previousCelebration!=coreAtmosphere.celebration)motion.eruption=coreAtmosphere.celebration;
    if(lostHomeMotion&&std::atomic_load(&cameraCue)->kind!=attendance::motion::Kind::None)log("live-play","Home possession lost: previous home crowd motion cleared immediately; no positive turnover shake.");
    auto supporters=std::atomic_load(&visitorPopulation);
    bool mixedSupporters=!p.game.neutral&&supporters->owns(match,epoch);
    float unmixedVenueScale=motion.venueScale;
    if(mixedSupporters)motion.venueScale*=attendance::visitors::reactionScale(supporters->home,supporters->home+supporters->away);
    std::shared_ptr<const attendance::motion::Cue> nextCamera=std::make_shared<attendance::motion::Cue>(cameraMotion.observe(*std::atomic_load(&cameraSettings),motion));
    wanted=attendance::pairing::merge(wanted,*nextCamera,now,remaining,playTracker.serial,(presentationCamera.playing(*nextCamera,now)||presentationCamera.requested(*nextCamera,now)),!returnAtmosphere.lostPossession());
    wanted=skillSwell.merge(wanted,now,remaining,!returnAtmosphere.lostPossession());
    wanted=scoringSoundHandoff.merge(wanted,p.game.phase,now,remaining,!returnAtmosphere.lostPossession());
    if(basicReactionEligible&&extraReactionSequence==playTracker.serial&&now<extraReactionUntil){wanted.extraCheer=extraReactionId;wanted.extraCheerUntil=extraReactionUntil;wanted.extraType=extraReactionType;}
    wanted.playSequence=playTracker.serial;wanted.staggered=audio->staggeredCheers;wanted.majorLive=liveReactionMajor;wanted.overlapMilestone=audio->staggeredCheers&&liveReactionMilestone;
    auto welcome=openingWelcome.observe(*audio,p,match,epoch,now,active&&enabled&&audioSetting&&!clockPlay,fullnessReady,remaining,&result);
    if(basicReactionEligible&&!reactionEligible)wanted=attendance::soundonly::restrain(wanted,remaining);
    wanted=attendance::openingwelcome::Model::merge(wanted,welcome);
    if(wanted.lineupCheer)wanted.lineupType=attendance::palette::lineup(playTracker.serial,wanted.lineupCheer,epoch);
    wanted.celebrationType=celebrationReactionType;
    wanted=hostileOpeningResult.merge(wanted,p,now);
    if(fullnessReady)wanted=attendance::population::layers(wanted,rendered->seats,remaining);
    if(active&&enabled&&audioSetting&&audio->lineups&&!clockPlay&&!p.simulated&&fullnessReady)
        wanted=attendance::background::merge(wanted,p.game,rendered->seats,remaining);
    auto carried=finalReactionCarry.observe(p.game,match,epoch,playTracker.serial,now,
        active&&enabled&&audioSetting&&!p.simulated&&!lostHomeMotion&&!coreAtmosphere.postgameMenu,
        audio->cameraShake&&audioAudible&&swellAvailable&&(sustainedAudio.healthy()||supportingSwell.healthy()),
        wanted,*nextCamera,std::atomic_load(&cameraSettings)->reactionFadeMs);
    wanted=carried.sound;nextCamera=std::make_shared<attendance::motion::Cue>(carried.camera);
    // Visitor success uses its configured percentage with negative-audio proof.
    // It never borrows the positive roar companion or rewrites home admission.
    if(awayHeardSequence!=playTracker.serial){awayHeardSequence=playTracker.serial;awayHeardId=awayHeardAt=awayExtraHeardId=0;}
    if(liveReactionId!=awayHeardId&&liveSourceSequence==playTracker.serial&&attendance::population::negative(liveReactionType)&&positiveCheer.liveDelivered(liveReactionId)){awayHeardId=liveReactionId;awayHeardAt=now;}
    auto awayMotion=motion;awayMotion.venueScale=unmixedVenueScale;awayMotion.lineup=awayMotion.critical=awayMotion.venueEligible=false;awayMotion.lineupId=0;
    awayMotion.touchdown=awayMotion.eruption=awayMotion.safety=awayMotion.clincher=awayMotion.blockedFieldGoal=0;
    awayMotion.openingKickoff=false;awayMotion.hostileOpening=0;
    auto visitorCelebration=awaySoundReactions.celebration(*audio,p,now,remaining,reactionEligible,reactionMinimum);
    awayMotion.eruption=visitorCelebration;awayMotion.eruptionAlive=visitorCelebration!=0;
    auto awayConfig=std::atomic_load(&cameraSettings);
    awayMotion.allowed=attendance::motion::awayScale(*awayConfig)>0&&liveAllowed&&liveSettings->enabled&&!p.game.neutral&&active&&reactionEligible&&!clockPlay&&enabled&&audioSetting&&audioContext.active&&audio->cameraShake&&nativeUnpaused()&&audioAudible&&!p.simulated&&situationalReactionsEnabled;
    float visitorStrength=mixedSupporters?attendance::visitors::reactionScale(supporters->away,supporters->home+supporters->away):.2f;
    awayMotion.venueScale*=attendance::motion::awayScale(*awayConfig)*visitorStrength;
    awayMotion.cancelLive=p.game.phase==3&&(holderSide>=0?holderSide:p.offense)==0&&awayLastCue.kind==attendance::motion::Kind::LivePlay&&!awayLastCue.impactOnly;
    auto visitorEvent=awaySoundReactions.motion();auto visitorBuildup=awaySoundReactions.buildup();auto visitorResult=awaySoundReactions.result();
    awayMotion.duringPlay=visitorEvent.id;awayMotion.duringImpact=visitorEvent.id&&(p.game.phase!=3||attendance::live::accent(visitorEvent.event));
    awayMotion.supplementalImpact=attendance::live::accent(visitorEvent.event);awayMotion.duringCatch=visitorEvent.event==attendance::live::Event::LongCatch||visitorEvent.event==attendance::live::Event::CompletedPass;
    awayMotion.profile=attendance::defense::shakeProfile(visitorEvent.event);awayMotion.softAccent=attendance::live::softSkill(visitorEvent.event);awayMotion.forcedImpact=false;attendance::defense::motion(visitorEvent,awayMotion);
    awayMotion.runBuildup=visitorBuildup.id;awayMotion.runProgress=visitorBuildup.progress;awayMotion.smoothRunBuildup=visitorBuildup.id&&!awaySoundReactions.taunt();awayMotion.buildupProfile=awaySoundReactions.taunt()?attendance::motion::Profile::Showboat:attendance::motion::Profile::Gain;
    if(visitorResult.accepted&&visitorResult.completed&&visitorResult.side==1){
     if(visitorResult.awayPoints>=6)awayMotion.touchdown=visitorResult.sequence;
     if(visitorResult.safety)awayMotion.safety=visitorResult.sequence;
     if(visitorResult.blockedFieldGoal)awayMotion.blockedFieldGoal=visitorResult.sequence;
    }
    if(extraReactionId&&extraReactionId!=awayExtraHeardId&&attendance::population::negative(extraReactionType)&&extraCheer.liveDelivered(extraReactionId)){awayExtraHeardId=extraReactionId;awayHeardAt=now;}
    if(mixedSupporters){wanted=attendance::visitors::audience(wanted,*supporters,rendered->seats,remaining,now);
        auto visitingLayer=awaySoundReactions.positiveIntent(*audio,p,match,epoch,now,remaining,reactionEligible,reactionMinimum);
        wanted=attendance::visitors::layer(wanted,visitingLayer,*supporters,rendered->seats,remaining);
        if(wanted.opponentCheer&&wanted.opponentPositive&&opponentCheer.liveDelivered(wanted.opponentCheer))awayHeardAt=now;}
    auto visitorCue=awayCameraMotion.observe(*awayConfig,awayMotion);visitorCue.visitor=true;visitorCue.soundAt=awayHeardAt;visitorCue.playSequence=playTracker.serial;
    if(visitorCue.kind!=attendance::motion::Kind::None&&
       (visitorCue.kind!=awayLastCue.kind||visitorCue.began!=awayLastCue.began||visitorCue.profile!=awayLastCue.profile||(!awayLastCue.soundAt&&visitorCue.soundAt))){
        std::ostringstream detail;detail<<"Play #"<<playTracker.serial<<": "<<attendance::motion::name(visitorCue.kind)
            <<"; visiting-supporter scale "<<visitorStrength<<"; envelope "<<visitorCue.percent<<"%, opening impact "<<visitorCue.impact
            <<"% before view/camera scaling; reaction voice proof observed this play "<<(visitorCue.soundAt?"yes":"not yet")
            <<"; actual applied output is counted in technical status.";log("away-motion",detail.str());
    }
    awayLastCue=visitorCue;
    if(attendance::motion::strength(visitorCue,now)>attendance::motion::strength(*nextCamera,now))nextCamera=std::make_shared<attendance::motion::Cue>(visitorCue);
    shownPostgameCarry=shownPostgameCarry||(p.game.phase==7&&carried.carrying);
    std::atomic_store(&cameraCue,nextCamera);
    std::shared_ptr<const attendance::sustained::Intent> next=std::make_shared<attendance::sustained::Intent>(wanted);std::atomic_store(&sustainedIntent,next);
    if(liveResult.id)log("live-play","Live reaction #"+std::to_string(liveReactionId)+": "+attendance::live::name(liveResult.event)+"; "+std::to_string(static_cast<int>(liveResult.yards))+" yards; Q"+std::to_string(p.game.quarter)+" "+std::to_string(static_cast<int>(p.game.seconds))+"s; fullness "+std::to_string(static_cast<int>(std::lround(100*remaining)))+"%; "+(!reactionEligible?"sound-only below layered-atmosphere gate; ":"")+"camera cue "+attendance::motion::name(nextCamera->kind)+(nextCamera->runBuildup?" (building)":"")+(liveResult.event==attendance::live::Event::ForcedFumble?"; sharp forced-fumble impact (35 ms attack / 50 ms handoff); immediate cheer requested.":liveResult.event==attendance::live::Event::FumbleRecovery?"; verified live defensive return; interception-style continuous shake when layered atmosphere qualifies; no pickup impact.":liveResult.event==attendance::live::Event::BrokenTackle?"; broken tackle is sound-only; soft swell requested, no new shake.":attendance::live::cheerOnlySkill(liveResult.event)?"; skill move is cheer-only; no new shake; immediate cheer requested.":defensiveImpact&&result.highLeverageStop?"; full result cheer requested.":liveReactionType==0?"; soft skill swell requested.":liveReactionType/4==2?"; native big-hit crowd reaction requested.":"; immediate cheer requested.")+(liveResult.event==attendance::live::Event::Sack?" Native sack label controls audio/replay only; shared confirmed loss/stop route controls motion.":""));
    if(result.accepted&&result.completed&&result.sequence&&(result.sequence!=playAuditSequence||result.homePoints>0||result.awayPoints>0)){
        playAuditSequence=result.sequence;
        const char* admitted=clockPlay?"kneel/clock-stop play; added reactions suppressed":!audioSetting||!audio->enabled||!audio->lineups?"soundscape/layers disabled":!fullnessReady?"seating measurement unavailable":
            reactionEligible?"eligible":basicReactionEligible?"sound-only; layered atmosphere not qualified":!(hadLayers||lineupAtmosphere.playContextActive())?"no qualifying atmosphere on this play":"fullness or competitiveness gate not met";
        if((playTracker.start.down==3||playTracker.start.down==4)&&!result.homePoints&&!result.awayPoints&&!result.firstDown&&!result.takeaway)
            log("defensive-proof","Play #"+std::to_string(result.sequence)+": knockout summary "+(knockoutSample.valid?"valid":"unavailable")+", current flag "+(knockoutSample.knockout?"on":"off")+", fresh knockout "+(catchKnockouts.proof()?"latched":"not observed")+"; defender deflection "+(defensiveAccents.deflectionProof()?"latched":defensiveAccents.pendingSide()?"waiting for defender ownership":"not observed")+"; result kind "+std::to_string(liveKind)+"; ball holder "+(ballHeld?"present":"none")+"; selected added reaction "+attendance::live::name(liveResult.event)+".");
        const char* sackReason=attendance::energy::clockManagement(playTracker.start)?"suppressed for kneel/clock-stop play":result.sack?"confirmed":playTracker.sackAtSnap&&!playTracker.sackRearmed(p)?"suppressed: native sack predicate still belongs to the previous play":playTracker.sawPassReleased&&playTracker.sawSack?"suppressed: verified forward pass released this snap":!playTracker.sawSack?(p.sack?"raw flag present, but no fresh sack proof for this snap":"native sack proof not observed"):
            playTracker.start.offense==0&&!p.game.neutral?"away defense; home crowd does not celebrate":
            playTracker.sawPick?"interception result takes precedence":p.offense!=playTracker.start.offense?"possession changed; sack ownership not established":
            attendance::energy::gainedYards(playTracker.start,p)>0.01f?"contradictory positive yardage":"not selected; check scoring and event settings";
        std::string yards=std::isfinite(result.yards)&&result.yards>-1000?std::to_string(static_cast<int>(std::lround(result.yards)))+" yards":"yardage unavailable";
        if(std::isfinite(result.touchdownYards)&&result.touchdownYards>=0)yards+="; touchdown distance "+std::to_string(static_cast<int>(std::lround(result.touchdownYards)))+" yards (pre-snap line to goal)"+(motion.touchdownScale>1?"; modest long-touchdown accent":"");
        log("play-summary","Play #"+std::to_string(result.sequence)+": "+attendance::energy::name(result.event)+"; "+yards+"; home/away points "+std::to_string(result.homePoints)+"/"+std::to_string(result.awayPoints)+"; added reactions "+admitted+(result.safety?"; safety confirmed":"")+(result.fiveYardLoss?"; 5+ yard defensive loss confirmed":"")+"; snap down "+std::to_string(playTracker.start.down)+"; snap play type "+std::to_string(playTracker.start.selectedType)+"; snap offense "+(playTracker.start.offense==0?"home":playTracker.start.offense==1?"away":"unknown")+"; sack "+sackReason+"; result phase "+std::to_string(p.game.phase)+(result.takeaway?"; opponent timeouts "+std::to_string(result.opponentTimeouts)+"; seals regulation "+(attendance::energy::sealsRegulationTakeaway(result,p.game)?"yes":"no"):"")+"; native excitement "+std::to_string(static_cast<int>(std::lround(crowdIntensity.native())))+" -> "+std::to_string(static_cast<int>(std::lround(crowdIntensity.output())))+" (latest audio callback); measured reaction distance "+std::to_string(static_cast<int>(std::lround(reactionDistance.yards())))+" yards; extra settling "+std::to_string(scoreExtra)+" ms; camera cue "+attendance::motion::name(nextCamera->kind)+".");
    }
    if((p.game.phase==4||p.game.phase==5)&&playTracker.played&&playTracker.serial&&playAuditEndSequence!=playTracker.serial){
        playAuditEndSequence=playTracker.serial;
        if(playAuditSequence!=playTracker.serial)log("play-summary","Play #"+std::to_string(playTracker.serial)+" ended; official result still pending. Penalty/injury scenes can delay or discard that result; live catch/run reactions depend on the earlier verified samples.");
    }
    if(live&&active)crowdBoost=audioContinuity.record(match,time,coreAtmosphere.parent,coreAtmosphere.limit);
    else crowdBoost=audioContinuity.gap(match,time,active,coreAtmosphere.limit);

}
attendance::CallbackGate atmosphereUpdateGate;
bool guardedAtmosphereTick(){return atmosphereUpdateGate.run(atmosphereTick);}
void temporarySeats(attendance::State s,int permanent){
    int missing=0,bathroom=0,arriving=0;bool was=false,now=false;
    auto density=std::atomic_load(&renderedOccupancy);
    bool packed=density->owns(audioMatch.load(),audioEpoch.load())&&attendance::packed::qualifies(density->seats,density->peak()*(100-permanent)*.01f);
    {std::lock_guard<std::mutex> guard(historyMutex);
        was=halftimeModel.active;
        // Authoritative quarter changes can report QuarterEnd rather than Preplay.
        s.phase=5;bathroom=halftimeModel.observe(s,permanent,halftimeExtra.load(),packed);
        now=halftimeModel.active;arriving=arrivalModel.missing;
        if(halftimeModel.releaseAtBreak)departureModel.halftimeVisible();
        missing=departureModel.paceTemporary(permanent,(std::max)(arriving,bathroom));
        // At the first hidden update after the return deadline, clear only
        // temporary seats. Permanent losses and original empty seats stay put.
        if(s.quarter==3&&halftimeModel.done&&!arriving)missing=0;
        if(s.quarter==3)halftimeModel.lastAbsent=(std::min)(halftimeModel.lastAbsent,(std::max)(0,missing-permanent));
    }
    shownArriving=(s.quarter<3&&bathroom==0)||arriving?(std::max)(0,missing-permanent):0;
    shownHalftime=bathroom>0||s.quarter==3?(std::max)(0,missing-permanent):0;
    if(was!=now)log("halftime",now?(s.quarter==2?"Fans are leaving their seats early for halftime; these temporary seats can fill again after the break.":"Halftime seat break active; temporarily absent fans will return during Q3."):"Halftime seat break ended; temporarily absent fans may return. Permanent departures stay empty.");
    if(arrivalMissing.exchange(missing)!=missing)arrivalReconcilePending=true;
}
void reportFourthBreakGroup(){
    int points=0;{std::lock_guard<std::mutex> guard(historyMutex);points=departureModel.takeFourthBreakNotice();}
    if(points)log("quarter-break","Q3/Q4 break group: "+std::to_string(points)+" percentage point(s) added to the permanent-departure target; shared crowd floors, quarter cap and hidden-update limits still apply.");
}
void checkpoint(const attendance::Settings& c,const attendance::State& s,bool warning){
    attendance::departures::Result result;std::string why;
    {std::lock_guard<std::mutex> guard(historyMutex);
        departureModel.observe(c,s,gameHistory,mode==0,true);
        result=departureModel.settle(s,static_cast<int>(currentLoss),0,c.quarterCap);
        why=attendance::departures::reason(c,s,gameHistory,result.catchup,result.traffic);
    }
    reportFourthBreakGroup();
    int before=100-static_cast<int>(currentLoss),after=100-result.loss;
    int margin=s.homeScore-s.awayScore;
    std::string text="Q"+std::to_string(s.quarter)+(warning?" two-minute checkpoint: ":" start: ");
    text+=margin==0?"score tied":std::string(s.neutral?"margin ":"home ")+(s.neutral?"":margin>0?"leads by ":"trails by ")+std::to_string(std::abs(margin));
    text+="; crowd ";
    if(after==before)text+="held at "+std::to_string(after)+"%";
    else text+="reduced from "+std::to_string(before)+"% to "+std::to_string(after)+"%";
    log(warning?"two-minute":"quarter-break",text+" ("+why+").");
    temporarySeats(s,result.loss);
    bool reconcile=arrivalReconcilePending.exchange(false);
    if(result.loss!=lastApplied||reconcile||visibilityGeneration!=entityGeneration.load()){
        removalLimit=static_cast<unsigned int>(result.loss*65536u/100u);activeLoss=result.loss;
        if(!guardedApply(result.loss>0||reconcile,reconcile)){enabled=false;log("fault","Crowd update failed; further atmosphere changes stopped.");return;}
        lastApplied=result.loss;
        visibilityGeneration=entityGeneration.load();
    }
    currentLoss=highestTarget=static_cast<float>(result.loss);shownCurrent=currentLoss;shownTarget=highestTarget;
}
attendance::arrivals::Settings arrivalSettings(){
    attendance::arrivals::Settings selected{arrivalEnabled.load(),arrivalStart.load(),arrivalLateStart.load(),arrivalFinish.load(),arrivalLateFinish.load(),arrivalBigBonus.load(),arrivalWeatherPenalty.load()};
    if(attendance::scope::playNow(at<int>(image+0xd9eb808)))selected.openingOverride=75;
    return selected;
}
void beginAttendanceHistory(attendance::State& s,bool midgame){
    if(attendance::scope::playNow(at<int>(image+0xd9eb808))){
        // Session-only plan. Never resume or append a resident Dynasty archive.
        s.variationSeed=static_cast<unsigned>(GetTickCount64()^lastMatch);
        attendanceSeed=s.variationSeed;publishSessionPlan(lastMatch,s);
        {std::lock_guard<std::mutex> guard(timelineMutex);attendanceArchive.active=0;}
        bootstrapPending=false;
        log("play-now","Basic offline atmosphere: native starting attendance and visitor appearance; live matchup/score/weather; no Dynasty turnout, travel or saved history.");
        return;
    }

    attendance::timeline::Point saved;bool matched=false;
    uint64_t sourceGame=0;auto fixture=std::atomic_load(&crowdFixture);
    if(midgame&&fixture->owns(s.team0,s.team1)){
        {std::lock_guard<std::mutex> guard(visitorPlanMutex);sourceGame=visitorPlans.source(attendance::visitors::key(fixture->seed));}
        if(sourceGame){std::lock_guard<std::mutex> guard(timelineMutex);matched=attendanceArchive.resume(s,saved,sourceGame);}
    }
    FILETIME stamp{};GetSystemTimeAsFileTime(&stamp);uint64_t id=(uint64_t(stamp.dwHighDateTime)<<32)|stamp.dwLowDateTime;
    resumeHomeLoss=resumeAwayLoss=-1;resumeLegacyBudget=matched&&!saved.sharedBudget&&saved.temporary>saved.loss;
    if(matched){attendance::copyCrowdPlan(saved.state,s);resumeHomeLoss=saved.homeLoss;resumeAwayLoss=saved.awayLoss;}
    publishSessionPlan(lastMatch,s);
    s.variationSeed=matched?saved.state.variationSeed:static_cast<unsigned>(id^(id>>32));
    bool changedSeed=attendanceSeed.exchange(s.variationSeed)!=s.variationSeed;
    {std::lock_guard<std::mutex> guard(crowdMutex);sharedFanExits={};sharedFanBasis={};sharedFanExitsSeeded=false;for(auto& item:startingCrowds){
        if(changedSeed)item.second.selection={};
        item.second.exits={};item.second.exitsSeeded=false;
    }if(changedSeed)seatPatternPending=!startingCrowds.empty();visibilityRetry=true;}
    if(midgame&&mode==0){
        auto c=std::atomic_load(&settings);auto seed=attendance::timeline::estimate(*c,arrivalSettings(),s,matched?&saved:nullptr,halftimeExtra.load());
        {std::lock_guard<std::mutex> guard(historyMutex);gameHistory=seed.history;departureModel=seed.departures;arrivalModel=seed.arrivals;halftimeModel=seed.halftime;}
        currentLoss=highestTarget=static_cast<float>(seed.loss);activeLoss=seed.loss;arrivalMissing=seed.temporary;
        shownCurrent=currentLoss;shownTarget=highestTarget;shownArriving=seed.arrivals.missing;
        shownHalftime=seed.halftime.active?(std::max)(0,seed.temporary-seed.loss):0;
        shownResumeSource=matched?2:1;exactVisibility=true;lastApplied=-1;visibilityRetry=true;
        log("resumed-game",std::string(matched?"Restored recorded attendance":"Estimated attendance; no matching crowd history")+": Q"+std::to_string(s.quarter)+", home "+std::to_string(s.homeScore)+" - away "+std::to_string(s.awayScore)+"; target "+std::to_string(100-(std::max)(seed.loss,seed.temporary))+"% of this game's original crowd."+(matched?" Source clock "+std::to_string(saved.state.seconds)+"s; loading-only records excluded.":"")+" Future departures use this new scenario.");
    }
    {std::lock_guard<std::mutex> guard(timelineMutex);attendanceArchive.begin(id,s,matched);}
    if(midgame&&s.quarter>=3)halftimeExitConversion.checked=true;
    bootstrapPending=false;
}
bool ordinaryOpeningAcknowledgement(A match,const attendance::State& s){
    if(!contextReady||match!=lastMatch||s.quarter!=1||s.homeScore||s.awayScore||s.seconds<s.quarterSeconds*0.95f||
       (s.phase!=6&&s.phase!=1&&s.phase!=5)||at<int>(image+0xd9fba20)!=shownTeam0||at<int>(image+0xd9fba24)!=shownTeam1)return false;
    std::lock_guard<std::mutex> guard(historyMutex);return matchLifecycle.keepAtKickoff(match);
}
bool ordinarySessionAcknowledgement(A match,const attendance::State& s){
    if(ordinaryOpeningAcknowledgement(match,s))return true;
    if(!contextReady||match!=lastMatch||s.phase==6||s.phase==7||at<int>(image+0xd9fba20)!=shownTeam0||at<int>(image+0xd9fba24)!=shownTeam1)return false;
    {std::lock_guard<std::mutex> guard(historyMutex);if(matchLifecycle.needsReset(match,false))return false;}
    std::lock_guard<std::mutex> guard(timelineMutex);return attendanceArchive.active!=0;
}
// Runs on presentation and native replay callbacks, including paused viewers.
// It changes visibility only; the live departure/history/audio models stay put.
bool replayPresentation(){
    attendance::timing::Scope measured(replayCrowdTimings);
    auto replay=replaySnapshot();
    if(attendance::scope::playNow(at<int>(image+0xd9eb808))){
        attendance::State current;A match=0;
        replayLoss=-1;replayWasActive=false;replayRestorePending=false;
        replaySuspendsAtmosphere=gameState(current,match)&&replayBinding&&(!replay.valid||replay.active);
        return replaySuspendsAtmosphere;
    }
    attendance::State s;A match=0;
    if(replay.active||replayWasActive||replayRestorePending){
        // Never restore or replay an old scene before the current match has
        // initialized. Let decision() see a world change or a resumed game.
        bool verifiedGame=gameState(s,match);
        bool currentSession=true;
        if(verifiedGame&&match==lastMatch&&contextReady){
            {std::lock_guard<std::mutex> guard(historyMutex);
                currentSession=!matchLifecycle.needsReset(match,s.phase==6)&&
                    !attendance::timeline::recordingRestarted(replay.valid,replay.recording,lastRecordingTime,s,departureModel.previous);
            }
            {std::lock_guard<std::mutex> guard(timelineMutex);currentSession=currentSession&&attendanceArchive.active!=0;}
            int team0=at<int>(image+0xd9fba20),team1=at<int>(image+0xd9fba24);
            if(team0>0&&team0<2047&&team1>0&&team1<2047&&(team0!=shownTeam0||team1!=shownTeam1))currentSession=false;
            auto signal=std::atomic_load(&resumeSignal);
            if(resumeTracker.pending(*signal,s,GetTickCount64(),ordinarySessionAcknowledgement(match,s)))currentSession=false;
        }
        if(!attendance::timeline::ownsReplayCrowd(verifiedGame,match,lastMatch,contextReady,currentSession)){
            replayLoss=-1;replaySuspendsAtmosphere=false;replayWasActive=false;replayRestorePending=false;
            return false;
        }
    }
    // Hold the current picture through a transient replay-manager rebuild.
    // A verified new world above still clears ownership and initializes normally.
    if(replayBinding&&!replay.valid)return replayWasActive||replayRestorePending;
    bool paused=pauseBinding&&!nativeUnpaused(),finalMenu=false;
    if(replay.active&&!paused){
        if(s.phase==7){std::lock_guard<std::mutex> guard(atmosphereMutex);finalMenu=coreAtmosphere.postgameMenu;}
    }
    auto policy=attendance::timeline::replayPolicy(replay.active,paused,finalMenu);
    replaySuspendsAtmosphere=policy.suspendAtmosphere;
    if(policy.historical){
        attendance::timeline::Point point;bool found=false;
        {std::lock_guard<std::mutex> guard(timelineMutex);found=attendanceArchive.replay(replay.playback,point);}
        int loss=found?point.total():(std::max)(activeLoss.load(),arrivalMissing.load());
        float h=found?point.homeLoss:-1,a=found?point.awayLoss:-1;
        auto livePopulation=std::atomic_load(&visitorPopulation);
        bool shared=found?point.sharedBudget:livePopulation->sharedBudget;
        if(replayHomeLoss.load()!=h||replayAwayLoss.load()!=a||replaySharedBudget.load()!=shared)visibilityRetry=true;
        replayHomeLoss=h;replayAwayLoss=a;replayVisitorArrivals=found?point.temporary:arrivalMissing.load();replayPermanentLoss=found?point.loss:activeLoss.load();replaySharedBudget=shared;
        replayLoss=loss;
        if(!replayWasActive)log("replay",found?std::string(policy.suspendAtmosphere?"Replay":"Automatic replay")+": showing the recorded crowd; live attendance is preserved.":"No earlier crowd record; using this scenario's initialized attendance.");
        replayWasActive=true;
        if(!refreshReplayVisibility()){enabled=false;log("fault","Replay crowd update failed; automatic updates stopped.");return true;}
        return true;
    }
    if(replayWasActive){replayWasActive=false;replayLoss=-1;replayHomeLoss=replayAwayLoss=-1;replayVisitorArrivals=-1;replayRestorePending=true;log("replay","Replay ended; restoring the live game's crowd.");}
    if(replayRestorePending){
        exactVisibility=true;
        if(!refreshReplayVisibility()){enabled=false;log("fault","Live crowd restoration failed; automatic updates stopped.");return false;}
        if(!visibilityRetry){exactVisibility=false;replayRestorePending=false;}
    }
    return false;
}
bool guardedReplayPresentation(){__try{return replayPresentation();}__except(EXCEPTION_EXECUTE_HANDLER){replayLoss=-1;replaySuspendsAtmosphere=false;replayWasActive=false;replayRestorePending=true;return false;}}
bool guardedSimulationDecision(){
    __try {
        int phase=at<int>(at<A>(image+attendance::bindings::GameStates)+0xa0);
        if(phase!=0&&phase!=5)return true;
        if(!nativeUnpaused())return true;
        attendance::State s;A match=0;
        return !gameState(s,match)||!offscreenSimulation(s)||guardedDecision();
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
void __fastcall replayUpdate(A entity){
    flushReplaySelectorTiming();
    originalReplayUpdate(entity);++replayCallbacks;
    if(!enabled||at<A>(entity)!=image+attendance::bindings::ReplayEntityVtable)return;
    std::unique_lock<std::mutex> frame(presentationMutex,std::try_to_lock);if(!frame.owns_lock())return;
    // This callback also runs when gameplay/post-update is suspended. Broadcast
    // clips keep audio intents fresh without feeding replay state to departures.
    if(!guardedEmptyService()){log("fault","2020 Mode could not update its crowd safely.");return;}
    bool historical=guardedReplayPresentation();
    guardedMajorReplay(true);
    if(!historical){
        // Normal/Fast Super Sim suspends the main presentation callback, but
        // this native replay entity still updates. Service only a verified
        // offscreen simulation, on this same serialized native update thread.
        if(!guardedSimulationDecision()){
            enabled=false;log("fault","Super Sim crowd update failed; automatic updates stopped.");
        }
    }
    if(historical&&!replaySuspendsAtmosphere&&!guardedAtmosphereTick()){
        playBindingsAvailable=false;crowdBoost=0;log("audio-inactive","Presentation audio update unavailable; attendance history remains active.");
    }
}
void recordAttendance(){
    if(!attendance::scope::dynasty(at<int>(image+0xd9eb808)))return;
    static uint64_t last=0;static int loss=-1,missing=-1;auto now=GetTickCount64();
    if(now-last<250&&loss==activeLoss.load()&&missing==arrivalMissing.load())return;
    attendance::State s;A match=0;if(mode!=0||!std::atomic_load(&settings)->enabled||!contextReady||replayWasActive||!gameState(s,match)||match!=lastMatch)return;
    // Offscreen sim has no new filmed plays. Its frozen recording timestamp
    // must not overwrite the crowd attached to the last real replay clip.
    if(offscreenSimulation(s))return;
    bool paused=s.phase!=6&&!nativeUnpaused();
    if(paused){
        // Preserve the last visible crowd at a real pause without allowing an
        // unconfirmed loading scene to seed its own Midgame Save restoration.
        std::lock_guard<std::mutex> guard(timelineMutex);
        if(bootstrapPending||attendanceArchive.games.empty()||attendanceArchive.games.back().id!=attendanceArchive.active||!attendanceArchive.games.back().resumeReady)return;
    }
    s.team0=shownTeam0;s.team1=shownTeam1;s.rank0=shownRank0;s.rank1=shownRank1;s.rivalry=shownRivalry;s.playoff=shownPlayoff;s.conferenceChampionship=shownConference;s.bowl=shownBowl;s.neutral=shownNeutral;s.precipitation=shownPrecipitation;
    s.variationSeed=attendanceSeed;applySessionPlan(s,match);
    auto replay=replaySnapshot();if(replay.active)return;
    if(simulationRecording>=0&&replay.valid&&replay.recording<=simulationRecording)return;
    simulationRecording=-1;
    attendance::timeline::Point p;
    if(!attendance::timeline::recordingTime(replayBinding,replay.valid,replay.recording,attendance::elapsedGameTime(s),p.time))return;
    p.state=s;p.loss=activeLoss;p.temporary=arrivalMissing;p.sharedBudget=false;
    auto supporters=std::atomic_load(&visitorPopulation);
    if(supporters->owns(match,audioEpoch.load())&&supporters->originalHome+supporters->originalAway){p.sharedBudget=supporters->sharedBudget;if(p.sharedBudget){p.loss=supporters->appliedLoss;p.temporary=supporters->appliedTemporary;}p.homeLoss=supporters->homeLoss;p.awayLoss=supporters->awayLoss;p.originalAwayShare=static_cast<float>(static_cast<double>(supporters->originalAway)/(supporters->originalHome+supporters->originalAway));}
    {std::lock_guard<std::mutex> guard(historyMutex);p.history=gameHistory;p.periodStart=(std::clamp)(departureModel.periodStartLoss,0,p.loss);}
    uint64_t recordedGame=0;
    {std::lock_guard<std::mutex> guard(timelineMutex);if(attendanceArchive.record(p,paused)&&!attendanceArchive.games.empty()&&attendanceArchive.games.back().resumeReady)recordedGame=attendanceArchive.active;}
    if(recordedGame){auto fixture=std::atomic_load(&crowdFixture);if(fixture->owns(s.team0,s.team1)){
        std::lock_guard<std::mutex> guard(visitorPlanMutex);visitorPlans.bind(attendance::visitors::key(fixture->seed),recordedGame);
    }}
    if(replay.valid)lastRecordingTime=replay.recording;
    last=now;loss=p.loss;missing=p.temporary;
}
bool guardedRecordAttendance(){__try{recordAttendance();return true;}__except(EXCEPTION_EXECUTE_HANDLER){return false;}}
bool carryPreparedOpening(A match,const attendance::State& s){
    int home=at<int>(image+0xd9fba20),away=at<int>(image+0xd9fba24);
    bool same=contextReady&&home>0&&away>0&&home==shownTeam0&&away==shownTeam1;
    {std::lock_guard<std::mutex> guard(historyMutex);if(!matchLifecycle.carryOpening(match,s,same))return false;}
    A previous=lastMatch;lastMatch=match;audioMatch=match;auto plan=std::atomic_load(&sessionCrowdPlan);if(plan->match==previous)publishSessionPlan(match,plan->state);
    auto density=std::atomic_load(&renderedOccupancy);
    if(density->owns(previous,audioEpoch.load())){auto carried=*density;carried.match=match;std::shared_ptr<const attendance::occupancy::Rendered> next=std::make_shared<attendance::occupancy::Rendered>(carried);std::atomic_store(&renderedOccupancy,next);}
    {std::lock_guard<std::mutex> guard(atmosphereMutex);if(atmosphereMatch==previous)atmosphereMatch=match;}
    return true;
}
void decision(){
    attendance::timing::Scope measured(decisionTimings);
    const auto now=GetTickCount64();const int pending=request.load();
    attendance::State s;A match=0;if(!gameState(s,match)){
        crowdChangeWindow.reset();
        observeWorldExit();
        lastFrame=now;return;
    }
    if(s.phase==7){
        // A postgame cutscene can stream a fresh crowd renderer. Rebuild its
        // selection and reapply the final picture, without advancing rules or
        // inventing a new session from the final scoreboard.
        shownSuperSim=false;bool ready=postgameRendererRetry.exchange(false);
        if(!ready&&now-lastDecision<500)return;
        lastDecision=now;bool owned=match==lastMatch&&contextReady&&mode==0&&std::atomic_load(&settings)->enabled;
        {std::lock_guard<std::mutex> guard(historyMutex);owned=owned&&!matchLifecycle.needsReset(match,false);}
        if(owned){
            shownQuarter=s.quarter;shownPhase=s.phase;shownSeconds=s.seconds;shownScore0=s.homeScore;shownScore1=s.awayScore;
            if(visibilityRetry||seatPatternPending||visibilityGeneration!=entityGeneration.load()){
                if(!guardedApply(activeLoss>0||arrivalMissing>0,true)){enabled=false;log("fault","Postgame crowd renderer could not be refreshed safely.");return;}
                visibilityGeneration=entityGeneration.load();
            }
        }
        lastFrame=now;return;
    }
    const bool sim=offscreenSimulation(s);const auto visualState=s;
    crowdChangeWindow.observe(match,s,sim);
    // Track short phase transitions before throttling the heavier work.
    bool pregame=s.phase==6;
    bool rendererReady=pregame&&pregameRendererRetry.exchange(false);
    if(!pending&&!rendererReady&&now-lastDecision<500)return;
    if(!pregame&&s.phase!=1&&s.phase!=5&&!sim){lastFrame=now;return;}
    // Normalize only our calculation snapshot. Never change a native phase,
    // clock, replay selector or Super Sim setting.
    if(sim)s.phase=5;
    lastDecision=now;shownQuarter=s.quarter;shownSeconds=s.seconds;shownScore0=s.homeScore;shownScore1=s.awayScore;shownPhase=s.phase;
    const auto generation=entityGeneration.load();
    bool newMatch=false;
    auto signal=std::atomic_load(&resumeSignal);bool openingAcknowledgement=ordinaryOpeningAcknowledgement(match,s);bool sessionAcknowledgement=ordinarySessionAcknowledgement(match,s);int previousResumeGame=resumeTracker.game;bool previousResumeKnown=resumeTracker.seen;
    bool nativeResume=attendance::scope::dynasty(at<int>(image+0xd9eb808))&&resumeTracker.observe(*signal,s,now,sessionAcknowledgement);
    if(sessionAcknowledgement&&!nativeResume&&previousResumeKnown&&resumeTracker.game!=previousResumeGame)
        log("new-game",openingAcknowledgement?"Midgame Save confirmed the new game; keeping the prepared opening crowd.":"Midgame Save announced this game's first played snap after simulation; keeping the original seating baseline and attendance history.");
    int team0=at<int>(image+0xd9fba20),team1=at<int>(image+0xd9fba24);
    bool changedContextMode=contextReady&&contextGameMode.load()!=at<int>(image+0xd9eb808);
    bool changedTeams=contextReady&&team0>0&&team0<2047&&team1>0&&team1<2047&&(team0!=shownTeam0||team1!=shownTeam1);
    auto replay=replaySnapshot();
    bool restartedRecording=attendance::timeline::recordingRestarted(replay.valid,replay.recording,lastRecordingTime,s,departureModel.previous);
    if(!pregame&&!nativeResume&&!changedTeams&&!restartedRecording)carryPreparedOpening(match,s);
    {std::lock_guard<std::mutex> guard(historyMutex);newMatch=matchLifecycle.needsReset(match,pregame);}
    newMatch=newMatch||nativeResume||changedTeams||changedContextMode||restartedRecording;
    bool newSession=!lastMatch||newMatch||generation!=lastGeneration;
    if(newSession){
        bool first=!lastMatch||newMatch;
        if(newMatch){
            // Remove our old masks before taking the new game's baseline when
            // Midgame Save reuses the resident crowd renderer.
            replayLoss=-1;
            if(!guardedApply(false,true)){enabled=false;log("fault","Previous crowd could not be released safely.");return;}
            resetMatch();crowdChangeWindow.observe(match,visualState,sim);
        }
        bootstrapPending=bootstrapPending||first;
        lastMatch=match;lastGeneration=generation;
        audioMatch=match;
        lastApplied=static_cast<int>(currentLoss);observed.clear();lastFrame=now;
        attendance::State detail=s;if(guardedContext(detail)){showContext(*std::atomic_load(&settings),detail);}else log("context-unavailable","Waiting for valid matchup data");
        refreshEntities=true;log("session","quarter="+std::to_string(s.quarter)+" clock="+std::to_string(s.seconds)+" home:away="+std::to_string(s.homeScore)+":"+std::to_string(s.awayScore));
    }
    {std::lock_guard<std::mutex> guard(historyMutex);matchLifecycle.join(match,pregame);}
    if(!contextReady){attendance::State detail=s;if(guardedContext(detail))showContext(*std::atomic_load(&settings),detail);}
    // School arrival curves and postseason floors require verified venue/team context.
    if(!contextReady)return;
    s.rank0=shownRank0;s.rank1=shownRank1;s.rivalry=shownRivalry;s.playoff=shownPlayoff;s.conferenceChampionship=shownConference;s.precipitation=shownPrecipitation;
    s.neutral=shownNeutral;s.team0=shownTeam0;s.team1=shownTeam1;s.bowl=shownBowl;applySessionPlan(s,match);
    {std::lock_guard<std::mutex> guard(timelineMutex);if(attendance::scope::dynasty(at<int>(image+0xd9eb808))&&!attendanceArchive.active)bootstrapPending=true;}
    if(bootstrapPending)beginAttendanceHistory(s,!pregame&&((nativeResume&&signal->applied)||s.quarter>1||s.homeScore||s.awayScore||s.seconds<s.quarterSeconds*0.95f));
    s.variationSeed=attendanceSeed;
    attendance::energy::Play detail;detail.game=s;
    if(!sim&&!pregame&&playData(match,detail)){s.offense=detail.offense;s.homeTimeouts=detail.homeTimeouts;s.toGoal=detail.toGoal;}
    const auto liveSettings=std::atomic_load(&settings);bool warning=false,accepted=false,freeze=false,upsetWatch=false;
    {std::lock_guard<std::mutex> guard(historyMutex);accepted=pregame||gameHistory.observe(*liveSettings,s);freeze=gameHistory.comeback&&attendance::inferiorOpponent(s)<1;upsetWatch=gameHistory.upsetWatch;}
    if(!accepted)return;
    shownSuperSim=sim;
    if(sim&&replay.valid)simulationRecording=replay.recording;
    if(pregame||nativeUnpaused()){
        std::lock_guard<std::mutex> guard(historyMutex);
        departureModel.observe(*liveSettings,s,gameHistory,mode==0);
    }
    reportFourthBreakGroup();
    bool changeWindow=(pregame||nativeUnpaused())&&crowdChangeWindow.take(match,visualState,sim);
    if(changeWindow&&!pregame){std::lock_guard<std::mutex> guard(historyMutex);warning=gameHistory.takeWarning();}
    if(freeze!=shownComeback.load())log("protection",freeze?"Comeback protection active; crowd departures are reduced or paused.":"Comeback protection ended; future checkpoints may remove more fans.");
    shownComeback=freeze;shownUpset=upsetWatch;

    lastQuarter=s.quarter;lastClock=s.seconds;lastScore0=s.homeScore;lastScore1=s.awayScore;
    int command=pregame?0:request.exchange(0);const auto c=std::atomic_load(&settings);
    if(command==5){mode=0;restoreAt=0;} // Automatic changes settle at the next safe selection/break.
    if(command==2||command==4){mode=1;currentLoss=command==4?100.0f:50.0f;highestTarget=currentLoss;lastApplied=-1;restoreAt=command==2?now+12000:0;}
    if(command==1||command==3){mode=2;currentLoss=highestTarget=0;lastApplied=-1;restoreAt=0;}
    if(restoreAt&&now>=restoreAt){mode=2;currentLoss=highestTarget=0;lastApplied=-1;restoreAt=0;}
    if(!c->enabled){currentLoss=highestTarget=0;}
    if(!changeWindow&&(mode!=0||!c->enabled)){
        std::lock_guard<std::mutex> guard(historyMutex);
        auto finished=arrivalModel.finish();if(finished.changed||arrivalMissing>0)arrivalReconcilePending=true;
        arrivalMissing=0;shownHalftime=shownArriving=0;
    }
    if(changeWindow){
    {std::lock_guard<std::mutex> guard(historyMutex);departureModel.beginWindow(s,sim?100:c->transitionCap);departureModel.anchorVisible(static_cast<int>(currentLoss),arrivalMissing.load());}
    attendance::arrivals::Change arriving;
    {std::lock_guard<std::mutex> guard(historyMutex);
        auto arrivals=arrivalSettings();
        arriving=mode==0&&c->enabled?arrivalModel.observe(arrivals,s):arrivalModel.finish();
        if(arriving.finished)arrivalCompletion.targetReached();
    }
    if(arriving.starting){
        {std::lock_guard<std::mutex> guard(historyMutex);departureModel.openingVisible();}
        captureArrivalBaseline=true;
        if(!guardedApply(false,false,true)){enabled=false;log("fault","Starting crowd could not be captured; arrivals stopped.");return;}
        log("arrivals",std::string(pregame?"Pregame":"First-quarter")+" arrival target: starting at "+std::to_string(100-arriving.missing)+"% of the game's original crowd; later arrivals settle at play selection.");
    }
    if(arriving.changed){
        arrivalReconcilePending=true;
        if(!arriving.starting)log("arrivals",arriving.finished?"First-quarter arrival target reached; visible catch-up settles at safe breaks.":"First-quarter arrival target: "+std::to_string(100-arriving.missing)+"% of the original crowd.");
    }
    if(mode==0&&c->enabled)temporarySeats(s,static_cast<int>(currentLoss));
    else {arrivalMissing=0;shownHalftime=shownArriving=0;}
    if(warning&&s.quarter==4&&contextReady&&c->warningEnabled&&c->enabled&&mode==0)checkpoint(*c,s,true);
    else if(warning&&s.quarter==4&&!contextReady&&c->warningEnabled&&mode==0)log("checkpoint-skipped","Two-minute departure skipped because matchup information was unavailable.");
    int extra=0;{std::lock_guard<std::mutex> guard(atmosphereMutex);extra=pendingLateExits;pendingLateExits=0;}
    if(extra&&c->enabled&&mode==0&&!s.playoff&&s.quarter==4){
        int before=static_cast<int>(currentLoss);attendance::departures::Result late;
        {std::lock_guard<std::mutex> guard(historyMutex);late=departureModel.settle(s,before,extra,c->quarterCap);}
        currentLoss=highestTarget=static_cast<float>(late.loss);
        log("late-score","Late-score departure: crowd reduced from "+std::to_string(100-before)+"% to "+std::to_string(100-static_cast<int>(currentLoss))+"% of the original crowd.");
    }
    if(!pregame&&mode==0&&c->enabled){
        attendance::departures::Result gradual;std::string why;
        {std::lock_guard<std::mutex> guard(historyMutex);
            gradual=departureModel.settle(s,static_cast<int>(currentLoss),0,c->quarterCap);
            why=attendance::departures::reason(*c,s,gameHistory,gradual.catchup,gradual.traffic);
        }
        if(gradual.changed){
            int before=static_cast<int>(currentLoss);currentLoss=highestTarget=static_cast<float>(gradual.loss);
            log("departures",std::string(sim?"Super Sim departures: ":"Between-play departures: ")+"crowd reduced from "+std::to_string(100-before)+"% to "+std::to_string(100-gradual.loss)+"% ("+why+").");
            // Recompute temporary absences against the new permanent target.
            temporarySeats(s,gradual.loss);
        }
    }
    if(mode==0&&c->enabled&&s.quarter>=3){
        int before=static_cast<int>(currentLoss),converted=0;
        {
            std::lock_guard<std::mutex> guard(historyMutex);
            int total=(std::max)(before,arrivalMissing.load());
            int conversionWanted=halftimeExitConversion.request(s,before,total,halftimeConvertPercent.load());
            if(conversionWanted>0){
                auto conversionResult=departureModel.convertSeatBreakers(s,before,conversionWanted,c->quarterCap);
                converted=(std::max)(0,conversionResult.loss-before);
                if(converted){
                    halftimeModel.convert(s,before,conversionResult.loss,total,halftimeExtra.load());
                    currentLoss=highestTarget=static_cast<float>(conversionResult.loss);
                    shownHalftime=(std::max)(0,total-conversionResult.loss);shownHalftimeConverted=converted;
                }
            }
        }
        if(converted)log("halftime","Home blowout: "+std::to_string(converted)+"% of the original crowd already away for halftime will not return. Visible crowd unchanged at this break; normal floors and quarter limits retained.");
    }
    } // Settle at selection, pregame or verified offscreen Sim, never while lined up.
    lastFrame=now;shownCurrent=currentLoss;shownTarget=highestTarget;
    int percent=static_cast<int>(currentLoss);
    bool reconcile=arrivalReconcilePending.load();
    if((percent!=lastApplied||reconcile||visibilityGeneration!=generation||visibilityRetry)&&(!seatPatternPending||pregame||changeWindow||command||exactVisibility||mode!=0||!c->enabled)){
        reconcile=arrivalReconcilePending.exchange(false);
        removalLimit=static_cast<unsigned int>(percent*65536u/100u);activeLoss=percent;
        if(!guardedApply(percent>0||reconcile||arrivalMissing>0,reconcile)){enabled=false;log("fault","Native crowd update failed; automatic updates stopped");return;}
        lastApplied=percent;
        visibilityGeneration=generation;
        if(!visibilityRetry)exactVisibility=false;
    }
    bool arrivalsSettled=false;
    {std::lock_guard<std::mutex> guard(historyMutex);
        if(mode==0&&c->enabled)arrivalsSettled=arrivalCompletion.settled(arrivalMissing.load(),percent,lastApplied==percent&&!arrivalReconcilePending.load()&&!visibilityRetry);
        else arrivalCompletion.reset();
    }
    if(arrivalsSettled)log("arrivals","Arrival catch-up complete; all remaining expected fans have been seated. Permanent departures stay empty.");
}
bool guardedDecision(){__try {decision();return true;}__except(EXCEPTION_EXECUTE_HANDLER){applying=false;thinning=false;return false;}}
void quarterBreak(int previous,int next){
    if(!enabled||mode!=0||replaySnapshot().active||next<=previous||previous<1||next>4||(pauseBinding&&!nativeUnpaused()))return;
    attendance::State s;A match=0;if(!gameState(s,match)||s.quarter!=next)return;
    auto c=std::atomic_load(&settings);if(!c->enabled)return;
    if(!guardedContext(s)){log("checkpoint-skipped","Quarter departure skipped because matchup information was unavailable.");return;}
    s.variationSeed=attendanceSeed;
    showContext(*c,s);
    attendance::arrivals::Change arrivals;
    {std::lock_guard<std::mutex> guard(historyMutex);if(!gameHistory.observe(*c,s,true)||!gameHistory.takeQuarter(previous,next))return;arrivals=arrivalModel.finish();if(arrivals.finished)arrivalCompletion.targetReached();}
    if(arrivals.changed){arrivalReconcilePending=true;log("arrivals","First-quarter arrival target reached at the quarter break; visible catch-up settles at safe breaks, with permanent departures preserved.");}

    lastMatch=match;lastGeneration=entityGeneration.load();lastQuarter=next;lastClock=s.seconds;lastScore0=s.homeScore;lastScore1=s.awayScore;
    audioMatch=match;
    {std::lock_guard<std::mutex> guard(historyMutex);departureModel.beginWindow(s,c->transitionCap);departureModel.anchorVisible(static_cast<int>(currentLoss),arrivalMissing.load());}
    checkpoint(*c,s,false);
    shownQuarter=s.quarter;shownSeconds=s.seconds;shownScore0=s.homeScore;shownScore1=s.awayScore;shownPhase=s.phase;
}
void guardedQuarterBreak(int previous,int next){__try {quarterBreak(previous,next);}__except(EXCEPTION_EXECUTE_HANDLER){applying=false;thinning=false;enabled=false;}}
void __fastcall periodChanged(A manager,int next){
    int previous=at<int>(manager+8);
    if(enabled&&!replaySnapshot().active&&next==1&&manager==at<A>(image+attendance::bindings::ClockManager)){
        A wrapper=at<A>(image+attendance::bindings::MatchWrapper),match=at<A>(wrapper+16);bool keep=false;
        {std::lock_guard<std::mutex> guard(historyMutex);keep=matchLifecycle.keepAtKickoff(match);}
        if(!keep){attendance::State opening;A actual=0;if(gameState(opening,actual)&&actual==match)keep=carryPreparedOpening(match,opening);}
        if(!keep){
            if(!guardedApply(false,true)){enabled=false;log("fault","Previous crowd could not be released at kickoff.");}
            resetMatch();log("new-game","Starting crowd and departure history reset");
        }
        else log("new-game","Kickoff: preserving the crowd already prepared during pregame.");
    }
    if(enabled&&next>=2&&manager==at<A>(image+attendance::bindings::ClockManager))collectingStartingCrowd=false;
    originalPeriod(manager,next);
    if(manager==at<A>(image+attendance::bindings::ClockManager))guardedQuarterBreak(previous,next);
}
void readResumeStatus(uint64_t now){
    attendance::resume::Signal next;
    try{
        auto path=pluginsFolder/L"MidgameSave"/L"midgamesave-status.json";
        std::error_code error;auto size=std::filesystem::file_size(path,error);
        if(!error&&size>0&&size<=65536){
            auto modified=std::filesystem::last_write_time(path,error);
            auto age=std::filesystem::file_time_type::clock::now()-modified;
            if(!error&&age>=-std::chrono::seconds(2)&&age<=std::chrono::seconds(10)){
                std::ifstream f(path,std::ios::binary);std::string text((std::istreambuf_iterator<char>(f)),std::istreambuf_iterator<char>());
                if(text.size()<=65536)next=attendance::resume::parse(text,static_cast<int>(GetCurrentProcessId()),now);
            }
        }
    }catch(...){/* Missing/partial foreign status never blocks ordinary games. */}
    std::shared_ptr<const attendance::resume::Signal> signal=std::make_shared<attendance::resume::Signal>(next);std::atomic_store(&resumeSignal,signal);
}
void loadAudienceFoundations(){
    auto file=dataFolder/L"History"/L"audience-foundations.txt";std::error_code ec;
    if(!std::filesystem::exists(file,ec)&&!ec)return;
    auto size=std::filesystem::file_size(file,ec);
    attendance::audience::Book next;std::ifstream input(file);
    if(ec||size>4*1024*1024||!next.read(input)){
        audienceStorageReady=false;log("audience-history","Unreadable audience foundations preserved. New turnout is inactive until this file is repaired or moved aside; other atmosphere features remain available.");return;
    }
    input.close();
    if(next.upgraded){
        auto backup=dataFolder/L"History"/L"audience-foundations.pre-2.1.4.txt";
        if(!CopyFileW(file.c_str(),backup.c_str(),TRUE)&&!(GetLastError()==ERROR_FILE_EXISTS&&std::filesystem::is_regular_file(backup,ec)&&!ec)){
            audienceStorageReady=false;log("audience-history","Audience correction deferred: original foundation backup could not be saved. Existing file preserved; fresh turnout keeps EA attendance.");return;
        }
        log("audience-history","Audience foundations upgraded once: "+std::to_string(next.migrated)+" established-school estimates corrected; frozen references and custom-program estimates retained. Original file saved as audience-foundations.pre-2.1.4.txt.");
    }
    std::lock_guard<std::mutex> guard(audienceMutex);audienceBook=std::move(next);
}
void saveAudienceFoundations(){
    if(!audienceStorageReady)return;
    static uint64_t written=0;static bool warned=false;attendance::audience::Book snapshot;
    {std::lock_guard<std::mutex> guard(audienceMutex);if(written==audienceBook.revision)return;snapshot=audienceBook;}
    auto failed=[&](){if(!warned){log("audience-history","Audience foundations could not be saved in the plugin History folder. This session keeps its in-memory foundation; check folder permissions/free space before restarting.");warned=true;}};
    auto dir=dataFolder/L"History";if(!attendance::ensureFolder(dir)){failed();return;}
    auto pending=dir/L"audience-foundations.pending",file=dir/L"audience-foundations.txt";
    {std::ofstream output(pending,std::ios::trunc);snapshot.write(output);output.flush();if(!output){failed();return;}}
    if(MoveFileExW(pending.c_str(),file.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)){written=snapshot.revision;warned=false;}else failed();
}
void loadVisitorPlans(){
    auto file=dataFolder/L"History"/L"visitor-plans.txt";std::error_code ec;
    if(!std::filesystem::exists(file,ec)&&!ec)return;
    auto fail=[&](){visitorPlanWritable=false;log("visitor-history","Saved visiting-fan plans could not be read; existing file preserved. This session can use new in-memory plans, but cannot replace the unreadable file.");};
    auto bytes=std::filesystem::file_size(file,ec);if(ec||bytes>128*1024){fail();return;}
    std::ifstream input(file);attendance::visitors::PlanBook next;if(!attendance::visitors::PlanBook::read(input,next)){fail();return;}
    std::lock_guard<std::mutex> guard(visitorPlanMutex);visitorPlans=std::move(next);
}
void saveVisitorPlans(){
    if(!visitorPlanWritable)return;
    static uint64_t written=0;static bool warned=false;attendance::visitors::PlanBook snapshot;
    {std::lock_guard<std::mutex> guard(visitorPlanMutex);if(written==visitorPlans.revision)return;snapshot=visitorPlans;}
    std::vector<uint64_t> liveGames;{std::lock_guard<std::mutex> guard(timelineMutex);for(const auto& game:attendanceArchive.games)liveGames.push_back(game.id);}
    // Do not publish a link to a new in-memory game before its resumable
    // checkpoint has reached disk. Retry on the next worker tick instead.
    if(!attendance::visitors::linkedHistoryReady(snapshot,liveGames,savedAttendanceGames))return;
    auto fail=[&](){if(!warned)log("visitor-history","Visiting-fan plans could not be saved in the plugin History folder. Current appearances remain active; check folder permissions/free space before restarting.");warned=true;};
    auto dir=dataFolder/L"History";if(!attendance::ensureFolder(dir)){fail();return;}
    auto pending=dir/L"visitor-plans.pending",file=dir/L"visitor-plans.txt";
    {std::ofstream output(pending,std::ios::trunc);snapshot.write(output);output.flush();if(!output){fail();return;}}
    if(MoveFileExW(pending.c_str(),file.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)){written=snapshot.revision;warned=false;}else fail();
}
void loadAttendanceHistory(){
    auto path=dataFolder/L"History"/L"attendance-history.txt";
    std::error_code error;auto size=std::filesystem::file_size(path,error);if(error||size>8*1024*1024)return;
    std::ifstream f(path);attendance::timeline::Archive archive;
    if(attendance::timeline::read(f,archive)){
        attendanceArchive=std::move(archive);
        savedAttendanceGames.clear();for(const auto& game:attendanceArchive.games)if(game.resumeReady&&!game.points.empty())savedAttendanceGames.push_back(game.id);
        f.clear();f.seekg(0);std::string version;f>>version;
        if(version!="DGA-HISTORY-6"){
            std::filesystem::copy_file(path,path.parent_path()/L"attendance-history.pre-2.1.5.txt",std::filesystem::copy_options::skip_existing,error);
            if(error){attendanceHistoryWritable=false;log("history","Could not create pre-2.1.5 crowd-history backup; existing history preserved on disk. This session uses in-memory history; check History folder permissions.");}
        }
    }
    else log("history","Unreadable crowd history ignored; resumed games will use an estimate.");
}
void saveAttendanceHistory(){
    if(!attendanceHistoryWritable)return;
    static uint64_t written=0;attendance::timeline::Archive snapshot;
    {std::lock_guard<std::mutex> guard(timelineMutex);if(attendanceArchive.revision==written)return;snapshot=attendanceArchive;}
    auto dir=dataFolder/L"History";if(!attendance::ensureFolder(dir))return;
    auto file=dir/L"attendance-history.txt",pending=dir/L"attendance-history.pending";
    {std::ofstream f(pending,std::ios::trunc);attendance::timeline::write(f,snapshot);f.flush();if(!f)return;}
    if(MoveFileExW(pending.c_str(),file.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)){
        written=snapshot.revision;savedAttendanceGames.clear();for(const auto& game:snapshot.games)if(game.resumeReady&&!game.points.empty())savedAttendanceGames.push_back(game.id);
    }
}
void writeStatus(std::ostream& state,unsigned long long now){

    auto audio=std::atomic_load(&coreSettings);auto config=std::atomic_load(&settings);auto motion=std::atomic_load(&cameraSettings);
    auto population=std::atomic_load(&stadiumOccupancy);auto density=std::atomic_load(&renderedOccupancy);
    auto liveAudio=audioWanted();bool known=population->owns(liveAudio.match,liveAudio.epoch),mapped=density->owns(liveAudio.match,liveAudio.epoch);
    auto atmosphere=wantedSwell();
    attendance::StatusReport report(state,GAMEDAY_VERSION);
    SYSTEMTIME clock{};GetLocalTime(&clock);
    report.field("Updated")<<std::setfill('0')<<std::right<<std::setw(2)<<clock.wHour<<":"<<std::setw(2)<<clock.wMinute<<":"<<std::setw(2)<<clock.wSecond<<std::setfill(' ')<<" (local time; refreshes every second)\n";

    report.section("GAME & MATCHUP");
    report.field("Game")<<"Q"<<shownQuarter<<" | "<<shownSeconds<<" seconds remaining | Home "<<shownScore0<<" - Away "<<shownScore1<<"\n";
    report.field("Presentation phase")<<shownPhase<<"\n";
    report.field("Game process ID")<<GetCurrentProcessId()<<"\n";
    report.field("DGA atmosphere callback")<<atmosphereTimings.calls.load()<<" calls; max "<<atmosphereTimings.maximumUs.load()<<" us; >=4ms "<<atmosphereTimings.over4ms.load()<<"; >=16ms "<<atmosphereTimings.over16ms.load()<<" (wall time; not FPS)\n";
    report.field("DGA attendance decision")<<decisionTimings.calls.load()<<" calls; max "<<decisionTimings.maximumUs.load()<<" us; >=4ms "<<decisionTimings.over4ms.load()<<"; >=16ms "<<decisionTimings.over16ms.load()<<"\n";
    report.field("DGA crowd apply")<<crowdApplyTimings.calls.load()<<" calls; max "<<crowdApplyTimings.maximumUs.load()<<" us; >=4ms "<<crowdApplyTimings.over4ms.load()<<"; >=16ms "<<crowdApplyTimings.over16ms.load()<<"\n";
    report.field("DGA seat preparation")<<seatPreparationTimings.calls.load()<<" calls; max "<<seatPreparationTimings.maximumUs.load()<<" us (layout preparation, not every play)\n";
    report.field("DGA crowd measurement")<<crowdMeasurementTimings.calls.load()<<" calls; max "<<crowdMeasurementTimings.maximumUs.load()<<" us (mask measurement after actual updates)\n";
    report.field("DGA replay crowd check")<<replayCrowdTimings.calls.load()<<" calls; max "<<replayCrowdTimings.maximumUs.load()<<" us; >=4ms "<<replayCrowdTimings.over4ms.load()<<"; >=16ms "<<replayCrowdTimings.over16ms.load()<<"\n";
    report.field("Forced native crowd passes")<<forcedCrowdPasses.load()<<"; max "<<nativeCrowdTimings.maximumUs.load()<<" us (native + DGA visibility work); unchanged replay refreshes skipped "<<unchangedReplaySkips.load()<<"\n";
    report.field("Natural crowd update")<<naturalCrowdTimings.calls.load()<<" calls; max "<<naturalCrowdTimings.maximumUs.load()<<" us; >=4ms "<<naturalCrowdTimings.over4ms.load()<<"; >=16ms "<<naturalCrowdTimings.over16ms.load()<<" (native + DGA observer; not isolated DGA time)\n";
    report.field("DGA sound callback work")<<"mixer "<<audioMixerTimings.maximumUs.load()<<" us; intensity "<<intensityCallbackTimings.maximumUs.load()<<" us; cheers "<<reactionCallbackTimings.maximumUs.load()<<" us; >=4ms "<<audioMixerTimings.over4ms.load()<<"/"<<intensityCallbackTimings.over4ms.load()<<"/"<<reactionCallbackTimings.over4ms.load()<<" (maxima; forwarded native updates excluded)\n";
    report.field("DGA camera callback work")<<"view "<<cameraCallbackTimings.maximumUs.load()<<" us; native HFA "<<sparseCameraTimings.maximumUs.load()<<" us; cannon observer "<<cannonCallbackTimings.maximumUs.load()<<" us; >=4ms "<<cameraCallbackTimings.over4ms.load()<<"/"<<sparseCameraTimings.over4ms.load()<<"/"<<cannonCallbackTimings.over4ms.load()<<" (maxima; forwarded native updates excluded)\n";
    report.field("Audio context samples")<<audioContextSnapshots.load()<<"; native world checks "<<audioWorldSnapshots.load()<<" (disabled contexts avoid native reads)\n";
    report.field("DGA replay selector work")<<replaySelectorTimings.calls.load()<<" calls; max "<<replaySelectorTimings.maximumUs.load()<<" us; relevant major refreshes "<<replayMajorSelectorSamples.load()<<" (forwarded native Eval excluded)\n";
    report.field("Replay selector work intervals")<<replaySelectorIntervals.calls.load()<<" intervals; max "<<replaySelectorIntervals.maximumUs.load()<<" us; >=4ms "<<replaySelectorIntervals.over4ms.load()<<"; >=16ms "<<replaySelectorIntervals.over16ms.load()<<" (accumulated DGA work between presentation/replay callbacks; not FPS)\n";
    report.field("Native replay selector Eval")<<replayOriginalTimings.calls.load()<<" roots; max "<<replayOriginalTimings.maximumUs.load()<<" us; >=4ms "<<replayOriginalTimings.over4ms.load()<<"; >=16ms "<<replayOriginalTimings.over16ms.load()<<" (native Eval including nested DGA hooks)\n";
    report.field("Major-play replay observer")<<majorReplayTimings.calls.load()<<" calls; max "<<majorReplayTimings.maximumUs.load()<<" us; >=4ms "<<majorReplayTimings.over4ms.load()<<"; >=16ms "<<majorReplayTimings.over16ms.load()<<" (includes native play/mode queries)\n";
    report.field("Native replay event / node")<<replayEventNativeTimings.calls.load()<<" / "<<replayNodeNativeTimings.calls.load()<<" calls; max "<<replayEventNativeTimings.maximumUs.load()<<" / "<<replayNodeNativeTimings.maximumUs.load()<<" us (native calls including any nested DGA hooks)\n";
    {size_t pending=0;uint64_t dropped=0;{std::lock_guard<std::mutex> guard(logMutex);pending=queuedLogs.size();dropped=queuedLogs.dropped();}report.field("Deferred log queue")<<pending<<" pending; "<<dropped<<" dropped by bounded overflow\n";}
    report.field("Log write failures")<<logWriteFailures.load()<<" failed batches; inspect folder permissions/free space if nonzero\n";
    report.field("Settings file")<<dataFolder.u8string()<<"/DynamicGamedayAtmosphere.ini\n";
    report.field("Diagnostic preview")<<*std::atomic_load(&diagnosticStatus)<<"\n";
    report.field("Pause-menu fullness")<<(!pauseMenuAvailable?"unavailable on this game layout":!pauseMenuSetting?"disabled":"enabled; updates when opening the pause menu")<<"\n";
    report.field("In-game settings tab")<<(!settingsMenuAvailable?"unavailable":settingsMenu.ready()?"Gameday Atmosphere; after the native settings tabs":"waiting for a verified settings screen")<<"\n";
    report.field("Pause settings tab")<<(!pauseSettingsMenuAvailable?"unavailable on this layout":pauseSettingsMenu.ready()?"Gameday Atmosphere; right of Auto-Subs":"waiting for a verified pause settings screen")<<"\n";
    report.field("Pause-menu displays")<<pauseMenuDisplays.load()<<"\n";
    report.field("Mode")<<(mode==0?"automatic":mode==1?"manual test":"restored")<<"\n";
    report.field("Team IDs (home/away)")<<shownTeam0<<" / "<<shownTeam1<<"\n";
    auto turnoutPlan=std::atomic_load(&initialApplied);
    int localKickoff=shownKickoffMinutes.load();
    report.field("Local kickoff")<<(localKickoff<0?"unavailable":std::to_string(localKickoff/60)+":"+(localKickoff%60<10?"0":"")+std::to_string(localKickoff%60)+(localKickoff>=1140?"; night game":"; daytime kickoff"))<<"\n";
    auto audienceRead=std::atomic_load(&audienceCaptureStatus);
    bool audienceReadMatches=audienceRead->home==shownTeam0&&audienceRead->away==shownTeam1&&shownTeam0>=0;
    report.field("Audience foundations")<<(!audienceBindings||!audienceStorageReady?"unavailable; fresh turnout keeps EA attendance":
        audienceReadMatches?(audienceRead->okay?"last matching native audience read verified; see Starting attendance for applied plan":"last matching native audience read failed; EA turnout preserved for that launch"):
        "native field labels verified; awaiting a matching fresh launch read")<<"\n";
    report.field("Latest audience read")<<audienceRead->home<<" / "<<audienceRead->away<<"; "<<audienceRead->detail<<"\n";
    report.field("Starting attendance")<<(!initialSetting?"EA; revised turnout disabled":!initialBinding?"EA; revised-turnout binding unavailable":turnoutPlan->owns(shownTeam0,shownTeam1)?"revised regular-season turnout":"EA; postseason/non-regular, resumed, late attach, or waiting for fresh launch")<<"\n";
    if(turnoutPlan->owns(shownTeam0,shownTeam1))report.field("EA / revised planned attendance")<<turnoutPlan->launch.ea<<" / "<<turnoutPlan->result.attendance<<" fans; "<<turnoutPlan->result.percent<<"% of capacity before arrivals/departures\n";
    if(turnoutPlan->owns(shownTeam0,shownTeam1)&&turnoutPlan->result.showcaseEvent>=0)
        report.field("Showcase rivalry attendance")<<attendance::showcase::event(turnoutPlan->result.showcaseEvent).name<<"; native event "<<turnoutPlan->result.showcaseEvent<<"; "<<turnoutPlan->result.kickoff<<"% kickoff /100% peak; team slots do not select this rule\n";
    report.field("Ranks (home/away)")<<shownRank0<<" / "<<shownRank1<<" (game-supplied ranks; 0 means unavailable)\n";
    report.field("Rivalry")<<(shownRivalry?"yes":"no")<<"\n";
    report.field("Venue")<<(shownNeutral?"neutral":"home stadium")<<"\n";
    report.field("Weather")<<(shownPrecipitation==1?"rain":shownPrecipitation==2?"snow":"dry")<<"\n";
    report.field("Conference championship")<<(shownConference?"yes":!shownConferenceKnown?"unavailable":"no")<<"\n";
    report.field("Postseason attendance rules")<<(shownPlayoff?"active":"inactive")<<"\n";
    report.field("Native HFA active")<<(shownHfa?"yes":"no")<<" (gameplay unchanged)\n";

    report.section("VISIBLE ATTENDANCE");
    report.field("2020 Mode")<<(!emptyMode2020?"off":emptyCrowdActive?"active; empty stadium override":"on; waiting for a verified stadium scene")<<"\n";
    report.field("2020 collective crowd / away band / home band mute")<<(stadiumSilence.muted(0)?"on":"off")<<" / "<<(stadiumSilence.muted(1)?"on":"off")<<" / "<<(stadiumSilence.muted(2)?"on":"off")<<(!silenceBindingsReady?"; native mixer bindings unavailable":!stadiumSilence.healthy()?"; mute controller fault; retirement will retry":"")<<"\n";
    report.field("2020 drum cadence mute")<<(stadiumSilence.drumMuted()?"on":"off")<<"\n";
    report.field("2020 PA announcer mute")<<(stadiumSilence.paMuted()?"on":"off")<<"\n";
    report.field("2020 ambience / reverb return")<<stadiumSilence.ambienceDb()<<" / "<<stadiumSilence.spaceDb()<<" dB (native outdoor ambience / existing reverb return)\n";
    report.field("2020 individual detail gain")<<stadiumSilence.detailDb()<<" dB; native field/player audio remains available\n";
    report.field("Crowd target")<<(emptyCrowdActive?0:100-(std::max)(activeLoss.load(),arrivalMissing.load()))<<"% of original attendance\n";
    report.field("Crowd measured");
    if(visibilityMeasured)state<<100.0*static_cast<double>(shownVisibleFans.load())/static_cast<double>((std::max)(1ull,shownOriginalFans.load()))<<"% of original attendance\n";
    else state<<"waiting for renderer\n";
    report.field("Crowd renderer")<<(visibilityRetry?"waiting for crowd update":visibilityMeasured?"verified":"waiting for first measurement")<<"\n";
    report.field("Seating pattern")<<shownSeatPatterns<<" area-based crowd(s), "<<shownSeatFallbacks<<" scattered fallback(s)"<<(seatPatternPending?"; waiting for native seats":"")<<"\n";
    {unsigned band=0,required=0;std::set<A> seen;std::lock_guard<std::mutex> guard(crowdMutex);
     for(const auto& item:startingCrowds){const auto& b=item.second;if(!b.geometry||!seen.insert(b.layoutKey?static_cast<A>(b.layoutKey):b.geometry).second)continue;band+=b.agents.band;
      required+=static_cast<unsigned>(b.supporters.homeProtected+b.supporters.awayProtected);}
     report.field("Protected crowd groups")<<band<<" band positions; "<<required<<" required military/academy positions; native visibility/culling preserved, 2020 Mode still empties stands\n";}
    auto supporters=std::atomic_load(&visitorPopulation);
    report.field("Visiting fan strength")<<visitorCrowdInfluence.load()<<"% (100 = normal travel model; saved plans retained)\n";
    report.field("Visiting fan appearances")<<(dynamicAwayFans?"enabled":"disabled")<<"; native generation "<<(appearanceBinding?"verified":"unavailable")<<"; "<<appearanceChanged<<" prepared scene(s); latest target "<<100*appearanceShare<<"%; travel "<<(appearanceTravel==1?"city distance":appearanceTravel==2?"approximate state distance":appearanceTravel==3?"same-state local bonus":"unknown / neutral")<<"; "<<(appearanceMiles>=0?std::to_string(appearanceMiles.load())+" miles":"distance unavailable")<<"\n";
    report.field("Native appearance selectors")<<(appearanceRestoreFailed?"restoration guard failed; further outfit overrides stopped":"temporary only; restored after generation")<<"\n";
    {std::lock_guard<std::mutex> guard(visitorPlanMutex);report.field("Visiting fan plan history")<<visitorPlans.entries.size()<<" saved fixture(s); latest "<<(appearancePlanSource==2?"reused exact fixture plan":appearancePlanSource==1?"new verified demand plan":"waiting for crowd generation")<<"; "<<(visitorPlanWritable?"storage ready":"unreadable file preserved; in-memory only")<<"\n";}
    report.field("Remaining supporters")<<supporters->home<<" home / "<<supporters->away<<" away; "<<100*supporters->share()<<"% visitors; "<<(supporters->verified?"measured from owned fan selections":"waiting for verified fan groups")<<"\n";
    report.field("Protected fan budget")<<(sharedFanBudgetApplied?"shared total":"legacy / waiting for verified layouts")<<"; target "<<sharedFanTarget<<" spectators including "<<sharedFanProtected<<" protected spectators; bands excluded\n";
    report.field("Permanent departures")<<activeLoss<<"% of original crowd\n";
    report.field("Still arriving")<<shownArriving<<"% of original crowd\n";
    report.field("Halftime fans staying away")<<shownHalftimeConverted<<"% converted to permanent departures\n";
    report.field("Halftime seat break")<<shownHalftime<<"% temporarily away\n";
    report.field("Departure formula")<<(shownPlayoff?"postseason (blowouts only)":attendance::profileName(static_cast<attendance::Profile>(shownProfile.load())))<<"\n";
    report.field("Comeback protection")<<(shownComeback?"active":"inactive")<<"\n";
    report.field("Upset retention")<<(shownUpset?"active":"inactive")<<"\n";
    report.field("Super Sim crowd updates")<<(!superSimBinding?"unavailable":shownSuperSim?"offscreen simulation":"waiting for selection, break or simulation")<<"\n";
    report.field("Scenario attendance")<<(shownResumeSource==2?"restored from crowd history":shownResumeSource==1?"estimated at resumed start":"ordinary game")<<"\n";

    report.section("CROWD AUDIO - CURRENT LEVELS");
    report.field("Crowd audio")<<(!audioHookInstalled?"unavailable":!audioSetting||!config->enabled?"disabled":!attendanceMixer.healthy()?"stopped after a fault":"enabled")<<"\n";
    if(mapped)report.field("Audio stadium fullness")<<100*(std::min)(liveAudio.remaining,liveAudio.arrivalFraction)<<"% of mapped seating\n";
    else report.field("Audio stadium fullness")<<"measuring seats; added soundscape waiting\n";
    report.field("Native crowd volume setting")<<nativeCrowdSlider<<"% (-1 means not observed)\n";
    report.field("Native audio callbacks")<<nativeAudioCallbacks<<"; native bus presets "<<attendanceMixer.creations()<<" / "<<attendanceMixer.releases()<<" retired; mixer "<<(attendanceMixer.healthy()?"healthy":"stopped after fault")<<"\n";
    report.field("Crowd mix")<<"eight native buses; no graph-volume cuts or separate chant cut\n";
    const char* busNames[]={"Pass anticipation gain","Continuous Beds gain","Chants gain","Collective Reactions gain","Student section gain","Individual detail gain","Swells gain"};
    for(unsigned i=0;i<7;++i)report.field(busNames[i])<<attendanceMixer.netDb(i)<<" dB versus native\n";
    report.field("Empty crowd mute")<<(attendanceMixer.muted()?"active":"inactive")<<"\n";
    report.field("Vanilla crossover");if(mapped)state<<100*attendance::balance::crossover(density->seats)<<"% mapped fullness (size-dependent)\n";else state<<"waiting for verified mapped seats\n";
    report.field("Old excitement limiter")<<"removed; native excitement preserved, earned upward reactions retained\n";
    report.field("Legacy crowd parameter writes")<<"disabled\n";
    report.field("Mapped seating baseline")<<(mapped?std::to_string(density->original)+" / "+std::to_string(density->seats)+" occupied positions":"waiting for verified layout")<<"\n";
    report.field("Packed-house fullness needed");if(mapped)state<<100*attendance::packed::minimum(density->seats)<<"% for "<<density->seats<<" stored positions\n";else state<<"waiting for verified layout\n";
    report.field("Native attendance / capacity")<<(known?std::to_string(population->attendance)+" / "+std::to_string(population->capacity):"waiting for verified values")<<" (reference only; sound uses mapped seating)\n";

    report.section("ATMOSPHERE & CAMERA - LIVE EFFECTS");
    report.field("Opening-drive allowance")<<attendance::lineup::openingStatus(shownOpeningDrive.load())<<"\n";
    report.field("Play-call atmosphere")<<(!audio->enabled||!audio->lineups?"disabled":shownLineup?(shownPackedHouse?"packed-house buildup active":"active"):shownBigAtmosphere?"big game; waiting for lineup / enough fans":"waiting for a qualifying lineup / enough fans")<<"\n";
    report.field("Upset Watch sound")<<(!audio->enabled||!audio->lineups||!audio->upsetWatch?"disabled":attendance::upsetsound::name(shownUpsetStage))<<"; ";
    if(shownUpsetStage)state<<(shownUpsetSound?"qualified":"waiting for enough fans")<<"; needs "<<100*shownUpsetMinimum.load()<<"% actual fullness; ";
    state<<"separate from upset retention and packed house\n";
    report.field("Native excitement")<<crowdIntensity.native()<<" -> "<<crowdIntensity.output()<<" (game -> crowd soundscape; excitement, not volume %)\n";
    report.field("Situational reactions")<<(situationalReactionsEnabled?"enabled; home letdowns and away counterparts, including sound-only games":"disabled")<<"\n";
    report.field("During-play reactions")<<(!std::atomic_load(&liveReactionSettings)->enabled?"disabled":!liveReactionBindings?"unavailable":"enabled; sound-only below layered gate, motion on qualifying layered plays")<<"\n";
    report.field("Live forced-fumble detection")<<((liveFumbleBinding||contactFumbleBinding)?(contactFumbleBinding?"enabled; native cause event or contact/loose-ball/fumble proof":"enabled; fresh native forced-fumble event"):"unavailable; completed-play fallback retained")<<"\n";
    report.field("Third/fourth-down pass breakups")<<(passBreakupBinding?"verified native deflection plus completed stop; small shake and cheer":"unavailable; optional native checks differ")<<"\n";
    report.field("Hit-stick shake")<<"truck-strength on confirmed third/fourth-down stops or losses; other landed hits keep sound"<<"\n";
    report.field("Skill / hit / broken tackle accents")<<(!audio->skillMoves?"disabled":skillBindings?"verified native success events; regular broken tackles sound-only; existing fullness gates":"unavailable; optional native checks differ")<<"\n";
    report.field("Staggered major cheers")<<(audio->staggeredCheers?"one extra cheer after 0.9s; native cheer overlap allowed":"disabled")<<"\n";
    report.field("Packed marquee reactions")<<(audio->contextReactions?"enabled; packed big games/rivalries only":"disabled")<<"\n";
    report.field("Live long catches / picks")<<shownLiveCatches<<" / "<<shownLivePicks<<"\n";
    report.field("Drive escalation")<<100*shownDrive.load()<<"% (excitement, no separate volume bonus)\n";
    report.field("Native sack/fumble checks")<<(defensiveReactionBindings?"verified":"unavailable; measured five-yard loss fallback independent")<<"\n";
    report.field("Blocked-field-goal detection")<<(fieldGoalBlockBinding?"verified native block flag":"unavailable; normal missed kicks do not count as blocks")<<"\n";
    report.field("Native HFA camera")<<(!sparseCameraAvailable?"filter unavailable":!sparseNativeCamera.healthy()?"filter stopped after a fault":nativeShakeMinimum==0?"unchanged (INI opt-out)":sparseNativeCamera.active()?"shake withheld below admission or while DGA owns motion":"shared atmosphere admission ready")<<" (gameplay HFA unchanged)\n";
    report.field("Cannon sound observer")<<(cannonAvailable?"ready":"unavailable")<<"; confirmed native blasts "<<cannonStarts.load()<<"\n";
    report.field("Added camera motion")<<(!audio->cameraShake||!audio->enabled?"disabled":!cameraAvailable?"unavailable":!presentationCamera.healthy()?"stopped after a fault":presentationCamera.playing(now)?attendance::motion::name(presentationCamera.kind()):"inactive")<<"\n";
    report.field("Away reaction camera output")<<presentationCamera.visitorChanged()<<" camera outputs changed; last "<<presentationCamera.visitorPercent()<<"%, peak "<<presentationCamera.visitorPeak()<<"% after view/camera scaling (runtime totals; requests alone are not applied output)\n";
    {auto venue=std::atomic_load(&renderedOccupancy);report.field("Crowd camera venue factor")<<attendance::motion::occupiedScale(venue->owns(audioMatch.load(),audioEpoch.load())?venue->seats:0,venue->peak()*(100-(std::max)(activeLoss.load(),arrivalMissing.load()))*.01f)<<"x from occupied seats; neutral crowd adds "<<attendance::motion::neutralScale(*motion)<<"x; native cannon impact is separate\n";}
    {auto venue=std::atomic_load(&renderedOccupancy);float full=venue->owns(audioMatch.load(),audioEpoch.load())?venue->peak()*(100-(std::max)(activeLoss.load(),arrivalMissing.load()))*.01f:0;
        report.field("Occupied reaction crowd")<<static_cast<uint64_t>(std::llround(attendance::population::occupied(venue->seats,full)))<<" mapped seats; added audio tier "<<attendance::population::size(venue->seats,full)<<" of 3; native reaction peaks preserved\n";
        auto dense=attendance::dense::layers(venue->seats,full);
        report.field("Dense-crowd layer support")<<dense.full*100<<"% medium tier; "<<dense.veryLarge*100<<"% very-large tier; up to "<<dense.multiplier()<<"x added layer intensity within native cap; existing admission required, no extra bus gain\n";}
    {auto cue=std::atomic_load(&cameraCue);if(cue->smoothRunBuildup)report.field("Run/YAC strength ramp")<<cue->buildupStrength<<"% reached before view/envelope; 800ms full-range rise; "<<(cue->tail?"fading from reached strength":"following 11-15 yard target")<<"\n";}
    report.field("Wide player-camera compensation")<<(std::atomic_load(&cameraSettings)->widePlayerCompensation?"extended wide-angle allowance up to 6x":"automatic view normalization up to 4x")<<"; close-ups normalized; neither setting changes FOV\n";
    report.field("Crowd shake scaling")<<"linear occupied seats: 0.80x at6060 to1.25x at61888; per-axis projection normalization; common reference 1.10x; framing 0.675x broad to 1.05x tight; distinct overlaps capped15% above strongest\n";
    {auto selected=attendance::gamecamera::sample(image,audioMatch.load(),read);auto cameraConfig=std::atomic_load(&cameraSettings);
        report.field("Gameplay camera preset")<<attendance::gamecamera::name(selected.active)<<"; offense "<<attendance::gamecamera::name(selected.offense)<<" / defense "<<attendance::gamecamera::name(selected.defense)<<"; configured scale "<<attendance::gamecamera::scale(cameraConfig->cameraPercent,selected.active)<<"x\n";}
    if(presentationCamera.playing(now)){
        report.field("Current camera strength")<<std::setprecision(3)<<presentationCamera.unscaled()<<"% envelope; pitch/yaw compensation "<<presentationCamera.zoomScale()<<"/"<<presentationCamera.yawScale()<<"x; framing "<<presentationCamera.framingScale()<<"x; camera setting "<<presentationCamera.presetScale()<<"x; vertical angular output "<<presentationCamera.percent()<<"%\n"<<std::setprecision(1);
        report.field("Camera screen movement estimate")<<std::setprecision(3)<<presentationCamera.screenPitch()<<"% frame height / "<<presentationCamera.screenYaw()<<"% frame width peak; vertical FOV "<<presentationCamera.fov()<<" degrees; output view 0x"<<std::hex<<presentationCamera.view()<<std::dec<<"; latest written output, not rendered-motion proof\n"<<std::setprecision(1);
    }
    report.field("Camera projection health")<<presentationCamera.cached()<<" brief cached outputs; "<<presentationCamera.invalid()<<" untrusted outputs skipped; "<<presentationCamera.clipped()<<" extreme wide outputs bounded (runtime totals)\n";
    auto presentationIntent=std::atomic_load(&sustainedIntent);
    report.field("Steady background roar")<<(presentationIntent->active&&presentationIntent->reason==attendance::sustained::Reason::Background?"requested; shares native sustained channel with reactions":"inactive or higher-priority reaction owns the channel")<<"; above60% fullness /10000 occupied mapped seats\n";
    report.field("Presentation atmosphere")<<(presentationIntent->active&&presentationIntent->reason==attendance::sustained::Reason::Opening?"pregame buildup":presentationIntent->active&&presentationIntent->reason==attendance::sustained::Reason::Postgame?"postgame stadium atmosphere":"inactive")<<"\n";
    report.field("Added crowd roar")<<(!audio->enabled||(!audio->opening&&!audio->overtime&&!audio->eruptions&&!audio->lineups)?"disabled":!swellAvailable?"unavailable":!sustainedAudio.healthy()?"stopped after a fault":sustainedAudio.playing()?"playing":"inactive")<<"\n";
    report.field("Supporting gameplay swell")<<(supportingSwell.playing()?"playing":"inactive")<<"\n";
    report.field("Successful-play reaction")<<(atmosphere.active&&atmosphere.reason==attendance::sustained::Reason::Result?(atmosphere.reaction?"settling":"active"):"inactive")<<"\n";
    report.field("Crowd eruption")<<((shownEruption&&now<shownDecisiveUntil.load())?"active":"inactive")<<"\n";
    report.field("Postgame celebration carry")<<(shownPostgameCarry?"eligible / retained; actual sound and camera states shown above":"inactive")<<"\n";

    report.section("REPLAYS");
    report.field("Replay crowd")<<(replayLoss>=0?std::to_string(100-replayLoss.load())+(replaySuspendsAtmosphere?"% (interactive; live target preserved)":"% (automatic; live target preserved)"):"inactive")<<"\n";
    report.field("Major-play replay sequence")<<(!majorReplayBindings?"unavailable on this native layout":majorReplayFailed?"stopped after a fault":!extraReplays||!bigPlayReplays?"native flow; full-screen additions disabled":!routineInsets?"full-screen additions; added insets disabled":replayMode==attendance::replaymode::Mode::RealtimeSim?"Slow Sim: native full-screen sequence, no added inset":replayMode!=attendance::replaymode::Mode::PlayCall?"native full-screen sequence; play-call destination not verified":"full native sequence, then separate inset repeat; live verification required")<<"\n";
    report.field("Extra post-play replays")<<(!replayPresentationInstalled?"unavailable":replayPresentationFailed?"stopped after a fault":!extraReplays?"disabled":"enabled")<<"\n";
    report.field("Touchdown replay additions")<<"excluded; native presentation retained"<<"\n";
    report.field("Routine insets")<<(routineInsets?"on":"off")<<"\n";
    report.field("Full-screen gains")<<(bigPlayReplays?std::to_string(replayMinimumYards.load())+"+ yards":"off")<<"\n";
    report.field("Defensive highlights")<<(defensiveReplays?"on":"off")<<"\n";

    report.section("DYNAMIC GAMEDAY ATTENDANCE - SETTINGS");
    report.field("Attendance enabled")<<(config->enabled?"yes":"no")<<"\n";
    report.field("First-quarter arrivals")<<(arrivalEnabled?"enabled":"disabled")<<"\n";
    report.field("Opening attendance policy")<<"regular 50-65% of peak ordinarily; severe-late starts 45%; weather-only floor 35%; usual marquee ceiling70%, exceptional cap80%; named rivalry events can start70-95%; small-venue access adds up to8 kickoff points; same-state adds2. Postseason 70-80%; championships 80-90%.\n";
    report.field("Fallback opening baselines")<<"normal "<<arrivalStart<<"% / severe "<<arrivalLateStart<<"% of planned crowd\n";
    report.field("Attendance profile")<<(attendanceProfile==0?"Fuller Crowds":"Default")<<" (peak changes on fresh regular-season Dynasty launches; existing crowd plans retained)\n";
    report.field("Arrival finish points")<<"normal 50% / marquee 30% / severe 75% / postseason 20% through Q1 when revised inputs qualify; protected rivalry fallback 30%; other fallback normal "<<arrivalFinish<<"%, severe "<<arrivalLateFinish<<"%\n";
    report.field("Departure strength")<<config->formulaStrength<<"% of normal formula\n";
    report.field("Quarter departure cap")<<config->quarterCap<<" percentage points (ordinary games)\n";
    report.field("Departure capacity floor")<<"15% of owned mapped seating; kickoff arrivals unchanged; no refill of existing empty seats\n";
    report.field("Rainy home-win departures")<<"last half of Q4; 22+ lead, full severity at 35+; up to 10 extra quarter points; shared floors preserved\n";
    report.field("Late blowout extra quarter allowance")<<config->lateTrafficQuarterExtra<<" points maximum; 28+ home deficit; gentle last-half start / stronger last third of Q4; floors/postseason limits preserved\n";
    attendance::State trafficState;int trafficCap=0,trafficLimit=0,trafficLoss=0;
    {std::lock_guard<std::mutex> guard(historyMutex);trafficState=departureModel.previous;if(attendance::valid(trafficState)){trafficCap=departureModel.quarterLimit(trafficState,attendance::elapsedGameTime(trafficState),config->quarterCap);trafficLimit=(std::min)(attendance::lossCeiling(trafficState),departureModel.periodStartLoss+trafficCap);trafficLoss=departureModel.committedLoss;}}
    report.field("Late traffic check");
    if(config->enabled&&config->formulaStrength>0&&config->lateTraffic&&mode==0&&attendance::valid(trafficState)&&attendance::traffic::lateHomeGame(trafficState)&&trafficState.awayScore-trafficState.homeScore>=17){
        state<<"Q4 allowance "<<trafficCap<<" points; minimum crowd allowed now "<<100-trafficLimit<<"% of original; ";
        if(trafficLoss>=attendance::lossCeiling(trafficState))state<<"crowd floor reached";
        else if(trafficLoss>=trafficLimit)state<<"quarter limit reached";
        else if(shownComeback)state<<"comeback protection still applies";
        else state<<"exits accrue with game time; waiting targets use safe breaks";
        state<<"\n";
    }else state<<"inactive / outside a late home blowout\n";
    report.field("Safe-break step size")<<config->transitionCap<<" points before wait-time adjustments\n";
    report.field("Halftime seat-break strength")<<halftimeExtra<<"% of normal temporary break\n";

    report.section("DYNAMIC GAMEDAY SOUNDSCAPE - SETTINGS");
    report.field("Atmosphere layers")<<(audio->enabled?"enabled":"disabled")<<"\n";
    report.field("Crowd bus gain limit")<<"+"<<busMaximumBoost<<"% reaction reference; dense Beds/Chants up to "<<4.f*busMaximumBoost.load()/30.f<<" dB; no separate event gain stacked\n";
    report.field("Chant policy")<<"same gain as Beds; separate legacy chant value ignored\n";
    attendance::State gate{shownQuarter.load(),shownScore0.load(),shownScore1.load(),shownPhase.load(),shownSeconds.load(),900};
    gate.rank0=shownRank0;gate.rank1=shownRank1;gate.rivalry=shownRivalry;gate.playoff=shownPlayoff;gate.conferenceChampionship=shownConference;gate.bowl=shownBowl;gate.night=localKickoff>=1140;
    float gateQuarter=at<float>(at<A>(at<A>(image+attendance::bindings::ClockManager)+0x28)+0x2c);if(gateQuarter>0)gate.quarterSeconds=gateQuarter;
    applySessionPlan(gate,liveAudio.match);
    report.field("Night atmosphere adjustment")<<(gate.night&&attendance::regularSeason(gate)?"10% lower admission thresholds; absolute 50% floor; packed classification unchanged":"inactive")<<"\n";
    report.field("Departure engagement pressure")<<attendance::context::departureEngagement(gate,attendance::elapsedGameTime(gate))<<"x (second-half night/scoring pace; existing floors/protection)\n";
    report.field("Big-game required fullness");if(mapped)state<<100*attendance::lineup::bigRequirement(*audio,gate,density->seats)<<"% (day size gate "<<100*attendance::lineup::bigMinimum(*audio,density->seats)<<"%)\n";else state<<"waiting for verified layout\n";
    report.field("Critical / goal-line requirement");if(mapped)state<<100*attendance::nightRequirement(gate,attendance::packed::standardMinimum(density->seats))<<"%\n";else state<<"waiting for verified layout\n";
    report.field("OT / late-drive requirement");if(mapped)state<<100*attendance::nightRequirement(gate,attendance::packed::specialMinimum(density->seats))<<"%\n";else state<<"waiting for verified layout\n";
    report.field("Audio density policy")<<"occupied seats plus fullness; dense Beds/Chants +3.5 to +4 dB at stock; reaction cap +30%; zero crowd mute\n";
    report.field("Play-call / packed / drive")<<(audio->lineups?"on":"off")<<" / "<<(audio->packedHouse?"on":"off")<<" / "<<(audio->driveEscalation?"on":"off")<<"\n";
    report.field("Last verified reaction distance")<<shownReactionDistance<<" yards; no distance-based fade extension\n";
    report.field("Opening / OT / eruptions")<<(audio->opening?"on":"off")<<" / "<<(audio->overtime?"on":"off")<<" / "<<(audio->eruptions?"on":"off")<<"\n";
    report.section("DYNAMIC GAMEDAY BROADCAST - CAMERA");
    A nativeCameraSettings=at<A>(image+attendance::bindings::CameraSettings);
    bool cameraOptionsReady=cameraBindings&&at<A>(nativeCameraSettings)==image+attendance::bindings::CameraSettingsVtable;
    report.field("Game Dynamic Camera Effects")<<(!cameraOptionsReady?"unavailable":at<unsigned short>(nativeCameraSettings+0x3e6)&&at<unsigned short>(nativeCameraSettings+0x41a)?"on":"off; all added camera motion blocked")<<"\n";
    report.field("Play-call profile")<<"40 seconds including its fade\n";
    report.field("Long catch minimum")<<std::atomic_load(&liveReactionSettings)->minimumCatchYards<<" yards at the catch\n";
    report.field("Long run / catch-and-run minimum")<<std::atomic_load(&liveReactionSettings)->minimumGainYards<<" total yards for cheer; shake builds at 11-15 total yards; live position "<<(runnerPositionBinding?"verified":"unavailable")<<"\n";
    report.field("Pass-in-flight anticipation")<<shownPassGate.load()<<"; verified flights this match "<<shownPassFlights<<"\n";
    report.field("Airborne-pass supporting swell")<<(supportingSwell.playing()&&supportingSwell.reason()==attendance::sustained::Reason::PassFlight?"playing; 0.5-second attack":"inactive")<<"\n";
    report.field("Native shake requires")<<"DGA crowd/context admission plus "<<nativeShakeMinimum<<"% minimum fullness (0 disables the native filter)\n";
    report.field("Camera shake enabled")<<(audio->cameraShake?"yes":"no")<<"\n";
    report.field("Away team reaction shake")<<(motion->awayPercent>0?"on; strength follows remaining visitors":"off; visiting crowd sounds remain")<<"; home venues only, normal crowd gates apply\n";
    report.field("Ordinary buildup strength")<<motion->ordinary*.5f<<"-"<<motion->ordinary<<"% chosen once per lineup\n";
    report.field("Critical-down strength")<<motion->critical*.75f<<"-"<<motion->critical<<"% before home-offense 0.75x scaling (third-and-10 or shorter; offensive fourth downs)\n";
    report.field("Punt buildup motion")<<"0.5x ordinary strength; return reactions use their own strength\n";
    report.field("Special lineup cheer strength")<<motion->lineupCheer*2.f/3.f<<"-"<<motion->lineupCheer<<"%; immediate cheer opportunities only\n";
    report.field("Hostile opening series")<<(shownHostileSeries?"active; ends on visitor first down, punt or possession/score change":"inactive")<<"\n";
    report.field("Opening-possession surge")<<(shownHostileOpening?"active":"inactive");if(shownNeutral)state<<"; each team's first offensive play, "<<motion->neutralPercent<<"%-strength crowd motion\n";else state<<"; away must have the first offensive snap\n";
    report.field("Hostile opening shake ceiling")<<motion->hostileOpeningMaximum<<"%; stable 80-100% opening range, then occupied-seat scaling\n";
    report.field("Cannon shake strength")<<motion->cannon<<"%; 0.9-second impact; all nine native cannon variants; no fullness requirement\n";
    report.field("Neutral-site crowd shake")<<motion->neutralPercent<<"% of configured strength for either team; native cannon strength independent\n";
    report.field("Live-play shake strength")<<motion->live<<"%; "<<motion->reactionFadeMs*.001f<<"-second post-play fade\n";
    report.field("Added reaction shape")<<"quick hit, short hold, smooth fade; existing strength/volume limits\n";
    report.field("Immediate long-catch impact")<<motion->catchImpact<<"%; separate from runs/returns and defensive impacts\n";
    report.field("Short-impact duration")<<"2-3 seconds at stock settings; per-event profile plus the short-impact duration adjustment\n";
    report.field("Touchdown strength")<<motion->touchdown<<"%\n";
    report.field("Eruption strength")<<motion->eruption<<"%\n";
    report.field("TD / eruption opening impact")<<motion->touchdownImpact<<"% / "<<motion->eruptionImpact<<"%\n";
    report.field("TD / eruption duration")<<motion->touchdownMs/1000<<"s / "<<motion->eruptionMs/1000<<"s\n";
    report.field("Safety shake duration")<<attendance::motion::safetyDuration(*motion)/1000.0<<"s (70% of touchdown duration)\n";
    report.section("SETTINGS - LOGGING");
    report.field("Log size limit")<<logLimitMB<<" MB per event log; readable and technical status refresh once per second\n";

    if(debugLog){
        report.section("DEBUG - TECHNICAL COUNTERS");
        report.field("Callbacks / replay callbacks")<<ticks<<" / "<<replayCallbacks<<"\n";
        report.field("Renderer calls")<<shownVisibilityCalls<<"\n";
        report.field("Camera output changes")<<presentationCamera.changed()<<"\n";
        report.field("Native HFA filter / restore")<<sparseNativeCamera.filtered()<<" intercepted events / "<<sparseNativeCamera.restores()<<" restorations\n";
        report.field("Roar starts / stops")<<sustainedAudio.starts()<<" / "<<sustainedAudio.stops()<<"\n";
        report.field("Native cheers requested")<<positiveCheer.sent()<<" (binding "<<(cheerAvailable&&positiveCheer.healthy()?"ready":"unavailable")<<")\n";
        report.field("Replay eligibility changes")<<insetEligibilityChanges<<" / "<<fullEligibilityChanges<<" / "<<defensiveRouteChanges<<" (inset / full / defense; not replay totals)\n";
    }
}
void readDiagnosticRequest(uint64_t now){
    auto directory=dataFolder/L"Diagnostics";
    auto source=directory/(L"request-"+std::to_wstring(GetCurrentProcessId())+L".txt"),processing=directory/(L"request-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(now)+L".pending");
    std::error_code error;
    if(!std::filesystem::exists(source,error)||error)return;
    std::filesystem::rename(source,processing,error);if(error)return;
    auto size=std::filesystem::file_size(processing,error);std::string text;
    if(!error&&size<=256){std::ifstream file(processing);std::getline(file,text);}
    std::filesystem::remove(processing,error);
    FILETIME raw{};GetSystemTimeAsFileTime(&raw);ULARGE_INTEGER clock{};clock.LowPart=raw.dwLowDateTime;clock.HighPart=raw.dwHighDateTime;
    attendance::diagnostic::Request command;
    if(!attendance::diagnostic::parse(text,GetCurrentProcessId(),clock.QuadPart/10000-11644473600000ull,command)){
        log("diagnostics","Ignored invalid, stale or wrong-process test request.");return;
    }
    command.deadline=now+1500;
    std::shared_ptr<const attendance::diagnostic::Request> next=std::make_shared<attendance::diagnostic::Request>(command);std::atomic_store(&diagnosticRequest,next);
    log("diagnostics",std::string("Requested ")+attendance::diagnostic::name(command.preset)+" preview; maximum "+std::to_string(command.seconds)+" seconds, current sound limits retained.");
}
void writeActivationStatus(const char* activation,const char* reason){
    if(!attendance::ensureFolder(logsFolder))return;
    std::ostringstream text;text<<"Gameday Atmosphere " GAMEDAY_VERSION "\nGame process ID: "<<GetCurrentProcessId()<<"\nActivation: "<<activation<<"\nReason: "<<reason<<"\nSee technical.log and launcher.log for startup details.\n";
    for(const auto* name:{L"status.txt",L"technical-status.txt"}){std::ofstream file(logsFolder/name,std::ios::trunc);file<<text.str();}
}
DWORD WINAPI worker(void*) {
    struct FlushOnExit {~FlushOnExit(){for(unsigned i=0;i<8;++i)drainLogs();}} flushOnExit;
    std::vector<wchar_t> path(32768);DWORD pathLength=GetModuleFileNameW(self,path.data(),static_cast<DWORD>(path.size()));if(!pathLength||pathLength>=path.size())return 1;
    attendance::runtimepaths::Layout paths;
    if(!attendance::runtimepaths::resolve(std::filesystem::path(std::wstring(path.data(),pathLength)),paths)){OutputDebugStringW(L"DGA: unsupported runtime folder; no hooks installed.\n");return 1;}
    folder=paths.runtime;dataFolder=paths.data;pluginsFolder=paths.plugins;logsFolder=dataFolder/L"Logs";image=reinterpret_cast<A>(GetModuleHandleW(nullptr));
    lastApplied=0;shownCurrent=shownTarget=0;
    attendance::ensureFolder(logsFolder);attendance::prepareReadableLogs(logsFolder);
    writeActivationStatus("STARTING","Checking runtime ownership and native bindings.");
    readSettings();log("starting","Gameday Atmosphere " GAMEDAY_VERSION "; attendance, retention and crowd audio.");
    if(GetPrivateProfileIntW(L"General",L"ConfigVersion",GAMEDAY_SCHEMA,(dataFolder/L"DynamicGamedayAtmosphere.ini").c_str())>GAMEDAY_SCHEMA){
        log("inactive","Settings belong to a newer version. Update the plugin before using this configuration.");writeActivationStatus("INACTIVE","Settings belong to a newer plugin version.");return 1;
    }
    HANDLE modules=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,GetCurrentProcessId());
    if(modules==INVALID_HANDLE_VALUE){log("inactive","Could not check for another atmosphere runtime; no hooks installed.");writeActivationStatus("INACTIVE","Could not check runtime ownership; no hooks installed.");return 1;}
    MODULEENTRY32W module{};module.dwSize=sizeof(module);bool conflict=false;
    if(Module32FirstW(modules,&module))do{
        if(module.hModule==self)continue;
        const std::wstring name=module.szModule;
        if(name.find(L"DynamicAttendanceNative")==0||name.find(L"DynamicAttendanceProbe")==0||name.find(L"DynamicGamedayAtmosphereNative")==0)conflict=true;
    }while(Module32NextW(modules,&module));
    CloseHandle(modules);
    if(conflict){log("inactive","An older attendance/atmosphere runtime is already loaded. Restart both MMC and the game; no new hooks installed.");writeActivationStatus("INACTIVE","Older runtime already loaded; restart MMC and the game.");return 1;}
    const std::wstring mutexName=L"Local\\Ace.DynamicGamedayAtmosphere.Game."+std::to_wstring(GetCurrentProcessId());
    HANDLE lifetime=CreateMutexW(nullptr,FALSE,mutexName.c_str());
    if(!lifetime||GetLastError()==ERROR_ALREADY_EXISTS){if(lifetime)CloseHandle(lifetime);log("inactive","Another atmosphere runtime already owns this game.");writeActivationStatus("INACTIVE","Another atmosphere runtime already owns this game.");return 1;}
    loadAttendanceHistory();loadAudienceFoundations();loadVisitorPlans();audienceBindings=attendance::audiencenative::bindingsReady(image);readResumeStatus(GetTickCount64());
    if(!install()){
        const char* reason=signatures?"No compatible crowd entities or hook installation failed":"Required native bytes differ; no hooks installed";
        log("inactive",reason);
        writeActivationStatus("INACTIVE",reason);
        return 1;
    }
    log("ready","Ready. Regular-season starting turnout and local night detection ready when native launch inputs qualify; other games retain EA turnout. First-quarter arrivals fill toward that planned crowd. Formula-based departures settle between plays and at breaks, with the largest exit window at halftime.");
    log(playBindingsAvailable?"ready":"audio-inactive",playBindingsAvailable?"Attendance-based audio and layered atmosphere ready. Native bus mix uses fullness and occupied seats; dense Beds/Chants up to +4 dB; reactions capped at +30%; no extra chant cut or native excitement limiter.":"Play data unavailable; winning eruptions and late-score departures are inactive.");
    if(replayPresentationInstalled&&extraReplays)log("ready","Extra post-play replays ready: routine run/pass insets and full-screen big-play sequences at "+std::to_string(replayMinimumYards.load())+" yards; defensive highlights "+(defensiveReplays?"enabled":"disabled")+". Normal replay playback and skipping remain active.");
    unsigned long long last=0,lastScan=0,lastPauseDisplays=0;int lastPauseValue=-2;bool replayFaultNoted=false;
    while(enabled){
        initialModeProof();
        // UI/loading may suspend presentation callbacks while quitting a game.
        // Remember that exit even if the next game reuses the same match object.
        observeWorldExit();
        std::string command;{std::ifstream f(dataFolder/L"Diagnostics"/L"legacy-control.txt");std::getline(f,command);}
        if(!command.empty()){
            {std::ofstream clear(dataFolder/L"Diagnostics"/L"legacy-control.txt",std::ios::trunc);}
            if(command=="observe")request=1;
            else if(command=="probe")request=2;
            else if(command=="empty")request=4;
            else if(command=="auto")request=5;
            else if(command=="restore"||command=="stop"){emptyMode2020=false;emptyModePending=true;request=3;}
            log("requested",command);
        }
        auto now=GetTickCount64();readDiagnosticRequest(now);if(now-last>1000){
            auto displays=pauseMenuDisplays.load();int value=pauseMenuLastPercent.load();
            if(displays!=lastPauseDisplays){
                if(value!=lastPauseValue)log("pause-menu",value>=0?"Displayed stadium fullness: "+std::to_string(value)+"%.":"Displayed stadium fullness: measurement unavailable.");
                lastPauseDisplays=displays;lastPauseValue=value;
            }
            if(replayPresentationFailed&&!replayFaultNoted){log("fault","Extra post-play replays stopped after a write fault; normal game presentation remains active.");replayFaultNoted=true;}
            {static bool noted=false;if(appearanceRestoreFailed&&!noted){log("away-crowd","Temporary appearance selectors could not be restored to their verified stream. Further outfit overrides stopped; native generation remains active.");noted=true;}}
            readResumeStatus(now);saveAttendanceHistory();saveAudienceFoundations();saveVisitorPlans();
            readSettings();
            if(attendance::ensureFolder(logsFolder)){
                std::ostringstream technical;writeStatus(technical,now);auto full=technical.str();
                {std::ofstream state(logsFolder/L"technical-status.txt",std::ios::trunc);state<<full;}
                {std::ofstream state(logsFolder/L"status.txt",std::ios::trunc);state<<attendance::readable::status(full);}
            }
            last=now;
        }
        if(now-lastScan>5000){
            bool needs=refreshEntities.exchange(false);{std::lock_guard<std::mutex> guard(entityMutex);if(entities.empty())needs=true;for(A e:entities)if(!entityValid(e))needs=true;}
            attendance::State s;A match=0;if(needs&&gameState(s,match))findEntities();lastScan=now;
        }
        drainLogs();
        Sleep(100);
    }
    return 0;
}
}
extern "C" void CrowdTick() {
    flushReplaySelectorTiming();
    ++ticks;
    if(!enabled)return;
    std::unique_lock<std::mutex> frame(presentationMutex,std::try_to_lock);if(!frame.owns_lock())return;
    if(!guardedEmptyService()){log("fault","2020 Mode could not update its crowd safely.");return;}
    bool replay=guardedReplayPresentation();
    guardedMajorReplay(true);
    if(!replay&&!guardedDecision()){enabled=false;log("fault","Attendance callback failed; automatic updates stopped");}
    if(enabled&&!replay&&!guardedRecordAttendance())log("history","Crowd snapshot unavailable; existing history preserved.");
    if(enabled&&!guardedAtmosphereTick()){playBindingsAvailable=false;crowdBoost=0;log("audio-inactive","Play data became unavailable; winning-event updates stopped. Quarter departures remain active.");}
}
extern "C" __declspec(dllexport) DWORD WINAPI DynamicGamedayAtmosphereStop(void*){emptyMode2020=false;emptyModePending=true;request=3;return 0;}
BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,LPVOID) {
    if(reason==DLL_PROCESS_ATTACH){self=instance;DisableThreadLibraryCalls(instance);HANDLE t=CreateThread(nullptr,0,worker,nullptr,0,nullptr);if(t)CloseHandle(t);}
    return TRUE;
}
