#pragma once
#include "ExcitementRules.h"
namespace attendance::resultfreshness {
// Keep the official tracker result for diagnostics, but don't start a new
// routine reaction after an injury/penalty scene has moved on to the lineup.
// Score changes remain authoritative even when their presentation is delayed.
class Window {
    uintptr_t match_=0;unsigned epoch_=0;uint64_t sequence_=0,ended_=0,last_=0;
public:
    void reset(){*this=Window{};}
    bool observe(uintptr_t match,unsigned epoch,uint64_t sequence,int phase,uint64_t now,const energy::Outcome& result){
        if(match_!=match||epoch_!=epoch||now<last_){reset();match_=match;epoch_=epoch;}
        last_=now;
        if(sequence_!=sequence){sequence_=sequence;ended_=0;}
        if(phase==3)ended_=0;
        else if(sequence&&(phase==4||phase==5)&&!ended_)ended_=now;
        if(!result.accepted||!result.completed||result.sequence!=sequence)return true;
        if(result.homePoints||result.awayPoints||phase==7)return true;
        return !ended_||now-ended_<=2500;
    }
};
}
