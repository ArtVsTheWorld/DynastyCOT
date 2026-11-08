#include "../Source/LiveReactions.h"
#include "../Source/LiveFumble.h"
#include "../Source/CoreAtmosphere.h"
#include "../Source/DirectorShake.h"
#include "../Source/DefensiveMotion.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    energy::Settings rules;
    for(int phase:{1,4,5})for(int offense:{0,1})for(bool neutral:{false,true}){
        energy::Tracker tracker;energy::Play p;p.game={2,7,7,1,300,720};p.game.neutral=neutral;
        p.down=1;p.offense=offense;p.toGoal=60;p.toFirst=10;p.resultStamp=10;
        tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);
        p.game.phase=phase;p.offense=1-offense;p.down=1;p.resultStamp=11;p.fumble=true;p.game.seconds=295;
        auto result=tracker.observe(rules,p);assert(result.accepted&&result.completed&&result.takeaway&&!result.forcedFumble);
        auto fallback=live::completedImpact(result,p.game,true,0,83);
        assert(bool(fallback.id)==(offense==1||neutral));
        if(fallback.id){assert(fallback.event==live::Event::FumbleRecovery);assert(!live::completedImpact(result,p.game,true,result.sequence,59).id);}
        auto own=result;own.takeaway=false;assert(!live::completedImpact(own,p.game,true,0,83).id);
        assert(live::cameraReaction(live::Event::FumbleRecovery));
        if(offense==1||neutral){
            core::Model sound;core::Settings settings;sound.react(settings,result,1,p.game,true);
            assert(result.event==energy::Event::FumbleRecovery&&sound.resultCheer&&sound.resultType==59);
            auto forced=result;forced.forcedFumble=true;
            auto impact=live::completedImpact(forced,p.game,true,0,83);
            assert(impact.event==live::Event::ForcedFumble&&live::cameraReaction(impact.event));
            assert(!live::completedImpact(forced,p.game,true,forced.sequence,59).id);
        }
    }
    // Own pickups never add a return cue or revive a gain buildup. The forced
    // hit remains independently eligible.
    for(int owner:{0,1})for(bool neutral:{false,true})for(bool forced:{false,true}){
        live::Fumbles f;energy::Play p;p.game={2,7,7,1,300,720};p.game.neutral=neutral;p.offense=owner;p.down=1;live::ForcedEvent e{999,0,true};
        f.observe(p,123,1,1000,true,e,0,9,true);p.game.phase=3;f.observe(p,123,1,1100,true,e,0,9,true);
        e.type=forced?32:0;auto hit=f.observe(p,123,1,1200,true,e,0,5,false,true);
        assert(bool(hit.id)==(forced&&(owner==1||neutral)));if(hit.id)assert(hit.event==live::Event::ForcedFumble);
        p.offense=owner;auto own=f.observe(p,123,1,1300,true,e,0,2,true);assert(!own.id&&!f.recoveryBuildup(1300).id);
        assert(!f.observe(p,123,1,1400,true,e,0,2,true).id);
    }
    // A held pickup must not revive a long-run ramp from pre-fumble yardage.
    live::Model run;live::Settings limits;energy::Play p;p.game={2,7,7,1,300,720};p.down=1;p.offense=0;p.fieldSpot=-30;p.attackDirection=1;
    uint64_t now=1000;auto sample=[&](int phase,float yards,bool held){now+=100;p.game.phase=phase;return run.observe(limits,p,123,1,now,true,456,0,held?2:5,NAN,-30+yards,held);};
    sample(1,0,true);sample(3,0,true);for(int n=1;n<=14;++n)sample(3,static_cast<float>(n),true);assert(run.runBuildup().id);
    run.cancelRunBuildup(true);p.fumble=true;sample(3,14,false);p.fumble=false;
    for(int n=15;n<=35;++n){assert(!sample(3,static_cast<float>(n),true).id);assert(!run.runBuildup().id);}
    sample(4,35,true);sample(1,0,true);sample(3,0,true);live::Outcome fresh;
    for(int n=1;n<=20;++n){auto event=sample(3,static_cast<float>(n),true);if(event.id)fresh=event;}
    assert(fresh.event==live::Event::LongRun);
    // Exercise the production recovery detector and camera router together:
    // home defensive returns and either neutral side match interception motion;
    // away returns at home venues, own pickups and below-gate events do not.
    for(int owner:{0,1})for(bool neutral:{false,true})for(bool gate:{false,true})for(bool forced:{false,true}){
        live::Fumbles f;energy::Play play;play.game={2,7,7,1,300,720};play.game.neutral=neutral;play.offense=owner;play.down=1;
        live::ForcedEvent e{999,0,true};
        f.observe(play,123,1,1000,true,e,0,2,true);
        play.game.phase=3;f.observe(play,123,1,1100,true,e,0,2,true);
        e.type=forced?32:0;auto force=f.observe(play,123,1,1200,true,e,0,5,false,true);
        play.offense=1-owner;auto recovery=f.observe(play,123,1,1300,true,e,0,2,true);
        bool admitted=owner==1||neutral;
        assert(bool(recovery.id)==admitted);
        if(recovery.id)assert(recovery.event==live::Event::FumbleRecovery&&recovery.side==1-owner);
        auto id=defensivemotion::cameraId(recovery,2,{});
        assert(bool(id)==admitted);
        assert(!f.observe(play,123,1,1400,true,e,0,2,true).id); // Held return cannot retrigger.
        assert(!f.recoveryBuildup(1400).id); // No pre-fumble yardage fallback.
        motion::Model fumbleCamera,interceptionCamera;motion::Settings config;motion::Input x;
        x.match=123;x.epoch=1;x.allowed=true;x.venueEligible=gate;x.duringAllowed=gate;x.neutral=neutral;x.phase=3;x.now=1200;
        if(force.id&&gate){x.duringPlay=1;x.duringImpact=x.forcedImpact=true;}
        fumbleCamera.observe(config,x);interceptionCamera.observe(config,x);
        x.now=1300;x.duringPlay=gate?id:0;x.duringImpact=x.forcedImpact=false;
        auto actual=fumbleCamera.observe(config,x);
        auto interception=admitted?live::Outcome{live::Event::Interception,1,1-owner,0}:live::Outcome{};
        x.duringPlay=gate?defensivemotion::cameraId(interception,2,{}):0;
        auto expected=interceptionCamera.observe(config,x);
        assert(actual.kind==expected.kind&&actual.impactOnly==expected.impactOnly&&actual.oscillationBegan==expected.oscillationBegan);
        assert(bool(actual.kind==motion::Kind::LivePlay)==(admitted&&gate));
        if(admitted&&gate)assert(!actual.impactOnly&&!actual.forcedImpact);
        for(uint64_t at=1400;at<=2200;at+=100){
            x.now=at;actual=fumbleCamera.observe(config,x);expected=interceptionCamera.observe(config,x);
            assert(std::fabs(motion::strength(actual,at)-motion::strength(expected,at))<.001f);
        }
        x.phase=4;x.now=2300;actual=fumbleCamera.observe(config,x);expected=interceptionCamera.observe(config,x);
        assert(actual.until==expected.until&&actual.tail==expected.tail);
        if(admitted&&gate)assert(actual.tail);
        x.now=2400;x.allowed=false;assert(fumbleCamera.observe(config,x).kind==motion::Kind::None);
    }
    std::puts("PASS: verified defensive fumble returns match interception motion, ownership/neutral/layer gates and forced-impact handoff; confirmed result-only defensive recovery has a once-only fallback impact; own pickups cannot revive prior gain motion.");
}
