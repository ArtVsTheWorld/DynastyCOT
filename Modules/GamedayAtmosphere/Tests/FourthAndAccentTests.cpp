#include "../Source/LiveReactions.h"
#include "../Source/DirectorShake.h"
#include "../Source/MotionSoundPairing.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    for(int direction:{-1,1}){
        live::Model model;live::Settings c;energy::Play p;p.game={4,26,17,1,60,720};p.down=4;p.toFirst=2;p.offense=0;p.fieldSpot=-30.f*direction;p.attackDirection=direction;
        auto tick=[&](uint64_t now,int phase,int kind,int ball,float catchSpot,float runner,bool held){p.game.phase=phase;return model.observe(c,p,123,1,now,true,456,kind,ball,catchSpot*direction,runner*direction,held,true);};
        tick(1000,1,0,2,0,-30,true);tick(1100,3,0,2,0,-30,true);tick(1200,3,1,1,0,NAN,false);
        auto fourth=tick(1300,3,1,3,-27,-27,true);assert(fourth.event==live::Event::FourthConversion&&fourth.yards==3);
        assert(!tick(1400,3,1,3,-27,-26,true).id);
        live::Outcome yac;for(int i=1;i<=20;++i){auto out=tick(1400+i*100,3,1,3,-27,-26.f+i,true);if(out.id)yac=out;}
        assert(yac.event==live::Event::RunAfterCatch&&yac.yards>=15&&yac.id!=fourth.id);
        assert(!tick(3500,3,1,3,-27,-5,true).id);
    }
    // A fourth-down run conversion is also separate from its long-gain milestone.
    {live::Model m;live::Settings c;energy::Play p;p.game={3,14,14,1,100,720};p.offense=0;p.down=4;p.toFirst=1;p.fieldSpot=-20;p.attackDirection=1;
        auto tick=[&](uint64_t now,int phase,float runner){p.game.phase=phase;return m.observe(c,p,123,1,now,true,456,0,9,0,runner,true,true);};
        tick(1000,1,-20);tick(1100,3,-20);assert(tick(1200,3,-18).event==live::Event::FourthConversion);
        live::Outcome run;for(int i=1;i<=20;++i){auto a=tick(1200+i*100,3,-18.f+i);if(a.id)run=a;}assert(run.event==live::Event::LongRun);}
    // Production result fallback: a live conversion is already delivered,
    // even if a skill or20-yard gain has since replaced its camera cue.
    {energy::Outcome result;result.accepted=result.completed=result.firstDown=true;result.sequence=75;result.side=0;result.event=energy::Event::FourthDown;
     State state{4,28,21,4,140,720};
     assert(live::completedImpact(result,state,true,0,59).event==live::Event::FourthConversion);
     assert(!live::completedImpact(result,state,true,0,59,true).id);
     result.event=energy::Event::FumbleRecovery;result.firstDown=false;result.takeaway=true;
     assert(live::completedImpact(result,state,true,0,59,true).event==live::Event::FumbleRecovery);}
    motion::Settings settings;settings.live=10;settings.catchImpact=8;motion::Model m;motion::Input x;x.match=123;x.epoch=1;x.allowed=x.duringAllowed=true;x.phase=3;x.now=1000;
    m.observe(settings,x);x.now=1100;x.duringPlay=1;x.duringImpact=x.supplementalImpact=true;x.impactMilliseconds=2400;x.reactionScale=.7f;
    auto a=m.observe(settings,x);assert(a.kind==motion::Kind::LivePlay&&a.impactOnly&&a.percent==7&&a.until==3500);
    x.now=1300;a=m.observe(settings,x);assert(motion::strength(a,1300)>0);
    x.now=1400;x.duringPlay=2;a=m.observe(settings,x);assert(a.began==1100); // The first accent still wins at the instant of the new attack.
    x.now=1500;x.supplementalImpact=false;x.reactionScale=1;x.impactMilliseconds=0;x.duringPlay=3;a=m.observe(settings,x);assert(a.began==1500&&a.percent==10);
    x.phase=4;x.now=1700;x.touchdown=1;a=m.observe(settings,x);assert(a.kind==motion::Kind::Touchdown);
    x.now=1800;x.duringPlay=4;x.supplementalImpact=true;a=m.observe(settings,x);assert(a.kind==motion::Kind::Touchdown);
    x.now=1900;x.eruption=1;x.eruptionAlive=true;a=m.observe(settings,x);assert(a.kind==motion::Kind::Eruption);
    x.now=2000;x.clincher=1;a=m.observe(settings,x);assert(a.kind==motion::Kind::Eruption);
    x.allowed=false;x.now=2100;assert(m.observe(settings,x).kind==motion::Kind::None);
    std::puts("PASS: immediate fourth-down catch/run crossing, independent later YAC/run accent; short bounded skill impacts; no supplemental timer restart; touchdown/eruption priority and suspension cleanup.");
}
