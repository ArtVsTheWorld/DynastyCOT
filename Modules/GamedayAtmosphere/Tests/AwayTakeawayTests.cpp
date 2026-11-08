#include "../Source/LiveReactions.h"
#include "../Source/LiveFumble.h"
#include "../Source/DirectorShake.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    for(bool neutral:{false,true})for(int side:{0,1}){
        live::Model detector;live::Settings settings;motion::Model camera;motion::Settings shake;
        energy::Play p;p.game={4,37,45,1,210,720};p.game.neutral=neutral;p.down=1;p.offense=side;p.fieldSpot=-20;p.attackDirection=1;
        auto tick=[&](uint64_t now,int phase,int kind,int ball,bool held){p.game.phase=phase;return detector.observe(settings,p,123,1,now,true,456,kind,ball,0,-20,held);};
        tick(1000,1,0,2,true);tick(1100,3,0,2,true);tick(1200,3,3,1,false);
        // No change to the offense label yet. A verified interception must
        // still clear a home-positive buildup without its old 900ms tail.
        auto pick=tick(1300,3,2,3,true);
        bool suppress=side==0&&!neutral;
        assert(detector.homePossessionLost()==suppress);
        assert(bool(pick.id)==!suppress);
        motion::Input x;x.match=123;x.epoch=1;x.now=1000;x.phase=1;x.allowed=x.lineup=x.venueEligible=true;x.lineupId=1;
        camera.observe(shake,x);x.now=1200;auto before=camera.observe(shake,x);assert(motion::strength(before,1200)>0);
        x.now=1300;x.phase=3;x.allowed=!detector.homePossessionLost();x.lineup=false;
        if(pick.id){x.duringPlay=pick.id;x.duringAllowed=true;}
        auto after=camera.observe(shake,x);
        if(suppress)assert(after.kind==motion::Kind::None&&motion::strength(after,1300)==0);
        else assert(after.kind==motion::Kind::LivePlay);
        p.offense=1-side;tick(1400,4,2,3,true);assert(detector.homePossessionLost()==suppress);
        tick(1500,1,2,2,true);assert(!detector.homePossessionLost());
    }
    // Run/catch motion stops at fresh home loose-ball proof, even on a short
    // play with no active long-return sound model. Own pickups stay quiet;
    // an away defensive recovery cannot clear the home possession-loss latch.
    {live::Model detector;live::Settings settings;energy::Play p;p.game={2,7,7,1,300,720};p.down=1;p.offense=0;p.fieldSpot=-20;p.attackDirection=1;
        detector.observe(settings,p,123,1,1000,true,456,0,9,0,-20,true);p.game.phase=3;
        detector.observe(settings,p,123,1,1100,true,456,0,9,0,-20,true);p.fumble=true;
        detector.observe(settings,p,123,1,1200,true,456,0,5,0,-20,false);assert(detector.homePossessionLost());
        p.fumble=false;detector.observe(settings,p,123,1,1300,true,456,0,9,0,-20,true);assert(detector.homePossessionLost());
        detector.defensiveHandoff({live::Event::FumbleRecovery,1,1,0});assert(detector.homePossessionLost());
        detector.defensiveHandoff({live::Event::ForcedFumble,2,0,0});assert(!detector.homePossessionLost());
    }
    // An official away takeaway still cancels motion if live samples missed
    // the catch; pause/world/disabled paths clear the ownership latch safely.
    {live::Model detector;live::Settings settings;energy::Play p;p.game={4,37,45,4,13,720};p.offense=1;p.down=1;
        energy::Outcome r;r.accepted=r.completed=r.takeaway=true;r.sequence=1;r.side=1;
        detector.observe(settings,p,123,1,1000,true,456,2,3,0,NAN,true,false,&r);assert(detector.homePossessionLost());
        detector.observe(settings,p,123,1,1100,false,456,2,3,0,NAN,true);assert(!detector.homePossessionLost());
        detector.observe(settings,p,123,2,1200,true,456,2,3,0,NAN,true);assert(!detector.homePossessionLost());
    }
    // A delayed native forced-fumble event can arrive after the away recovery.
    // It produces no positive home reaction but still retires home motion.
    {live::Fumbles fumbles;energy::Play p;p.game={2,7,7,1,300,720};p.offense=0;p.down=1;
        live::ForcedEvent native{777,0,true};
        fumbles.observe(p,123,1,1000,true,native,0,9,true);p.game.phase=3;
        fumbles.observe(p,123,1,1100,true,native,0,9,true);native.type=32;p.offense=1;
        assert(!fumbles.observe(p,123,1,1200,true,native,0,9,true).id);assert(fumbles.homeBallLost());
        p.game.phase=1;fumbles.observe(p,123,1,1300,true,native,0,9,true);assert(!fumbles.homeBallLost());
    }
    // Stale interception metadata at the line is not a new takeaway.
    {live::Model detector;live::Settings settings;energy::Play p;p.game={2,7,7,1,300,720};p.offense=0;p.down=1;p.fieldSpot=-20;p.attackDirection=1;
        detector.observe(settings,p,123,1,1000,true,456,2,3,0,-20,true);p.game.phase=3;
        detector.observe(settings,p,123,1,1100,true,456,2,3,0,-20,true);assert(!detector.homePossessionLost());
    }
    std::puts("PASS: away interception cancels home buildup before possession-label handoff; no old tail; genuine home/neutral picks preserved; loose-ball/official-takeaway cleanup, recovery isolation, defensive rearm and stale/world/pause rejection.");
}
