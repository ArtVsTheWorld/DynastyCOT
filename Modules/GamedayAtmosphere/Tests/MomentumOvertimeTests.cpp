#include "../Source/ExcitementRules.h"
#include "../Source/DepartureModel.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace attendance;
using namespace attendance::energy;
Play sample(){Play p;p.game={3,14,14,1,200,300};p.down=1;p.offense=1;p.toFirst=10;p.toGoal=40;p.resultStamp=100;return p;}
int main(){
    energy::Settings c;Tracker tracker;Model model;Play p=sample();
    float heat=0;uint64_t lastSequence=0;
    // Sack / TFL / third-down stop, confirmed by the native down/distance result.
    for(int down:{1,2,3}){
        p.down=down;p.game.phase=1;tracker.observe(c,p);p.game.phase=3;tracker.observe(c,p);
        p.game.phase=5;p.down=down+1;p.toFirst+=3;p.game.seconds-=6;p.resultStamp+=1;
        auto result=tracker.observe(c,p);
        assert(result.completed&&result.sequence>lastSequence&&result.side==0);lastSequence=result.sequence;
        assert(result.event==(down==3?Event::ThirdStop:Event::BackfieldStop));
        model.react(c,result,down*20.0,p.game);assert(model.momentum.heat>heat);heat=model.momentum.heat;
        for(int repeat=0;repeat<100;++repeat){auto duplicate=tracker.observe(c,p);assert(!duplicate.completed&&duplicate.event==Event::None);}
        assert(model.momentum.chain==static_cast<unsigned>(down)&&model.momentum.heat<=30);
    }
    float end=model.momentum.heat;model.momentum.advance(model.momentum.last+45,45);
    assert(std::fabs(model.momentum.heat-end*0.5f)<0.001f);
    model.momentum.advance(1000,45);assert(model.momentum.heat==0&&model.momentum.chain==0);
    Momentum m;assert(m.observe(10,0,1,0,45));assert(!m.observe(10,0,1,0,45));
    assert(m.observe(10,20,2,0,45));float before=m.heat;m.setback();assert(m.heat<before&&m.chain==0);
    m.reset();m.observe(10,0,1,0,45);m.observe(10,20,2,1,45);assert(m.chain==1&&m.side==1);
    m.observe(0,30,3,1,45);assert(m.chain==0); // An ordinary play breaks the consecutive run, while heat decays.
    for(int i=4;i<1000;++i)m.observe(16,30+i,static_cast<uint64_t>(i),1,45);
    assert(m.heat<=30&&m.chain<=5);
    // Exactly one primary reaction: a winning pick-six takes precedence over its components.
    tracker.reset();p=sample();p.game={4,17,21,1,20,300};p.down=3;tracker.observe(c,p);
    p.game.phase=3;tracker.observe(c,p);p.game.phase=4;p.game.seconds=10;p.game.homeScore=23;p.resultStamp=101;p.interception=true;
    auto winner=tracker.observe(c,p);assert(winner.event==Event::LastSecondLead&&winner.completed);
    assert(tracker.observe(c,p).event==Event::None);
    Envelope priorities;assert(priorities.trigger(Event::BackfieldStop,0,p.game,1));
    assert(!priorities.trigger(Event::BackfieldStop,1,p.game,2));
    assert(priorities.trigger(Event::PickSix,2,p.game,2));assert(!priorities.trigger(Event::ThirdStop,3,p.game,3));
    assert(priorities.trigger(Event::LastSecondLead,3,p.game,2)); // Upgrade the same play, never add its gain twice.
    assert(!priorities.trigger(Event::LastSecondLead,4,p.game,2));
    assert(priorities.trigger(Event::OvertimeWin,4,p.game,2));
    // Neutral-site offensive plays keep the scoring side even when both defenses are eligible.
    for(int side:{0,1}){
        tracker.reset();p=sample();p.game.neutral=true;p.offense=side;p.down=4;p.firstDown=false;
        tracker.observe(c,p);p.game.phase=3;tracker.observe(c,p);p.game.phase=5;p.down=1;p.firstDown=true;
        auto conversion=tracker.observe(c,p);assert(conversion.event==Event::FourthDown&&conversion.side==side);
    }
    // Native OT is quarter 5. Quarter 6 is EndOfGame and must not get an OT floor.
    p=sample();p.game={5,24,24,1,0,300};p.down=1;
    for(int side:{0,1}){p.offense=side;assert(anticipationPoints(c,p,0)>=20);}
    for(int down:{1,2,3,4}){p.down=down;p.offense=1;assert(anticipationPoints(c,p,0)>=30);}
    State regulation=p.game;regulation.quarter=3;
    assert(eventPoints(Event::Turnover,p.game)>eventPoints(Event::Turnover,regulation)*1.5f);
    assert(eventPoints(Event::ThirdStop,p.game)>eventPoints(Event::ThirdStop,regulation)*1.5f);
    Model ot;ot.update(c,p,0,1);assert(ot.overtimeActive&&ot.gainDb(c)>=4);
    p.game.quarter=6;p.game.phase=7;Model ended;ended.update(c,p,0,1);assert(!ended.overtimeActive&&ended.matchupFloor==0);
    p.game={5,24,24,1,0,0};assert(normalizeUntimedPeriod(p.game,300)&&valid(p.game));
    State invalid=p.game;invalid.quarter=4;invalid.quarterSeconds=0;assert(!normalizeUntimedPeriod(invalid,300));
    invalid=p.game;invalid.quarterSeconds=0;assert(!normalizeUntimedPeriod(invalid,NAN));
    // Every OT possession keeps a stationary clock. Fresh results still count once.
    tracker.reset();p=sample();p.game={5,24,24,1,0,300};p.offense=1;p.down=1;p.toFirst=10;
    for(int down:{1,2,3}){
        p.game.phase=1;p.down=down;tracker.observe(c,p);p.game.phase=3;tracker.observe(c,p);
        p.game.phase=5;p.down=down+1;p.toFirst+=3;++p.resultStamp;
        auto result=tracker.observe(c,p);assert(result.completed&&result.event==(down==3?Event::ThirdStop:Event::BackfieldStop));
        assert(!tracker.observe(c,p).completed);
    }
    // Resetting the unused OT clock cannot block subsequent possessions.
    p.game.phase=1;p.game.seconds=300;p.down=4;assert(tracker.observe(c,p).accepted);
    p.game.phase=3;tracker.observe(c,p);p.game.phase=5;p.offense=0;p.down=1;++p.resultStamp;
    assert(tracker.observe(c,p).event==Event::FourthStop);
    // Replaying a scoreless OT result with no fresh native record cannot produce momentum.
    tracker.reset();p=sample();p.game={5,24,24,1,0,300};p.down=3;tracker.observe(c,p);p.game.phase=3;tracker.observe(c,p);
    p.game.phase=5;p.down=4;assert(!tracker.observe(c,p).completed);
    // OT shootouts use native PAT down -1. They still get snap anticipation,
    // valid conversions and defensive stops, with the correct neutral-side owner.
    for(int side:{0,1})for(bool successful:{false,true}){
        tracker.reset();p=sample();p.game={5,24,24,1,0,300};p.game.neutral=true;p.down=-1;p.offense=side;
        assert(anticipationPoints(c,p,0)>=30);tracker.observe(c,p);p.game.phase=3;tracker.observe(c,p);
        p.game.phase=1;p.offense=1-side;++p.resultStamp;
        if(successful){if(side==0)p.game.homeScore+=2;else p.game.awayScore+=2;}
        auto conversion=tracker.observe(c,p);
        assert(conversion.event==(successful?Event::OvertimeConversion:Event::ConversionStop));
        assert(conversion.completed&&conversion.side==(successful?side:1-side)&&tracker.observe(c,p).event==Event::None);
    }
    // A stale first-down flag alone must not suppress a verified backfield loss.
    tracker.reset();p=sample();tracker.observe(c,p);p.game.phase=3;tracker.observe(c,p);
    p.game.phase=1;p.down=2;p.toFirst=14;p.firstDown=true;assert(tracker.observe(c,p).event==Event::BackfieldStop);
    // No new departures or arrivals in OT, including a carried Q4 warning.
    attendance::Settings attendance;History history;history.warningPending=true;
    p.game={5,24,24,1,0,300};int loss=42;
    departures::Model exits;
    for(int seconds:{0,300,0,300}){p.game.seconds=static_cast<float>(seconds);assert(history.observe(attendance,p.game));exits.observe(attendance,p.game,history);assert(exits.settle(p.game,loss).loss==loss);}
    // Winner celebrations beat a weaker result of the same snap.
    tracker.reset();p=sample();p.game={5,24,24,1,0,300};p.offense=0;tracker.observe(c,p);p.game.phase=3;tracker.observe(c,p);
    p.game.phase=7;p.game.quarter=6;p.game.homeScore=30;++p.resultStamp;
    auto otWinner=tracker.observe(c,p);assert(otWinner.event==Event::OvertimeWin&&tracker.observe(c,p).event==Event::None);
    model.reset();p=sample();p.game={5,24,24,1,0,300};p.game.rivalry=true;p.game.rank0=132;p.game.rank1=3;
    for(int i=0;i<1000;++i){
        Outcome o;o.accepted=o.completed=true;o.event=Event::Turnover;o.sequence=i+1;
        model.react(c,o,i*0.05,p.game);model.update(c,p,i*0.05,0.9f);
        assert(model.gainDb(c)<=6&&model.score<=100&&model.momentum.heat<=30);
    }
    c.momentum=0;c.overtimeAtmosphere=0;model.update(c,p,60,1);assert(model.momentum.heat==0&&!model.overtimeActive);
    State overtimeState=p.game;overtimeState.quarter=5;
    assert(eventPoints(Event::Turnover,overtimeState,false)<eventPoints(Event::Turnover,overtimeState,true));
    Outcome unverified;unverified.completed=true;unverified.event=Event::Turnover;unverified.sequence=2000;
    model.reset();assert(!model.react(c,unverified,0,overtimeState)&&model.momentum.heat==0);
    // Regulation momentum uses football time, with no wall-time/pause penalty.
    MomentumClock clock;State football{2,14,14,1,200,300};
    assert(clock.advance(football,0)==0);
    assert(clock.advance(football,600)==0);
    football.seconds=155;assert(clock.advance(football,601)==45);
    football.seconds=170;assert(clock.advance(football,602)==45); // Rewind / correction.
    football.seconds=155;assert(clock.advance(football,603)==45); // No double charge.
    football.seconds=150;assert(clock.advance(football,604)==50);
    football.seconds=0;assert(clock.advance(football,605)==200);
    football.quarter=3;football.seconds=300;assert(clock.advance(football,606)==200);
    football.seconds=290;assert(clock.advance(football,607)==210);
    football.quarter=5;football.seconds=0;assert(clock.advance(football,608)==800);
    assert(clock.advance(football,1200)==800); // Untimed play selection / timeout.
    football.phase=3;assert(clock.advance(football,1201)==800);
    for(int i=1;i<=200;++i)clock.advance(football,1201+i*0.05);
    assert(std::fabs(clock.seconds-810)<0.001);
    football.phase=5;clock.advance(football,1212);clock.advance(football,1400);
    assert(std::fabs(clock.seconds-810)<0.001);
    // The integrated model decays by 45 game seconds, not 45 wall seconds.
    model.reset();p=sample();p.offense=0;
    Outcome big;big.accepted=big.completed=true;big.event=Event::ExplosiveGain;big.sequence=1;
    model.react(c=energy::Settings{},big,0,p.game);end=model.momentum.heat;
    model.update(c,p,600,1);assert(std::fabs(model.momentum.heat-end)<0.001f);
    p.game.seconds-=45;model.update(c,p,601,1);
    assert(std::fabs(model.momentum.heat-end*0.5f)<0.001f);
    // Verified field movement works without either presentation goal distance or first-down flag.
    for(int side:{0,1})for(int direction:{-1,1})for(int down:{1,2,3,4}){
        tracker.reset();p=sample();p.game.neutral=true;p.offense=side;p.down=down;
        p.toGoal=-1;p.firstDown=false;p.fieldSpot=direction*-30.0f;p.attackDirection=direction;
        tracker.observe(c,p);p.game.phase=3;tracker.observe(c,p);
        p.game.phase=1;p.down=1;p.fieldSpot+=direction*22;p.game.seconds-=8;++p.resultStamp;
        auto gain=tracker.observe(c,p);
        Event expected=down==4?Event::FourthDown:down==3?Event::ThirdDownConversion:Event::ExplosiveGain;
        assert(gain.event==expected&&gain.completed&&gain.side==side);
        assert(!tracker.observe(c,p).completed);
    }
    for(float yards:{9.0f,10.0f,19.0f,25.0f}){
        tracker.reset();p=sample();p.offense=0;p.toGoal=-1;p.fieldSpot=-20;p.attackDirection=1;
        tracker.observe(c,p);p.game.phase=3;tracker.observe(c,p);
        p.game.phase=1;p.down=1;p.fieldSpot+=yards;p.game.seconds-=7;
        auto gain=tracker.observe(c,p);
        assert(gain.event==(yards>=10?Event::ExplosiveGain:Event::None));
    }
    // Short third/fourth-down conversions still build momentum; higher stakes earn more.
    for(int down:{3,4}){
        tracker.reset();p=sample();p.offense=0;p.down=down;p.toFirst=2;p.toGoal=-1;
        p.fieldSpot=20;p.attackDirection=1;tracker.observe(c,p);p.game.phase=3;tracker.observe(c,p);
        p.game.phase=1;p.down=1;p.fieldSpot=23;p.game.seconds-=5;
        auto converted=tracker.observe(c,p);
        assert(converted.event==(down==3?Event::ThirdDownConversion:Event::FourthDown));
        assert(eventPoints(converted.event,p.game)>eventPoints(Event::ExplosiveGain,p.game));
    }
    // A long gain that does not earn a first down is not an offensive momentum play.
    tracker.reset();p=sample();p.offense=0;p.down=1;p.toFirst=25;p.toGoal=-1;p.fieldSpot=0;p.attackDirection=1;
    tracker.observe(c,p);p.game.phase=3;tracker.observe(c,p);p.game.phase=1;p.down=2;p.fieldSpot=20;
    assert(tracker.observe(c,p).event==Event::None);
    // Final-90-second drives ramp progressively toward the goal line, including neutral venues.
    for(int side:{0,1}){
        p=sample();p.game={4,20,24,1,60,300};p.offense=side;p.down=1;
        if(side==1){p.game=reversed(p.game);p.game.neutral=true;}
        float prior=0;
        for(float yards:{20.0f,15.0f,10.0f,5.0f,1.0f}){
            p.toGoal=yards;float ramp=anticipationPoints(c,p,0);assert(ramp>prior);prior=ramp;
        }
        p.toGoal=20;Model redZone;
        for(int i=0;i<200;++i)redZone.update(c,p,i*0.05,1);
        assert(redZone.gainDb(c)>c.baselineDb);
        p.toGoal=1;
        for(int i=200;i<400;++i)redZone.update(c,p,i*0.05,1);
        assert(redZone.gainDb(c)>3.5f&&redZone.gainDb(c)<=c.maxDb);
    }
    p=sample();p.offense=0;p.down=1;p.toGoal=5;p.game={4,20,24,1,90,300};
    assert(lateDriveAnticipation(c,p)>0);p.game.seconds=91;assert(lateDriveAnticipation(c,p)==0);
    p.game.seconds=60;p.game.awayScore=40;assert(lateDriveAnticipation(c,p)==0);
    p.game.awayScore=24;p.game.quarter=3;assert(lateDriveAnticipation(c,p)==0);
    p.game.quarter=4;p.offense=1;assert(lateDriveAnticipation(c,p)==0);
    p.offense=0;p.game.phase=5;assert(lateDriveAnticipation(c,p)==0);
    c.chaos=0;p.game.phase=1;assert(lateDriveAnticipation(c,p)==0);
    std::cout<<"PASS: 10-yard first downs and conversion bonuses, game-clock decay, late red-zone buildup, consecutive-play momentum and caps, single-play priority, neutral symmetry, stationary/reset OT clocks, native result dedupe, every-down anticipation, OT protection and winners.\n";
}
