#include "../Source/LiveReactions.h"
#include "../Source/DirectorShake.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    for(int kind:{0,3})for(int direction:{-1,1}){
        live::Model m;live::Settings c;c.minimumGainYards=20;energy::Play p;p.game={1,0,0,1,700,720};p.offense=0;p.down=1;p.fieldSpot=-30.f*direction;p.attackDirection=direction;
        uint64_t time=1000;
        auto tick=[&](int phase,float gain,bool held=true){p.game.phase=phase;time+=500;return m.observe(c,p,123,1,time,true,456,kind,9,0,p.fieldSpot+gain*direction,held);};
        tick(1,0);tick(3,0);assert(!tick(3,10.99f).id&&!m.runBuildup().id);
        assert(!tick(3,11).id&&m.runBuildup().id&&m.runBuildup().progress==.0001f);
        assert(!tick(3,13).id&&std::fabs(m.runBuildup().progress-.5f)<.00001f);
        assert(!tick(3,15).id&&m.runBuildup().progress==1);
        auto hit=tick(3,20);assert(hit.event==live::Event::LongRun&&m.runBuildup().progress==1);
        assert(!tick(3,32).id&&m.runBuildup().progress==1); // No repeated cheer at each yard.
        tick(4,32);assert(!m.runBuildup().id);
        tick(1,0);tick(3,0);assert(!tick(3,30,false).id&&!m.runBuildup().id); // Loose ball cannot create a scramble.
    }
    live::Model m;live::Settings c;c.minimumGainYards=20;energy::Play p;p.game={2,7,7,1,400,720};p.offense=0;p.down=1;p.fieldSpot=-30;p.attackDirection=1;
    uint64_t time=1000;
    auto tick=[&](int phase,int kind,int ball,float gain,bool held){p.game.phase=phase;time+=500;return m.observe(c,p,123,1,time,true,456,kind,ball,-30+gain,-30+gain,held);};
    tick(1,0,2,0,false);tick(3,0,2,0,false);tick(3,3,1,25,false);assert(!m.runBuildup().id);
    assert(tick(3,1,4,20,true).event==live::Event::LongCatch&&!m.runBuildup().id); // Air-yard catch remains immediate.
    tick(1,0,2,0,false);tick(3,0,2,0,false);tick(3,3,1,5,false);assert(!tick(3,1,4,2,true).id);
    assert(!tick(3,1,4,11,true).id&&m.runBuildup().id&&m.runBuildup().progress==.0001f);
    assert(!tick(3,1,4,15,true).id&&m.runBuildup().progress==1);
    assert(tick(3,1,4,20,true).event==live::Event::RunAfterCatch&&m.runBuildup().progress==1);
    m.suspend();assert(!m.runBuildup().id);
    {live::Model guarded;energy::Play shortPass=p;shortPass.game.phase=1;uint64_t t=10000;
        auto sample=[&](int phase,int kind,int ball,float caught,float runner,bool held){shortPass.game.phase=phase;t+=16;return guarded.observe(c,shortPass,123,1,t,true,456,kind,ball,-30+caught,-30+runner,held);};
        sample(1,0,2,0,0,false);sample(3,0,2,0,0,false);sample(3,3,1,0,20,false);
        assert(!sample(3,1,4,25,2,true).id&&guarded.catchAirYards()<0); // Stale long-catch field disagrees with the actual two-yard receiver.
        assert(!sample(3,1,4,2,2,true).id&&guarded.catchAirYards()==2);
        assert(!sample(3,1,4,2,30,true).id&&!guarded.runBuildup().id); // One teleported position cannot latch a long YAC shake.
        assert(!sample(3,1,4,2,3,true).id&&!guarded.runBuildup().id);
        for(float gain=4;gain<=19;++gain)assert(!sample(3,1,4,2,gain,true).id);
        assert(sample(3,1,4,2,20,true).event==live::Event::RunAfterCatch);
    }
    motion::Model camera;motion::Settings s;s.reactionFadeMs=2000;motion::Input x;x.match=123;x.epoch=1;x.phase=3;x.allowed=x.duringAllowed=true;x.now=10000;x.runBuildup=1;x.runProgress=.0001f;
    auto cue=camera.observe(s,x);auto began=cue.began;
    x.now+=200;cue=camera.observe(s,x);assert(std::fabs(motion::strength(cue,x.now)-1)<.001f);
    x.runProgress=.5f;x.now+=200;cue=camera.observe(s,x);assert(motion::strength(cue,x.now)>5.49f&&motion::strength(cue,x.now)<5.51f);
    x.runProgress=1;x.duringPlay=1;x.now+=200;cue=camera.observe(s,x);assert(cue.began==began&&motion::strength(cue,x.now)==10);
    x.duringPlay=0;x.runBuildup=0;x.phase=4;x.now+=100;cue=camera.observe(s,x);assert(cue.tail&&motion::strength(cue,x.now)==10);
    x.now+=1000;cue=camera.observe(s,x);assert(motion::strength(cue,x.now)>7.4f&&motion::strength(cue,x.now)<7.5f);
    x.now+=1000;assert(camera.observe(s,x).kind==motion::Kind::None);
    x.phase=3;x.runBuildup=2;x.runProgress=1;x.duringAllowed=false;x.now+=100;assert(camera.observe(s,x).kind==motion::Kind::None);
    motion::Model dropped;x.duringAllowed=true;x.runProgress=.2f;x.now=20000;dropped.observe(s,x);x.now+=200;auto partial=dropped.observe(s,x);float partialLevel=motion::strength(partial,x.now);
    x.runBuildup=0;x.now+=100;auto fading=dropped.observe(s,x);assert(fading.tail&&motion::strength(fading,x.now)<=partialLevel+.001f); // A lost runner cannot jump to full strength.
    std::puts("PASS: mirrored runs/QB pass-record scrambles, 11-to-15-yard motion ramp, one cheer at 20, immediate air catches, short YAC, loose-ball rejection and two-second settling.");
}
