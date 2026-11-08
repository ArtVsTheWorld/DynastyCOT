#include "../Source/OpeningKickoffAtmosphere.h"
#include "../Source/ReturnAtmosphere.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    for(int formation:{23,24})for(int side:{0,1}){
        kickoff::Model kick;energy::Play p;p.game={1,0,0,1,720,720};p.game.rank0=2;p.game.rank1=7;p.down=0;p.offense=side;
        sustained::Intent base;base.active=true;base.match=123;base.epoch=1;
        assert(kick.observe(p,123,1,1000,true,true,formation,.60f));
        auto first=kick.merge(base,1000);assert(first.cheers&&first.lineupCheer&&first.continuous&&first.supporting&&first.size==3&&first.attackMs==750&&first.intensity>700);
        auto id=first.lineupCheer;
        for(uint64_t now=1250;now<=11000;now+=250){assert(!kick.observe(p,123,1,now,true,true,formation,.60f));auto out=kick.merge(base,now);assert(out.lineupCheer==id&&out.continuous);if(now>=2500)assert(!out.cheers);}
        p.game.phase=3;p.game.seconds=718;
        for(uint64_t now=11250;now<=18000;now+=250){assert(!kick.observe(p,123,1,now,true,false,formation,.60f));auto out=kick.merge(base,now);assert(out.continuous&&out.supporting&&!out.cheers);}
        auto winner=base;winner.celebration=1;winner.reason=sustained::Reason::Eruption;assert(kick.merge(winner,18000).celebration==1);
        auto returned=base;returned.reason=sustained::Reason::Result;returned.priority=4;returned.intensity=999;assert(kick.merge(returned,18000).intensity==999);
        p.game.phase=4;kick.observe(p,123,1,18250,true,false,formation,.60f);float peak=kick.merge(base,18250).intensity;
        p.game.phase=5;float previous=peak;
        for(uint64_t now=18500;now<=22250;now+=250){kick.observe(p,123,1,now,true,false,formation,.60f);auto out=kick.merge(base,now);assert(out.intensity<=previous);previous=out.intensity;}
        assert(!kick.merge(base,22250).continuous&&kick.replacesLineup());
        p.game.phase=1;p.down=1;kick.observe(p,123,1,22500,true,false,3,.60f);assert(!kick.replacesLineup());
        p.down=0;p.game.seconds=720;assert(!kick.observe(p,123,1,22750,true,true,formation,.60f)); // Once per opening, not every kickoff.
        kick.reset();assert(!kick.observe(p,123,1,1000,false,true,formation,.60f)); // No fullness/atmosphere admission.
        p.resumed=true;assert(!kick.observe(p,123,1,1250,true,true,formation,.60f));p.resumed=false;
        p.simulated=true;assert(!kick.observe(p,123,1,1500,true,true,formation,.60f));p.simulated=false;
        p.game.rank0=p.game.rank1=100;assert(!kick.observe(p,123,1,1750,true,true,formation,.60f));
        p.game.rank0=2;p.game.rank1=7;assert(kick.observe(p,123,1,2000,true,true,formation,.60f));kick.suspend();assert(!kick.merge(base,2100).continuous);
    }
    // Crossing a fullness threshold while already lined up starts one buildup,
    // instead of silently waiting for the next play call. Expiry never loops.
    core::Settings c;c.opening=false;lineup::Model layers;energy::Play p;p.game={1,0,0,1,720,720};p.game.rank0=2;p.game.rank1=7;p.down=1;p.offense=1;
    layers.observe(c,p,123,1,1000,true,.55f,true,nullptr,53913);assert(!layers.playContextActive()&&!layers.serial);
    assert(layers.observe(c,p,123,1,1250,true,.75f,true,nullptr,53913));assert(layers.playContextActive()&&layers.serial==1);
    layers.observe(c,p,123,1,2000,true,.75f,true,nullptr,53913);assert(layers.layeredActive());
    layers.observe(c,p,123,1,45000,true,.75f,true,nullptr,53913);assert(layers.serial==1&&!layers.running);
    layers.observe(c,p,123,1,46000,true,.75f,true,nullptr,53913);assert(layers.serial==1&&!layers.running);
    // Return layer already supplies both sustained channels and yields to eruptions.
    live::ReturnAtmosphere returns;live::Outcome gain{live::Event::KickReturn,1,0,40};p.offense=0;p.game.phase=3;
    returns.observe(p,123,1,50000,true,gain,true,false);sustained::Intent base;base.active=true;base.match=123;base.epoch=1;
    auto out=returns.merge(base,50000,.8f);assert(out.continuous&&out.supporting&&out.size==3&&out.priority>=4);
    base.celebration=1;assert(returns.merge(base,50000,.8f).celebration==1);
    std::puts("PASS: immediate opening-kick cheer, sustained roar/swell, four-second whistle fade, return/eruption priority, qualification/simulation/resume guards, one-shot threshold crossing and existing long-return layers.");
}
