#include "../Source/ArrivalRules.h"
#include "../Source/DepartureModel.h"
#include "../Source/HailMaryPass.h"
#include "../Source/CoreAtmosphere.h"
#include "../Source/DirectorShake.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    for(float q:{60.f,300.f,720.f,1200.f})for(bool ranked:{false,true}){
        State s{2,14,14,5,q*.25f,q};if(ranked){s.rank0=6;s.rank1=11;}
        arrivals::Halftime patient,ordinary;auto plain=s;plain.rank0=plain.rank1=0;
        patient.observe(s,5,100,!ranked);ordinary.observe(plain,5);
        s.seconds=plain.seconds=q*.125f;
        int early=patient.observe(s,5,100,!ranked),normal=ordinary.observe(plain,5);
        assert(early-5<=std::lround((normal-5)*.25f)+1&&patient.waitingForBreak);
        s.seconds=plain.seconds=0;int peak=patient.observe(s,5,100,!ranked);assert(patient.releaseAtBreak&&peak==ordinary.observe(plain,5));
        departures::Model pacing;pacing.beginWindow(s,4);pacing.anchorVisible(5,early);assert(pacing.paceTemporary(5,peak)<=early+4);
        pacing.halftimeVisible();assert(pacing.paceTemporary(5,peak)==peak); // Only the actual halftime break releases the held-back group.
        s.quarter=3;s.seconds=q;assert(patient.observe(s,5,100,!ranked)==peak);
        s.seconds=q*.875f;int returned=patient.observe(s,5,100,!ranked);assert(returned<peak&&returned>5&&!patient.releaseAtBreak);
        s.seconds=q*.75f;assert(patient.observe(s,5,100,!ranked)==0&&patient.done);
        patient.reset();s.quarter=2;s.seconds=q*.02f;patient.observe(s,5,100,!ranked);
        s.quarter=3;s.seconds=q;assert(patient.observe(s,5,100,!ranked)==peak&&patient.releaseAtBreak); // Fast Sim skips the zero-clock frame.
    }
    for(int quarter:{1,2,4})for(int down:{3,4})for(int margin:{0,8,9})for(float seconds:{119.f,120.f,121.f})for(bool neutral:{false,true}){
        energy::Play p;p.game={quarter,14+margin,14,1,seconds,720};p.game.neutral=neutral;p.offense=neutral?0:1;p.down=down;p.toFirst=5;p.toGoal=40;p.fieldSpot=-20;p.attackDirection=1;p.resultStamp=1;
        energy::Tracker tracker;energy::Settings settings;tracker.observe(settings,p);p.game.phase=3;tracker.observe(settings,p);
        p.game.phase=4;p.resultStamp=2;p.game.seconds-=1;p.down=down==3?4:1;if(down==4)p.offense=1-p.offense;p.fieldSpot=-19;p.toFirst=4;
        auto stop=tracker.observe(settings,p);bool expected=quarter==4&&margin<=8&&seconds<=120;
        assert(stop.completed&&stop.highLeverageStop==expected);
        if(expected){auto profile=postplay::choose(stop,p.game);assert(profile.layered&&profile.strength>=.95f&&core::resultReaction(stop)==59);}
    }
    energy::Play snap;snap.game={2,14,14,1,10,720};snap.down=1;snap.offense=0;snap.toGoal=50;
    energy::Outcome score;score.accepted=score.completed=true;score.sequence=1;score.side=0;score.homePoints=6;score.event=energy::Event::HomeScore;
    hailmary::Pass pass;pass.observe(1,49);assert(pass.scored(score,snap));score.hailMary=true;
    State end=snap.game;end.phase=4;end.seconds=0;end.homeScore+=6;core::Settings c;core::Model eruption;
    assert(eruption.react(c,score,10,end,true,true,.6f)&&eruption.event==energy::Event::HailMaryTouchdown);
    assert(!eruption.react(c,score,10.1,end,true,true,.6f));
    core::Model noLayers;assert(!noLayers.react(c,score,10,end,false));
    pass.observe(2,2);score.sequence=2;assert(!pass.scored(score,snap)); // A long screen/YAC touchdown is not a Hail Mary.
    pass.observe(3,49);score.sequence=3;snap.game.seconds=16;assert(!pass.scored(score,snap));snap.game.seconds=10;snap.game.quarter=1;assert(!pass.scored(score,snap));
    snap.game.quarter=4;snap.offense=1;score.side=1;score.homePoints=0;score.awayPoints=6;assert(pass.scored(score,snap));
    core::Model away;end.quarter=4;assert(!away.react(c,score,10,end,true));end.neutral=true;assert(away.react(c,score,10,end,true));
    score.event=energy::Event::LastSecondLead;core::Model winner;assert(winner.react(c,score,10,end,true)&&winner.event==energy::Event::LastSecondLead); // Existing winning eruption wins priority.
    motion::Model camera;motion::Settings motionSettings;motionSettings.live=10;motionSettings.catchImpact=8;motion::Input x;x.match=123;x.epoch=1;x.allowed=x.duringAllowed=true;x.phase=4;x.now=10000;x.duringPlay=1;x.duringImpact=true;
    camera.observe(motionSettings,x);x.now+=200;auto cue=camera.observe(motionSettings,x);assert(motion::strength(cue,x.now)==10&&cue.impactOnly);
    std::puts("PASS: close big/packed halftime patience, ordinary peak/Q3 return timing, skipped-clock Sim, temporary pacing at the break, Q4-only one-score 120-second stops, layered defensive profile/impact, verified Hail Mary touchdowns and existing eruption priority.");
}
