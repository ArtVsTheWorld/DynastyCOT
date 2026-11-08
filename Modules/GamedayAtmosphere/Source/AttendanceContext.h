#pragma once
#include "AttendanceRules.h"
namespace attendance::context {
inline float smooth(float v){v=(std::clamp)(v,0.0f,1.0f);return v*v*(3-2*v);}
inline float quality(int rank){return rank>0?std::exp(-(rank-1)/35.0f):0;}
inline float separation(const State& raw){
    State s=ranked(raw);if(s.rank0<1||s.rank1<1)return 0;
    // Log ranks distinguish a top-end mismatch from the same gap lower down.
    float gap=std::abs(std::log((s.rank0+8.0f)/(s.rank1+8.0f)));
    return gap/(gap+1.4f);
}
inline float importance(const State& raw){
    State s=ranked(raw);float a=quality(s.rank0),b=quality(s.rank1);
    float opponent=s.neutral?(std::max)(a,b):b;
    float value=0.6f*std::sqrt(a*b)+0.4f*opponent;
    if(marqueeMatchup(s))value=(std::max)(value,0.85f);
    if(s.playoff)value=(std::max)(value,championship(s)?1.0f:0.95f);
    return value;
}
inline float variation(const State& s){
    if(!s.variationSeed)return 0;
    unsigned x=s.variationSeed;x^=x>>16;x*=0x7feb352du;x^=x>>15;x*=0x846ca68bu;x^=x>>16;
    return static_cast<float>(x&65535)/32767.5f-1;
}
inline float comparableHomeWin(const State& raw){
    State s=ranked(raw);
    if(s.neutral||s.homeScore<=s.awayScore||s.rank0<1||s.rank1<1)return 0;
    return (1-smooth(relativeGap(s)))*(1-inferiorOpponent(s));
}
// Pressure/approach speed, not another attendance deduction or new floor.
// Observe scoring pace only after halftime, normalized for custom quarters.
// A tense low-scoring game receives only a quarter of the ordinary modifier.
inline float departureEngagement(const State& s,float elapsed){
    if(!valid(s)||!regularSeason(s)||s.quarter>4||s.phase==7||!std::isfinite(elapsed))return 1;
    float progress=(std::clamp)(elapsed/(4*s.quarterSeconds),0.f,1.f);
    if(progress<=.50f)return 1;
    float ramp=smooth((progress-.50f)/.25f);
    float relevance=.25f+.75f*smooth((std::abs(s.homeScore-s.awayScore)-8)/13.f);
    float pace=(s.homeScore+s.awayScore)/progress;
    float low=smooth((48-pace)/24.f),high=smooth((pace-48)/24.f);
    float scoring=1+ramp*relevance*(.15f*low-.10f*high);
    float night=1+(s.night?.20f:0.f)*ramp*relevance;
    return scoring*night;
}
struct HalftimeTarget {float permanent=0,temporary=0;};
struct HalftimeTiming {float departure=.25f,mainReturn=.25f,lastReturn=.25f,lateDelay=0;};
inline HalftimeTiming halftimeTiming(const State& s){
    if(!s.variationSeed)return {}; // Unseeded legacy estimates keep their original timetable.
    auto sample=[&](unsigned salt){State x=s;x.variationSeed^=salt;return .5f+.5f*variation(x);};
    return {.20f+.10f*sample(0x48414c46u),.15f+.10f*sample(0x52455431u),
        .25f+.10f*sample(0x52455432u),.02f+.04f*sample(0x4c415445u)};
}
inline float halftimeReturnWeight(const State& s){
    auto timing=halftimeTiming(s);float elapsed=1-s.seconds/s.quarterSeconds;
    // Most seat-breakers return promptly; the smaller second wave trails off.
    return .8f*smooth(1-elapsed/timing.mainReturn)+
        .2f*smooth(1-(elapsed-timing.lateDelay)/(timing.lastReturn-timing.lateDelay));
}
inline HalftimeTarget halftime(const State& s){
    int margin=std::abs(s.homeScore-s.awayScore);
    float wet=badWeather(s)?0.15f:0;
    float patience=!s.neutral&&s.homeScore>s.awayScore?0.30f-0.15f*inferiorOpponent(s):0;
    float choose=(std::clamp)(0.65f+wet-patience+0.08f*variation(s),0.0f,1.0f);
    const float temporary=20+10*choose;
    if(s.playoff)return {1+3*choose,temporary};
    bool special=marqueeMatchup(s);
    float close=special?1+2*choose:3+2*choose;
    if(margin<=8)return {close,temporary};
    if(margin<=16){float t=smooth((margin-8)/8.0f);return {close+(10-close)*t,temporary};}
    float patienceFactor=1-0.25f*comparableHomeWin(s);
    if(margin<=24){float t=(margin-17)/7.0f;return {patienceFactor*(std::clamp)(10+10*t+2*(choose-0.5f),10.0f,20.0f),temporary};}
    if(margin<=34){float t=(margin-25)/9.0f;return {patienceFactor*(std::clamp)(20+10*t+3*(choose-0.5f),20.0f,30.0f),temporary};}
    float t=(std::clamp)((margin-35)/14.0f,0.0f,1.0f);
    return {patienceFactor*(30+10*(0.4f*t+0.6f*choose)),temporary};
}
// Read-only, verified possession context. Unknown values leave the score/time
// model in charge; neutral sites remain symmetric without attendance splits.
inline float lateOpportunity(const State& s){
    float window=s.quarterSeconds*.25f;
    if(s.neutral||s.quarter!=4||s.seconds>window||s.homeScore>=s.awayScore)return 1;
    if(s.offense<0||s.offense>1||s.homeTimeouts<0||s.homeTimeouts>3)return 1;
    int margin=s.awayScore-s.homeScore;
    float late=smooth((window-s.seconds)/(window*(150.f/180.f))),multi=smooth((margin-8)/9.0f);
    if(s.offense==0){
        float red=s.toGoal>=0&&s.toGoal<=20?0.12f:0;
        return 1-late*(0.15f+red)*(1-multi*0.5f);
    }
    return 1+late*multi*(0.20f+0.10f*(3-s.homeTimeouts));
}
}
