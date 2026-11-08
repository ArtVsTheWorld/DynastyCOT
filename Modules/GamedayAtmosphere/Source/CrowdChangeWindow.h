#pragma once
#include "AttendanceRules.h"
#include <cstdint>
namespace attendance {
// Observe every presentation phase, including phases decision() otherwise
// skips. Clock progress alone never opens a visual change window.
struct CrowdChangeWindow {
    uintptr_t match=0;int phase=-1,quarter=0;bool pending=false;
    float highTime=0;int highHome=0,highAway=0;bool seen=false,simulation=false;
    void reset(){*this=CrowdChangeWindow{};}
    void observe(uintptr_t current,const State& s,bool offscreenSimulation=false){
        if(!current||!valid(s))return;
        if(match!=current){reset();match=current;}
        float time=elapsedGameTime(s);
        if(seen&&(time+0.01f<highTime||s.homeScore<highHome||s.awayScore<highAway)){pending=false;phase=-1;return;}
        bool sim=offscreenSimulation&&(s.phase==0||s.phase==5);
        bool advanced=!seen||time>highTime+.01f||s.homeScore>highHome||s.awayScore>highAway;
        highTime=time;highHome=s.homeScore;highAway=s.awayScore;seen=true;
        bool safe=s.phase==5||s.phase==6||sim;
        if(!safe)pending=false;
        else if(s.phase!=phase||s.quarter!=quarter||(sim&&(!simulation||advanced)))pending=true;
        phase=s.phase;quarter=s.quarter;simulation=sim;
    }
    bool take(uintptr_t current,const State& s,bool offscreenSimulation=false){
        if(current!=match||!pending||(s.phase!=5&&s.phase!=6&&!(simulation&&offscreenSimulation&&s.phase==0)))return false;
        pending=false;return true;
    }
};
}
