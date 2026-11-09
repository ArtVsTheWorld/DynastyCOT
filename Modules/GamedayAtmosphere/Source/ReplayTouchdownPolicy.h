#pragma once
#include "MajorReplay.h"
namespace attendance::replaypolicy {
// Same-snap evidence only. Never alter native presentation while a result is
// unknown, and never turn a touchdown/return TD into an added takeaway replay.
class TouchdownExclusion {
    uintptr_t match_=0;unsigned epoch_=0;uint64_t sequence_=0;
    bool completed_=false,touchdown_=false;
public:
    void reset(){*this=TouchdownExclusion{};}
    void observe(uintptr_t match,unsigned epoch,uint64_t sequence,int beforeHome,int beforeAway,int home,int away,bool completed,bool simulated){
        if(!match||!sequence||simulated){reset();return;}
        if(match_!=match||epoch_!=epoch||sequence_!=sequence){reset();match_=match;epoch_=epoch;sequence_=sequence;}
        if(beforeHome<0||beforeAway<0||home<beforeHome||away<beforeAway){completed_=false;return;}
        touchdown_=touchdown_||majorreplay::touchdownScoreProof(beforeHome,beforeAway,home,away);
        completed_=completed_||completed;
    }
    void nativeTouchdown(){touchdown_=true;}
    bool allowed()const{return match_&&sequence_&&completed_&&!touchdown_;}
};
}
