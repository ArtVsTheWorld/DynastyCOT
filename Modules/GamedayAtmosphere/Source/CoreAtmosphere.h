#pragma once
#include "SustainedCrowd.h"
#include "AudioLevels.h"
#include "AttendanceAudio.h"
#include "PostPlayAtmosphere.h"

namespace attendance::core {
inline constexpr float NormalMaximumDb=1.9382003f, HighLeverageMaximumDb=2.6066754f,EruptionMaximumDb=3.521825f;
inline constexpr float NormalMaximumPercent=25,HighLeverageMaximumPercent=35,EruptionMaximumPercent=50;
inline constexpr double EruptionSeconds=45,RoarHoldSeconds=25,OpeningReleaseSeconds=20;
// Absolute lower bound. Qualified plays supply their stricter size-based gate.
inline constexpr float ReactionMinimumFullness=.50f;
inline float competition(const State& s){float x=(std::clamp)((std::abs(s.homeScore-s.awayScore)-16)/8.0f,0.0f,1.0f);return 1-x*x*(3-2*x);}
struct Settings {bool enabled=true,opening=true,eruptions=true,overtime=true,lineups=true,cameraShake=true;float maximumPercent=NormalMaximumPercent,eruptionPercent=EruptionMaximumPercent,lineupPercent=50,chantMaximumPercent=15,highLeveragePercent=HighLeverageMaximumPercent;bool packedHouse=true,driveEscalation=true;float bigGameMinimum=.55f;bool skillMoves=true,staggeredCheers=true,contextReactions=true,upsetWatch=true;};
inline float normalLimit(const Settings& c){return c.enabled&&std::isfinite(c.maximumPercent)?c.maximumPercent>=NormalMaximumPercent?NormalMaximumDb:volume::boostDb(c.maximumPercent):0;}
inline float highLeverageLimit(const Settings& c){return (std::max)(normalLimit(c),std::isfinite(c.highLeveragePercent)?volume::boostDb((std::clamp)(c.highLeveragePercent,0.0f,HighLeverageMaximumPercent)):0);}
inline float eruptionLimit(const Settings& c){return (std::max)(normalLimit(c),std::isfinite(c.eruptionPercent)?c.eruptionPercent>=EruptionMaximumPercent?EruptionMaximumDb:volume::boostDb(c.eruptionPercent):0);}
inline float callbackLimit(const Settings& c,float observed,uint64_t now,uint64_t until,uint64_t highUntil=0){
    float normal=normalLimit(c);if(normal<=0||!std::isfinite(observed))return normal;
    float ceiling=now<highUntil?highLeverageLimit(c):normal;
    if(c.eruptions&&now<until)ceiling=(std::max)(ceiling,eruptionLimit(c));
    return (std::clamp)(observed,normal,ceiling);
}
inline bool decisive(energy::Event e){return e==energy::Event::LastSecondLead||e==energy::Event::OvertimeWin||e==energy::Event::GameWinningScore;}
inline bool positiveResult(energy::Event e){
    using energy::Event;
    return e==Event::ExplosiveGain||e==Event::ThirdDownConversion||e==Event::FourthDown||e==Event::BackfieldStop||
        e==Event::ThirdStop||e==Event::FourthStop||e==Event::Turnover||e==Event::FumbleRecovery||e==Event::GoalLine||
        e==Event::HomeScore||e==Event::LongTouchdown||e==Event::TakeLead||e==Event::TieScore||e==Event::PickSix||e==Event::Sack||e==Event::ForcedFumble||e==Event::MadeFieldGoal||e==Event::OvertimeConversion||e==Event::ConversionStop||e==Event::Safety||e==Event::BlockedFieldGoal||e==Event::GameSealingStop||e==Event::GameSealingFirstDown;
}
// Listening-tested families: yardage/conversions and defensive stops. The
// native game retains its own opponent-success reactions.
inline int resultReaction(const energy::Outcome& r){
    using energy::Event;
    if(r.safety)return 91; // Same cheer family as a touchdown, with a shorter profile.
    switch(r.event){
    case Event::ConversionStop:case Event::BackfieldStop:case Event::ThirdStop:case Event::FourthStop:
    case Event::GameSealingStop:case Event::Turnover:case Event::FumbleRecovery:case Event::GoalLine:case Event::Sack:case Event::ForcedFumble:case Event::BlockedFieldGoal:return 59;
    case Event::GameSealingFirstDown:return 91;
    case Event::ExplosiveGain:return r.yards>=40?91:83;
    case Event::OvertimeConversion:case Event::ThirdDownConversion:case Event::FourthDown:return 83;
    default:return r.firstDown?83:91;
    }
}
struct Model {
    energy::OpeningProfile openingProfile;
    float parent=0,beds=0,limit=NormalMaximumDb,openingWeight=0,intensity=0,carryDb=0;
    double eruptionAt=-1,postgameAt=-1,openingReleaseAt=-1;energy::Event event=energy::Event::None;
    uint64_t sequence=0,celebration=0;bool erupting=false,highLeverage=false,sawLive=false,postgameSeen=false,postgameHeld=false,postgameMenu=false;int winner=0;
    double eruptionSeconds=EruptionSeconds,roarHoldSeconds=RoarHoldSeconds;
    double resultCheerAt=-1000;uint64_t resultCheer=0,resultSequence=0;int resultType=91;
    postplay::Profile resultProfile;float resultWeight=0;
    bool resultScoring=false,resultLongTouchdown=false,resultOvertime=false;
    float eruptionMinimum=ReactionMinimumFullness,resultMinimum=ReactionMinimumFullness;
    void reset(){*this=Model{};}
    float eruptionWeight(double now)const{
        double age=now-eruptionAt;if(postgameMenu||eruptionAt<0||age<0)return 0;
        if(postgameHeld)return static_cast<float>((std::min)(age/.25,1.0));
        if(age>=eruptionSeconds)return 0;
        return age<.25?shape::smooth(static_cast<float>(age/.25)):age<=eruptionSeconds-25?1.0f:1-shape::smooth(static_cast<float>((age-(eruptionSeconds-25))/25));
    }
    bool finishPostgame(){
        if(postgameMenu)return false;
        // The stats screen can be the first observed final snapshot. Latch
        // it even before update() sees phase 7, so highlights/unpause cannot
        // start a late winning eruption or revive the final-play carry.
        bool held=postgameHeld;postgameSeen=postgameMenu=true;postgameHeld=erupting=false;parent=beds=intensity=0;return held;
    }
    uint64_t eruptionLease(uint64_t now)const{
        return !erupting?0:postgameHeld?now+750:static_cast<uint64_t>((eruptionAt+eruptionSeconds)*1000);
    }
    bool react(const Settings& c,const energy::Outcome& result,double now,const State& s,bool layeredActive=false,bool reactionsAllowed=true,float minimum=ReactionMinimumFullness,unsigned distanceExtraMs=0,bool durationOwned=false){
        if(result.accepted&&!s.neutral&&(result.visitorScore||(result.takeaway&&result.side==1))){eruptionAt=-1;postgameHeld=false;resultCheerAt=-1000;event=energy::Event::None;return false;}
        if(result.accepted&&result.setback&&!s.neutral)resultCheerAt=-1000;
        if(postgameMenu)return false;
        if(!reactionsAllowed||!std::isfinite(minimum)||minimum<ReactionMinimumFullness||minimum>1)return false;
        bool own=result.side==0||(s.neutral&&result.side==1);
        // Verified big plays receive their own bounded reaction, including at
        // ordinary matchups. Routine short first downs still need active layers.
        bool scoring=result.safety||result.homePoints>=6||result.awayPoints>=6||(overtime(s)&&(result.homePoints==2||result.awayPoints==2));
        bool otMoment=overtime(s)&&(result.event==energy::Event::OvertimeConversion||result.event==energy::Event::ConversionStop||result.event==energy::Event::MadeFieldGoal||result.event==energy::Event::ThirdStop||result.event==energy::Event::FourthStop);
        bool scoreUpgrade=scoring&&result.sequence==resultSequence&&!resultScoring;
        bool longTouchdown=c.eruptions&&layeredActive&&postplay::longCloseTouchdown(result);
        if((!durationOwned||scoring||result.contextClincher)&&c.enabled&&c.lineups&&normalLimit(c)>0&&result.accepted&&result.completed&&result.sequence&&
           (result.sequence!=resultSequence||scoreUpgrade)&&own&&competition(s)>0&&(result.safety||positiveResult(result.event)||(layeredActive&&result.firstDown))&&std::isfinite(now)&&
           (now-resultCheerAt>=4||scoreUpgrade||otMoment)&&(eruptionAt<0||now-eruptionAt>=eruptionSeconds||otMoment)){
            resultSequence=result.sequence;resultCheerAt=now;resultType=resultReaction(result);resultProfile=postplay::choose(result,s,longTouchdown);resultLongTouchdown=longTouchdown;resultProfile.release+=(std::min)(distanceExtraMs,8000u)*.001;resultScoring=scoring||result.contextClincher;resultOvertime=otMoment;resultMinimum=minimum;++resultCheer;
        }
        if(!c.enabled||!c.eruptions||normalLimit(c)<=0||!result.accepted||!std::isfinite(now))return false;
        // The verified tracker supplies only home scores, or either side at a neutral venue.
        if(result.side<0||result.side>1||(result.side==1&&!s.neutral))return false;
        energy::Event selected=result.event;
        bool contextStop=c.contextReactions&&result.contextEruption&&result.completed&&selected==energy::Event::GameSealingStop;
        bool early=result.completed&&result.firstTouchdown&&result.openingPlay>=1&&result.openingPlay<=energy::OpeningTouchdownPlayLimit&&s.quarter==1&&layeredActive;
        bool otTakeaway=result.completed&&result.takeaway&&overtime(s);
        bool lateTakeaway=energy::sealsRegulationTakeaway(result,s);
        bool hailMary=result.accepted&&result.completed&&result.hailMary&&(result.side==0?result.homePoints:result.awayPoints)>=6&&layeredActive;
        if(early)selected=energy::Event::EarlyTouchdown;
        if(hailMary)selected=energy::preferred(selected,energy::Event::HailMaryTouchdown);
        if(otTakeaway)selected=energy::preferred(selected,energy::Event::OvertimeTakeaway);
        if(lateTakeaway)selected=energy::preferred(selected,energy::Event::LateTakeaway);
        if((selected==energy::Event::EarlyTouchdown&&!early)||(selected==energy::Event::OvertimeTakeaway&&!otTakeaway)||(selected==energy::Event::LateTakeaway&&!lateTakeaway)||(selected==energy::Event::HailMaryTouchdown&&!hailMary))return false;
        if(!decisive(selected)&&!contextStop&&selected!=energy::Event::EarlyTouchdown&&selected!=energy::Event::OvertimeTakeaway&&selected!=energy::Event::LateTakeaway&&selected!=energy::Event::HailMaryTouchdown)return false;
        if(selected==energy::Event::LastSecondLead&&(s.quarter!=4||s.seconds<0||s.seconds>30))return false;
        bool same=result.sequence&&result.sequence==sequence;
        if(same&&energy::priority(selected)<=energy::priority(event))return false;
        // A postgame confirmation upgrades the existing score; never restart its roar.
        if(same&&eruptionAt>=0){event=selected;if(selected==energy::Event::OvertimeWin){eruptionSeconds=(std::max)(eruptionSeconds,60+(std::min)(distanceExtraMs,8000u)*.001);roarHoldSeconds=eruptionSeconds-20;}return false;}
        if(eruptionAt>=0&&now-eruptionAt<8&&energy::priority(selected)<=energy::priority(event))return false;
        eruptionAt=now;event=selected;sequence=result.sequence;winner=result.side;eruptionMinimum=minimum;
        eruptionSeconds=(selected==energy::Event::OvertimeWin||selected==energy::Event::OvertimeTakeaway?60:45)+(std::min)(distanceExtraMs,8000u)*.001;roarHoldSeconds=eruptionSeconds-20;
        ++celebration;return true;
    }
    void update(const Settings& c,const energy::Play& p,double now,float fraction,float permanentFraction,float leverage=0,bool reactionsAllowed=true,float reactionMinimum=ReactionMinimumFullness){
        if(!valid(p.game)||!std::isfinite(now)||!std::isfinite(fraction)||!std::isfinite(permanentFraction))return;
        const State& s=p.game;float normal=normalLimit(c);limit=normal;
        if(fraction<eruptionMinimum){eruptionAt=-1;postgameHeld=false;event=energy::Event::None;}
        if(fraction<resultMinimum)resultCheerAt=-1000;
        (void)permanentFraction;openingWeight=0;
        int margin=std::abs(s.homeScore-s.awayScore);
        if(s.phase!=6&&s.phase!=7)sawLive=true;
        if(s.phase==7&&!postgameSeen){
            postgameSeen=true;State story=energyPerspective(s);
            bool carrying=eruptionAt>=0&&now>=eruptionAt&&now-eruptionAt<eruptionSeconds;
            bool upset=story.homeScore>story.awayScore&&margin<=8&&upsetDiscrepancy(s)>=0.45f;
            if(sawLive&&c.enabled&&c.eruptions&&normal>0&&upset&&!carrying&&reactionsAllowed&&std::isfinite(reactionMinimum)&&reactionMinimum>=ReactionMinimumFullness&&fraction>=reactionMinimum){
                eruptionAt=now;eruptionSeconds=45;roarHoldSeconds=25;event=energy::Event::UpsetWin;winner=s.neutral&&s.awayScore>s.homeScore?1:0;eruptionMinimum=reactionMinimum;++celebration;
            }
            bool winnerMatches=winner==0?s.homeScore>s.awayScore:s.neutral&&s.awayScore>s.homeScore;
            postgameHeld=c.enabled&&c.eruptions&&normal>0&&winnerMatches&&eruptionAt>=0&&now>=eruptionAt&&now-eruptionAt<eruptionSeconds;
        }
        float weight=c.enabled&&c.eruptions?eruptionWeight(now):0;
        erupting=!postgameMenu&&eruptionAt>=0&&now>=eruptionAt&&(postgameHeld||now-eruptionAt<eruptionSeconds)&&c.enabled&&c.eruptions;
        if(normal<=0||!c.enabled){parent=beds=intensity=openingWeight=resultWeight=0;erupting=highLeverage=false;return;}
        resultWeight=c.lineups&&s.phase!=7&&s.phase!=6?postplay::envelope(now-resultCheerAt,resultProfile)*competition(s):0;
        // A new live snap ends an old result. Never cheer yesterday's play
        // over the next live ball; a fresh completed outcome can start again.
        if(s.phase==3){resultCheerAt=-1000;resultWeight=0;}
        leverage=std::isfinite(leverage)?(std::clamp)(leverage,0.0f,1.0f):0;highLeverage=leverage>0;
        limit=(std::max)(highLeverage?highLeverageLimit(c):normal,normal+(eruptionLimit(c)-normal)*weight);
        float base=balance::crowdBoost(fraction,c.maximumPercent);
        float moment=(std::clamp)(c.highLeveragePercent,0.0f,HighLeverageMaximumPercent);
        base+=((std::max)(base,moment)-base)*leverage*std::sqrt((std::clamp)(fraction,0.0f,1.0f));
        float peak=(std::clamp)(c.eruptionPercent,0.0f,EruptionMaximumPercent);
        float percent=base+((std::max)(base,peak)-base)*weight*std::sqrt((std::clamp)(fraction,0.0f,1.0f));
        parent=(std::min)(volume::boostDb(percent),limit);beds=0;
        float presence=std::sqrt((std::clamp)(fraction,0.0f,1.0f));
        intensity=(std::max)(erupting?1000*weight*presence:0,resultWeight*(650+350*resultProfile.strength)*presence);

    }
    sustained::Intent intent(const Settings& c,const energy::Play& p,uintptr_t match,unsigned epoch,uint64_t now,bool allowed)const{
        sustained::Intent out;out.match=match;out.epoch=epoch;out.updated=now;out.offense=p.offense;
        // Phase 2 can be a live timeout/camera transition. Existing result
        // envelopes may finish there; this creates no new play or lineup.
        bool phase=p.game.phase>=1&&p.game.phase<=7;
        if(!allowed||!c.enabled||normalLimit(c)<=0||!phase||postgameMenu)return out;
        out.active=true;out.intensity=intensity;
        out.staggered=c.staggeredCheers;
        if(c.lineups&&resultWeight>0&&p.game.phase!=3&&p.game.phase!=6&&p.game.phase!=7&&resultCheer&&now/1000.0>=resultCheerAt&&now/1000.0-resultCheerAt<2.5){out.resultType=resultType;out.resultCheer=resultCheer;out.resultCheerUntil=static_cast<uint64_t>((resultCheerAt+2.5)*1000);}
        if(erupting&&c.eruptions){
            if(postgameHeld||now/1000.0-eruptionAt<roarHoldSeconds){
                out.celebration=celebration;out.celebrationUntil=static_cast<uint64_t>((eruptionAt+6)*1000);
                if(!resultOvertime||resultCheerAt<eruptionAt)out.resultCheer=0;out.deepCheers=true;
                out.reason=sustained::Reason::Eruption;out.priority=6;out.continuous=true;out.size=3;out.attackMs=500;out.releaseMs=20000;out.supporting=true;
                return out;
            }
            // Keep the volume envelope's final ten seconds, but let a still
            // eligible OT/drive buildup carry the voice rather than briefly
            // fading it to quiet before returning to the same atmosphere.
        }
        if(!erupting&&resultWeight>0){
            out.earnedReaction=true;
            out.reactionHoldUntil=static_cast<uint64_t>((resultCheerAt+resultProfile.hold)*1000);
            out.reactionUntil=static_cast<uint64_t>((resultCheerAt+resultProfile.hold+resultProfile.release)*1000);
            out.scoringReaction=resultScoring;
            out.reactionId=resultSequence;
            out.reason=sustained::Reason::Result;out.priority=5;out.continuous=true;out.size=resultProfile.size;
            out.attackMs=500;out.releaseMs=static_cast<int>(resultProfile.release*1000);out.supporting=resultProfile.strength>=.68f;
            out.deepCheers=resultProfile.layered;out.cheers=false;
            if(now/1000.0-resultCheerAt>=resultProfile.hold){out.reaction=true;out.continuous=false;out.supporting=false;}
        }
        return out;
    }
};
}
