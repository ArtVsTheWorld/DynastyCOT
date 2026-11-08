#pragma once
#include "AttendanceContext.h"
namespace attendance::halftime {
struct ExitConversion {
    bool checked=false;
    void reset(){checked=false;}
    int request(const State& s,int permanent,int temporary,int percent=25){
        if(checked||!valid(s)||s.quarter<3)return 0;
        checked=true;
        int deficit=s.awayScore-s.homeScore,absent=(std::max)(0,temporary-permanent);
        if(s.quarter!=3||s.seconds<s.quarterSeconds*.75f||s.neutral||s.playoff||s.conferenceChampionship||bigGameMatchup(s)||deficit<18||permanent>=20||!absent)return 0;
        float stillHere=context::smooth((20-permanent)/10.f);
        float severity=.8f+.2f*context::smooth((deficit-18)/10.f);
        return (std::min)(absent,static_cast<int>(std::lround(absent*(std::clamp)(percent,0,25)*.01f*stillHere*severity)));
    }
};
}
