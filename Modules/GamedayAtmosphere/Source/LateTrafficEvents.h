#pragma once
#include "LateTraffic.h"
#include "ExcitementRules.h"
namespace attendance::traffic {
inline State perspective(const energy::Play& p){State s=p.game;s.offense=p.offense;s.homeTimeouts=p.homeTimeouts;s.toGoal=p.toGoal;return s;}
class TurnoverExits {
    uint64_t sequence_=0;
public:
    void reset(){sequence_=0;}
    int observe(const energy::Outcome& result,const energy::Play& before,const energy::Play& after,int maximum=10,bool comeback=false){
        const State s=perspective(after);
        if(!result.accepted||!result.completed||!result.sequence||result.sequence==sequence_||!result.takeaway||result.side!=1||!valid(before.game)||
           before.game.quarter!=4||before.game.quarterSeconds!=s.quarterSeconds||before.game.seconds<s.seconds||
           before.offense!=0||after.offense!=1||after.simulated||!lateHomeGame(s)||s.playoff||s.homeScore>=s.awayScore)return 0;
        sequence_=result.sequence;
        bool known=s.homeTimeouts>=0&&s.homeTimeouts<=3;
        bool kneel=known&&after.down==1&&s.seconds<=(3-s.homeTimeouts)*40.f&&s.homeTimeouts<=1;
        float oldChance=recovery(perspective(before)),newChance=kneel?0:recovery(s);
        if(comeback&&newChance>.05f)return 0;
        float drop=(std::max)(0.f,oldChance-newChance);
        if(!kneel&&drop<.20f)return 0;
        float late=1-s.seconds/window(s);
        int points=static_cast<int>(std::lround((3+7*drop)*(.60f+.40f*late)));
        if(kneel)points=(std::max)(points,8);
        return (std::clamp)(points,0,(std::clamp)(maximum,0,15));
    }
};
}
