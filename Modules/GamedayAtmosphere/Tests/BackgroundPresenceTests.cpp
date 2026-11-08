#include "../Source/AttendanceAudio.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
 for(uint64_t seats:{6060ull,15000ull,20000ull,29271ull,45000ull,61888ull}){
  for(int f=1;f<=800;++f){auto mix=balance::mix(f*.001f,seats);
   if(f*.001f>=balance::crossover(seats))for(unsigned i=1;i<7;++i)assert(mix.net(i)==mix.net(0));
  }
 }
 auto audition=balance::mix(.94f,29271);
 for(unsigned i:{1u,2u,4u})assert(std::fabs(audition.net(i)-3.5f)<.005f);
 auto small=balance::mix(1,6060);for(unsigned i=0;i<7;++i)assert(std::fabs(small.net(i)-volume::boostDb(15))<.001f);
 auto packed=balance::mix(1,61888);for(unsigned i:{1u,2u,4u})assert(std::fabs(packed.net(i)-4.f)<.001f);
 for(unsigned i:{0u,3u,5u,6u})assert(std::fabs(packed.net(i)-volume::boostDb(30))<.001f);
 for(float strength:{0.f,25.f,50.f,100.f})for(float maximum:{0.f,10.f,17.5f,30.f}){
  auto mix=balance::mix(1,61888,maximum,strength);
  assert(mix.net(1)<=4.f*(maximum/30)*(strength/100)+.006f);
  if(!maximum||!strength)assert(!mix.needed());
 }
 // Both admission dimensions must qualify. Changing either one at the
 // start/end of its blend cannot create a level discontinuity.
 for(auto seats:{21740ull,25000ull,29271ull,30000ull,50000ull,61888ull}){
  float prior=-100;
  for(int f=1;f<=10000;++f){auto mix=balance::mix(f*.0001f,seats);float level=mix.net(1);
   assert(level>=prior-.0001f);if(f>1)assert(level-prior<.03f);prior=level;
  }
 }
 assert(balance::denseBackground(.8f,61888)==0&&balance::denseBackground(1,20000)==0);
 assert(balance::mix(0,61888).mute);
 std::puts("PASS: isolated +3.5dB anchor, +4dB largest packed background, unchanged reaction ceiling, ordinary/sparse invariance, smooth two-dimensional admission, strength/zero controls and native mute.");
}
