#pragma once
#include "ExcitementRules.h"
namespace attendance::hailmary {
struct Pass {
    uint64_t sequence=0;float air=-1;
    void reset(){*this=Pass{};}
    void observe(uint64_t play,float yards){
        if(sequence!=play){sequence=play;air=-1;}
        if(play&&std::isfinite(yards)&&yards>=0&&yards<=100)air=yards;
    }
    bool scored(const energy::Outcome& result,const energy::Play& snap)const{
        int points=result.side==0?result.homePoints:result.awayPoints;
        return sequence&&result.sequence==sequence&&result.accepted&&result.completed&&points>=6&&points<=8&&
            energy::normalDown(snap)&&result.side==snap.offense&&(snap.game.quarter==2||snap.game.quarter==4)&&
            snap.game.seconds>=0&&snap.game.seconds<=15&&snap.toGoal>=30&&snap.toGoal<=100&&
            air>=30&&air>=(std::max)(30.f,snap.toGoal-5);
    }
};
}
