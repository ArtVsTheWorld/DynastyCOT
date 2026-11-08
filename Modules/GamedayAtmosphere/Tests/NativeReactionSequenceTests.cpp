#include "../Source/LiveFumble.h"
#include "../Source/CoreAtmosphere.h"
#include "../Source/DirectorShake.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
energy::Play fixture(int side=0){energy::Play p;p.game={2,7,7,1,300,720};p.down=1;p.offense=side;p.toGoal=60;p.toFirst=10;p.fieldSpot=-10;p.attackDirection=1;p.resultStamp=10;return p;}
int main(){
    live::Settings cutoffs;
    // Captured native route: kind 0 snap -> held ball 2 -> fresh kind 1
    // airborne ball 1 -> held ball 3 receiver. No kind-3 flight is required.
    for(int direction:{-1,1})for(float airYards:{2.f,20.f,31.f}){
        auto p=fixture();p.fieldSpot=-10.f*direction;p.attackDirection=direction;live::Model m;live::PassFlight flight;
        uint64_t now=1000;
        auto tick=[&](int phase,int kind,int ball,bool held,float spot){p.game.phase=phase;now+=100;flight.observe(p,123,1,now,true,456,kind,ball,held);return m.observe(cutoffs,p,123,1,now,true,456,kind,ball,spot,spot,held);};
        tick(1,0,9,true,p.fieldSpot);tick(3,0,2,true,p.fieldSpot);tick(3,0,1,false,p.fieldSpot);
        assert(!tick(3,1,1,false,p.fieldSpot).id);tick(3,1,1,false,p.fieldSpot);assert(flight.weight(now)>0);
        float caught=p.fieldSpot+airYards*direction;auto reaction=tick(3,1,3,true,caught);
        if(airYards>=20){assert(reaction.event==live::Event::LongCatch&&reaction.yards==airYards);assert(!tick(3,1,3,true,caught+direction).id);}
        else {assert(!reaction.id);live::Outcome burst;
            for(int yards=3;yards<=20;++yards){auto event=tick(3,1,3,true,p.fieldSpot+yards*direction);if(event.id)burst=event;}
            assert(burst.event==live::Event::RunAfterCatch&&burst.yards==15&&m.runBuildup().progress==1);
        }
    }
    // A runner may already be in held ball state 2 on the first live sample;
    // requiring a transient live ball-9 frame misses that entire run.
    {auto p=fixture();live::Model runs;uint64_t now=1000;runs.observe(cutoffs,p,123,1,now,true,456,0,9,p.fieldSpot,p.fieldSpot,true);
        p.game.phase=3;live::Outcome out;for(int yards=0;yards<=20;++yards){now+=100;auto hit=runs.observe(cutoffs,p,123,1,now,true,456,0,2,p.fieldSpot,p.fieldSpot+yards,true);if(hit.id)out=hit;}
        assert(out.event==live::Event::LongRun&&out.yards==15);}
    // A stale completion cannot create a catch without this play's reset.
    // A fresh held ball-3 receiver can confirm one if its brief flight sample
    // was missed, but an airborne provisional kind-1 record cannot.
    for(int stale=0;stale<2;++stale){auto p=fixture();live::Model m;m.observe(cutoffs,p,123,1,1000,true,456,1,9,-10,-10,true);p.game.phase=3;
        m.observe(cutoffs,p,123,1,1100,true,456,stale?0:1,1,-10,-10,false);
        auto out=m.observe(cutoffs,p,123,1,1200,true,456,1,3,20,20,true);assert(bool(out.id)==(stale==1));}
    for(int recoveryBall:{2,3,9}){auto p=fixture(1);live::Fumbles f;live::ForcedEvent event{999,0,true};
        f.observe(p,123,1,1000,true,event,0,9,false);p.game.phase=3;f.observe(p,123,1,1100,true,event,0,2,true);
        event.type=32;assert(f.observe(p,123,1,1200,true,event,0,5,false,true).event==live::Event::ForcedFumble);
        p.offense=0;assert(f.observe(p,123,1,1300,true,event,4,recoveryBall,true).event==live::Event::FumbleRecovery);}
    // A settled non-scoring result can precede its score publication. Keep
    // only this observed snap eligible, then upgrade sound/motion exactly once.
    for(bool fumble:{false,true})for(bool resumed:{false,true}){
        auto p=fixture(fumble?1:0);p.resumed=resumed;energy::Tracker tracker;energy::Settings rules;core::Model sound;core::Settings config;
        tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);
        p.game.phase=4;p.resultStamp=11;p.game.seconds=295;p.down=fumble?1:2;p.offense=0;p.fumble=fumble;
        auto settled=tracker.observe(rules,p);assert(settled.completed&&!settled.homePoints&&tracker.scorePending);
        sound.react(config,settled,10,p.game,true,true);auto cheer=sound.resultCheer;
        p.game.homeScore=13;p.down=-1;auto td=tracker.observe(rules,p);
        assert(td.completed&&td.sequence==settled.sequence&&td.homePoints==6&&td.side==0&&(td.event==energy::Event::HomeScore||td.event==energy::Event::LongTouchdown)&&!tracker.scorePending);
        sound.react(config,td,10.25,p.game,true,true);assert(sound.resultScoring&&sound.resultCheer==cheer+1);
        assert(!tracker.observe(rules,p).completed);
        motion::Model camera;motion::Settings settings;motion::Input x;x.match=123;x.epoch=1;x.allowed=true;x.phase=4;x.now=1000;x.duringPlay=1;x.duringImpact=true;
        camera.observe(settings,x);x.now=1250;x.touchdown=td.sequence;assert(camera.observe(settings,x).kind==motion::Kind::Touchdown);
    }
    // Stale delayed-score windows expire on game progression, simulation,
    // replay rewind, or the next actual snap; they cannot invent another TD.
    for(int stop=0;stop<4;++stop){auto p=fixture();energy::Tracker tracker;energy::Settings rules;
        tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);p.game.phase=4;p.down=2;p.resultStamp=11;
        tracker.observe(rules,p);assert(tracker.scorePending);
        if(stop==0)p.game.seconds-=6;if(stop==1)p.simulated=true;if(stop==2)p.game.seconds+=1;
        if(stop==3){p.game.phase=1;tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);}
        tracker.observe(rules,p);assert(!tracker.scorePending);
    }
    std::puts("PASS: captured native kind-1/ball-3 completions and YAC, held-ball-2 runs/recoveries, preserved cutoffs and stale/snap guards, once-only delayed TD upgrades with sound/motion takeover, resumed games and bounded score-window cleanup.");
}
