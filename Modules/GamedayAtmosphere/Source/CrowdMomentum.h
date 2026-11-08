#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include "AttendanceRules.h"

namespace attendance::energy {
// Regulation decay follows elapsed football time, with a high-water mark so
// replays and clock corrections cannot charge for the same seconds twice.
// Overtime is untimed: only continuously observed live-play seconds count.
struct MomentumClock {
    double seconds=0,lastWall=-1;float highElapsed=-1,length=0;int quarter=0,phase=-1;
    double advance(const State& s,double wall){
        if(!valid(s)||!std::isfinite(wall)||(lastWall>=0&&wall<lastWall))return seconds;
        float elapsed=elapsedGameTime(s);
        if(s.quarter<=4){
            if(quarter>=1&&quarter<=4&&s.quarter>=quarter&&std::fabs(length-s.quarterSeconds)<0.01f){
                if(elapsed>=highElapsed){seconds+=elapsed-highElapsed;highElapsed=elapsed;}
            }else if(quarter==0){highElapsed=elapsed;}
            else if(std::fabs(length-s.quarterSeconds)>=0.01f){highElapsed=elapsed;}
        }else if(overtime(s)&&quarter>=1&&quarter<=4&&std::fabs(length-s.quarterSeconds)<0.01f){
            seconds+=(std::max)(0.0f,4*length-highElapsed);
        }else if(overtime(s)&&quarter==5&&s.phase==3&&phase==3&&lastWall>=0){
            seconds+=(std::min)(0.25,wall-lastWall);
        }
        lastWall=wall;length=s.quarterSeconds;quarter=(std::max)(quarter,s.quarter);phase=s.phase;
        return seconds;
    }
};
struct Momentum {
    float heat=0;unsigned chain=0;int side=-1;
    uint64_t sequence=0;double last=-1,lastBig=-1000;
    void reset(){*this=Momentum{};}
    void advance(double now,float halfLife){
        if(!std::isfinite(now))return;
        if(last>=0&&now>=last)heat*=std::exp2(-static_cast<float>(now-last)/(std::clamp)(halfLife,10.0f,120.0f));
        if(last<0||now>=last)last=now;
        if(now-lastBig>75)chain=0;
        if(heat<0.05f)heat=0;
    }
    void setback(){heat*=0.35f;chain=0;side=-1;}
    bool observe(float points,double now,uint64_t play,int team,float halfLife){
        if(!play||play==sequence||!std::isfinite(points)||!std::isfinite(now)||(last>=0&&now<last))return false;
        advance(now,halfLife);sequence=play;
        if(points<=0){chain=0;return false;}
        if(team!=side){chain=0;heat*=0.5f;side=team;}
        chain=(std::min)(5u,chain+1);lastBig=now;
        heat=(std::clamp)(heat+points*(1+0.2f*(chain-1)),0.0f,30.0f);return true;
    }
};
}
