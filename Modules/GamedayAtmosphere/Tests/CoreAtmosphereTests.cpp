#include "../Source/CoreAtmosphere.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
bool near(float a,float b){return std::fabs(a-b)<.01f;}
energy::Outcome event(energy::Event e,int side=0,uint64_t sequence=1){energy::Outcome o;o.event=e;o.side=side;o.sequence=sequence;o.accepted=true;return o;}
int main(){
    core::Settings c;energy::Play p;p.game={1,0,0,1,720,720};p.offense=0;
    for(float fraction:{.1f,.4f,.6f,.75f,.8f,.9f,1.f})for(int kind:{0,1,2,3}){
        core::Model m;p.game.rivalry=kind==1;p.game.playoff=kind==2;p.game.bowl=kind==3?47:127;
        m.update(c,p,1,fraction,fraction);assert(near(volume::boostPercent(m.parent),balance::crowdBoost(fraction))&&m.beds==0);
        assert(m.intent(c,p,123,1,1000,true).reason==sustained::Reason::None); // No continuous opening/OT or matchup volume baseline.
    }
    core::Model m;p.game={4,23,20,4,30,720};assert(m.react(c,event(energy::Event::LastSecondLead),0,p.game));
    m.update(c,p,1,1,1);assert(near(volume::boostPercent(m.parent),50));auto w=m.intent(c,p,123,1,1000,true);assert(w.reason==sustained::Reason::Eruption&&w.supporting&&w.celebration);
    assert(!m.react(c,event(energy::Event::LastSecondLead),2,p.game));m.update(c,p,36,1,1);assert(volume::boostPercent(m.parent)>25&&volume::boostPercent(m.parent)<50);
    m.update(c,p,46,1,1);assert(!m.erupting&&near(volume::boostPercent(m.parent),25));
    for(auto e:{energy::Event::ThirdStop,energy::Event::Turnover,energy::Event::OpeningTouchdown}){core::Model x;assert(!x.react(c,event(e),0,p.game));}
    core::Model critical;critical.update(c,p,1,1,1,1);assert(near(volume::boostPercent(critical.parent),35)&&critical.highLeverage);
    critical.update(c,p,2,1,1,.5f);assert(near(volume::boostPercent(critical.parent),30));
    critical.update(c,p,3,1,1,0);assert(near(volume::boostPercent(critical.parent),25)&&!critical.highLeverage);
    assert(near(volume::boostPercent(core::callbackLimit(c,100,1000,0,1500)),35));
    assert(near(volume::boostPercent(core::callbackLimit(c,100,1600,0,1500)),25));
    assert(near(volume::boostPercent(core::callbackLimit(c,100,1000,1500,1500)),50));
    assert(near(volume::boostPercent(core::callbackLimit(c,100,1600,1500,1500)),25));
    core::Model held;p.game={5,20,20,1,0,720};held.update(c,p,0,1,1);p.game.homeScore=23;p.game.phase=7;held.react(c,event(energy::Event::OvertimeWin),1,p.game);held.update(c,p,2,1,1);
    for(double now:{60.,180.,600.}){held.update(c,p,now,1,1);auto carry=held.intent(c,p,123,1,static_cast<uint64_t>(now*1000),true);assert(held.postgameHeld&&carry.continuous&&near(volume::boostPercent(held.parent),50));}
    assert(held.finishPostgame()&&!held.finishPostgame());held.update(c,p,601,1,1);assert(!held.intent(c,p,123,1,601000,true).active&&!held.erupting);
    core::Model upset;p.game={4,28,24,5,10,720};p.game.rank0=127;p.game.rank1=3;upset.update(c,p,0,.8f,.8f);p.game.phase=7;upset.update(c,p,1,.8f,.8f);assert(upset.event==energy::Event::UpsetWin&&upset.celebration==1);upset.update(c,p,200,.8f,.8f);assert(upset.celebration==1);
    core::Model away;p.game={4,20,23,4,20,720};assert(!away.react(c,event(energy::Event::LastSecondLead,1),0,p.game));p.game.neutral=true;assert(away.react(c,event(energy::Event::LastSecondLead,1),0,p.game));
    auto visitor=event(energy::Event::None);visitor.visitorScore=true;p.game.neutral=false;m.react(c,visitor,47,p.game);assert(m.eruptionAt<0);
    c.maximumPercent=10;c.eruptionPercent=25;assert(core::normalLimit(c)<=volume::boostDb(10)+.001f&&core::eruptionLimit(c)<=volume::boostDb(25)+.001f);
    c.enabled=false;assert(core::normalLimit(c)==0);
    std::puts("PASS: attendance-only volume, removed matchup/opening/OT gain, 25/35/50-percent ceilings, native layers, dedupe, postgame/menu hold, upset completion, neutral symmetry and lower custom limits.");
}
