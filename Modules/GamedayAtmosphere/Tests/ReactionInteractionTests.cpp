#include "../Source/DirectorShake.h"
#include "../Source/ScoringSoundHandoff.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
enum class Event {Lineup,Critical,Run,Catch,Skill,Hit,Touchdown,Eruption,Safety,Block,Clincher};
bool pulse(Event e){return e>=Event::Touchdown;}
motion::Input request(Event e,uint64_t now,uint64_t id,bool post=false){
    motion::Input x;x.match=123;x.epoch=1;x.now=now;x.allowed=x.duringAllowed=x.eruptionAlive=true;
    x.phase=pulse(e)||post?4:e<=Event::Critical?1:3;
    if(e<=Event::Critical){x.phase=1;x.lineup=x.venueEligible=true;x.lineupId=id;x.critical=e==Event::Critical;}
    else if(!pulse(e)){x.duringPlay=id;x.duringImpact=e!=Event::Run;x.duringCatch=e==Event::Catch;x.softAccent=e==Event::Skill;x.supplementalImpact=x.softAccent;x.impactMilliseconds=x.softAccent?2000:0;x.reactionScale=x.softAccent?.5f:1;}
    else if(e==Event::Touchdown)x.touchdown=id;else if(e==Event::Eruption)x.eruption=id;else if(e==Event::Safety)x.safety=id;else if(e==Event::Block)x.blockedFieldGoal=id;else x.clincher=id;
    return x;
}
int main(){
    constexpr Event events[]{Event::Lineup,Event::Critical,Event::Run,Event::Catch,Event::Skill,Event::Hit,Event::Touchdown,Event::Eruption,Event::Safety,Event::Block,Event::Clincher};
    // Every ordered pair: no amplitude or oscillator jump at the handoff,
    // bounded independent overlap, and higher scoring profiles cannot be downgraded.
    for(auto first:events)for(auto second:events){motion::Model model;motion::Settings settings;settings.live=10;settings.catchImpact=8;motion::Cue old;
        for(uint64_t now=1000;now<=1900;now+=100)old=model.observe(settings,request(first,now,1));
        float before=motion::strength(old,2000);auto out=model.observe(settings,request(second,2000,2,pulse(first)));
        if(std::fabs(motion::strength(out,2000)-before)>=.001f)std::fprintf(stderr,"Handoff %d -> %d: %.4f -> %.4f\n",static_cast<int>(first),static_cast<int>(second),before,motion::strength(out,2000));
        assert(std::fabs(motion::strength(out,2000)-before)<.001f);
        if(before>0)assert((out.oscillationBegan?out.oscillationBegan:out.began)==(old.oscillationBegan?old.oscillationBegan:old.began));
        if(first==Event::Eruption)assert(out.kind==motion::Kind::Eruption);
        if(first==Event::Touchdown&&second!=Event::Eruption&&second!=Event::Touchdown)assert(out.kind==motion::Kind::Touchdown);
        for(uint64_t now=2100;now<=3000;now+=100){out=model.observe(settings,request(second,now,2,pulse(first)));assert(motion::strength(out,now)>=0&&motion::strength(out,now)<=30.01f);}
    }
    // A lower impact arriving under a short TD is consumed, not queued to
    // resurrect after the TD. The next actual snap rearms normally.
    {motion::Model model;motion::Settings settings;settings.live=10;settings.catchImpact=8;settings.touchdownMs=3000;auto x=request(Event::Touchdown,1000,1);model.observe(settings,x);
        for(uint64_t now=1100;now<=5000;now+=100){x=request(now<2000?Event::Touchdown:Event::Hit,now,now<2000?1:2,true);auto cue=model.observe(settings,x);if(now>=4000)assert(cue.kind==motion::Kind::None);}
        assert(model.observe(settings,request(Event::Run,5100,3)).kind==motion::Kind::LivePlay);
    }
    // A realistic multi-event chain: fourth-down impact -> gain buildup ->
    // skill accent -> forced fumble -> sound-only pickup -> TD -> eruption.
    {motion::Model model;motion::Settings settings;settings.live=10;settings.catchImpact=8;auto x=request(Event::Hit,1000,1);auto cue=model.observe(settings,x);
        x.now=1200;cue=model.observe(settings,x);auto anchor=cue.oscillationBegan?cue.oscillationBegan:cue.began;
        x=request(Event::Run,1300,0);x.runBuildup=77;x.runProgress=.1f;cue=model.observe(settings,x);assert(motion::strength(cue,x.now)==10);
        x.now=1500;x.runProgress=1;x.duringPlay=2;cue=model.observe(settings,x);assert(motion::strength(cue,x.now)==10);
        x=request(Event::Skill,1600,3);cue=model.observe(settings,x);assert(!cue.softAccent&&motion::strength(cue,x.now)>=10&&motion::strength(cue,x.now)<=11.5f);
        x=request(Event::Hit,1700,4);cue=model.observe(settings,x);auto forcedEnd=cue.until;
        x.duringPlay=0;x.now=1900;cue=model.observe(settings,x);assert(cue.until==forcedEnd);
        x=request(Event::Touchdown,2000,1);cue=model.observe(settings,x);assert(cue.kind==motion::Kind::Touchdown&&motion::strength(cue,x.now)==10);
        x.now=2200;model.observe(settings,x);x=request(Event::Eruption,2300,1);cue=model.observe(settings,x);
        assert(cue.kind==motion::Kind::Eruption&&cue.oscillationBegan==anchor);
        x=request(Event::Run,2400,5);assert(model.observe(settings,x).kind==motion::Kind::LivePlay); // A genuinely new snap retires the celebration.
        x.allowed=false;x.now=2500;assert(model.observe(settings,x).kind==motion::Kind::None);
    }
    // Result-to-result, lineup-to-result and live-to-score sound bridges use
    // one fixed half-second bridge, without restarting timers or event IDs.
    for(bool lineup:{false,true})for(bool scoring:{false,true})for(float fullness:{.5f,1.f}){
        soundhandoff::Model handoff;sustained::Intent a;a.active=a.continuous=true;a.match=123;a.epoch=1;a.reason=lineup?sustained::Reason::Lineup:sustained::Reason::Result;
        a.priority=lineup?1:4;a.reactionId=1;a.updated=1000;a.intensity=800*std::sqrt(fullness);handoff.merge(a,3,1000,fullness);
        auto next=a;next.reason=sustained::Reason::Result;next.priority=scoring?5:4;next.reactionId=2;next.scoringReaction=scoring;next.intensity=200*std::sqrt(fullness);next.releaseMs=5000;
        for(uint64_t now=1100;now<=1700;now+=50){next.updated=now;auto out=handoff.merge(next,scoring?4:3,now,fullness);
            if(now<=1350)assert(out.intensity>=a.intensity-.01f);if(now>=1600)assert(out.intensity==next.intensity);
            assert(out.reactionId==2&&out.releaseMs==5000&&out.priority==next.priority&&out.intensity<=1000*std::sqrt(fullness));}
        next.updated=1800;next.intensity=0;next.liveCheer=3;next.liveType=95;assert(handoff.merge(next,4,1800,fullness).intensity==0);
    }
    std::puts("PASS: 121 ordered camera handoffs, oscillator/amplitude continuity, scoring priority, no hidden lower-effect resurrection, fixed sound bridges, caps and negative/new-snap isolation.");
}
