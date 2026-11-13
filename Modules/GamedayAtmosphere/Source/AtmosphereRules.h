#pragma once
#include "AttendanceRules.h"

namespace attendance {
inline bool upset(const State& raw){
    State s=energyPerspective(raw);
    if(s.homeScore<=s.awayScore||!top25(s.rank1))return false;
    return (!top25(s.rank0)&&s.rank1<=10)||(top25(s.rank0)&&s.rank0-s.rank1>=10);
}
inline float upsetDiscrepancy(const State& raw){
    State s=energyPerspective(raw);
    if(s.rank0<1||s.rank1<1||s.rank1>25||s.rank0<=s.rank1)return 0;
    return relativeGap(s);
}
inline int upsetStage(const State& raw,int previous=0){
    State s=energyPerspective(raw);
    if(upsetDiscrepancy(s)<=0||s.quarter<2||s.quarter>4)return 0;
    int deficit=s.awayScore-s.homeScore;
    if(deficit>14)return 0;
    if(s.quarter==4&&s.rank1<=10&&(deficit<=0||(previous==3&&deficit<=3)))return 3;
    if((s.quarter>=3&&deficit<=8)||deficit<=0)return 2;
    return 1;
}
inline const char* upsetStageName(int stage){return stage==3?"electric upset bid":stage==2?"upset threat":stage==1?"underdog hanging around":"inactive";}
// Scores and elapsed live game time must both move forward. Pauses and replay
// rewinds cannot advance timers, rewrite deficit history, or retrigger warning.
struct History {
    bool seen=false,comeback=false,released=false,warningPending=false,warningDone=false,upsetWatch=false;
    int maxDeficit=0,bestDeficit=255,lastHome=0,lastAway=0,watchStage=0;
    int comebackSide=-1,maxAwayDeficit=0;
    unsigned quarterMask=0;
    float elapsed=0,deteriorationSince=-1;
    void reset(){*this=History{};}
    bool observe(const Settings& c,const State& s,bool authoritativeQuarter=false){
        if(!valid(s)||(!authoritativeQuarter&&s.phase!=1&&s.phase!=5))return false;
        float now=elapsedGameTime(s);
        if(springContext(s)){
            // Native spring situations can rewind clocks/scores. They never
            // establish competitive comeback/upset protection or rearm waves.
            seen=true;elapsed=(std::max)(elapsed,now);lastHome=s.homeScore;lastAway=s.awayScore;
            comeback=released=warningPending=upsetWatch=false;watchStage=0;return true;
        }
        if(seen&&(now+0.01f<elapsed||s.homeScore<lastHome||s.awayScore<lastAway))return false;
        float crossing=4*s.quarterSeconds-120;
        if(seen&&!warningDone&&s.quarter==4&&s.quarterSeconds>120&&elapsed<crossing&&now>=crossing)warningPending=true;
        int deficit=s.awayScore-s.homeScore;
        maxAwayDeficit=(std::max)(maxAwayDeficit,-deficit);
        if(s.neutral){
            if(comebackSide==1)deficit=-deficit;
            else if(comebackSide<0&&maxAwayDeficit>=c.comebackDeficit&&maxAwayDeficit+deficit>=c.comebackRecovered&&-deficit>=-8){comebackSide=1;maxDeficit=maxAwayDeficit;deficit=-deficit;}
        }
        watchStage=upsetStage(s,watchStage);upsetWatch=watchStage>0;
        maxDeficit=(std::max)(maxDeficit,deficit);
        bool known=s.offense>=0&&s.offense<=1&&s.homeTimeouts>=0&&s.homeTimeouts<=3;
        // Two scores still require another possession, even with the ball.
        // Unknown context stays conservative; neutral labels stay symmetric.
        float needed=deficit>8?(deficit<=16?60.0f:100.0f):0;
        if(known&&!s.neutral){
            needed-=10*s.homeTimeouts;
            if(s.offense==1)needed+=20;
            else if(s.toGoal>=0&&s.toGoal<=20)needed-=10;
        }
        bool opportunity=s.quarter!=4||deficit<=8||!known||s.seconds>(std::max)(10.0f,needed);
        bool qualifies=opportunity&&maxDeficit>=c.comebackDeficit&&maxDeficit-deficit>=c.comebackRecovered&&deficit>=-8;
        if(!comeback&&qualifies&&(!released||deficit<bestDeficit||std::abs(deficit)<=8)){
            if(s.neutral&&comebackSide<0)comebackSide=0;
            comeback=true;released=false;bestDeficit=deficit;deteriorationSince=-1;
        }
        if(comeback){
            bestDeficit=(std::min)(bestDeficit,deficit);
            if(deficit < -8||!opportunity){comeback=false;released=true;deteriorationSince=-1;}
            else if(std::abs(deficit)<=8||deficit<bestDeficit+7)deteriorationSince=-1;
            else {
                if(deteriorationSince<0)deteriorationSince=now;
                if(now-deteriorationSince>=c.comebackDeteriorationSeconds*s.quarterSeconds/720.0f){comeback=false;released=true;}
            }
        }
        seen=true;elapsed=now;lastHome=s.homeScore;lastAway=s.awayScore;return true;
    }
    bool takeWarning(){if(!warningPending||warningDone)return false;warningPending=false;warningDone=true;return true;}
    bool takeQuarter(int previous,int next){
        if(previous<1||next<=previous||next>4||(quarterMask&(1u<<next)))return false;
        quarterMask|=1u<<next;return true;
    }
};
inline float situationEmphasis(const State& s,int closePercent,int matchupPercent){
    if(!valid(s))return 1;
    int boost=std::abs(s.homeScore-s.awayScore)<=8?closePercent:0;
    if(marqueeMatchup(s))boost+=matchupPercent;
    return 1+(std::clamp)(boost,0,35)*0.01f;
}
}
