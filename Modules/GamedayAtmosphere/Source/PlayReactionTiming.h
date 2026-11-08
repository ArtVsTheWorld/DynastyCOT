#pragma once
#include "LiveReactions.h"
namespace attendance::timing {
// Distance remains useful for verified play ownership and diagnostics, but
// no longer lengthens sound or camera effects. All live tails share one fade.
inline unsigned extraMs(float,float,unsigned=0){return 0;}
// One high-water distance per verified play. Sound and motion consume the
// same frozen extension; neither consumer adds elapsed-frame increments.
class Distance {
    uintptr_t match_=0;unsigned epoch_=0;uint64_t sequence_=0,last_=0,sampleAt_=0;
    int side_=-1,originSide_=-1,worldDirection_=0,direction_=0;
    float origin_=0,lastSpot_=0,yards_=0,minimum_=15;bool returning_=false,owned_=false,frozen_=false,position_=false;
    unsigned extra_=0;float threshold_=15;
public:
    void reset(){*this=Distance{};}
    // Measurement alone (for example two yards after a short catch) does not
    // admit a full long-play sound or suppress the ordinary lineup bridge.
    bool ownsDuration()const{return owned_&&(threshold_<=0||yards_>=threshold_);}
    float yards()const{return yards_;}unsigned extension()const{return extra_;}
    void observe(const energy::Play& p,uintptr_t match,unsigned epoch,uint64_t sequence,uint64_t now,bool allowed,
                 const live::Outcome& event,float rawSpot,float gain,bool ranAfterCatch,int worldDirection,bool held,
                 live::RunBuildup buildup,float kickYards,const live::Settings& settings,unsigned maximum){
        if(match!=match_||epoch!=epoch_){reset();match_=match;epoch_=epoch;}
        if(!allowed||!match||!valid(p.game)||p.simulated||now<last_||(last_&&now-last_>750)){reset();match_=match;epoch_=epoch;last_=now;return;}last_=now;
        bool newPlay=sequence!=sequence_;
        if(newPlay){sequence_=sequence;owned_=returning_=frozen_=position_=false;yards_=0;extra_=0;sampleAt_=0;originSide_=p.offense;worldDirection_=worldDirection;}
        if(!sequence)return;
        if(frozen_&&p.game.phase==3&&!newPlay){
            // The next snap may be visible before its tracker ID advances.
            // Retire the previous distance once; do not resurrect its tail
            // on that new ball while waiting for a fresh verified sequence.
            owned_=returning_=position_=false;yards_=0;extra_=0;sampleAt_=0;return;
        }
        float gainMinimum=std::isfinite(settings.minimumGainYards)?(std::clamp)(settings.minimumGainYards,10.f,60.f):20.f;
        float catchMinimum=std::isfinite(settings.minimumCatchYards)?(std::clamp)(settings.minimumCatchYards,10.f,60.f):20.f;
        if(newPlay)minimum_=threshold_=gainMinimum;
        if(event.id&&event.event==live::Event::LongCatch)minimum_=threshold_=catchMinimum;
        if(event.id&&event.event==live::Event::ForcedFumble){owned_=returning_=position_=false;yards_=0;extra_=0;}
        bool returnEvent=event.id&&(event.event==live::Event::Interception||event.event==live::Event::FumbleRecovery);
        if(returnEvent&&p.game.phase==3){
            owned_=returning_=true;frozen_=false;side_=event.side;minimum_=threshold_=0;yards_=0;extra_=0;
            position_=std::isfinite(rawSpot)&&std::fabs(rawSpot)<=60;origin_=lastSpot_=rawSpot;sampleAt_=now;
            direction_=side_==originSide_?worldDirection_:-worldDirection_;
        }
        if(!frozen_&&p.game.phase==3){
            bool kick=std::isfinite(kickYards)&&kickYards>=30;
            if(kick){owned_=returning_=true;minimum_=threshold_=40;yards_=(std::max)(yards_,kickYards);}
            else if(returning_){
                bool coherent=position_&&held&&(p.offense==side_||returnEvent)&&std::isfinite(rawSpot)&&std::fabs(rawSpot)<=60&&sampleAt_&&now>=sampleAt_&&std::fabs(rawSpot-lastSpot_)<=3.5f+30.f*(now-sampleAt_)*.001f;
                if(coherent){if(direction_==1||direction_==-1)yards_=(std::max)(yards_,(rawSpot-origin_)*direction_);lastSpot_=rawSpot;sampleAt_=now;}
            }else if(buildup.id||ranAfterCatch||(event.id&&(event.event==live::Event::LongRun||event.event==live::Event::RunAfterCatch))){
                owned_=true;minimum_=threshold_=ranAfterCatch&&!buildup.id?catchMinimum:gainMinimum;
                if(std::isfinite(gain)&&gain>=0&&gain<=110)yards_=(std::max)(yards_,gain);
                if(buildup.id&&std::isfinite(buildup.yards))yards_=(std::max)(yards_,buildup.yards);
            }
            extra_=extraMs(yards_,threshold_,maximum);
        }
        if(p.game.phase!=3)frozen_=true;
    }
    unsigned forResult(const energy::Outcome& result,const energy::Play& snap,unsigned maximum)const{
        if(!sequence_||!result.accepted||!result.completed||result.sequence!=sequence_||result.safety)return 0;
        float distance=yards_,threshold=minimum_;
        bool touchdown=result.homePoints>=6||result.awayPoints>=6;
        if(touchdown&&result.side==snap.offense&&std::isfinite(result.touchdownYards)&&result.touchdownYards>=0){distance=(std::max)(distance,result.touchdownYards);threshold=threshold_;}
        if(!touchdown&&!owned_)return 0;
        return extraMs(distance,threshold,maximum);
    }
};
}
