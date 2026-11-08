#include "../Source/FinalReactionCarry.h"
#include "../Source/CoreAtmosphere.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    State s{4,28,21,4,0,720};sustained::Intent sound;sound.match=123;sound.epoch=1;sound.active=sound.continuous=sound.earnedReaction=true;
    sound.reason=sustained::Reason::Result;sound.intensity=900;sound.reactionId=2;sound.liveCheer=4;sound.resultCheer=3;sound.cheers=sound.deepCheers=true;
    sound.reactionHoldUntil=7000;sound.reactionUntil=12000;
    motion::Cue cue{123,1,2000,1000,12000,motion::Kind::Touchdown,15,false,20,1000};
    for(auto kind:{motion::Kind::Touchdown,motion::Kind::Safety,motion::Kind::BlockedKick,motion::Kind::Clincher,motion::Kind::LivePlay}){
        finalcarry::Model m;cue.kind=kind;cue.tail=kind==motion::Kind::LivePlay;cue.began=1000;cue.until=cue.tail?6000:12000;cue.updated=2000;
        sound.updated=2000;s.phase=4;m.observe(s,123,1,2,2000,true,true,sound,cue);
        auto original=cue;auto soundInput=sound;soundInput.reason=sustained::Reason::None;soundInput.earnedReaction=false;soundInput.intensity=0;
        s.phase=7;float last=900;
        for(uint64_t now=2050;now<12000;now+=50){auto out=m.observe(s,123,1,2,now,true,true,soundInput,{},5000);
            if(now<7050){assert(out.carrying&&out.sound.intensity<=last+.001f&&out.sound.intensity>0);last=out.sound.intensity;
                assert(!out.sound.liveCheer&&!out.sound.resultCheer&&!out.sound.lineupCheer&&!out.sound.cheers&&!out.sound.deepCheers);}
            if(now<cue.until){original.updated=now;assert(out.camera.began==cue.began&&out.camera.until==cue.until&&out.camera.oscillationBegan==cue.oscillationBegan);
                assert(std::abs(motion::strength(out.camera,now)-motion::strength(original,now))<.0001f);}
        }
        assert(!m.observe(s,123,1,2,12000,true,true,soundInput,{}).carrying);
    }
    // A carrier still live on the final marker gets one bounded tail, preserving phase and initial level.
    finalcarry::Model m;cue.kind=motion::Kind::LivePlay;cue.tail=false;cue.began=1000;cue.until=46000;cue.updated=2000;s.phase=3;sound.updated=2000;
    m.observe(s,123,1,2,2000,true,true,sound,cue);s.phase=7;
    auto out=m.observe(s,123,1,2,2050,true,true,{},{});assert(out.camera.tail&&out.camera.until==7050&&out.camera.oscillationBegan==1000);
    assert(std::abs(motion::strength(out.camera,2050)-motion::strength(cue,2050))<.001f);
    m.suspend();assert(!m.observe(s,123,1,2,2100,true,true,{},{}).carrying);
    // New play/owner/epoch, an interruption, stale input or mere lineup/unconfirmed buildup cannot be carried.
    for(int rejection=0;rejection<9;++rejection){m.reset();s.phase=4;sound.updated=cue.updated=3000;cue.tail=true;cue.began=2000;cue.until=10000;cue.runBuildup=false;sound.earnedReaction=true;
        if(rejection==0)cue.kind=motion::Kind::Ordinary;else cue.kind=motion::Kind::LivePlay;
        if(rejection==1){cue.runBuildup=true;cue.confirmedRun=false;}
        if(rejection==2){sound.updated=cue.updated=1000;}
        if(rejection<=2)sound.earnedReaction=false;
        m.observe(s,123,1,2,3000,true,true,sound,cue);
        if(rejection==3)m.suspend();s.phase=7;
        auto owner=rejection==4?124:123;unsigned epoch=rejection==5?2:1;uint64_t sequence=rejection==6?3:2;
        out=m.observe(s,owner,epoch,sequence,rejection==7?4000:3050,rejection!=8,true,{},{});
        assert(!out.carrying);
    }
    // Eruptions take precedence and cannot uncover an old weaker tail later.
    m.reset();s.phase=4;sound.updated=cue.updated=5000;cue.runBuildup=false;sound.earnedReaction=true;m.observe(s,123,1,2,5000,true,true,sound,cue);
    s.phase=7;auto eruption=sound;eruption.reason=sustained::Reason::Eruption;eruption.celebration=5;eruption.updated=5050;
    assert(m.observe(s,123,1,2,5050,true,true,eruption,{}).sound.celebration==5);
    assert(!m.observe(s,123,1,2,5100,true,true,{},{}).carrying);
    // Scoring audio retains its remaining original lifetime, never more than 11 seconds after final.
    m.reset();s.phase=4;sound.scoringReaction=true;sound.reactionUntil=50000;sound.reactionHoldUntil=40000;sound.updated=10000;
    m.observe(s,123,1,2,10000,true,false,sound,{});s.phase=7;
    assert(m.observe(s,123,1,2,10050,true,false,{},{}).carrying);
    assert(!m.observe(s,123,1,2,21050,true,false,{},{}).carrying);
    // Custom long scoring holds are capped without resetting the camera phase.
    m.reset();s.phase=4;cue.kind=motion::Kind::Touchdown;cue.tail=false;cue.began=9000;cue.updated=10000;cue.until=50000;cue.oscillationBegan=9000;
    m.observe(s,123,1,2,10000,true,true,{},cue);s.phase=7;
    out=m.observe(s,123,1,2,10050,true,true,{},{});assert(out.camera.until==50000&&out.camera.oscillationBegan==9000);
    assert(m.observe(s,123,1,2,20050,true,true,{},{}).carrying);
    assert(!m.observe(s,123,1,2,21050,true,true,{},{}).carrying);
    core::Model earlyMenu;core::Settings config;energy::Play final;final.game={4,28,21,7,0,720};final.game.rank0=60;final.game.rank1=3;
    earlyMenu.sawLive=true;assert(!earlyMenu.finishPostgame()&&earlyMenu.postgameMenu&&earlyMenu.postgameSeen);
    earlyMenu.update(config,final,30,1,1,0,true,.5f);assert(!earlyMenu.erupting&&!earlyMenu.intent(config,final,123,1,30000,true).active);
    std::puts("PASS: final-play sound/motion continuity, original clocks/phase, bounded tails, no repeated cheers, menu/interruption/ownership cleanup and eruption priority.");
}
