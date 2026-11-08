#include "../Source/DirectorShake.h"
#include <cassert>
#include <cmath>
#include <cstdio>
using namespace attendance::motion;
int main(){
 for(bool neutral:{false,true})for(bool eruption:{false,true}){
  Settings c;c.live=5;Model m;Input x;x.match=123;x.epoch=1;x.now=1000;x.phase=3;x.allowed=x.duringAllowed=true;x.duringPlay=1;x.neutral=neutral;
  m.observe(c,x);x.now=1300;auto before=m.observe(c,x);float from=strength(before,x.now);assert(from>0);
  auto oscillation=before.oscillationBegan; x.phase=4;x.now=1350;x.duringPlay=0;x.eruptionAlive=eruption;if(eruption)x.eruption=1;else x.touchdown=1;
  auto cue=m.observe(c,x);float start=strength(cue,x.now);assert(cue.scoreBuildup&&std::fabs(start-from)<.001f&&cue.oscillationBegan==oscillation);
  float previous=start,target=(eruption?c.eruptionImpact:c.touchdownImpact)*(neutral?.75f:1.f);
  for(unsigned dt=10;dt<=1800;dt+=10){x.now=1350+dt;cue=m.observe(c,x);float next=strength(cue,x.now);
   assert(next<=target+.001f);if(dt<=300)assert(next+.001f>=previous);if(dt==300)assert(std::fabs(next-target)<.001f);previous=next;
  }
 }
 // An isolated touchdown still has its configured impact; suspension/world changes cannot inherit old motion.
 for(int reason=0;reason<3;++reason){Settings c;Model m;Input x;x.match=123;x.epoch=1;x.now=1000;x.phase=3;x.allowed=true;x.duringPlay=1;m.observe(c,x);x.now=1300;m.observe(c,x);
  if(reason==0){x.allowed=false;x.now=1350;m.observe(c,x);x.allowed=true;}if(reason==1)++x.epoch;if(reason==2)++x.match;
  x.phase=4;x.now=1400;x.touchdown=1;auto cue=m.observe(c,x);assert(!cue.scoreBuildup);x.now=1500;cue=m.observe(c,x);assert(strength(cue,x.now)>c.touchdown);
 }
 std::puts("PASS: gain-to-peak score handoff rises monotonically in 300ms without phase reset; TD/eruption, neutral scale, isolated impacts and suspension/world guards.");
}
