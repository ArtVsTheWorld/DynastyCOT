#pragma once
#include "SustainedCrowd.h"
#include "DenseCrowd.h"
namespace attendance::background {
// Comfortable home leads settle more gently than home deficits: unchanged
// through 17 points, smoothly reaching 65% background at a 42-point lead.
inline float leadEngagement(const State& s){
    return s.neutral?1.f:1.f-.35f*dense::smooth((static_cast<float>(s.homeScore-s.awayScore)-17.f)/25.f);
}
// Reuse the verified native sustained channel at low priority, not a timer
// repeatedly firing cheer samples. Reactions take ownership of the same voice.
inline sustained::Intent merge(sustained::Intent out,const State& s,uint64_t positions,float fullness){
    if(!out.active||out.reason!=sustained::Reason::None||!valid(s)||
       (s.phase!=1&&s.phase!=3&&s.phase!=4&&s.phase!=5&&s.phase!=6)||
       positions<512||positions>12800000||!std::isfinite(fullness)||fullness<.60f||fullness>1)return out;
    double occupied=positions*static_cast<double>(fullness);if(occupied<10000)return out;
    auto tiers=dense::layers(positions,fullness);
    float weight=dense::smooth((fullness-.60f)/.20f)*dense::smooth(static_cast<float>((occupied-10000)/14000));
    if(weight<=0)return out;
    out.reason=sustained::Reason::Background;out.priority=0;out.continuous=true;out.reaction=false;
    out.size=tiers.veryLarge>.5f?3:tiers.full>.5f?2:1;
    // A losing home crowd settles progressively; neutral support is shared.
    // This low-priority bed never attenuates an earned cheer or scoring roar.
    float deficit=s.neutral?0.f:static_cast<float>((std::max)(0,s.awayScore-s.homeScore));
    float engagement=(1.f-.80f*dense::smooth((std::min)(deficit/35.f,1.f)))*leadEngagement(s);
    out.intensity=(std::max)(out.intensity,weight*(350+200*tiers.full+100*tiers.veryLarge)*engagement);
    out.cheers=false;out.supporting=false;out.attackMs=4000;out.releaseMs=2000;
    return out;
}
}
