#include "../Source/LiveReactions.h"
#include "../Source/DirectorShake.h"
#include "../Source/CoreAtmosphere.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    for(int lead:{-7,0,1,2,3,4,10})for(int phase:{1,4,5})for(int offense:{0,1})for(bool neutral:{false,true}){
        energy::Tracker tracker;energy::Settings rules;energy::Play p;p.game={4,17+lead,17,1,45,720};p.game.neutral=neutral;
        p.down=4;p.offense=offense;p.selectedType=21;p.specialTeams=true;p.resultStamp=10;p.toGoal=25;
        tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);
        p.game.phase=phase;p.offense=1-offense;p.down=1;p.resultStamp=11;p.blockedFieldGoal=true;
        auto result=tracker.observe(rules,p);bool own=offense==1||neutral;
        assert(result.blockedFieldGoal==own);
        auto hit=live::completedImpact(result,p.game,true,0,83);assert(bool(hit.id)==own);
        if(own){assert(hit.event==live::Event::BlockedFieldGoal&&live::cheerType(hit)==59);
            int defenseLead=result.side==0?lead:-lead;bool strong=defenseLead>=0&&defenseLead<=3;
            motion::Settings settings;motion::Model camera;motion::Input x;x.match=123;x.epoch=1;x.allowed=true;x.now=1000;x.phase=phase;x.duringPlay=hit.id;x.duringImpact=true;
            x.blockedFieldGoal=result.sequence;x.gameSavingBlock=strong;auto cue=camera.observe(settings,x);
            assert(cue.kind==motion::Kind::BlockedKick);
            x.now=1300;cue=camera.observe(settings,x);assert(motion::strength(cue,x.now)==(strong?20.f:14.f));
            if(strong){x.now=1500;x.touchdown=result.sequence;assert(camera.observe(settings,x).kind==motion::Kind::Touchdown);}
            assert(!live::completedImpact(result,p.game,true,result.sequence,59).id);
            assert(!live::completedImpact(result,p.game,false,0,83).id);
            core::Model sound;core::Settings c;sound.react(c,result,10,p.game,true,true);assert(sound.resultType==59&&!sound.resultScoring);
        }
        assert(!tracker.observe(rules,p).completed);
    }
    for(int invalid=0;invalid<5;++invalid){energy::Tracker tracker;energy::Settings rules;energy::Play p;
        p.game={2,7,7,1,300,720};p.down=4;p.offense=1;p.selectedType=invalid==0?22:21;p.specialTeams=true;p.resultStamp=10;
        tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);p.game.phase=4;p.offense=0;p.down=1;p.resultStamp=invalid==1?10:11;p.blockedFieldGoal=invalid!=2;
        if(invalid==3)p.simulated=true;if(invalid==4)p.selectedType=22;
        auto out=tracker.observe(rules,p);assert(out.blockedFieldGoal==(invalid==4));
    }
    std::puts("PASS: verified FG-block proof, fresh result and FG identity, home/neutral ownership, 0-3-point lead TD-style impact, regular impact otherwise, live/presentation phases, duplicate/gate/stale/punt/miss guards and touchdown takeover.");
}
