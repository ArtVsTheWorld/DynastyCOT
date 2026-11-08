#include "../Source/ContextReactions.h"
#include "../Source/CoreAtmosphere.h"
#include "../Source/LiveReactions.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
struct Test {
    energy::Play snap,after;energy::Outcome r;
    Test(){snap.game={4,24,21,1,35,720};snap.game.rank0=4;snap.game.rank1=7;snap.offense=1;snap.down=4;snap.toGoal=4;
        after=snap;after.game.phase=5;after.offense=0;after.down=1;after.awayTimeouts=1;
        r.accepted=r.completed=true;r.sequence=1;r.side=0;r.event=energy::Event::FourthStop;}
    energy::Outcome run(float fullness=.99f,bool enabled=true){return contextreaction::apply(r,snap,after,fullness,53913,enabled);}
};
int main(){
    {Test t;auto r=t.run();assert(r.goalLineStop&&r.contextEruption&&r.event==energy::Event::GameSealingStop&&r.contextStrength>1.2f);
        core::Model m;core::Settings c;assert(m.react(c,r,1,t.after.game,true,true,.6f));assert(m.event==energy::Event::GameSealingStop);}
    for(int bad=0;bad<10;++bad){Test t;if(bad==0)t.snap.game.rank0=t.snap.game.rank1=50;if(bad==1)t.after.awayTimeouts=3;if(bad==2)t.after.awayTimeouts=-1;if(bad==3)t.after.game.seconds=90;if(bad==4)t.after.offense=1;if(bad==5)t.r.completed=false;if(bad==6)t.r.sequence=0;if(bad==7){t.r.side=1;t.snap.toGoal=20;}if(bad==8)t.after.game.homeScore=21;if(bad==9)t.after.game.homeScore=35;assert(!t.run().contextEruption);}
    {Test t;assert(!t.run(.6f).contextEruption);assert(!t.run(.99f,false).contextEruption);}
    {Test t;t.snap.game.rank0=t.snap.game.rank1=50;t.snap.game.rivalry=true;assert(t.run().contextEruption);}
    // Goal-line stops are normal impacts even outside the premium class.
    {Test t;t.snap.game.rank0=t.snap.game.rank1=50;t.snap.down=3;t.after.down=4;t.after.offense=1;auto r=t.run();assert(r.thirdGoalStop&&!r.contextEruption&&r.contextStrength==1);assert(live::completedImpact(r,t.after.game,true,0,83).event==live::Event::ThirdGoalStop);}
    {Test t;t.snap.down=3;t.after.down=4;t.after.offense=1;t.r.firstDown=true;assert(!t.run().thirdGoalStop);t.r.firstDown=false;t.r.awayPoints=3;assert(!t.run().thirdGoalStop);}
    {Test t;t.snap.offense=0;t.snap.down=2;t.snap.toGoal=35;t.r.firstDown=true;t.r.event=energy::Event::ThirdDownConversion;auto r=t.run();assert(r.contextClincher&&!r.contextEruption&&r.event==energy::Event::GameSealingFirstDown);
        core::Settings c;core::Model m;assert(!m.react(c,r,1,t.after.game,true,true,.6f,0,true));assert(m.resultCheer&&m.resultType==91&&m.resultScoring);}
    {Test t;t.after.game.phase=7;t.after.offense=1;t.after.down=4;t.r.sack=true;assert(t.run().contextEruption);}
    {Test t;t.snap.game.neutral=t.after.game.neutral=true;t.snap.game.homeScore=t.after.game.homeScore=21;t.snap.game.awayScore=t.after.game.awayScore=24;t.snap.offense=0;t.after.offense=1;t.after.homeTimeouts=1;t.r.side=1;assert(t.run().contextEruption);}
    {Test t;t.r.sack=true;t.snap.down=3;t.after.offense=1;t.after.down=4;assert(!t.run().contextEruption);} // No presumed change of possession.
    float prior=1;for(int seconds=180;seconds>=0;--seconds){Test t;t.after.game.seconds=float(seconds);float x=contextreaction::strength(t.snap,t.after.game,true);assert(x>=prior&&x<=1.31f);prior=x;}
    // The sack does not prove a win until Final; confirmation is bounded to
    // this play/world, score and clock-expiring evidence, never a later snap.
    for(int bad=0;bad<7;++bad){Test t;contextreaction::Model model;t.snap.down=3;t.snap.toGoal=20;t.after=t.snap;t.after.game.phase=4;t.after.game.seconds=0;t.after.down=4;t.r.sack=true;t.r.event=energy::Event::Sack;
        assert(!model.observe(t.r,t.snap,t.after,1,123,1,1000,.99f,53913,true).contextEruption);
        t.after.game.phase=7;uint64_t sequence=1,now=2000;uintptr_t match=123;unsigned epoch=1;
        if(bad==1)sequence=2;if(bad==2)match=456;if(bad==3)epoch=2;if(bad==4)now=22000;if(bad==5)t.after.game.awayScore=28;if(bad==6)t.after.simulated=true;
        auto result=model.observe({},t.snap,t.after,sequence,match,epoch,now,.99f,53913,true);assert(result.contextEruption==(bad==0));
        assert(!model.observe({},t.snap,t.after,sequence,match,epoch,now+10,.99f,53913,true).contextEruption);
    }
    // Both normal goal-line stop impacts request the defensive cheer family.
    // A third-down stop stays on the same possession; fourth down flips it.
    for(int down:{3,4}){Test t;t.snap.game.rank0=t.snap.game.rank1=50;t.snap.down=down;
        t.after.down=down==3?4:1;t.after.offense=down==3?1:0;auto result=t.run();
        auto impact=live::completedImpact(result,t.after.game,true,0,83);
        assert(impact.id&&live::cheerType(impact)==59&&core::resultReaction(result)==59);
        core::Model sound;core::Settings settings;assert(!sound.react(settings,result,1,t.after.game,true));
        sound.update(settings,t.after,1.25,1,1);auto wanted=sound.intent(settings,t.after,123,1,1250,true);
        assert(wanted.resultCheer&&wanted.resultType==59&&!wanted.celebration);
    }
    std::puts("PASS: normal third/fourth goal-line stops; packed big/rivalry-only context; clock/possession/timeout sealing proof; no speculative sack eruptions; clinching first-down profile; neutral symmetry and smooth bounded strength.");
}
