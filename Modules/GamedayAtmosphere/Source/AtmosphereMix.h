#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace attendance::mix {
inline constexpr float NormalMaximumDb=6,MaximumDb=8;
inline float normalCeiling(float requested){return std::isfinite(requested)?(std::clamp)(requested,0.0f,NormalMaximumDb):0;}
inline float ceiling(float requested){return std::isfinite(requested)?(std::clamp)(requested,0.0f,MaximumDb):0;}
inline float callbackCeiling(float normalRequested,float decisiveRequested,float observed,bool eruptions,uint64_t now,uint64_t expires){
    float normal=normalCeiling(normalRequested);
    if(normal<=0||!eruptions||now>=expires)return normal;
    return (std::max)(normal,(std::min)(ceiling(decisiveRequested),ceiling(observed)));
}
inline float ambienceGain(float requestedParent,float appliedParent,float extra,float limit){
    if(!std::isfinite(requestedParent)||!std::isfinite(appliedParent)||!std::isfinite(extra))return 0;
    limit=ceiling(limit);
    float parent=(std::max)((std::clamp)(requestedParent,0.0f,limit),(std::clamp)(appliedParent,0.0f,limit));
    return std::floor((std::min)((std::max)(0.0f,extra),limit-parent)*4+0.0001f)*0.25f;
}
inline float ambienceForTotal(float requestedParent,float appliedParent,float total,float limit){
    if(!std::isfinite(total))return 0;
    float parent=(std::max)(requestedParent,appliedParent);
    return ambienceGain(requestedParent,appliedParent,(std::max)(0.0f,total-parent),limit);
}
inline float parentGain(float requested,float appliedAmbience,float limit){
    if(!std::isfinite(requested)||!std::isfinite(appliedAmbience))return 0;
    limit=ceiling(limit);
    float room=(std::max)(0.0f,limit-(std::max)(0.0f,appliedAmbience));
    return (std::clamp)(std::round(requested*4)*0.25f,0.0f,room);
}
// Hold only a recently verified gain through a short same-match state gap.
// Never retain an audio object, and never carry across disable, menus or a new match.
struct Continuity {
    uintptr_t match=0;double at=-1;float db=0;
    void reset(){*this=Continuity{};}
    float record(uintptr_t owner,double now,float level,float limit){match=owner;at=now;db=(std::clamp)(level,0.0f,ceiling(limit));return db;}
    float gap(uintptr_t owner,double now,bool eligible,float limit){
        if(!eligible||!owner||owner!=match||at<0||!std::isfinite(now)||now<at){reset();return 0;}
        double age=now-at;
        if(age>=2.5)return 0;
        return (std::min)(db,ceiling(limit))*static_cast<float>(age<=1.5?1:2.5-age);
    }
};
}
