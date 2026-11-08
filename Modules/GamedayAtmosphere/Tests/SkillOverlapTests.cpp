#include "../Source/DefensiveAccents.h"
#include "../Source/SkillSwell.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    assert(defense::shakeProfile(live::Event::Truck)==motion::Profile::Truck);
    assert(!live::softSkill(live::Event::Truck));
    for(bool neutral:{false,true})for(auto event:{live::Event::Truck,live::Event::Hurdle,live::Event::StiffArm}){
        motion::Model m;motion::Settings s;motion::Input x;x.match=123;x.epoch=1;x.allowed=x.duringAllowed=true;x.phase=3;x.neutral=neutral;
        x.runBuildup=7;x.runProgress=.02f;x.smoothRunBuildup=true;x.now=1000;m.observe(s,x);
        x.now=1100;auto run=m.observe(s,x);float before=motion::strength(run,1200);
        x.now=1200;x.duringPlay=1;x.supplementalImpact=x.duringImpact=true;x.profile=defense::shakeProfile(event);x.softAccent=live::softSkill(event);
        auto cue=m.observe(s,x);assert(motion::strength(cue,x.now)>=before&&motion::strength(cue,x.now)<=before*1.15f+.01f);
        const auto profile=x.profile;const auto peak=motion::profileStrength(profile,0)*(neutral?.75f:1.f);
        const auto reached=event==live::Event::Truck?1300ull:1650ull;
        // One-shot proof disappears on the next update. The accent must keep
        // its original identity while the independent distance ramp survives.
        x.duringPlay=0;x.profile=motion::Profile::Default;x.softAccent=x.duringImpact=x.supplementalImpact=false;
        for(x.now=1250;x.now<=reached;x.now+=50)cue=m.observe(s,x);
        assert(cue.profile==profile&&cue.impactOnly&&cue.began==1200&&cue.until==3200);
        assert(motion::strength(cue,reached)>=peak&&motion::strength(cue,reached)<=peak*1.15f+.01f);
        for(x.now=reached+50;x.now<=3250;x.now+=50){cue=m.observe(s,x);assert(motion::strength(cue,x.now)<=peak*1.15f+.01f);}
        assert(cue.runBuildup&&cue.began==1000&&!cue.confirmedRun);
        x.runProgress=1;
        for(x.now=3300;x.now<=4300;x.now+=50)cue=m.observe(s,x);
        assert(cue.runBuildup&&cue.began==1000&&cue.percent>7);
    }
    for(int cancel=0;cancel<5;++cancel){
        motion::Model m;motion::Settings s;motion::Input x;x.match=1;x.allowed=x.duringAllowed=true;x.phase=3;x.now=1000;x.duringPlay=1;
        x.duringImpact=x.supplementalImpact=true;x.profile=motion::Profile::Truck;m.observe(s,x);
        x.now=1100;x.duringPlay=0;assert(motion::strength(m.observe(s,x),1100)==7.5f);
        if(cancel==0)x.allowed=false;if(cancel==1)x.duringAllowed=false;if(cancel==2)x.cancelLive=true;
        if(cancel==3){x.phase=4;x.touchdown=1;}if(cancel==4){x.match=2;x.duringPlay=0;}
        x.now=1200;auto cue=m.observe(s,x);assert(cue.kind==motion::Kind::None||cue.kind==motion::Kind::Touchdown);
        x.allowed=x.duringAllowed=true;x.cancelLive=false;x.touchdown=0;
        for(x.now=1300;x.now<=4000;x.now+=100){cue=m.observe(s,x);assert(cue.profile!=motion::Profile::Truck);}
    }
    // A larger takeaway stays intact; a weaker skill cannot extend it or
    // erase its deadline. The skill remains on its own original clock.
    {motion::Model m;motion::Settings s;motion::Input x;x.match=1;x.allowed=x.duringAllowed=true;x.phase=3;x.now=1000;x.duringPlay=1;
        x.duringImpact=true;x.profile=motion::Profile::Force;x.forcedImpact=true;m.observe(s,x);
        x.now=1200;auto first=m.observe(s,x);x.now=1300;x.duringPlay=2;x.profile=motion::Profile::Truck;x.forcedImpact=false;x.supplementalImpact=true;
        auto next=m.observe(s,x);assert(next.profile==first.profile&&next.began==first.began&&next.until==first.until);
        x.duringPlay=0;for(x.now=1400;x.now<=2000;x.now+=50){next=m.observe(s,x);assert(next.profile==motion::Profile::Force&&motion::strength(next,x.now)<=13*1.15f);}
    }
    // A hurdle swell must not replace a playing return's ID, hold/release,
    // cheer, priority or voice type. It can only lift its existing intensity.
    {skillswell::Model swell;energy::Play p;p.game={2,14,14,3,300,720};p.down=1;p.offense=0;
        swell.observe(p,123,1,1000,true,{live::Event::Hurdle,1,0,0},8);
        sustained::Intent base;base.active=true;base.match=123;base.epoch=1;base.reason=sustained::Reason::Result;base.priority=4;
        base.motionCompanion=true;base.continuous=true;base.intensity=100;base.reactionId=77;base.reactionUntil=9000;base.reactionHoldUntil=5000;
        base.releaseMs=5000;base.size=3;base.liveCheer=77;
        for(uint64_t now=1000;now<3000;now+=50){swell.observe(p,123,1,now,true,{},0);base.updated=now;auto out=swell.merge(base,now,1);
            auto empty=base;empty.continuous=false;empty.intensity=0;auto silent=swell.merge(empty,now,1,false);assert(silent.intensity==0&&!silent.supporting);
            auto blocked=swell.merge(base,now,1,false);assert(blocked.intensity==base.intensity&&blocked.reactionId==base.reactionId);
            assert(out.reactionId==77&&out.reactionUntil==9000&&out.reactionHoldUntil==5000&&out.releaseMs==5000&&out.size==3);
            assert(out.continuous&&!out.reaction&&!out.supportingOnly&&out.liveCheer==77&&out.priority==4&&out.intensity>=100);
        }
    }
    std::puts("PASS: quick truck onset; independent hurdle/stiff-arm accents over weak run ramps; stronger-envelope selection; retained ramp/deadlines; cancellation/score priority; no audio hold/identity replacement.");
}
