#include "../Source/LiveReactions.h"
#include "../Source/ReturnAtmosphere.h"
#include "../Source/DirectorShake.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    for(int kind:{0,3})for(int direction:{-1,1}){
        live::Model runner;live::Settings settings;assert(settings.minimumGainYards==15&&settings.minimumCatchYards==20);
        live::ReturnAtmosphere sound;energy::Play p;p.game={2,7,7,1,300,720};p.offense=0;p.down=1;p.fieldSpot=-30.f*direction;p.attackDirection=direction;
        uint64_t now=1000;auto tick=[&](int phase,float yards,int ball=9,bool held=true){
            p.game.phase=phase;now+=100;auto event=runner.observe(settings,p,123,1,now,true,456,kind,ball,0,p.fieldSpot+yards*direction,held);
            sound.observe(p,123,1,now,true,event,held,false,4500,runner.runBuildup());return event;
        };
        tick(1,0);tick(3,0);for(float gain=1;gain<11;++gain)assert(!tick(3,gain).id&&!runner.runBuildup().id);
        assert(!tick(3,11).id&&runner.runBuildup().id);
        sustained::Intent base;base.active=true;base.match=123;base.epoch=1;
        auto buildup=sound.merge(base,now,.9f);assert(buildup.continuous&&buildup.supporting&&buildup.reason==sustained::Reason::Result);
        for(float gain=12;gain<15;++gain)assert(!tick(3,gain).id);
        auto hit=tick(3,15);assert(hit.event==live::Event::LongRun&&runner.runBuildup().progress==1);
        auto peak=sound.merge(base,now,.9f);assert(peak.supporting&&peak.continuous&&peak.size==3&&peak.intensity>900);
        assert(!tick(3,20,1).id&&runner.runBuildup().progress==1); // Tucked QB changes state while moving sideways, not a thrown pass.
        assert(!tick(3,14,9,false).id&&runner.runBuildup().id); // Brief carrier-pointer dropout.
        assert(!tick(3,14).id&&runner.runBuildup().progress==1); // Backward cut does not lower the established reaction.
        tick(4,14);auto fading=sound.merge(base,now,.9f);assert(!runner.runBuildup().id&&fading.continuous&&!fading.reaction&&fading.supporting&&fading.intensity>0&&fading.releaseMs==3375);
        base.celebration=1;assert(sound.merge(base,now,.9f).reason==sustained::Reason::None); // Winning celebration keeps priority.
    }
    {live::Model m;live::Settings s;energy::Play p;p.game={2,7,7,1,300,720};p.offense=0;p.down=1;p.fieldSpot=-30;p.attackDirection=1;uint64_t t=1000;
        auto step=[&](int phase,int kind,int ball,float caught,float gain,bool held){p.game.phase=phase;t+=100;return m.observe(s,p,123,1,t,true,456,kind,ball,-30+caught,-30+gain,held);};
        step(1,0,2,0,0,false);step(3,0,2,0,0,false);step(3,3,1,0,0,false);
        assert(!step(3,1,4,15,15,true).id&&!m.runBuildup().id); // A 15-air-yard immediate tackle is not a 20-yard long catch or a catch-and-run.
        step(4,1,4,15,15,false);step(1,0,2,0,0,false);step(3,0,2,0,0,false);step(3,3,1,0,0,false);step(3,1,4,2,2,true);
        for(float gain=3;gain<15;++gain)assert(!step(3,1,4,2,gain,true).id);
        assert(step(3,1,4,2,15,true).event==live::Event::RunAfterCatch);
        step(1,0,2,0,0,false);step(3,0,2,0,0,false);step(3,3,1,0,0,false);assert(step(4,1,4,20,20,false).event==live::Event::LongCatch);
    }
    {motion::Model camera;motion::Settings s;motion::Input x;x.match=123;x.epoch=1;x.allowed=x.duringAllowed=true;x.phase=4;x.now=1000;x.duringPlay=1;x.duringImpact=true;
        auto cue=camera.observe(s,x);assert(cue.until==7400);x.now=7400;assert(camera.observe(s,x).kind==motion::Kind::None);}
    std::puts("PASS: fixed 11-to-15-yard runs/YAC with fifteen-yard cheer, supporting roar/gameplay swell, one cheer trigger, lateral/QB-state/carrier-gap continuity, independent 20-air-yard catches, whistle/eruption priority and 6.4-second impacts.");
}
