#include "../Source/VisitorCheerGate.h"
#include "../Source/AwayCrowdAudio.h"
#include <cassert>
#include <chrono>
#include <cstdio>
using namespace attendance;
int main(){
 visitors::Population p;p.verified=true;p.match=123;p.epoch=7;p.updated=1000;p.home=801;p.away=199;p.originalHome=p.originalAway=10000;
 assert(!visitorcheers::eligible(p,123,7,1000));p.home=800;p.away=200;assert(visitorcheers::eligible(p,123,7,1000));
 p.home=799;p.away=201;assert(visitorcheers::eligible(p,123,7,1000));
 assert(!visitorcheers::eligible(p,124,7,1000)&&!visitorcheers::eligible(p,123,8,1000));
 assert(visitorcheers::eligible(p,123,7,4000)&&!visitorcheers::eligible(p,123,7,4001)&&!visitorcheers::eligible(p,123,7,999));
 p.updated=0;assert(!visitorcheers::eligible(p,123,7,1000));p.updated=1000;p.verified=false;assert(!visitorcheers::eligible(p,123,7,1000));p.verified=true;
 // Same original share, different current attendance: arrivals and departures
 // determine eligibility; original/planned capacity never enters the rule.
 p.originalHome=8000;p.originalAway=2000;p.home=800;p.away=199;assert(!visitorcheers::eligible(p,123,7,1000));
 p.home=400;p.away=100;assert(visitorcheers::eligible(p,123,7,1000));p.away=99;assert(!visitorcheers::eligible(p,123,7,1000));
 p.home=p.away=0;assert(!visitorcheers::eligible(p,123,7,1000));
 sustained::Intent i;i.active=true;i.match=123;i.epoch=7;i.opponentCheer=92;i.opponentPositive=true;i.visitorLayer=true;
 i.liveCheer=5;i.extraCheer=6;i.resultCheer=7;i.celebration=8;i.intensity=100;i.continuous=i.supporting=i.reaction=true;i.reason=sustained::Reason::Result;
 auto quiet=visitorcheers::restrain(i,false);assert(!quiet.opponentCheer&&!quiet.intensity&&!quiet.continuous&&!quiet.supporting);
 assert(quiet.liveCheer==5&&quiet.extraCheer==6&&quiet.resultCheer==7&&quiet.celebration==8); // Home routes unaffected.
 auto home=i;home.opponentPositive=home.visitorLayer=false;assert(visitorcheers::restrain(home,false).opponentCheer==92&&visitorcheers::restrain(home,false).intensity==100);
 auto spring=i;spring.spring=true;assert(visitorcheers::restrain(spring,false).opponentCheer==92);
 auto diagnostic=i;diagnostic.reason=sustained::Reason::Diagnostics;assert(visitorcheers::restrain(diagnostic,false).opponentCheer==92);
 // Exact integer boundary for totals not divisible by five.
 p.home=800;p.away=200;assert(visitorcheers::eligible(p,123,7,1000));p.home=801;assert(!visitorcheers::eligible(p,123,7,1000));++p.away;assert(visitorcheers::eligible(p,123,7,1000));
 auto start=std::chrono::steady_clock::now();volatile unsigned count=0;for(unsigned n=0;n<200000;++n)count+=visitorcheers::eligible(p,123,7,1000);
 auto us=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-start).count();
 printf("PASS: inclusive20%%, current owned fresh supporter counts, empty/stale/invalid scopes, arrivals/departures, home/negative/spring/diagnostic preservation; 200000 gates %.3f ms (%u)\n",us/1000,count);
}
