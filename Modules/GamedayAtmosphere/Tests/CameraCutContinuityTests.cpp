#include "../Source/DirectorShake.h"
#include "../Source/ExcitementRules.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    motion::Settings s;
    // Post-play cuts and the next lineup retain a TD's original clock.
    motion::Model camera;motion::Input x;x.match=123;x.epoch=1;x.allowed=true;x.phase=4;x.now=1000;x.touchdown=1;
    auto cue=camera.observe(s,x);auto until=cue.until,anchor=cue.oscillationBegan;
    for(uint64_t now=1250;now<11000;now+=250){x.now=now;x.phase=now<2000?4:now<3000?2:now<5000?5:1;x.lineup=x.venueEligible=x.phase==1;x.lineupId=2;
        cue=camera.observe(s,x);assert(cue.kind==motion::Kind::Touchdown&&cue.until==until&&cue.oscillationBegan==anchor&&motion::strength(cue,now)>0);}
    x.now=11000;x.phase=3;x.lineup=false;cue=camera.observe(s,x);assert(cue.kind!=motion::Kind::Touchdown&&cue.kind!=motion::Kind::LivePlay); // A new snap ends the old celebration; its own lineup can settle normally.
    // A live return settles through cuts; the next lineup cannot reset/extend it.
    camera.reset();x={};x.match=123;x.epoch=1;x.allowed=true;x.phase=3;x.now=20000;x.duringPlay=1;
    camera.observe(s,x);x.now=20250;camera.observe(s,x);x.now=20500;x.phase=4;cue=camera.observe(s,x);until=cue.until;anchor=cue.oscillationBegan;
    float previous=motion::strength(cue,x.now);
    for(uint64_t now=20750;now<until;now+=250){x.now=now;x.phase=now<21500?5:1;cue=camera.observe(s,x);float level=motion::strength(cue,now);assert(cue.kind==motion::Kind::LivePlay&&cue.until==until&&cue.oscillationBegan==anchor&&level<=previous);previous=level;}
    x.now=until;assert(camera.observe(s,x).kind==motion::Kind::None);
    // An authoritative sack arriving only at the next lineup is admitted once.
    x.now=30000;x.phase=1;x.duringPlay=2;x.duringImpact=true;cue=camera.observe(s,x);assert(cue.kind==motion::Kind::LivePlay&&cue.impactOnly);
    until=cue.until;x.now=30250;assert(motion::strength(camera.observe(s,x),x.now)>8);x.now=30500;assert(camera.observe(s,x).until==until);
    // Pause/interactive replay uses suspension; do not replay the consumed hit.
    x.allowed=false;x.now=30750;assert(camera.observe(s,x).kind==motion::Kind::None);x.allowed=true;x.now=31000;assert(camera.observe(s,x).kind==motion::Kind::None);
    // A confirmed result on phase 1 really can be a defensive sack.
    energy::Tracker plays;energy::Settings rules;energy::Play p;p.game={1,0,0,1,720,720};p.offense=1;p.down=1;p.toFirst=10;p.toGoal=60;p.fieldSpot=-10;p.attackDirection=1;p.resultStamp=1;
    plays.observe(rules,p);p.game.phase=3;plays.observe(rules,p);p.game.seconds=715;p.game.phase=1;p.down=2;p.toFirst=15;p.toGoal=65;p.fieldSpot=-15;p.resultStamp=2;p.sack=true;
    auto result=plays.observe(rules,p);assert(result.completed&&result.sack&&result.side==0);
    // The native sack fact can be ready while both position sources are
    // missing. It must still create exactly one eligible impact, on either
    // supported defense and at either result phase.
    for(int phase:{4,1})for(bool neutral:{false,true})for(int offense:{0,1}){
        energy::Tracker tracker;energy::Play missing;
        missing.game={1,0,0,1,720,720};missing.game.neutral=neutral;
        missing.offense=offense;missing.down=1;missing.resultStamp=10;
        tracker.observe(rules,missing);missing.game.phase=3;tracker.observe(rules,missing);
        missing.game.phase=phase;missing.down=2;missing.resultStamp=11;missing.sack=true;
        auto hit=tracker.observe(rules,missing);
        bool supported=neutral||offense==1;
        assert(hit.completed&&hit.sack==supported&&hit.yards==-1000);
        if(supported){
            assert(hit.side==1-offense&&hit.event==energy::Event::Sack);
            motion::Model output;motion::Input impact;
            impact.match=123;impact.epoch=1;impact.allowed=true;impact.phase=phase;
            impact.now=40000;impact.duringPlay=hit.sequence;impact.duringImpact=true;
            auto shake=output.observe(s,impact);assert(shake.impactOnly&&shake.kind==motion::Kind::LivePlay);
            impact.now+=250;assert(motion::strength(output.observe(s,impact),impact.now)>0);
        }
        assert(!tracker.observe(rules,missing).completed);
    }
    // No native flag, stale flags, a positive gain, a pick or changed offense
    // must not acquire a sack just because a result/position is incomplete.
    for(int scenario=0;scenario<6;++scenario){
        energy::Tracker tracker;energy::Play bad;
        bad.game={1,0,0,1,720,720};bad.offense=1;bad.down=1;bad.resultStamp=20;
        bad.sack=scenario==1;tracker.observe(rules,bad);
        bad.game.phase=3;tracker.observe(rules,bad);bad.game.phase=4;bad.down=2;
        bad.resultStamp=scenario==1?20:21;bad.sack=scenario!=0;
        if(scenario==2){tracker.start.toGoal=60;bad.toGoal=55;}
        if(scenario==3){bad.interception=true;bad.offense=0;}
        if(scenario==4)bad.offense=0;
        if(scenario==5)bad.simulated=true;
        auto rejected=tracker.observe(rules,bad);assert(!rejected.sack);
    }
    // A released pass contradicts a stale native sack predicate. Reproduce
    // a first-down sack followed by a zero-yard second-down incompletion.
    for(bool released:{false,true})for(int phase:{4,1}){
        energy::Tracker tracker;energy::Settings pairRules;energy::Play pairPlay;
        pairPlay.game={1,0,0,1,500,720};pairPlay.game.neutral=true;pairPlay.offense=1;pairPlay.down=1;pairPlay.toFirst=10;pairPlay.fieldSpot=0;pairPlay.attackDirection=1;pairPlay.resultStamp=10;
        tracker.observe(pairRules,pairPlay);pairPlay.game.phase=3;tracker.observe(pairRules,pairPlay);
        pairPlay.game.phase=phase;pairPlay.game.seconds=495;pairPlay.down=2;pairPlay.toFirst=17;pairPlay.fieldSpot=-7;pairPlay.resultStamp=11;pairPlay.sack=true;
        auto first=tracker.observe(pairRules,pairPlay);assert(first.sack&&first.yards==-7);
        pairPlay.game.phase=1;tracker.observe(pairRules,pairPlay);pairPlay.game.phase=3;pairPlay.game.seconds=490;pairPlay.passReleased=false;tracker.observe(pairRules,pairPlay);
        pairPlay.sack=false;pairPlay.sackSampled=true;tracker.observe(pairRules,pairPlay); // Verified reset allows a genuine consecutive sack.
        pairPlay.resultStamp=12;pairPlay.passReleased=released;tracker.observe(pairRules,pairPlay);
        pairPlay.game.phase=phase;pairPlay.game.seconds=485;pairPlay.down=3;pairPlay.passReleased=false;pairPlay.sack=true;
        auto second=tracker.observe(pairRules,pairPlay);assert(second.completed&&second.sack==!released);
        if(released)assert(second.event!=energy::Event::Sack&&tracker.sawPassReleased);
    }
    std::puts("PASS: TD/live impact continuity through cuts, timeout transitions and selection, unchanged expiry/oscillation, fresh-snap and pause/replay cleanup, and once-only late-confirmed defensive sack.");
}
