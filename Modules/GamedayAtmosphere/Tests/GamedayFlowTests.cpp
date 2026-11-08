#include "../Source/LineupAtmosphere.h"
#include "../Source/DirectorShake.h"
#include "../Source/LiveReactions.h"
#include "../Source/ArrivalRules.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace attendance;
energy::Play fixture(){energy::Play p;p.game={1,0,0,1,720,720};p.game.rank0=12;p.game.rank1=19;p.down=1;p.offense=0;p.toGoal=75;return p;}
sustained::Intent observe(lineup::Model& model,energy::Play& p,uint64_t now,float fullness=1){core::Settings settings;model.observe(settings,p,123,1,now,true,fullness,true,nullptr,53913);sustained::Intent out;out.active=true;out.match=123;out.epoch=1;out.updated=now;return model.merge(out,settings,p,now,fullness);}
int main(){
    auto p=fixture();lineup::Model flow;
    auto first=observe(flow,p,1000);assert(first.cheers&&first.lineupCheer&&flow.cheerMoment);
    p.game.phase=3;observe(flow,p,2000);p.game.phase=5;observe(flow,p,3000);p.game.phase=1;p.game.seconds=650;
    observe(flow,p,4000);auto ordinary=observe(flow,p,5000);assert(!ordinary.cheers&&!ordinary.lineupCheer&&!flow.cheerMoment&&ordinary.continuous);
    assert(flow.shakeChoice>=0&&flow.shakeChoice<=1);float choice=flow.shakeChoice;
    observe(flow,p,5100);assert(flow.shakeChoice==choice);
    p.game.phase=5;observe(flow,p,6000);p.game.phase=1;p.toGoal=10;
    auto goal=observe(flow,p,7000);assert(goal.cheers&&flow.cheerMoment);
    p.game.phase=3;assert(!observe(flow,p,7100).cheers);
    // Attaching midgame cannot invent a first-offensive-play cheer.
    auto mid=fixture();mid.game.seconds=300;lineup::Model attached;observe(attached,mid,1000);assert(!attached.cheerMoment);
    // A new home drive that can win gets one immediate burst, not every call.
    auto late=fixture();late.game={4,20,23,1,150,720};late.offense=1;lineup::Model drive;observe(drive,late,1000);
    late.game.phase=5;observe(drive,late,2000);late.offense=0;late.game.phase=1;
    auto winning=observe(drive,late,3000);assert(winning.cheers&&drive.cheerMoment);
    late.game.phase=3;observe(drive,late,4000);late.game.phase=5;observe(drive,late,5000);late.game.phase=1;
    observe(drive,late,6000);assert(!drive.cheerMoment);
    // Below 50%, even a big game's opening, goal line and score stay native.
    for(float f:{.0f,.2f,.499f}){auto small=fixture();small.toGoal=2;lineup::Model quiet;auto out=observe(quiet,small,1000,f);
        assert(!quiet.playContextActive()&&!out.cheers&&!out.continuous&&!lineup::cameraVenue(core::Settings{},small.game,f,true,53913));
        assert(!lineup::reactionEligible(small.game,f,true,true,.60f));}
    assert(lineup::reactionEligible(p.game,.60f,true,true));assert(!lineup::reactionEligible(p.game,1,true,false));
    core::Model sound;core::Settings settings;energy::Outcome score;score.accepted=score.completed=true;score.sequence=1;score.side=0;score.event=energy::Event::LastSecondLead;
    auto final=fixture();final.game={4,24,23,4,2,720};
    assert(!sound.react(settings,score,10,final.game,false,false));sound.update(settings,final,10,.2f,.2f,0,false);assert(!sound.erupting&&sound.resultWeight==0);
    assert(sound.react(settings,score,11,final.game,true,true));sound.update(settings,final,12,.8f,.8f);assert(sound.erupting);
    sound.update(settings,final,13,.49f,.49f);assert(!sound.erupting&&sound.resultWeight==0&&sound.intensity==0);
    // Every normal lineup has one stable random 2.5..5% choice; special cheers use 4..6%.
    motion::Settings camera;assert(camera.ordinary==5&&camera.critical==10&&camera.lineupCheer==6&&camera.touchdownMs==11000);
    for(int n=0;n<4;++n){motion::Model m;motion::Input x;x.match=123;x.epoch=1;x.allowed=x.lineup=x.venueEligible=true;x.lineupId=1;x.phase=1;x.now=1000;x.lineupScale=n/3.f;
        m.observe(camera,x);x.now=2000;assert(std::fabs(motion::strength(m.observe(camera,x),2000)-(2.5f+2.5f*n/3.f))<.001f);
        x.lineupCheer=true;x.now=2100;assert(std::fabs(motion::strength(m.observe(camera,x),2100)-(2.5f+2.5f*n/3.f))<.001f);
        x.now=2300;assert(std::fabs(motion::strength(m.observe(camera,x),2300)-(4+2*n/3.f))<.001f); // Smooth transition to a cheer's stronger target.
    }
    // Defensive impacts can arrive at postplay, without sustained live shake.
    motion::Model impact;motion::Input x;x.match=123;x.epoch=1;x.allowed=true;x.phase=5;x.now=10000;x.duringPlay=1;x.duringImpact=true;
    auto hit=impact.observe(camera,x);assert(hit.kind==motion::Kind::LivePlay&&hit.impactOnly);x.now=10200;assert(motion::strength(impact.observe(camera,x),x.now)>0);
    x.now=16400;assert(impact.observe(camera,x).kind==motion::Kind::None);
    // Native confirmed flags, not an arbitrary loss, select sack/strip accents.
    for(bool strip:{false,true}){energy::Tracker tracker;energy::Settings c;auto play=fixture();play.offense=1;play.game.phase=1;play.toFirst=10;play.resultStamp=1;
        tracker.observe(c,play);play.game.phase=3;tracker.observe(c,play);play.game.phase=4;play.down=2;play.toFirst=15;play.resultStamp=2;play.sack=!strip;play.forcedFumble=play.fumble=strip;
        auto result=tracker.observe(c,play);assert(result.completed&&result.side==0&&(strip?result.forcedFumble:result.sack));
        assert(core::resultReaction(result)==59);assert(!tracker.observe(c,play).completed);
    }
    {energy::Tracker tracker;energy::Settings c;auto play=fixture();play.offense=1;play.resultStamp=1;
        tracker.observe(c,play);play.game.phase=3;tracker.observe(c,play);play.game.phase=4;play.resultStamp=2;play.interception=true;play.fumble=play.forcedFumble=true;play.down=2;
        auto ambiguous=tracker.observe(c,play);assert(ambiguous.completed&&!ambiguous.forcedFumble); // No guessed team after a double turnover.
    }
    // A fourth-down stop inside the five requires confirmed possession change.
    for(int down:{3,4})for(float distance:{2.f,5.f,6.f}){
        energy::Tracker tracker;energy::Settings c;auto play=fixture();play.offense=1;play.down=down;play.toGoal=distance;play.resultStamp=1;
        tracker.observe(c,play);play.game.phase=3;tracker.observe(c,play);
        play.game.phase=4;play.resultStamp=2;auto uncertain=tracker.observe(c,play);assert(!uncertain.goalLineStop);
        play.game.phase=5;play.offense=0;play.down=1;
        auto stop=tracker.observe(c,play);assert(stop.completed&&stop.goalLineStop==(down==4&&distance<=5));
        if(stop.goalLineStop){assert(stop.side==0&&stop.event==energy::Event::GoalLine&&core::resultReaction(stop)==59);assert(!tracker.observe(c,play).goalLineStop);}
    }
    // Punts and kickoffs count from the return catch, in either world direction.
    for(int type:{22,23,24})for(int flight:{-1,1}){live::Returns returns;auto play=fixture();play.offense=1;play.down=type==22?4:0;
        uint64_t time=1000;auto tick=[&](int phase,float z,bool held,int side){play.game.phase=phase;play.offense=side;time+=100;return returns.observe(play,123,1,time,true,type,z,held);};
        assert(!tick(1,0,true,1).id);assert(!tick(3,0,true,1).id);assert(!tick(3,40.f*flight,false,1).id);
        assert(!tick(3,40.f*flight,true,0).id);
        int peak=type==22?40:35;for(int gain=1;gain<peak;++gain)assert(!tick(3,(40.f-gain)*flight,true,0).id);
        auto longReturn=tick(3,(40.f-peak)*flight,true,0);assert(longReturn.yards==peak&&longReturn.event==(type==22?live::Event::PuntReturn:live::Event::KickReturn));
        assert(!tick(3,-1.f*flight,true,0).id);
    }
    {live::Returns returns;auto play=fixture();play.down=0;play.offense=1;
        returns.observe(play,123,1,1000,true,23,0,true);play.game.phase=3;returns.observe(play,123,1,1100,true,23,0,true);
        play.offense=0;assert(!returns.observe(play,123,1,1200,true,23,-40,true).id); // No observed kick flight.
    }
    // Temporary absences use 20..30% of those left and complete both quarter-length-scaled ramps.
    for(int minutes=1;minutes<=20;++minutes){float q=minutes*60.f;State s{2,0,21,5,q/4,q};arrivals::Halftime half;
        int permanent=20;assert(half.observe(s,permanent)==permanent);int prior=permanent;
        for(int n=1;n<=20;++n){s.seconds=q/4*(1-n/20.f);int missing=half.observe(s,permanent);assert(missing>=prior);prior=missing;}
        int absent=prior-permanent;assert(absent>=16&&absent<=24);
        s.quarter=3;for(int n=0;n<=20;++n){s.seconds=q-q/4*n/20.f;int missing=half.observe(s,permanent);if(n<20){assert(missing<=prior);prior=missing;}else assert(missing==0&&half.done);}
    }
    std::cout<<"PASS: sparse immediate lineup cheers, stable gentle random motion, hard measured-fullness gates, longer TD profile, native defensive impacts, catch-based 30-to-40-yard return buildup and gradual quarter-scaled halftime absences.\n";
}
