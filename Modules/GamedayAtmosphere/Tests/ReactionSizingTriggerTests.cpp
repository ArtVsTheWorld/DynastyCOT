#include "../Source/SpringReactions.h"
#include "../Source/CrowdPopulation.h"
#include "../Source/ReactionPalette.h"
#include "../Source/AwayCrowdAudio.h"
#include "../Source/AttendanceAudio.h"
#include <cassert>
#include <iostream>
using namespace attendance;
int main(){
 struct Case{uint64_t positions;float fullness;int expected;};
 for(const auto& c:{Case{2000,1,1},Case{6060,.49f,1},Case{6060,.5f,2},Case{100000,.09f,1},Case{100000,.10f,2},Case{15879,8897.f/15879,2},Case{15879,8630.f/15879,2},Case{30000,1,2},Case{60000,.60f,3},Case{60000,.59f,2},Case{60000,0,1}})assert(population::size(c.positions,c.fullness)==c.expected);
 assert(population::sizeForCount(2999,1)==1&&population::sizeForCount(3000,1)==2&&population::sizeForCount(8000,.09f)==1&&population::sizeForCount(8000,.8f)==2);
 energy::Play p;p.game={2,7,7,4,60,180};p.game.spring=true;p.game.springDynasty=77;p.game.springNumber=250;p.game.team0=1502;p.game.team1=p.game.springSchool=1145;p.offense=1;
 energy::Outcome r;r.accepted=r.completed=true;r.sequence=1;r.yards=0;
 for(int offense:{-1,0,1}){
  assert(spring::reaction(p,r,offense)==spring::Reaction::None);
  for(int failure=0;failure<5;++failure){auto f=r;f.takeaway=failure==0;f.sack=failure==1;f.turnoverOnDowns=failure==2;f.yards=failure==3?-1.f:0.f;if(failure==4)f.event=energy::Event::FourthStop;
   assert(spring::reaction(p,f,offense)==(offense==1?spring::Reaction::Disappointment:spring::Reaction::None));}
 }
 r.event=energy::Event::ThirdStop;r.yards=10;assert(spring::reaction(p,r,1)==spring::Reaction::None);r.event=energy::Event::None;assert(spring::reaction(p,r,1)==spring::Reaction::Cheer);
 r.yards=2;assert(spring::reaction(p,r,1)==spring::Reaction::None);r.forcedFumble=true;assert(spring::reaction(p,r,1)==spring::Reaction::None);r.forcedFumble=false;
 r.takeaway=true;spring::Reactions lease;auto selected=lease.observe(p,r,123,1,1,1000,true,1);assert(selected.selected&&selected.intent.resultType==102&&selected.intent.populationSize==2);
 assert(population::reactionType(selected.intent.resultType,2)==102&&population::reactionType(selected.intent.resultType,1)==101);
 r={};assert(lease.observe(p,r,123,1,1,1100,true,1).intent.resultCheer==1);assert(!lease.observe(p,r,123,1,1,1600,true,1).intent.resultCheer);
 r.accepted=r.completed=true;r.sequence=2;r.sack=true;assert(!lease.observe(p,r,123,1,2,1700,true,1,true).intent.active);
 // A routine supported palette request cannot silently stay small in a
 // medium audience after population and supporter clamps run.
 p.down=1;assert(palette::tier(p,p.game)==2);auto type=palette::resultType(r,p,p.game,false,2,1);assert(type%4>=2);
 visitors::Population pop;pop.verified=true;pop.match=123;pop.epoch=1;pop.originalHome=pop.home=60000;pop.originalAway=pop.away=8000;
 sustained::Intent w;w.active=true;w.match=123;w.epoch=1;w.resultCheer=1;w.resultCheerUntil=2000;w.resultType=90;
 auto audience=visitors::audience(w,pop,100000,.68f,1000);assert(audience.opponentSize==2&&audience.supporterSize==3);
 // Verified routing: only beds/chant/student children attenuate. Parent,
 // Reactions, anticipation, sweeteners and swells keep exact net gain.
 auto mix=balance::mix(.56f,15879),duck=balance::ambient(mix,.75f);
 for(unsigned i:{0u,3u,5u,6u})assert(duck.net(i)==mix.net(i));
 for(unsigned i:{1u,2u,4u})assert(duck.net(i)<mix.net(i)&&duck.net(i)>=mix.net(i)-12);
 auto empty=balance::mix(0,15879);assert(balance::ambient(empty,.75f).mute);
 std::cout<<"PASS: small/medium/large count and fullness boundaries, spring/visiting groups, snap-only boo triggers, failed conversion precedence, medium selectors, sparse clamps, fixed expiry and untouched native reaction/other bus levels. Audibility unverified.\n";
}
