#pragma once
#include "SustainedCrowd.h"
#include "ReactionShape.h"
#include "ReactionCatalog.h"

namespace attendance::soundhandoff {
// Carry an already-earned crowd level through the next reaction's attack.
// This never creates an event, restarts a voice or changes gain/timer limits.
class Model {
    uintptr_t match_=0;unsigned epoch_=0;uint64_t last_=0,began_=0,score_=0;
    bool eruption_=false,voiced_=false;float prior_=0,floor_=0;
    sustained::Reason reason_=sustained::Reason::None;int priority_=0,phase_=-1;
    bool scoring_=false;uint64_t live_=0,skill_=0,result_=0;
public:
    void reset(){*this=Model{};}
    sustained::Intent merge(sustained::Intent out,int phase,uint64_t now,float fullness,bool earnedAllowed=true){
        if(out.match!=match_||out.epoch!=epoch_){reset();match_=out.match;epoch_=out.epoch;}
        if(!earnedAllowed||!out.active||!sustained::fresh(out,out.match,out.epoch,now)||!std::isfinite(fullness)||fullness<=0||fullness>1||
           now<last_||(last_&&now-last_>750)){reset();return out;}
        float presence=std::sqrt(fullness);
        bool eruption=out.reason==sustained::Reason::Eruption&&out.celebration;
        bool scoring=eruption||(out.scoringReaction&&out.reason==sustained::Reason::Result&&out.reactionId);
        bool reaction=(eruption||(out.reason==sustained::Reason::Result&&out.reactionId))&&(!scoring||phase!=3);
        bool negative=out.liveCheer&&catalog::negative(out.liveType);
        uint64_t id=eruption?out.celebration:out.reactionId;
        bool newSnap=phase==3&&phase_!=3;
        if(newSnap&&(reason_==sustained::Reason::Result||reason_==sustained::Reason::Eruption)){began_=0;voiced_=false;}
        bool changed=id!=score_||eruption!=eruption_||scoring!=scoring_||reason_!=out.reason||
            (out.liveCheer&&out.liveCheer!=live_)||(out.skillAccent&&out.skillAccent!=skill_)||(out.resultCheer&&out.resultCheer!=result_);
        if(reaction&&changed){
            began_=!negative&&voiced_&&last_&&now-last_<=250&&out.priority>=priority_?now:0;
            floor_=prior_;
        }
        if(!reaction||negative)began_=0;
        if(reaction&&began_&&now>=began_&&now-began_<500){
            auto age=now-began_;
            float hold=age<=250?1.f:1-shape::smooth(static_cast<float>(age-250)/250.f);
            out.intensity=(std::max)(out.intensity,floor_*presence*hold);
        }
        voiced_=!negative&&out.reason!=sustained::Reason::None&&
            (out.continuous||out.reaction)&&std::isfinite(out.intensity)&&out.intensity>0;
        prior_=voiced_?(std::clamp)(out.intensity/presence,0.f,1000.f):0;last_=now;
        score_=id;eruption_=eruption;scoring_=scoring;reason_=out.reason;priority_=out.priority;phase_=phase;
        if(out.liveCheer)live_=out.liveCheer;if(out.skillAccent)skill_=out.skillAccent;if(out.resultCheer)result_=out.resultCheer;
        return out;
    }
};
}
