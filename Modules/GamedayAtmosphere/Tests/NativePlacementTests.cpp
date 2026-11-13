#include "../Source/AwayCrowd.h"
#include <chrono>
#include <cstdio>
#include <cassert>
using namespace attendance;
int main(){for(unsigned n:{12800u,100000u}){std::vector<seats::Position> pos(n);agents::Tags tags;tags.verified=true;tags.type.assign(n,0);tags.fixed.assign(n,0);tags.side.resize(n);
for(unsigned i=0;i<n;++i){unsigned stand=i/12800,row=(i%12800)/160,col=i%160;pos[i]={float(col)*.65f+float(stand%2)*120-110,float(row)*.7f,float(row)*.9f+float(stand/2)*80-150,0};tags.side[i]=col>=80;}
for(bool neutral:{false,true})for(float share:{.3f,.5f,.7f}){if(!neutral&&share>.35f)continue;for(bool reverse:{false,true}){auto native=tags;if(reverse)for(auto& side:native.side)side^=1;visitors::seating::Stats stats;auto start=std::chrono::steady_clock::now();auto result=visitors::seating::assign(pos,native,share,73,nullptr,neutral,&stats,true);auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();assert(result.size()==n);unsigned count=0,aligned=0;for(unsigned i=0;i<n;++i){count+=result[i];aligned+=result[i]==native.side[i];}assert(count==unsigned(std::llround(n*share)));assert(aligned>n/2);assert(stats.sizes[0]==0);std::printf("native placement|n %u|neutral %d|reverse %d|share %.2f|aligned %.3f|core %u|parties %u|ms %.3f\n",n,neutral,reverse,share,double(aligned)/n,stats.core,stats.parties,ms);}}
for(unsigned i=0;i<n;++i)tags.side[i]=((i%12800)/160+i%160)%2;assert(visitors::seating::assign(pos,tags,.12f,73,nullptr,false,nullptr,true)==tags.side);}
std::puts("PASS native side anchors, reversed participants, dense championship geometry, exact totals and unverified-layout native fallback; no game access.");}
