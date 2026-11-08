#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace attendance::packed {
// Whole authored seating positions, including originally empty seats. Never
// substitute official capacity, current occupied seats or a camera's draw count.
inline constexpr uint64_t SmallPositions=6060,LargePositions=61888;
inline float interpolate(uint64_t positions,double smallFraction,double largeFraction){
    if(positions<512||positions>12800000)return 1.01f;
    double size=(std::clamp)((static_cast<double>(positions)-SmallPositions)/(LargePositions-SmallPositions),0.0,1.0);
    return static_cast<float>(smallFraction+(largeFraction-smallFraction)*size);
}
inline float minimum(uint64_t positions){return interpolate(positions,.98,.60);}
inline float standardMinimum(uint64_t positions){return interpolate(positions,.90,.55);}
inline float specialMinimum(uint64_t positions){return interpolate(positions,.85,.50);}
inline bool qualifies(uint64_t positions,float fullness,bool known=true){
    return known&&std::isfinite(fullness)&&fullness>=minimum(positions);
}
inline float strength(uint64_t positions,float fullness){
    if(!qualifies(positions,fullness))return 0;
    float start=minimum(positions),span=(std::max)(.01f,1-start);
    float x=(std::clamp)((fullness-start)/span,0.0f,1.0f);
    return x*x*(3-2*x);
}
}
