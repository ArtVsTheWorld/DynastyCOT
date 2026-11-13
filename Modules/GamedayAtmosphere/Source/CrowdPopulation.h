#pragma once
#include "SustainedCrowd.h"
#include "VenueMotion.h"
#include "ReactionCatalog.h"
#include "DenseCrowd.h"
namespace attendance::population {
inline double occupied(uint64_t positions,float fullness){
    return positions>=512&&positions<=12800000&&std::isfinite(fullness)?positions*static_cast<double>((std::clamp)(fullness,0.f,1.f)):0;
}
inline float presence(uint64_t positions,float fullness){
    return static_cast<float>(std::sqrt((std::clamp)(occupied(positions,fullness)/packed::LargePositions,0.,1.)));
}
inline int sizeForCount(double count,float fullness){
    // Mapped occupied spectators, not the native attendance/capacity figure.
    // A small venue can still sound medium when normally attended; a very
    // sparse large venue remains small even with several thousand spectators.
    if(!std::isfinite(count)||count<3000||!std::isfinite(fullness)||fullness<.10f)return 1;
    return count>=36000&&fullness>=.60f?3:2;
}
inline int size(uint64_t positions,float fullness){return sizeForCount(occupied(positions,fullness),fullness);}
inline bool reactionFamily(int type){return catalog::usable(type);}
inline bool negative(int type){return catalog::negative(type);}
inline int reactionType(int type,int sizeCode){if(catalog::usable(type)&&sizeCode>=2)type=(type/4)*4+(std::max)(2,type%4);return catalog::selector(type,sizeCode);}
// Replace the existing density-presence factor for DGA's added layer intent.
// Native excitement/cheer peaks, background quieting and volume caps remain
// with their existing owners. Apply once, before final-play intent capture.
inline sustained::Intent layers(sustained::Intent out,uint64_t positions,float fullness){
    if(!out.active||out.reason==sustained::Reason::Diagnostics||out.reason==sustained::Reason::Postgame)return out;
    auto density=dense::layers(positions,fullness);
    out.populationSize=size(positions,fullness);
    if((out.celebration||out.scoringReaction||out.majorLive)&&dense::majorSize(positions,fullness))out.populationSize=3;
    if(density.veryLarge>.5f&&out.reason==sustained::Reason::Lineup&&out.continuous)out.size=(std::max)(out.size,2);
    out.size=(std::min)(out.size,out.populationSize);
    // Admission and occupied-seat mix still apply; these use the largest native variant.
    if(out.celebration||out.reason==sustained::Reason::Eruption||out.passFlight){out.populationSize=3;out.size=3;}
    if(out.hostileOpeningResult)out.populationSize=3; // Largest one-shot cheer, unchanged supporting roar size.
    if(out.reason==sustained::Reason::None)return out;
    if(!std::isfinite(fullness)||fullness<=0||!std::isfinite(out.intensity)){out.intensity=0;return out;}
    float previous=std::sqrt((std::clamp)(fullness,0.f,1.f));
    float layerPresence=presence(positions,fullness)*density.multiplier();
    out.intensity=(std::clamp)(out.intensity*layerPresence/previous,0.f,1000.f);
    return out;
}
}
