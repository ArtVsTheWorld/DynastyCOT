#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace attendance::dense {
inline float smooth(float x){x=(std::clamp)(x,0.f,1.f);return x*x*(3-2*x);}
struct Layers {
    float full=0,veryLarge=0;
    float multiplier()const{return 1.f+.20f*full+.12f*veryLarge;}
    float swellMinimum()const{return .55f-.21f*full-.08f*veryLarge;}
};
// These weights strengthen an already-admitted sound intent. They never
// authorize atmosphere, alter the bus mix, or create a cheer/camera event.
// Tier centers: 80% / 20k, then 85% / 45k occupied mapped positions.
inline Layers layers(uint64_t positions,float fullness){
    if(positions<512||positions>12800000||!std::isfinite(fullness)||fullness<=0||fullness>1)return {};
    double occupied=positions*static_cast<double>(fullness);
    Layers out;
    out.full=smooth((fullness-.78f)/.04f)*smooth(static_cast<float>((occupied-18000.)/4000.));
    out.veryLarge=(std::min)(out.full,smooth((fullness-.83f)/.04f)*smooth(static_cast<float>((occupied-40000.)/10000.)));
    return out;
}
inline bool majorSize(uint64_t positions,float fullness){
    return std::isfinite(fullness)&&fullness>=.80f&&fullness<=1&&positions>=512&&positions<=12800000&&positions*static_cast<double>(fullness)>=24000;
}
}
