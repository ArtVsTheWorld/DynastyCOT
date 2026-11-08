#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace attendance::occupancy {
struct Snapshot {
    uintptr_t match=0;unsigned epoch=0;int stadium=-1,attendance=-1,capacity=-1;
    bool valid()const{return match&&stadium>=0&&stadium<1023&&capacity>0&&capacity<=1000000&&attendance>=0&&attendance<=capacity;}
    bool owns(uintptr_t owner,unsigned generation)const{return valid()&&match==owner&&epoch==generation;}
    float peak()const{return valid()?static_cast<float>(attendance)/capacity:1;}
};
// Native attendance is the game's original crowd, not a rendered-agent count.
// Our arrivals/exits are shares of that crowd. Camera culling is irrelevant.
inline float fraction(const Snapshot& s,uintptr_t owner,unsigned epoch,float remaining){
    if(!std::isfinite(remaining))return 0;
    float retained=(std::clamp)(remaining,0.0f,1.0f);
    return retained*(s.owns(owner,epoch)?s.peak():1.0f);
}
}
