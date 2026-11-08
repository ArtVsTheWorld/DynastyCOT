#include "../Source/CoreAtmosphere.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    core::Settings settings;unsigned cases=0;
    for(int seconds=0;seconds<=61;++seconds)for(int timeouts=-1;timeouts<=4;++timeouts)for(int lead:{-7,0,1,3,7,8,9,21})for(int side:{0,1})for(bool neutral:{false,true}){
        State s{4,30+lead,30,4,static_cast<float>(seconds),720};s.neutral=neutral;
        if(side==1)std::swap(s.homeScore,s.awayScore);
        energy::Outcome r;r.accepted=r.completed=r.takeaway=true;r.sequence=1;r.event=energy::Event::Turnover;r.side=side;r.defendingLead=lead;r.opponentTimeouts=timeouts;r.opponentToGoal=80;
        bool sealing=seconds<=60&&lead>0&&lead<=8&&(seconds==0||(timeouts>=0&&timeouts<=2&&seconds<=(3-timeouts)*40));
        assert(energy::sealsRegulationTakeaway(r,s)==sealing);
        core::Model model;bool erupted=model.react(settings,r,1,s,true,true);
        assert(erupted==(sealing&&(side==0||neutral)));
        if(erupted){assert(model.event==energy::Event::LateTakeaway);auto token=model.celebration;assert(!model.react(settings,r,2,s,true,true)&&model.celebration==token);}
        ++cases;
    }
    energy::Outcome r;r.accepted=r.completed=r.takeaway=true;r.sequence=1;r.event=energy::Event::FumbleRecovery;r.side=0;r.defendingLead=3;r.opponentTimeouts=1;
    State s{4,30,27,4,60,720};assert(energy::sealsRegulationTakeaway(r,s)); // Any field position, not only the red zone.
    r.opponentTimeouts=2;assert(!energy::sealsRegulationTakeaway(r,s));s.seconds=40;assert(energy::sealsRegulationTakeaway(r,s));
    r.opponentTimeouts=3;assert(!energy::sealsRegulationTakeaway(r,s));r.opponentTimeouts=-1;assert(!energy::sealsRegulationTakeaway(r,s));
    r.opponentTimeouts=0;core::Model gated;assert(!gated.react(settings,r,1,s,true,false));settings.eruptions=false;assert(!gated.react(settings,r,1,s,true,true));settings.eruptions=true;
    r.takeaway=false;assert(!energy::sealsRegulationTakeaway(r,s));r.takeaway=true;s.quarter=3;assert(!energy::sealsRegulationTakeaway(r,s));
    // The tracker retains the OTHER team's timeouts for either side at neutral sites.
    for(int side:{0,1}){
        energy::Tracker tracker;energy::Settings rules;energy::Play p;p.game={4,30,27,1,55,720};p.game.neutral=true;p.offense=1-side;p.down=2;p.toGoal=75;p.toFirst=10;p.resultStamp=1;p.homeTimeouts=3;p.awayTimeouts=1;
        tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);p.game.seconds=50;p.game.phase=4;p.resultStamp=2;p.interception=true;tracker.observe(rules,p);
        p.game.phase=1;p.down=1;p.offense=side;p.interception=false;auto result=tracker.observe(rules,p);
        assert(result.completed&&result.takeaway&&result.opponentTimeouts==(side==0?1:3));
    }
    std::printf("PASS: %u late-takeaway clock/timeout/lead/side cases, conservative kneel math, any-field sealing, unknown/disabled/fullness rejection, duplicate protection and both-team timeout propagation.\n",cases);
}
