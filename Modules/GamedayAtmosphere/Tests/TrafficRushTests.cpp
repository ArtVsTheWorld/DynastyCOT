#include "../Source/DepartureModel.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int run(int deficit,int initial,int extra,int length=720,int cadence=30,bool wet=false,bool postseason=false,bool neutral=false){
    State s{4,14,14+deficit,5,float(length),float(length)};s.homeTimeouts=0;s.offense=1;s.rank0=s.rank1=50;s.precipitation=wet?1:0;s.playoff=postseason;s.neutral=neutral;
    Settings c;c.lateTrafficQuarterExtra=extra;History h;departures::Model m;int loss=initial;
    h.observe(c,s);m.observe(c,s,h);m.beginWindow(100);assert(m.settle(s,loss,0,c.quarterCap).loss==loss);
    for(int time=1;time<=length;++time){s.seconds=float(length-time);assert(h.observe(c,s)&&m.observe(c,s,h));
        if(time%cadence==0||time==length){m.beginWindow(s,4);int before=loss;loss=m.settle(s,loss,0,c.quarterCap).loss;assert(loss>=before&&loss<=lossCeiling(s)&&loss-initial<=45+(postseason||neutral?0:10));}
    }
    return loss;
}
int main(){
    for(int length:{60,360,720,1200}){
        // Exercise the production caller, not just the standalone policy.
        // The old integration guard silently kept both new windows at 25%.
        Settings policy;History story;State early{4,0,35,5,length*.40f,float(length)};early.offense=1;early.homeTimeouts=0;
        assert(departures::trafficTarget(policy,early,story,elapsedGameTime(early),-1)>0);
        departures::Model earlyCap;earlyCap.observe(policy,early,story);assert(earlyCap.quarterLimit(early,elapsedGameTime(early),45)>45);
        early.awayScore=21;early.seconds=length*.30f;assert(departures::trafficTarget(policy,early,story,elapsedGameTime(early),-1)>0);
        early.seconds=length*.34f;assert(!departures::trafficTarget(policy,early,story,elapsedGameTime(early),-1));
        early.playoff=true;early.seconds=length*.30f;assert(!departures::trafficTarget(policy,early,story,elapsedGameTime(early),-1));
        State s{4,0,42,5,length*.5f,float(length)};s.homeTimeouts=0;s.offense=1;
        assert(!traffic::extraQuarterAllowance(s,-1,10)&&!traffic::blowoutUrgency(s));
        s.seconds=length*.225f;assert(traffic::extraQuarterAllowance(s,-1,10)==10&&traffic::blowoutUrgency(s)>0);
        assert(traffic::extraQuarterAllowance(s,-1,100)==10&&traffic::extraQuarterAllowance(s,-1,0)==0);
        s.seconds=length*.5f+.01f;assert(!traffic::extraQuarterAllowance(s,-1,10));
        s.seconds=length*.45f;assert(traffic::activation(s)>0&&traffic::activation(s)<.35f);
        s.seconds=length/3.f;assert(std::fabs(traffic::activation(s)-.35f)<.0001f);
        for(int deficit:{17,21,27}){s.awayScore=deficit;s.seconds=length/3.f+.01f;assert(!traffic::lateHomeGame(s));
            s.seconds=length/3.f;assert(!traffic::activation(s));s.seconds=length*.30f;assert(traffic::activation(s)>.99f);}
        s.awayScore=16;s.seconds=length/3.f;assert(!traffic::lateHomeGame(s));
        for(int deficit:{0,8,16,17,21,27,28,35,42,70}){s.awayScore=deficit;s.seconds=0;int value=traffic::extraQuarterAllowance(s,-1,10);
            assert(value>=0&&value<=10);if(deficit<28)assert(value==0);else assert(value>0);}
    }
    State s{4,0,35,5,120,720};s.homeTimeouts=0;s.offense=1;
    for(int blocked=0;blocked<7;++blocked){State x=s;if(blocked==0)x.playoff=true;if(blocked==1)x.neutral=true;if(blocked==2)x.quarter=3;if(blocked==3)x.quarter=5;if(blocked==4)x.phase=7;if(blocked==5)x.homeScore=42;if(blocked==6)x.seconds=361;assert(!traffic::extraQuarterAllowance(x,-1,10));}
    // Better comeback context never increases the traffic allowance.
    s.awayScore=28;s.seconds=170;s.offense=0;s.homeTimeouts=3;auto hopeful=traffic::extraQuarterAllowance(s,-1,10);s.offense=1;s.homeTimeouts=0;assert(traffic::extraQuarterAllowance(s,-1,10)>=hopeful);
    // Postseason targets retain their original scale and activation ramp.
    s={4,0,35,5,171,720};s.playoff=true;float old=lossCeiling(s)*.60f*context::smooth((180-s.seconds)/36.f);assert(std::fabs(traffic::lossTarget(s)-old)<.01f);
    for(int length:{60,360,720,1200})for(int deficit:{21,28,35,42}){
        int base=run(deficit,5,0,length,1),enhanced=run(deficit,5,10,length,1);assert(enhanced>=base);if(deficit<28)assert(enhanced==base);
        int rare=run(deficit,5,10,length,(std::max)(1,length/6));assert(std::abs(enhanced-rare)<=1);
    }
    assert(run(35,5,10)>run(35,5,0));assert(run(42,20,10)<=75&&run(42,20,10)>=run(42,20,0));
    assert(run(42,20,10,720,30,true)<=85);
    assert(run(42,5,10,720,30,false,true)==run(42,5,0,720,30,false,true));
    assert(run(42,5,10,720,30,false,false,true)==run(42,5,0,720,30,false,false,true));
    // Quarter exceptions require observed enabled formula/traffic policy.
    for(int disabled=0;disabled<5;++disabled){Settings c;History h;departures::Model m;State x{4,0,42,5,90,720};x.homeTimeouts=0;x.offense=1;
        if(disabled==0)c.lateTraffic=0;if(disabled==1)c.formulaStrength=0;if(disabled==2)c.enabled=0;if(disabled==3)c.lateTrafficQuarterExtra=0;if(disabled==4)h.comeback=true;
        m.observe(c,x,h);assert(m.quarterLimit(x,elapsedGameTime(x),45)==45);}
    // Scoring/possession reversals never refill earlier earned exits. Locked
    // live views do not commit pressure; the next safe window owns rendering.
    {Settings c;History h;departures::Model m;State x{4,0,42,5,180,720};x.homeTimeouts=0;x.offense=1;m.observe(c,x,h);m.settle(x,5);
        for(int clock=179;clock>=120;--clock){x.seconds=float(clock);m.observe(c,x,h);}assert(m.committedLoss==5&&m.plannedLoss>5);
        m.beginWindow(4);auto next=m.settle(x,5);assert(next.loss<=9);int prior=next.loss;x.homeScore=43;x.seconds=119;m.observe(c,x,h);assert(m.settle(x,prior).loss>=prior);}
    std::puts("PASS: faster quarter-scaled home traffic, bounded 0-10-point 28+ exception, severity/outlook, fixed zero/postseason/neutral limits, cadence-independent targets, crowd floors, safe-window commitment and no refill.");
}
