#include "../Source/ReturnAtmosphere.h"
#include "../Source/DirectorShake.h"
#include "../Source/CoreAtmosphere.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    // Keep the same total time, with one brief hold and no rebound.
    for(unsigned total:{1000u,4500u,8500u,16000u}){
        float previous=1;
        for(unsigned age=0;age<=total;++age){
            float value=shape::tail(static_cast<float>(age),static_cast<float>(total),static_cast<float>(shape::tailHold(total)));
            assert(std::isfinite(value)&&value>=0&&value<=previous+.00001f);previous=value;
        }
        assert(shape::tail(100.f,static_cast<float>(total),static_cast<float>(shape::tailHold(total)))==1);
        assert(previous==0&&shape::tail(-1.f,static_cast<float>(total),0)==0);
    }
    for(auto event:{energy::Event::ExplosiveGain,energy::Event::ThirdDownConversion,energy::Event::FourthStop,energy::Event::Sack,energy::Event::MadeFieldGoal,energy::Event::Turnover,energy::Event::HomeScore,energy::Event::Safety}){
        energy::Outcome result;result.event=event;result.yards=44;
        State state{2,14,14,4,80,720};auto profile=postplay::choose(result,state);
        assert(postplay::envelope(0,profile)==1);
        float previous=1;
        for(double age=.01;age<profile.hold+profile.release+1;age+=.01){float value=postplay::envelope(age,profile);assert(value>=0&&value<=previous+.00001f);previous=value;}
        assert(previous==0&&postplay::envelope(.1,profile)>postplay::envelope(2,profile));
    }
    assert(live::cheerType({live::Event::LongCatch,1,0,44})==91);
    assert(live::cheerType({live::Event::LongCatch,1,0,20})==83);
    assert(live::cheerType({live::Event::Interception,1,0,50})==59);
    // Reproduce the clip: a 44-yard catch, brief YAC, tackle and live camera
    // changes. Both channels hold at the whistle and end on one deadline.
    for(auto event:{live::Event::LongCatch,live::Event::LongRun,live::Event::RunAfterCatch,live::Event::Interception,live::Event::FumbleRecovery,live::Event::KickReturn,live::Event::PuntReturn}){
        energy::Play p;p.game={1,14,7,3,406,720};p.offense=0;
        live::ReturnAtmosphere sound;motion::Model motion;motion::Settings settings;settings.live=10;settings.catchImpact=8;settings.reactionFadeMs=4500; // Explicit custom timing exercises the envelope independently of defaults.
        motion::Input x;x.match=123;x.epoch=1;x.allowed=x.duringAllowed=true;x.phase=3;x.now=1000;x.duringPlay=1;x.distanceExtraMs=1265;
        auto cue=motion.observe(settings,x);
        sound.observe(p,123,1,1000,true,{event,1,0,44},true,false,4500,{},1265,true);
        sustained::Intent base;base.active=true;base.match=123;base.epoch=1;
        auto hit=sound.merge(base,1000,1);assert(hit.continuous&&hit.supporting&&hit.attackMs==500);
        for(uint64_t now=1100;now<1400;now+=100){x.now=now;x.duringPlay=0;motion.observe(settings,x);sound.observe(p,123,1,now,true,{},true,false,4500,{1,1,45},1265,true);}
        const uint64_t whistle=1400,deadline=7165;
        for(uint64_t now=whistle;now<deadline;now+=50){
            p.game.phase=now<2400?4:now<3400?2:5;x.phase=p.game.phase;x.now=now;cue=motion.observe(settings,x);
            sound.observe(p,123,1,now,true,{},false,false,4500,{},1265,true);
            auto out=sound.merge(base,now,1);assert(cue.until==deadline&&out.intensity>0);
            assert(std::fabs(out.intensity/950-motion::strength(cue,now)/10)<.0001f);
            bool held=now-whistle<shape::tailHold(5765);
            assert(out.continuous==held&&out.reaction==!held&&out.supporting==held&&out.releaseMs==4565);
            base.celebration=1;assert(sound.merge(base,now,1).reason==sustained::Reason::None);base.celebration=0;
        }
        sound.observe(p,123,1,deadline,true,{},false,false,4500,{},1265,true);assert(sound.merge(base,deadline,1).reason==sustained::Reason::None);
        x.now=deadline;assert(motion.observe(settings,x).kind==motion::Kind::None);
    }
    // Immediate impacts keep their initial peak while the cheer starts.
    motion::Cue impact{123,1,1000,1000,6900,motion::Kind::LivePlay,8,false,0,1000,true};
    impact.updated=1200;assert(motion::strength(impact,1200)==8);impact.updated=1800;assert(motion::strength(impact,1800)==8);
    impact.updated=6000;assert(motion::strength(impact,6000)<1);
    std::puts("PASS: bounded attack/hold/ease-out shapes, explosive cheer choice, seven live-play sound/motion handoffs, unchanged deadlines and celebration priority.");
}
