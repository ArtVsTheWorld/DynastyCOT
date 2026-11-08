#include "../Source/LineupAtmosphere.h"
#include <cassert>
#include <cstdio>
#include <set>
using namespace attendance;
using energy::Event;
energy::Play initial(){energy::Play p;p.game={1,0,0,6,720,720};p.game.rank0=6;p.game.rank1=11;p.game.variationSeed=73;p.offense=0;p.down=0;p.toGoal=65;p.toFirst=10;p.resultStamp=1;return p;}
energy::Outcome opening(unsigned scoringPlay,int side,int source,bool earlierTd=false,bool simulated=false){
    energy::Tracker t;energy::Settings c;auto p=initial();p.toGoal=35;p.game.neutral=side==1;t.observe(c,p);energy::Outcome score;
    for(unsigned i=1;i<=scoringPlay;++i){
        p.game.phase=1;p.down=i==1?0:1;p.offense=i==scoringPlay&&source!=0?1-side:side;
        t.observe(c,p);p.game.phase=3;t.observe(c,p);
        p.game.phase=4;p.game.seconds-=5;p.resultStamp+=10;p.interception=i==scoringPlay&&source==1;
        if(earlierTd&&i==1)p.game.awayScore+=6;
        if(i==scoringPlay){if(side==0)p.game.homeScore+=6;else p.game.awayScore+=6;}
        p.simulated=simulated&&i==1;auto out=t.observe(c,p);if(i==scoringPlay)score=out;
        p.simulated=false;p.game.phase=1;p.down=1;t.observe(c,p);
    }
    return score;
}
energy::Outcome takeaway(int quarter,float seconds,int lead,float goal,int side,bool neutral,bool fumble=false,bool flip=true,bool fresh=true){
    energy::Tracker t;energy::Settings c;auto p=initial();p.game={quarter,20+lead,20,1,seconds+((quarter==4)?5.f:0.f),720};p.game.neutral=neutral;p.offense=1;p.down=2;p.toGoal=goal;p.homeTimeouts=p.awayTimeouts=0;
    if(side==1){p.game=reversed(p.game);p.offense=0;}
    t.observe(c,p);p.game.phase=3;t.observe(c,p);p.game.phase=4;p.game.seconds=seconds;
    if(fresh)p.resultStamp+=10;p.interception=!fumble;p.fumble=fumble;t.observe(c,p);
    // Some result facts disappear before possession settles; the tracker must
    // retain only fresh facts from this verified play.
    p.game.phase=1;p.down=1;if(flip)p.offense=side;p.interception=p.fumble=false;return t.observe(c,p);
}
int main(){
    std::set<unsigned> attacks,supports;std::set<int> sizes;
    for(unsigned i=0;i<1000;++i){
        auto p=variety::choose(73,i),same=variety::choose(73,i);assert(p.attackMs==same.attackMs&&p.supportDelayMs==same.supportDelayMs);
        assert(p.attackMs>=2500&&p.attackMs<=3500&&p.envelopeAttackMs>=2200&&p.envelopeAttackMs<=2800);
        assert(p.roarMs>=26000&&p.roarMs<=30000&&p.releaseMs>=10000&&p.releaseMs<=14000&&p.totalMs()==40000);
        assert(p.supportDelayMs>=1200&&p.supportDelayMs<=2000&&p.size(1)==3&&p.size(.2f)==1);
        attacks.insert(p.attackMs);supports.insert(p.supportDelayMs);sizes.insert(p.size(.75f));
    }
    assert(attacks.size()>100&&supports.size()>100&&sizes.size()==2);
    core::Settings c;
    for(unsigned number=1;number<=6;++number)for(int source=0;source<3;++source)for(int side=0;side<2;++side){
        auto result=opening(number,side,source);assert(result.completed&&result.firstTouchdown&&result.openingPlay==(number<=2?number:0));
        auto p=initial();p.game.phase=4;p.game.neutral=side==1;p.game.homeScore=side==0?6:0;p.game.awayScore=side==1?6:0;
        core::Model m;assert(m.react(c,result,1,p.game,true)==(number<=2));if(number<=2)assert(m.event==Event::EarlyTouchdown);
        core::Model quiet;assert(!quiet.react(c,result,1,p.game,false));
        if(side==1){p.game.neutral=false;core::Model away;assert(!away.react(c,result,1,p.game,true));}
    }
    auto earlier=opening(4,0,0,true);assert(!earlier.firstTouchdown);core::Model noSecond;auto p=initial();p.game.homeScore=p.game.awayScore=6;assert(!noSecond.react(c,earlier,1,p.game,true));
    auto sim=opening(3,0,0,false,true);assert(sim.openingPlay==0);core::Model noSim;assert(!noSim.react(c,sim,1,p.game,true));
    // Use the actual lineup model, tracker and core together at the new floor.
    for(float fraction:{.499f,.50f}){
        lineup::Model layers;energy::Tracker actual;core::Model eruption;energy::Settings settings;auto q=initial();q.offense=1;
        auto observe=[&](uint64_t now){layers.observe(c,q,123,1,now,true,fraction,true,nullptr,53913);auto out=actual.observe(settings,q);return eruption.react(c,out,now/1000.0,q.game,layers.layeredActive());};
        observe(1000);q.game.phase=1;observe(2000);observe(6000);q.game.phase=3;observe(7000);q.game.phase=4;q.game.seconds=710;q.game.homeScore=6;
        assert(observe(10000)==(fraction>=.50f));assert(!observe(10050)); // Below50 cannot erupt; repeated result cannot restart.
    }
    // Even a zero-score/full-clock resumed scenario cannot claim a known start.
    energy::Tracker resumed;energy::Settings resumeSettings;auto resumedPlay=initial();resumedPlay.resumed=true;resumed.observe(resumeSettings,resumedPlay);
    resumedPlay.game.phase=1;resumed.observe(resumeSettings,resumedPlay);resumedPlay.game.phase=3;resumed.observe(resumeSettings,resumedPlay);
    resumedPlay.game.homeScore=6;resumedPlay.game.phase=4;assert(resumed.observe(resumeSettings,resumedPlay).openingPlay==0);
    // A score may appear during the live phase before the result phase arrives.
    energy::Tracker delayed;auto live=initial();delayed.observe(resumeSettings,live);live.game.phase=1;delayed.observe(resumeSettings,live);live.game.phase=3;delayed.observe(resumeSettings,live);
    live.game.homeScore=6;delayed.observe(resumeSettings,live);live.game.phase=4;auto liveScore=delayed.observe(resumeSettings,live);assert(liveScore.firstTouchdown&&liveScore.openingPlay==1);
    for(bool fumble:{false,true})for(int side:{0,1}){
        auto result=takeaway(5,0,0,12,side,true,fumble);assert(result.takeaway&&result.completed&&result.side==side);
        p=initial();p.game={5,20,20,1,0,720};p.game.neutral=true;core::Model m;assert(m.react(c,result,1,p.game));assert(m.event==Event::OvertimeTakeaway&&m.eruptionSeconds==60);
        m.update(c,p,2,1,1);assert(std::fabs(volume::boostPercent(m.parent)-50)<.01f);m.update(c,p,36,1,1);assert(m.intent(c,p,123,1,36000,true).continuous);m.update(c,p,62,1,1);assert(!m.erupting);
        assert(!takeaway(5,0,0,12,side,true,fumble,false).takeaway);assert(!takeaway(5,0,0,12,side,true,fumble,true,false).takeaway);
        p.game.neutral=false;core::Model venue;assert(venue.react(c,result,1,p.game)==(side==0));
    }
    for(float seconds:{0.f,30.f,31.f})for(int lead:{-1,0,1,7,28})for(float goal:{0.f,20.f,21.f}){
        auto result=takeaway(4,seconds,lead,goal,0,false);p=initial();p.game={4,20+lead,20,1,seconds,720};core::Model m;
        assert(m.react(c,result,1,p.game)==(seconds<=60&&lead>0&&lead<=8));
    }
    // Changing possession without interception/fumble proof is not a takeaway.
    energy::Tracker t;energy::Settings ec;p=initial();p.game={5,20,20,1,0,720};p.down=4;p.offense=1;t.observe(ec,p);p.game.phase=3;t.observe(ec,p);p.game.phase=1;p.offense=0;p.down=1;p.resultStamp=2;
    auto downs=t.observe(ec,p);assert(!downs.takeaway);core::Model noDowns;assert(!noDowns.react(c,downs,1,p.game));
    // Final interceptions need no next lineup, but an unclassified fumble
    // cannot be assumed to have changed hands just because the game ended.
    for(bool pick:{false,true}){
        t.reset();p=initial();p.game={4,24,20,1,5,720};p.offense=1;p.down=2;p.toGoal=12;t.observe(ec,p);p.game.phase=3;t.observe(ec,p);
        p.game.phase=7;p.game.seconds=0;p.down=13;p.interception=pick;p.fumble=!pick;p.resultStamp=2;auto out=t.observe(ec,p);core::Model m;
        assert(out.takeaway==pick&&m.react(c,out,1,p.game)==pick);
    }
    // A winning confirmation upgrades the same OT play without replaying its cheer.
    auto result=takeaway(5,0,0,12,0,false);core::Model won;p.game={5,20,20,1,0,720};assert(won.react(c,result,1,p.game));auto token=won.celebration;
    result.event=Event::OvertimeWin;p.game.homeScore=23;p.game.phase=7;assert(!won.react(c,result,3,p.game));won.update(c,p,3,1,1);won.update(c,p,180,1,1);assert(won.celebration==token&&won.postgameHeld&&won.intent(c,p,1,1,180000,true).continuous);won.finishPostgame();assert(!won.intent(c,p,1,1,180001,true).active);
    core::Model tail;lineup::Model otLayer;p=initial();p.game={5,20,20,1,0,720};result=takeaway(5,0,0,12,0,false);assert(tail.react(c,result,1,p.game));
    otLayer.observe(c,p,123,1,1000,true,1,true,nullptr,53913);otLayer.observe(c,p,123,1,56000,true,1,true,nullptr,53913);tail.update(c,p,56,1,1,otLayer.momentWeight());
    auto fading=tail.intent(c,p,123,1,56000,true);assert(tail.erupting&&!fading.celebration);auto handoff=otLayer.merge(fading,c,p,56000,1);
    assert(handoff.reason==sustained::Reason::Overtime&&handoff.continuous&&handoff.intensity>=fading.intensity&&volume::boostPercent(tail.parent)<=50.01f);
    // Go-ahead eruptions use the new inclusive 30-second boundary.
    for(float seconds:{0.f,29.f,30.f,31.f,60.f,90.f}){
        t.reset();p=initial();p.game={4,20,21,1,120,720};p.down=4;t.observe(ec,p);p.game.phase=3;t.observe(ec,p);p.game.phase=4;p.game.seconds=seconds;p.game.homeScore=23;auto out=t.observe(ec,p);core::Model m;
        assert(m.react(c,out,1,p.game)==(seconds<=30));
    }
    std::puts("PASS: bounded stable variety, actual first-two plays including kickoff, first-TD/active-layer gating, simulation/away rejection, fresh interception/fumble possession proof, OT 60-second/postgame carry, close game-sealing takeover boundaries and 30-second go-ahead cutoff.");
}
