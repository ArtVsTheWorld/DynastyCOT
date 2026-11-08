#pragma once
#include "LiveReactions.h"
#include "ReactionShape.h"
namespace attendance::live {
class ReturnAtmosphere {
    uintptr_t match_=0;unsigned epoch_=0;uint64_t began_=0,last_=0,tail_=0,heldAt_=0,ramp_=0;
    int side_=-1;bool active_=false,rampOnly_=false;float level_=1;unsigned release_=5000,hold_=0;
    int phase_=-1;
    uint64_t liveBegan_=0;bool exhausted_=false;
    bool pendingPossession_=false,lostPossession_=false;
public:
    void reset(){*this=ReturnAtmosphere{};}
    bool lostPossession()const{return lostPossession_;}
    void observe(const energy::Play& p,uintptr_t match,unsigned epoch,uint64_t now,bool allowed,const Outcome& event,bool held,bool fumble,unsigned release=5000,RunBuildup buildup={},unsigned distanceExtra=0,bool durationCarry=false){
        if(match!=match_||epoch!=epoch_){reset();match_=match;epoch_=epoch;}
        // A new defensive impact replaces a previous offensive return. It
        // has its own short sound/motion envelope, not a continuing carrier.
        if(event.id&&event.event==Event::ForcedFumble){active_=false;lostPossession_=pendingPossession_=false;}
        if(!allowed||!match||p.simulated||!valid(p.game)||fumble||now<last_||(last_&&now-last_>750)){active_=false;last_=now;return;}last_=now;
        bool newSnap=p.game.phase==3&&phase_!=3;phase_=p.game.phase;
        if(newSnap){active_=false;tail_=0;liveBegan_=now;exhausted_=false;lostPossession_=pendingPossession_=false;}
        if(p.game.phase==3&&liveBegan_&&now>=liveBegan_&&now-liveBegan_>=45000){active_=false;exhausted_=true;}
        if(exhausted_)return;
        if(!tail_)release_=(std::clamp)(release,1000u,8000u)+(std::min)(distanceExtra,8000u);
        bool runEvent=event.id&&(event.event==Event::LongRun||event.event==Event::RunAfterCatch);
        bool growing=buildup.id&&std::isfinite(buildup.progress)&&buildup.progress>0&&p.game.phase==3&&(held||(heldAt_&&now>=heldAt_&&now-heldAt_<=250));
        if(growing){
            if(!active_||(rampOnly_&&ramp_!=buildup.id)){began_=heldAt_=now;tail_=0;side_=p.offense;active_=rampOnly_=true;level_=0;lostPossession_=pendingPossession_=false;}
            // Later runner samples refine a confirmed catch/return; they
            // cannot demote it into an unconfirmed buildup or lose its hold.
            ramp_=buildup.id;
            level_=(std::max)(level_,.25f+.75f*(std::clamp)(buildup.progress,0.f,1.f));
        }
        if(event.id&&(runEvent||event.event==Event::LongCatch||event.event==Event::KickReturn||event.event==Event::PuntReturn||event.event==Event::Interception||event.event==Event::FumbleRecovery)){
            if(!growing)began_=heldAt_=now;
            tail_=0;side_=event.side;active_=true;rampOnly_=false;level_=1;
            lostPossession_=false;pendingPossession_=event.event==Event::Interception||event.event==Event::FumbleRecovery;
        }
        if(durationCarry&&!active_&&p.game.phase==3&&held){began_=heldAt_=now;tail_=0;side_=p.offense;active_=true;rampOnly_=false;level_=1;}
        if(!active_)return;
        // The live interception signal can precede the global possession label.
        // Keep its newly admitted sound alive through that bounded handoff.
        if(held&&p.offense==side_)pendingPossession_=false;
        bool possessionPending=pendingPossession_&&now>=began_&&now-began_<1000&&p.game.phase==3;
        if(p.game.phase==3&&p.offense!=side_&&!possessionPending){lostPossession_=true;active_=false;return;}
        if(now<began_||(p.game.phase==3&&now-began_>=45000)){active_=false;return;}
        if(held)heldAt_=now;
        // A tackled carrier may be released before the native whistle phase.
        // Confirmed reactions keep their voice until that phase arrives, so
        // the full release is AFTER the whistle. An unconfirmed gain buildup
        // can still fade when abandoned. Fumbles/possession/world changes and
        // the existing 45-second live guard remain independent cancellations.
        if(!tail_&&(p.game.phase!=3||(rampOnly_&&!growing))){tail_=now;hold_=rampOnly_?0:shape::tailHold(release_);}
        if(tail_&&now-tail_>=release_)active_=false;
    }
    sustained::Intent merge(sustained::Intent out,uint64_t now,float fullness)const{
        if(!active_||!out.active||out.match!=match_||out.epoch!=epoch_||now<last_||now-last_>750||out.celebration||out.scoringReaction||out.reason==sustained::Reason::Eruption)return out;
        // The official post-play response has its own longer hold/release.
        // A short return tail must not fade that response out underneath it
        // and expose it again at a higher level when this tail expires.
        if(out.reason==sustained::Reason::Result&&out.priority>4)return out;
        bool releasing=tail_&&now-tail_>=hold_;
        float weight=level_*(tail_?shape::tail(static_cast<float>(now-tail_),static_cast<float>(release_),static_cast<float>(hold_)):1.f);
        out.reason=sustained::Reason::Result;out.continuous=!releasing;out.reaction=releasing;out.priority=(std::max)(out.priority,4);
        out.reactionId=began_;
        out.earnedReaction=!rampOnly_;
        out.reactionHoldUntil=tail_?tail_+hold_:0;out.reactionUntil=tail_?tail_+release_:0;
        out.size=(std::max)(out.size,rampOnly_?2:3);out.supporting=!releasing;out.attackMs=500;out.releaseMs=static_cast<int>(release_-shape::tailHold(release_));
        if(rampOnly_)out.releaseMs=static_cast<int>(release_);
        out.intensity=950*std::sqrt((std::clamp)(fullness,0.f,1.f))*weight;
        return out;
    }
};
}
