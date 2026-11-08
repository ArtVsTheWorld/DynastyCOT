#include "../Source/SoundOnlyReactions.h"
#include "../Source/LineupAtmosphere.h"
#include "../Source/LiveReactions.h"
#include "../Source/DirectorShake.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
energy::Play wake(){energy::Play p;p.game={2,3,0,1,574,720};p.game.rank0=96;p.game.rank1=17;p.down=1;p.offense=0;p.selectedType=3;p.toGoal=60;p.toFirst=10;return p;}
energy::Outcome outcome(energy::Event event,uint64_t sequence=1){energy::Outcome r;r.accepted=r.completed=true;r.sequence=sequence;r.side=0;r.event=event;return r;}
int main(){
    core::Settings config;energy::Play p=wake();auto snap=p;
    auto upset=upsetsound::evaluate(p.game,18829,.521f,true);assert(!upset.qualified&&upset.minimum>.62f);
    lineup::Model layers;layers.observe(config,p,123,1,1000,true,.521f,true,nullptr,18829);
    assert(!layers.playContextActive()&&!lineup::cameraAdmission(config,p,.521f,true,18829,3).allowed);
    assert(soundonly::eligible(config,p,.521f,true));
    for(float fullness:{.01f,.20f,.49f,.521f,.90f,1.f})assert(soundonly::eligible(config,p,fullness,true));
    for(float fullness:{0.f,-.1f,1.1f,NAN,INFINITY})assert(!soundonly::eligible(config,p,fullness,true));
    assert(!soundonly::eligible(config,p,.521f,false));
    config.enabled=false;assert(!soundonly::eligible(config,p,.521f,true));config.enabled=true;
    config.lineups=false;assert(!soundonly::eligible(config,p,.521f,true));config.lineups=true;
    config.maximumPercent=0;assert(!soundonly::eligible(config,p,.521f,true));config.maximumPercent=25;
    p.simulated=true;assert(!soundonly::eligible(config,p,.521f,true));p.simulated=false;
    p.game.phase=6;assert(!soundonly::eligible(config,p,.521f,true));p.game.phase=4;
    // Native result families stay simple: one cheer, no escalation or roar.
    for(auto event:{energy::Event::HomeScore,energy::Event::ExplosiveGain,energy::Event::ThirdDownConversion,energy::Event::FourthDown,
        energy::Event::ThirdStop,energy::Event::FourthStop,energy::Event::Sack,energy::Event::ForcedFumble,
        energy::Event::Turnover,energy::Event::FumbleRecovery,energy::Event::MadeFieldGoal}){
        soundonly::Model model;auto r=outcome(event);auto sound=model.observe(r,snap,p,123,1,true,false,false);
        assert(sound.sequence==1&&sound.type==core::resultReaction(r));
        assert(!model.observe(r,snap,p,123,1,true,false,false).sequence);
        core::Model full;full.react(config,r,10,p.game,false,false);full.update(config,p,10,.521f,.521f,0,false);
        assert(!full.erupting&&!full.resultCheer&&full.resultWeight==0);
    }
    // Sound-only touchdowns do not need the event to be an ordinary TD;
    // late/opening winning names still get only one normal native cheer.
    for(auto event:{energy::Event::EarlyTouchdown,energy::Event::LastSecondLead,energy::Event::GameWinningScore}){
        soundonly::Model model;auto r=outcome(event);r.homePoints=6;
        assert(model.observe(r,snap,p,123,1,true,false,false).type==91);
    }
    // Away scores/takeaways request disappointment, never positive reactions.
    for(bool score:{false,true})for(bool neutral:{false,true}){
        soundonly::Model model;p.game.neutral=neutral;auto r=outcome(score?energy::Event::HomeScore:energy::Event::Turnover);r.side=1;
        r.takeaway=!score;r.visitorScore=score;r.awayPoints=score?6:0;
        auto sound=model.observe(r,snap,p,123,1,true,false,false);
        assert(sound.type==(neutral?core::resultReaction(r):95));
        model.reset();assert(bool(model.observe(r,snap,p,123,1,true,false,false,false).sequence)==neutral);
    }p.game.neutral=false;
    // Ordinary short plays cannot invent a cheer; a native first down can.
    {soundonly::Model model;auto r=outcome(energy::Event::None);r.yards=2;
        assert(!model.observe(r,snap,p,123,1,true,false,false).sequence);r.sequence=2;r.firstDown=true;
        assert(model.observe(r,snap,p,123,1,true,false,false).type==83);
    }
    for(int type:{37,38}){auto knee=snap;knee.selectedType=type;soundonly::Model model;
        assert(!soundonly::eligible(config,knee,.521f,true));
        assert(!model.observe(outcome(energy::Event::Sack),knee,p,123,1,true,false,false).sequence);
    }
    // Layered/disabled/already-covered opportunities are consumed immediately.
    for(int unavailable:{0,1,2}){soundonly::Model model;auto r=outcome(energy::Event::ExplosiveGain);
        assert(!model.observe(r,snap,p,123,1,unavailable!=0,unavailable==1,unavailable==2).sequence);
        assert(!model.observe(r,snap,p,123,1,true,false,false).sequence);
        r.homePoints=6;r.event=energy::Event::HomeScore;
        assert(model.observe(r,snap,p,123,1,true,false,true).type==91);
        assert(!model.observe(r,snap,p,123,1,true,false,true).sequence);
    }
    // Supporting skill/pass swells are bounded; passive layers and secondary
    // cheers disappear, while a one-shot cheer ID retains native deduplication.
    for(float fullness:{.01f,.2f,.521f,1.f})for(int kind:{0,1,2}){
        sustained::Intent intent;intent.active=true;intent.match=123;intent.epoch=1;intent.updated=1000;
        intent.liveCheer=42;intent.liveType=83;intent.liveCheerUntil=1500;intent.intensity=1000;intent.size=3;
        intent.reason=kind==1?sustained::Reason::PassFlight:sustained::Reason::Result;
        intent.passFlight=kind==1?1:0;intent.skillAccent=kind==2?1:0;
        intent.supporting=intent.continuous=intent.staggered=intent.deepCheers=intent.majorLive=true;
        intent.celebration=intent.lineup=intent.lineupCheer=intent.resultCheer=1;intent.layerBoostPercent=50;intent.shake=true;intent.shakeTier=3;
        auto out=soundonly::restrain(intent,fullness);
        assert(out.liveCheer==42&&out.liveType==83&&out.liveCheerUntil==1500);
        assert(!out.shake&&!out.shakeTier&&!out.layerBoostPercent&&!out.celebration&&!out.lineup&&!out.resultCheer&&!out.deepCheers&&!out.staggered&&!out.majorLive);
        assert(out.intensity<=450*std::sqrt(fullness)&&out.size<=2);
        if(!kind)assert(!out.intensity&&!out.continuous&&!out.supporting&&out.reason==sustained::Reason::None);
        else assert(out.supportingOnly);
    }
    // Discovering a live long gain below the gate cannot create camera motion.
    {motion::Model camera;motion::Settings settings;motion::Input x;x.match=123;x.epoch=1;x.phase=3;x.now=1000;
        bool layered=lineup::reactionEligible(p.game,.521f,true,false);
        x.allowed=x.duringAllowed=layered;x.duringPlay=layered?1:0;x.runBuildup=layered?1:0;x.runProgress=1;
        assert(camera.observe(settings,x).kind==motion::Kind::None);
    }
    assert(core::normalLimit(config)==core::NormalMaximumDb&&config.highLeveragePercent==35&&config.eruptionPercent==50);
    std::puts("PASS: Wake/Virginia 52.1% sound-only route, verified results/home-negative/neutral ownership, modest single voices, capped pass/skill swells, no layered roar/motion/bonuses, no deferred duplicate or knee/sim/unknown reactions, unchanged volume limits.");
}
