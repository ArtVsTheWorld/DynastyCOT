#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include "PackedHouse.h"
namespace attendance::motion {
inline float occupiedScale(uint64_t positions,float fullness){
    if(positions<512||positions>12800000||!std::isfinite(fullness)||fullness<=0)return 0;
    double occupied=positions*static_cast<double>((std::clamp)(fullness,0.f,1.f));
    // Linear occupied-seat response: .80 at 6,060 and 1.25 at 61,888.
    // Continue smoothly below the small reference; the gate still decides
    // whether any crowd motion is eligible in the first place.
    return static_cast<float>((std::min)(1.25,.80+.45*(occupied-6060.)/(61888.-6060.)));
}
inline float venueScale(uint64_t positions){return occupiedScale(positions,1.f);}
}
