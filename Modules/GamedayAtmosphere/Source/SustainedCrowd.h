#pragma once
#include "ExcitementRules.h"

namespace attendance::sustained {
enum class Reason { None, Opening, Upset, LateDrive, DefensiveDown, Overtime, Eruption, Lineup, Result, PassFlight, Postgame, Diagnostics, Background };
inline bool lineupReason(Reason reason){return reason==Reason::Lineup||reason==Reason::Upset;}
inline const char* name(Reason r){
    switch(r){case Reason::Background:return "occupied-crowd background roar";case Reason::Diagnostics:return "diagnostic preview";case Reason::Postgame:return "postgame stadium atmosphere";case Reason::Opening:return "big-game opening";case Reason::Upset:return "developing upset";
    case Reason::LateDrive:return "late scoring opportunity";case Reason::DefensiveDown:return "critical defensive down";
    case Reason::Eruption:return "winning celebration";case Reason::Overtime:return "overtime roar";case Reason::Lineup:return "play-call atmosphere";case Reason::Result:return "successful-play reaction";case Reason::PassFlight:return "pass-in-flight anticipation";default:return "quiet";}
}
struct Intent {
    uintptr_t match=0;unsigned epoch=0;uint64_t updated=0;bool active=false;
    bool spring=false; // One light result route; native playing reactions take precedence.
    Reason reason=Reason::None;int priority=0,offense=-1,size=1;float intensity=0;bool reaction=false,continuous=false;
    uint64_t celebration=0,celebrationUntil=0;int releaseMs=4000;
    uint64_t lineup=0,lineupBegan=0,lineupUntil=0;float layerBoostPercent=0;bool shake=false;
    int shakeTier=0,attackMs=4000;bool supporting=false,cheers=true,cameraPrime=false;
    uint64_t lineupCheer=0,lineupCheerBegan=0,lineupCheerUntil=0;
    uint64_t liveCheer=0,liveCheerUntil=0;int liveType=83;
    uint64_t extraCheer=0,extraCheerUntil=0;int extraType=83;
    uint64_t opponentCheer=0,opponentCheerUntil=0;int opponentType=0,opponentSize=1,supporterSize=0;
    bool opponentPositive=false;
    bool visitorLayer=false; // Added visiting sustained response, not native bus audio.
    bool deepCheers=false;uint64_t resultCheer=0,resultCheerUntil=0;int resultType=91;
    bool scoringReaction=false;uint64_t diagnosticToken=0;
    // Reaction lifetime, independent of the short one-shot cheer request.
    uint64_t reactionId=0;
    bool hostileOpening=false;int lineupType=91;
    bool hostileOpeningResult=false;
    bool motionCompanion=false,supportingOnly=false;
    bool staggered=false,majorLive=false,overlapMilestone=false;
    uint64_t playSequence=0;
    // Stable airborne-pass identity; no cheer, score or camera event.
    uint64_t passFlight=0;
    uint64_t skillAccent=0;
    uint64_t reactionHoldUntil=0,reactionUntil=0;
    bool earnedReaction=false;
    int celebrationType=91;
    int populationSize=3; // Default preserves standalone/diagnostic callers.
};
inline bool passAnticipation(const Intent& wanted){return wanted.passFlight&&wanted.reason!=Reason::None&&!wanted.celebration&&!wanted.motionCompanion&&wanted.reason!=Reason::Result&&wanted.reason!=Reason::Eruption;}
inline Intent supportingIntent(Intent wanted){
    if(!wanted.supporting)wanted.reason=Reason::None;
    else if(passAnticipation(wanted)){
        wanted.reason=Reason::PassFlight;wanted.priority=2;wanted.continuous=true;
        wanted.attackMs=500;wanted.releaseMs=1200;wanted.lineup=0;
    }else if(wanted.reason!=Reason::Eruption&&!wanted.diagnosticToken&&!wanted.hostileOpening&&!wanted.motionCompanion&&!wanted.earnedReaction&&wanted.reason!=Reason::Result)wanted.attackMs=4000;
    return wanted;
}
inline Intent select(const energy::Settings& c,const energy::Play& p,const energy::Model& model,
                     uintptr_t match,unsigned epoch,uint64_t now,bool allowed,float permanentFraction=1){
    Intent out;out.match=match;out.epoch=epoch;out.updated=now;out.offense=p.offense;
    const State& s=p.game;
    bool phase=s.phase==1||s.phase==3||s.phase==4||s.phase==5||s.phase==6||s.phase==7;
    if(!allowed||!c.enabled||!c.sustained||!valid(s)||!phase||model.heldFraction<=0)return out;
    out.active=true;
    // This is excitement, not a gain multiplier. Never reduce native intensity.
    float heat=(std::clamp)((model.score-35)/65.0f,0.0f,1.0f);
    out.intensity=1000*heat*std::sqrt(model.heldFraction);
    if(s.phase==7){
        double age=now/1000.0-model.victoryAt;
        if(model.victoryAt<0||age<0||age>=10)out.intensity=0;
        else out.intensity*=age<=4?1.0f:static_cast<float>((10-age)/6);
        return out; // Match the existing winning handoff, then restore native excitement.
    }
    double surgeAge=now/1000.0-model.surge.began;
    out.reaction=model.surge.peak>0&&surgeAge>=0&&surgeAge<16;
    if(out.reaction)return out; // Let the game's celebration have the stage.
    // The opening channel holds across snaps and play selection until the
    // opening possession ends. The wider opening mix can still fade afterward.
    if(model.openingWeight>0.2f&&model.openingProfile.fade<0){
        out.reason=Reason::Opening;out.priority=3;out.continuous=true;out.size=3;
        out.intensity=(std::max)(out.intensity,1000.0f*std::sqrt((std::clamp)(permanentFraction,0.0f,1.0f)));
        return out;
    }
    if(s.phase!=1&&s.phase!=6)return out; // Added anticipation ends at the snap.
    int margin=std::abs(s.homeScore-s.awayScore);
    if(margin>16)return out;
    auto choose=[&](Reason reason,int priority){if(priority>out.priority){out.reason=reason;out.priority=priority;}};
    if(s.phase==1&&(energy::normalDown(p)||energy::overtimeTry(p))){
        if(c.upsetWatch&&model.stage>=2&&model.score>=55&&(s.neutral||p.offense==0||p.down>=3))choose(Reason::Upset,2);
        if(c.chaos&&competitiveOvertime(s)&&c.overtimeAtmosphere)choose(Reason::Overtime,4);
        if(c.chaos&&(s.neutral||p.offense==1)&&(p.down==3||p.down==4)&&energy::anticipationPoints(c,p,model.stage)>=18)choose(Reason::DefensiveDown,4);
        // Include the approach to the red zone, with the strongest energy inside it.
        bool drive=s.quarter==4&&s.seconds<=90&&margin<=8&&(s.neutral||p.offense==0)&&p.toGoal>=0&&p.toGoal<=35;
        if(drive)choose(Reason::LateDrive,5);
    }
    if(out.reason!=Reason::None){
        float tension=(std::clamp)((std::max)(model.anticipation,20.0f)/50.0f,0.0f,1.0f);
        if(out.reason==Reason::LateDrive&&p.toGoal>=0)tension=(std::max)(tension,1-(std::clamp)(p.toGoal/50.0f,0.0f,1.0f));
        if(out.reason==Reason::Opening)tension=(std::max)(tension,model.openingWeight);
        float fraction=out.reason==Reason::Opening?(std::clamp)(permanentFraction,0.0f,1.0f):model.heldFraction;
        out.intensity=(std::max)(out.intensity,(550+450*tension)*std::sqrt(fraction));
        out.size=out.reason==Reason::Opening||tension>=0.75f||model.score>=85?3:model.score>=60?2:1;
    }
    return out;
}
inline bool fresh(const Intent& intent,uintptr_t match,unsigned epoch,uint64_t now){
    return intent.active&&intent.match&&intent.match==match&&intent.epoch==epoch&&now>=intent.updated&&now-intent.updated<=750;
}
// One extra voice, bounded episodes, and a quiet gap. Higher-priority contexts
// can take over an existing voice without sending another start event.
struct Episode {
    bool playing=false,continuous=false;Reason reason=Reason::None;int priority=0,offense=-1;
    uint64_t began=0,quietUntil=0,lineup=0,reactionId=0,passFlight=0;uintptr_t match=0;unsigned epoch=0;
    uint64_t openingBegan=0;uintptr_t openingMatch=0;unsigned openingEpoch=0;
    bool openingExpired(const Intent& wanted,uint64_t now)const{
        return wanted.reason==Reason::Opening&&wanted.continuous&&openingMatch==wanted.match&&openingEpoch==wanted.epoch&&
            (now<openingBegan||now-openingBegan>=300000);
    }
    bool mustStop(const Intent& wanted,uint64_t now)const{
        return playing&&(!fresh(wanted,match,epoch,now)||wanted.reason==Reason::None||wanted.reaction||
            (continuous?(!wanted.continuous||openingExpired(wanted,now)):
                ((wanted.offense>=0&&offense>=0&&wanted.offense!=offense)||now<began||now-began>=(lineupReason(reason)?28000u:20000u))));
    }
    bool canStart(const Intent& wanted,uint64_t now)const{
        bool freshResult=wanted.reason==Reason::Result&&((wanted.reactionId&&wanted.reactionId!=reactionId)||(wanted.resultCheer&&now<wanted.resultCheerUntil));
        bool freshPass=passAnticipation(wanted)&&wanted.passFlight!=passFlight;
        return !playing&&(now>=quietUntil||wanted.reason==Reason::Eruption||freshResult||freshPass||(wanted.lineup&&wanted.lineup!=lineup))&&fresh(wanted,wanted.match,wanted.epoch,now)&&wanted.reason!=Reason::None&&!wanted.reaction&&!openingExpired(wanted,now);
    }
    void started(const Intent& wanted,uint64_t now){
        playing=true;continuous=wanted.continuous;reason=wanted.reason;priority=wanted.priority;offense=wanted.offense;began=now;match=wanted.match;epoch=wanted.epoch;lineup=wanted.lineup;reactionId=wanted.reactionId;passFlight=wanted.passFlight;
        if(continuous&&wanted.reason==Reason::Opening&&(openingMatch!=match||openingEpoch!=epoch)){openingBegan=now;openingMatch=match;openingEpoch=epoch;}
    }
    void stopped(uint64_t now,unsigned release=4000){quietUntil=now+(continuous?release:(std::max)(8000u,release));playing=continuous=false;reason=Reason::None;priority=0;}
};
}
