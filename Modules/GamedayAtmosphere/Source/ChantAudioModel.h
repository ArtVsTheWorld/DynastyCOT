#pragma once
#include <algorithm>
#include <cmath>
#include "AudioLevels.h"

namespace attendance::chants {
// Near-full crowds keep native chants. The old -98 calibration was strongly
// mix-dependent and could silence chants after only seven percent departed.
inline float attendancePercent(float retainedFraction,int strength=100,float maximumReduction=50,float emphasis=1) noexcept {
    if(!std::isfinite(retainedFraction)||!std::isfinite(emphasis)||!std::isfinite(maximumReduction)||strength<=0)return 0;
    retainedFraction=(std::clamp)(retainedFraction,0.0f,1.0f);
    const float amount=static_cast<float>((std::clamp)(strength,0,100))*0.01f;
    constexpr float noCutAbove=0.70f,fullReductionAt=0.25f;
    float fade=(std::clamp)((noCutAbove-retainedFraction)/(noCutAbove-fullReductionAt),0.0f,1.0f);
    float reduction=retainedFraction==0?100.0f:(std::clamp)(maximumReduction,0.0f,100.0f)*amount*fade*fade*(3-2*fade);
    float gain=(1-reduction*0.01f)*(retainedFraction>0?(std::clamp)(emphasis,1.0f,1.35f):1);
    return 100*(1-(std::clamp)(gain,0.0f,1.0f));
}
}
