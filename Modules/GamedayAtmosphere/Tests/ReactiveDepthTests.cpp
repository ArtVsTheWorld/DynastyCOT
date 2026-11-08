#include "../Source/LineupAtmosphere.h"
#include "../Source/DirectorShake.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
energy::Play play(){energy::Play p;p.game={2,20,20,1,400,720};p.game.rank0=6;p.game.rank1=11;p.game.variationSeed=73;p.down=1;p.offense=0;p.toGoal=65;p.toFirst=10;p.resultStamp=1;return p;}
int main(){
    core::Settings c;energy::Settings events;
    // Run the actual tracker and lineup model together. Margin is measured
    // before the snap; scoring may turn an eight-point lead into fourteen.
    for(float yards:{59.99f,60.f,65.f,100.f})for(int lead:{-9,-8,0,8,9})for(int side:{0,1})for(bool neutral:{false,true})for(float fraction:{.599f,.60f}){
        auto p=play();p.game.homeScore+=lead;p.game.neutral=neutral;p.offense=side;p.toGoal=yards;
        lineup::Model layers;energy::Tracker tracker;core::Model crowd;
        auto tick=[&](uint64_t now){layers.observe(c,p,123,1,now,true,fraction,true,nullptr,53913);auto result=tracker.observe(events,p);return crowd.react(c,result,now/1000.0,p.game,layers.layeredActive());};
        tick(1000);tick(6000);p.game.phase=3;tick(7000);p.game.phase=4;p.game.seconds-=12;p.resultStamp=2;if(side==0)p.game.homeScore+=6;else p.game.awayScore+=6;
        bool expected=yards>=60&&std::abs(lead)<=8&&(side==0||neutral)&&fraction>=.60f;
        assert(!tick(20000));assert(!tick(20050));
        assert(crowd.resultLongTouchdown==expected&&crowd.celebration==0);
        if(expected){
            crowd.update(c,p,20.25,1,1);
            auto wanted=crowd.intent(c,p,123,1,20250,true);
            assert(wanted.scoringReaction&&wanted.reason==sustained::Reason::Result&&!wanted.celebration&&wanted.size==3);
            energy::Outcome standard;standard.event=energy::Event::HomeScore;standard.preSnapMargin=std::abs(lead);
            auto profile=postplay::choose(standard,p.game);
            assert(crowd.resultProfile.strength>profile.strength&&crowd.resultProfile.strength<=1);
            assert(crowd.resultProfile.hold==profile.hold&&crowd.resultProfile.release==profile.release);
            assert(wanted.releaseMs==static_cast<int>(profile.release*1000));
            assert(volume::boostPercent(crowd.parent)<=25.01f&&!crowd.erupting);
        }
    }
    // A return is not measured from the offense's original line of scrimmage.
    auto p=play();p.offense=1;energy::Tracker returns;returns.observe(events,p);p.game.phase=3;returns.observe(events,p);p.game.phase=4;p.game.homeScore+=6;p.interception=true;p.resultStamp=2;
    auto pickSix=returns.observe(events,p);assert(pickSix.touchdownYards<0);core::Model noInventedDistance;assert(!noInventedDistance.react(c,pickSix,20,p.game,true));
    // Critical downs keep maximum reaction size and tier 3 at both camera
    // preparation and lineup. The native camera handoff remains responsible.
    for(unsigned seed=1;seed<=100;++seed)for(int down:{3,4})for(int side:{0,1}){
        p=play();p.game.phase=5;p.game.variationSeed=seed;p.down=down;p.offense=side;lineup::Model layers;
        layers.observe(c,p,123,1,1000,true,1,true,nullptr,53913);sustained::Intent base;base.active=true;base.match=123;base.epoch=1;base.updated=1000;
        auto prime=layers.merge(base,c,p,1000,1);assert(prime.cameraPrime&&prime.shakeTier==3);
        p.game.phase=1;layers.observe(c,p,123,1,2000,true,1,true,nullptr,53913);layers.observe(c,p,123,1,5000,true,1,true,nullptr,53913);base.updated=5000;
        auto loud=layers.merge(base,c,p,5000,1);assert(loud.size==3&&!loud.deepCheers&&loud.supporting&&loud.shakeTier==3);
        assert(loud.lineupCheerUntil-loud.lineupCheerBegan==1500&&layers.momentWeight()==1);
        p.game.phase=3;layers.observe(c,p,123,1,5100,true,1,true,nullptr,53913);base.updated=5100;auto snap=layers.merge(base,c,p,5100,1);assert(!snap.cheers&&!snap.shake&&snap.continuous);
    }
    // Verified big plays receive their own bounded result even without layers.
    for(auto event:{energy::Event::ExplosiveGain,energy::Event::ThirdDownConversion,energy::Event::FourthDown,energy::Event::ThirdStop,energy::Event::BackfieldStop,energy::Event::Turnover}){
        core::Model m;p=play();energy::Outcome result;result.accepted=result.completed=true;result.sequence=1;result.event=event;
        assert(!m.react(c,result,1,p.game,true)&&m.resultCheer==1);m.update(c,p,1,.8f,.8f);auto wanted=m.intent(c,p,123,1,1000,true);assert(wanted.resultType==core::resultReaction(result));assert(wanted.resultCheer==1&&!wanted.celebration&&volume::boostPercent(m.parent)<=5.01f);
        m.react(c,result,9,p.game,true);assert(m.resultCheer==1);result.sequence=2;m.react(c,result,9,p.game,true);assert(m.resultCheer==2);
        result.sequence=3;m.react(c,result,10,p.game,true);assert(m.resultCheer==2);assert(!m.intent(c,p,123,1,12000,true).resultCheer);
        core::Model quiet;quiet.react(c,result,20,p.game,false);assert(quiet.resultCheer==1);
        core::Model away;result.side=1;away.react(c,result,1,p.game,true);assert(!away.resultCheer);p.game.neutral=true;away.react(c,result,1,p.game,true);assert(away.resultCheer==1);
    }
    // Moving the lineup sound earlier must not remove long-TD eruption
    // eligibility just because the user waits for the lineup sound to settle.
    p=play();p.toGoal=65;lineup::Model settled;energy::Tracker delayedTracker;core::Model lateSnap;
    auto delayedTick=[&](uint64_t now){
        auto out=delayedTracker.observe(events,p);bool had=settled.playContextActive();
        settled.observe(c,p,300,1,now,true,.9f,true,nullptr,53913);
        return lateSnap.react(c,out,now/1000.0,p.game,had||settled.playContextActive());
    };
    delayedTick(1000);delayedTick(6000);delayedTick(35000);
    assert(!settled.layeredActive()&&settled.playContextActive());p.game.phase=3;delayedTick(36000);
    p.game.phase=4;p.game.seconds-=12;p.game.homeScore+=6;p.resultStamp=2;
    assert(!delayedTick(49000)&&lateSnap.resultLongTouchdown&&lateSnap.celebration==0);
    settled.suspend();assert(!settled.playContextActive());
    // Long TDs share the capped 20/16 camera profile with the normal TD clock and priority.
    for(float scale:{1.f,postplay::LongTouchdownScale}){
        motion::Model model;motion::Settings settings;motion::Input in;
        in.match=123;in.epoch=1;in.allowed=true;in.phase=4;in.now=1000;in.touchdown=1;in.touchdownScale=scale;
        auto cue=model.observe(settings,in);assert(cue.kind==motion::Kind::Touchdown&&cue.until==12000);
        in.now=1200;cue=model.observe(settings,in);assert(std::fabs(motion::strength(cue,in.now)-20)<.001f);
        in.now=3000;cue=model.observe(settings,in);assert(std::fabs(motion::strength(cue,in.now)-16)<.001f);
        assert(cue.began==1000&&cue.until==12000); // Repeated observations do not restart it.
        in.now=3100;in.eruption=1;in.eruptionAlive=true;cue=model.observe(settings,in);
        assert(cue.kind==motion::Kind::Eruption&&cue.percent==22.5f&&cue.reactionScale==1);
    }
    // Real decisive scores still erupt even when they also qualify as long TDs.
    auto final=play();final.game.quarter=4;final.game.seconds=20;final.game.homeScore=26;
    energy::Outcome winner;winner.accepted=winner.completed=true;winner.side=0;winner.sequence=1;winner.homePoints=6;
    winner.touchdownYards=65;winner.preSnapMargin=0;winner.event=energy::Event::LastSecondLead;
    core::Model winning;assert(winning.react(c,winner,1,final.game,true)&&winning.event==energy::Event::LastSecondLead);

    std::puts("PASS: 60-yard/pre-snap one-score boundaries, actual tracker/layer gating, away/neutral scope, no fabricated return distance, modest long-TD accent with standard timing/caps and decisive-score priority, maximum critical reactions/shake, result accents with cooldown and no extra gain.");
}
