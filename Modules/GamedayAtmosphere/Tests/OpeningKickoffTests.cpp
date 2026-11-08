#include "../Source/LineupAtmosphere.h"
#include "../Source/DirectorShake.h"
#include <cassert>
#include <cmath>
#include <cstdio>
using namespace attendance;
int main(){
    core::Settings settings;energy::Settings events;
    for(int formation:{23,24})for(int side:{0,1}){
        energy::Play p;p.game={1,0,0,6,720,720};p.game.rank0=2;p.game.rank1=7;p.offense=side;p.down=0;
        energy::Tracker tracker;lineup::Model sound;motion::Model camera;motion::Settings movement;
        tracker.primeOpeningIntro(p);
        assert(tracker.openingReliable&&!tracker.seen&&!tracker.serial&&!tracker.played);
        sound.observe(settings,p,123,1,1000,true,.60f,true,nullptr,51498);
        p.game.phase=5;tracker.observe(events,p);sound.observe(settings,p,123,1,1100,true,.60f,true,nullptr,51498);
        assert(tracker.openingReliable&&tracker.openingSeen&&!tracker.openingKickUsed&&!tracker.serial);
        assert(!packed::qualifies(51498,.60f,true)); // Exact reported Georgia opening: not packed, but a big game.
        p.game.phase=1;tracker.observe(events,p);sound.observe(settings,p,123,1,1200,true,.60f,true,nullptr,51498);
        sound.observe(settings,p,123,1,2200,true,.60f,true,nullptr,51498);
        auto admitted=lineup::cameraAdmission(settings,p,.60f,true,51498,formation,tracker.openingReliable&&!tracker.openingKickUsed);
        assert(admitted.allowed&&!admitted.critical&&sound.layeredActive());
        motion::Input input;input.match=123;input.epoch=1;input.now=2200;input.phase=1;
        input.allowed=input.lineup=input.venueEligible=input.openingKickoff=true;input.lineupId=sound.serial;
        input.hostileOpening=movement.hostileOpeningMaximum;
        camera.observe(movement,input);input.now=3200;assert(motion::strength(camera.observe(movement,input),3200)>0);
        p.game.phase=3;tracker.observe(events,p);assert(tracker.openingKickUsed&&tracker.openingKick&&tracker.serial==1);
        input.phase=3;input.lineup=false;input.now=3300;auto carry=camera.observe(movement,input);assert(carry.carryPlay&&!carry.tail);
        input.phase=4;input.now=5000;auto fade=camera.observe(movement,input);assert(fade.tail&&fade.until==7000);
        input.now=7000;assert(camera.observe(movement,input).kind==motion::Kind::None);
        p.game.phase=1;assert(!lineup::cameraAdmission(settings,p,.49f,true,51498,formation,true).allowed);
        assert(!lineup::cameraAdmission(settings,p,.60f,true,51498,formation,false).allowed);
        p.game.rank0=p.game.rank1=0;assert(!lineup::cameraAdmission(settings,p,.60f,true,51498,formation,true).allowed);
    }
    // Actual output at each end and midpoint of the kickoff range, including
    // neutral and occupied-seat scaling. The separate hostile call stays 8-10; kickoff is another25% gentler.
    for(float choice:{0.f,.5f,1.f})for(float crowd:{.8f,1.25f})for(bool neutral:{false,true}){
        motion::Settings movement;motion::Input input;input.match=321;input.epoch=1;
        input.allowed=input.lineup=input.venueEligible=true;input.lineupId=1;input.phase=1;
        input.hostileOpening=movement.hostileOpeningMaximum;input.lineupScale=choice;
        input.venueScale=crowd;input.neutral=neutral;
        for(bool kickoff:{false,true}){
            motion::Model camera;input.openingKickoff=kickoff;input.now=1000;camera.observe(movement,input);
            input.now=3000;auto cue=camera.observe(movement,input);
            float expected=((kickoff?6.f:8.f)+2.f*choice)*crowd*(neutral?.75f:1.f)*(kickoff?.75f:1.f);
            assert(std::abs(motion::strength(cue,input.now)-expected)<.001f);
        }
    }
    for(bool resumed:{false,true}){
        energy::Tracker tracker;energy::Play p;p.game={1,0,0,6,720,720};p.resumed=resumed;p.simulated=!resumed;
        tracker.primeOpeningIntro(p);assert(!tracker.openingReliable&&!tracker.openingSeen&&!tracker.serial);
    }
    {energy::Tracker tracker;energy::Play p;p.game={1,0,0,6,720,720};tracker.primeOpeningIntro(p);
        p.game.phase=5;p.simulated=true;tracker.observe(events,p);assert(!tracker.openingSeen&&!tracker.openingReliable);
        p.game.phase=1;p.simulated=false;tracker.observe(events,p);assert(!tracker.openingReliable);}
    {energy::Tracker tracker;energy::Play p;p.game={1,0,0,5,720,720};tracker.observe(events,p);
        p.game.phase=1;tracker.observe(events,p);assert(!tracker.openingReliable); // Late attach cannot invent an observed intro.
        tracker.reset();p.game.phase=6;tracker.primeOpeningIntro(p);assert(tracker.openingReliable);}
    std::puts("PASS: verified intro-to-selection kickoff proof, gentler 4.5-6 kickoff versus 8-10 hostile-call output, occupied/neutral scaling, kicking/receiving sides, carried shake/whistle fade, sparse/resume/Sim/late-attach guards and new-match reset.");
}
