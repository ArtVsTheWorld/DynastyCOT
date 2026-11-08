#include "../Source/SituationalReactions.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
struct Case {
    situational::Model m;energy::Play p,snap;energy::Outcome result;uint64_t sequence=0,time=1000;bool allowed=true;
    Case(int side=0,bool neutral=false){p.game={2,7,7,1,400,720};p.game.neutral=neutral;p.offense=side;p.down=3;p.fieldSpot=-30;p.attackDirection=1;p.toFirst=10;snap=p;tick(1,0,2,0,true);sequence=1;tick(3,0,2,0,true);}
    situational::Outcome tick(int phase,int kind,int ball,float gain,bool held){p.game.phase=phase;time+=100;return m.observe(p,snap,result,123,1,sequence,time,allowed,kind,ball,-30+gain,-30+gain,held);}
    void finish(float gain,bool first=false){result.accepted=result.completed=true;result.sequence=sequence;result.yards=gain;result.firstDown=first;p.fieldSpot=snap.fieldSpot+gain;p.down=first?1:4;}
};
int main(){
    // Native live catch transition can be shorter than an observer tick.
    // Reproduce the 33-yard completion and delayed 5-yard conversion without
    // supplying that transition: neither may invent an incompletion sound.
    for(int side:{0,1})for(bool neutral:{false,true})for(int phase:{1,4,5})for(float gain:{5.f,33.f,-2.f}){
        Case c(side,neutral);c.tick(3,3,1,32,false);c.finish(gain,true);
        auto out=c.tick(phase,3,1,32,false);
        assert(out.event!=situational::Event::DeepIncomplete&&out.event!=situational::Event::DeepDefensiveIncomplete);
    }
    {Case c;c.tick(3,3,1,20,false);c.finish(0);auto o=c.tick(4,3,1,20,false);assert(o.type==95&&o.event==situational::Event::FailedConversion);assert(!c.tick(4,3,1,20,false).id);}
    {Case c;c.snap.down=c.p.down=1;c.tick(3,3,1,15,false);c.finish(0);assert(c.tick(4,3,1,15,false).event==situational::Event::DeepIncomplete);}
    {Case c(1);c.tick(3,3,1,15,false);c.finish(0);auto o=c.tick(4,3,1,15,false);assert(o.type==59&&o.event==situational::Event::DeepDefensiveIncomplete);}
    {Case c(1);c.tick(3,3,1,14.9f,false);c.finish(0);assert(!c.tick(4,3,1,14.9f,false).id);}
    {Case c(1);c.tick(3,3,1,15,false);auto o=c.tick(3,1,4,15,true);assert(o.type==95&&o.event==situational::Event::DeepVisitorCatch);assert(!c.tick(3,1,4,30,true).id);}
    {Case c;c.tick(3,3,1,20,false);c.p.offense=1;auto o=c.tick(3,2,4,20,true);assert(o.type==95&&o.event==situational::Event::HomeInterception);}
    {Case c(1);c.finish(-1);auto o=c.tick(4,0,9,-1,false);assert(o.type==59&&o.event==situational::Event::HomeTackleForLoss);}
    {Case c(1);c.finish(-.1f);assert(!c.tick(4,0,9,-.1f,false).id);}
    {Case c;c.finish(12,true);assert(!c.tick(4,1,4,12,true).id);}
    {Case c;c.finish(0);c.result.homePoints=3;assert(!c.tick(4,0,9,0,false).id);}
    for(int side:{0,1}){
        Case c(side,true);c.tick(3,3,1,20,false);assert(!c.tick(3,1,4,20,true).id); // Split crowd: no global letdown for a positive catch.
        Case d(side,true);d.tick(3,3,1,20,false);d.finish(0);auto o=d.tick(4,3,1,20,false);assert(o.type==59&&o.event==situational::Event::DeepDefensiveIncomplete);
    }
    {Case c;c.allowed=false;c.tick(3,3,1,20,false);c.allowed=true;c.finish(0);assert(!c.tick(4,3,1,20,false).id);}
    {Case c;c.tick(3,3,1,20,false);c.time+=1000;c.finish(0);assert(!c.tick(4,3,1,20,false).id);}
    {Case c;c.p.simulated=true;c.tick(3,3,1,20,false);c.finish(0);assert(!c.tick(4,3,1,20,false).id);}
    // Layered OT still needs a disappointed home-crowd sound for visitor
    // successes; the below-layered sound-only fallback does not run there.
    for(int side:{0,1})for(bool neutral:{false,true})for(bool made:{false,true})for(bool kick:{false,true}){
        situational::Model m;energy::Play p;p.game={5,24,24,1,0,720};p.game.neutral=neutral;p.down=kick?4:-1;p.offense=side;p.selectedType=kick?21:103;
        auto snap=p;energy::Outcome r;m.observe(p,snap,r,123,1,0,1000,true,0,2,0,0,true,p.selectedType);
        p.game.phase=3;m.observe(p,snap,r,123,1,1,1100,true,0,2,0,0,true,p.selectedType);
        p.game.phase=4;p.offense=1-side;r.accepted=r.completed=true;r.sequence=1;r.side=made?side:1-side;
        if(made){if(side==0)r.homePoints=kick?3:2;else r.awayPoints=kick?3:2;}
        auto out=m.observe(p,snap,r,123,1,1,1200,true,0,9,0,0,false,p.selectedType);
        if(made){assert(bool(out.id)==(!neutral&&side==1));if(out.id)assert(out.type==95&&out.event==situational::Event::OvertimeVisitorScore);}
        else{assert(out.id&&out.type==(!neutral&&side==0?95:59));if(!kick&&(neutral||side==1))assert(out.event==situational::Event::OvertimeDefensiveConversionStop);}
        assert(!m.observe(p,snap,r,123,1,1,1300,true,0,9,0,0,false,p.selectedType).id);
    }
    std::puts("PASS: home disappointment, deep incompletions, away catch/INT letdown, home TFL, neutral symmetry, OT tries/kicks and layered visitor scores, dedupe, stale/sim/lost-context guards.");
}
