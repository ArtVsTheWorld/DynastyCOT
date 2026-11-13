#include "../Source/AwayCrowd.h"
#include "../Source/VisitorPlans.h"
#include <cassert>
#include <iostream>
using namespace attendance;
int main(){
 const unsigned n=12800;std::vector<seats::Position> pos(n);agents::Tags tags;tags.verified=true;tags.type.assign(n,0);tags.fixed.assign(n,0);tags.side.resize(n);
 for(unsigned i=0;i<n;++i){unsigned row=i/160,col=i%160;pos[i]={35.f+float(row)*.7f,float(row)*.7f,float(col)*.65f-52,0};tags.side[i]=col>=120;}
 for(float share:{.03f,.05f,.08f,.15f}){visitors::seating::Stats old,now;bool fallback=false;
  auto a=visitors::seating::assign(pos,tags,share,73,nullptr,false,&old,true);auto b=visitors::seating::assign(pos,tags,share,73,nullptr,false,&now,true,&fallback,true);assert(a.size()==n&&b.size()==n&&!fallback);
  unsigned count=0;for(auto side:b)count+=side;assert(count==unsigned(std::llround(n*share))&&now.unassigned==0);
  double inner=now.innerSeats?double(now.innerVisitors)/now.innerSeats:0,edge=now.edgeSeats?double(now.edgeVisitors)/now.edgeSeats:0;
  assert(inner>.89&&edge>.65&&edge<.96&&double(now.core)/count>.55);unsigned middle=now.sizes[2]+now.sizes[3]+now.sizes[4];assert(middle>now.parties/2);
  std::cout<<"section|"<<share<<"|oldCore "<<old.core<<"|newCore "<<now.core<<"|inner "<<inner<<"|edge "<<edge<<"|parties "<<now.parties<<"|2to4 "<<middle<<'\n';
  // The fixed authored identities survive deterministic arrivals; no new
  // visibility policy or supporters are introduced by the seating algorithm.
  assert(b==visitors::seating::assign(pos,tags,share,73,nullptr,false,nullptr,true,nullptr,true));
  for(unsigned i=0;i<n;++i)if(b[i]&&!now.coreMask[i]){
   assert(now.party[i]>=0);for(unsigned j:{i%160?i-1:i,i%160<159?i+1:i,i>=160?i-160:i,i+160<n?i+160:i})
    if(b[j]&&!now.coreMask[j])assert(now.party[i]==now.party[j]);
  }
  for(unsigned arrival:{50u,90u,100u}){unsigned coreVisible=0,visitorsVisible=0;
   for(unsigned i=0;i<n;++i)if(b[i]&&fanHash(i+977)%100<arrival){++visitorsVisible;coreVisible+=now.coreMask[i]!=0;}
   assert(coreVisible>visitorsVisible/2);std::cout<<"arrivalProxy|"<<share<<'|'<<arrival<<'|'<<coreVisible<<'|'<<visitorsVisible<<'\n';
  }
 }
 // A small native anchor must not cap a larger supported modeled section.
 auto thin=tags;for(unsigned i=0;i<n;++i)thin.side[i]=i%160>=155;
 visitors::seating::Stats grown;bool retained=false;auto expanded=visitors::seating::assign(pos,thin,.08f,73,nullptr,false,&grown,true,&retained,true);
 assert(!retained&&grown.unassigned==0&&grown.core>n*.08*.55&&expanded.size()==n);
 for(double ratio:{.2,.5,1.,2.})for(double travel:{.16,1.,1.7}){visitormodel::Inputs d;d.homeFans=d.homeTickets=70000;d.awayFans=70000*ratio;d.homePrestige=8;d.awayPrestige=8;d.travelFactor=travel;d.seed=73;
  auto old=visitormodel::demand(d),now=visitormodel::demand(d,100,true);assert(old.valid&&now.valid&&now.share>=old.share&&now.share<=.18);
  if(old.share>=.12)assert(now.share-old.share<.001);std::cout<<"share|"<<ratio<<'|'<<travel<<'|'<<old.share<<'|'<<now.share<<'\n';
  d.neutral=true;assert(visitormodel::demand(d).share==visitormodel::demand(d,100,true).share);
 }
 std::cout<<"PASS concentration, connected native anchors, exact totals, ordinary tapered lift and unchanged neutral support; no game access.\n";
}
