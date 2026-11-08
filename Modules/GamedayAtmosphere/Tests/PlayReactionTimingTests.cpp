#include "../Source/PlayReactionTiming.h"
#include "../Source/ReturnAtmosphere.h"
#include "../Source/DirectorShake.h"
#include "../Source/ResultFreshness.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    // A prior first-down flag cannot turn the reported -7-yard play into a
    // first down; genuinely short positive conversions still work.
    energy::Play before,after;before.offense=after.offense=0;before.down=after.down=1;
    before.fieldSpot=10;before.attackDirection=1;before.toFirst=10;after.fieldSpot=3;
    assert(!energy::earnedFirstDown(before,after,true));after.fieldSpot=15;
    assert(energy::earnedFirstDown(before,after,true));
    for(int phase:{1,4,5}){
        resultfreshness::Window window;energy::Outcome result;
        window.observe(123,1,1,3,1000,result);window.observe(123,1,1,4,1500,result);
        result.accepted=result.completed=true;result.sequence=1;result.yards=5;
        assert(window.observe(123,1,1,phase,3000,result));
        assert(!window.observe(123,1,1,phase,39500,result)); // 38-second delay
        result.homePoints=6;assert(window.observe(123,1,1,phase,39600,result));
        result.homePoints=0;result.sequence=2;assert(window.observe(123,1,2,phase,39700,result));
        assert(window.observe(456,2,2,phase,39800,result)); // world ownership reset
    }
    for(float yards:{0.f,15.f,20.f,30.f,60.f,99.f,110.f,NAN})for(unsigned oldCap:{0u,4000u,8000u})assert(timing::extraMs(yards,15,oldCap)==0);
    for(auto event:{live::Event::LongRun,live::Event::RunAfterCatch,live::Event::Interception,live::Event::FumbleRecovery,live::Event::KickReturn})for(float yards:{15.f,40.f,99.f}){
        energy::Play p;p.game={2,14,14,3,300,720};p.down=1;p.offense=0;
        live::ReturnAtmosphere sound;motion::Model camera;motion::Settings settings;assert(settings.reactionFadeMs==5000);
        sustained::Intent base;base.active=true;base.match=123;base.epoch=1;
        motion::Input x;x.match=123;x.epoch=1;x.allowed=true;x.now=1000;x.phase=3;x.duringPlay=1;
        camera.observe(settings,x);sound.observe(p,123,1,1000,true,{event,1,0,yards},true,false,settings.reactionFadeMs);
        x.now=1250;camera.observe(settings,x);p.game.phase=4;x.phase=4;x.now=1500;x.duringPlay=0;
        auto cue=camera.observe(settings,x);sound.observe(p,123,1,1500,true,{},false,false,settings.reactionFadeMs);assert(cue.until==6500);
        for(uint64_t now=1750;now<6500;now+=250){x.now=now;p.game.phase=x.phase=now<3000?2:5;cue=camera.observe(settings,x);sound.observe(p,123,1,now,true,{},false,false,settings.reactionFadeMs);assert(cue.until==6500&&motion::strength(cue,now)>0&&sound.merge(base,now,1).reason==sustained::Reason::Result);}
        x.now=6500;assert(camera.observe(settings,x).kind==motion::Kind::None);sound.observe(p,123,1,6500,true,{},false,false,settings.reactionFadeMs);assert(sound.merge(base,6500,1).reason==sustained::Reason::None);
    }
    std::puts("PASS: fixed five-second live post-whistle settling at all play distances, no retired distance extension, synchronized return sound/motion, timeout/selection continuity and expiry.");
}
