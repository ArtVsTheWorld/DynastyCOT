#include "../Source/LineupAtmosphere.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
energy::Play play(){energy::Play p;p.game={1,0,0,5,720,720};p.game.rank0=6;p.game.rank1=11;p.down=1;p.offense=0;p.toGoal=70;return p;}
int main(){
    for(int side:{0,1})for(int margin:{-9,-8,0,8,9})for(float seconds:{0.f,120.f,121.f}){
      auto late=play();late.offense=side;late.game.quarter=4;late.game.seconds=seconds;late.game.homeScore=20+margin;late.game.awayScore=20;late.toGoal=80;
      assert((lineup::lateDrive(late)>0)==(seconds<=120&&std::abs(margin)<=8));
      late.simulated=true;assert(lineup::lateDrive(late)==0);
    }
    core::Settings c;auto p=play();lineup::Model m;
    auto tick=[&](uint64_t now,float fraction=1){m.observe(c,p,123,1,now,true,fraction,true,nullptr,53913);sustained::Intent b;b.active=true;b.match=123;b.epoch=1;b.updated=now;return m.merge(b,c,p,now,fraction);};
    auto prepared=tick(1000);assert(prepared.cameraPrime&&prepared.shakeTier==1&&prepared.intensity==0&&!prepared.lineup&&prepared.reason==sustained::Reason::None);p.game.phase=1;tick(2000);auto full=tick(6000);assert(full.reason==sustained::Reason::Lineup&&full.size==3&&full.supporting&&full.shakeTier==1&&full.intensity==1000);
    auto serial=m.serial;tick(6500);assert(m.serial==serial);
    p.game.phase=3;auto snap=tick(7000);assert(!snap.shake&&!snap.cheers&&snap.intensity==full.intensity&&snap.reason==sustained::Reason::Lineup);
    auto longPlay=tick(23000);assert(longPlay.intensity==snap.intensity&&longPlay.continuous&&longPlay.supporting); // A long touchdown cannot outlast a snap-based tail.
    p.game.phase=4;p.game.homeScore=6;auto whistle=tick(23100);assert(whistle.intensity==snap.intensity&&!whistle.shake&&!whistle.cheers);
    assert(tick(27000).intensity==whistle.intensity);auto release=tick(27100);assert(release.reason==sustained::Reason::None&&release.intensity==whistle.intensity);
    auto tail=tick(32100);assert(tail.intensity>0&&tail.intensity<release.intensity);assert(tick(37200).intensity==0);
    p.game.phase=5;tick(43000);p.game.phase=1;tick(44000);assert(m.serial==serial+1);p.down=3;assert(tick(45999).shakeTier==3);assert(tick(48000).shakeTier==3&&m.momentWeight()==1);
    m.suspend();assert(!tick(49000).lineup);p.game.phase=5;tick(50000);p.game.phase=1;c.opening=false;tick(51000,.599f);assert(!tick(55000,.599f).lineup);
    p.game.phase=5;tick(56000,.60f);p.game.phase=1;tick(57000,.60f);assert(tick(61000,.60f).lineup);
    p.game.phase=3;tick(62000);assert(tick(106900).reason==sustained::Reason::Lineup);assert(tick(107000).reason==sustained::Reason::None);assert(tick(117100).intensity==0); // Stuck live-ball protection.
    p.game.phase=5;tick(118000);p.game.phase=1;tick(119000);tick(123000);p.game.phase=3;tick(124000);p.game.phase=4;tick(125000);
    auto beforeFresh=m.serial;p.game.phase=1;auto fresh=tick(126000);assert(m.serial==beforeFresh+1&&!m.carrying&&fresh.shakeTier==3); // Fresh lineup resets the carry and starts its own attack.
    tick(130000);p.game.phase=3;tick(131000);m.suspend();assert(!tick(132000).lineup&&!m.carrying); // Pause/replay cannot retain the result bridge.
    for(int kind=0;kind<6;++kind){
        auto q=play();q.game.rivalry=false;q.game.rank0=q.game.rank1=0;q.game.phase=1;q.game.quarter=2;
        if(kind==0)q.game.rivalry=true;if(kind==1){q.game.rank0=6;q.game.rank1=11;}if(kind==2)q.game.playoff=true;
        if(kind==3)postseason(q.game,false,true);if(kind==4)q.game.hfaActive=true;
        lineup::Model x;x.observe(c,q,555,1,1000,true,.8f,true,nullptr,26101);sustained::Intent b;b.active=true;b.match=555;b.epoch=1;b.updated=5000;x.observe(c,q,555,1,5000,true,.8f,true,nullptr,26101);
        assert((x.merge(b,c,q,5000,.8f).lineup!=0)==(kind!=5&&kind!=4&&kind!=0));
    }
    auto ordinary=play();ordinary.game.rivalry=false;ordinary.game.rank0=ordinary.game.rank1=0;ordinary.game.phase=1;ordinary.game.quarter=2;lineup::Model x;
    ordinary.game.hfaActive=true;x.observe(c,ordinary,777,1,1000,true,.8f,true,nullptr,26101);ordinary.game.hfaActive=false;ordinary.game.phase=5;x.observe(c,ordinary,777,1,2000,true,.8f,true,nullptr,26101);ordinary.game.phase=1;assert(!x.observe(c,ordinary,777,1,3000,true,.8f,true,nullptr,26101));
    assert(!x.observe(c,ordinary,778,2,4000,true,.8f,true,nullptr,26101)); // HFA cannot leak into the next matchup.
    auto q=play();q.game={5,20,20,1,0,720};q.game.rivalry=false;q.toGoal=8;lineup::Model ot;
    assert(ot.observe(c,q,888,1,1000,true,.55f,true,nullptr,53913)&&ot.momentWeight()==1);sustained::Intent b;b.active=true;b.match=888;b.epoch=1;
    for(int phase:{1,3,4,5}){q.game.phase=phase;ot.observe(c,q,888,1,6000,true,.55f,true,nullptr,53913);b.updated=6000;auto w=ot.merge(b,c,q,6000,.55f);assert(w.reason==sustained::Reason::Overtime&&w.continuous&&w.supporting);assert(w.shake==(phase==1||phase==5));assert(w.cameraPrime==(phase==5));}
    auto held=ot.merge(b,c,q,6000,.55f);auto cheerToken=held.lineupCheer;assert(!held.cheers);
    auto first=ot.merge(b,c,q,6000,.55f).lineup;ot.observe(c,q,888,1,42000,true,.55f,true,nullptr,53913);b.updated=42000;assert(ot.merge(b,c,q,42000,.55f).lineup!=first&&ot.merge(b,c,q,42000,.55f).lineupCheer==cheerToken);
    q.game.phase=1;ot.observe(c,q,888,1,26500,true,.55f,true,nullptr,53913);b.updated=26500;auto nextPlay=ot.merge(b,c,q,26500,.55f);assert(nextPlay.lineupCheer!=cheerToken&&nextPlay.lineupCheerBegan==26500&&nextPlay.cheers);
    ot.observe(c,q,888,1,27000,true,.549f,true,nullptr,53913);auto below=ot.merge(b,c,q,27000,.549f);assert(below.reason==sustained::Reason::None&&!below.shake&&!below.cheers); // Graceful release, no new event below the floor.
    q=play();q.game={4,20,23,1,80,720};q.game.rivalry=false;q.toGoal=15;assert(lineup::lateDrive(q)>.7f);
    lineup::Model late;assert(late.observe(c,q,999,1,1000,true,.55f,true,nullptr,53913));b.match=999;b.updated=5000;late.observe(c,q,999,1,5000,true,.55f,true,nullptr,53913);auto l=late.merge(b,c,q,5000,.55f);assert(l.reason==sustained::Reason::LateDrive&&l.shakeTier==3);
    q.offense=1;assert(lineup::lateDrive(q)>.7f);q.game.neutral=true;q.game=attendance::reversed(q.game);assert(lineup::lateDrive(q)>.7f);
    b.reason=sustained::Reason::Eruption;b.celebration=1;assert(late.merge(b,c,q,5000,.55f).reason==sustained::Reason::Eruption);
    for(float minutes:{60.f,300.f,720.f,1200.f}){q.game={1,0,0,1,minutes,minutes};assert(lineup::opening(q.game)==1);q.game.seconds=minutes*.25f;assert(lineup::opening(q.game)==0);}
    // Holding at the line sustains excitement, then a long decay. The remaining
    // roar is held at its current strength when snapping during release.
    auto fadedPlay=play();fadedPlay.game.phase=1;lineup::Model fade;fade.observe(c,fadedPlay,1000,1,1000,true,1,true,nullptr,53913);
    fade.observe(c,fadedPlay,1000,1,5000,true,1,true,nullptr,53913);float peak=fade.weight;
    uint64_t releaseAt=1000+fade.profile.roarMs;
    fade.observe(c,fadedPlay,1000,1,releaseAt+3000,true,1,true,nullptr,53913);assert(fade.weight>0&&fade.weight<peak&&fade.reason==sustained::Reason::None);
    fadedPlay.game.phase=3;fade.observe(c,fadedPlay,1000,1,releaseAt+3100,true,1,true,nullptr,53913);assert(fade.carrying&&fade.reason==sustained::Reason::Lineup);
    fade.observe(c,fadedPlay,1000,1,releaseAt+15000,true,1,true,nullptr,53913);assert(fade.weight>0);
    fadedPlay.game.phase=4;fade.observe(c,fadedPlay,1000,1,releaseAt+16000,true,1,true,nullptr,53913);
    fade.observe(c,fadedPlay,1000,1,releaseAt+31000,true,1,true,nullptr,53913);assert(fade.weight==0);
    auto rout=play();rout.game.phase=1;lineup::Model blowout;blowout.observe(c,rout,2000,1,1000,true,1,true,nullptr,53913);blowout.observe(c,rout,2000,1,5000,true,1,true,nullptr,53913);assert(blowout.weight>0);
    rout.game.awayScore=28;blowout.observe(c,rout,2000,1,5100,true,1,true,nullptr,53913);assert(!blowout.playContextActive()&&!blowout.running&&blowout.shakeTier==0);
    blowout.observe(c,rout,2000,1,15101,true,1,true,nullptr,53913);assert(blowout.weight==0);
    rout.game.awayScore=0;lineup::Model unknown;assert(!unknown.observe(c,rout,2000,1,1000,true,1,false,nullptr,53913)&&!unknown.playContextActive());
    assert(lineup::competitive(rout.game)==1);rout.game.awayScore=20;assert(lineup::competitive(rout.game)==.5f);rout.game.awayScore=24;assert(lineup::competitive(rout.game)==0);
    // Every added opening effect now respects the firm 60-percent minimum.
    auto openingGame=play();openingGame.game.phase=1;lineup::Model admitted;core::Settings openingConfig;
    admitted.observe(openingConfig,openingGame,3000,1,1000,true,.42f,true,nullptr,53913);admitted.observe(openingConfig,openingGame,3000,1,5000,true,.42f,true,nullptr,53913);
    assert(!admitted.layeredActive()&&!admitted.playContextActive());
    assert(!lineup::cameraVenue(openingConfig,openingGame.game,.42f,true));
    openingGame.game.quarter=2;assert(!lineup::cameraVenue(openingConfig,openingGame.game,.42f,true));
    openingGame.game.quarter=1;openingGame.game.rank0=openingGame.game.rank1=0;openingGame.game.rivalry=true;
    assert(!lineup::cameraVenue(openingConfig,openingGame.game,.42f,true)&&!lineup::bigGame(openingGame.game));
    std::puts("PASS: big-game eligibility without native HFA, size-based thresholds, softer baseline/critical shake, opening decay at all quarter lengths, bounded live/result handoff without post-snap cheers/shake, OT/late continuous layers, native HFA isolation and eruption priority.");
}
