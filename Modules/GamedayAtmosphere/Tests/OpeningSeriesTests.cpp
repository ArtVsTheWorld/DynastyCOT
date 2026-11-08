#include "../Source/OpeningKickoffAtmosphere.h"
#include "../Source/HostileOpeningResult.h"
#include "../Source/DirectorShake.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
namespace {
struct Series {
    core::Settings c;lineup::Model layers;openingresult::Model results;
    energy::Play p,snap;uint64_t now=1000,sequence=0;
    Series(){p.game={1,0,0,6,720,720};p.down=0;p.offense=1;p.selectedType=23;p.specialTeams=true;p.fieldSpot=-25;p.toFirst=10;p.attackDirection=1;tick();}
    bool tick(const energy::Outcome* outcome=nullptr,int kind=0,bool pass=false){
        now+=100;layers.observe(c,p,123,1,now,true,.9f,true,outcome,34153);
        return results.observe(p,snap,outcome?*outcome:energy::Outcome{},123,1,sequence,now,layers.hostileOpening,layers.hostileSeriesActive,kind,pass,layers.openingDrive.awayOpeningSeries());
    }
    void call(int down,int type=103){p.down=down;p.selectedType=type;p.specialTeams=type==22;p.game.phase=5;tick();p.game.phase=1;tick();}
    void start(){snap=p;++sequence;p.game.phase=3;tick();}
    bool finish(float gain,bool firstDown=false,int kind=0,bool pass=false){
        p.game.phase=4;p.game.seconds-=5;p.fieldSpot+=gain;p.toFirst=(std::max)(0.f,p.toFirst-gain);
        energy::Outcome out;out.accepted=out.completed=true;out.sequence=sequence;out.yards=gain;out.firstDown=firstDown;
        return tick(&out,kind,pass);
    }
};
}
int main(){
    // Wire the same intro/tracker/lineup/kickoff/admission/motion sequence as
    // production. All protected native events qualify without ranked teams.
    unsigned protectedEvents=0;
    for(int event=0;event<1023;++event)if(showcase::event(event)){
        ++protectedEvents;energy::Play p;p.game={1,0,0,6,720,720};p.game.rank0=66;p.game.rank1=73;
        p.game.rivalry=true;p.game.rivalryEvent=event;p.game.team0=1500;p.game.team1=1501;
        p.down=0;p.offense=0;p.selectedType=23;p.specialTeams=true;
        assert(!lineup::bigGame(p.game)&&lineup::kickoffMatchup(p.game));
        core::Settings c;energy::Settings e;energy::Tracker tracker;lineup::Model layers;kickoff::Model audio;motion::Model camera;motion::Settings movement;
        tracker.primeOpeningIntro(p);layers.observe(c,p,123,1,1000,true,.77f,true,nullptr,34153);
        uint64_t now=1000;bool heardStart=false;
        for(int phase:{5,1,1}){now+=500;p.game.phase=phase;tracker.observe(e,p);layers.observe(c,p,123,1,now,true,.77f,true,nullptr,34153);
            bool proof=c.opening&&lineup::kickoffMatchup(p.game)&&tracker.openingReliable&&!tracker.openingKickUsed;
            heardStart=audio.observe(p,123,1,now,true,proof,p.selectedType,.77f)||heardStart;
            auto admitted=lineup::cameraAdmission(c,p,.77f,true,34153,23,proof,layers.openingDriveQualified);assert(admitted.allowed&&layers.shakeTier>0);
            motion::Input input;input.match=123;input.epoch=1;input.now=now;input.phase=phase;input.allowed=input.lineup=input.venueEligible=true;
            input.openingKickoff=proof;input.lineupId=layers.cameraLineupId();input.lineupScale=layers.cameraChoice();input.hostileOpening=movement.hostileOpeningMaximum;
            auto cue=camera.observe(movement,input);if(now==2500)assert(motion::strength(cue,now)>0&&cue.carryPlay);
        }
        assert(heardStart);
        auto unknown=p.game;unknown.rivalryEvent=1000;assert(!lineup::kickoffMatchup(unknown));
        unknown=p.game;unknown.rivalry=false;assert(!lineup::kickoffMatchup(unknown));
        p.resumed=true;assert(!audio.observe(p,456,2,now+100,true,true,23,.77f));
        assert(!lineup::cameraAdmission(c,p,.49f,true,34153,23,true).allowed);
    }
    assert(protectedEvents==24);
    Series stops;stops.call(1);assert(stops.layers.hostileOpening&&stops.layers.hostileSeriesActive);
    stops.start();assert(stops.layers.hostileSeriesActive);assert(stops.finish(2));auto cheer=stops.results.cheer;
    stops.call(2);assert(stops.layers.hostileOpening);stops.start();assert(stops.finish(0,false,4,true));assert(stops.results.cheer!=cheer);
    stops.call(3);assert(stops.layers.hostileOpening);stops.start();assert(stops.finish(1));
    stops.call(4,22);assert(stops.layers.hostileOpening);stops.start();assert(!stops.layers.hostileSeriesActive&&stops.layers.openingDrive.awaySeriesClosed);
    stops.p.game.phase=4;stops.tick();stops.p.offense=0;stops.call(1);assert(!stops.layers.hostileOpening);
    // More than two yards gets no special defensive cheer but does not end a
    // series that has not earned a first down.
    Series medium;medium.call(1);medium.start();assert(!medium.finish(5));medium.call(2);assert(medium.layers.hostileOpening);medium.start();assert(medium.finish(2));
    Series unclassified;unclassified.call(4,106);unclassified.p.specialTeams=true;unclassified.tick();
    assert(unclassified.layers.hostileSeriesActive);unclassified.start();assert(unclassified.layers.openingDrive.awayOpeningSeries());
    assert(!unclassified.finish(10,true)&&!unclassified.layers.hostileSeriesActive);
    Series toggled;toggled.call(1);toggled.start();toggled.c.opening=false;assert(!toggled.finish(0,false,4,true));
    toggled.c.opening=true;toggled.call(2);toggled.start();assert(toggled.finish(0,false,4,true));
    Series conversion;conversion.call(1);conversion.start();assert(!conversion.finish(10,true));assert(!conversion.layers.hostileSeriesActive);
    conversion.p.toFirst=10;conversion.call(1);assert(!conversion.layers.hostileOpening);
    // Native next-lineup down/field proof closes it even when the official
    // first-down result was missing/late. Selection/penalty transitions alone
    // cannot supply that fallback.
    Series delayed;delayed.call(1);delayed.start();delayed.finish(5);
    delayed.p.game.phase=5;delayed.p.down=1;delayed.tick();assert(delayed.layers.openingDrive.awayOpeningSeries());
    delayed.p.toFirst=10;delayed.p.game.phase=1;delayed.tick();assert(!delayed.layers.hostileOpening);
    Series penalty;penalty.call(1);penalty.start();penalty.finish(0,false,4,true);penalty.call(2);
    penalty.p.game.phase=2;penalty.tick();assert(penalty.layers.openingDrive.awayOpeningSeries());penalty.p.game.phase=1;penalty.tick();assert(penalty.layers.hostileOpening);
    // Pauses stop output, not knowledge of the opening series.
    penalty.layers.suspend();penalty.results.suspend();penalty.p.game.phase=5;penalty.tick();penalty.p.game.phase=1;penalty.tick();assert(penalty.layers.hostileOpening);
    // Continuous loud bed across the live play and between snaps, without a
    // scheduled cheer at every away lineup. It can outlast one 40-second loop.
    Series bed;bed.call(1);bed.start();
    for(int i=0;i<500;++i){bed.tick();sustained::Intent base;base.active=true;base.match=123;base.epoch=1;base.updated=bed.now;
        auto sound=bed.layers.merge(base,bed.c,bed.p,bed.now,.9f);assert(sound.hostileOpening&&sound.continuous&&sound.size==3&&!sound.cheers&&!sound.lineupCheer);}
    for(int end=0;end<5;++end){Series x;x.call(1);x.start();x.p.game.phase=4;energy::Outcome result;result.accepted=result.completed=true;result.sequence=x.sequence;
        if(end==0)result.takeaway=true;if(end==1){result.awayPoints=6;x.p.game.awayScore=6;}if(end==2){result.safety=true;result.homePoints=2;x.p.game.homeScore=2;}
        if(end==3)x.p.offense=0;if(end==4){x.p.game.phase=3;x.p.selectedType=21;x.p.specialTeams=true;}
        assert(!x.tick(&result)&&!x.layers.hostileSeriesActive);
    }
    Series crossing;crossing.p.game.seconds=10;crossing.call(1);crossing.start();crossing.finish(0,false,4,true);
    crossing.p.game.quarter=2;crossing.p.game.seconds=720;crossing.call(2);assert(crossing.layers.hostileOpening);crossing.start();assert(crossing.finish(1));
    crossing.p.game.quarter=3;crossing.call(3);assert(!crossing.layers.hostileOpening);
    for(int disabled=0;disabled<4;++disabled){Series x;x.call(1);if(disabled==0)x.p.resumed=true;if(disabled==1)x.p.simulated=true;if(disabled==2)x.c.opening=false;if(disabled==3)x.p.game.neutral=true;
        x.tick();assert(!x.layers.hostileSeriesActive);
    }
    std::puts("PASS: all24 native-event showcase kickoff admissions through actual tracker/lineup/motion handoff; continuous first-away-series bed and repeated largest defensive cheers; first-down/punt/score/possession cleanup, missing-result fallback, penalty/pause/quarter continuity and neutral/resume/sim guards");
}
