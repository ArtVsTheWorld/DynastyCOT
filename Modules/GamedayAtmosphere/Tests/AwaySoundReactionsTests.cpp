#include "../Source/AwaySoundReactions.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
struct Fixture {
 awaysound::Model model;energy::Settings energy;live::Settings live;energy::Play p;skills::Sample sample;awaysound::Samples extra;uint64_t time=1000,sequence=1;bool allowed=true;
 Fixture(int offense=1,bool neutral=false){p.game={2,7,7,1,400,720};p.game.neutral=neutral;p.offense=offense;p.down=1;p.selectedType=103;p.toGoal=60;p.toFirst=10;p.fieldSpot=-10;p.attackDirection=1;}
 awaysound::Outcome tick(int phase,int kind=0,int ball=2,float spot=-10,bool held=true){p.game.phase=phase;time+=100;extra.worldZ=spot;if(phase==3&&kind==3&&ball==1){p.passReleased=true;p.resultStamp=1;}model.track(energy,p,sequence);return model.observe(live,p,123,1,sequence,time,allowed,456,kind,ball,spot,spot,held,false,sample,extra);}
 void begin(){tick(1);tick(3);}
};
int main(){
 for(bool neutral:{false,true}){Fixture f(1,neutral);f.begin();f.tick(3,3,1,10,false);auto catchSound=f.tick(3,1,4,10);assert(bool(catchSound.type)==!neutral);if(neutral)assert(f.model.extraType()==0&&!f.model.motion().id&&!f.model.buildup().id);if(!neutral)assert(catchSound.type/4==5);assert(!f.tick(3,1,4,20).type);
  f.p.game.awayScore+=6;f.p.game.seconds-=8;auto score=f.tick(4,1,4,50);assert(bool(score.type)==!neutral);if(!neutral)assert(score.scoring&&score.type/4==17);assert(!f.tick(5).type);
 }
 // The existing visitor-deep-catch route can speak before the20-yard shadow
 // threshold. Cover it across ticks without blocking a later verified TD.
 {Fixture f;f.begin();f.tick(3,3,1,5,false);f.model.cover(f.sequence);
  assert(!f.tick(3,1,4,10).type);assert(!f.tick(3,1,4,20).type);
  f.p.game.awayScore+=6;f.p.game.seconds-=8;auto score=f.tick(4,1,4,50);assert(score.scoring&&score.type/4==17);assert(!f.tick(5).type);
  ++f.sequence;f.p.resultStamp+=1;f.begin();f.tick(3,3,1,10,false);assert(f.tick(3,1,4,10).type);}
 {Fixture f;f.begin();f.model.cover(f.sequence,true);f.p.game.awayScore+=6;f.p.game.seconds-=8;assert(!f.tick(4).type);}
 // A home success cannot produce its opponent's disappointment counterpart.
 {Fixture f(0);f.begin();f.tick(3,3,1,10,false);assert(!f.tick(3,1,4,10).type);f.p.game.homeScore+=6;f.p.game.seconds-=8;assert(!f.tick(4,1,4,50).type);}
 // A home third-down incompletion becomes the existing away defensive-stop sound.
 {Fixture f(0);f.p.down=3;f.begin();f.tick(3,3,1,0,false);f.p.down=4;f.p.game.seconds-=5;auto out=f.tick(4,3,1,0,false);assert(out.type/4==11);assert(!f.tick(5).type);}
 for(int unavailable:{0,1,2,3}){Fixture f;f.allowed=unavailable!=0;f.p.simulated=unavailable==1;f.live.enabled=unavailable!=2;f.p.selectedType=unavailable==3?37:103;f.begin();f.tick(3,3,1,10,false);assert(!f.tick(3,1,4,10).type);f.allowed=true;f.p.simulated=false;f.live.enabled=true;assert(!f.tick(4,1,4,10).type);}
 // No fresh ready/live proof after a discontinuity; no score from a stale graph.
 {Fixture f;f.begin();f.model.reset();f.tick(3,3,1,10,false);assert(!f.tick(3,1,4,10).type);f.p.game.awayScore+=6;assert(!f.tick(4).type);}
 // Main and shadow serials may differ after a gap; each proof stays on current main play.
 {Fixture f;f.sequence=31;f.begin();f.p.game.awayScore+=6;f.p.game.seconds-=5;auto out=f.tick(4);assert(out.scoring&&out.type/4==19);assert(!f.tick(5).type);}
 // Mirrored live gain keeps independent motion proof even when an existing negative sound covered it.
 {Fixture f;f.begin();f.model.cover(f.sequence);f.tick(3,3,1,0,false);f.tick(3,1,4,0);f.tick(3,1,4,6);
  auto motion=f.model.motion();assert(motion.id&&motion.side==1);}
 // Shadow kick-return and fumble proofs are native detectors, not invented triggers.
 for(int side:{0,1}){Fixture f(side);f.begin();f.p.game.seconds-=5;f.p.game.phase=4;f.p.down=side?1:4;
  auto out=f.tick(4);(void)out;assert(!f.model.motion().id);}
 {Fixture f(0);f.extra.selectedType=23;f.p.selectedType=23;f.p.down=0;
  f.tick(1,0,2,-35,true);f.tick(3,0,1,-35,false);f.tick(3,0,1,0,false);f.p.offense=1;
  f.tick(3,0,2,20,true);awaysound::Outcome sound;for(int spot=19;spot>=-15;--spot)sound=f.tick(3,0,2,static_cast<float>(spot),true);
  assert(sound.type&&catalog::negative(sound.type)&&f.model.motion().event==live::Event::KickReturn);}
 for(bool own:{false,true}){Fixture f(own?1:0);f.begin();f.p.fumble=true;f.extra.fumble=true;f.tick(3,0,5,-10,false);
  f.p.fumble=false;f.extra.fumble=false;f.p.offense=1;auto recovery=f.tick(3,0,2,-10,true);
  assert((f.model.motion().event==live::Event::FumbleRecovery)==!own);
  if(!own)assert(recovery.type&&recovery.type/4==9);
 }
 // A home-offense loss routes to the visiting defense even though the
 // primary home-only result classifier does not call it a defensive event.
 {Fixture f(0);f.p.resultStamp=10;f.begin();f.p.down=2;f.p.resultStamp=11;f.p.fieldSpot=-15;f.p.toFirst=15;f.p.toGoal=65;f.p.game.seconds-=3;
  f.tick(4,0,2,-15);auto event=f.model.motion();assert(event.event==live::Event::TackleForLoss&&event.side==1);
  assert(!f.tick(4,0,2,-15).scoring&&!f.model.motion().id);}
 std::puts("PASS: away-only existing-trigger counterparts, live-to-score upgrade once, split-crowd/home ownership, disabled/sim/kneel/gap exclusion and main-play sequence alignment.");
}
