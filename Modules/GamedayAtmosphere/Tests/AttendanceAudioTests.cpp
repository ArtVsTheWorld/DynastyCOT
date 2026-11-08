#include "../Source/AttendanceAudio.h"
#include <cassert>
#include <cstdio>
#include <limits>
using namespace attendance;
int main(){
 auto small=balance::mix(.25f,6060);auto half=balance::mix(.5f,6060);
 assert(std::fabs(std::pow(10.f,small.net(1)/20)-.5f)<.00001f);
 for(float boundary:{0.f,5.f/14,1.f}){
  float lo=balance::sparseProgress((std::max)(0.f,boundary-.0001f));
  float hi=balance::sparseProgress((std::min)(1.f,boundary+.0001f));
  assert(hi>=lo&&hi-lo<.001f);
 }
 assert(balance::mix(.7f,6060).needed()==false);
 assert(balance::mix(.9f,6060).net(1)>0);
 assert(balance::mix(.5f,61888).net(1)>0&&half.net(1)<0);
 assert(!balance::mix(.5f,0).valid&&!balance::mix(-1,6060).valid&&!balance::mix(std::numeric_limits<float>::quiet_NaN(),6060).valid);
 for(uint64_t seats:{6060,12000,20000,30000,61888}){
  auto zero=balance::mix(0,seats);assert(zero.mute&&zero.needed()&&zero.gain[0]==-60);
  auto crossing=balance::mix(balance::crossover(seats),seats);assert(!crossing.needed());
  std::array<float,7> last;last.fill(-1000);
  for(int n=1;n<=1000;++n){auto m=balance::mix(n*.001f,seats);assert(m.valid&&!m.mute);
   for(unsigned i=0;i<7;++i){bool bed=i==1||i==2||i==4;assert(m.net(i)<=(bed?4.f:volume::boostDb(30))+.0001f&&m.gain[i+1]<=.001f);}
   assert(m.net(1)==m.net(2));for(unsigned i=0;i<7;++i){assert(m.net(i)>=last[i]-.00001f);assert(std::pow(10.f,m.net(i)/20)>=balance::SignalFloors[i]-.00001f);last[i]=m.net(i);}
  }
  auto off=balance::mix(1,seats,0);assert(!off.needed());
  auto capped=balance::mix(1,seats,17.5f);assert(capped.net(3)<=volume::boostDb(17.5f)+.0001f&&capped.net(1)<=4.f*17.5f/30+.003f);
 }
 assert(std::fabs(balance::mix(1,61888).net(1)-4.f)<.0001f);
 assert(std::fabs(balance::mix(1,61888).net(3)-volume::boostDb(30))<.0001f);
 for(float strength:{0.f,50.f,100.f,125.f,150.f,200.f})for(float fullness:{.0001f,.25f,.7f,1.f})for(uint64_t seats:{6060,61888}){
  auto normal=balance::mix(fullness,seats,30,strength),hostile=balance::mix(fullness,seats,30,strength,true);
  for(unsigned i=0;i<7;++i){assert(normal.net(i)<=8.001f&&hostile.net(i)<=8.001f);assert(std::pow(10.f,normal.net(i)/20)>=balance::SignalFloors[i]-.0001f);}
  if(strength==0)assert(!normal.needed()&&!hostile.needed());
 }
 assert(balance::mix(1,61888,30,200).key==balance::mix(1,61888,30,100).key);
 assert(balance::mix(1,61888,30,200).net(3)==balance::mix(1,61888,30,100).net(3));
 assert(balance::mix(1,61888,30,100,true).net(1)>5.99f);
 assert(balance::mix(.25f,6060,30,200).net(1)==balance::mix(.25f,6060).net(1));
 std::puts("PASS: rounded signal floors and smooth 25%-full half-volume anchor, occupied-size crossover, chant parity, native dynamics gain bounds, zero mute and invalid-context fallback.");
}
