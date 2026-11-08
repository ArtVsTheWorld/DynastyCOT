#include "../Source/DepartureModel.h"
#include "../Source/LateTrafficEvents.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int run(int timeouts,int offense,bool trafficEnabled=true,int step=1){
    Settings c;c.lateTraffic=trafficEnabled;History h;departures::Model m;State s{4,14,35,5,210,720};s.homeTimeouts=timeouts;s.offense=offense;s.rank0=s.rank1=50;
    int loss=20;m.committedLoss=m.plannedLoss=loss;m.hazard=m.committedHazard=-std::log(.8);
    for(int seconds=210;seconds>=0;seconds-=step){s.seconds=static_cast<float>(seconds);h.observe(c,s);assert(m.observe(c,s,h));m.beginWindow(100);int before=loss;loss=m.settle(s,loss,0,c.quarterCap).loss;assert(loss>=before&&loss<=65&&loss<=lossCeiling(s));}
    return loss;
}
int main(){
    State s{4,20,29,5,120,720};s.offense=0;s.homeTimeouts=3;
    auto hopeful=traffic::recovery(s);s.homeTimeouts=0;assert(traffic::recovery(s)<hopeful);float own=traffic::recovery(s);s.offense=1;assert(traffic::recovery(s)<own);
    s.offense=0;s.homeTimeouts=1;assert(traffic::recovery(s,1)>traffic::recovery(s,0));float base=traffic::lossTarget(s);assert(traffic::lossTarget(s,elapsedGameTime(s)-10)<base);
    assert(traffic::recentScore(s,elapsedGameTime(s)+1)==0);float recent=traffic::recentScore(s,elapsedGameTime(s)-10);s.seconds-=90;assert(traffic::recentScore(s,elapsedGameTime(s)-100)<recent);
    for(float length:{60.f,360.f,720.f,1200.f}){s.quarterSeconds=length;s.seconds=length*.25f+.01f;assert(!traffic::lateHomeGame(s));s.seconds=length*.20f;assert(traffic::lateHomeGame(s));}
    s={4,0,35,5,30,720};float regular=traffic::lossTarget(s);s.playoff=true;float playoff=traffic::lossTarget(s);s.bowl=47;assert(regular>playoff&&playoff>traffic::lossTarget(s));s.neutral=true;assert(!traffic::lossTarget(s));
    assert(run(0,1)>=run(3,0));assert(std::abs(run(0,1,true,1)-run(0,1,true,5))<=1);
    assert(run(0,1)>=run(0,1,false));
    {departures::Model m;Settings c;History h;State p{4,0,14,5,120,720};p.homeTimeouts=1;p.offense=0;m.observe(c,p,h);p.homeScore=7;p.seconds=115;m.observe(c,p,h);assert(m.lastHomeScoreAt==elapsedGameTime(p));float scored=m.lastHomeScoreAt;
        p.seconds=100;m.observe(c,p,h);assert(m.lastHomeScoreAt==scored);p.awayScore=21;p.seconds=95;m.observe(c,p,h);assert(m.lastHomeScoreAt<0);}
    {traffic::TurnoverExits exits;energy::Outcome result;result.accepted=result.completed=result.takeaway=true;result.side=1;result.sequence=1;
        energy::Play before;before.game={4,20,27,1,45,720};before.offense=0;before.homeTimeouts=0;before.down=2;before.toGoal=10;
        auto after=before;after.game.phase=4;after.game.seconds=40;after.offense=1;after.down=1;
        assert(exits.observe(result,before,after)>=8&&!exits.observe(result,before,after));
        result.sequence=2;after.simulated=true;assert(!exits.observe(result,before,after));after.simulated=false;
        result.sequence=3;after.game.neutral=true;assert(!exits.observe(result,before,after));after.game.neutral=false;
        result.sequence=4;after.game.playoff=true;assert(!exits.observe(result,before,after));after.game.playoff=false;
        result.sequence=5;assert(!exits.observe(result,before,after,0));
        result.sequence=6;before.game.seconds=175;before.homeTimeouts=3;after.game.seconds=170;after.homeTimeouts=3;assert(!exits.observe(result,before,after,10,true));
    }
    std::puts("PASS: quarter-scaled late window, deficit/possession/timeout outlook, recent-score grace/decay, unknown-data patience, faster bounded targets, no refill, postseason/neutral limits and once-only costly-turnover groups.");
}
