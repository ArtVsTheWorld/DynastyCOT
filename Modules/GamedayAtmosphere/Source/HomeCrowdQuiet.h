#pragma once
#include "BackgroundRoar.h"
#include "LineupAtmosphere.h"
#include "DirectorShake.h"

namespace attendance::homequiet {
// A presentation envelope, not crowd history or a new scoring detector.
struct Envelope {
    uintptr_t match=0;unsigned epoch=0;uint64_t updated=0;
    float leadGain=1,quiet=0,visitorShare=-1;
    bool newTouchdown=false;int corroboration=0; // 1 matched, 2 pending, 3 inconsistent, 4 expired
    bool tdObserved=false;
    uint64_t touchdownSequence=0,ageMs=0;bool holding=false;
    bool owns(uintptr_t owner,unsigned generation)const{return match&&match==owner&&epoch==generation;}
    bool fresh(uintptr_t owner,unsigned generation,uint64_t now)const{
        return owns(owner,generation)&&now>=updated&&now-updated<=750;
    }
    // Retained diagnostic evidence must not become an audio lease. A paused
    // envelope can still describe a held TD, but fresh() continues gating sound.
    const char* status(uintptr_t owner,unsigned generation,uint64_t now,bool audible)const{
        if(!owns(owner,generation))return "no matching snapshot";
        if(!fresh(owner,generation,now)||!audible)return holding?"TD hold retained; live application suspended":"snapshot retained; live application suspended";
        return holding?"TD hold active; live mix eligible":"live mix eligible; no TD hold";
    }
    float postGain()const{
        // No alumni geography/acoustic measurement is inferred. This is a
        // listening curve based on the current owned spectator split only.
        float floor=visitorShare>=0&&visitorShare<=1?.20f+.40f*std::sqrt(visitorShare):.35f;
        return 1-(1-floor)*quiet;
    }
    float ambientGain()const{return (std::min)(leadGain,postGain());}
};
class Model {
    uintptr_t match_=0;unsigned epoch_=0;uint64_t previous_=0,age_=0,sequence_=0;
    int home_=0,away_=0,pendingHome_=0,pendingAway_=0;uint64_t pending_=0,pendingAt_=0,lastTd_=0;float elapsed_=0;bool seen_=false,holding_=false;
public:
    void reset(){*this=Model{};}
    // Pause/replay time neither advances the fade nor erases a valid score.
    void suspend(){previous_=0;}
    Envelope observe(const energy::Play& p,const energy::Outcome& result,uintptr_t match,unsigned epoch,
                     uint64_t sequence,uint64_t now,bool allowed,float visitorShare=-1,const energy::Play* snap=nullptr){
        if(!allowed||!match||!valid(p.game)||p.game.neutral||p.simulated||p.game.phase==6||p.game.phase==7){reset();return {};}
        if(match_!=match||epoch_!=epoch){reset();match_=match;epoch_=epoch;}
        float elapsed=elapsedGameTime(p.game);
        bool corrected=seen_&&(p.game.homeScore<home_||p.game.awayScore<away_||elapsed+.01f<elapsed_||now<previous_);
        if(corrected){holding_=false;age_=0;sequence_=0;}
        if(holding_&&previous_&&now>=previous_)age_+=(std::min)(now-previous_,uint64_t(250));
        bool freshResult=!corrected&&result.accepted&&result.completed&&sequence&&result.sequence==sequence;
        bool touchdown=freshResult&&result.side==1&&result.homePoints==0&&result.awayPoints>=6&&result.awayPoints<=8;
        int proof=0;
        if(corrected){pending_=0;}
        bool firstTd=touchdown&&sequence!=lastTd_;if(firstTd)lastTd_=sequence;
        if(firstTd&&seen_&&sequence!=sequence_&&!pending_){
            pending_=sequence;pendingAt_=now;pendingHome_=snap?snap->game.homeScore:home_;pendingAway_=snap?snap->game.awayScore:away_;
        }
        bool newlyScored=false;
        if(pending_){
            int hd=p.game.homeScore-pendingHome_,ad=p.game.awayScore-pendingAway_;
            if(sequence!=pending_||now<pendingAt_||now-pendingAt_>1500){proof=4;pending_=0;}
            else if(hd==0&&ad>=6&&ad<=8){proof=1;holding_=true;age_=0;sequence_=pending_;pending_=0;newlyScored=true;}
            else if(hd==0&&ad==0)proof=2;
            else {proof=3;pending_=0;}
        }
        // Any verified home score takes precedence (including a try return).
        if(freshResult&&result.homePoints>0){holding_=false;age_=0;}
        // Unknown selected-type data may not end the lull on a menu. An
        // actual fresh 1st-4th down snap is sufficient even without that data.
        bool ordinary=energy::normalDown(p)&&!p.specialTeams&&
            (lineup::offensivePlay(p.selectedType)||(p.selectedType<=0&&p.game.phase==3));
        if(holding_&&sequence!=sequence_&&ordinary&&(p.game.phase==1||p.game.phase==3)){holding_=false;age_=0;}
        previous_=now;home_=p.game.homeScore;away_=p.game.awayScore;elapsed_=elapsed;seen_=true;
        Envelope out;out.match=match;out.epoch=epoch;out.updated=now;
        out.leadGain=background::leadEngagement(p.game);
        out.quiet=holding_?dense::smooth((static_cast<float>(age_)-1500.f)/2500.f):0;
        out.visitorShare=std::isfinite(visitorShare)?visitorShare:-1;
        out.newTouchdown=newlyScored;out.tdObserved=firstTd;out.corroboration=proof;
        out.touchdownSequence=sequence_;out.ageMs=age_;out.holding=holding_;
        return out;
    }
};
// Apply to the home ambient layer BEFORE adding the visiting sustained voice.
// One-shot IDs, cheers, actual earned reactions and camera envelopes are intact.
inline sustained::Intent restrain(sustained::Intent out,const Envelope& envelope,uint64_t now){
    if(!envelope.fresh(out.match,out.epoch,now)||envelope.quiet<=0||out.earnedReaction||out.scoringReaction||
       out.reason==sustained::Reason::Result||out.reason==sustained::Reason::Eruption||
       out.reason==sustained::Reason::Postgame||out.reason==sustained::Reason::Diagnostics||
       out.reason==sustained::Reason::PassFlight)return out;
    out.intensity*=envelope.postGain();
    return out;
}
inline bool allowHomeCompanion(const Envelope& envelope,const motion::Cue& cue,uint64_t now){
    return !envelope.fresh(cue.match,cue.epoch,now)||envelope.quiet<=0||cue.visitor||
           (cue.kind!=motion::Kind::Ordinary&&cue.kind!=motion::Kind::Critical);
}
}
