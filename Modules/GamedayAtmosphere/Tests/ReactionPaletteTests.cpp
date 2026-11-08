#include "../Source/ReactionPalette.h"
#include "../Source/CrowdPopulation.h"
#include <cassert>
#include <set>
#include <cstdio>
using namespace attendance;
int main(){
 unsigned count=0;for(int id=0;id<112;++id){bool expected=id%4!=0&&id!=13&&id!=14&&id!=15&&id!=23;assert(catalog::usable(id)==expected);count+=expected;}
 assert(count==80&&!catalog::usable(-1)&&!catalog::usable(112));
 for(int id=0;id<112;++id)for(int tier=1;tier<=3;++tier){auto mapped=population::reactionType(id,tier);assert(!mapped||catalog::usable(mapped));if(mapped)assert(mapped/4==id/4&&mapped%4<=tier&&mapped%4<=id%4);}
 assert(population::reactionType(23,3)==22&&population::reactionType(13,3)==0);
 energy::Play snap;snap.game={2,7,7,1,400,720};snap.offense=0;snap.down=1;snap.selectedType=103;
 auto critical=snap;critical.down=3;std::set<int> reached;
 auto collect=[&](int requested){assert(catalog::usable(requested));for(auto seats:{6060ull,20000ull,61888ull})for(float fullness:{.25f,.5f,.9f,1.f}){int id=population::reactionType(requested,population::size(seats,fullness));assert(catalog::usable(id));reached.insert(id);}};
 // Context-to-family coverage uses the actual palette, not a direct catalog loop.
 for(uint64_t play=1;play<=100;++play){for(bool negative:{false,true})for(auto state:{snap,critical}){
  for(auto event:{live::Event::Truck,live::Event::Juke,live::Event::LongCatch,live::Event::LongRun,live::Event::Interception,live::Event::FumbleRecovery,live::Event::Sack,live::Event::PassBreakup,live::Event::TackleForLoss,live::Event::OvertimeFieldGoal}){
   for(float yards:{5.f,20.f,40.f}){live::Outcome out{event,play,negative?1:0,yards};int type=palette::liveType(out,state,state.game,play,1,negative);assert(type==palette::liveType(out,state,state.game,play,1,negative));collect(type);}
  }
  energy::Outcome td;td.accepted=td.completed=true;td.event=energy::Event::HomeScore;td.side=negative?1:0;if(negative)td.awayPoints=6;else td.homePoints=6;
  collect(palette::resultType(td,state,state.game,true,play,1,negative));collect(palette::resultType(td,state,state.game,false,play,1,negative));
 }
 collect(palette::lineup(play,play+1,1));collect(palette::situationType({play,103,situational::Event::AwayShowboat},critical,snap.game,play,1));}
 for(int id=1;id<112;++id)if(catalog::usable(id)){if(!reached.count(id))std::fprintf(stderr,"Missing selector %d\n",id);assert(reached.count(id));}assert(reached.size()==80);
 assert(population::size(128000,.25f)==1&&population::size(128000,.50f)==2);
 assert(population::size(61888,.90f)==3&&population::size(6060,1)==1);
 assert(palette::tier(snap,snap.game)==1&&palette::tier(critical,snap.game)==2);
 // A packed venue must not promote a routine play into a maximum reaction.
 for(bool negative:{false,true})for(uint64_t play=1;play<=100;++play){
  for(auto event:{live::Event::BrokenTackle,live::Event::CompletedPass,live::Event::TackleForLoss}){
   int small=palette::liveType({event,play,0,5},snap,snap.game,play,1,negative);
   assert(small%4==1&&population::reactionType(small,3)==small);
  }
  energy::Outcome gain;gain.firstDown=true;gain.yards=10;
  assert(palette::resultType(gain,snap,snap.game,true,play,1,negative)%4==1);
  gain.yards=25;assert(palette::resultType(gain,snap,snap.game,true,play,1,negative)%4==3);
  gain={};gain.event=energy::Event::ThirdStop;
  assert(palette::resultType(gain,critical,critical.game,false,play,1,negative)%4==2);
  gain.turnoverOnDowns=true;assert(palette::resultType(gain,critical,critical.game,false,play,1,negative)%4==3);
 }
 assert(population::reactionType(palette::liveType({live::Event::LongCatch,1,0,20},critical,snap.game,1,1,true),3)==22);
 energy::Outcome score;score.side=0;score.homePoints=6;score.takeaway=true;assert(palette::resultType(score,snap,snap.game,true,1,1)/4==8); // A pick-six is not a receiving catch.
 // OT visitor scores keep their semantic family instead of the old generic aww.
 energy::Outcome visitor;visitor.side=1;visitor.awayPoints=6;visitor.event=energy::Event::HomeScore;
 assert(palette::situationType({1,95,situational::Event::OvertimeVisitorScore},snap,snap.game,1,1,&visitor,true)/4==17);
 visitor.awayPoints=3;assert(palette::situationType({1,95,situational::Event::OvertimeVisitorScore},snap,snap.game,1,1,&visitor,false)/4==7);
 for(auto event:{live::Event::Juke,live::Event::Spin,live::Event::BrokenTackle})assert(!live::cameraReaction(event));
 std::puts("PASS: all80 audible selectors reachable through existing semantic palette; four exclusions, deterministic variety, importance/fullness/occupied-seat ceilings, neutral-compatible families and unchanged sound-only skill motion.");
}
