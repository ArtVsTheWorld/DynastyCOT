#include "../Source/LiveReactions.h"
#include "../Source/ReturnAtmosphere.h"
#include "../Source/DirectorShake.h"
#include "../Source/PlayReactionTiming.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
struct Test {
    live::Returns returns;live::ReturnAtmosphere sound;motion::Model camera;motion::Settings settings;
    energy::Play p;uint64_t now=1000;int direction,type;bool allowed=true;
    Test(int d=1,int t=22,int kicker=1,bool neutral=false):direction(d),type(t){
        p.game={1,0,0,1,700,720};p.game.neutral=neutral;p.down=0;p.offense=kicker;
        tick(1,20,false);tick(3,20,false);tick(3,-40,false);p.offense=1-kicker;tick(3,-40,true);
    }
    live::Outcome tick(int phase,float spot,bool held){now+=100;p.game.phase=phase;
        auto event=returns.observe(p,123,1,now,allowed,type,spot*direction,held);
        auto build=returns.buildup();sound.observe(p,123,1,now,allowed,event,held,p.fumble,5000,build);
        motion::Input in;in.match=123;in.epoch=1;in.now=now;in.phase=phase;in.allowed=in.duringAllowed=allowed;
        in.runBuildup=build.id;in.runProgress=build.progress;in.duringPlay=event.id;camera.observe(settings,in);return event;
    }
    float movement(){motion::Input in;in.match=123;in.epoch=1;in.now=now;in.phase=p.game.phase;in.allowed=in.duringAllowed=allowed;auto build=returns.buildup();in.runBuildup=build.id;in.runProgress=build.progress;return motion::strength(camera.observe(settings,in),now);}
    sustained::Intent voice(){sustained::Intent out;out.active=true;out.match=123;out.epoch=1;out.updated=now;return sound.merge(out,now,1);}
};
int main(){
    for(int direction:{-1,1})for(int type:{22,23,24})for(int kicker:{0,1})for(bool neutral:{false,true}){
        Test t(direction,type,kicker,neutral);bool own=kicker==1||neutral;uint64_t rampStart=0;int start=type==22?30:25,peak=start+10;
        for(int gain=1;gain<=45;++gain){auto event=t.tick(3,-40.f+gain,true);assert(bool(event.id)==(own&&gain==peak));
            if(event.id)assert(event.event==(type==22?live::Event::PuntReturn:live::Event::KickReturn)&&event.yards==peak);
            if(!own||gain<start){assert(!t.returns.buildup().id);assert(t.voice().reason==sustained::Reason::None);}
            if(own&&gain==start){assert(t.returns.buildup().id);rampStart=t.returns.buildup().id;assert(t.movement()<1.01f);
                auto swell=t.voice();assert(swell.reason==sustained::Reason::Result&&swell.continuous&&swell.supporting&&swell.intensity>0&&swell.intensity<300&&!swell.liveCheer&&!swell.resultCheer);}
            if(own&&gain==start+5){assert(std::fabs(t.returns.buildup().progress-.5f)<.0001f);assert(std::fabs(t.movement()-5.5f)<.01f);assert(t.voice().intensity<950);}
            if(own&&gain>=peak){assert(t.returns.buildup().id==rampStart&&t.returns.buildup().progress==1);assert(t.movement()==10&&t.voice().intensity==950);}
        }
        if(own){t.tick(4,5,false);assert(t.movement()==10);for(int n=1;n<50;++n){t.tick(4,5,false);assert(t.movement()<=10&&t.voice().intensity<=950);}t.tick(4,5,false);assert(t.movement()==0);}
    }
    {Test t;for(int n=1;n<=35;++n)t.tick(3,-40.f+n,true);t.tick(4,-5,false);assert(t.movement()<6&&t.voice().intensity<950);}
    {Test t;assert(!t.tick(3,0,true).id&&!t.returns.buildup().id);} // Teleport cannot fake a 40-yard return.
    {Test t;for(int n=1;n<=35;++n)t.tick(3,-40.f+n,true);t.p.fumble=true;t.tick(3,-5,false);t.p.fumble=false;
        assert(!t.tick(3,1,true).id&&!t.returns.buildup().id);}
    {Test t;for(int n=1;n<=35;++n)t.tick(3,-40.f+n,true);t.now+=900;assert(!t.tick(3,0,true).id&&!t.returns.buildup().id);}
    // A partial kick-return buildup does not borrow the confirmed-return hold.
    timing::Distance distance;live::Settings limits;energy::Play p;p.game={1,0,0,3,700,720};p.down=0;p.offense=0;
    distance.observe(p,123,1,1,1000,true,{},0,0,false,1,true,{1000,.5f,35},35,limits,0);assert(!distance.ownsDuration());
    distance.observe(p,123,1,1,1100,true,{live::Event::KickReturn,1,0,40},0,0,false,1,true,{1000,1,40},40,limits,0);assert(distance.ownsDuration());
    std::puts("PASS: kickoffs25-to35 and punts30-to40, one peak cheer, mirrored/neutral ownership, synchronized tails, partial-stop limits and fumble/teleport/stale rejection.");
}
