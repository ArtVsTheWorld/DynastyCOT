#include "../Source/SkillSwell.h"
#include "../Source/MotionSoundPairing.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    assert(live::cheerType({live::Event::Truck,1,0,0})==11);
    for(auto event:{live::Event::StiffArm,live::Event::BrokenTackle})assert(live::cheerType({event,1,0,0})==0);
    assert(live::cheerType({live::Event::Hurdle,1,0,0})==83);
    assert(live::cheerType({live::Event::HitStick,1,0,0})==59);
    // A regular broken tackle keeps its short sound but cannot start or
    // refresh motion. A separately earned long run/TD keeps its own envelope.
    assert(!live::cameraReaction(live::Event::BrokenTackle));
    for(auto event:{live::Event::Truck,live::Event::StiffArm,live::Event::Hurdle})assert(live::cameraReaction(event));
    {motion::Settings config;config.live=10;config.catchImpact=8;motion::Model model;motion::Input x;x.match=123;x.epoch=1;x.allowed=x.duringAllowed=true;x.phase=3;x.now=1000;
        x.duringPlay=live::cameraReaction(live::Event::BrokenTackle)?1:0;
        assert(model.observe(config,x).kind==motion::Kind::None);
        x.now=1100;x.duringPlay=2;auto run=model.observe(config,x);assert(run.kind==motion::Kind::LivePlay);
        x.now=1200;x.duringPlay=live::cameraReaction(live::Event::BrokenTackle)?3:0;auto kept=model.observe(config,x);
        assert(kept.began==run.began&&kept.until==run.until&&kept.reactionScale==run.reactionScale);
        x.now=1300;x.phase=4;x.touchdown=4;auto td=model.observe(config,x);assert(td.kind==motion::Kind::Touchdown);
        x.now=1400;x.duringPlay=live::cameraReaction(live::Event::BrokenTackle)?5:0;kept=model.observe(config,x);
        assert(kept.kind==motion::Kind::Touchdown&&kept.began==td.began&&kept.until==td.until);
        skillswell::Model sound;energy::Play play;play.game={2,14,14,3,300,720};play.down=1;play.offense=0;
        sound.observe(play,123,1,1000,true,{live::Event::BrokenTackle,1,0,0},1);
        sustained::Intent intent;intent.active=true;intent.match=123;intent.epoch=1;intent.updated=1200;
        sound.observe(play,123,1,1200,true,{},0);auto out=sound.merge(intent,1200,1);
        assert(out.supporting&&out.intensity>0&&!out.liveCheer&&!out.resultCheer);
    }
    // Jukes and spins use a native positive cheer without changing a separately
    // earned gain/score envelope, and do not open the gentle skill-swell route.
    for(auto event:{live::Event::Juke,live::Event::Spin}){
        assert(live::cheerOnlySkill(event)&&live::cheerType({event,1,0,0})==83&&!live::cameraReaction(event)&&!live::softSkill(event));
        motion::Settings config;config.live=10;config.catchImpact=8;motion::Model model;motion::Input x;x.match=321;x.epoch=1;x.allowed=x.duringAllowed=true;x.phase=3;x.now=1000;
        x.duringPlay=live::cameraReaction(event)?1:0;assert(model.observe(config,x).kind==motion::Kind::None);
        x.duringPlay=2;x.now=1100;auto gain=model.observe(config,x);
        x.now=1200;x.duringPlay=live::cameraReaction(event)?3:0;auto kept=model.observe(config,x);
        assert(kept.began==gain.began&&kept.until==gain.until&&kept.reactionScale==gain.reactionScale);
        skillswell::Model sound;energy::Play play;play.game={2,14,14,3,300,720};play.down=1;play.offense=0;
        sound.observe(play,321,1,1200,true,{event,3,0,0},3);
        sustained::Intent base;base.active=true;base.match=321;base.epoch=1;base.updated=1200;
        assert(!sound.merge(base,1200,1).supporting);
    }
    motion::Settings settings;settings.live=10;settings.catchImpact=8;motion::Model camera;motion::Input in;in.match=123;in.epoch=1;in.phase=3;in.now=1000;
    in.allowed=in.duringAllowed=in.duringImpact=in.softAccent=true;in.duringPlay=1;in.impactMilliseconds=2000;in.reactionScale=.5f;
    auto cue=camera.observe(settings,in);assert(cue.softAccent&&cue.impactOnly&&motion::strength(cue,1000)==0);
    in.duringPlay=0;float previous=0;
    for(uint64_t now:{1100ull,1200ull,1300ull,1450ull}){in.now=now;cue=camera.observe(settings,in);float level=motion::strength(cue,now);assert(level>=previous&&level<=5.001f);previous=level;}
    assert(std::fabs(previous-5)<.001f);in.now=2000;cue=camera.observe(settings,in);assert(motion::strength(cue,2000)>0&&motion::strength(cue,2000)<5);
    in.now=2999;cue=camera.observe(settings,in);assert(motion::strength(cue,in.now)<.01f);in.now=3000;assert(camera.observe(settings,in).kind==motion::Kind::None);
    // Sound owns its own short lifetime and does not depend on camera settings.
    skillswell::Model swell;energy::Play p;p.game={2,14,14,1,300,720};p.down=1;p.offense=0;
    p.game.phase=3;swell.observe(p,123,1,1000,true,{live::Event::Hurdle,1,0,0},1);
    sustained::Intent base;base.active=true;base.match=123;base.epoch=1;
    for(uint64_t now:{1000ull,1200ull,1450ull,1800ull,2400ull,2999ull}){
        swell.observe(p,123,1,now,true,{},0);base.updated=now;auto out=swell.merge(base,now,1);
        assert(out.supportingOnly&&out.supporting&&out.attackMs==500&&out.releaseMs==1550&&out.size==2);
        assert(!out.liveCheer&&!out.resultCheer&&!out.celebration&&out.intensity<=650);
        if(now==1450)assert(out.intensity==650);
        auto score=base;score.scoringReaction=true;score.reason=sustained::Reason::Result;score.priority=5;
        assert(!swell.merge(score,now,1).supporting);
        score=base;score.celebration=1;assert(!swell.merge(score,now,1).supporting);
    }
    swell.observe(p,123,1,3000,true,{},0);base.updated=3000;assert(!swell.merge(base,3000,1).supporting);
    swell.observe(p,123,1,3100,true,{live::Event::Spin,2,0,0},2);swell.observe(p,123,1,3200,false,{},0);base.updated=3200;assert(!swell.merge(base,3200,1).supporting);
    std::puts("PASS: gentle 450ms skill build/fade, bounded 50% live strength, independent short skill swells, successful hurdle cheer, native truck big-hit family, hit-stick separation and scoring/suspension priority.");
}
