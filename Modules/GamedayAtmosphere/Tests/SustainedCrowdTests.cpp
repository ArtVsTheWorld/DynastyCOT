#include "../Source/SustainedCrowd.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
using namespace attendance::sustained;
int main(){
    energy::Settings c;energy::Play p;p.game={4,17,17,1,70,900};p.down=1;p.offense=0;p.toGoal=12;
    energy::Model m;m.score=100;m.anticipation=50;m.heldFraction=1;
    auto choose=[&](uint64_t now=1000){return select(c,p,m,123,4,now,true);};
    Intent close=choose();assert(close.reason==Reason::LateDrive&&close.size==3&&close.intensity==1000);
    p.toGoal=35;m.anticipation=0;m.score=60;assert(choose().intensity<close.intensity);
    p.game.homeScore=42;p.game.awayScore=7;assert(choose().reason==Reason::None);
    p.game.homeScore=17;p.game.awayScore=17;p.offense=1;p.down=3;p.toGoal=40;p.toFirst=12;
    assert(choose().reason==Reason::DefensiveDown);
    p.offense=0;assert(choose().reason==Reason::None); // Don't cheer against the home defense's opponent.
    p.game.neutral=true;assert(choose().reason==Reason::DefensiveDown);
    p.game.neutral=false;p.game.quarter=5;p.down=1;assert(choose().reason==Reason::Overtime);
    p.down=-1;assert(choose().reason==Reason::Overtime);p.down=1;
    p.game.quarter=2;m.stage=2;m.score=65;assert(choose().reason==Reason::Upset);
    p.game={1,0,0,6,900,900};p.offense=-1;m.openingWeight=1;m.heldFraction=.6f;m.score=60;
    assert(choose().reason==Reason::Opening&&choose().size==3&&choose().intensity==1000); // Arrivals don't drain opening hype.
    assert(select(c,p,m,123,4,1000,true,.25f).intensity<1000); // Permanent exits still matter.
    p.game.phase=1;p.down=-1;m.openingProfile.fade=0;assert(choose().reason==Reason::None); // Opening possession has ended.
    p.game.phase=6;m.openingProfile.fade=-1;
    m.surge.trigger(energy::Event::OpeningTouchdown,0,p.game);assert(choose().reason==Reason::None&&choose().reaction);
    assert(choose(0).reaction); // Celebration priority covers the first attack frame too.
    m.surge={};p.game.phase=3;assert(choose().reason==Reason::Opening&&choose().continuous);
    p.game.phase=5;assert(choose().reason==Reason::Opening&&choose().continuous);
    p.game.phase=7;assert(choose().reason==Reason::None);
    m.score=100;m.heldFraction=1;assert(choose().intensity==0);
    m.victoryAt=1;assert(choose(2000).intensity==1000);assert(choose(8000).intensity==500);assert(choose(11000).intensity==0);
    c.sustained=0;assert(!choose().active);c.sustained=1;
    assert(!select(c,p,m,123,4,1000,false).active);
    Episode e;Intent wanted=close;e.started(wanted,1000);
    for(uint64_t now=1100;now<21000;now+=100){wanted.updated=now;assert(!e.mustStop(wanted,now));assert(!e.canStart(wanted,now));}
    wanted.updated=21000;assert(e.mustStop(wanted,21000));e.stopped(21000);
    wanted.updated=28999;assert(!e.canStart(wanted,28999));wanted.updated=29000;assert(e.canStart(wanted,29000));
    e.started(wanted,29000);wanted.offense=1;assert(e.mustStop(wanted,29100));wanted.offense=0;
    wanted.reaction=true;assert(e.mustStop(wanted,29100));wanted.reaction=false;
    wanted.active=false;assert(e.mustStop(wanted,29100));wanted.active=true;
    assert(e.mustStop(wanted,29751)); // Lost lease: no stale sound after a suspended presentation update.
    wanted.updated=30000;wanted.epoch=5;assert(e.mustStop(wanted,30000));
    wanted.epoch=4;wanted.match=124;assert(e.mustStop(wanted,30000));
    wanted.match=123;wanted.reason=Reason::None;assert(e.mustStop(wanted,30000));
    assert(!fresh(wanted,123,4,29999)); // Clock anomalies cannot validate a future snapshot.
    Episode opening;wanted=close;wanted.continuous=true;wanted.reason=Reason::Opening;wanted.updated=1000;opening.started(wanted,1000);
    for(uint64_t now=2000;now<301000;now+=1000){wanted.updated=now;wanted.offense=static_cast<int>(now%2);assert(!opening.mustStop(wanted,now));}
    wanted.updated=301000;assert(opening.mustStop(wanted,301000));opening.stopped(301000);
    wanted.updated=310000;assert(!opening.canStart(wanted,310000)); // Bounded whole-opening failsafe cannot restart indefinitely.
    wanted.epoch=5;assert(opening.canStart(wanted,310000));opening.started(wanted,310000);
    wanted.updated=310100;wanted.continuous=false;assert(opening.mustStop(wanted,310100));
    Episode overtime;wanted.reason=Reason::Overtime;wanted.continuous=true;wanted.updated=1000;wanted.reaction=false;overtime.started(wanted,1000);
    for(uint64_t time=2000;time<=1200000;time+=1000){wanted.updated=time;assert(!overtime.mustStop(wanted,time));}
    wanted.active=false;assert(overtime.mustStop(wanted,1200000));
    Episode postgame;wanted.reason=Reason::Eruption;wanted.continuous=true;wanted.active=true;wanted.updated=1000;postgame.started(wanted,1000);
    for(uint64_t time=2000;time<=1200000;time+=1000){wanted.updated=time;assert(!postgame.mustStop(wanted,time)&&!postgame.canStart(wanted,time));}
    wanted.active=false;assert(postgame.mustStop(wanted,1200000)); // Menu revokes the lease; no new voice was stacked.
    std::puts("PASS: contextual/neutral anticipation, full tested opening profile, 20-second episodes, cooldowns, snap/reaction handoff, disable and stale/context rejection.");
}
