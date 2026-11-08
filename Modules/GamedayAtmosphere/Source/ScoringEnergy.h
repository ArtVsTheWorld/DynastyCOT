#pragma once
#include "AttendanceRules.h"

namespace attendance::energy {
// Background shootout excitement, capped below the larger event/anticipation sources.
// Both teams must contribute. Use game progress, not wall time or an opening-TD projection.
struct ScoringPace {
    float highTime=-1,recent=0;
    void advance(const State& s){
        if(!valid(s)||s.quarter>4)return;
        float elapsed=elapsedGameTime(s);
        if(highTime>=0&&elapsed<highTime)return;
        if(highTime>=0)recent*=std::exp2(-(elapsed-highTime)/(s.quarterSeconds*0.5f));
        highTime=elapsed;
    }
    void scored(const State& s,int points){
        advance(s);
        if(s.quarter<=4&&points>0)recent=(std::min)(4.0f,recent+(points>=6?2.5f:1.5f));
    }
    float value(const State& s)const{
        if(!valid(s)||s.quarter>4||highTime<0)return 0;
        int gap=std::abs(s.homeScore-s.awayScore);
        float competition=gap<=8?1.0f:gap<=16?0.6f:gap<=24?0.2f:0;
        float quarters=highTime/s.quarterSeconds;
        float balanced=2.0f*(std::min)(s.homeScore,s.awayScore);
        float pace=(std::clamp)((balanced/(std::max)(1.0f,quarters)-12)*0.75f,0.0f,6.0f);
        return (pace+recent)*(std::clamp)(quarters,0.0f,1.0f)*competition;
    }
};
}
