#include "../Source/LineupAtmosphere.h"
#include "../Source/LiveReactions.h"
#include "../Source/DirectorShake.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
energy::Play before(int down,int offense,bool neutral,int direction){
    energy::Play p;p.game={2,14,14,1,300,720};p.game.neutral=neutral;p.down=down;p.offense=offense;
    p.fieldSpot=-10.f*direction;p.attackDirection=direction;p.toGoal=60;p.toFirst=10;p.resultStamp=10;return p;
}
int main(){
    for(int down:{1,2,3})for(int offense:{0,1})for(bool neutral:{false,true})for(int direction:{-1,1})for(int phase:{4,5,1})for(float yards:{-10.f,-5.f,-4.98f,-4.f,0.f,5.f}){
        energy::Tracker tracker;energy::Settings rules;auto p=before(down,offense,neutral,direction);
        tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);
        p.game.phase=phase;p.game.seconds=295;p.down=down+1;p.fieldSpot+=yards*direction;p.toGoal-=yards;p.toFirst-=yards;p.resultStamp=11;
        auto result=tracker.observe(rules,p);bool expected=(offense==1||neutral)&&yards<=-5;
        assert(result.completed&&!result.sack&&result.fiveYardLoss==expected);
        auto hit=live::completedImpact(result,p.game,true,0,83);assert((hit.id!=0)==expected);
        if(expected){
            assert(hit.event==live::Event::TackleForLoss&&live::cheerType(hit)==59&&hit.side==1-offense&&hit.yards==yards);
            assert(!live::completedImpact(result,p.game,true,result.sequence,59).id); // One impact, not one per update.
            assert(live::completedImpact(result,p.game,true,result.sequence,95).id); // A prior letdown cannot swallow the defensive result.
            assert(!live::completedImpact(result,p.game,false,0,83).id); // Fullness/atmosphere/master switches still gate it.
            motion::Model motion;motion::Settings settings;motion::Input x;x.match=123;x.epoch=1;x.allowed=x.duringAllowed=true;x.duringImpact=true;
            x.phase=phase;x.now=1000;x.duringPlay=hit.id;x.profile=motion::Profile::Loss;auto cue=motion.observe(settings,x);
            assert(cue.impactOnly&&cue.kind==motion::Kind::LivePlay&&cue.percent==8&&cue.until==3700);
            x.duringPlay=0;x.phase=2;x.now=1200;assert(motion::strength(motion.observe(settings,x),1200)==8);
            x.phase=5;x.now=3500;assert(motion::strength(motion.observe(settings,x),3500)>0);
            x.now=3700;assert(motion.observe(settings,x).kind==motion::Kind::None);
            auto profile=postplay::choose(result,p.game);assert(profile.strength>=.72f&&profile.size==3);
        }
        assert(!tracker.observe(rules,p).completed);
    }
    // Native sack proof still admits a shorter sack and missing yardage.
    for(bool missing:{false,true}){energy::Tracker tracker;energy::Settings rules;auto p=before(1,1,false,1);
        if(missing){p.fieldSpot=NAN;p.toGoal=-1;}tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);
        p.game.phase=4;p.down=2;p.game.seconds=295;p.resultStamp=11;p.sack=true;
        if(!missing){p.fieldSpot-=2;p.toGoal+=2;p.toFirst+=2;}auto result=tracker.observe(rules,p);
        assert(result.sack&&!result.fiveYardLoss&&live::completedImpact(result,p.game,true,0,83).event==live::Event::Sack);
    }
    // Missing gain, stale result, no snap, special teams, and an interception
    // cannot be mistaken for a five-yard defensive loss.
    for(int scenario=0;scenario<6;++scenario){energy::Tracker tracker;energy::Settings rules;auto p=before(1,1,false,1);
        if(scenario==0){p.fieldSpot=NAN;p.toGoal=-1;}if(scenario==3)p.specialTeams=true;
        tracker.observe(rules,p);if(scenario!=2){p.game.phase=3;tracker.observe(rules,p);}
        p.game.phase=4;p.game.seconds=295;p.down=2;p.resultStamp=scenario==1?10:11;
        p.fieldSpot-=10;if(scenario!=0)p.toGoal+=10;p.toFirst+=10;if(scenario==4){p.interception=true;p.offense=0;p.down=1;}
        p.simulated=scenario==5;auto result=tracker.observe(rules,p);assert(!result.fiveYardLoss);
    }
    // Loss proof does not depend on a yards-to-first-down field or on the
    // optional selected-player coordinate source. Either verified field source works.
    for(bool goalOnly:{false,true}){energy::Tracker tracker;energy::Settings rules;auto p=before(1,1,false,1);p.toFirst=-1;
        if(goalOnly)p.fieldSpot=NAN;tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);
        p.game.phase=4;p.down=2;p.game.seconds=295;p.resultStamp=11;p.fieldSpot-=5;p.toGoal+=5;
        auto result=tracker.observe(rules,p);assert(result.fiveYardLoss&&result.yards==-5&&live::completedImpact(result,p.game,true,0,83).id);
    }
    // A fourth-down loss needs actual pre-flip proof; never reinterpret the
    // new offense's mirrored field position as the previous team's loss.
    for(bool measured:{false,true}){energy::Tracker tracker;energy::Settings rules;auto p=before(4,1,false,1);
        tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);p.game.seconds=295;p.resultStamp=11;
        if(measured){p.game.phase=4;p.fieldSpot-=5;p.toGoal+=5;p.toFirst+=5;assert(!tracker.observe(rules,p).completed);}
        p.game.phase=5;p.offense=0;p.down=1;p.fieldSpot=15;p.toGoal=35;
        auto result=tracker.observe(rules,p);assert(result.fiveYardLoss==measured);
        if(measured){assert(result.yards==-5&&live::completedImpact(result,p.game,true,0,83).id);}
    }
    std::puts("PASS: 432 normal-down loss/owner/neutral/axis/phase boundaries, exact five-yard fallback, tested defensive cheer and 10% impact, gates/dedup/timeout fade, short/native sacks, stale/unknown/special-team guards and measured fourth-down losses.");
}
