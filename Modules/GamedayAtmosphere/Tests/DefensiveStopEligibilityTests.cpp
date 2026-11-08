#include "../Source/LineupAtmosphere.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    for(int down:{3,4})for(int type:{3,21,22,105,106,154,155})for(int quarter:{2,4}){
        energy::Settings rules;energy::Tracker tracker;energy::Play p;p.game={quarter,14,14,1,60,720};p.down=down;p.offense=1;p.toGoal=4;p.toFirst=4;p.resultStamp=1;p.specialTeams=type>0&&!lineup::offensivePlay(type);
        tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);p.game.phase=1;p.game.seconds=55;p.offense=0;p.down=1;p.resultStamp=2;
        auto result=tracker.observe(rules,p);assert(result.completed&&!result.takeaway);
        bool ordinary=!p.specialTeams;assert(result.highLeverageStop==(ordinary&&quarter==4));
        assert(result.goalLineStop==(ordinary&&down==4));
        if(!ordinary)assert(result.event!=energy::Event::ThirdStop&&result.event!=energy::Event::FourthStop&&result.event!=energy::Event::GoalLine);
    }
    std::puts("PASS: real close offensive third/fourth-down stops qualify; punts/field goals do not masquerade as critical stops or goal-line stands in either half.");
}
