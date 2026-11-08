#pragma once
#include <algorithm>
#include <cmath>

namespace attendance::shape {
inline float smooth(float x){x=(std::clamp)(x,0.f,1.f);return x*x*(3-2*x);}
// Keep the existing total settling time. Its first short portion is a hold,
// not extra time repeatedly added by camera cuts or result publication.
inline unsigned tailHold(unsigned total){return (std::min)(1200u,total/4);}
inline float tail(float age,float total,float hold){
    if(!std::isfinite(age)||!std::isfinite(total)||!std::isfinite(hold)||age<0||total<=0||age>=total)return 0;
    hold=(std::clamp)(hold,0.f,total*.5f);
    return age<=hold?1.f:1-smooth((age-hold)/(total-hold));
}
inline float impact(float age,float total,float attack=100.f){
    return smooth(age/attack)*tail(age,total,(std::min)(1000.f,total*.2f));
}
inline float skillBuildup(float age,float total){return smooth(age/450.f)*tail(age,total,450.f);}
// A clear onset settles slightly into a sustained cheer, then eases down.
// Values never exceed one: shape changes cannot raise the volume ceiling.
inline float response(double age,double hold,double release,float strength){
    if(!std::isfinite(age)||!std::isfinite(hold)||!std::isfinite(release)||!std::isfinite(strength)||age<0||hold<0||release<=0||age>=hold+release)return 0;
    float sustained=.84f+.12f*(std::clamp)(strength,0.f,1.f);
    if(age<=hold)return 1-(1-sustained)*smooth(static_cast<float>(age/(std::min)(.8,(std::max)(hold,.001))));
    return sustained*(1-smooth(static_cast<float>((age-hold)/release)));
}
}
