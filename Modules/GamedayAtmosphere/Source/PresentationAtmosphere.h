#pragma once
#include "LineupAtmosphere.h"
namespace attendance::presentationaudio {
class Model {
    uintptr_t match_=0;unsigned epoch_=0;int phase_=-1;uint64_t began_=0;
public:
    void reset(){*this=Model{};}
    sustained::Intent observe(const core::Settings& c,const State& s,uintptr_t match,unsigned epoch,uint64_t now,
                              bool allowed,bool known,float fullness,uint64_t positions,bool finalMenu){
        sustained::Intent out;out.match=match;out.epoch=epoch;out.updated=now;
        if(match!=match_||epoch!=epoch_||s.phase!=phase_){match_=match;epoch_=epoch;phase_=s.phase;began_=now;}
        bool opening=s.phase==6,closing=s.phase==7;
        if(!allowed||!match||!valid(s)||!known||!std::isfinite(fullness)||!c.enabled||!c.lineups||core::normalLimit(c)<=0||
           (!opening&&!closing)||(opening&&!c.opening)||(closing&&(finalMenu||s.homeScore==s.awayScore||(!s.neutral&&s.homeScore<s.awayScore)))||
           now<began_||now-began_>=(opening?300000ull:120000ull))return out;
        bool big=lineup::bigGame(s),packed=c.packedHouse&&attendance::packed::qualifies(positions,fullness);
        bool special=closing&&overtime(s)&&c.overtime&&fullness>=attendance::packed::specialMinimum(positions);
        bool lightOpening=opening&&positions>=512&&expectedPeak(s)>=.90f&&fullness>=.55f;
        bool qualified=(positions>=512&&big&&fullness>=lineup::bigRequirement(c,s,positions))||packed||special||lightOpening;
        float competition=lineup::competitive(s);if(!qualified||competition<=0)return out;
        float close=std::abs(s.homeScore-s.awayScore)<=8?1.f:.75f;
        float level=lineup::ordinaryImportance(attendance::packed::strength(positions,fullness),close,opening?1.f:0.f);
        if(big)level=(std::max)(level+lineup::bigGamePremium(s,fullness,close),opening?1.f:0.f);
        if(special)level=(std::max)(level,.8f);
        if(lightOpening&&!big&&!packed)level=(std::min)(level,.35f);
        level=(std::clamp)(level,0.f,1.f)*competition;
        float presence=std::sqrt((std::clamp)(fullness/.8f,0.f,1.f));
        out.active=true;out.reason=opening?sustained::Reason::Opening:sustained::Reason::Postgame;
        out.continuous=true;out.priority=1;out.intensity=(500+500*level)*presence;
        out.size=level>=.65f?3:level>=.35f?2:1;out.supporting=level>=.55f;
        out.cheers=false;out.attackMs=3000;out.releaseMs=15000;
        // No fabricated play, cheer, camera cue, gain bonus or departure update.
        return out;
    }
};
}
