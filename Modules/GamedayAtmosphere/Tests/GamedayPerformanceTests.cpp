#include "../Source/AwayCrowd.h"
#include "LegacySeatPattern.h"
#include "LegacyVisitorSeating.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <numeric>
#include <limits>
using Clock=std::chrono::steady_clock;
static volatile unsigned sink=0;
static std::vector<attendance::seats::Position> bowl(unsigned n){
 std::vector<attendance::seats::Position> out(n);
 for(unsigned i=0;i<n;++i){unsigned stand=i%4,row=(i/4)/128,col=(i/4)%128;float along=(float(col)-64)*.7f,depth=28+row*.7f,height=1+row*.4f;
  out[i]=stand<2?attendance::seats::Position{stand?depth:-depth,height,along,0}:attendance::seats::Position{along,height,stand==2?depth+28:-depth-28,0};}
 return out;
}
template<class F>static double median(F action){std::vector<double> ms;for(int i=0;i<7;++i){auto at=Clock::now();action();ms.push_back(std::chrono::duration<double,std::milli>(Clock::now()-at).count());}std::sort(ms.begin(),ms.end());return ms[ms.size()/2];}
int main(){
 using namespace attendance;
 unsigned checks=0;
 for(int seed:{0,17,999})for(double scale:{5.,9.,55.}){seats::NoiseCache cache(scale,seed);for(int k=0;k<2500;++k){double x=(k%137-68)*.37,z=(k/137-9)*1.13;assert(cache.sample(x,z)==seats::noise(x,z,scale,seed));++checks;}}

 for(unsigned n:{1024u,8192u,32768u})for(unsigned seed:{0u,17u,991u})for(unsigned fill:{100u,73u,35u}){
  auto p=bowl(n);std::vector<baselineSeats::Position> old(n);for(unsigned i=0;i<n;++i)old[i]={p[i].x,p[i].y,p[i].z,p[i].packed};
  std::vector<unsigned short> masks(n);for(unsigned i=0;i<n;++i)masks[i]=fanHash(i+11)%100<fill?0:7;
  auto a=seats::build(p,masks,7,seed);auto b=baselineSeats::build(old,masks,7,seed);
  assert(int(a.policy)==int(b.policy)&&a.original==b.original&&a.rank==b.rank);++checks;
 }
 // Invalid inputs continue to use the native/scattered fallback.
 for(float bad:{std::numeric_limits<float>::quiet_NaN(),10000.f}){auto p=bowl(1024);p[0].x=bad;std::vector<unsigned short> mask(1024);assert(seats::build(p,mask,7).policy==seats::Policy::Scattered);++checks;}
 for(unsigned n:{1024u,8192u,32768u})for(bool neutral:{false,true})for(float share:{.03f,.15f,.35f}){
  auto p=bowl(n);agents::Tags tags;tags.verified=true;tags.type.assign(n,0);tags.side.assign(n,0);tags.fixed.assign(n,0);
  // Use the exact native ordinary-fan enum, with protected groups mixed in.
  for(unsigned i=0;i<n;++i){tags.type[i]=i%97==0?10:i%61==0?1:0;tags.side[i]=i%5==0;tags.fixed[i]=tags.type[i]==10;}
  visitors::seating::Stats a;visitors::legacySeating::Stats b;
  auto x=visitors::seating::assign(p,tags,share,99,nullptr,neutral,&a),y=visitors::legacySeating::assign(p,tags,share,99,nullptr,neutral,&b);
  if(share==.15f)assert(x.size()==n);
  assert(x==y&&a.core==b.core&&a.scattered==b.scattered&&a.parties==b.parties&&a.unassigned==b.unassigned&&a.sizes==b.sizes&&a.coreMask==b.coreMask&&a.party==b.party);++checks;
 }
 auto p=bowl(32768);std::vector<baselineSeats::Position> old(p.size());for(unsigned i=0;i<p.size();++i)old[i]={p[i].x,p[i].y,p[i].z,0};std::vector<unsigned short> mask(p.size());
 double before=median([&](){sink=baselineSeats::build(old,mask,7,99).original;});double after=median([&](){sink=seats::build(p,mask,7,99).original;});
 std::printf("BENCH|seat-pattern|%.6f|%.6f|%.2f\n",before,after,100*(1-after/before));
 agents::Tags tags;tags.verified=true;tags.type.assign(p.size(),0);tags.side.assign(p.size(),0);tags.fixed.assign(p.size(),0);
 for(bool neutral:{false,true}){
  before=median([&](){sink=static_cast<unsigned>(visitors::legacySeating::assign(p,tags,.15f,99,nullptr,neutral).size());});
  after=median([&](){sink=static_cast<unsigned>(visitors::seating::assign(p,tags,.15f,99,nullptr,neutral).size());});
  std::printf("BENCH|%s-visitors|%.6f|%.6f|%.2f\n",neutral?"neutral":"home",before,after,100*(1-after/before));
 }
 std::printf("PASS %u exact seating/rank/party comparisons and invalid-input checks; offline timings, not FPS.\n",checks);
}
