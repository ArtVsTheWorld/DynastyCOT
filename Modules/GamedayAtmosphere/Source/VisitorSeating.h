#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <queue>
#include "SeatPattern.h"
#include "CrowdAgents.h"
namespace attendance::visitors::seating {
struct Cell {int x,y,z;bool operator==(const Cell& o)const{return x==o.x&&y==o.y&&z==o.z;}};
struct Hash {size_t operator()(const Cell& c)const{return fanHash(c.x)^(size_t(fanHash(c.y))<<1)^(size_t(fanHash(c.z))<<2);}};
struct Summary {unsigned core=0,scattered=0,parties=0,unassigned=0,footprint=0,innerSeats=0,innerVisitors=0,edgeSeats=0,edgeVisitors=0,blocks=0;std::array<unsigned,9> sizes{};};
struct Stats : Summary {std::vector<unsigned char> coreMask;std::vector<int> party;};

// Label the physical components first. A separate boundary index is used only
// to pick the next block; an aisle contact never becomes a seat or party edge.
inline unsigned growCluster(const std::vector<seats::Position>& positions,const std::vector<unsigned>& seats,
 const std::vector<std::vector<unsigned>>& edges,const std::vector<unsigned char>& boundary,
 const std::vector<double>& scores,const std::vector<unsigned>& anchor,unsigned start,unsigned seed,
 unsigned footprint,std::vector<unsigned char>& core){
 if(!footprint)return 0;
 std::vector<unsigned> blockOf(positions.size(),UINT32_MAX),sizes,work;
 auto boundaryCell=[](const seats::Position& p){return Cell{int(std::floor(p.x/6.0)),int(std::floor(p.y/2.0)),int(std::floor(p.z/6.0))};};
 std::unordered_map<Cell,std::vector<unsigned>,Hash> boundaries;
 for(unsigned first:seats)if(blockOf[first]==UINT32_MAX){
  unsigned id=static_cast<unsigned>(sizes.size());work.clear();work.push_back(first);blockOf[first]=id;
  for(size_t k=0;k<work.size();++k){
   unsigned i=work[k];if(boundary[i])boundaries[boundaryCell(positions[i])].push_back(i);
   for(unsigned j:edges[i])if(blockOf[j]==UINT32_MAX){blockOf[j]=id;work.push_back(j);}
  }
  sizes.push_back(static_cast<unsigned>(work.size()));
 }
 double x=0,y=0,z=0;
 for(unsigned i:anchor){x+=positions[i].x;y+=positions[i].y;z+=positions[i].z;}
 x/=anchor.size();y/=anchor.size();z/=anchor.size();
 auto radius=[&](unsigned i){const auto& p=positions[i];return (p.x-x)*(p.x-x)+2*(p.y-y)*(p.y-y)+(p.z-z)*(p.z-z);};
 using Entry=std::pair<double,unsigned>;
 std::priority_queue<Entry> nearby,frontier;
 std::vector<unsigned char> visited(sizes.size()),queued(positions.size());
 std::vector<double> best(sizes.size(),std::numeric_limits<double>::infinity());
 nearby.push({0,start});
 unsigned placed=0,blocks=0;
 while(!nearby.empty()&&placed<footprint){
  unsigned first=nearby.top().second;nearby.pop();unsigned id=blockOf[first];
  if(visited[id])continue;
  visited[id]=1;++blocks;frontier={};frontier.push({scores[first],first});queued[first]=1;work.clear();
  // Leave an uneven fringe on the large blocks that make up a section.
  unsigned limit=sizes[id];
  if(limit<footprint&&limit>=32)limit=limit*(84+fanHash(first^seed)%12)/100;
  while(!frontier.empty()&&work.size()<limit&&placed<footprint){
   unsigned i=frontier.top().second;frontier.pop();core[i]=1;++placed;
   work.push_back(i);
   for(unsigned j:edges[i])if(!queued[j]){queued[j]=1;frontier.push({scores[j]-.0008*radius(j),j});}
  }
  if(placed==footprint)break;
  // Only the occupied footprint can nominate a neighbor. No remote stand or
  // vertically stacked deck is admitted on the strength of its team colors.
  for(unsigned i:work)if(boundary[i]){
   const auto& p=positions[i];Cell c=boundaryCell(p);
   for(int dx=-1;dx<=1;++dx)for(int dy=-1;dy<=1;++dy)for(int dz=-1;dz<=1;++dz){
    auto hit=boundaries.find({c.x+dx,c.y+dy,c.z+dz});if(hit==boundaries.end())continue;
    for(unsigned j:hit->second){
     unsigned next=blockOf[j];if(visited[next]||sizes[next]<16)continue;
     const auto& q=positions[j];double height=q.y-p.y,horizontal=(q.x-p.x)*(q.x-p.x)+(q.z-p.z)*(q.z-p.z);
     if(std::abs(height)>2||horizontal>36)continue;
     double cost=radius(j)+4*(horizontal+height*height);
     if(cost>=best[next])continue;
     best[next]=cost;nearby.push({-cost,j});
    }
   }
  }
 }
 return blocks;
}

inline std::vector<unsigned char> assign(const std::vector<seats::Position>& pos,const agents::Tags& tags,float share,unsigned seed,const std::vector<unsigned char>* occupied,bool neutral,Stats* stats=nullptr,bool nativeAnchors=false,bool* nativeFallback=nullptr,bool concentrated=false,bool clustered=false,Summary* summary=nullptr){
 if(nativeFallback)*nativeFallback=false;
 if(summary)*summary={};
 if(stats)*stats={};
 auto fallback=[&](){if(nativeFallback)*nativeFallback=true;return tags.side;};
 auto side=tags.side;if(!tags.verified||pos.size()>200000||pos.size()!=side.size()||tags.type.size()!=side.size()||tags.fixed.size()!=side.size()||!std::isfinite(share)||(clustered?share<.01f:share<.01)||share>.70||(occupied&&occupied->size()!=side.size()))return {};
 const bool cluster=clustered&&concentrated&&nativeAnchors&&!neutral;
 const size_t n=pos.size();std::vector<unsigned> seats;unsigned count=0,fixed=0;bool flip=neutral&&share>.5f;
 for(unsigned i=0;i<n;++i)if(!tags.protectedBand(i)&&(!occupied||(*occupied)[i])){++count;if(agents::regular(tags.type[i])&&!tags.protectedFan(i))seats.push_back(i);else fixed+=flip?!side[i]:side[i];}
 unsigned desired=static_cast<unsigned>(std::llround(count*(flip?1-share:share))),target=(std::min)(static_cast<unsigned>(seats.size()),desired>fixed?desired-fixed:0);
 if(cluster&&(fixed>desired||desired-fixed>seats.size()))return fallback();
 std::vector<unsigned char> eligible(n),selected(n),core(n);for(unsigned i:seats){eligible[i]=1;side[i]=flip?1:0;}
 auto cell=[](const seats::Position& p){return Cell{int(std::floor(p.x/1.5f)),int(std::floor(p.y/1.5f)),int(std::floor(p.z/1.5f))};};
 std::unordered_map<Cell,std::vector<unsigned>,Hash> grid;grid.reserve(seats.size());
 for(unsigned i:seats){const auto& p=pos[i];if(!std::isfinite(p.x)||!std::isfinite(p.y)||!std::isfinite(p.z)||std::abs(p.x)>300||std::abs(p.z)>350||p.y<-.5||p.y>150)return {};grid[cell(p)].push_back(i);}
 if(cluster)for(const auto& entry:grid)if(entry.second.size()>96)return fallback();
 if(seats.size()>512&&grid.size()<seats.size()/64)return {}; // Collapsed/duplicate geometry cannot establish seat adjacency.
 // Geometry-derived seat edges: nearest same-row seats and adjacent row seats.
 // Gaps above these conservative physical bounds (aisles/decks) are not crossed.
 std::vector<std::vector<unsigned>> edges(n);
 std::vector<unsigned char> boundary(cluster?n:0);
 for(const auto& entry:grid){
  // Occupants of this cell use exactly the same neighboring cell list.
  const Cell c=entry.first;std::array<const std::vector<unsigned>*,27> neighbors{};unsigned usedCells=0;
  for(int x=-1;x<=1;++x)for(int y=-1;y<=1;++y)for(int z=-1;z<=1;++z){auto at=grid.find({c.x+x,c.y+y,c.z+z});if(at!=grid.end())neighbors[usedCells++]=&at->second;}
  for(unsigned i:entry.second){const auto& p=pos[i];std::array<double,4> best{1e9,1e9,1e9,1e9};std::array<unsigned,4> found{UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX};
   for(unsigned cellIndex=0;cellIndex<usedCells;++cellIndex){const auto& occupants=*neighbors[cellIndex];
   for(size_t k=0;k<(std::min)(occupants.size(),size_t(96));++k){unsigned j=occupants[k];if(i==j)continue;const auto& q=pos[j];double dx=q.x-p.x,dy=q.y-p.y,dz=q.z-p.z,d=dx*dx+dz*dz;
    if(d<.04||std::abs(dy)>.9)continue;int slot;
    if(std::abs(dy)<=.18){if(d>1.0)continue;double along=std::abs(p.x)>std::abs(p.z)?dz:dx;slot=along<0?0:1;}
    else{if(d>1.26||d<.16)continue;slot=dy<0?2:3;}
    double metric=d+dy*dy;if(metric<best[slot]||(metric==best[slot]&&j<found[slot])){best[slot]=metric;found[slot]=j;}
   }
  }
  for(unsigned j:found)if(j!=UINT32_MAX){edges[i].push_back(j);edges[j].push_back(i);}
  if(cluster)boundary[i]=std::find(found.begin(),found.end(),UINT32_MAX)!=found.end();
  }
 }
 for(auto& e:edges){std::sort(e.begin(),e.end());e.erase(std::unique(e.begin(),e.end()),e.end());}
 // Side bits are decoded from the verified native crowd-agent stream:
 // 0 = the fixture's home participant, 1 = away. Never infer camera sides.
 std::vector<unsigned char> anchor(n),seen(n);unsigned anchorCount=0;std::vector<unsigned> mainAnchor;
 if(nativeAnchors){
  const unsigned selectedSide=flip?0:1;std::vector<unsigned> largest;
  for(unsigned start:seats)if(!seen[start]&&tags.side[start]==selectedSide){std::vector<unsigned> component{start};seen[start]=1;for(size_t k=0;k<component.size();++k)for(unsigned j:edges[component[k]])if(eligible[j]&&!seen[j]&&tags.side[j]==selectedSide){seen[j]=1;component.push_back(j);}if(component.size()>largest.size())largest=std::move(component);}
  if(largest.size()<16)return fallback(); // No supported native section; retain EA layout.
  mainAnchor=largest;
  for(unsigned i:seats)if(tags.side[i]==selectedSide){anchor[i]=1;++anchorCount;}
  if(neutral&&(anchorCount<seats.size()/5||anchorCount>seats.size()*4/5))return fallback();
 }
 unsigned corner=fanHash(seed+401)%4;double sx=corner&1?1.:-1.,sz=corner&2?1.:-1.;
 seats::NoiseCache sectionNoise(5,int(seed%100000)+413);
 auto score=[&](unsigned i){auto& p=pos[i];
  if(nativeAnchors)return (anchor[i]?2.:0.)+.22*sectionNoise.sample(p.x,p.z)+.08*seats::random({int(i),int(seed),419});
  return .70*std::exp(-std::pow((p.x-sx*36)/32,2)-std::pow((p.z-sz*48)/32,2))+.22*sectionNoise.sample(p.x,p.z)+.08*seats::random({int(i),int(seed),419});};
 unsigned used=0,blocks=0;std::vector<unsigned> order=seats;std::vector<double> scores;
 // Neutral layouts never use the home-site main-section score.
 if(!neutral||nativeAnchors){scores.resize(n);for(unsigned i:seats)scores[i]=score(i);}
 if(!neutral||nativeAnchors){if(!(concentrated&&nativeAnchors&&!neutral))std::sort(order.begin(),order.end(),[&](unsigned a,unsigned b){return scores[a]==scores[b]?a<b:scores[a]>scores[b];});
  // Reserve a fuzzy footprint; native home pockets soften the edges.
  unsigned footprint=(std::min)(unsigned(order.size()),neutral&&nativeAnchors?target*11/10:concentrated?target*3/4:target*21/40);
  if(nativeAnchors&&(!concentrated||neutral))footprint=(std::min)(footprint,anchorCount);
  if(concentrated&&nativeAnchors&&!neutral){
   // Saved v4 plans use one physical component. Fresh v5 plans also admit
   // neighboring blocks across short aisle gaps without adding seat edges.
   unsigned start=*std::max_element(mainAnchor.begin(),mainAnchor.end(),[&](unsigned a,unsigned b){return scores[a]<scores[b];});
   if(cluster)blocks=growCluster(pos,seats,edges,boundary,scores,mainAnchor,start,seed,footprint,core);
   else{std::priority_queue<std::pair<double,unsigned>> frontier;std::vector<unsigned char> queued(n);frontier.push({scores[start],start});queued[start]=1;
   unsigned placed=0;while(!frontier.empty()&&placed<footprint){unsigned i=frontier.top().second;frontier.pop();core[i]=1;++placed;for(unsigned j:edges[i])if(eligible[j]&&!queued[j]){queued[j]=1;frontier.push({scores[j],j});}}
   blocks=placed?1:0;}
   for(unsigned i:seats)if(core[i]){bool edge=cluster&&boundary[i];for(unsigned j:edges[i])edge|=!core[j];const unsigned divisor=edge?5:25;if(fanHash(i^seed^439)%divisor){selected[i]=1;++used;}}
  }else for(unsigned k=0;k<footprint;++k){unsigned i=order[k];core[i]=1;if(fanHash(i^seed^439)%5){selected[i]=1;++used;}}
 }
 std::sort(order.begin(),order.end(),[&](unsigned a,unsigned b){unsigned x=fanHash(a^seed^421),y=fanHash(b^seed^421);return x==y?a<b:x<y;});
 std::vector<int> party(n,-1);std::vector<std::vector<unsigned>> groups;
 auto free=[&](unsigned i,int own){if(!eligible[i]||selected[i]||core[i])return false;for(unsigned j:edges[i])if(selected[j]&&(core[j]||party[j]!=own))return false;return true;};
 for(unsigned pass=0;pass<2&&used<target;++pass)for(unsigned leader:order){if(used>=target)break;if(!free(leader,-1))continue;
  unsigned r=fanHash(leader^seed^431)%100,want=r<5?1:r<30?2:r<55?3:r<80?4:r<88?5:r<94?6:r<98?7:8;
  if(pass)want=8;want=(std::min)(want,target-used);int id=int(groups.size());std::vector<unsigned> group{leader};selected[leader]=1;party[leader]=id;++used;
  for(size_t k=0;k<group.size()&&group.size()<want;++k){auto next=edges[group[k]];std::sort(next.begin(),next.end(),[&](unsigned a,unsigned b){return fanHash(a^seed)<fanHash(b^seed);});
   for(unsigned j:next)if(group.size()<want&&free(j,id)){
    // Parties larger than four spread into another row instead of long stripes.
    unsigned same=0;for(unsigned member:group)same+=std::abs(pos[member].y-pos[j].y)<.18;
    if(same>=4)continue;selected[j]=1;party[j]=id;group.push_back(j);++used;
   }
  }groups.push_back(std::move(group));
 }
 // Extend existing parties before considering a denser main section. This keeps
 // separate parties separated and limits every scattered component to 8 seats.
 for(auto& g:groups)for(size_t k=0;k<g.size()&&g.size()<8&&used<target;++k)for(unsigned j:edges[g[k]])if(g.size()<8&&used<target&&free(j,party[g[0]])){
  unsigned same=0;for(unsigned member:g)same+=std::abs(pos[member].y-pos[j].y)<.18;if(same>=4)continue;
  selected[j]=1;party[j]=party[g[0]];g.push_back(j);++used;
 }
 if(!neutral&&used<target){std::sort(order.begin(),order.end(),[&](unsigned a,unsigned b){return scores[a]==scores[b]?a<b:scores[a]>scores[b];});for(unsigned i:order)if(!selected[i]&&used<target){bool next=false,touchesParty=false;for(unsigned j:edges[i]){next|=core[j]!=0;touchesParty|=party[j]>=0;}if(next&&(!cluster||!touchesParty)){core[i]=selected[i]=1;++used;}}}
 Stats report;report.blocks=blocks;report.unassigned=target-used;for(unsigned i:seats){if(core[i]){++report.footprint;bool edge=cluster&&boundary[i];for(unsigned j:edges[i])edge|=!core[j];if(edge){++report.edgeSeats;report.edgeVisitors+=selected[i];}else{++report.innerSeats;report.innerVisitors+=selected[i];}}if(selected[i]){side[i]=flip?0:1;if(core[i])++report.core;else ++report.scattered;}}
 for(const auto& g:groups){++report.parties;++report.sizes[g.size()];}
 if(summary)*summary=report;
 if(stats){report.coreMask=std::move(core);report.party=std::move(party);*stats=std::move(report);}
 // Unachievable geometry/occupancy budgets keep the native appearance intact.
 if(used!=target)return nativeAnchors?fallback():std::vector<unsigned char>{};return side;
}
}
