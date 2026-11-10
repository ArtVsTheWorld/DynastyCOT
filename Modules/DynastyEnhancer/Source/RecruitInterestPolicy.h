#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
namespace recruitingfog::interest {
enum class Mode:unsigned {Native,Feedback,RankOnly,Blind};
enum class Standing:unsigned {Favorite,Contender,InTheMix,Outside,LongShot,Unknown};
inline const char* label(Standing s){constexpr const char* names[]={"FAVORITE","CONTENDER","IN THE MIX","OUTSIDE LOOKING IN","LONG SHOT","INTEREST UNKNOWN"};return names[unsigned(s)<=5?unsigned(s):5];}
inline unsigned effective(bool recruiting,unsigned scale,unsigned choice){return !recruiting?0:scale==4?3:choice<=2?choice:1;}
inline uint64_t mix(uint64_t value){value^=value>>30;value*=0xbf58476d1ce4e5b9ull;value^=value>>27;value*=0x94d049bb133111ebull;return value^(value>>31);}
struct School {int team=-1;std::string name;double influence=0;Standing standing=Standing::Unknown;unsigned nativeOrder=0;};
// Relative competitive gaps, not the game's commitment percentage or odds.
// Stable boundary uncertainty is bounded; no week, grade, trait or refresh seed.
inline void classify(std::vector<School>& schools,uint64_t recruit,uint32_t league){
 std::vector<double> values;for(const auto& s:schools)if(s.team>=0&&std::isfinite(s.influence)&&s.influence>=0)values.push_back(s.influence);
 if(values.empty())return;std::sort(values.begin(),values.end());const double leader=values.back();if(leader<=0)return;
 // A native small card exposes only the top five schools. Normalize against
 // the same leader in either view so trimming that array cannot change tiers.
 const double scale=leader;
 constexpr std::array<double,4> edges{{.10,.25,.50,.80}};
 for(auto& s:schools){if(s.team<0||!std::isfinite(s.influence)||s.influence<0)continue;
  const auto seed=mix(recruit^(uint64_t(league)<<32)^uint32_t(s.team));const double uncertainty=(double(seed%2001)/1000.-1.)*.025;
  const double gap=(leader-s.influence)/scale;unsigned tier=0;while(tier<edges.size()&&gap>edges[tier]+uncertainty)++tier;s.standing=Standing(tier);
 }
}
inline void group(std::vector<School>& schools){std::stable_sort(schools.begin(),schools.end(),[](const auto& a,const auto& b){if(a.standing!=b.standing)return a.standing<b.standing;if(a.name!=b.name)return a.name<b.name;return a.team<b.team;});}
inline int band(Standing s){return s==Standing::Unknown?0:90-int(s)*20;}
}
