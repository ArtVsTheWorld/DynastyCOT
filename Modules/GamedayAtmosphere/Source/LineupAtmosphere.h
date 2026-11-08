#pragma once
#include "CoreAtmosphere.h"
#include "AtmosphereMix.h"
#include "ReactionVariety.h"
#include "DriveEscalation.h"
#include "AttendanceContext.h"
#include "PackedHouse.h"
#include "OpeningDrive.h"
#include "UpsetAtmosphere.h"
#include "DenseCrowd.h"
#include "ShowcaseAttendance.h"
namespace attendance::lineup {
inline constexpr uint64_t SnapTailMs=10000;
inline constexpr uint64_t LiveCarryMs=45000,PostPlayHoldMs=4000;
// Share matchup identity with attendance patience; fullness and situation
// admission remain specific to the effect, not automatic big-game privileges.
inline bool bigGame(const State& s){return marqueeMatchup(s);}
inline float bigMinimum(const core::Settings& c,uint64_t positions){
    float extra=std::isfinite(c.bigGameMinimum)?(std::clamp)(c.bigGameMinimum,.55f,.90f):.55f;
    return (std::max)(packed::standardMinimum(positions),extra);
}
inline bool kickoffMatchup(const State& s){return bigGame(s)||(s.rivalry&&static_cast<bool>(showcase::event(s.rivalryEvent)));}
inline float openingGameRequirement(const core::Settings& c,const State& s,uint64_t positions,bool identity){
    float normal=nightRequirement(s,bigMinimum(c,positions));
    if(positions<512||positions>12800000||!c.opening||!identity||!valid(s))return normal;
    // Arrivals must not silence the opening of an actual marquee matchup.
    // Keep the absolute 50% visual-density safeguard; ease back into the
    // stadium-size requirement as opening energy settles through Q1.
    float openingWeight=s.phase==6?1.f:s.quarter==1?1-(std::clamp)((1-s.seconds/s.quarterSeconds-.10f)/.65f,0.f,1.f):0.f;
    return normal+(core::ReactionMinimumFullness-normal)*openingWeight;
}
inline float bigRequirement(const core::Settings& c,const State& s,uint64_t positions){return openingGameRequirement(c,s,positions,bigGame(s));}
inline float kickoffRequirement(const core::Settings& c,const State& s,uint64_t positions){return openingGameRequirement(c,s,positions,kickoffMatchup(s));}
inline bool openingDiscount(const State& s,uint64_t positions){return bigGame(s)||s.rivalry||(positions>=13000&&positions<=12800000);}
inline float openingMinimum(const State& s,uint64_t positions){return nightRequirement(s,openingDiscount(s,positions)&&expectedPeak(s)>=.90f?(std::min)(.55f,packed::standardMinimum(positions)):packed::standardMinimum(positions));}
enum class OpeningAdmission {Inactive,Normal,Discounted};
inline OpeningAdmission openingAdmission(bool qualified,const State& s,uint64_t positions){
    if(!qualified)return OpeningAdmission::Inactive;
    return openingMinimum(s,positions)<nightRequirement(s,packed::standardMinimum(positions))-.00001f?OpeningAdmission::Discounted:OpeningAdmission::Normal;
}
inline const char* openingStatus(OpeningAdmission admission){
    switch(admission){
    case OpeningAdmission::Normal:return "qualified at normal size/night requirement; no special opening discount";
    case OpeningAdmission::Discounted:return "qualified with special opening discount; 90%+ expected peak and big game, rivalry or 13,000+ stored positions";
    default:return "inactive / first possession ended, disabled or below requirement";
    }
}
inline bool cameraVenue(const core::Settings& c,const State& s,float fraction,bool fullnessKnown,uint64_t positions=0){
    return fullnessKnown&&positions>=512&&std::isfinite(fraction)&&((bigGame(s)&&fraction>=bigRequirement(c,s,positions))||(c.packedHouse&&fraction>=nightRequirement(s,packed::minimum(positions))));
}
inline bool offensivePlay(int selectedType){return selectedType>0&&!((selectedType>=21&&selectedType<=25)||selectedType==105||selectedType==106||selectedType==154||selectedType==155||energy::clockManagementType(selectedType));}
inline bool criticalCamera(const energy::Play& p,int selectedType){
    if(!energy::normalDown(p))return false;
    if(p.down==3)return std::isfinite(p.toFirst)&&p.toFirst>=0&&p.toFirst<=10.01f;
    if(p.down==4)return offensivePlay(selectedType);
    return true;
}
inline bool shakePlay(const energy::Play& p,int type,bool openingKickoff){
    if(energy::clockManagementType(type))return false;
    if(p.down==-1)return false; // PATs and two-point tries.
    if(type==23||type==24)return openingKickoff;
    if(type==25||p.down==0)return false;
    bool kick=type==21||type==22||type==105||type==106||type==154||type==155;
    if(kick)return energy::normalDown(p);
    return energy::normalDown(p)&&(p.down!=4||criticalCamera(p,type));
}
inline float opening(const State& s){
    if(!valid(s)||s.quarter!=1)return 0;
    float progress=1-s.seconds/s.quarterSeconds;
    return 1-(std::clamp)((progress-.10f)/.65f,0.0f,1.0f);
}
// Fade extra layers from a two-score game to none at a four-score margin.
// Ordinary native reactions and fullness-based volume remain independent.
inline float competitive(const State& s){return core::competition(s);}
inline bool reactionEligible(const State& s,float fullness,bool known,bool context,float minimum=core::ReactionMinimumFullness){return known&&context&&std::isfinite(fullness)&&std::isfinite(minimum)&&minimum>=core::ReactionMinimumFullness&&minimum<=1&&fullness>=minimum&&competitive(s)>0;}
inline float lateDrive(const energy::Play& p){
    if(p.simulated||energy::clockManagement(p))return 0;
    const auto& s=p.game;
    if(energy::normalDown(p)&&!p.specialTeams&&s.quarter==4&&s.phase!=7&&s.seconds>=0&&s.seconds<=120&&
       p.offense>=0&&p.offense<=1&&std::abs(s.homeScore-s.awayScore)<=8)return .85f+.15f*(1-s.seconds/120.f);
    if(!energy::normalDown(p)||s.quarter!=4||s.seconds>180||s.phase==7||p.toGoal<0||p.toGoal>40||p.offense<0||p.offense>1||(!s.neutral&&p.offense!=0))return 0;
    int margin=p.offense==0?s.homeScore-s.awayScore:s.awayScore-s.homeScore;
    if(margin>0||margin< -8)return 0;
    float field=1-(std::clamp)(p.toGoal/50.0f,0.0f,1.0f),clock=1-(std::clamp)(s.seconds/240.0f,0.0f,1.0f);
    return (std::clamp)(.4f+.35f*field+.25f*clock,0.0f,1.0f);
}
inline float goalLine(const energy::Play& p){
    const auto& s=p.game;
    if(!energy::normalDown(p)||p.toGoal<0||p.toGoal>20||std::abs(s.homeScore-s.awayScore)>8||s.phase==7||(!s.neutral&&p.offense!=0))return 0;
    return .35f+.35f*(1-p.toGoal/20);
}
inline float ordinaryImportance(float density,float close,float openingWeight){
    // Preserve the existing ordinary-game foundation exactly.
    return (std::max)((.20f+.32f*density)*close,.62f*openingWeight*close);
}
inline float bigGamePremium(const State& s,float fraction,float close){
    float presence=std::sqrt((std::clamp)(fraction/.8f,0.0f,1.0f));
    // The final quarter of Q4 scales with any configured quarter length.
    float finish=s.quarter==4&&s.quarterSeconds>0?1-(std::clamp)(s.seconds/(s.quarterSeconds*.25f),0.0f,1.0f):0;
    return (.20f+(s.playoff||s.conferenceChampionship?.05f:0)+.10f*finish)*close*presence;
}
inline constexpr float PuntCameraScale=.50f,HomeOffenseCriticalCameraScale=.85f;
struct CameraAdmission {bool allowed=false,critical=false;float strengthScale=1;};
inline CameraAdmission cameraAdmission(const core::Settings& c,const energy::Play& p,float fraction,bool known,uint64_t positions,int selectedType,bool openingKickoff=false,bool openingDrive=false){
    CameraAdmission out;
    if(!known||!std::isfinite(fraction)||fraction<0||fraction>1||!c.enabled||!c.lineups||!c.cameraShake||core::normalLimit(c)<=0||!valid(p.game)||p.simulated||
       (p.game.phase!=1&&p.game.phase!=5)||competitive(p.game)<=0||selectedType<=0||!shakePlay(p,selectedType,openingKickoff))return out;
    bool venue=cameraVenue(c,p.game,fraction,known,positions);
    bool special=fraction>=nightRequirement(p.game,packed::specialMinimum(positions))&&((c.overtime&&competitiveOvertime(p.game))||lateDrive(p)>0);
    bool important=energy::normalDown(p)&&p.down>=3&&std::abs(p.game.homeScore-p.game.awayScore)<=8&&offensivePlay(selectedType)&&criticalCamera(p,selectedType);
    out.critical=important&&(fraction>=nightRequirement(p.game,packed::standardMinimum(positions))||special);
    bool kickoff=c.opening&&openingKickoff&&kickoffMatchup(p.game)&&(selectedType==23||selectedType==24)&&fraction>=kickoffRequirement(c,p.game,positions);
    out.allowed=venue||kickoff||special||out.critical||(c.opening&&openingDrive&&fraction>=openingMinimum(p.game,positions));
    // Scale only pre-snap motion. Sound and earned live/result reactions
    // retain their own strengths; the motion model carries this into its tail.
    if(selectedType==22)out.strengthScale=PuntCameraScale;
    else if(out.critical&&!p.game.neutral&&p.offense==0)out.strengthScale=HomeOffenseCriticalCameraScale;
    return out;
}
struct Model {
    uint64_t positionsForSound=0;
    uintptr_t match=0;unsigned epoch=0;int phase=-1;uint64_t serial=0,began=0,tailAt=0,continuousBegan=0;
    bool running=false,continuousMode=false;float weight=0,tailWeight=0,strength=0;int shakeTier=0;
    uint64_t variationSeed=0;variety::Profile profile;
    uint64_t cheerSerial=0,cheerBegan=0;
    bool cheerMoment=false,openingObserved=false,homeOffenseSeen=false,drivePending=true,lineupAdmitted=false;
    int previousOffense=-1,previousHome=-1,previousAway=-1,previousQuarter=0;
    float shakeChoice=0;
    bool carrying=false,leverage=false;uint64_t liveAt=0,postAt=0;float carryWeight=0,carryCompetition=1;
    sustained::Reason reason=sustained::Reason::None;
    drive::Model drive;bool packedHouse=false,qualifiedPlay=false;float qualifiedMinimum=1.01f;
    OpeningDrive openingDrive;bool openingDriveQualified=false;
    upsetsound::Route upsetSound;
    bool hostileOpening=false,hostileSeriesActive=false;uint64_t hostileCheer=0,hostileCheerBegan=0;
    unsigned neutralOpeningSeen=0;int hostileCheerOffense=-1;
    void reset(){*this=Model{};}
    void handOffResult(){if(carrying&&!continuousMode){carrying=running=false;liveAt=postAt=tailAt=0;weight=0;reason=sustained::Reason::None;}}
    void suspend(){positionsForSound=0;upsetSound={};qualifiedPlay=openingDriveQualified=hostileOpening=hostileSeriesActive=false;qualifiedMinimum=1.01f;cheerMoment=false;running=carrying=leverage=false;liveAt=postAt=0;tailAt=0;continuousMode=false;weight=0;shakeTier=0;reason=sustained::Reason::None;}
    uint64_t cameraLineupId()const{return phase==5?cheerSerial+1:cheerSerial;}
    float cameraChoice()const{return phase==5?static_cast<float>(variety::mix(variationSeed^(cheerSerial+1)^0x5348414b45ull)%1001)/1000.0f:shakeChoice;}
    float momentWeight()const{return leverage&&strength>0?(std::clamp)(weight/strength,0.0f,1.0f):0;}
    bool layeredActive()const{return weight>0&&reason!=sustained::Reason::None;}
    // The sound may settle before the snap. A verified result still belongs
    // to the play call that qualified, until another lineup or suspension.
    bool playContextActive()const{return qualifiedPlay;}
    float audibleEnvelope(uint64_t now)const{
        if(strength<=0)return 0;
        float envelope=weight/strength;
        // The early cheer needs excited voices before the slower native roar
        // attack finishes. Hold the same requested intensity until that
        // envelope catches up, without changing either gain or native input.
        if(phase==1&&running&&now>=began&&now-began<profile.envelopeAttackMs)envelope=(std::max)(envelope,1.0f);
        return envelope;
    }
    float excitement(uint64_t now,float fraction)const{
        if(strength<=0)return 0;
        float envelope=audibleEnvelope(now);
        float presence=std::sqrt((std::clamp)(fraction/.8f,0.0f,1.0f));
        return (500+500*strength)*envelope*presence;
    }
    bool observe(const core::Settings& c,const energy::Play& p,uintptr_t owner,unsigned generation,uint64_t now,bool allowed,float fraction=1,bool fullnessKnown=true,const energy::Outcome* result=nullptr,uint64_t positions=0){
        if(owner!=match||generation!=epoch){reset();match=owner;epoch=generation;}
        positionsForSound=fullnessKnown?positions:0;
        bool start=false;const auto& s=p.game;
        bool firstDrive=openingDrive.observe(p,result);openingDriveQualified=false;
        bool priorHostileSeries=hostileSeriesActive;hostileOpening=hostileSeriesActive=false;
        if(valid(s)&&s.quarter==1&&s.homeScore==0&&s.awayScore==0&&s.seconds>=s.quarterSeconds-1&&!p.resumed)openingObserved=true;
        if(p.resumed)openingObserved=false;
        if(s.neutral&&s.phase==3&&(phase==1||phase==5)&&energy::normalDown(p)&&p.offense>=0&&p.offense<=1)
            neutralOpeningSeen|=1u<<p.offense;
        if(previousHome>=0&&(s.homeScore!=previousHome||s.awayScore!=previousAway))drivePending=true;
        if(previousQuarter==2&&s.quarter>=3)drivePending=true;
        if(previousQuarter>0&&s.quarter>previousQuarter&&overtime(s))drivePending=true;
        if(p.down==0||p.down==-1)drivePending=true;
        bool newLineup=s.phase==1&&phase!=1;
        if(newLineup){
            lineupAdmitted=false;
            bool normal=energy::normalDown(p),home=normal&&(p.offense==0||s.neutral);
            bool newDrive=normal&&(drivePending||p.offense!=previousOffense);
            bool firstHome=home&&openingObserved&&!homeOffenseSeen&&s.quarter==1;
            int lead=p.offense==1?s.awayScore-s.homeScore:s.homeScore-s.awayScore;
            bool winningDrive=home&&newDrive&&((s.quarter==4&&s.seconds<=180)||overtime(s))&&lead<=0&&lead>=-8;
            cheerMoment=home&&((p.toGoal>=0&&p.toGoal<=10)||firstHome||winningDrive);
            if(home)homeOffenseSeen=true;
            if(normal){previousOffense=p.offense;drivePending=false;}
        }
        previousHome=s.homeScore;previousAway=s.awayScore;previousQuarter=s.quarter;
        if(c.enabled&&c.lineups&&c.driveEscalation)drive.observe(p,result);else drive.reset();packedHouse=false;
        variationSeed=s.variationSeed?s.variationSeed:static_cast<uint64_t>(owner)^generation;
        bool supported=allowed&&fullnessKnown&&std::isfinite(fraction)&&fraction>=core::ReactionMinimumFullness&&c.enabled&&c.lineups&&core::normalLimit(c)>0&&valid(s)&&(s.phase==1||s.phase==3||s.phase==4||s.phase==5||s.phase==6);
        if(s.phase==2){
            // A native timeout/camera transition is not a new play or a pause
            // menu. Stop lineup scheduling, but retain this play's admission
            // so its already-earned result can finish its original tail.
            bool keep=qualifiedPlay&&allowed&&fullnessKnown&&std::isfinite(fraction)&&fraction>=qualifiedMinimum&&
                c.enabled&&c.lineups&&core::normalLimit(c)>0&&valid(s)&&competitive(s)>0;
            float minimum=qualifiedMinimum;suspend();
            if(keep){qualifiedPlay=true;qualifiedMinimum=minimum;}
            phase=s.phase;return false;
        }
        float standard=nightRequirement(s,attendance::packed::standardMinimum(positions)),specialMinimum=nightRequirement(s,attendance::packed::specialMinimum(positions));
        openingDriveQualified=supported&&c.opening&&firstDrive&&fraction>=openingMinimum(s,positions)&&competitive(s)>0;
        bool ot=supported&&c.overtime&&overtime(s)&&fraction>=specialMinimum;
        float late=supported&&fraction>=specialMinimum?lateDrive(p):0;
        bool big=bigGame(s),packed=c.packedHouse&&fullnessKnown&&fraction>=nightRequirement(s,attendance::packed::minimum(positions));
        bool critical=supported&&fraction>=standard&&energy::normalDown(p)&&p.down>=3&&std::abs(s.homeScore-s.awayScore)<=8;
        float goal=supported&&fraction>=standard?goalLine(p):0;
        float bigThreshold=bigRequirement(c,s,positions);
        float competition=competitive(s);
        upsetSound=upsetsound::evaluate(s,positions,fraction,fullnessKnown,supported&&c.upsetWatch&&!p.simulated);
        bool upset=upsetSound.qualified&&competition>0;
        bool bigQualified=big&&fraction>=bigThreshold;
        bool kickoffQualified=c.opening&&kickoffMatchup(s)&&openingObserved&&!p.resumed&&!p.simulated&&s.quarter==1&&!s.homeScore&&!s.awayScore&&
            s.seconds>=s.quarterSeconds-1&&p.down==0&&(p.selectedType==23||p.selectedType==24)&&fraction>=kickoffRequirement(c,s,positions);
        bool standardRoute=supported&&competition>0&&(bigQualified||packed||critical||goal>0||openingDriveQualified||kickoffQualified);
        bool lightOpening=openingDriveQualified&&fraction<standard&&!bigQualified&&!packed&&!critical&&goal<=0;
        bool upsetFinish=upset&&upsetSound.climax;
        bool special=ot||late>0||upsetFinish,regular=standardRoute||upset;
        bool neutralOpener=s.neutral&&openingObserved&&!p.resumed&&!p.simulated&&s.quarter==1&&p.offense>=0&&p.offense<=1&&!(neutralOpeningSeen&(1u<<p.offense));
        bool ordinaryAway=energy::normalDown(p);
        hostileSeriesActive=regular&&!lightOpening&&c.opening&&!s.neutral&&openingDrive.awayOpeningSeries()&&p.offense==1&&ordinaryAway;
        hostileOpening=(hostileSeriesActive||(regular&&!lightOpening&&c.opening&&neutralOpener&&energy::normalDown(p)&&!p.specialTeams))&&(s.phase==1||s.phase==5);
        if(priorHostileSeries&&!hostileSeriesActive&&continuousMode){continuousMode=running=carrying=false;weight=strength=tailWeight=0;liveAt=postAt=tailAt=0;reason=sustained::Reason::None;lineupAdmitted=false;}
        packedHouse=standardRoute&&!big&&!ot&&late<=0&&packed&&attendance::packed::qualifies(positions,fraction,fullnessKnown);
        bool enterLineup=s.phase==1&&(newLineup||!lineupAdmitted);
        if(enterLineup){
            qualifiedPlay=regular||special;qualifiedMinimum=1.01f;
            if(regular){if(bigQualified)qualifiedMinimum=bigThreshold;if(kickoffQualified)qualifiedMinimum=(std::min)(qualifiedMinimum,kickoffRequirement(c,s,positions));if(packed)qualifiedMinimum=(std::min)(qualifiedMinimum,nightRequirement(s,attendance::packed::minimum(positions)));if(critical||goal>0)qualifiedMinimum=(std::min)(qualifiedMinimum,standard);if(openingDriveQualified)qualifiedMinimum=(std::min)(qualifiedMinimum,openingMinimum(s,positions));}
            if(ot||late>0)qualifiedMinimum=(std::min)(qualifiedMinimum,specialMinimum);
            if(upset)qualifiedMinimum=(std::min)(qualifiedMinimum,upsetSound.minimum);
        }
        if(!supported){suspend();phase=s.phase;return false;}
        if(!regular&&!special){
            qualifiedPlay=false;qualifiedMinimum=1.01f;cheerMoment=false;
            if(!tailAt&&weight>0){tailAt=now;tailWeight=weight;}
            running=continuousMode=carrying=false;liveAt=postAt=0;shakeTier=0;reason=sustained::Reason::None;
            float tail=tailAt&&now>=tailAt?1-(std::clamp)(static_cast<float>(now-tailAt)/SnapTailMs,0.0f,1.0f):0;
            weight=tailWeight*tail;if(tail<=0)tailAt=0;
            phase=s.phase;return false;
        }
        if(enterLineup)lineupAdmitted=true;
        int margin=std::abs(s.homeScore-s.awayScore);
        float close=margin<=8?1.0f:margin<=16?.75f:margin<=24?.35f:.12f;
        float openingWeight=c.opening?opening(s):0;
        float density=attendance::packed::strength(positions,fraction);
        float importance=ordinaryImportance(density,close,openingWeight);
        // Big games build ON the same foundation. Retain the former opening
        // peak as a floor, then add energy rather than replacing packed density.
        if(big)importance=(std::max)({importance+bigGamePremium(s,fraction,close),.22f*close,openingWeight});
        if(upset)importance=(std::max)(importance,upsetSound.importance*close);
        importance=(std::min)(1.0f,importance+.35f*drive.heat*close);
        importance=(std::max)(importance,goal);
        if(hostileOpening||hostileSeriesActive)importance=1;
        if((s.phase==1||s.phase==5)&&critical)importance=(std::max)(importance,p.down==4?1.0f:.95f);
        if(lightOpening)importance=(std::min)(importance,.45f);
        importance*=competition;
        if(special||hostileSeriesActive){
            leverage=true;
            carrying=false;liveAt=postAt=0;
            if(!continuousMode){continuousBegan=now;began=now;++serial;profile=variety::choose(variationSeed,serial);start=true;}
            continuousMode=true;running=true;tailAt=0;weight=hostileSeriesActive||ot?1.0f:(std::max)(late,upsetFinish?importance:0.f);strength=weight;
            reason=hostileSeriesActive?sustained::Reason::Lineup:ot?sustained::Reason::Overtime:late>0?sustained::Reason::LateDrive:sustained::Reason::Upset;
        }else{
            if(s.phase==1)leverage=critical||goal>=.525f||hostileOpening;
            if(continuousMode){continuousMode=false;running=false;tailAt=now;tailWeight=weight;}
            if(enterLineup){began=now;++serial;profile=variety::choose(variationSeed,serial);if(hostileOpening){profile.attackMs=800;profile.envelopeAttackMs=900;profile.supportDelayMs=0;}running=true;carrying=false;liveAt=postAt=0;tailAt=0;start=true;}
            // Carry an existing buildup through the live ball, then bridge the
            // whistle to the native result reaction. No extra gain or cheer is
            // introduced here, and a stuck live phase has a strict time limit.
            if(s.phase==3&&phase==1&&running&&audibleEnvelope(now)>0&&now>=began&&now-began<profile.totalMs()){carrying=true;liveAt=now;postAt=0;carryWeight=strength*audibleEnvelope(now);carryCompetition=competition;}
            bool hold=false;
            if(carrying){
                if(s.phase!=3&&!postAt)postAt=now;
                hold=now>=liveAt&&now-liveAt<LiveCarryMs&&(!postAt||(now>=postAt&&now-postAt<PostPlayHoldMs));
                if(!hold){
                    tailAt=(std::min)(liveAt+LiveCarryMs,postAt?postAt+PostPlayHoldMs:liveAt+LiveCarryMs);
                    tailWeight=carryWeight;running=carrying=false;
                }
            }
            if(hold){weight=carryWeight*(std::min)(1.f,competition/(std::max)(.001f,carryCompetition));reason=upset?sustained::Reason::Upset:sustained::Reason::Lineup;}
            else if(running&&s.phase==1&&now>=began&&now-began<profile.totalMs()){
                auto age=now-began;float envelope=(std::min)({1.0f,static_cast<float>(age)/profile.envelopeAttackMs,static_cast<float>(profile.totalMs()-age)/profile.releaseMs});
                strength=importance;weight=importance*envelope;reason=age<profile.roarMs?(upset?sustained::Reason::Upset:sustained::Reason::Lineup):sustained::Reason::None;
            }else if(running&&!carrying&&now>=began&&now-began>=profile.totalMs()){
                running=false;tailAt=0;weight=0;reason=sustained::Reason::None;
            }else if(running||tailAt){
                if(!tailAt){tailAt=now;tailWeight=weight;}
                running=false;float tail=now>=tailAt?1-(std::clamp)(static_cast<float>(now-tailAt)/SnapTailMs,0.0f,1.0f):0;
                weight=tailWeight*tail;reason=sustained::Reason::None;
                if(tail<=0)tailAt=0;
            }else{weight=0;reason=sustained::Reason::None;}
        }
        if(enterLineup){
            ++cheerSerial;cheerBegan=now;
            if(hostileOpening&&hostileCheerOffense!=p.offense){hostileCheer=cheerSerial;hostileCheerBegan=now;hostileCheerOffense=p.offense;}
            shakeChoice=static_cast<float>(variety::mix(variationSeed^cheerSerial^0x5348414b45ull)%1001)/1000.0f;
        }
        shakeTier=0;
        if(c.cameraShake&&(s.phase==1||s.phase==5)&&(standardRoute||ot||late>0)&&(special||importance>.10f))shakeTier=(ot||late>0||critical)?3:goal>=.6f?2:1;
        phase=s.phase;return start;
    }
    sustained::Intent merge(sustained::Intent out,const core::Settings& c,const energy::Play& p,uint64_t now,float fraction)const{
        if(!out.active||!c.enabled||!c.lineups||out.match!=match||out.epoch!=epoch||out.reason==sustained::Reason::Eruption||out.celebration)return out;
        if(hostileSeriesActive&&!p.game.neutral&&p.offense==1)out.hostileOpening=true;
        if(out.reason==sustained::Reason::Result){
            // Let the result take the stage while retaining whichever native
            // excitement is stronger. No scheduled lineup cheer is added.
            float level=excitement(now,fraction);
            // If the next play is already being selected, prepare its camera
            // normally rather than missing that handoff during the result.
            if(shakeTier>0&&(p.game.phase==1||p.game.phase==5)){out.shake=true;out.shakeTier=shakeTier;out.cameraPrime=p.game.phase==5;}
            out.intensity=(std::max)(out.intensity,level);return out;
        }
        // Prepare only the camera behind play selection. Audio still starts
        // with the lineup and follows its existing attack and result bridge.
        if(shakeTier>0){out.shake=true;out.shakeTier=shakeTier;out.cameraPrime=p.game.phase==5;}
        out.lineupCheer=cheerMoment?cheerSerial:0;out.lineupCheerBegan=cheerBegan;
        out.lineupCheerUntil=cheerBegan+1500;
        out.cheers=cheerMoment&&p.game.phase==1;out.deepCheers=false;
        if(hostileOpening){
            out.hostileOpening=true;out.lineupType=p.game.neutral?91:59;out.lineupCheer=hostileCheer;out.lineupCheerBegan=hostileCheerBegan;out.lineupCheerUntil=hostileCheerBegan+1500;
            out.cheers=p.game.phase==1;out.deepCheers=true;out.intensity=(std::max)(out.intensity,excitement(now,fraction));
        }
        if(!p.game.neutral&&p.offense==1){out.cheers=out.deepCheers=false;out.lineupCheer=0;}
        if(weight<=0)return out;
        float level=excitement(now,fraction);
        out.intensity=(std::max)(out.intensity,level);
        auto cycle=continuousMode&&now>=continuousBegan?(now-continuousBegan)/40000:0;
        auto variation=continuousMode?variety::choose(variationSeed,serial,cycle):profile;
        out.lineup=(serial<<32)|cycle;out.lineupBegan=continuousMode?continuousBegan+cycle*40000:began;out.lineupUntil=continuousMode?now+750:began+profile.roarMs;
        out.shake=shakeTier>0;out.shakeTier=shakeTier;out.reason=reason;out.priority=continuousMode?4:1;out.size=variation.size(strength);
        out.continuous=reason!=sustained::Reason::None;out.releaseMs=static_cast<int>(variation.releaseMs);out.attackMs=static_cast<int>(variation.attackMs);
        // Full ordinary crowds can keep a quiet supporting layer through a
        // two-score margin. The larger tier adds presence, not another gain
        // bonus. Existing admission, score fading and lifetime still own it.
        out.supporting=strength>=dense::layers(positionsForSound,fraction).swellMinimum()&&now>=began+variation.supportDelayMs&&reason!=sustained::Reason::None;
        if(hostileSeriesActive&&!p.game.neutral&&p.offense==1){out.size=3;out.attackMs=800;out.supporting=reason!=sustained::Reason::None;}
        out.cheers=out.cheers&&reason!=sustained::Reason::None;return out;
    }
};
inline float childGain(float requestedParent,float appliedParent,float percent,float limit){return mix::ambienceForTotal(requestedParent,appliedParent,volume::boostDb((std::clamp)(percent,0.0f,50.0f)),limit);}
}
