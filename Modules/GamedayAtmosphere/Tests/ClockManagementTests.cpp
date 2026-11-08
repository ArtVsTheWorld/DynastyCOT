#include "../Source/LineupAtmosphere.h"
#include "../Source/LiveReactions.h"
#include "../Source/ContextReactions.h"
#include "../Source/DirectorShake.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
energy::Play fixture(int type,int side,int down,bool neutral){
    energy::Play p;p.game={4,37,45,1,15,720};p.game.rank0=23;p.game.rank1=11;p.game.neutral=neutral;
    p.offense=side;p.down=down;p.toFirst=10;p.toGoal=50;p.fieldSpot=0;p.attackDirection=1;
    p.selectedType=type;p.specialTeams=type>0&&!lineup::offensivePlay(type);p.resultStamp=1;p.homeTimeouts=2;p.awayTimeouts=0;
    return p;
}
int main(){
    unsigned cases=0;
    for(int type:{37,38})for(int side:{0,1})for(int down:{1,2,3,4})for(bool neutral:{false,true})for(int phase:{1,4,5,7}){
        energy::Settings rules;energy::Tracker tracker;core::Settings config;
        auto p=fixture(type,side,down,neutral),snap=p;
        assert(energy::clockManagement(p));
        assert(!lineup::cameraAdmission(config,p,1,true,49156,type).allowed);
        p.game.phase=5;assert(!lineup::cameraAdmission(config,p,1,true,49156,type).allowed);p.game.phase=1;
        tracker.observe(rules,p);p.game.phase=3;p.game.seconds=14;tracker.observe(rules,p);
        p.game.phase=phase;p.game.seconds=13;p.resultStamp=2;p.sack=true;
        p.fieldSpot=-1;p.toFirst=11;p.down=down==4?1:down+1;
        if(down==4)p.offense=1-side;
        // The next call can replace the selected type before the old result.
        p.selectedType=3;p.specialTeams=false;
        auto result=tracker.observe(rules,p);
        assert(result.accepted&&result.completed&&result.sequence==1);
        assert(result.event==energy::Event::None&&!result.sack&&!result.fiveYardLoss&&!result.highLeverageStop&&!result.goalLineStop);
        auto contextual=contextreaction::apply(result,tracker.start,p,1,49156);
        assert(!contextual.thirdGoalStop&&!contextual.contextEruption&&!contextual.contextClincher);
        assert(!live::completedImpact(contextual,p.game,true,0,59).id);
        // Frozen call owns live/postplay gates, but a fresh normal lineup rearms.
        p.game.phase=4;assert(energy::clockManagementContext(p,tracker.start,tracker.serial));
        p.game.phase=1;assert(!energy::clockManagementContext(p,tracker.start,tracker.serial));
        ++cases;
    }
    // Reproduce the last snap: Ole Miss knee at 13 seconds, native sack flag.
    // Earlier buildup must clear and the bogus result must not add an impact.
    {auto p=fixture(3,1,1,false);core::Settings config;motion::Settings settings;motion::Model camera;
        motion::Input x;x.match=123;x.epoch=1;x.now=1000;x.phase=1;x.allowed=x.lineup=x.venueEligible=true;x.lineupId=1;
        camera.observe(settings,x);x.now=2000;assert(motion::strength(camera.observe(settings,x),x.now)>0);
        p.selectedType=37;x.now=2100;x.allowed=!energy::clockManagementContext(p,p,0);
        assert(camera.observe(settings,x).kind==motion::Kind::None);
        p.game.phase=3;x.phase=3;x.now=2200;assert(camera.observe(settings,x).kind==motion::Kind::None);
        p.game.phase=4;x.phase=4;x.now=2300;assert(camera.observe(settings,x).kind==motion::Kind::None);
        p.game.phase=1;p.selectedType=3;x.phase=1;x.now=2400;x.allowed=true;x.lineupId=2;
        assert(lineup::cameraAdmission(config,p,1,true,49156,3).allowed);
        assert(camera.observe(settings,x).kind==motion::Kind::Ordinary);
    }
    // Same late situation on a real offense/fake-spike play still allows a sack.
    for(int type:{3,107,-1}){
        auto p=fixture(type,1,1,false);energy::Settings rules;energy::Tracker tracker;
        tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);
        p.game.phase=4;p.game.seconds=13;p.resultStamp=2;p.sack=true;p.down=2;p.fieldSpot=-1;
        auto result=tracker.observe(rules,p);assert(result.sack&&result.event==energy::Event::Sack);
        assert(live::completedImpact(result,p.game,true,0,59).event==live::Event::Sack);
    }
    // A kick with a stale sack predicate cannot masquerade as a real sack.
    for(int type:{21,22,105,106,154,155}){
        auto p=fixture(type,1,1,false);energy::Settings rules;energy::Tracker tracker;
        tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);p.game.phase=4;p.down=2;p.sack=true;p.resultStamp=2;p.fieldSpot=-1;
        assert(!tracker.observe(rules,p).sack);
    }
    std::printf("PASS: %u knee/clock-stop result cases, native-sack false positive suppression, no lineup/postplay shake, frozen-call ownership, fresh-play rearming and genuine/fake-spike sack preservation.\n",cases);
}
