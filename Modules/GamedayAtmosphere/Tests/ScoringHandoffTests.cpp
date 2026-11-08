#include "../Source/ScoringSoundHandoff.h"
#include "../Source/CoreAtmosphere.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
sustained::Intent running(uint64_t now,float fullness){sustained::Intent out;out.match=123;out.epoch=1;out.updated=now;out.active=out.continuous=out.supporting=true;out.reason=sustained::Reason::Result;out.priority=4;out.reactionId=7;out.size=3;out.intensity=950*std::sqrt(fullness);return out;}
int main(){
    for(float fullness:{.5f,.75f,1.f})for(bool neutral:{false,true})for(bool eruption:{false,true}){
        core::Settings c;core::Model sound;soundhandoff::Model handoff;
        auto run=running(1000,fullness);assert(handoff.merge(run,3,1000,fullness).intensity==run.intensity);
        energy::Play p;p.game={2,7,0,4,400,720};p.game.neutral=neutral;p.offense=0;p.down=1;
        energy::Outcome score;score.accepted=score.completed=true;score.sequence=7;score.side=0;score.homePoints=6;score.preSnapMargin=0;
        score.event=energy::Event::HomeScore;score.hailMary=eruption;score.touchdownYards=eruption?65.f:25.f;
        sound.react(c,score,1.1,p.game,true,true,.5f);
        for(uint64_t now=1100;now<=1700;now+=25){
            sound.update(c,p,now*.001,fullness,fullness,0,true,.5f);auto native=sound.intent(c,p,123,1,now,true);
            auto joined=handoff.merge(native,4,now,fullness);
            assert(joined.reason==native.reason&&joined.resultCheer==native.resultCheer&&joined.celebration==native.celebration);
            assert(joined.continuous==native.continuous&&joined.releaseMs==native.releaseMs&&joined.priority==native.priority);
            if(now<=1350)assert(joined.intensity+0.01f>=run.intensity);
            if(now>=1600)assert(joined.intensity==native.intensity);
            assert(joined.intensity<=1000*std::sqrt(fullness)+.01f&&sound.parent<=sound.limit&&sound.limit<=core::eruptionLimit(c));
        }
    }
    // A smaller ordinary result, no previous sound, an old lease and all
    // ownership/suspension boundaries cannot manufacture a scoring buildup.
    for(int cause=0;cause<6;++cause){
        soundhandoff::Model handoff;auto run=running(1000,1);handoff.merge(run,3,1000,1);
        auto score=running(cause==2?1800:1100,1);score.reason=sustained::Reason::Eruption;score.celebration=1;score.priority=6;score.intensity=0;
        if(cause==0)handoff.reset();if(cause==1)score.active=false;if(cause==3)score.match=124;if(cause==4)score.epoch=2;
        auto out=handoff.merge(score,cause==5?3:4,score.updated,1);assert(out.intensity==0);
    }
    // Upgrading a touchdown to an eruption carries the prior score level too.
    soundhandoff::Model upgrade;upgrade.merge(running(1000,1),3,1000,1);
    auto td=running(1100,1);td.scoringReaction=true;td.priority=5;td.intensity=970;upgrade.merge(td,4,1100,1);
    auto burst=td;burst.updated=1200;burst.reason=sustained::Reason::Eruption;burst.celebration=1;burst.priority=6;burst.intensity=0;
    assert(upgrade.merge(burst,4,1200,1).intensity==970);
    // Ordinary non-scoring reactions keep their own envelope unchanged.
    auto ordinary=running(1250,1);ordinary.intensity=200;assert(upgrade.merge(ordinary,4,1250,1).intensity==200);
    // Every negative family cancels a previous positive attack-floor carry,
    // including the smaller population selectors and newly enabled families.
    for(int id=0;id<112;++id)if(catalog::usable(id)&&catalog::negative(id)){
        soundhandoff::Model h;auto before=running(1000,1);h.merge(before,3,1000,1);
        auto negative=running(1100,1);negative.liveCheer=1;negative.liveType=id;negative.intensity=0;
        assert(h.merge(negative,4,1100,1).intensity==0);
        auto after=negative;after.updated=1200;after.liveCheer=0;after.scoringReaction=true;after.priority=5;after.reactionId=8;
        assert(h.merge(after,4,1200,1).intensity==0);
    }
    std::puts("PASS: earned run-to-TD/eruption continuity, first-attack floor, TD upgrade, exact existing timers/IDs, density/caps, and cold/stale/pause/world/new-snap guards.");
}
