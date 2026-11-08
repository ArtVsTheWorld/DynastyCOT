#include "../Source/ExcitementRules.h"
#include <cassert>
#include <iostream>
using namespace attendance;
using namespace attendance::energy;
Play sample(){Play p;p.game={4,14,17,1,110,300};p.down=3;p.offense=1;p.homeTimeouts=1;p.toFirst=12;p.toGoal=40;return p;}
int main(){
    energy::Settings c;c.baselineDb=0;Play p=sample();Tracker t;
    assert(t.observe(c,p).event==Event::None);
    p.game.phase=3;p.game.seconds=107;t.observe(c,p);
    p.game.phase=4;p.game.awayScore=23;p.game.seconds=100;
    auto o=t.observe(c,p);assert(o.lateDeparture&&o.event==Event::None);
    assert(o.visitorScore);
    for(int i=0;i<20;i++)assert(!t.observe(c,p).lateDeparture);
    p.game.awayScore=24;assert(!t.observe(c,p).lateDeparture); // PAT cannot repeat the TD.
    p.game.awayScore=17;p.game.seconds=110;p.game.phase=1;assert(!t.observe(c,p).accepted);
    p.game.phase=3;t.observe(c,p);p.game.awayScore=24;p.game.seconds=100;p.game.phase=4;
    assert(!t.observe(c,p).lateDeparture); // Replay cannot arm a second copy.
    Play before=sample(),after=before;after.game.awayScore+=6;
    assert(lateExit(c,before,after));after.homeTimeouts=2;assert(!lateExit(c,before,after));
    after.homeTimeouts=-1;assert(!lateExit(c,before,after));after.homeTimeouts=0;
    after.game.seconds=121;assert(!lateExit(c,before,after));after.game.seconds=120;assert(lateExit(c,before,after));
    after.game.playoff=true;assert(!lateExit(c,before,after));after.game.playoff=false;
    after.game.awayScore=before.game.awayScore+1;assert(!lateExit(c,before,after));
    before.down=-1;after.game.awayScore+=6;assert(!lateExit(c,before,after));
    before=sample();after=before;after.game.homeScore+=7;assert(!lateExit(c,before,after));
    t.reset();p=sample();p.offense=0;p.down=4;t.observe(c,p);p.game.phase=3;t.observe(c,p);
    p.game.phase=5;p.down=1;p.firstDown=true;assert(t.observe(c,p).event==Event::FourthDown);
    assert(t.observe(c,p).event==Event::None);
    t.reset();p=sample();p.down=4;p.toGoal=2;t.observe(c,p);p.game.phase=3;t.observe(c,p);
    p.game.phase=5;p.down=1;p.offense=0;assert(t.observe(c,p).event==Event::GoalLine);
    t.reset();p=sample();p.resultStamp=1;t.observe(c,p);p.game.phase=3;t.observe(c,p);
    p.game.phase=4;p.game.homeScore+=6;p.resultStamp=2;p.interception=true;
    assert(t.observe(c,p).event==Event::PickSix);assert(t.observe(c,p).event==Event::None);
    t.reset();p=sample();p.offense=0;t.observe(c,p);p.game.phase=3;t.observe(c,p);
    p.game.phase=4;p.game.homeScore+=6;assert(t.observe(c,p).event==Event::LateDrive);
    p=sample();float close=eventPoints(Event::HomeScore,p.game);p.game.homeScore=42;p.game.awayScore=7;
    assert(eventPoints(Event::HomeScore,p.game)<close*0.15f);
    Model closeModel,blowout,underdog,minorUnderdog;
    p=sample();Play b=p;b.game.awayScore=44;
    for(int i=0;i<240;i++){closeModel.update(c,p,i*0.05,1);blowout.update(c,b,i*0.05,1);}
    assert(closeModel.score>blowout.score+40&&closeModel.anticipation>25&&blowout.anticipation==0);
    assert(closeModel.gainDb(c)>0&&blowout.gainDb(c)==0);
    float beforeSnap=closeModel.anticipation;p.game.phase=3;
    for(int i=240;i<260;i++)closeModel.update(c,p,i*0.05,1);
    assert(closeModel.anticipation==beforeSnap);p.game.phase=1;
    p.game.homeScore=21;p.game.rank0=132;p.game.rank1=3;Play m=p;m.game.rank0=26;
    for(int i=0;i<240;i++){underdog.update(c,p,i*0.05,1);minorUnderdog.update(c,m,i*0.05,1);}
    assert(underdog.stage==3&&underdog.score>minorUnderdog.score);
    Model eruption;assert(eruption.event(Event::LateDrive,0,p.game));
    assert(!eruption.event(Event::HomeScore,1,p.game));
    for(int i=0;i<400;i++){eruption.update(c,p,i*0.05,1);assert(eruption.score>=0&&eruption.score<=100&&eruption.gainDb(c)<=6);}
    assert(eruption.surge.value(20)==0);
    eruption.setback();assert(eruption.anticipation==0&&eruption.surge.value(20)==0);
    // A postplay attach never invents an eruption or a late-score departure.
    Tracker attached;p=sample();p.game.phase=4;p.game.awayScore=30;
    assert(attached.observe(c,p).event==Event::None&&!attached.observe(c,p).lateDeparture);
    // Fourth-down result can arrive after PlayCall begins; wait for the new
    // down/possession rather than consuming a still-unsettled result.
    t.reset();p=sample();p.offense=0;p.down=4;t.observe(c,p);p.game.phase=3;t.observe(c,p);
    p.game.phase=5;assert(t.observe(c,p).event==Event::None);
    p.down=1;p.firstDown=true;assert(t.observe(c,p).event==Event::FourthDown);
    p.offense=0;float ant=underdog.anticipation;
    for(int i=240;i<300;i++)underdog.update(c,p,i*0.05,1);
    assert(underdog.anticipation<ant);c.enabled=0;underdog.update(c,p,16,1);assert(underdog.gainDb(c)==0);
    c.enabled=1;Model bigGame;Play lsu=sample();lsu.game.homeScore=lsu.game.awayScore=17;
    lsu.game.rank0=6;lsu.game.rank1=11;lsu.game.rivalry=true;lsu.offense=0;
    for(int i=0;i<240;i++)bigGame.update(c,lsu,i*0.05,0.93f);
    assert(bigGame.matchupFloor>2.8f&&bigGame.gainDb(c)>=3); // Reported LSU/Bama regression.
    lsu.game.quarter=1;lsu.game.seconds=300;Model kickoff;kickoff.update(c,lsu,0,0.85f);
    assert(kickoff.gainDb(c)>2.7f); // First observation: floor does not wait for energy to accumulate.
    lsu.game.quarter=2;Model middle;middle.update(c,lsu,0,0.85f);assert(middle.gainDb(c)<kickoff.gainDb(c));
    lsu.game.awayScore=52;Model gone;for(int i=0;i<200;i++)gone.update(c,lsu,i*0.05,0.5f);
    assert(gone.matchupFloor==0&&gone.gainDb(c)==0); // Big-game status doesn't sustain a blowout roar.
    lsu.game.awayScore=17;c.maxDb=4;kickoff.update(c,lsu,1,1);assert(kickoff.gainDb(c)<=4);
    c.maxDb=12;kickoff.update(c,lsu,2,0);assert(kickoff.gainDb(c)==0);
    // Opening kickoff is a live play too, but only the actual game opening qualifies.
    t.reset();p=sample();p.game={1,0,0,1,300,300};p.down=0;p.offense=1;
    t.observe(c,p);p.game.phase=3;t.observe(c,p);p.game.phase=4;p.game.seconds=290;p.game.homeScore=6;
    assert(t.observe(c,p).event==Event::OpeningReturn);
    assert(t.observe(c,p).event==Event::None);p.game.homeScore=7;assert(t.observe(c,p).event==Event::None);
    p.game.homeScore=0;p.game.phase=1;p.game.seconds=300;assert(!t.observe(c,p).accepted);
    t.reset();p.game={1,0,0,1,300,300};p.down=0;p.offense=0;t.observe(c,p);p.game.phase=3;t.observe(c,p);
    p.game.phase=1;p.down=1;p.offense=0;p.game.seconds=300;t.observe(c,p);p.game.phase=3;t.observe(c,p);
    p.game.phase=4;p.game.homeScore=6;p.game.seconds=285;assert(t.observe(c,p).event==Event::OpeningTouchdown);
    // A later snap, a Q3 kickoff or attaching late cannot invent an opening event.
    for(int kind=0;kind<3;++kind){
        t.reset();p=sample();p.game={kind==1?3:1,0,0,1,kind==2?250.0f:300.0f,300};p.down=kind==1?0:1;p.offense=0;
        t.observe(c,p);p.game.phase=3;t.observe(c,p);
        if(kind==0){p.game.phase=1;p.down=2;p.game.seconds=299;t.observe(c,p);p.game.phase=3;t.observe(c,p);}
        p.game.phase=4;p.game.seconds-=5;p.game.homeScore=6;
        auto event=t.observe(c,p).event;assert(event!=Event::OpeningTouchdown&&event!=Event::OpeningReturn);
    }
    // A field goal at 0:00 that takes the lead is an eruption, not just touchdowns.
    t.reset();p=sample();p.game={4,20,21,1,5,300};p.offense=0;p.down=4;t.observe(c,p);
    p.game.phase=3;t.observe(c,p);p.game.phase=7;p.game.seconds=0;p.game.homeScore=23;
    assert(t.observe(c,p).event==Event::GameWinningScore);assert(t.observe(c,p).event==Event::None);
    assert(eventPoints(Event::LastSecondLead,p.game)>eventPoints(Event::HomeScore,p.game));
    for(float seconds:{0.0f,10.0f,30.0f,31.0f,60.0f,89.0f,90.0f,91.0f,120.0f})for(int side:{0,1}){
        t.reset();p=sample();p.game={4,20,21,1,120,300};p.offense=0;p.down=4;
        if(side==1){p.game=reversed(p.game);p.game.neutral=true;p.offense=1;}
        t.observe(c,p);p.game.phase=3;t.observe(c,p);p.game.phase=4;p.game.seconds=seconds;
        if(side==0)p.game.homeScore+=3;else p.game.awayScore+=3;
        o=t.observe(c,p);assert(o.event==(seconds<=30?Event::LastSecondLead:Event::LateDrive)&&o.side==side);
        assert(t.observe(c,p).event==Event::None);
    }
    // Neutral venues treat equivalent home and away moments identically.
    for(int side:{0,1}){
        t.reset();p=sample();p.game={1,0,0,1,300,300};p.game.neutral=true;p.offense=1-side;p.down=0;
        t.observe(c,p);p.game.phase=3;t.observe(c,p);p.game.phase=4;p.game.seconds=292;
        if(side==0)p.game.homeScore=6;else p.game.awayScore=6;
        o=t.observe(c,p);assert(o.event==Event::OpeningReturn&&o.side==side&&!o.visitorScore);
        p=sample();p.game.neutral=true;p.offense=side;
        assert(anticipationPoints(c,p,0)>0);p.game.neutral=false;
        if(side==0)assert(anticipationPoints(c,p,0)==0);
    }
    p=sample();p.game.neutral=true;after=p;after.game.awayScore+=6;assert(!lateExit(c,p,after));
    // Confirm the additional live-play events use real completed snaps.
    for(Event expected:{Event::Turnover,Event::FourthStop,Event::TakeLead,Event::TieScore,Event::LongTouchdown}){
        t.reset();p=sample();p.game={3,10,13,1,200,300};p.offense=0;p.down=1;p.resultStamp=1;
        if(expected==Event::Turnover||expected==Event::FourthStop){p.offense=1;p.down=4;}
        if(expected==Event::TieScore){p.game.quarter=4;p.game.seconds=180;}
        if(expected==Event::LongTouchdown){p.game.homeScore=20;p.toGoal=60;}
        t.observe(c,p);p.game.phase=3;t.observe(c,p);
        p.game.phase=1;p.down=1;p.offense=0;p.game.seconds-=10;
        if(expected==Event::Turnover){p.interception=true;p.resultStamp=2;}
        if(expected==Event::TakeLead||expected==Event::LongTouchdown)p.game.homeScore+=6;
        if(expected==Event::TieScore)p.game.homeScore+=3;
        assert(t.observe(c,p).event==expected);assert(t.observe(c,p).event==Event::None);
    }
    energy::Settings louder;Model normal;Play ordinary=sample();ordinary.game.homeScore=0;ordinary.game.awayScore=42;ordinary.down=1;
    for(int i=0;i<400;i++)normal.update(louder,ordinary,i*0.05,1);
    assert(normal.gainDb(louder)==0); // Ordinary games no longer have a universal lift.
    Model stadium;lsu.game.homeScore=lsu.game.awayScore=17;lsu.game.quarter=1;
    stadium.update(louder,lsu,0,1);assert(stadium.gainDb(louder)==3);
    assert(stadium.event(Event::OpeningTouchdown,0,lsu.game));
    for(int i=1;i<400;i++){stadium.update(louder,lsu,i*0.05,1);assert(stadium.gainDb(louder)<=6);}
    louder.maxDb=0;assert(stadium.gainDb(louder)==0);
    Envelope dramatic;assert(dramatic.trigger(Event::LastSecondLead,0,lsu.game));
    assert(dramatic.trigger(Event::LastSecondLead,4,lsu.game)); // A new winning score beats cooldown at a neutral venue.
    std::cout<<"PASS: unified energy, staged upset gaps, pre-snap anticipation, contextual surges, bounded gains, replay/score dedupe and late-score timeout exits.\n";
}
