#pragma once
#include "LineupAtmosphere.h"
namespace attendance::openingwelcome {
// A separate sound-only opening. No play/result, volume premium or camera event.
class Model {
    uintptr_t match_=0;unsigned epoch_=0;uint64_t began_=0;lineup::OpeningDrive drive_;
public:
    void reset(){*this=Model{};}
    sustained::Intent observe(const core::Settings& c,const energy::Play& p,uintptr_t match,unsigned epoch,uint64_t now,bool allowed,bool known,float fullness,const energy::Outcome* result=nullptr){
        if(match!=match_||epoch!=epoch_){reset();match_=match;epoch_=epoch;}
        bool first=drive_.observe(p,result);const auto& s=p.game;
        bool intro=s.phase==6&&drive_.armed&&!drive_.closed;
        sustained::Intent out;out.match=match;out.epoch=epoch;out.updated=now;
        if(!allowed||!known||!match||!c.enabled||!c.lineups||!c.opening||p.resumed||p.simulated||!valid(s)||
           !std::isfinite(fullness)||fullness<=0||fullness>1||(!intro&&!first)||(s.phase!=6&&s.phase!=1&&s.phase!=5))return out;
        if(!began_)began_=now;
        if(now<began_||now-began_>=300000)return out;
        out.active=true;out.reason=sustained::Reason::Opening;out.priority=1;out.continuous=true;out.size=1;
        out.intensity=300*std::sqrt(fullness);out.supporting=out.supportingOnly=true;out.cheers=false;out.attackMs=2000;out.releaseMs=2500;
        // The first welcome is consumed even if a stronger native/mod cue owns it.
        if(now-began_<1200){out.lineupCheer=0x4f50454e00000001ull;out.lineupCheerBegan=began_;out.lineupCheerUntil=began_+1200;out.cheers=true;out.lineupType=83;}
        return out;
    }
    static sustained::Intent merge(sustained::Intent current,const sustained::Intent& welcome){
        if(!welcome.active||!current.active||current.match!=welcome.match||current.epoch!=welcome.epoch||current.reason!=sustained::Reason::None||current.celebration||current.liveCheer||current.resultCheer||current.passFlight||current.skillAccent||current.motionCompanion)return current;
        return welcome;
    }
};
}
