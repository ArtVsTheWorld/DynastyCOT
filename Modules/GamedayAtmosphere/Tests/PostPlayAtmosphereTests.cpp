#include "../Source/LineupAtmosphere.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
energy::Play play(){energy::Play p;p.game={2,17,17,4,400,720};p.down=1;p.offense=0;p.toGoal=50;p.toFirst=10;return p;}
int main(){
    core::Settings c;
    for(auto event:{energy::Event::ExplosiveGain,energy::Event::ThirdDownConversion,energy::Event::FourthDown,energy::Event::BackfieldStop,energy::Event::ThirdStop,energy::Event::FourthStop,energy::Event::Turnover,energy::Event::HomeScore})
    for(int margin:{0,7,14,21,35})for(float fullness:{.1f,.25f,.5f,.75f,.9f,1.f}){
        auto p=play();p.game.homeScore+=margin;energy::Outcome r;r.accepted=r.completed=true;r.sequence=1;r.event=event;r.yards=25;
        core::Model m;if(margin>=24){assert(!m.react(c,r,10,p.game,false)&&!m.resultCheer);m.update(c,p,10,fullness,fullness);assert(m.resultWeight==0);continue;}assert(!m.react(c,r,10,p.game,false)&&m.resultCheer==1);m.update(c,p,10,fullness,fullness);
        auto out=m.intent(c,p,123,1,10000,true);
        if(fullness<core::ReactionMinimumFullness){assert(m.resultWeight==0&&out.reason==sustained::Reason::None&&out.intensity==0);continue;}
        assert(out.reason==sustained::Reason::Result&&out.intensity>0&&out.intensity<=1000&&out.continuous&&!out.shake&&!out.cheers);
        assert(!m.erupting&&!out.celebration&&volume::boostPercent(m.parent)<=25.001f);
        auto original=m.resultCheer;for(int n=0;n<100;++n)m.react(c,r,10.1,p.game,true);assert(m.resultCheer==original);
        double fadeAt=10+m.resultProfile.hold+1;
        m.update(c,p,fadeAt,fullness,fullness);auto fading=m.intent(c,p,123,1,static_cast<uint64_t>(fadeAt*1000),true);
        assert(fading.reason==sustained::Reason::Result&&fading.reaction&&!fading.continuous&&!fading.supporting&&fading.intensity<out.intensity&&!fading.resultCheer);
        assert(!m.intent(c,p,123,1,static_cast<uint64_t>(fadeAt*1000),false).active);
        m.update(c,p,30,fullness,fullness);assert(m.intent(c,p,123,1,30000,true).reason==sustained::Reason::None);
    }
    auto p=play();energy::Outcome r;r.accepted=r.completed=true;r.sequence=1;r.event=energy::Event::Turnover;
    core::Model home;r.side=1;home.react(c,r,1,p.game);assert(!home.resultCheer);p.game.neutral=true;home.react(c,r,1,p.game);assert(home.resultCheer==1);
    p.game.neutral=false;r.side=0;r.event=energy::Event::None;r.firstDown=true;core::Model routine;
    routine.react(c,r,1,p.game,false);assert(!routine.resultCheer);routine.react(c,r,1,p.game,true);assert(routine.resultCheer==1);
    c.lineups=false;core::Model off;off.react(c,r,1,p.game,true);assert(!off.resultCheer);c.lineups=true;
    core::Model snap;r.event=energy::Event::ExplosiveGain;snap.react(c,r,1,p.game);snap.update(c,p,1,1,1);assert(snap.resultWeight>0);
    p.game.phase=3;snap.update(c,p,2,1,1);assert(!snap.resultWeight&&!snap.intent(c,p,123,1,2000,true).resultCheer);
    p=play();r.event=energy::Event::Turnover;core::Model cancel;cancel.react(c,r,1,p.game);r.setback=true;r.sequence=2;r.event=energy::Event::None;
    cancel.react(c,r,2,p.game);cancel.update(c,p,2,1,1);assert(!cancel.resultWeight);
    // A result wins over anticipation, with no newly armed shake or scheduled
    // lineup cheer, and never replaces a higher-priority winning eruption.
    p=play();p.game.phase=1;p.game.rivalry=true;lineup::Model layers;layers.observe(c,p,123,1,1000,true,1,true,nullptr,26101);layers.observe(c,p,123,1,5000,true,1,true,nullptr,26101);
    r={};r.accepted=r.completed=true;r.sequence=1;r.event=energy::Event::FourthStop;core::Model result;
    p.game.phase=4;result.react(c,r,6,p.game);result.update(c,p,6,1,1);
    auto before=result.intent(c,p,123,1,6000,true),after=layers.merge(before,c,p,6000,1);
    assert(after.reason==sustained::Reason::Result&&after.intensity>=before.intensity&&!after.shake&&!after.lineupCheer);
    p.game.phase=5;layers.observe(c,p,123,1,6100,true,1,true,nullptr,26101);before.updated=6100;after=layers.merge(before,c,p,6100,1);
    assert(after.reason==sustained::Reason::Result&&after.cameraPrime&&after.shake&&!after.lineupCheer); // The next camera handoff still arms while the previous cheer settles.
    // A fresh native stamp plus updated field/down proof can confirm at the
    // whistle. Old post-play values and replay rewinds cannot invent a cheer.
    for(bool fresh:{false,true})for(bool updated:{false,true}){
        energy::Tracker tracker;energy::Settings settings;p=play();p.game.phase=1;p.toGoal=70;p.resultStamp=100;
        tracker.observe(settings,p);p.game.phase=3;tracker.observe(settings,p);p.game.phase=4;p.game.seconds-=6;
        if(fresh)p.resultStamp=101;if(updated){p.toGoal=50;p.firstDown=true;}
        auto outcome=tracker.observe(settings,p);assert(outcome.completed==(fresh&&updated));
        if(fresh&&updated){assert(outcome.event==energy::Event::ExplosiveGain&&outcome.yards>=20);assert(!tracker.observe(settings,p).completed);}
    }
    p=play();p.game.phase=1;p.resultStamp=1;energy::Tracker tracker;energy::Settings settings;
    tracker.observe(settings,p);p.game.phase=3;tracker.observe(settings,p);p.game.phase=4;p.game.seconds-=6;p.resultStamp=2;p.firstDown=true;
    assert(!tracker.observe(settings,p).completed); // A sticky first-down flag alone is not fresh field-position proof.
    tracker.reset();p=play();p.game.phase=1;p.resultStamp=1;
    tracker.observe(settings,p);p.game.phase=3;tracker.observe(settings,p);p.game.phase=4;p.game.seconds+=20;p.resultStamp=2;p.down=2;
    assert(!tracker.observe(settings,p).accepted);
    std::puts("PASS: context-sensitive verified post-play roars/cheers, finite hold/fade, native-volume caps, ordinary games, home/neutral ownership, priority, fresh-whistle proof, duplicate/rewind rejection and snap/disable cleanup.");
}
