#pragma once
#include <algorithm>
#include <cmath>
namespace attendance::motion {
// These profiles select strength/timing only. Admission and ownership belong
// to the existing play detectors; a profile cannot create an event.
enum class Profile { Default,Gain,Showboat,Skill,Catch,Stop,Loss,Conversion,Downs,Takeaway,Force,GoalStop,HighStop,Truck };
inline float importance(float value){return std::isfinite(value)?(std::clamp)(value,0.f,1.f):0.f;}
inline float profileStrength(Profile p,float leverage){
    float low=11,high=11;
    switch(p){
    case Profile::Gain:low=10;high=12;break;
    case Profile::Showboat:low=3;high=5;break;
    case Profile::Skill:low=3;high=4;break;
    case Profile::Truck:low=7.5f;high=9;break;
    case Profile::Catch:low=8;high=10;break;
    case Profile::Stop:low=6;high=8;break;
    case Profile::Loss:case Profile::Conversion:low=8;high=10;break;
    case Profile::Downs:low=10;high=12;break;
    case Profile::Takeaway:low=12;high=14;break;
    case Profile::Force:case Profile::GoalStop:case Profile::HighStop:low=13;high=15;break;
    default:break;
    }
    return low+(high-low)*importance(leverage);
}
inline unsigned profileDuration(Profile p){
    switch(p){case Profile::Skill:case Profile::Truck:return 2000;case Profile::Stop:return 2300;
    case Profile::Force:case Profile::GoalStop:case Profile::HighStop:return 3000;
    default:return 2700;}
}
}
