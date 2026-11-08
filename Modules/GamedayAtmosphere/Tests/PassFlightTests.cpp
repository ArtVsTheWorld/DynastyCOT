#include "../Source/LiveReactions.h"
#include <cassert>
#include <iostream>
using namespace attendance;
int main(){
    live::PassFlight air;energy::Play p;p.game={2,14,14,1,600,720};p.down=2;p.offense=0;
    uint64_t now=1000;bool allowed=true;uintptr_t match=123,record=456;unsigned epoch=1;
    auto sample=[&](int phase,int kind,int ball,bool held){p.game.phase=phase;now+=16;air.observe(p,match,epoch,now,allowed,record,kind,ball,held);};
    auto begin=[&](){sample(1,1,4,true);sample(3,0,2,true);};
    begin();sample(3,3,1,false);now+=160;sample(3,3,1,false);assert(air.weight(now)==1);
    sustained::Intent out;out.active=true;out.match=match;out.epoch=epoch;out.updated=now;
    auto anticipation=air.merge(out,now,.8f);assert(anticipation.reason==sustained::Reason::PassFlight&&anticipation.continuous&&anticipation.intensity>700);
    assert(anticipation.passFlight&&anticipation.supporting&&anticipation.supportingOnly&&sustained::supportingIntent(anticipation).attackMs==500);
    assert(!anticipation.liveCheer&&!anticipation.shake&&!anticipation.celebration);
    out.reason=sustained::Reason::Lineup;out.intensity=1000;auto layered=air.merge(out,now,.8f);assert(layered.intensity==1000&&layered.reason==sustained::Reason::Lineup&&layered.supporting); // Reuse the existing roar and request its supporting swell.
    auto support=sustained::supportingIntent(layered);assert(support.reason==sustained::Reason::PassFlight&&support.passFlight==anticipation.passFlight&&support.attackMs==500&&support.releaseMs==1200);
    for(auto reason:{sustained::Reason::Result,sustained::Reason::Eruption}){layered.reason=reason;assert(!sustained::passAnticipation(layered)&&sustained::supportingIntent(layered).reason==reason);}
    out.reason=sustained::Reason::Eruption;assert(air.merge(out,now,.8f).reason==sustained::Reason::Eruption);
    sample(3,1,4,true);assert(air.weight(now)==1);now+=600;sample(4,1,4,true);assert(air.weight(now)>0&&air.weight(now)<.6f);
    now+=650;sample(4,1,4,true);assert(air.weight(now)==0);
    begin();sample(3,3,4,true);assert(air.weight(now)==0); // Holder/ball 4 is not flight.
    // The reset can already be observed at the line. A quick throw need not
    // repeat kind 0 in a sampled live frame before it can swell.
    sample(1,0,9,true);sample(3,1,1,false);now+=160;sample(3,1,1,false);assert(air.weight(now)==1&&air.airborne());
    sample(3,1,3,true);assert(!air.airborne());
    sample(1,1,9,true);sample(3,1,1,false);now+=160;sample(3,1,1,false);assert(air.weight(now)==0); // Stale completion cannot replace reset proof.
    sample(1,0,9,true);record=789;sample(3,1,1,false);assert(air.weight(now)==0);record=456;
    begin();sample(3,3,1,false);now+=160;sample(3,3,1,false);allowed=false;sample(3,3,1,false);assert(air.weight(now)==0);allowed=true;
    sample(3,3,1,false);assert(air.weight(now)==0); // No mid-flight attachment.
    begin();sample(3,3,1,false);now+=900;sample(3,3,1,false);assert(air.weight(now)==0);
    begin();sample(3,3,1,false);epoch=2;sample(3,3,1,false);assert(air.weight(now)==0);
    begin();record=789;sample(3,3,1,false);assert(air.weight(now)==0);record=456;
    begin();p.simulated=true;sample(3,3,1,false);assert(air.weight(now)==0);p.simulated=false;
    begin();sample(3,3,1,false);for(int i=0;i<82;++i){now+=100;sample(3,3,1,false);}assert(air.weight(now)<1);
    for(int i=0;i<15;++i){now+=100;sample(3,3,1,false);}assert(air.weight(now)==0); // Eight-second maximum, no restart.
    // The anticipation sound belongs to the ball in flight, for both offenses.
    // Reproduce the away third-and-20 pass before its home-defender swat.
    for(int side:{0,1})for(int kind:{1,3}){
        live::PassFlight flight;energy::Play play;play.game={3,20,14,1,400,720};play.down=3;play.toFirst=20;play.offense=side;
        flight.observe(play,123,1,1000,true,456,0,9,true);play.game.phase=3;
        flight.observe(play,123,1,1100,true,456,0,2,true);flight.observe(play,123,1,1200,true,456,kind,1,false);
        flight.observe(play,123,1,1400,true,456,kind,1,false);
        sustained::Intent sound;sound.active=true;sound.match=123;sound.epoch=1;sound.updated=1400;
        sound.reason=sustained::Reason::Lineup;sound.continuous=true;sound.intensity=700;
        auto request=flight.merge(sound,1400,.92f);auto swell=sustained::supportingIntent(request);
        assert(request.passFlight&&swell.reason==sustained::Reason::PassFlight&&swell.attackMs==500&&swell.intensity>0);
        assert(!request.liveCheer&&!request.shake&&!request.celebration);
        play.game.phase=4;flight.observe(play,123,1,1500,true,456,3,1,false);
        for(uint64_t t=1600;t<=2800;t+=100)flight.observe(play,123,1,t,true,456,3,1,false);
        assert(flight.weight(2800)==0);
    }
    std::cout<<"PASS: home/away airborne anticipation including the captured away third-and-20; existing-voice reuse, catch/incompletion fade, once per snap, bounded lease, no guessed catches, pause/sim/world/stale guards.\n";
}
