#include "../Source/LineupAtmosphere.h"
#include "../Source/DirectorShake.h"
#include "../Source/LiveReactions.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
energy::Play opening(int side,float length){energy::Play p;p.game={1,0,0,6,length,length};p.offense=side;p.down=0;p.toGoal=75;p.toFirst=10;return p;}
int main(){
    core::Settings config;motion::Settings settings;
    assert(std::abs(motion::hostileOpeningStrength(6060)-8.f)<.001f&&std::abs(motion::hostileOpeningStrength(61888)-12.5f)<.001f);
    assert(std::abs(motion::hostileOpeningStrength(512)-10*motion::venueScale(512))<.001f&&std::abs(motion::hostileOpeningStrength(200000)-12.5f)<.001f);
    assert(motion::hostileOpeningStrength(0)==0&&motion::hostileOpeningStrength(6060,0)==0);
    float previous=0;for(uint64_t seats=512;seats<=100000;seats+=127){float value=motion::hostileOpeningStrength(seats);assert(value>=previous&&value>6.79f&&value<=12.501f);previous=value;}
    for(int side:{0,1})for(float length:{60.f,300.f,720.f,1200.f})for(uint64_t seats:{6060ull,26101ull,61888ull}){
        auto p=opening(side,length);lineup::Model layers;motion::Model camera;float gate=packed::standardMinimum(seats);
        auto tick=[&](uint64_t now){layers.observe(config,p,123,1,now,true,gate,true,nullptr,seats);};
        tick(1000);assert(!layers.hostileOpening);
        p.game.phase=5;p.down=1;tick(2000);assert(layers.hostileOpening==(side==1));
        auto id=layers.cameraLineupId();auto choice=layers.cameraChoice();assert(id);
        motion::Input x;x.match=123;x.epoch=1;x.allowed=x.venueEligible=x.lineup=true;x.lineupId=id;x.phase=5;x.now=2000;
        x.venueScale=motion::venueScale(seats);x.hostileOpening=layers.hostileOpening?settings.hostileOpeningMaximum:0;
        auto cue=camera.observe(settings,x);assert(motion::strength(cue,x.now)==0);
        x.now=2500;cue=camera.observe(settings,x);float half=motion::strength(cue,x.now);assert(half>0&&half<cue.percent);
        x.now=3000;cue=camera.observe(settings,x);assert(motion::strength(cue,x.now)==cue.percent);
        p.game.phase=1;tick(3000);assert(layers.cameraLineupId()==id&&layers.cameraChoice()==choice&&layers.hostileOpening==(side==1));
        x.phase=1;cue=camera.observe(settings,x);assert(cue.began==2000&&motion::strength(cue,x.now)==cue.percent);
        tick(4000);sustained::Intent intent;intent.active=true;intent.match=123;intent.epoch=1;intent.updated=4000;
        auto sound=layers.merge(intent,config,p,4000,gate);
        assert(sound.hostileOpening==(side==1)&&sound.continuous);
        if(side==1){assert(!sound.cheers&&!sound.deepCheers&&!sound.lineupCheer&&sound.supporting&&sound.attackMs==800&&sound.size==3&&layers.leverage);}
        auto cheer=layers.hostileCheer;
        // Returning to selection before any snap may preserve the opening,
        // but does not schedule another hostile cheer burst.
        p.game.phase=5;tick(4100);p.game.phase=1;tick(4200);assert(layers.hostileCheer==cheer);
        p.game.phase=3;tick(4300);p.game.seconds-=5;p.game.phase=4;tick(4400);assert(!layers.hostileOpening);
        p.game.phase=5;p.down=2;tick(4500);p.game.phase=1;tick(4600);assert(layers.hostileOpening==(side==1));
        // A later first visitor possession is never treated as the opening.
        p.game.phase=3;tick(4700);p.game.phase=4;energy::Outcome firstDown;firstDown.accepted=firstDown.completed=firstDown.firstDown=true;
        layers.observe(config,p,123,1,4750,true,gate,true,&firstDown,seats);p.offense=1;p.game.phase=5;p.down=1;tick(4800);p.game.phase=1;tick(4900);assert(!layers.hostileOpening);
    }
    for(int blocked=0;blocked<7;++blocked){auto p=opening(1,720);lineup::Model m;auto c=config;
        if(blocked==0)c.opening=false;if(blocked==1)c.lineups=false;if(blocked==2)c.enabled=false;
        if(blocked==3)p.resumed=true;if(blocked==4)p.simulated=true;if(blocked==5)p.game.seconds-=10;
        m.observe(c,p,123,1,1000,true,.9f,true,nullptr,6060);p.game.phase=1;p.down=1;
        m.observe(c,p,123,1,2000,true,blocked==6?.89f:.9f,true,nullptr,6060);assert(!m.hostileOpening);
    }
    // A split neutral crowd welcomes EACH team's first Q1 offensive snap,
    // once each, even after an opening score. No replayed/sim/resumed opener.
    for(int first:{0,1})for(float length:{60.f,720.f,1200.f}){
        auto p=opening(first,length);p.game.neutral=true;lineup::Model layers;
        auto tick=[&](uint64_t now){layers.observe(config,p,321,1,now,true,1,true,nullptr,61888);};
        tick(1000);p.game.phase=5;p.down=1;tick(1100);assert(layers.hostileOpening);
        p.game.phase=1;tick(1200);auto firstCheer=layers.hostileCheer;assert(firstCheer);
        p.game.phase=3;tick(1300);p.game.seconds-=5;p.game.phase=4;tick(1400);
        p.game.phase=5;p.down=2;tick(1500);assert(!layers.hostileOpening);
        p.game.phase=1;tick(1600);p.game.phase=3;tick(1700);
        p.game.phase=4;p.game.homeScore=first==0?7:0;p.game.awayScore=first==1?7:0;tick(1800);
        p.game.phase=5;p.offense=1-first;p.down=1;tick(1900);assert(layers.hostileOpening);
        p.game.phase=1;tick(2000);assert(layers.hostileOpening&&layers.hostileCheer!=firstCheer);
        sustained::Intent base;base.active=true;base.match=321;base.epoch=1;base.updated=2000;
        auto sound=layers.merge(base,config,p,2000,1);assert(sound.hostileOpening&&sound.cheers&&sound.lineupCheer==layers.hostileCheer);
        p.game.phase=3;tick(2100);p.offense=first;tick(2150); // A live possession flip is not another snap.
        p.game.phase=4;tick(2200);p.game.phase=5;p.down=1;tick(2300);assert(!layers.hostileOpening);
        p.game.phase=1;tick(2400);p.game.phase=3;tick(2500);p.game.phase=4;tick(2600);
        p.game.phase=5;p.offense=1-first;tick(2700);assert(!layers.hostileOpening);
    }
    for(int blocked=0;blocked<3;++blocked){
        auto p=opening(0,720);p.game.neutral=true;lineup::Model layers;
        layers.observe(config,p,321,1,1000,true,1,true,nullptr,61888);
        p.game.phase=1;p.down=1;p.resumed=blocked==0;p.simulated=blocked==1;if(blocked==2)p.game.quarter=2;
        layers.observe(config,p,321,1,1100,true,1,true,nullptr,61888);assert(!layers.hostileOpening);
    }
    // A fresh native sack on fourth down remains identifiable even when the
    // very first settled sample already belongs to the new offense.
    for(int scenario=0;scenario<7;++scenario){energy::Tracker tracker;energy::Settings rules;auto p=opening(1,720);
        p.game={2,14,14,1,300,720};p.down=scenario==1?3:4;p.resultStamp=10;p.fieldSpot=NAN;p.toGoal=-1;
        if(scenario==2)p.specialTeams=true;
        tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);
        p.game.phase=5;p.game.seconds=295;p.down=1;p.offense=0;p.resultStamp=scenario==3?10:11;p.sack=true;
        if(scenario==4)p.interception=true;if(scenario==5)p.fumble=true;if(scenario==6)p.simulated=true;
        auto out=tracker.observe(rules,p);assert(out.sack==(scenario==0));
        if(scenario==0){assert(!out.fiveYardLoss&&out.yards==-1000);auto impact=live::completedImpact(out,p.game,true,0,83);assert(impact.event==live::Event::Sack&&impact.id);}
    }
    // An earned impact retains priority over the prepared next lineup. Its
    // expiry blends into that lineup, rather than jumping to a mature target.
    motion::Model priority;motion::Input x;x.match=123;x.epoch=1;x.allowed=x.venueEligible=x.lineup=true;x.lineupId=1;x.now=1000;x.phase=5;
    x.duringPlay=1;x.duringImpact=true;auto cue=priority.observe(settings,x);assert(cue.kind==motion::Kind::LivePlay);
    x.now=1200;priority.observe(settings,x);x.now=7400;cue=priority.observe(settings,x);assert(cue.kind==motion::Kind::Ordinary&&motion::strength(cue,x.now)==0);
    x.now=7600;cue=priority.observe(settings,x);assert(motion::strength(cue,x.now)==5);
    x.allowed=false;x.now=7700;assert(priority.observe(settings,x).kind==motion::Kind::None);
    std::puts("PASS: earlier selection ramp, preserved lineup identity/priority, first-away-series hostile profile, shared venue curve, gates/false starts/quarter lengths and fresh fourth-down sack ownership proof.");
}
