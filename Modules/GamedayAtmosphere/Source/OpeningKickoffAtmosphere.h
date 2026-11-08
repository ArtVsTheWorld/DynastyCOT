#pragma once
#include "LineupAtmosphere.h"
namespace attendance::kickoff {
// A verified fresh opening only: never manufacture this on a scenario load,
// replay, later kickoff or a sparse/unqualified lineup.
class Model {
    uintptr_t match_=0;unsigned epoch_=0;uint64_t began_=0,last_=0,liveAt_=0,tailAt_=0;
    bool used_=false,active_=false,cover_=false;float level_=0;
public:
    void reset(){*this=Model{};}
    void suspend(){active_=cover_=false;}
    bool replacesLineup()const{return cover_;}
    bool observe(const energy::Play& p,uintptr_t match,unsigned epoch,uint64_t now,bool allowed,bool openingProof,int formation,float fullness){
        if(match!=match_||epoch!=epoch_){reset();match_=match;epoch_=epoch;}
        if(!allowed||!match||p.simulated||p.resumed||!valid(p.game)||!std::isfinite(fullness)||now<last_||(last_&&now-last_>750)){active_=cover_=false;last_=now;return false;}last_=now;
        if(liveAt_&&p.game.phase==1){active_=cover_=false;return false;}
        bool start=!used_&&openingProof&&lineup::kickoffMatchup(p.game)&&p.game.phase==1&&p.game.quarter==1&&p.game.homeScore==0&&p.game.awayScore==0&&p.game.seconds>=p.game.quarterSeconds-1&&(formation==23||formation==24);
        if(start){used_=active_=cover_=true;began_=now;liveAt_=tailAt_=0;}
        if(!active_)return false;
        if(p.game.phase==3){if(!liveAt_)liveAt_=now;}
        else if(liveAt_&&!tailAt_)tailAt_=now;
        if(now-began_>=90000||(liveAt_&&now-liveAt_>=45000)||(tailAt_&&now-tailAt_>=4000)||(tailAt_&&p.game.phase==1)){active_=false;return start;}
        level_=950*std::sqrt((std::clamp)(fullness,0.f,1.f))*(tailAt_?1-static_cast<float>(now-tailAt_)/4000:1.f);
        return start;
    }
    sustained::Intent merge(sustained::Intent out,uint64_t now)const{
        if(!active_||!out.active||out.match!=match_||out.epoch!=epoch_||now<last_||now-last_>750||out.celebration||out.reason==sustained::Reason::Eruption||(out.reason==sustained::Reason::Result&&out.priority>=4))return out;
        out.reason=sustained::Reason::Lineup;out.priority=(std::max)(out.priority,2);out.continuous=!tailAt_;out.reaction=tailAt_!=0;out.size=3;out.supporting=!tailAt_;
        out.attackMs=750;out.releaseMs=4000;out.intensity=level_;
        out.lineup=0x8000000000000001ull;out.lineupBegan=began_;out.lineupUntil=now+750;
        out.lineupCheer=0x8000000000000001ull;out.lineupCheerBegan=began_;out.lineupCheerUntil=began_+1500;
        out.cheers=!liveAt_&&now>=began_&&now-began_<1500;out.deepCheers=false;
        return out;
    }
};
}
