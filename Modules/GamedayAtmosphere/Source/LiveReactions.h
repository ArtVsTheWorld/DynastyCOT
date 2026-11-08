#pragma once
#include "ExcitementRules.h"
#include "SustainedCrowd.h"
#include "ReactionCatalog.h"
namespace attendance::live {
enum class Event {None,LongCatch,Interception,RunAfterCatch,LongRun,Sack,ForcedFumble,KickReturn,PuntReturn,FourthConversion,GoalLineStop,CompletedPass,CriticalStop,FumbleRecovery,TackleForLoss,BlockedFieldGoal,Truck,StiffArm,Hurdle,Juke,Spin,HitStick,BrokenTackle,ThirdGoalStop,CatchKnockout,PassBreakup,HitStickStop,DownsStop,OvertimeStop,OvertimeConversion,OvertimeConversionStop,OvertimeFieldGoal,OvertimeMissedFieldGoal};
inline bool skill(Event e){return e==Event::Truck||e==Event::StiffArm||e==Event::Hurdle||e==Event::Juke||e==Event::Spin;}
inline bool accent(Event e){return skill(e)||e==Event::HitStick||e==Event::BrokenTackle;}
inline bool cheerOnlySkill(Event e){return e==Event::Juke||e==Event::Spin;}
inline bool softSkill(Event e){return e==Event::StiffArm||e==Event::Hurdle||e==Event::BrokenTackle;}
// A verified live defensive recovery uses the same continuous return cue as
// an interception. Official-result-only recoveries remain excluded below.
inline bool cameraReaction(Event e){return !cheerOnlySkill(e)&&e!=Event::None&&e!=Event::HitStick&&e!=Event::BrokenTackle;}
inline const char* accentName(Event e){switch(e){case Event::OvertimeStop:return "Overtime defensive stop";case Event::OvertimeConversion:return "Overtime two-point conversion";case Event::OvertimeConversionStop:return "Overtime two-point stop";case Event::OvertimeFieldGoal:return "Overtime made field goal";case Event::OvertimeMissedFieldGoal:return "Overtime opponent missed field goal";case Event::Truck:return "Successful truck";case Event::StiffArm:return "Successful stiff arm";case Event::Hurdle:return "Successful hurdle";case Event::Juke:return "Successful juke";case Event::Spin:return "Successful spin";case Event::HitStick:return "Landed hit-stick tackle";case Event::BrokenTackle:return "Broken tackle";case Event::ThirdGoalStop:return "Third-down goal-line stop";default:return nullptr;}}
inline const char* name(Event e){if(auto label=accentName(e))return label;switch(e){case Event::DownsStop:return "Turnover on downs";case Event::PassBreakup:return "Confirmed home defensive pass breakup";case Event::HitStickStop:return "Confirmed big hit on a defensive stop or loss";case Event::CatchKnockout:return "Home defensive catch knockout";case Event::BlockedFieldGoal:return "Blocked field goal";case Event::TackleForLoss:return "Defensive tackle for loss (5+ yards)";case Event::FumbleRecovery:return "Defensive fumble recovery";case Event::CriticalStop:return "High-leverage defensive stop";case Event::LongCatch:return "Long completion";case Event::Interception:return "Interception";case Event::RunAfterCatch:return "Long catch-and-run";case Event::LongRun:return "Long run";case Event::Sack:return "Defensive sack";case Event::ForcedFumble:return "Forced fumble";case Event::KickReturn:return "Long kickoff return";case Event::PuntReturn:return "Long punt return";case Event::FourthConversion:return "Live fourth-down conversion";case Event::GoalLineStop:return "Fourth-down goal-line stand";case Event::CompletedPass:return "Confirmed long pass at the whistle";default:return "inactive";}}
struct Outcome {Event event=Event::None;uint64_t id=0;int side=-1;float yards=0;};
inline int cheerType(const Outcome& event){
    if(event.event==Event::Truck)return 11; // Native BIGHIT_POS, large size.
    if(event.event==Event::Hurdle||cheerOnlySkill(event.event))return 83; // Successful clearance: positive cheer plus the gentle buildup.
    if(softSkill(event.event))return 0; // A short swell accompanies the softer motion.
    switch(event.event){
    case Event::OvertimeStop:case Event::OvertimeConversionStop:case Event::OvertimeMissedFieldGoal:case Event::DownsStop:case Event::PassBreakup:case Event::HitStickStop:case Event::CatchKnockout:case Event::ThirdGoalStop:case Event::HitStick:case Event::BlockedFieldGoal:case Event::Interception:case Event::Sack:case Event::TackleForLoss:case Event::ForcedFumble:case Event::GoalLineStop:case Event::CriticalStop:case Event::FumbleRecovery:return 59;
    // Use the already listening-tested large cheer for explosive completions
    // and gains; ordinary threshold plays keep the conversion family.
    default:return std::isfinite(event.yards)&&event.yards>=40?91:83;
    }
}
inline Outcome completedImpact(const energy::Outcome& result,const State& state,bool allowed,uint64_t previousSequence,int previousType,bool fourthAlready=false){
    bool phase=state.phase==4||state.phase==5||state.phase==1;
    bool loss=result.fiveYardLoss&&std::isfinite(result.yards)&&result.yards>=-100&&result.yards<=-4.99f;
    // Official defensive takeaways can supply one missed recovery impact. Own
    // pickups lack takeaway proof; verified force and live return retain priority.
    bool recovery=result.takeaway&&result.event==energy::Event::FumbleRecovery;
    bool fourth=result.firstDown&&result.event==energy::Event::FourthDown;
    if(fourth&&fourthAlready)return {}; // Live crossing already owns this conversion, even after a later gain accent.
    if(!allowed||!result.accepted||!result.completed||!result.sequence||!phase||result.side<0||result.side>1||(!state.neutral&&result.side!=0)||
       (result.sequence==previousSequence&&!catalog::negative(previousType))||!(fourth||recovery||result.sack||loss||result.forcedFumble||result.goalLineStop||result.thirdGoalStop||result.highLeverageStop||result.blockedFieldGoal||result.turnoverOnDowns))return {};
    Event selected=fourth?Event::FourthConversion:result.blockedFieldGoal?Event::BlockedFieldGoal:result.goalLineStop?Event::GoalLineStop:result.thirdGoalStop?Event::ThirdGoalStop:result.forcedFumble?Event::ForcedFumble:recovery?Event::FumbleRecovery:result.sack?Event::Sack:loss?Event::TackleForLoss:result.turnoverOnDowns?Event::DownsStop:Event::CriticalStop;
    return {selected,result.sequence,result.side,loss?result.yards:0};
}
struct Settings {bool enabled=true;float minimumCatchYards=20,minimumGainYards=15;bool passFlight=true;};
inline constexpr float RunShakeStartYards=11.f,RunShakePeakYards=15.f;
struct RunBuildup {uint64_t id=0;float progress=0,yards=0;};
// Verified native traces: kind 3 / ball 1 / no holder is the pass in flight.
// Ball 4 can already name the receiver before the stats record becomes kind 1;
// a non-null holder by itself is therefore NOT proof of a completed catch.
class PassFlight {
    uintptr_t match_=0,record_=0,readyRecord_=0;unsigned epoch_=0;int phase_=-1,side_=-1;
    uint64_t last_=0,snap_=0,began_=0,ended_=0;bool ready_=false,snapped_=false,reset_=false,used_=false,air_=false;
public:
    void reset(){*this=PassFlight{};}
    void observe(const energy::Play& p,uintptr_t match,unsigned epoch,uint64_t now,bool allowed,uintptr_t record,int kind,int ball,bool held){
        if(match!=match_||epoch!=epoch_){reset();match_=match;epoch_=epoch;}
        if(!allowed||!match||p.simulated||!valid(p.game)||now<last_||(last_&&now-last_>750)){reset();return;}last_=now;
        if(p.game.phase==1){ready_=energy::normalDown(p)&&p.offense>=0&&p.offense<=1;side_=p.offense;snapped_=air_=false;began_=ended_=0;readyRecord_=kind==0?record:0;}
        if(p.game.phase==3&&phase_==1&&ready_&&energy::normalDown(p)&&p.offense==side_){snapped_=true;record_=record;snap_=now;reset_=record&&record==readyRecord_;used_=false;}
        if(snapped_&&p.game.phase==3&&record&&record==record_&&p.offense==side_&&now>=snap_&&now-snap_<=45000){
            if(kind==0)reset_=true;
            bool flight=reset_&&(kind==3||kind==1)&&ball==1&&!held;
            if(flight&&!used_){used_=air_=true;began_=now;}
            if(air_&&(!flight||now-began_>=8000)){air_=false;ended_=now;}
        }else if(air_){air_=false;ended_=now;}
        if(p.game.phase!=1&&p.game.phase!=3)ready_=snapped_=false;
        phase_=p.game.phase;
    }
    float weight(uint64_t now)const{
        if(!began_||now<last_||now-last_>750||now<began_)return 0;
        if(air_)return (std::clamp)(static_cast<float>(now-began_)/150.0f,0.0f,1.0f);
        return ended_&&now>=ended_?1-(std::clamp)(static_cast<float>(now-ended_)/1200.0f,0.0f,1.0f):0;
    }
    uint64_t flightId()const{return began_;}
    bool airborne()const{return air_;}
    sustained::Intent merge(sustained::Intent out,uint64_t now,float fullness)const{
        float w=weight(now);
        if(!out.active||out.match!=match_||out.epoch!=epoch_||w<=0||out.celebration||out.reason==sustained::Reason::Result||out.reason==sustained::Reason::Eruption)return out;
        out.intensity=(std::max)(out.intensity,900*w*std::sqrt((std::clamp)(fullness,0.0f,1.0f)));
        out.passFlight=began_;out.supporting=true;out.size=3;
        // Keep the existing roar and use the shared gameplay swell. If no
        // roar remains, the swell alone is enough; the primary is a fault fallback.
        if(out.reason==sustained::Reason::None){out.reason=sustained::Reason::PassFlight;out.priority=2;out.continuous=true;out.attackMs=500;out.releaseMs=1200;out.supportingOnly=true;}
        return out;
    }
};
// BallControl stores world Z. The native yard-line/catch coordinates can use
// the opposite axis. Resolve that relationship at the line, never from the
// gain we are trying to detect, and keep it frozen through the live play.
class RunnerCoordinates {
    uintptr_t match_=0;unsigned epoch_=0;int quarter_=0,side_=-1,phase_=-1,axis_=0;
    int known_[2]{0,0};uint64_t last_=0;
public:
    void reset(){*this=RunnerCoordinates{};}
    int axis()const{return axis_;}
    float observe(const energy::Play& p,uintptr_t match,unsigned epoch,uint64_t now,bool allowed,float worldZ){
        const float unavailable=std::numeric_limits<float>::quiet_NaN();
        if(match!=match_||epoch!=epoch_||p.game.quarter!=quarter_){reset();match_=match;epoch_=epoch;quarter_=p.game.quarter;}
        bool stale=now<last_||(last_&&now-last_>750);last_=now;
        if(stale||!allowed||!match||p.offense<0||p.offense>1){axis_=0;side_=-1;phase_=-1;return unavailable;}
        bool position=std::isfinite(worldZ)&&std::fabs(worldZ)<=60;
        if(p.game.phase==1){
            side_=p.offense;axis_=0;
            if(position&&std::isfinite(p.fieldSpot)&&std::fabs(p.fieldSpot)<=50.01f){
                float same=std::fabs(worldZ-p.fieldSpot),opposite=std::fabs(-worldZ-p.fieldSpot);
                if(same<=.75f&&opposite>=3)known_[side_]=axis_=1;
                else if(opposite<=.75f&&same>=3)known_[side_]=axis_=-1;
                // Near midfield either sign may fit. Only reuse an axis
                // already verified for this team in this quarter/world.
                else if(known_[side_]&&std::fabs(known_[side_]*worldZ-p.fieldSpot)<=.75f)axis_=known_[side_];
            }
        }else if((p.game.phase!=3&&p.game.phase!=4)||side_!=p.offense||
                 (phase_!=1&&phase_!=3&&phase_!=4))axis_=0;
        phase_=p.game.phase;
        return axis_&&position?axis_*worldZ:unavailable;
    }
};
// Return yardage starts at the actual catch/recovery, not the kicking LOS.
// Flight establishes a world-space direction; no yard-line axis assumption.
class Returns {
    uintptr_t match_=0;unsigned epoch_=0;uint64_t last_=0,serial_=0,began_=0;
    int phase_=-1,kicker_=-1,type_=0,direction_=0;bool ready_=false,live_=false,flight_=false,caught_=false,used_=false;
    float launch_=0,catch_=0,best_=0,lastReturnSpot_=0;uint64_t returnAt_=0;
    RunBuildup buildup_;
public:
    void reset(){*this=Returns{};}
    float yards()const{return buildup_.id&&live_?best_:-1;}
    RunBuildup buildup()const{return buildup_;}
    Outcome observe(const energy::Play& p,uintptr_t match,unsigned epoch,uint64_t now,bool allowed,int selectedType,float worldZ,bool held){
        if(match!=match_||epoch!=epoch_){reset();match_=match;epoch_=epoch;}
        Outcome out;bool stale=now<last_||(last_&&now-last_>750);last_=now;
        if(stale||!allowed||!match||!valid(p.game)||p.simulated){ready_=live_=false;phase_=-1;buildup_={};return out;}
        bool position=std::isfinite(worldZ)&&std::fabs(worldZ)<=65;
        if(p.game.phase==1){ready_=position&&(selectedType==22||selectedType==23||selectedType==24)&&(p.offense==0||p.offense==1);kicker_=p.offense;type_=selectedType;launch_=worldZ;live_=false;}
        if(p.game.phase==3&&phase_==1&&ready_){live_=true;began_=now;flight_=caught_=used_=false;direction_=0;best_=0;returnAt_=0;buildup_={};}
        if(live_&&now>=began_&&now-began_>45000){live_=false;buildup_={};}
        if(p.game.phase==3&&live_&&position&&now>=began_&&now-began_<=45000){
            if(!caught_&&!held&&std::fabs(worldZ-launch_)>=10){flight_=true;direction_=worldZ>launch_?-1:1;}
            if(flight_&&held&&p.offense==1-kicker_){
                if(!caught_){caught_=true;catch_=lastReturnSpot_=worldZ;returnAt_=now;}
                float yards=(worldZ-catch_)*direction_;
                bool coherent=returnAt_&&now>=returnAt_&&std::fabs(worldZ-lastReturnSpot_)<=3.5f+30.f*static_cast<float>(now-returnAt_)*.001f;
                if(coherent){if(yards>=0&&yards<=110)best_=(std::max)(best_,yards);lastReturnSpot_=worldZ;returnAt_=now;}
                if(coherent&&(p.offense==0||p.game.neutral)&&yards>=0&&yards<=110){
                    float start=type_==22?30.f:25.f,peak=start+10;
                    if(best_>=start)buildup_={began_,(std::clamp)((best_-start)/10.f,.0001f,1.f),best_};
                    if(!used_&&best_>=peak){used_=true;out={type_==22?Event::PuntReturn:Event::KickReturn,++serial_,p.offense,best_};}
                }
            }
            if(p.fumble||(!held&&caught_&&now-returnAt_>250)||(caught_&&p.offense!=1-kicker_)){buildup_={};if(p.fumble)live_=false;}
        }
        if(p.game.phase!=1&&p.game.phase!=3){ready_=live_=false;buildup_={};}
        phase_=p.game.phase;return out;
    }
};
class Model {
    uintptr_t match_=0,record_=0;unsigned epoch_=0;int phase_=-1,side_=-1;
    uint64_t last_=0,began_=0,serial_=0,whistle_=0;bool ready_=false,snapped_=false,reset_=false,flight_=false,used_=false;
    float spot_=0,line_=0;int direction_=0;bool caught_=false,fourth_=false,fourthUsed_=false;
    RunBuildup buildup_;bool observedRun_=false,afterCatchRun_=false,gainCancelled_=false,homePossessionLost_=false;float catchAirYards_=-1,catchGain_=0;
    float runnerLast_=0,measuredGain_=0;uint64_t runnerAt_=0;
public:
    void reset(){*this=Model{};}
    void suspend(){ready_=snapped_=reset_=flight_=used_=fourthUsed_=false;phase_=-1;record_=0;whistle_=0;buildup_={};observedRun_=afterCatchRun_=homePossessionLost_=false;catchAirYards_=-1;catchGain_=0;}
    RunBuildup runBuildup()const{return buildup_;}
    bool fourthConverted()const{return fourthUsed_;}
    bool homePossessionLost()const{return homePossessionLost_;}
    void defensiveHandoff(const Outcome& event){
        if(event.id&&event.side==0&&(event.event==Event::ForcedFumble||event.event==Event::Interception||event.event==Event::FumbleRecovery))homePossessionLost_=false;
    }
    void cancelRunBuildup(bool forPlay=false){buildup_={};gainCancelled_=gainCancelled_||forPlay;}
    float catchAirYards()const{return catchAirYards_;}
    float gain()const{return measuredGain_;}bool ranAfterCatch()const{return afterCatchRun_;}
    Outcome observe(const Settings& c,const energy::Play& p,uintptr_t match,unsigned epoch,uint64_t now,bool allowed,uintptr_t record,int kind,int ball,float catchSpot,float runnerSpot=std::numeric_limits<float>::quiet_NaN(),bool held=false,bool goingForIt=false,const energy::Outcome* completed=nullptr){
        if(match!=match_||epoch!=epoch_){reset();match_=match;epoch_=epoch;}
        Outcome out;
        uint64_t prior=last_;
        if(last_&&now>=last_&&now-last_>750)suspend();
        if(!allowed||!c.enabled||!match||!valid(p.game)||p.simulated||now<last_){last_=now;suspend();return out;}last_=now;
        bool normal=energy::normalDown(p);
        if(p.game.phase==1){ready_=normal;side_=p.offense;spot_=p.fieldSpot;direction_=p.attackDirection;snapped_=false;whistle_=0;buildup_={};observedRun_=afterCatchRun_=homePossessionLost_=false;catchAirYards_=-1;catchGain_=0;
            fourth_=p.down==4&&goingForIt&&std::isfinite(p.toFirst)&&p.toFirst>=0&&p.toFirst<=100;line_=p.toFirst;}
        if(p.game.phase==3&&phase_==1&&ready_&&normal&&p.offense==side_){
            snapped_=true;began_=now;record_=record;reset_=flight_=used_=caught_=fourthUsed_=gainCancelled_=false;whistle_=0;runnerLast_=spot_;runnerAt_=now;measuredGain_=0;
        }
        if(p.fumble){
            if(snapped_&&reset_&&p.game.phase==3&&side_==0&&p.offense==side_&&!p.game.neutral)homePossessionLost_=true;
            cancelRunBuildup(true);
        }
        if(completed&&completed->accepted&&completed->completed&&completed->takeaway&&completed->side==1&&!p.game.neutral)homePossessionLost_=true;
        if(snapped_&&now>=began_&&now-began_>45000){snapped_=false;buildup_={};}
        bool post=p.game.phase==4||p.game.phase==5;
        if(post&&phase_==3&&snapped_&&now>=prior&&now-prior<=250)whistle_=now;
        // The catch/result fields may settle after the first whistle frame.
        // Retain only the already-observed snap/record for one bounded second;
        // never reconstruct a snap in replay, simulation or a new lineup.
        bool settling=post&&whistle_&&now>=whistle_&&now-whistle_<=1000;
        if((p.game.phase==3||settling)&&snapped_&&now>=began_&&now-began_<=45000&&record&&record==record_){
            // A quarterback scramble can retain the native pass-record kind.
            // A held rushing-state ball is sufficient proof of a live runner;
            // never infer a rush from a loose or airborne ball.
            bool running=p.game.phase==3&&held&&p.offense==side_&&(ball==2||ball==9||observedRun_)&&!flight_&&(kind==0||kind==3);
            if(kind==0||running)reset_=true;
            observedRun_=observedRun_||running;
            // A fresh pass record also uses kind 1 while its ball is in flight,
            // followed by a held ball-3 receiver. The preceding kind-0 reset
            // prevents last play's completion or the snap from proving flight.
            if(reset_&&(ball==4||((kind==3||kind==1)&&ball==1&&!held)||(kind==1&&ball==3&&held)))flight_=true;
            // Reject the old home buildup immediately, including the native
            // label-handoff frame before offense changes to the interceptor.
            if(reset_&&flight_&&kind==2&&!used_&&side_==0&&!p.game.neutral){homePossessionLost_=true;cancelRunBuildup(true);used_=true;}
            if(reset_&&!used_&&!gainCancelled_&&std::isfinite(spot_)&&std::fabs(spot_)<=50.01f&&(direction_==1||direction_==-1)){
                bool own=side_==0||p.game.neutral;
                float catchTolerance=(std::max)(3.5f,prior&&now>=prior?20.f*static_cast<float>(now-prior)*.001f:3.5f);
                bool catchAgrees=!held||!std::isfinite(runnerSpot)||std::fabs(runnerSpot-catchSpot)<=catchTolerance;
                if(flight_&&kind==1&&ball!=1&&p.offense==side_&&!caught_&&std::isfinite(catchSpot)&&std::fabs(catchSpot)<=50&&catchAgrees){
                    caught_=true;
                    runnerLast_=catchSpot;runnerAt_=now;
                    float gained=std::round((catchSpot-spot_)*direction_);
                    catchAirYards_=gained;
                    catchGain_=(catchSpot-spot_)*direction_;
                    float minimum=std::isfinite(c.minimumCatchYards)?(std::clamp)(c.minimumCatchYards,10.0f,60.0f):20;
                    if(own&&std::isfinite(catchSpot)&&std::fabs(catchSpot)<=50&&gained<=100&&
                       (gained>=minimum||(fourth_&&!fourthUsed_&&gained>=(std::max)(.1f,line_)))){
                        bool conversion=fourth_&&!fourthUsed_&&gained>=(std::max)(.1f,line_);
                        if(conversion)fourthUsed_=true;else used_=true;
                        out={conversion?Event::FourthConversion:Event::LongCatch,++serial_,side_,gained};
                    }
                }else if(flight_&&kind==2){
                    // A fresh interception result is already authoritative.
                    // The global possession label can trail the actual catch.
                    used_=true;if(side_==1||p.game.neutral)out={Event::Interception,++serial_,1-side_,0};
                }
                // Never use an airborne ball, a loose ball or a kicked ball as
                // a runner. Short completions remain eligible for one later YAC burst.
                bool carrying=!gainCancelled_&&!p.fumble&&p.game.phase==3&&held&&p.offense==side_&&((kind==1&&caught_)||running);
                float gain=(runnerSpot-spot_)*direction_;
                // Reject a transient stale/teleported carrier coordinate rather
                // than instantly latching a long-gain reaction on a short catch.
                bool coherent=carrying&&std::isfinite(runnerSpot)&&runnerAt_&&now>=runnerAt_&&
                    std::fabs(runnerSpot-runnerLast_)<=3.5f+30.f*static_cast<float>(now-runnerAt_)*.001f;
                if(coherent){runnerLast_=runnerSpot;runnerAt_=now;if(gain>=0&&gain<=110)measuredGain_=(std::max)(measuredGain_,gain);}
                if(coherent&&caught_&&gain-catchGain_>.5f)afterCatchRun_=true;
                float minimum=std::isfinite(c.minimumGainYards)?(std::clamp)(c.minimumGainYards,10.0f,60.0f):15;
                // Camera/companion buildup is independent of the configurable cheer cutoff.
                constexpr float rampStart=RunShakeStartYards,rampWidth=RunShakePeakYards-RunShakeStartYards;
                bool runningGain=!caught_||afterCatchRun_;
                if((!used_||buildup_.id)&&own&&coherent&&runningGain&&std::fabs(runnerSpot)<=60&&gain>=rampStart&&gain<=110){
                    float progress=(std::clamp)((gain-rampStart)/rampWidth,.0001f,1.f);
                    if(progress>=buildup_.progress)buildup_={began_,progress,gain};
                }
                if(!out.id&&!used_&&own&&coherent&&std::fabs(runnerSpot)<=60&&gain<=110&&
                   ((runningGain&&gain>=minimum)||(fourth_&&!fourthUsed_&&gain>=(std::max)(.1f,line_)))){
                    bool conversion=fourth_&&!fourthUsed_&&gain>=(std::max)(.1f,line_);
                    if(conversion)fourthUsed_=true;else used_=true;
                    out={conversion?Event::FourthConversion:caught_?Event::RunAfterCatch:Event::LongRun,++serial_,side_,gain};
                }
                // A receiver can be down before a held-runner sample reaches
                // the cutoff. Use the verified completed-play tracker then,
                // including whole-yard credit at the configured gain boundary.
                bool confirmed=settling&&completed&&completed->accepted&&completed->completed&&completed->sequence&&completed->side==side_&&
                    !completed->takeaway&&!completed->setback&&p.offense==side_&&std::isfinite(completed->yards);
                float credited=confirmed?std::round(completed->yards):-1000;
                float passMinimum=afterCatchRun_?minimum:(std::max)(minimum,std::isfinite(c.minimumCatchYards)?(std::clamp)(c.minimumCatchYards,10.f,60.f):20.f);
                float completedMinimum=kind==1?passMinimum:minimum;
                if(!out.id&&!used_&&own&&!gainCancelled_&&!p.fumble&&confirmed&&credited>=completedMinimum&&credited<=110&&((kind==1&&flight_&&caught_)||(kind==0&&!flight_)||(kind==3&&observedRun_&&!flight_))){
                    used_=true;out={kind==1?Event::CompletedPass:Event::LongRun,++serial_,side_,credited};
                }
            }
        }
        if(p.game.phase!=1&&p.game.phase!=3){ready_=false;if(!settling)snapped_=false;}
        // Keep the high-water reaction through lateral/backward cuts. A short
        // carrier-pointer gap is not proof the run ended; real flight, a new
        // record, an interception or changed possession still cancels it.
        bool briefCarrierGap=!held&&runnerAt_&&now>=runnerAt_&&now-runnerAt_<=250;
        if(buildup_.id&&p.game.phase==3&&((!held&&!briefCarrierGap)||p.offense!=side_||record!=record_||
           !((kind==1&&caught_)||((kind==0||kind==3)&&!flight_&&(ball==9||observedRun_)))))buildup_={};
        if(p.game.phase!=3)buildup_={};
        phase_=p.game.phase;return out;
    }
};
}
