#include "../Source/MotionSoundPairing.h"
#include "../Source/LineupAtmosphere.h"
#include "../Source/SituationalReactions.h"
#include "../Source/ReturnAtmosphere.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
sustained::Intent base(uint64_t now){sustained::Intent out;out.active=true;out.match=123;out.epoch=1;out.updated=now;return out;}
int main(){
    motion::Settings settings;settings.live=10;settings.catchImpact=8;
    // Every short impact has one matching roar/swell hold and release, even
    // when its one-shot cheer was native, deduplicated or voice-limited.
    for(float strength:{8.f,10.f})for(float fullness:{.5f,.7f,1.f}){
        motion::Model camera;motion::Input x;x.match=123;x.epoch=1;x.allowed=true;x.phase=3;x.now=1000;x.duringPlay=1;x.duringImpact=true;x.duringCatch=strength==8;
        auto cue=camera.observe(settings,x);auto end=cue.until;sustained::Episode voice;unsigned starts=0,stops=0;uint64_t soundEnd=0;
        for(uint64_t now=1000;now<end;now+=50){x.now=now;x.duringPlay=0;x.phase=now<2000?3:now<3000?2:5;cue=camera.observe(settings,x);
            auto out=pairing::merge(base(now),cue,now,fullness,1);
            assert(out.reason==sustained::Reason::Result&&out.supporting&&out.motionCompanion&&!out.supportingOnly&&!out.liveCheer&&!out.resultCheer);
            if(voice.mustStop(out,now)){++stops;soundEnd=now+out.releaseMs;voice.stopped(now,out.releaseMs);}
            else if(voice.canStart(out,now)){++starts;voice.started(out,now);}
            if(now>1000&&motion::strength(cue,now)>0)assert(out.intensity>0);
        }
        assert(starts==1&&stops==1&&soundEnd==end);
        x.now=end;cue=camera.observe(settings,x);assert(pairing::merge(base(end),cue,end,fullness,1).reason==sustained::Reason::None);
    }
    // A camera preparation hidden behind a cutaway makes no new sound.
    motion::Cue prep{123,1,2000,1000,0,motion::Kind::Ordinary,5,false};
    assert(pairing::merge(base(2000),prep,2000,1,1,false).reason==sustained::Reason::None);
    auto prelude=pairing::merge(base(2000),prep,2000,1,1,true);assert(prelude.supportingOnly&&prelude.supporting&&prelude.continuous&&!prelude.lineupCheer);
    auto real=base(2000);real.reason=sustained::Reason::Lineup;real.continuous=true;real.lineup=99;
    auto kept=pairing::merge(real,prep,2000,1,1,true);assert(kept.lineup==99&&!kept.supportingOnly);
    // The scoring response wins, but a custom 30-second camera profile cannot
    // outlive all of its sound. Its original cue clock is never restarted.
    motion::Cue score{123,1,11000,1000,31000,motion::Kind::Touchdown,15,false,20};
    auto scoring=base(11000);scoring.reason=sustained::Reason::Result;scoring.priority=5;scoring.reaction=true;scoring.scoringReaction=true;scoring.releaseMs=9000;scoring.resultCheer=8;
    auto longer=pairing::merge(scoring,score,11000,1,1);assert(longer.continuous&&!longer.reaction&&longer.releaseMs>13000&&longer.scoringReaction&&longer.resultCheer==8);
    scoring.reason=sustained::Reason::Eruption;scoring.celebration=7;kept=pairing::merge(scoring,score,11000,1,1);assert(kept.celebration==7&&kept.releaseMs>=9000&&kept.intensity>0);
    auto stale=base(1000);assert(pairing::merge(stale,score,11000,1,1).reason==sustained::Reason::None);
    // Neutral-site opening welcomes the first offense, rather than treating
    // either half of the stadium as the away crowd. Never repeats next play.
    for(int side:{0,1}){energy::Play p;p.game={1,0,0,6,720,720};p.game.neutral=true;p.down=0;p.offense=side;p.toGoal=75;
        lineup::Model layers;core::Settings config;layers.observe(config,p,123,1,1000,true,1,true,nullptr,61888);
        p.game.phase=5;p.down=1;layers.observe(config,p,123,1,2000,true,1,true,nullptr,61888);p.game.phase=1;
        layers.observe(config,p,123,1,3000,true,1,true,nullptr,61888);auto welcome=layers.merge(base(3000),config,p,3000,1);
        assert(layers.hostileOpening&&welcome.hostileOpening&&welcome.lineupType==91&&welcome.deepCheers);
        p.game.phase=3;layers.observe(config,p,123,1,3100,true,1,true,nullptr,61888);p.game.phase=4;layers.observe(config,p,123,1,3200,true,1,true,nullptr,61888);
        p.game.phase=1;p.down=2;layers.observe(config,p,123,1,3300,true,1,true,nullptr,61888);assert(!layers.hostileOpening);
    }
    // New native kind-1 airborne records work in the situational layer too.
    for(int side:{0,1})for(bool neutral:{false,true}){situational::Model m;auto p=energy::Play{};p.game={2,7,7,1,300,720};p.game.neutral=neutral;p.down=1;p.offense=side;p.fieldSpot=-10;p.attackDirection=1;
        auto snap=p;energy::Outcome result;m.observe(p,snap,result,123,1,0,1000,true,0,9,-10,-10,true,3);
        p.game.phase=3;m.observe(p,snap,result,123,1,1,1100,true,0,2,-10,-10,true,3);
        m.observe(p,snap,result,123,1,1,1200,true,1,1,20,20,false,3);
        p.game.phase=4;result.accepted=result.completed=true;result.sequence=1;result.yards=0; // Official zero-yard incompletion proof.
        auto response=m.observe(p,snap,result,123,1,1,1300,true,3,1,20,20,false,3);assert(response.id&&response.type==(side==0&&!neutral?95:59));
    }
    // An offensive return cannot keep cheering after losing possession.
    // Only a newly observed interception/recovery gets label-handoff grace.
    for(bool neutral:{false,true}){
        energy::Play p;p.game={2,7,7,3,300,720};p.game.neutral=neutral;p.offense=0;p.down=1;
        live::ReturnAtmosphere sound;sound.observe(p,123,1,20000,true,{live::Event::LongRun,1,0,25},true,false);
        p.offense=1;sound.observe(p,123,1,20100,true,{},true,false);assert(sound.lostPossession());
        motion::Model m;motion::Input x;x.match=123;x.epoch=1;x.allowed=true;x.phase=3;x.now=20000;x.duringPlay=1;
        m.observe(settings,x);x.now=20200;m.observe(settings,x);x.cancelLive=true;x.now=20300;auto fading=m.observe(settings,x);
        assert(fading.cancelled&&fading.tail&&fading.until==20800);
        auto silence=pairing::merge(base(20300),fading,20300,1,1,false,false);assert(silence.reason==sustained::Reason::None);
        x.now=20800;assert(m.observe(settings,x).kind==motion::Kind::None);
        // A fresh forced-fumble impact remains a valid defensive cheer.
        sound.observe(p,123,1,20200,true,{live::Event::ForcedFumble,2,0,0},false,true);assert(!sound.lostPossession());
        sound.observe(p,123,1,20300,true,{live::Event::FumbleRecovery,3,0,0},false,false);assert(!sound.lostPossession());
        assert(sound.merge(base(20300),20300,1).continuous);
        p.offense=0;sound.observe(p,123,1,20400,true,{},true,false);assert(!sound.lostPossession());
        p.offense=1;sound.observe(p,123,1,20500,true,{},true,false);assert(sound.lostPossession()); // No repeated grace once possession settled.
    }
    // A pickup is sound-only and leaves the original forced-fumble impact
    // at its existing amplitude and expiry, without creating a recovery ramp.
    {motion::Model m;motion::Input x;x.match=123;x.epoch=1;x.allowed=true;x.phase=3;x.now=30000;x.duringPlay=1;x.duringImpact=true;
        auto initial=m.observe(settings,x);x.now=30200;auto impact=m.observe(settings,x);assert(motion::strength(impact,x.now)==10);
        x.now=30300;x.duringPlay=0;auto pickup=m.observe(settings,x);assert(pickup.began==initial.began&&pickup.until==initial.until);
        x.now=30700;x.phase=4;pickup=m.observe(settings,x);assert(pickup.impactOnly&&pickup.until==initial.until);
        x.now=initial.until;assert(m.observe(settings,x).kind==motion::Kind::None);
    }
    // Every automatic camera category shares the same close/wide correction.
    std::array<float,16> projection{};projection[0]=2;projection[11]=1;
    for(auto kind:{motion::Kind::Ordinary,motion::Kind::Critical,motion::Kind::LivePlay,motion::Kind::Touchdown,motion::Kind::Eruption,motion::Kind::Safety,motion::Kind::BlockedKick}){
        projection[5]=3.1715948f;assert(motion::projectionScale(projection,kind)==1);
        projection[5]=12;assert(motion::projectionScale(projection,kind)>.264f&&motion::projectionScale(projection,kind)<.265f);
        projection[5]=1.5857974f;assert(motion::projectionScale(projection,kind)==2);assert(motion::projectionScale(projection,kind,true)==2);
        projection[5]=.11f;assert(motion::projectionScale(projection,kind)==4);assert(motion::projectionScale(projection,kind,true)==6);
        projection[5]=NAN;assert(motion::projectionScale(projection,kind)==0);
    }
    std::puts("PASS: complete sound/motion envelopes, visible-only prelude, scoring priority, stale guards, neutral opening/pass counterparts, lost-possession cancellation, recovery handoff and every-category close/wide compensation.");
}
