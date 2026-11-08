#include "../Source/LineupAtmosphere.h"
#include "../Source/ReturnAtmosphere.h"
#include "../Source/PlayReactionTiming.h"
#include "../Source/DirectorShake.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    core::Settings settings;motion::Settings cameraSettings;cameraSettings.reactionFadeMs=4500; // Explicit custom sound/motion duration below.
    energy::Play p;p.game={2,14,14,1,300,720};p.down=1;p.offense=0;p.toGoal=60;p.toFirst=10;
    lineup::Model lineup;core::Model core;live::ReturnAtmosphere sound;motion::Model camera;
    const uintptr_t match=123;const unsigned epoch=1;
    lineup.observe(settings,p,match,epoch,1000,true,1,true,nullptr,53913);
    assert(lineup.playContextActive());auto minimum=lineup.qualifiedMinimum;
    p.game.phase=3;lineup.observe(settings,p,match,epoch,2000,true,1,true,nullptr,53913);
    sound.observe(p,match,epoch,2000,true,{live::Event::LongRun,1,0,60},true,false,4500,{},2000,true);
    motion::Input input;input.match=match;input.epoch=epoch;input.allowed=true;input.phase=3;input.now=2000;input.duringPlay=1;input.distanceExtraMs=2000;
    camera.observe(cameraSettings,input);
    p.game.phase=4;sound.observe(p,match,epoch,2200,true,{},false,false,4500,{},2000,true);
    input.phase=4;input.now=2200;input.duringPlay=0;auto cue=camera.observe(cameraSettings,input);const auto deadline=cue.until;
    assert(deadline==8700);
    // Timeouts, selection, changed possession and ordinary camera cuts keep
    // the earned tail's ORIGINAL deadline; none can reset its duration.
    for(uint64_t now=2400;now<deadline;now+=100){
        p.game.phase=now<4000?2:now<5000?5:1;if(now>=5000)p.offense=1;
        lineup.observe(settings,p,match,epoch,now,true,1,true,nullptr,53913);
        bool eligible=lineup::reactionEligible(p.game,1,true,lineup.playContextActive(),lineup.qualifiedMinimum);
        assert(eligible&&lineup.qualifiedMinimum<=minimum);
        sound.observe(p,match,epoch,now,eligible,{},false,false,4500,{},2000,true);
        core.update(settings,p,now*.001,1,1);
        auto wanted=core.intent(settings,p,match,epoch,now,true);
        wanted=sound.merge(wanted,now,1);
        assert(wanted.active&&wanted.reaction==(now>=3400)&&wanted.continuous==(now<3400)&&wanted.releaseMs==5300&&wanted.intensity>0);
        input.phase=p.game.phase;input.now=now;cue=camera.observe(cameraSettings,input);
        assert(cue.until==deadline&&motion::strength(cue,now)>0);
    }
    p.game.phase=1;sound.observe(p,match,epoch,deadline,true,{},false,false,4500,{},2000,true);
    auto expired=core.intent(settings,p,match,epoch,deadline,true);assert(sound.merge(expired,deadline,1).reason==sustained::Reason::None);
    input.now=deadline;assert(camera.observe(cameraSettings,input).kind==motion::Kind::None);
    // A timeout can precede the whistle's official down/result publication.
    // It must neither discard a verified sack nor invent one before a snap.
    for(bool snapped:{false,true}){
        energy::Tracker tracker;energy::Settings rules;energy::Play sack;
        sack.game={2,14,14,1,300,720};sack.down=1;sack.offense=1;sack.toGoal=60;sack.toFirst=10;sack.resultStamp=1;
        tracker.observe(rules,sack);
        if(snapped){sack.game.phase=3;tracker.observe(rules,sack);}
        sack.game.phase=2;sack.resultStamp=2;sack.sack=true;
        assert(!tracker.observe(rules,sack).completed);
        sack.game.phase=5;sack.down=2;sack.toGoal=65;sack.toFirst=15;
        auto completed=tracker.observe(rules,sack);assert(completed.sack==snapped&&completed.completed==snapped);
        assert(!tracker.observe(rules,sack).completed);
    }
    {lineup::Model unattached;p.game.phase=2;
        unattached.observe(settings,p,match,epoch,9000,true,1,true,nullptr,53913);
        assert(!unattached.playContextActive()&&!unattached.layeredActive());}
    // A real official post-play response takes priority over a run's fading
    // layer. The older tail must not pull down or restart that result.
    p.game.phase=3;p.offense=0;live::ReturnAtmosphere overlap;
    overlap.observe(p,match,epoch,10000,true,{live::Event::LongRun,2,0,25},true,false);
    p.game.phase=4;overlap.observe(p,match,epoch,10200,true,{},false,false);
    sustained::Intent result;result.active=true;result.match=match;result.epoch=epoch;
    result.reason=sustained::Reason::Result;result.priority=5;result.continuous=true;result.intensity=1000;result.releaseMs=8200;
    auto official=overlap.merge(result,10400,1);
    assert(official.continuous&&!official.reaction&&official.intensity==1000&&official.releaseMs==8200);
    // Carrier release during a tackle is NOT the whistle. A confirmed run
    // keeps both layers live, then receives the full fade after phase 4.
    {live::ReturnAtmosphere tackle;motion::Model shake;p.game.phase=3;p.offense=0;
        motion::Input hit;hit.match=match;hit.epoch=epoch;hit.allowed=hit.duringAllowed=true;hit.phase=3;hit.now=12000;
        hit.runBuildup=1;hit.runProgress=1;hit.duringPlay=1;shake.observe(cameraSettings,hit);
        tackle.observe(p,match,epoch,12000,true,{live::Event::LongRun,3,0,25},true,false,4500);
        sustained::Intent base;base.active=true;base.match=match;base.epoch=epoch;
        for(uint64_t now=12100;now<=13000;now+=100){
            hit.now=now;hit.runBuildup=hit.duringPlay=0;auto before=shake.observe(cameraSettings,hit);assert(!before.tail);
            tackle.observe(p,match,epoch,now,true,{},false,false,4500);assert(tackle.merge(base,now,1).continuous);
        }
        p.game.phase=4;hit.phase=4;hit.now=13100;auto after=shake.observe(cameraSettings,hit);assert(after.tail&&after.until==17600);
        tackle.observe(p,match,epoch,13100,true,{},false,false,4500);auto fading=tackle.merge(base,13100,1);assert(fading.continuous&&!fading.reaction&&fading.releaseMs==3375);
    }
    // A short catch may be measured for possible future YAC, but it cannot
    // start a long-play roar/hold before reaching an activation threshold.
    {timing::Distance shortCatch;live::Settings liveSettings;p.game.phase=3;
        shortCatch.observe(p,match,epoch,1,18000,true,{},0,3,true,1,true,{},-1,liveSettings,4000);
        assert(!shortCatch.ownsDuration()&&!shortCatch.extension());}
    // A delayed native play ID must never resurrect the previous play's
    // distance extension when a new live snap is already visible.
    timing::Distance distance;live::Settings liveSettings;p.game.phase=3;
    distance.observe(p,match,epoch,1,20000,true,{live::Event::LongRun,3,0,60},0,60,false,1,true,{},-1,liveSettings,4000);
    assert(distance.ownsDuration()&&distance.extension()==0);
    p.game.phase=4;distance.observe(p,match,epoch,1,20200,true,{},0,60,false,1,false,{},-1,liveSettings,4000);
    p.game.phase=1;distance.observe(p,match,epoch,1,20400,true,{},0,60,false,1,false,{},-1,liveSettings,4000);
    p.game.phase=3;distance.observe(p,match,epoch,1,20600,true,{},0,0,false,1,true,{},-1,liveSettings,4000);
    assert(!distance.ownsDuration()&&!distance.extension());
    distance.observe(p,match,epoch,1,20800,true,{},0,60,true,1,true,{},-1,liveSettings,4000);
    assert(!distance.ownsDuration()&&!distance.extension());
    distance.observe(p,match,epoch,2,21000,true,{live::Event::LongRun,4,0,25},0,25,false,1,true,{},-1,liveSettings,4000);
    assert(distance.ownsDuration()&&distance.extension()==0);
    // Custom short celebrations must retire the earlier, longer run shake;
    // finishing the celebration cannot expose that lower-priority cue again.
    for(int scoring=0;scoring<3;++scoring){
        motion::Model priority;motion::Settings custom;custom.reactionFadeMs=8000;custom.touchdownMs=3000;custom.eruptionMs=5000;
        motion::Input x;x.match=match;x.epoch=epoch;x.allowed=x.duringAllowed=true;x.phase=3;x.now=40000;x.duringPlay=1;x.distanceExtraMs=4000;
        priority.observe(custom,x);x.phase=4;x.now=40100;x.duringPlay=0;auto older=priority.observe(custom,x);assert(older.until==52100);
        x.now=40200;x.touchdown=scoring==0?1:0;x.eruption=scoring==1?1:0;x.safety=scoring==2?1:0;x.eruptionAlive=true;
        auto celebration=priority.observe(custom,x);assert(celebration.kind!=motion::Kind::LivePlay);auto end=celebration.until;assert(end<older.until);
        for(x.now+=100;x.now<end;x.now+=100)priority.observe(custom,x);
        x.now=end;assert(priority.observe(custom,x).kind==motion::Kind::None);
    }
    // Explicit cancellation must not replay the earned sound on returning
    // from replay/pause, a replaced world, a disabled feature or simulation.
    for(int cause=0;cause<5;++cause){
        live::ReturnAtmosphere cancelled;p.game.phase=3;p.simulated=false;
        cancelled.observe(p,match,epoch,30000,true,{live::Event::Interception,1,0,0},true,false);
        p.game.phase=4;cancelled.observe(p,match,epoch,30200,true,{},false,false);
        p.simulated=cause==4;
        cancelled.observe(p,cause==1?456:match,cause==2?2:epoch,30400,cause!=0&&cause!=3,{},false,false);
        sustained::Intent base;base.active=true;base.match=match;base.epoch=epoch;
        assert(cancelled.merge(base,30400,1).reason==sustained::Reason::None);
    }
    std::puts("PASS: absolute post-whistle deadlines through timeout/selection/possession/camera changes, official response priority, delayed play-ID cleanup, fresh snaps and explicit cancellations.");
}
