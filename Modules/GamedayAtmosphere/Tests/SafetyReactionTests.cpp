#include "../Source/CoreAtmosphere.h"
#include "../Source/DirectorShake.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
energy::Outcome score(int side,bool neutral,int down=2,int quarter=2,bool loose=false){
    energy::Settings rules;energy::Tracker tracker;energy::Play p;p.game={quarter,14,14,1,300,720};p.game.neutral=neutral;p.down=down;p.offense=1-side;p.resultStamp=1;
    tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);p.game.phase=4;p.game.seconds=295;p.resultStamp=2;p.fumble=loose;
    if(side)p.game.awayScore+=2;else p.game.homeScore+=2;
    return tracker.observe(rules,p);
}
int main(){
    core::Settings sound;
    for(int side:{0,1})for(bool neutral:{false,true})for(int quarter:{2,5})for(int down:{0,1,4,-1})for(bool loose:{false,true}){
        auto result=score(side,neutral,down,quarter,loose);assert(result.accepted&&result.completed&&result.side==side);
        assert(result.safety==(down!=-1));if(result.safety)assert(!result.firstTouchdown&&!result.takeaway&&result.touchdownYards<0);
        if(!result.safety)continue;
        State s{quarter,14,14,4,295,720};s.neutral=neutral;if(side)s.awayScore+=2;else s.homeScore+=2;
        core::Model crowd;assert(!crowd.react(sound,result,1,s,true,true));
        if(side&&!neutral){assert(!crowd.resultCheer);continue;}
        assert(crowd.resultCheer&&crowd.resultType==91&&crowd.resultProfile.layered);
        energy::Outcome touchdown=result;touchdown.safety=false;touchdown.event=energy::Event::HomeScore;
        auto full=postplay::choose(touchdown,s),shorter=postplay::choose(result,s);
        assert(std::fabs(shorter.strength-full.strength)<.0001f&&std::fabs(shorter.hold/full.hold-.7)<.0001&&std::fabs(shorter.release/full.release-.7)<.0001);
        energy::Play p;p.game=s;crowd.update(sound,p,1,1,1);auto intent=crowd.intent(sound,p,123,1,1000,true);
        assert(intent.reason==sustained::Reason::Result&&intent.continuous&&intent.supporting&&intent.deepCheers&&!intent.celebration);
        assert(volume::boostPercent(crowd.parent)<=25.001f);auto once=crowd.resultCheer;crowd.react(sound,result,2,s,true,true);assert(crowd.resultCheer==once);
        core::Model gate;assert(!gate.react(sound,result,1,s,true,false)&&!gate.resultCheer);
    }
    motion::Model camera;motion::Settings settings;motion::Input x;x.match=123;x.epoch=1;x.allowed=true;x.phase=4;x.now=1000;x.safety=1;
    auto cue=camera.observe(settings,x);assert(cue.kind==motion::Kind::Safety&&cue.until==8700&&cue.percent==12&&cue.impact==16);
    auto until=cue.until;
    for(uint64_t now=1250;now<until;now+=250){x.now=now;x.phase=now<3000?4:now<5000?5:1;cue=camera.observe(settings,x);assert(cue.kind==motion::Kind::Safety&&cue.until==until&&motion::strength(cue,now)>0);}
    x.now=until;assert(camera.observe(settings,x).kind==motion::Kind::None);
    x.now=9000;assert(camera.observe(settings,x).kind==motion::Kind::None); // Same score cannot replay it.
    x.allowed=false;x.now=9250;x.safety=2;camera.observe(settings,x);x.allowed=true;x.now=9500;assert(camera.observe(settings,x).kind==motion::Kind::None);
    // Decisive safety scores still receive the existing winning eruption.
    energy::Tracker plays;energy::Settings events;energy::Play p;p.game={4,20,21,1,20,720};p.down=2;p.offense=1;p.resultStamp=1;
    plays.observe(events,p);p.game.phase=3;plays.observe(events,p);p.game.phase=4;p.game.seconds=15;p.game.homeScore=22;p.resultStamp=2;
    auto winning=plays.observe(events,p);assert(winning.safety&&winning.event==energy::Event::LastSecondLead);
    core::Model erupt;assert(erupt.react(sound,winning,10,p.game,true,true));
    x={};x.match=123;x.epoch=1;x.allowed=x.eruptionAlive=true;x.phase=4;x.now=10000;x.safety=x.eruption=1;
    camera.reset();assert(camera.observe(settings,x).kind==motion::Kind::Eruption);
    std::array<float,16> projection{};projection[0]=2;projection[5]=12;projection[11]=1;
    assert(motion::projectionScale(projection,motion::Kind::Safety)<.43f);
    settings.touchdown=7;settings.touchdownMs=6000;assert(motion::configured(settings,motion::Kind::Safety)==7&&motion::safetyDuration(settings)==4200);
    // A scoreboard correction or attach without an observed snap is not a safety event.
    energy::Tracker attached;p.game={2,16,14,4,300,720};assert(!attached.observe(events,p).safety);
    p.game.homeScore=14;assert(!plays.observe(events,p).accepted);
    std::puts("PASS: verified home/neutral safeties on normal/kick downs, PAT/two-point exclusion, no fumble-takeaway misclassification, shortened TD-family sound/motion, qualification/dedupe, camera-cut/zoom continuity, custom TD settings and decisive-eruption priority.");
}
