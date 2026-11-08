#pragma once
#include <algorithm>
#include <cmath>
namespace attendance::volume {
// The game expects logarithmic mixer packets. Volume controls and attendance
// curves use signal percentages; convert only for those native mixer interfaces.
inline float nativeDb(float gain){return !std::isfinite(gain)?0:gain<=0?-160.0f:20*std::log10(gain);}
inline float boostDb(float percent){return nativeDb(1+(std::clamp)(percent,0.0f,1000.0f)*0.01f);}
inline float boostPercent(float db){return std::isfinite(db)?100*(std::pow(10.0f,db/20)-1):0;}
inline float cutPercent(float db){return std::isfinite(db)?100*(1-(std::clamp)(std::pow(10.0f,db/20),0.0f,1.0f)):0;}
}
