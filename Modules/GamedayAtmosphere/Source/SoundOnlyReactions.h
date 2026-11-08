#pragma once
#include "CoreAtmosphere.h"
namespace attendance::soundonly {
inline bool eligible(const core::Settings& c,const energy::Play& p,float fullness,bool known,bool enabled=true){
    return enabled&&known&&std::isfinite(fullness)&&fullness>0&&fullness<=1&&c.enabled&&c.lineups&&core::normalLimit(c)>0&&
        valid(p.game)&&!p.simulated&&!energy::clockManagement(p)&&
        (p.game.phase==1||p.game.phase==3||p.game.phase==4||p.game.phase==5);
}
struct Outcome {uint64_t sequence=0;int type=0;};
class Model {
    uintptr_t match_=0;unsigned epoch_=0;uint64_t seen_=0;bool scored_=false;
public:
    void reset(){*this=Model{};}
    Outcome observe(const energy::Outcome& r,const energy::Play& snap,const energy::Play& p,uintptr_t match,unsigned epoch,
                    bool allowed,bool layered,bool coveredByLive,bool situational=true){
        if(match_!=match||epoch_!=epoch){reset();match_=match;epoch_=epoch;}
        if(!r.accepted||!r.completed||!r.sequence)return {};
        int points=r.side==1?r.awayPoints:r.homePoints;
        bool scoring=r.safety||points==3||(points>=6&&points<=8)||(energy::overtimeTry(snap)&&points==2);
        if(seen_==r.sequence&&(!scoring||scored_))return {};
        if(seen_!=r.sequence){seen_=r.sequence;scored_=false;}scored_=scored_||scoring;
        // Consume unavailable/covered results now, never queue them for later
        // arrival, unpause, setting changes or a new fullness threshold.
        if(!allowed||layered||!match||p.simulated||energy::clockManagement(snap)||(!energy::normalDown(snap)&&snap.down!=0&&snap.down!=-1))return {};
        if(!p.game.neutral&&energy::overtimeTry(snap)&&snap.selectedType!=21&&snap.offense==0&&!r.homePoints&&!r.awayPoints&&p.offense!=snap.offense)
            return situational?Outcome{r.sequence,95}:Outcome{};
        if(!p.game.neutral&&(r.visitorScore||(r.takeaway&&r.side==1)))return situational?Outcome{r.sequence,95}:Outcome{};
        if(r.side<0||r.side>1||(!p.game.neutral&&r.side!=0)||(!scoring&&coveredByLive))return {};
        if(scoring||r.firstDown||core::positiveResult(r.event))return {r.sequence,core::resultReaction(r)};
        return {};
    }
};
// Native one-shot cheers keep the current native excitement/volume. Only
// verified airborne-pass and skill swells may add a short supporting sound;
// no passive roar, layered follow-up, celebration or volume premium is added.
inline sustained::Intent restrain(sustained::Intent out,float fullness){
    out.layerBoostPercent=0;out.shake=false;out.shakeTier=0;out.cameraPrime=false;
    out.lineup=out.lineupCheer=out.celebration=out.resultCheer=0;
    out.staggered=out.deepCheers=out.majorLive=out.overlapMilestone=false;
    out.scoringReaction=false;out.size=(std::min)(out.size,2);
    if(out.passFlight||out.skillAccent){
        out.intensity=(std::min)(out.intensity,450.f*std::sqrt((std::clamp)(fullness,0.f,1.f)));
        out.supportingOnly=true;
    }else{
        out.reason=sustained::Reason::None;out.intensity=0;out.continuous=out.reaction=out.supporting=false;
    }
    return out;
}
}
