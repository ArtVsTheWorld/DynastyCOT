#pragma once
#include "AttendanceRules.h"
#include "ExcitementRules.h"
namespace attendance::spring {
// spring is set only from the owned, fresh launch admission. That admission
// distinguishes a stamped, actually launched game250 from an idle marker row.
inline bool verified(const State& s){return springContext(s);}
inline float halftimeWave(const State& s){return 10.f+static_cast<float>((s.variationSeed^static_cast<unsigned>(s.springSchool*2246822519u))%6);}
inline float departureTarget(const State& s){
    if(!verified(s)||!valid(s))return 0;
    // Real native Q2 -> Q3 boundary, including drills starting late in Q2.
    // Score, school prestige and rivalries never accelerate spring exits.
    float progress=(std::clamp)((elapsedGameTime(s)-2*s.quarterSeconds)/(2*s.quarterSeconds),0.f,1.f);
    float end=40.f+static_cast<float>((s.variationSeed^static_cast<unsigned>(s.springSchool*2654435761u))%16);
    if(s.quarter<=2)return 4.f*(std::clamp)(elapsedGameTime(s)/(2*s.quarterSeconds),0.f,1.f);
    const float afterHalf=4+halftimeWave(s);
    return afterHalf+(end-afterHalf)*progress;
}
enum class Reaction {None,Cheer,Disappointment};
inline Reaction reaction(const energy::Play& p,const energy::Outcome& r,int snapOffense=-1){
    if(!verified(p.game)||p.simulated||!r.accepted||!r.completed||!r.sequence||p.offense<0||p.offense>1)return Reaction::None;
    const int offense=snapOffense; // Never infer snap possession from the next lineup.
    const bool failure=r.takeaway||r.sack||r.turnoverOnDowns||r.event==energy::Event::FourthStop||(r.yards>-1000&&r.yards<0);
    if(failure)return offense==1?Reaction::Disappointment:Reaction::None;
    if(r.homePoints>=6||r.awayPoints>=6)return (offense==0?r.homePoints:offense==1?r.awayPoints:0)>=6?Reaction::Cheer:Reaction::None;
    // A failed conversion outranks its yardage. A ten-yard third-down stop
    // is still a failed conversion, not an eight-plus-yard spring cheer.
    if(r.event==energy::Event::ThirdStop||r.event==energy::Event::ConversionStop||r.goalLineStop||r.thirdGoalStop)return Reaction::None;
    // The competitive tracker setback flag is home-oriented; an away first
    // down can set it. Use verified offensive detail, never that polarity flag.
    if(r.firstDown||r.yards>=8)return Reaction::Cheer;
    return Reaction::None;
}
}
