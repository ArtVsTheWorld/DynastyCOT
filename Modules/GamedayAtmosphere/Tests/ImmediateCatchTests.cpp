#include "../Source/LiveReactions.h"
#include "../Source/DirectorShake.h"
#include <cassert>
#include <iostream>
using namespace attendance;
struct Case {
    live::Settings config;live::Model model;energy::Play p;energy::Outcome result;
    uintptr_t owner=123,record=456;unsigned epoch=1;bool allowed=true;
    Case(){p.game={1,7,3,1,100,720};p.offense=0;p.down=1;p.fieldSpot=27.536476f;p.attackDirection=1;p.toFirst=10;
        result.accepted=result.completed=true;result.sequence=1;result.side=0;result.yards=20;}
    live::Outcome tick(uint64_t now,int phase,int kind,int ball,float caught=0,float runner=NAN,bool held=false,bool proof=false){
        p.game.phase=phase;return model.observe(config,p,owner,epoch,now,allowed,record,kind,ball,caught,runner,held,false,proof?&result:nullptr);
    }
    void begin(){tick(1000,1,1,4);tick(1100,3,0,2);tick(1200,3,3,1);}
};
int main(){
    for(int phase:{4,5}){
        Case c;c.begin();assert(!c.tick(1216,phase,3,1).id);
        auto delayed=c.tick(1248,phase,1,4,48);assert(delayed.event==live::Event::LongCatch&&delayed.yards==20);
        assert(!c.tick(1280,phase,1,4,48).id); // Same result cannot restart.
    }
    for(int direction:{-1,1})for(int phase:{4,5}){
        Case c;c.p.fieldSpot=direction==1?27.536476f:-27.536476f;c.p.attackDirection=direction;
        c.begin();assert(!c.tick(1216,phase,3,1).id);
        auto credited=c.tick(1248,phase,1,4,46.f*direction,NAN,false,true);
        assert(credited.event==live::Event::CompletedPass&&credited.yards==20);
        assert(!c.tick(1280,phase,1,4,46.f*direction,NAN,false,true).id);
    }
    // Catch-and-run ending just before a 20-yard live sample still qualifies
    // from the current completed-play proof, without requiring a held dead ball.
    {Case c;c.p.fieldSpot=-20;c.begin();assert(!c.tick(1216,3,1,4,-18,-.1f,true).id);
        assert(c.tick(1248,4,1,4,-18,NAN,false,true).event==live::Event::CompletedPass);}
    {Case c;c.tick(1000,1,0,1);c.tick(1100,3,0,9,0,45,true);
        assert(c.tick(1216,4,0,1,0,NAN,false,true).event==live::Event::LongRun);}
    for(float yards:{19.f,19.49f,19.5f,20.f,20.1f}){
        Case c;c.begin();c.result.yards=yards;
        auto out=c.tick(1216,4,1,4,46,NAN,false,true);assert(bool(out.id)==(yards>=19.5f));
    }
    for(int invalid=0;invalid<11;++invalid){
        Case c;c.begin();c.tick(1216,4,3,1);
        if(invalid==0)c.record=789;
        if(invalid==1)c.allowed=false;
        if(invalid==2)c.epoch=2;
        if(invalid==3)c.owner=999;
        if(invalid==4)c.p.simulated=true;
        if(invalid==5)c.result.side=1;
        if(invalid==6)c.result.sequence=0;
        if(invalid==7)c.result.takeaway=true;
        if(invalid==8)c.result.setback=true;
        if(invalid==9)c.result.completed=false;
        if(invalid==10)c.result.yards=NAN;
        assert(!c.tick(1248,4,1,4,46,NAN,false,true).id);
    }
    {Case c;c.begin();assert(!c.tick(1216,4,3,1).id);c.tick(1700,4,3,1);c.tick(2200,4,3,1);
        assert(!c.tick(2248,4,1,4,48,NAN,false,true).id);} // Expired one-second window.
    {Case c;c.begin();c.tick(1216,4,3,1);c.tick(1248,1,3,1);
        assert(!c.tick(1280,4,1,4,48,NAN,false,true).id);} // New lineup retires prior snap.
    {Case c;c.begin();assert(!c.tick(1600,4,1,4,48,NAN,false,true).id);} // Unobserved transition after a gap.
    {Case c;c.begin();c.result.yards=25;assert(!c.tick(1216,4,3,1,0,NAN,false,true).id);} // Incomplete pass.
    {Case c;c.begin();c.config.enabled=false;assert(!c.tick(1216,4,1,4,48,NAN,false,true).id);}
    // Production requests an impact at phase 5 too, with the established fade.
    motion::Settings settings;settings.live=10;settings.catchImpact=8;motion::Model camera;motion::Input x;x.match=123;x.epoch=1;x.allowed=true;x.phase=5;x.now=5000;x.duringPlay=1;x.duringImpact=true;
    auto cue=camera.observe(settings,x);assert(cue.impactOnly&&cue.kind==motion::Kind::LivePlay);
    x.now=5200;cue=camera.observe(settings,x);assert(motion::strength(cue,x.now)>9);
    x.now=8150;cue=camera.observe(settings,x);assert(motion::strength(cue,x.now)>0&&motion::strength(cue,x.now)<7);
    x.now=11400;assert(camera.observe(settings,x).kind==motion::Kind::None);
    std::cout<<"PASS: late whistle/selection completions, exact 20-yard credited pass/run, mirrored coordinates, one-second proof window, no stale/sim/away/new-world duplicate, and short postplay impact/fade.\n";
}
