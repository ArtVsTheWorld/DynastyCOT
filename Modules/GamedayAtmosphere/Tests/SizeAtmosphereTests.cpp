#include "../Source/LineupAtmosphere.h"
#include <cassert>
#include <iostream>
using namespace attendance;
energy::Play fixture(){energy::Play p;p.game={2,14,14,1,600,720};p.offense=0;p.down=1;p.toGoal=75;p.toFirst=10;return p;}
int main(){
    core::Settings c;
    assert(packed::standardMinimum(6060)==.90f&&packed::standardMinimum(61888)==.55f);
    assert(packed::specialMinimum(6060)==.85f&&packed::specialMinimum(61888)==.50f);
    assert(packed::standardMinimum(512)==.90f&&packed::specialMinimum(200000)==.50f);
    for(uint64_t n:{512ull,6060ull,11700ull,30464ull,51424ull,56099ull,61888ull,120000ull}){
        float standard=packed::standardMinimum(n),special=packed::specialMinimum(n);
        for(int kind=0;kind<5;++kind)for(bool reaches:{false,true}){
            auto p=fixture();float needed=kind>=3?special:standard;
            if(kind==0){p.game.rank0=6;p.game.rank1=11;}
            if(kind==1){p.down=3;p.toFirst=5;}
            if(kind==2)p.toGoal=5;
            if(kind==3){p.game.quarter=5;p.game.seconds=0;}
            if(kind==4){p.game.quarter=4;p.game.seconds=90;p.toGoal=30;}
            float fullness=needed+(reaches?0:-.0001f);
            lineup::Model m;m.observe(c,p,123,1,1000,true,fullness,true,nullptr,n);
            m.observe(c,p,123,1,1500,true,fullness,true,nullptr,n);
            assert(m.playContextActive()==reaches);
            if(!reaches){assert(m.qualifiedMinimum>1);sustained::Intent base;base.active=true;base.match=123;base.epoch=1;base.updated=1500;
                auto quiet=m.merge(base,c,p,1500,fullness);assert(!quiet.cheers&&!quiet.lineupCheer&&!quiet.continuous&&!quiet.shake);continue;}
            assert(m.qualifiedMinimum==needed&&m.layeredActive());
            assert(lineup::reactionEligible(p.game,fullness,true,true,m.qualifiedMinimum));
            assert(!lineup::reactionEligible(p.game,needed-.0001f,true,true,m.qualifiedMinimum));
            assert(lineup::cameraVenue(c,p.game,fullness,true,n)==(kind==0)); // Independent sound routes do not unlock venue-only pre-snap motion.
            p.game.phase=3;m.observe(c,p,123,1,1600,true,fullness,true,nullptr,n);
            p.game.phase=4;m.observe(c,p,123,1,1700,true,fullness,true,nullptr,n);
            assert(m.playContextActive()&&m.qualifiedMinimum==needed);
            // No hidden 60% veto at a 50/55% admitted play. An eruption keeps
            // its own threshold even after the live lineup context is gone.
            core::Model sound;energy::Outcome score;score.accepted=score.completed=true;score.sequence=1;score.side=0;score.event=energy::Event::LastSecondLead;
            auto finish=fixture();finish.game={4,21,20,4,1,720};
            assert(sound.react(c,score,2,finish.game,true,true,m.qualifiedMinimum));
            sound.update(c,finish,2,fullness,fullness,0,true,m.qualifiedMinimum);assert(sound.erupting&&sound.eruptionMinimum==needed);
            finish.game.phase=7;sound.update(c,finish,3,fullness,fullness,0,false,1.01f);
            assert(sound.erupting&&sound.postgameHeld);sound.update(c,finish,90,fullness,fullness,0,false,1.01f);assert(sound.erupting);
            assert(sound.finishPostgame());assert(!sound.intent(c,finish,123,1,91000,true).active);
            core::Model drop;drop.react(c,score,2,finish.game,true,true,needed);finish.game.phase=4;drop.update(c,finish,3,needed-.001f,needed-.001f,0,false,1.01f);assert(!drop.erupting);
        }
        // All size-based routes fail closed without a verified layout.
        auto unknown=fixture();unknown.game.rank0=6;unknown.game.rank1=11;lineup::Model m;m.observe(c,unknown,123,1,1000,true,1,true,nullptr,0);assert(!m.playContextActive());
    }
    // A true big game gets an opening allowance. Ordinary small venues retain their normal threshold.
    auto p=fixture();p.game={1,0,0,1,720,720};p.game.rank0=6;p.game.rank1=11;
    lineup::Model opening;opening.observe(c,p,123,1,1000,true,.60f,true,nullptr,6060);assert(opening.playContextActive());
    float previous=1;
    for(uint64_t n=512;n<=200000;n+=37){float standard=packed::standardMinimum(n),special=packed::specialMinimum(n);
        assert(standard<=previous&&standard>=.55f&&standard<=.90f&&special>=.50f&&special<=.85f);
        assert(std::fabs(standard-special-.05f)<.000001f&&packed::minimum(n)>standard);previous=standard;}
    // Ordinary volume curves are independent of stadium size/context gates.
    assert(balance::crowdBoost(.75f)==0&&balance::crowdBoost(1)==25&&balance::mix(1,6060).net(1)==balance::mix(1,6060).net(2));
    std::cout<<"PASS: 90-to-55/85-to-50 size gates, all five contexts at exact boundaries, inherited reaction thresholds, 50/55% eruptions and postgame carry, big-game opening allowance/unknown-layout rejection, venue-only anticipation motion and unchanged ordinary volume.\n";
}
