#include "../Source/LineupAtmosphere.h"
#include <cassert>
#include <iostream>
#include <limits>
using namespace attendance;
constexpr uint64_t Seats=27663;
constexpr float Fullness=15371.f/27663;
energy::Play fixture(){energy::Play p;p.game={2,7,3,1,720,720};p.game.rank0=120;p.game.rank1=12;p.offense=0;p.down=1;p.toFirst=10;p.toGoal=75;return p;}
sustained::Intent tick(lineup::Model& m,const core::Settings& c,const energy::Play& p,uint64_t now,float fullness=Fullness,bool allowed=true){
    m.observe(c,p,123,1,now,allowed,fullness,true,nullptr,Seats);
    sustained::Intent out;out.active=true;out.match=123;out.epoch=1;out.updated=now;
    return m.merge(out,c,p,now,fullness);
}
int main(){
    core::Settings c;auto p=fixture();
    const float packedGate=packed::minimum(Seats),standard=packed::standardMinimum(Seats),special=packed::specialMinimum(Seats);
    auto route=upsetsound::evaluate(p.game,Seats,Fullness,true);
    assert(route.stage==2&&route.qualified&&route.minimum>.549f&&route.minimum<.550f);
    assert(packedGate>.832f&&standard>.764f&&Fullness<special);
    assert(!lineup::bigGame(p.game)&&!packed::qualifies(Seats,Fullness,true));
    assert(!lineup::cameraAdmission(c,p,Fullness,true,Seats,1).allowed);
    assert(!upsetsound::evaluate(p.game,Seats,route.minimum-.00001f,true).qualified);
    assert(upsetsound::evaluate(p.game,Seats,route.minimum,true).qualified);
    for(uint64_t seats:{512ull,6060ull,27663ull,61888ull,200000ull}){
        float before=1;
        for(int rank=13;rank<=255;++rank){
            auto s=p.game;s.rank0=rank;auto r=upsetsound::evaluate(s,seats,.5f,true);
            if(r.stage){assert(r.minimum>=.5f&&r.minimum<=packed::standardMinimum(seats)&&r.minimum<=before);before=r.minimum;}
        }
    }
    for(int bad:{-1,0,256,1000}){auto s=p.game;s.rank0=bad;assert(!upsetsound::evaluate(s,Seats,1,true).qualified);}
    for(int opponent:{0,26,120}){auto s=p.game;s.rank1=opponent;assert(!upsetsound::evaluate(s,Seats,1,true).qualified);}
    auto mild=p.game;mild.rank0=30;mild.rank1=12;auto mildRoute=upsetsound::evaluate(mild,Seats,Fullness,true);
    assert(!mildRoute.qualified&&mildRoute.minimum>route.minimum);
    for(float f:{.499f,std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),1.01f})assert(!upsetsound::evaluate(p.game,Seats,f,true).qualified);
    assert(!upsetsound::evaluate(p.game,Seats,1,false).qualified&&!upsetsound::evaluate(p.game,0,1,true).qualified&&!upsetsound::evaluate(p.game,12800001,1,true).qualified);
    assert(!upsetsound::evaluate(p.game,Seats,1,true,false).qualified);
    auto away=reversed(p.game);assert(!upsetsound::evaluate(away,Seats,1,true).qualified);
    away.neutral=true;auto neutral=upsetsound::evaluate(away,Seats,Fullness,true);
    assert(neutral.qualified&&neutral.minimum==route.minimum&&neutral.importance==route.importance);
    for(int quarter:{1,5,6}){auto s=p.game;s.quarter=quarter;assert(!upsetsound::evaluate(s,Seats,1,true).qualified);}
    for(int phase:{0,2,6,7,8}){auto s=p.game;s.phase=phase;assert(!upsetsound::evaluate(s,Seats,1,true).qualified);}
    auto trailing=p.game;trailing.awayScore=14;
    auto chase=upsetsound::evaluate(trailing,Seats,Fullness,true);assert(chase.stage==1&&chase.minimum>route.minimum&&chase.importance<route.importance&&!chase.qualified);
    trailing.awayScore=22;assert(!upsetsound::evaluate(trailing,Seats,1,true).qualified);
    auto late=p;late.game.quarter=4;late.game.seconds=90;
    auto finish=upsetsound::evaluate(late.game,Seats,Fullness,true);
    assert(finish.qualified&&finish.stage==3&&finish.climax&&finish.minimum<route.minimum&&finish.importance>route.importance);
    for(bool enabled:{false,true}){auto config=c;config.upsetWatch=enabled;lineup::Model packedFinish;tick(packedFinish,config,late,1000,.9f);assert(packedFinish.continuousMode&&packedFinish.reason==sustained::Reason::LateDrive);}
    auto longQuarter=late.game;longQuarter.quarterSeconds=1200;longQuarter.seconds=150;
    auto shortQuarter=late.game;shortQuarter.quarterSeconds=240;shortQuarter.seconds=30;
    assert(upsetsound::evaluate(longQuarter,Seats,Fullness,true).minimum==finish.minimum&&upsetsound::evaluate(shortQuarter,Seats,Fullness,true).minimum==finish.minimum);
    trailing=late.game;trailing.awayScore=17;assert(!upsetsound::evaluate(trailing,Seats,1,true).qualified);

    lineup::Model model;tick(model,c,p,1000);auto buildup=tick(model,c,p,5000);
    assert(model.playContextActive()&&model.qualifiedMinimum==route.minimum&&!model.packedHouse);
    assert(buildup.continuous&&buildup.lineup&&buildup.reason==sustained::Reason::Upset&&buildup.intensity>0&&!buildup.shake&&!buildup.lineupCheer);
    auto serial=model.serial,cheer=model.cheerSerial;tick(model,c,p,6000);assert(model.serial==serial&&model.cheerSerial==cheer);
    p.game.phase=3;auto live=tick(model,c,p,7000);assert(live.continuous&&model.carrying&&model.playContextActive()&&!live.cheers);
    p.game.phase=4;tick(model,c,p,8000);assert(model.playContextActive());
    energy::Outcome result;result.accepted=result.completed=true;result.sequence=1;result.side=0;result.event=energy::Event::Sack;
    core::Model sound;bool eligible=lineup::reactionEligible(p.game,Fullness,true,model.playContextActive(),model.qualifiedMinimum);assert(eligible);
    sound.react(c,result,8,p.game,true,eligible,model.qualifiedMinimum);sound.update(c,p,8.1,Fullness,Fullness,model.momentWeight(),eligible,model.qualifiedMinimum);
    auto reaction=model.merge(sound.intent(c,p,123,1,8100,true),c,p,8100,Fullness);
    assert(reaction.reason==sustained::Reason::Result&&reaction.resultCheer&&sound.resultMinimum==route.minimum&&sound.limit==core::normalLimit(c));
    // A failed bid releases layers and admission without another cheer.
    p.game.awayScore=28;auto lost=tick(model,c,p,9000);assert(!model.playContextActive()&&!lost.continuous&&!lost.cheers&&!lost.shake);
    assert(tick(model,c,p,20000).intensity==0);
    // Master switches, missing crowd data, suspension and match ownership fail closed.
    for(int disable=0;disable<6;++disable){auto config=c;auto q=fixture();lineup::Model m;tick(m,config,q,1000);tick(m,config,q,5000);
        if(disable==0)config.upsetWatch=false;if(disable==1)config.enabled=false;if(disable==2)config.lineups=false;if(disable==3)config.maximumPercent=0;
        if(disable==5)q.simulated=true;
        tick(m,config,q,6000,Fullness,disable!=4);assert(!m.playContextActive()&&!m.upsetSound.qualified&&!m.layeredActive());
    }
    model.suspend();assert(!model.playContextActive()&&model.upsetSound.stage==0&&model.weight==0);
    auto ordinary=fixture();ordinary.game.rank0=ordinary.game.rank1=0;
    model.observe(c,ordinary,124,2,21000,true,Fullness,true,nullptr,Seats);assert(!model.playContextActive()&&model.upsetSound.stage==0);
    // Neutral sites mirror the underdog; home venues never add an away lineup cheer.
    for(bool neutralSite:{false,true}){auto q=fixture();q.offense=1;q.toGoal=5;q.game.neutral=neutralSite;lineup::Model m;
        tick(m,c,q,1000);auto out=tick(m,c,q,1400);assert(out.cheers==neutralSite&&(out.lineupCheer!=0)==neutralSite);
    }
    // Late underdog tension can use the existing high-leverage cap, never a new boost.
    lineup::Model final;auto out=tick(final,c,late,1000);auto continuousId=final.serial;
    assert(out.reason==sustained::Reason::Upset&&out.continuous&&!out.shake&&!final.packedHouse&&final.momentWeight()==1);
    for(int phase:{1,3,4,5}){late.game.phase=phase;auto tension=tick(final,c,late,5000);assert(tension.supporting&&final.serial==continuousId&&final.playContextActive()&&!final.shakeTier);}
    core::Model finale;finale.update(c,late,5,Fullness,Fullness,final.momentWeight(),true,final.qualifiedMinimum);
    assert(finale.highLeverage&&finale.parent>0&&finale.parent<=core::highLeverageLimit(c));
    // Confirmed upset victory inherits the admitted density and eruption priority.
    float minimum=final.qualifiedMinimum;late.game.phase=7;tick(final,c,late,6000);
    finale.update(c,late,6,Fullness,Fullness,0,true,minimum);
    auto eruption=final.merge(finale.intent(c,late,123,1,6000,true),c,late,6000,Fullness);
    assert(finale.erupting&&finale.postgameHeld&&finale.eruptionMinimum==minimum&&eruption.reason==sustained::Reason::Eruption&&eruption.celebration);
    assert(finale.limit<=core::eruptionLimit(c)&&c.maximumPercent==25&&c.highLeveragePercent==35&&c.eruptionPercent==50&&c.chantMaximumPercent==15);
    assert(packed::minimum(Seats)==packedGate&&packed::standardMinimum(Seats)==standard&&packed::specialMinimum(Seats)==special);
    std::cout<<"PASS: independent Upset Watch sound admission, UNC/Notre Dame sample, relative ranks, proportional clocks, neutral ownership, failed bids, reset/switches, reaction/eruption handoffs, unchanged packed/camera gates and volume caps.\n";
}
