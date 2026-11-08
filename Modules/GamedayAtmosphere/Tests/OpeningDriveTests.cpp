#include "../Source/LineupAtmosphere.h"
#include "../Source/LiveReactions.h"
#include "../Source/DirectorShake.h"
#include <cassert>
#include <iostream>
using namespace attendance;
energy::Play fresh(int side,float length=720){energy::Play p;p.game={1,0,0,6,length,length};p.offense=side;p.down=0;p.toGoal=75;p.toFirst=10;return p;}
int main(){
    core::Settings c;
    for(uint64_t positions:{512ull,6060ull,26101ull,56099ull,61888ull,120000ull})for(int side:{0,1})for(float length:{60.f,300.f,720.f,1200.f}){
        float gate=packed::standardMinimum(positions);assert(gate<packed::minimum(positions));
        auto p=fresh(side,length);lineup::Model m;
        auto tick=[&](uint64_t now,float fullness){m.observe(c,p,123,1,now,true,fullness,true,nullptr,positions);};
        tick(1000,gate);assert(!m.openingDriveQualified&&!m.playContextActive()); // Ordinary intro/kickoff unchanged.
        p.game.phase=1;p.down=1;tick(2000,gate-.0001f);assert(!m.playContextActive());
        tick(3000,gate);tick(7000,gate);assert(m.openingDriveQualified&&m.playContextActive()&&!m.packedHouse&&m.qualifiedMinimum==gate);
        auto camera=lineup::cameraAdmission(c,p,gate,true,positions,3,false,m.openingDriveQualified);
        assert(camera.allowed&&!camera.critical);assert(!lineup::cameraAdmission(c,p,gate-.0001f,true,positions,3,false,true).allowed);
        assert(lineup::reactionEligible(p.game,gate,true,m.playContextActive(),m.qualifiedMinimum));
        sustained::Intent base;base.active=true;base.match=123;base.epoch=1;base.updated=7000;
        auto sound=m.merge(base,c,p,7000,gate);assert(sound.lineup&&sound.continuous&&sound.shakeTier==1);
        core::Model gain;gain.update(c,p,7,gate,gate,m.momentWeight());assert(volume::boostPercent(gain.parent)<=(side==1?35.01f:25.01f)); // Hostile away opener uses the existing leverage ceiling.
        p.game.phase=3;p.game.seconds-=5;tick(7100,gate);p.game.phase=4;tick(7200,gate);
        assert(m.openingDriveQualified&&m.playContextActive());
        p.game.phase=5;p.down=2;tick(7300,gate);p.game.phase=1;tick(7400,gate);assert(m.playContextActive());
        // A paused/replayed interval suspends effects, not knowledge of the first possession.
        m.suspend();assert(!m.playContextActive());p.game.phase=5;tick(7500,gate);p.game.phase=1;tick(7600,gate);assert(m.playContextActive());
        p.game.phase=3;tick(7700,gate);p.game.phase=4;p.offense=1-side;p.down=1;tick(7800,gate);
        assert(m.playContextActive()); // Keep the return and whistle eligible.
        p.game.phase=5;tick(7900,gate);p.game.phase=1;tick(8000,gate);
        assert(!m.openingDriveQualified&&!m.playContextActive()&&m.openingDrive.closed);
        p.offense=side;tick(8100,gate);assert(!m.playContextActive()); // Does not recur for each team's first drive.
        // New world/epoch starts a fresh tracker, rather than inheriting the closed drive.
        p=fresh(side,length);m.observe(c,p,124,2,9000,true,gate,true,nullptr,positions);p.game.phase=1;p.down=1;
        m.observe(c,p,124,2,9100,true,gate,true,nullptr,positions);assert(m.openingDriveQualified);
    }
    // Near-sellout plans alone cannot discount small ordinary games.
    for(bool night:{false,true})for(uint64_t seats:{6060ull,12999ull,13000ull,30000ull}){
        auto ordinary=fresh(0);ordinary.game.night=night;
        ordinary.game.startingAttendance=true;ordinary.game.plannedPeak=95000;ordinary.game.plannedCapacity=100000;
        ordinary.game.plannedKickoff=60;ordinary.game.plannedFill=50;ordinary.game.plannedStadium=650;ordinary.game.plannedWeek=1;ordinary.game.plannedYear=2026;
        float normal=nightRequirement(ordinary.game,packed::standardMinimum(seats));
        assert(lineup::openingMinimum(ordinary.game,seats)==(seats<13000?normal:nightRequirement(ordinary.game,.55f)));
        assert(lineup::openingAdmission(false,ordinary.game,seats)==lineup::OpeningAdmission::Inactive);
        assert(lineup::openingAdmission(true,ordinary.game,seats)==(seats<13000?lineup::OpeningAdmission::Normal:lineup::OpeningAdmission::Discounted));
        assert(std::string(lineup::openingStatus(lineup::openingAdmission(true,ordinary.game,seats))).find(seats<13000?"no special opening discount":"with special opening discount")!=std::string::npos);
        ordinary.game.rivalry=true;assert(lineup::openingMinimum(ordinary.game,seats)==nightRequirement(ordinary.game,.55f));
        ordinary.game.rivalry=false;ordinary.game.rank0=4;ordinary.game.rank1=10;
        assert(lineup::openingMinimum(ordinary.game,seats)==nightRequirement(ordinary.game,.55f));
    }
    {auto noPlan=fresh(0);noPlan.game.night=true;assert(lineup::openingAdmission(true,noPlan.game,13000)==lineup::OpeningAdmission::Normal);}
    auto p=fresh(0);lineup::OpeningDrive tracker;tracker.observe(p);p.game.phase=1;p.down=1;assert(tracker.observe(p));
    p.game.phase=3;p.offense=1;assert(tracker.observe(p)); // Turnover return remains live.
    p.game.phase=4;assert(tracker.observe(p));p.game.phase=5;assert(!tracker.observe(p));
    for(int ending=0;ending<5;++ending){
        p=fresh(0);lineup::OpeningDrive t;t.observe(p);p.game.phase=1;p.down=1;assert(t.observe(p));p.game.phase=3;assert(t.observe(p));
        if(ending==0)p.game.homeScore=6;if(ending==1)p.game.awayScore=2;
        if(ending==2){p.specialTeams=true;p.down=4;assert(t.observe(p));p.offense=1;p.down=1;}
        energy::Outcome out;if(ending==3){out.accepted=out.completed=out.takeaway=true;out.sequence=1;}
        if(ending==4)p.game.homeScore=3;
        p.game.phase=4;assert(t.observe(p,&out));p.game.phase=5;assert(!t.observe(p,&out)&&t.closed);
        p.game.phase=1;p.offense=1;assert(!t.observe(p));
    }
    // A fake punt/field goal that converts is still the same offensive possession.
    p=fresh(0);lineup::OpeningDrive fake;fake.observe(p);p.game.phase=1;p.down=1;assert(fake.observe(p));
    p.game.phase=3;p.down=4;p.specialTeams=true;assert(fake.observe(p));p.game.phase=4;p.down=1;assert(fake.observe(p));
    p.game.phase=5;p.specialTeams=false;assert(fake.observe(p)&&!fake.closed);
    // Score/result processing can settle at the next lineup. The runtime keeps
    // the previous play's admission for that result before starting the new call.
    p=fresh(0);lineup::Model score;score.observe(c,p,123,1,1000,true,.55f,true,nullptr,61888);p.game.phase=1;p.down=1;
    score.observe(c,p,123,1,2000,true,.55f,true,nullptr,61888);p.game.phase=3;score.observe(c,p,123,1,3000,true,.55f,true,nullptr,61888);
    bool had=score.playContextActive();float minimum=score.qualifiedMinimum;p.game.phase=5;p.game.homeScore=6;p.down=-1;
    energy::Outcome td;td.accepted=td.completed=true;td.sequence=1;td.homePoints=6;td.side=0;td.event=energy::Event::HomeScore;
    score.observe(c,p,123,1,4000,true,.55f,true,&td,61888);assert(!score.playContextActive());
    assert(lineup::reactionEligible(p.game,.55f,true,had,minimum));
    core::Model reaction;assert(!reaction.react(c,td,4,p.game,had,true,minimum));assert(reaction.resultCheer==1);
    // The newly admitted ordinary opener really reaches the long-catch detector
    // and immediate-tackle camera envelope, below the packed-house threshold.
    p=fresh(0);p.fieldSpot=-25;p.attackDirection=1;lineup::Model layers;live::Model catches;live::Settings cutoffs;cutoffs.minimumCatchYards=20;
    uint64_t clock=1000;layers.observe(c,p,123,1,clock,true,.55f,true,nullptr,61888);
    auto catchTick=[&](int phase,int kind,int ball){
        clock+=100;p.game.phase=phase;p.down=1;bool before=layers.playContextActive();float old=layers.qualifiedMinimum;
        layers.observe(c,p,123,1,clock,true,.55f,true,nullptr,61888);
        bool allowed=lineup::reactionEligible(p.game,.55f,true,before||layers.playContextActive(),(std::min)(old,layers.qualifiedMinimum));
        return catches.observe(cutoffs,p,123,1,clock,allowed,456,kind,ball,-5);
    };
    catchTick(1,1,1);catchTick(3,0,2);catchTick(3,3,4);auto caught=catchTick(4,1,4);
    assert(caught.event==live::Event::LongCatch&&caught.yards==20&&!layers.packedHouse);
    motion::Model camera;motion::Settings movement;motion::Input impact;impact.match=123;impact.epoch=1;impact.phase=4;impact.now=clock;
    impact.allowed=impact.duringAllowed=impact.duringImpact=impact.duringCatch=true;impact.duringPlay=caught.id;
    auto cue=camera.observe(movement,impact);assert(cue.kind==motion::Kind::LivePlay&&cue.impactOnly&&cue.percent==movement.catchImpact);
    // Neither attaching halfway through a drive, resuming a scenario nor Super Sim invents an opening.
    for(int skip=0;skip<3;++skip){p=fresh(0);p.game.phase=1;p.down=1;if(skip==0)p.game.seconds-=5;if(skip==1)p.resumed=true;if(skip==2)p.simulated=true;
        lineup::Model m;m.observe(c,p,123,1,1000,true,.55f,true,nullptr,61888);assert(!m.playContextActive());
        p.resumed=p.simulated=false;m.observe(c,p,123,1,2000,true,.55f,true,nullptr,61888);assert(!m.playContextActive());}
    // A kickoff-return score ends the opening possession before a scrimmage drive.
    p=fresh(0);lineup::OpeningDrive noDrive;noDrive.observe(p);p.game.homeScore=6;p.game.phase=4;assert(!noDrive.observe(p));
    p.game.phase=1;p.down=1;assert(!noDrive.observe(p));
    // A short-quarter opening drive can cross Q1, but not halftime.
    p=fresh(0,60);lineup::OpeningDrive crossing;crossing.observe(p);p.game.phase=1;p.down=1;assert(crossing.observe(p));p.game.quarter=2;assert(crossing.observe(p));p.game.quarter=3;assert(!crossing.observe(p));
    // Disabling opening atmosphere, bad measurements and blowouts keep existing gates.
    for(int disabled=0;disabled<5;++disabled){p=fresh(0);lineup::Model m;auto cfg=c;if(disabled==0)cfg.opening=false;if(disabled==1)cfg.lineups=false;
        m.observe(cfg,p,123,1,1000,true,.55f,true,nullptr,61888);p.game.phase=1;p.down=1;if(disabled==4)p.game.awayScore=24;
        m.observe(cfg,p,123,1,2000,true,.55f,disabled!=2,nullptr,disabled==3?0:61888);assert(!m.openingDriveQualified&&!m.playContextActive());}
    // The pre-existing big-game 50% opening allowance stays intact.
    p=fresh(0);p.game.rank0=6;p.game.rank1=11;lineup::Model big;big.observe(c,p,123,1,1000,true,.50f,true,nullptr,6060);
    p.game.phase=1;p.down=1;big.observe(c,p,123,1,2000,true,.50f,true,nullptr,6060);assert(big.playContextActive()&&big.qualifiedMinimum==.50f&&!big.openingDriveQualified);
    std::cout<<"PASS: first-possession atmosphere at critical-down fullness, gentle motion/result inheritance, 1-20 minute quarters, scoring/punt/turnover ends, return/whistle continuity, no resume/sim/second-drive replay, guards and preserved big-game opening allowance/volume caps.\n";
}
