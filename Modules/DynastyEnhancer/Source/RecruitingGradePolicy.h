#pragma once
#include <cmath>
#include <string>
#include "EnhancementOptions.h"

namespace recruitingfog {
enum class Visibility {Unknown, Revealed, Range};
inline char grade(int rating){return rating>=90?'A':rating>=80?'B':rating>=70?'C':rating>=60?'D':'F';}
inline std::string display(Visibility visibility,double value,double low=0,double high=100){
    if(enhancements::options().recruitingScale==enhancements::GradeScale::Blind)return u8"–";
    if(visibility==Visibility::Unknown)return "?";
    if(visibility==Visibility::Revealed){
        if(!std::isfinite(value)||value<0||value>100||value!=std::floor(value))return "?";
        return enhancements::gradeLabel(static_cast<int>(value),enhancements::options().recruitingScale);
    }
    if(!std::isfinite(low)||!std::isfinite(high)||low<0||high>100||low>high)return "?";
    const std::string first=enhancements::gradeLabel(static_cast<int>(std::floor(low)),enhancements::options().recruitingScale),last=enhancements::gradeLabel(static_cast<int>(std::ceil(high)),enhancements::options().recruitingScale);
    return first==last?first:first+u8"–"+last;
}
}
