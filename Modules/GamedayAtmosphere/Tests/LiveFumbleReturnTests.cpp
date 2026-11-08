#include "../Source/LiveFumble.h"
#include "../Source/ReturnAtmosphere.h"
#include "../Source/SituationalReactions.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    for(int state=0;state<=10;++state)for(int sub=-1;sub<=5;++sub)assert(live::isFumbleState(state,sub)==(state==5&&(sub==1||sub==2)));
    for(bool neutral:{false,true})for(int owner:{0,1}){
        live::Fumbles m;energy::Play p;p.game={2,7,7,1,400,720};p.game.neutral=neutral;p.down=1;p.offense=owner;
        live::ForcedEvent event{999,0,true};uint64_t now=1000;
        auto tick=[&](int phase,int type,bool held){p.game.phase=phase;event.type=type;now+=100;return m.observe(p,123,1,now,true,event,0,9,held);};
        tick(1,0,false);tick(3,0,true);auto fumble=tick(3,32,false);assert(bool(fumble.id)==(owner==1||neutral));if(fumble.id)assert(fumble.event==live::Event::ForcedFumble&&fumble.side==1-owner);
        assert(!tick(3,32,false).id); // One current event, no repeated shake.
        tick(1,32,false);tick(3,32,true);assert(!tick(3,32,false).id); // Previous play's forced-fumble marker cannot retrigger.
        tick(3,0,true);auto fresh=tick(3,32,false);assert(bool(fresh.id)==(owner==1||neutral));
    }
    {live::Fumbles m;energy::Play p;p.game={2,7,7,1,400,720};p.down=1;p.offense=1;live::ForcedEvent e{999,0,true};
        m.observe(p,123,1,1000,true,e,0,2,false);p.game.phase=3;m.observe(p,123,1,1100,true,e,3,1,false);e.type=32;
        assert(!m.observe(p,123,1,1200,true,e,3,5,false).id); // Airborne/provisional fumble codes are insufficient.
        p.simulated=true;assert(!m.observe(p,123,1,1300,true,e,0,9,true).id);
    }
    live::ReturnAtmosphere returnSound;energy::Play p;p.game={1,0,0,3,700,720};p.offense=0;p.down=0;
    live::Outcome trigger{live::Event::KickReturn,1,0,40};sustained::Intent base;base.active=true;base.match=123;base.epoch=1;
    returnSound.observe(p,123,1,1000,true,trigger,true,false);
    for(uint64_t now=1000;now<=11000;now+=100){returnSound.observe(p,123,1,now,true,{},true,false);base.updated=now;auto sound=returnSound.merge(base,now,.9f);assert(sound.reason==sustained::Reason::Result&&sound.continuous&&sound.supporting&&sound.size==3&&sound.intensity>900);}
    p.game.phase=4;returnSound.observe(p,123,1,11100,true,{},false,false);base.updated=11100;assert(returnSound.merge(base,11100,.9f).reason==sustained::Reason::Result);
    returnSound.observe(p,123,1,16100,true,{},false,false);assert(returnSound.merge(base,16100,.9f).reason==sustained::Reason::None);
    p.game.phase=3;returnSound.observe(p,123,1,16200,true,trigger,true,false);base.celebration=1;assert(returnSound.merge(base,16200,.9f).reason==sustained::Reason::None);base.celebration=0;
    returnSound.observe(p,123,1,16300,true,{},true,true);assert(returnSound.merge(base,16300,.9f).reason==sustained::Reason::None);
    for(int side:{0,1})for(bool neutral:{false,true}){
        situational::Model kicks;energy::Play snap;snap.game={2,7,7,1,400,720};snap.game.neutral=neutral;snap.down=4;snap.offense=side;
        auto play=snap;energy::Outcome result;
        kicks.observe(play,snap,result,123,1,0,1000,true,0,2,NAN,NAN,false,21);
        play.game.phase=3;kicks.observe(play,snap,result,123,1,1,1100,true,0,2,NAN,NAN,false,21);
        play.game.phase=4;play.offense=1-side;play.down=1;result.accepted=result.completed=true;result.sequence=1;
        auto miss=kicks.observe(play,snap,result,123,1,1,1200,true,0,2,NAN,NAN,false,3);assert(miss.id&&miss.type==((side==0&&!neutral)?95:59));
    }
    {energy::Tracker tracker;energy::Settings settings;energy::Play kick;kick.game={2,7,7,1,400,720};kick.down=4;kick.offense=0;kick.toFirst=5;kick.toGoal=30;kick.resultStamp=1;
        tracker.observe(settings,kick);kick.game.phase=3;tracker.observe(settings,kick);kick.game.phase=4;kick.game.homeScore=10;kick.resultStamp=2;
        auto made=tracker.observe(settings,kick);assert(made.completed&&made.event==energy::Event::MadeFieldGoal);
        core::Model sound;core::Settings c;assert(!sound.react(c,made,10,kick.game,true));assert(sound.resultCheer&&sound.resultType==91); // Cheer, not an ordinary field-goal eruption.
    }
    std::puts("PASS: fresh native forced-fumble events, prior-play/airball/sim/polarity guards, continuous long-return roar and fade, eruption priority, close missed-field-goal home/away/neutral responses.");
}
