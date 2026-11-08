#pragma once
#include "LiveReactions.h"
#include "ReactionShape.h"
namespace attendance::skillswell {
class Model {
    uintptr_t match_=0;unsigned epoch_=0;uint64_t began_=0,id_=0,last_=0;int phase_=-1;
public:
    void reset(){*this=Model{};}
    void observe(const energy::Play& p,uintptr_t match,unsigned epoch,uint64_t now,bool allowed,const live::Outcome& event,uint64_t id){
        if(match!=match_||epoch!=epoch_){reset();match_=match;epoch_=epoch;}
        if(!allowed||!match||!valid(p.game)||p.simulated||now<last_||(last_&&now-last_>750)){began_=id_=0;phase_=-1;last_=now;return;}
        if(p.game.phase==3&&phase_!=3)began_=id_=0;
        if(event.id&&live::softSkill(event.event)&&id){began_=now;id_=id;}
        if(began_&&(now<began_||now-began_>=2000))began_=id_=0;
        phase_=p.game.phase;last_=now;
    }
    sustained::Intent merge(sustained::Intent out,uint64_t now,float fullness,bool earnedAllowed=true)const{
        if(!earnedAllowed||!began_||!sustained::fresh(out,match_,epoch_,now)||out.match!=match_||out.epoch!=epoch_||now<last_||now-last_>750||now<began_||now-began_>=2000||
           !std::isfinite(fullness)||fullness<=0||fullness>1||out.celebration||out.scoringReaction||out.reason==sustained::Reason::Eruption||
           (out.reason==sustained::Reason::Result&&out.priority>=4&&!out.motionCompanion))return out;
        float level=650.f*std::sqrt(fullness)*shape::skillBuildup(static_cast<float>(now-began_),2000);
        // An existing run/return sound keeps its identity and original hold.
        // A short skill accent may lift it, never replace it with a two-second
        // supporting-only voice or send it into an early release.
        if((out.continuous||out.reaction)&&out.intensity>0){
            out.intensity=(std::max)(out.intensity,level);return out;
        }
        out.reason=sustained::Reason::Result;out.priority=(std::max)(out.priority,4);out.reactionId=id_;
        out.skillAccent=id_;
        out.earnedReaction=true;
        out.reactionHoldUntil=began_+450;out.reactionUntil=began_+2000;
        out.supporting=out.supportingOnly=out.motionCompanion=true;out.size=2;out.attackMs=500;out.releaseMs=1550;
        out.continuous=now-began_<450;out.reaction=!out.continuous;
        out.intensity=(std::max)(out.intensity,level);
        return out;
    }
};
}
