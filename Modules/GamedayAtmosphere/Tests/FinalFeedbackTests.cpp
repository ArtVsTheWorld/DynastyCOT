#include "../Source/LineupAtmosphere.h"
#include "../Source/LiveFumble.h"
#include "../Source/ReturnAtmosphere.h"
#include "../Source/DirectorShake.h"
#include "../Source/HalftimeExitConversion.h"
#include "../Source/ArrivalRules.h"
#include "../Source/DepartureModel.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    for(int points:{1,2})for(bool away:{false,true}){
        energy::Play p;p.game={4,14,14,1,15,720};p.game.neutral=away;p.down=-1;p.offense=away?1:0;p.resultStamp=1;
        if(points==2){if(away)p.game.awayScore=13;else p.game.homeScore=13;}
        energy::Tracker tracker;energy::Settings events;tracker.observe(events,p);p.game.phase=3;tracker.observe(events,p);
        p.game.phase=4;p.game.seconds=12;p.resultStamp=2;if(away)p.game.awayScore+=points;else p.game.homeScore+=points;
        auto scored=tracker.observe(events,p);assert(scored.completed&&scored.event==energy::Event::LastSecondLead&&scored.side==(away?1:0));
        core::Model atmosphere;core::Settings c;assert(atmosphere.react(c,scored,10,p.game,true)&&atmosphere.erupting==false); // react queues; update supplies the envelope.
        atmosphere.update(c,p,10.25,.9f,.9f,0,true);assert(atmosphere.erupting&&volume::boostPercent(atmosphere.parent)<=50.001f);
        assert(!atmosphere.react(c,scored,10.5,p.game,true));
    }
    {energy::Play p;p.game={1,7,10,1,600,720};p.down=3;p.toFirst=2;p.toGoal=60;p.offense=1;
        lineup::Model m;core::Settings c;m.observe(c,p,123,1,1000,true,.9f,true,nullptr,51498);m.observe(c,p,123,1,5000,true,.9f,true,nullptr,51498);
        float before=m.excitement(5000,.9f);assert(before>900);
        p.game.phase=3;m.observe(c,p,123,1,5050,true,.9f,true,nullptr,51498);assert(m.carrying&&m.excitement(5050,.9f)>=before-.001f);
        p.game.phase=4;m.observe(c,p,123,1,7000,true,.9f,true,nullptr,51498);assert(m.excitement(7000,.9f)>=before-.001f);
    }
    {live::Fumbles f;energy::Play p;p.game={2,7,7,1,300,720};p.down=1;p.offense=1;live::ForcedEvent e{1234,0,true};
        f.observe(p,123,1,1000,true,e,0,2,false,true);p.game.phase=3;f.observe(p,123,1,1100,true,e,0,9,true,true);
        e.type=32;assert(f.observe(p,123,1,1200,true,e,3,5,false,true).event==live::Event::ForcedFumble);
        p.offense=0;auto recovered=f.observe(p,123,1,1300,true,e,4,9,true,true);assert(recovered.event==live::Event::FumbleRecovery&&!f.recoveryBuildup(1300).id);
        f.observe(p,123,1,2050,true,e,4,9,true,true);assert(!f.recoveryBuildup(2050).id);
        f.observe(p,123,1,2800,true,e,4,9,true,true);assert(!f.recoveryBuildup(2800).id);
        p.game.phase=4;f.observe(p,123,1,2900,true,e,4,9,false,true);assert(!f.recoveryBuildup(2900).id);
    }
    {live::Fumbles f;energy::Play p;p.game={2,7,7,1,300,720};p.down=1;p.offense=1;live::ForcedEvent none;
        f.observe(p,123,1,1000,true,none,0,2,false,true);p.game.phase=3;f.observe(p,123,1,1100,true,none,0,9,true,true);
        assert(!f.observe(p,123,1,1200,true,none,3,5,false,true).id);p.offense=0;
        assert(f.observe(p,123,1,1300,true,none,4,9,true,true).event==live::Event::FumbleRecovery);
    }
    {motion::Model m;motion::Settings s;s.live=10;s.catchImpact=8;assert(s.reactionFadeMs==5000);motion::Input x;x.match=123;x.epoch=1;x.allowed=x.duringAllowed=true;x.phase=3;x.now=1000;x.duringPlay=1;
        m.observe(s,x);x.now=1200;assert(motion::strength(m.observe(s,x),1200)==10);
        x.now=1900;m.observe(s,x);x.phase=4;x.now=2000;x.duringPlay=0;auto tail=m.observe(s,x);assert(tail.tail&&tail.until==7000);
        x.now=4500;tail=m.observe(s,x);assert(motion::strength(tail,4500)>7&&motion::strength(tail,4500)<8);
        x.now=7000;assert(m.observe(s,x).kind==motion::Kind::None);
    }
    {live::ReturnAtmosphere r;energy::Play p;p.game={2,7,7,3,300,720};p.down=1;p.offense=1;
        live::Outcome pick{live::Event::Interception,1,0,0};r.observe(p,123,1,1000,true,pick,true,false);
        sustained::Intent sound;sound.active=true;sound.match=123;sound.epoch=1;
        assert(r.merge(sound,1000,.9f).continuous);
        p.offense=0;r.observe(p,123,1,1200,true,{},true,false);assert(r.merge(sound,1200,.9f).continuous);
        p.game.phase=4;r.observe(p,123,1,1500,true,{},false,false);auto tail=r.merge(sound,1500,.9f);assert(tail.continuous&&!tail.reaction&&tail.supporting&&tail.intensity>0&&tail.releaseMs==3800);
    }
    State half{3,0,18,5,720,720};halftime::ExitConversion conversion;
    assert(conversion.request(half,5,30)==5&&!conversion.request(half,5,30));
    for(int loss:{0,5,10,15,19,20}){conversion.reset();int amount=conversion.request(half,loss,loss+25);assert(amount>=0&&amount<=6);if(loss>=20)assert(!amount);}
    for(bool big:{false,true}){conversion.reset();auto s=half;if(big){s.rank0=6;s.rank1=11;}else s.playoff=true;assert(!conversion.request(s,5,30));}
    conversion.reset();half.homeScore=20;assert(!conversion.request(half,5,30));half.homeScore=0;
    arrivals::Halftime seats;seats.active=seats.seen=true;seats.lastQuarter=3;seats.highTime=1440;seats.breakPercent=25;seats.lastAbsent=25;seats.temporary=30;
    seats.convert(half,5,10,30,100);assert(seats.observe(half,10)==30); // The already-empty seats do not empty a second time.
    half.seconds=630;int returning=seats.observe(half,10);assert(returning<30&&returning>10);
    departures::Model departures;departures.beginWindow(1);auto changed=departures.convertSeatBreakers(half,5,5,1);assert(changed.loss<=6&&departures.windowBudget==1);
    std::puts("PASS: regulation winning PAT/two-point scores, native limits/dedupe, critical-down snap continuity, live forced fumbles with sound-only pickups, five-second whistle fade and once-only smooth halftime permanent conversion with caps and no double removal.");
}
