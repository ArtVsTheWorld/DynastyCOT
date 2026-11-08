#pragma once
#include "AttendanceRules.h"

namespace attendance::energy {
struct OpeningProfile {
    bool seen=false,armed=false,drive=false;int offense=-1,home=0,away=0;
    float high=0,fade=-1;
    float observe(const State& s,int side,int down,bool enabled,bool big){
        if(!valid(s))return 0;
        if(!seen){
            seen=true;
            armed=enabled&&big&&s.quarter==1&&s.homeScore==0&&s.awayScore==0&&
                s.seconds>=s.quarterSeconds-1&&(s.phase==6||s.phase==1||s.phase==5);
        }
        if(!enabled||!big||!armed||s.quarter!=1)return 0;
        high=(std::max)(high,elapsedGameTime(s));
        if(!drive&&s.phase==3&&down>=1&&down<=4&&(side==0||side==1)){
            drive=true;offense=side;home=s.homeScore;away=s.awayScore;
        }
        bool result=drive&&(s.homeScore!=home||s.awayScore!=away||
            ((s.phase==1||s.phase==5)&&down>=1&&down<=4&&side>=0&&side!=offense));
        if(fade<0&&(result||high>=s.quarterSeconds*0.5f))fade=(std::min)(high,s.quarterSeconds*0.5f);
        return fade<0?1.0f:1-(std::clamp)((high-fade)/(s.quarterSeconds*0.25f),0.0f,1.0f);
    }
};
}
