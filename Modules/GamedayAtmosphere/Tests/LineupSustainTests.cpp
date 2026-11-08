#include "../Source/LineupAtmosphere.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
energy::Play play(){energy::Play p;p.game={1,0,0,1,720,720};p.game.rank0=6;p.game.rank1=11;p.down=1;p.offense=0;p.toGoal=70;return p;}
sustained::Intent tick(lineup::Model& m,const core::Settings& c,const energy::Play& p,uint64_t now,float fullness=1,bool allowed=true,uint64_t positions=53913){
    m.observe(c,p,123,1,now,allowed,fullness,true,nullptr,positions);sustained::Intent out;out.active=true;out.match=123;out.epoch=1;out.updated=now;return m.merge(out,c,p,now,fullness);
}
int main(){
    core::Settings c;c.opening=false;
    // Quick snaps must carry the excited early-voice envelope, not the much
    // lower raw roar attack. This also covers an unsampled zero-age attack.
    for(uint64_t delay:{1ull,50ull,250ull,500ull,1000ull,2000ull}){
        auto p=play();lineup::Model fast;tick(fast,c,p,1000);auto before=tick(fast,c,p,1000+delay);
        p.game.phase=3;auto after=tick(fast,c,p,1001+delay);assert(fast.carrying&&std::abs(before.intensity-after.intensity)<.01f);
    }
    for(unsigned seed=1;seed<=200;++seed)for(float fullness:{.599f,.60f,.7f,.9f,1.f}){
        auto p=play();p.game.variationSeed=seed;lineup::Model m;tick(m,c,p,1000,fullness);auto early=tick(m,c,p,1500,fullness);
        assert((early.lineup!=0)==(fullness>=.60f));if(fullness<.60f)continue;
        assert(early.cheers&&early.intensity>0);
        auto full=tick(m,c,p,5000,fullness),held=tick(m,c,p,24999,fullness);
        assert(std::fabs(early.intensity-full.intensity)<.001f&&std::fabs(full.intensity-held.intensity)<.001f);
        assert(held.reason==sustained::Reason::Lineup&&held.continuous&&held.lineupCheer==early.lineupCheer);
        auto fading=tick(m,c,p,1000+m.profile.roarMs+2000,fullness);
        assert(fading.reason==sustained::Reason::None&&fading.intensity>0&&fading.intensity<held.intensity&&!fading.cheers);
        auto expired=tick(m,c,p,1000+m.profile.totalMs()+1,fullness);assert(!expired.intensity&&!expired.continuous&&!m.running);
        p.game.phase=3;auto old=tick(m,c,p,1000+m.profile.totalMs()+100,fullness);assert(!m.carrying&&!old.continuous&&!old.intensity);
    }
    // A missed presentation tick cannot hand a completed roar to a new snap.
    auto p=play();lineup::Model stalled;tick(stalled,c,p,1000);tick(stalled,c,p,5000);
    p.game.phase=3;auto late=tick(stalled,c,p,1000+stalled.profile.totalMs()+1);assert(!stalled.carrying&&!late.continuous&&!late.intensity);
    // A real snap during the sustained portion remains connected through a
    // long play and a post-whistle bridge, but carries no scheduled cheer/shake.
    p=play();lineup::Model bridge;tick(bridge,c,p,1000);auto peak=tick(bridge,c,p,25000);
    p.game.phase=3;auto live=tick(bridge,c,p,25100);assert(bridge.carrying&&live.intensity==peak.intensity&&!live.cheers&&!live.shake);
    p.game.phase=4;auto result=tick(bridge,c,p,35100);assert(result.intensity==peak.intensity);
    auto paused=tick(bridge,c,p,35200,1,false);assert(!paused.intensity&&!paused.continuous&&!bridge.playContextActive());
    // Independent ordinary/critical/OT thresholds have not been lowered.
    p=play();p.game.rivalry=false;p.game.rank0=p.game.rank1=0;lineup::Model ordinary;assert(!tick(ordinary,c,p,1000,.84f,true,26101).lineup);assert(!tick(ordinary,c,p,5000,.84f,true,26101).lineup);
    p.down=3;lineup::Model critical;assert(!tick(critical,c,p,1000,.599f).lineup);assert(!tick(critical,c,p,5000,.599f).lineup);
    p.game.rank0=6;p.game.rank1=11;p.down=1;c.packedHouse=false;c.bigGameMinimum=.67f;lineup::Model custom;assert(!tick(custom,c,p,1000,.66f).lineup);assert(!tick(custom,c,p,5000,.66f).lineup);
    c.lineups=false;lineup::Model disabled;assert(!tick(disabled,c,p,1000).lineup);
    std::puts("PASS:60-percent big-game gate, preserved ordinary/critical thresholds, early excited cheer,exact40-second profile with26-30-second sustain and10-14-second fade, bounded expiry/missed frames, no stale snap restart, live/result continuity and pause/disable/custom settings.");
}
