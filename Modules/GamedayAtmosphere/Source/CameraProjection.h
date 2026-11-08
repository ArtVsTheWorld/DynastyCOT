#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace attendance::motion::projection {
// The 35-degree/16:9 lens is a geometry reference, not a preset identifier.
// The common gain is from 2.0.31. Broad framing folds in the accepted 90%
// preference from 2.0.34; tight framing retains the 2.0.33 strength.
inline constexpr float ReferenceY=3.1715948f,ReferenceX=ReferenceY*9.f/16.f;
inline constexpr float ReferenceStrength=1.10f;
inline constexpr uint64_t FallbackMs=250;
// A bounded framing preference, separate from optical compensation. These
// focal lengths come from the accepted wide/tight live views, not preset IDs.
// Smoothstep gives broad views 32.5% less motion and tight views a modest 5%
// lift, including custom cameras. Evaluate in focal space to avoid an atan
// in the camera callback; the slope reaches zero at both endpoints.
inline constexpr float BroadFocalY=1.9437481f,TightFocalY=2.9128082f;
inline float framingGain(float focalY){
    const float t=std::clamp((focalY-BroadFocalY)/(TightFocalY-BroadFocalY),0.f,1.f);
    return .675f+.375f*t*t*(3.f-2.f*t);
}
struct Scale {
    float pitch=1,yaw=1,fx=ReferenceX,fy=ReferenceY,framing=1;
    bool valid=false,cached=false,clipped=false;
};
inline bool valid(const std::array<float,16>& p){
    for(float v:p)if(!std::isfinite(v))return false;
    return p[0]>.1f&&p[0]<10000&&p[5]>.1f&&p[5]<10000&&
        std::fabs(p[15])<.001f&&std::fabs(std::fabs(p[11])-1)<.01f;
}
inline Scale scale(const std::array<float,16>& p,bool extended=false){
    Scale s;if(!valid(p))return s;
    s.fx=p[0];s.fy=p[5];s.framing=framingGain(p[5]);s.valid=true;
    float maximum=extended?6.f:4.f;
    s.pitch=(std::min)(ReferenceY/p[5],maximum);
    s.yaw=(std::min)(ReferenceX/p[0],maximum);
    s.clipped=s.pitch<ReferenceY/p[5]||s.yaw<ReferenceX/p[0];
    return s;
}
inline float verticalFov(const Scale& s){return s.valid?2.f*std::atan(1.f/s.fy)*57.295779513f:0;}
// A fixed-size, thread-owned cache. Only a missing/invalid read uses history;
// valid changing projections normalize immediately, keeping screen amplitude
// steady through a native zoom without introducing correction lag.
class History {
    struct Entry {uint64_t owner=0;uintptr_t view=0,match=0;unsigned epoch=0;uint64_t at=0;std::array<float,16> p{};};
    std::array<Entry,8> entries_{};size_t next_=0;
public:
    Scale observe(uint64_t owner,uintptr_t view,uintptr_t match,unsigned epoch,uint64_t now,
                  const std::array<float,16>& p,bool extended=false){
        Entry* found=nullptr;
        for(auto& entry:entries_)if(entry.owner==owner&&entry.view==view&&entry.match==match&&entry.epoch==epoch){found=&entry;break;}
        if(valid(p)){
            if(!found){found=&entries_[next_];next_=(next_+1)%entries_.size();}
            *found={owner,view,match,epoch,now,p};return scale(p,extended);
        }
        if(found&&now>=found->at&&now-found->at<=FallbackMs){auto s=scale(found->p,extended);s.cached=true;return s;}
        return {}; // No trustworthy projection: leave the native output alone.
    }
};
}
