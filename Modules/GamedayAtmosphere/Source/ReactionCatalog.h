#pragma once
#include <array>
#include <cstddef>
#include <algorithm>
#include <cstdint>
namespace attendance::catalog {
// Listening session20261030: size selectors1..3; not distinct recording counts.
struct Family {const char* name;unsigned mask;bool negative;};
inline constexpr std::array<Family,28> Families{{
 {"BC_ACTION_POS",14,false},{"BC_ACTION_NEG",14,true},{"BIGHIT_POS",14,false},{"BIGHIT_NEG",0,true},
 {"CATCH_POS",14,false},{"CATCH_NEG",6,true},{"FG_POS",14,false},{"FG_NEG",14,true},
 {"FUM_POS",14,false},{"FUM_NEG",14,true},{"INCOMP_POS",14,false},{"INCOMP_NEG",14,true},
 {"INT_POS",14,false},{"INT_NEG",14,true},{"SACK_POS",14,false},{"SACK_NEG",14,true},
 {"TD_CATCH_POS",14,false},{"TD_CATCH_NEG",14,true},{"TD_RUN_POS",14,false},{"TD_RUN_NEG",14,true},
 {"YARDS_GAINED_POS",14,false},{"YARDS_GAINED_NEG",14,true},{"GEN_YEAH",14,false},{"GEN_AWW",14,true},
 {"GEN_OH",14,false},{"GEN_BOO",14,true},{"GEN_APPL_CHEER",14,false},{"GEN_ANTIC",14,false}
}};
inline bool family(int f){return f>=0&&f<28&&Families[f].mask!=0;}
inline bool usable(int id){return id>=0&&family(id/4)&&(Families[id/4].mask&(1u<<(id%4)))!=0;}
inline bool negative(int id){return id>=0&&id/4<28&&Families[id/4].negative;}
inline int selector(int requested,int crowdSize){
 if(requested<0||!family(requested/4)||requested%4==0)return 0;
 int f=requested/4,size=(std::min)((std::clamp)(crowdSize,1,3),requested%4);
 while(size>0&&!(Families[f].mask&(1u<<size)))--size;
 return size?f*4+size:0; // Failed23 falls back to22;13..15 never reach dispatch.
}
inline uint64_t hash(uint64_t play,uint64_t event,unsigned epoch){uint64_t x=play*0x9e3779b97f4a7c15ull+event*0xbf58476d1ce4e5b9ull+epoch;x^=x>>30;x*=0xbf58476d1ce4e5b9ull;x^=x>>27;return x;}
inline int type(int f,int tier=3){return family(f)?selector(f*4+(std::clamp)(tier,1,3),3):0;}
template<std::size_t N>int choose(const std::array<int,N>& families,uint64_t key,int tier=3){return type(families[key%N],tier);}
}
