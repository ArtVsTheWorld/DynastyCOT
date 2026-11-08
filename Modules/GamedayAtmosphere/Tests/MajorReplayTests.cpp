#include "../Source/MajorReplay.h"
#include <cassert>
#include <cstdio>
using namespace attendance::majorreplay;
Input manual(uintptr_t match,unsigned epoch,uint64_t now,int phase,bool allowed,bool paused,bool replaying,bool dispatch=true){return {match,epoch,now,phase,allowed,paused,replaying,dispatch,Mode::PlayCall,true};}
int main(){
 assert(recordingAvailable(10,11)&&!recordingAvailable(-1,11)&&!recordingAvailable(10,10)&&!recordingAvailable(10,9)&&!recordingAvailable(10,191)&&!recordingAvailable(NAN,11));
 // A confirmed recorded TD can recover a missing authored request, once,
 // after the on-field presentation has yielded to a real play-call screen.
 for(bool proof:{false,true})for(bool ui:{false,true}){
  Model recovery;recovery.arm(123,1,1000,5,Kind::Touchdown,Mode::PlayCall);
  auto r=manual(123,1,2000,4,true,false,false);r.recordingReady=proof;r.playcallOpen=ui;
  assert(!recovery.tick(r));r.now=10000;assert(!recovery.tick(r));
  r.phase=5;r.now=11000;assert(!recovery.tick(r));r.now=11750;
  assert(recovery.tick(r)==(proof&&ui));if(proof&&ui)assert(recovery.takeNotice()==Notice::RecordedFallback);
  r.now=12000;assert(!recovery.tick(r));r.phase=3;r.now++;assert(!recovery.tick(r)&&recovery.stage()==Stage::Idle);
 }
 for(auto kind:{Kind::Touchdown,Kind::Takeaway}){
  Model fallback;fallback.arm(123,1,1000,1,kind,Mode::PlayCall);
  assert(fallback.suppressInset(123,1));auto f=manual(123,1,2000,5,true,false,false);
  assert(!fallback.tick(f));f.now=2749;assert(!fallback.tick(f));
  f.now=2750;assert(fallback.tick(f)&&fallback.takeNotice()==Notice::FallbackInset);
  f.now=3000;assert(!fallback.tick(f)&&fallback.suppressInset(123,1));
  f.replaying=true;assert(!fallback.tick(f)&&fallback.stage()==Stage::Inset);
 }
 // No destination or no intercepted native request means no early fallback.
 for(bool queued:{false,true})for(bool ui:{false,true}){
  if(queued&&ui)continue;
  Model fallback;fallback.arm(123,1,1000,1,Kind::Touchdown,Mode::PlayCall);
  if(queued)fallback.suppressInset(123,1);
  auto f=manual(123,1,2000,5,true,false,false);f.playcallOpen=ui;assert(!fallback.tick(f));
  f.now=5000;assert(!fallback.tick(f));
 }
 // A one-clip takeaway package may finish at the verified play-call destination.
 {Model one;one.arm(123,1,1000,1,Kind::Takeaway,Mode::PlayCall);one.suppressInset(123,1);
  one.node(123,1,100,false);auto f=manual(123,1,1200,4,true,false,true);one.tick(f);
  one.node(123,1,100,true);f=manual(123,1,2200,5,true,false,false);assert(!one.tick(f));
  f.now=2450;assert(one.tick(f));}
 for(auto kind:{Kind::Touchdown,Kind::Takeaway}){
  Model m;m.arm(123,1,1000,7,kind,Mode::PlayCall);assert(m.routing(123,1));
  assert(m.suppressInset(123,1));Input x=manual(123,1,1100,4,true,false,false);assert(!m.tick(x));
  m.node(123,1,456,false);x.now=1200;x.replaying=true;assert(!m.tick(x));assert(m.stage()==Stage::Full);
  m.node(123,1,456,true);x.now=2200;x.replaying=false;assert(!m.tick(x));
  if(kind==Kind::Takeaway){x.now=3000;assert(!m.tick(x));m.node(123,1,789,false);x.now=3100;x.replaying=true;assert(!m.tick(x));m.node(123,1,789,true);x.now=4100;x.replaying=false;assert(!m.tick(x));}
  x.now+=500;assert(!m.tick(x)); // An on-field celebration cannot receive PIP.
  x.phase=5;x.playcallOpen=false;x.now++;assert(!m.tick(x));
  x.playcallOpen=true;x.now++;assert(!m.tick(x));x.now+=249;assert(!m.tick(x));x.now++;assert(m.tick(x));assert(m.stage()==Stage::InsetRequested);
  assert(m.suppressInset(123,1));x.now+=50;assert(!m.tick(x));
  x.phase=5;x.replaying=true;x.now+=50;assert(!m.tick(x)&&m.stage()==Stage::Inset);
  x.replaying=false;x.now+=500;assert(!m.tick(x)&&m.stage()==Stage::Done);
  m.arm(123,1,x.now,7,kind,Mode::PlayCall);assert(m.stage()==Stage::Done); // Same result cannot replay.
  x.phase=3;x.now++;assert(!m.tick(x)&&m.stage()==Stage::Idle);
 }
 Model m;m.arm(123,1,1000,8,Kind::Touchdown,Mode::PlayCall);Input x=manual(123,1,2000,4,true,true,false);m.tick(x);x.now=122000;assert(!m.tick(x));x.paused=false;x.now++;assert(!m.tick(x)&&m.stage()==Stage::WaitingFull);
 x.allowed=false;x.now++;assert(!m.tick(x)&&m.stage()==Stage::Idle);
 m.arm(123,1,1000,9,Kind::Touchdown,Mode::PlayCall);x=manual(124,1,1100,4,true,false,false);assert(!m.tick(x)&&m.stage()==Stage::Idle);
 m.arm(123,1,1000,9,Kind::Touchdown,Mode::PlayCall);assert(!m.suppressInset(123,2));x=manual(123,1,61001,4,true,false,false);assert(!m.tick(x)&&m.stage()==Stage::WaitingFull);
 x.phase=5;x.now++;assert(!m.tick(x));x.now+=250;assert(!m.tick(x)&&m.stage()==Stage::Unavailable);
 m.reset();m.arm(123,1,1000,9,Kind::Touchdown,Mode::PlayCall);assert(m.suppressInset(123,1));x.phase=4;x.now++;assert(!m.tick(x));x.phase=5;x.now++;assert(!m.tick(x));x.now+=750;assert(m.tick(x));
 // Full playback can finish immediately before native play selection begins.
 m.reset();m.arm(123,1,1000,10,Kind::Touchdown,Mode::PlayCall);m.node(123,1,555,false);
 x=manual(123,1,1100,4,true,false,true);m.tick(x);m.node(123,1,555,true);
 x=manual(123,1,2000,5,true,false,false);assert(!m.tick(x));x.now=2250;assert(m.tick(x));
 // A selector refresh observes completion but cannot dispatch presentation events.
 m.reset();m.arm(123,1,1000,11,Kind::Touchdown,Mode::PlayCall);m.node(123,1,555,false);
 x=manual(123,1,1100,4,true,false,true);m.tick(x);m.node(123,1,555,true);
 x=manual(123,1,2000,4,true,false,false,false);m.tick(x);x.now=3000;assert(!m.tick(x));x.dispatch=true;assert(!m.tick(x));
 x.phase=5;x.now++;assert(!m.tick(x));x.now+=250;assert(m.tick(x));
 // Slow Sim and unknown UI keep full-screen promotion, never intercept Event12,
 // dispatch PIP, or emit a missing-inset timeout (including a 60s clip).
 for(auto mode:{Mode::RealtimeSim,Mode::Unknown}){
  m.reset();m.arm(123,1,1000,20,Kind::Touchdown,mode);
  assert(m.routing(123,1)&&!m.suppressInset(123,1));assert(m.takeNotice()==Notice::NativeOnly);
  m.node(123,1,99,false);x=manual(123,1,1100,4,true,false,true);x.mode=mode;x.playcallOpen=false;m.tick(x);
  x.now=80000;x.phase=5;assert(!m.tick(x)&&m.stage()!=Stage::Inset&&m.stage()!=Stage::Unavailable);
  m.node(123,1,99,true);x.replaying=false;x.now++;assert(!m.tick(x)&&m.stage()==Stage::Done);
 }
 // Stop pending manual work immediately on a mode switch. Returning to manual
 // does not resurrect an inset for the old play; the next play may arm one.
 m.reset();m.arm(123,1,1000,21,Kind::Touchdown,Mode::PlayCall);assert(m.suppressInset(123,1));
 x=manual(123,1,1200,4,true,false,true);x.mode=Mode::RealtimeSim;m.tick(x);assert(!m.suppressInset(123,1));
 x.mode=Mode::PlayCall;x.now=90000;x.replaying=false;assert(!m.tick(x)&&m.stage()!=Stage::Unavailable);
 m.arm(123,1,90001,22,Kind::Touchdown,Mode::PlayCall);assert(m.suppressInset(123,1));
 // No fallback can cut into an active native replay or a still-active node.
 x=manual(123,1,160002,4,true,false,true);assert(!m.tick(x)&&m.stage()!=Stage::Unavailable);
 x.replaying=false;m.node(123,1,99,false);x.now++;assert(!m.tick(x));m.node(123,1,99,true);
 // An authored inset that starts first is acknowledged rather than duplicated.
 x=manual(123,1,160004,5,true,false,true);assert(!m.tick(x)&&m.stage()==Stage::Inset);
 x.replaying=false;x.now++;assert(!m.tick(x)&&m.stage()==Stage::Done);
 // A phase number alone is not enough to infer play-call capability.
 attendance::replaymode::Tracker modes;using S=attendance::replaymode::Sample;
 assert(modes.update(123,1,S{true,false,false})==Mode::Unknown);
 assert(modes.update(123,1,S{true,false,true})==Mode::PlayCall);
 assert(modes.update(123,1,S{true,false,false})==Mode::PlayCall); // During the snap/full replay.
 assert(modes.update(123,1,S{true,true,true})==Mode::RealtimeSim); // Sim takes precedence.
 assert(modes.update(123,1,S{true,false,false})==Mode::Unknown); // No stale manual capability.
 assert(modes.update(123,1,S{true,false,true})==Mode::PlayCall);
 assert(modes.update(123,2,S{true,false,false})==Mode::Unknown);
 assert(modes.update(123,2,S{true,false,true})==Mode::PlayCall);
 assert(modes.update(123,2,S{})==Mode::Unknown);
 assert(modes.update(0,2,S{true,false,true})==Mode::Unknown);
 std::puts("PASS: major native full-screen -> separate inset order; two-clip takeaway gaps; no cutoff during playback; once-only PIP, pause exclusion, new snap/world/disable cleanup unavailable-recording fallback, Slow Sim/unknown UI, native inset adoption and mode-switch cleanup.");
}
