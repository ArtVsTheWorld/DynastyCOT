#pragma once
#include <algorithm>
#include <cmath>
#include "MatchupClass.h"
namespace attendance {
enum class Profile { Automatic=0,Default=1,Reduced=2,BadWeather=3 };
struct Settings {
    int enabled=1; Profile profile=Profile::Automatic;
    int comebackDeficit=21,comebackRecovered=14,comebackDeteriorationSeconds=60;
    int warningEnabled=1,formulaStrength=100,quarterCap=45,transitionCap=4;
    int lateTraffic=1,lateTrafficQuarterExtra=10;
};
struct State {
    int quarter=0,homeScore=0,awayScore=0,phase=0;float seconds=0,quarterSeconds=0;
    bool rivalry=false,playoff=false;
    int rank0=0,rank1=0,precipitation=0,bowl=127;
    bool neutral=false;
    int team0=-1,team1=-1;
    unsigned variationSeed=0;
    int offense=-1,homeTimeouts=-1;
    float toGoal=-1;
    bool conferenceChampionship=false,hfaActive=false;
    bool startingAttendance=false,night=false;
    int plannedKickoff=-1,plannedFill=-1;
    int plannedPeak=0,plannedCapacity=0,plannedStadium=-1,plannedWeek=-1,plannedYear=0;
    bool exceptionalOpening=false;
    bool spring=false;int springSchool=-1,springDynasty=0,springNumber=-1;
    float mappedPeak=-1; // Refreshed only from the owned whole-layout baseline.
    int rivalryEvent=-1; // Native event identity; not inferred from team slots.
};
inline bool springContext(const State& s){return s.spring&&s.springDynasty!=0&&s.springNumber>=0&&(s.springSchool==s.team0||s.springSchool==s.team1);}
inline bool arrivalPlan(const State& s){
    int cap=s.exceptionalOpening&&s.plannedPeak==s.plannedCapacity&&s.plannedCapacity>=1000?95:80;
    return s.startingAttendance&&s.plannedKickoff>=35&&s.plannedKickoff<=cap&&s.plannedFill>=10&&s.plannedFill<=100;
}
inline bool crowdPlan(const State& s){return arrivalPlan(s)&&s.plannedCapacity>=1000&&s.plannedCapacity<=200000&&s.plannedPeak>=1&&s.plannedPeak<=s.plannedCapacity&&s.plannedStadium>=0&&s.plannedStadium<1023&&s.plannedWeek>=0&&s.plannedWeek<=31&&s.plannedYear>=0&&s.plannedYear<=4095;}
inline void copyCrowdPlan(const State& from,State& to){
    to.spring=from.spring;to.springSchool=from.springSchool;to.springDynasty=from.springDynasty;to.springNumber=from.springNumber;
    if(!crowdPlan(from))return;
    to.startingAttendance=true;to.night=from.night;to.plannedKickoff=from.plannedKickoff;to.plannedFill=from.plannedFill;
    to.plannedPeak=from.plannedPeak;to.plannedCapacity=from.plannedCapacity;to.plannedStadium=from.plannedStadium;to.plannedWeek=from.plannedWeek;to.plannedYear=from.plannedYear;to.exceptionalOpening=from.exceptionalOpening;
}
inline void inheritCrowdPlan(const State& from,State& to){
    if(crowdPlan(to)||from.team0!=to.team0||from.team1!=to.team1||
       (to.plannedStadium>=0&&from.plannedStadium>=0&&to.plannedStadium!=from.plannedStadium))return;
    copyCrowdPlan(from,to);
}
inline float expectedPeak(const State& s){return crowdPlan(s)?static_cast<float>(s.plannedPeak)/(std::max)(1,s.plannedCapacity):-1.f;}
inline bool marqueeMatchup(const State& s){return matchup::big(s.rank0,s.rank1,s.rivalry,s.playoff,s.conferenceChampionship);}
// Atmosphere admission can ease at night; actual packed-house classification,
// occupied-seat strength and the absolute visual-density safeguard do not.
inline bool regularSeason(const State& s){return !s.playoff&&!s.conferenceChampionship&&s.bowl==127;}
inline float nightRequirement(const State& s,float normal){
    return s.night&&regularSeason(s)&&std::isfinite(normal)&&normal>=.50f&&normal<=1?
        (std::max)(.50f,normal*.90f):normal;
}
inline void postseason(State& s,bool cfp,bool conference){s.conferenceChampionship=conference;s.playoff=cfp||conference;}
inline bool championship(const State& s){return s.playoff&&!s.conferenceChampionship&&s.bowl==47;}
inline bool badWeather(const State& s){return s.precipitation==1||s.precipitation==2;}
inline int lossCeiling(const State& s){
    if(s.spring)return 60; // Separate one-fanbase model, never competitive visitor/capacity floors.
    int ceiling=(championship(s)?20:s.playoff?40:75)+(badWeather(s)?10:0);
    // A ranked home winner keeps at least 35% of its original crowd. This
    // departure limit survives mismatches/weather; existing exits never refill.
    if(!s.neutral&&s.rank0>=1&&s.rank0<=25&&s.homeScore>s.awayScore)ceiling=(std::min)(ceiling,65);
    // Departure-only capacity floor. Arrival and temporary opening gaps retain
    // their policy; this cannot refill existing empty seats or invent turnout.
    float peak=std::isfinite(s.mappedPeak)&&s.mappedPeak>=0&&s.mappedPeak<=1?s.mappedPeak:expectedPeak(s);
    if(std::isfinite(peak)&&peak>0&&peak<=1)
        ceiling=(std::min)(ceiling,(std::max)(0,static_cast<int>(std::floor(100*(1-.15f/peak)+.00001f))));
    return ceiling;
}
// The native rank reader accepts 1..255. Unknown FBS ranks remain unknown;
// an unranked stock FCS team sorts below that entire supported FBS range.
inline bool fcsTeam(int team){return team>=1500&&team<=1504;}
inline int effectiveRank(int rank,int team){return rank>=1&&rank<=255?rank:fcsTeam(team)?256:0;}
inline State ranked(State s){s.rank0=effectiveRank(s.rank0,s.team0);s.rank1=effectiveRank(s.rank1,s.team1);return s;}
inline float relativeGap(const State& raw){
    State s=ranked(raw);if(s.rank0<1||s.rank1<1)return 0;
    float gap=static_cast<float>(std::abs(s.rank0-s.rank1));return gap/(gap+50.0f);
}
inline float inferiorOpponent(const State& raw){
    if(raw.neutral)return 0;
    if(fcsTeam(raw.team1))return 1;
    State s=ranked(raw);if(s.rank0<1||s.rank1<=s.rank0)return 0;
    return (std::clamp)((s.rank1-s.rank0-30)/40.0f,0.0f,1.0f);
}
inline State reversed(State s){std::swap(s.homeScore,s.awayScore);std::swap(s.rank0,s.rank1);std::swap(s.team0,s.team1);return s;}
// A neutral venue follows the underdog's story, regardless of the scoreboard label.
inline State energyPerspective(State s){s=ranked(s);return s.neutral&&s.rank0>=1&&s.rank1>=1&&s.rank1>s.rank0?reversed(s):s;}
inline bool valid(const State& s){return s.quarter>=1&&s.quarter<=20&&s.homeScore>=0&&s.homeScore<=255&&s.awayScore>=0&&s.awayScore<=255&&std::isfinite(s.seconds)&&std::isfinite(s.quarterSeconds)&&s.quarterSeconds>=60&&s.quarterSeconds<=1200&&s.seconds>=0&&s.seconds<=s.quarterSeconds+1;}
inline bool normalizePregame(State& s){
    if(s.phase!=6||s.quarter<0||s.quarter>1||s.homeScore||s.awayScore||
       !std::isfinite(s.quarterSeconds)||s.quarterSeconds<60||s.quarterSeconds>1200)return false;
    // Presentation can precede the native Q1 clock start. Normalize only our
    // snapshot; never change the game's quarter or clock.
    s.quarter=1;s.seconds=s.quarterSeconds;return true;
}
// Native Quarter enum: OT is 5; EndOfGame is 6, not a second overtime.
inline bool overtime(const State& s){return s.quarter==5;}
inline void normalizeFinalPresentation(State& s){
    // A captured regulation winner briefly uses phase 8 while Quarter is
    // EndOfGame. It is final presentation, not a sixth regulation quarter.
    // Only normalize our snapshot; never change the native phase or clock.
    if(s.quarter==6&&s.phase==8)s.phase=7;
}
inline bool competitiveOvertime(const State& s){return overtime(s)&&s.phase!=7&&std::abs(s.homeScore-s.awayScore)<=8;}
inline bool untimedPeriod(const State& s){return (overtime(s)||s.quarter==6)&&s.quarterSeconds==0&&s.seconds==0;}
inline bool normalizeUntimedPeriod(State& s,float regulationLength){
    if(!untimedPeriod(s)||!std::isfinite(regulationLength)||regulationLength<60||regulationLength>1200)return false;
    s.quarterSeconds=regulationLength;return true;
}
inline float elapsedGameTime(const State& s){return (overtime(s)||s.quarter==6)?4*s.quarterSeconds:(s.quarter-1)*s.quarterSeconds+s.quarterSeconds-s.seconds;}
inline bool top25(int rank){return rank>=1&&rank<=25;}
inline bool bigGameMatchup(const State& s){return marqueeMatchup(s);}
inline bool playoffBowl(int bowl){return bowl>=37&&bowl<=53;}
inline Profile profileFor(const Settings& c,const State& s){
    if(c.profile!=Profile::Automatic)return c.profile;
    // Special matchups take priority over rain or snow.
    // Regular-season rivalries share marquee patience. Postseason retains its
    // existing rank/weather profile; its dedicated caps/rates still own protection.
    if(regularSeason(s)?marqueeMatchup(s):(top25(s.rank0)&&top25(s.rank1)))return Profile::Reduced;
    if(s.precipitation==1||s.precipitation==2)return Profile::BadWeather;
    return Profile::Default;
}
inline const char* profileName(Profile p){return p==Profile::Reduced?"reduced":p==Profile::BadWeather?"bad weather":"default";}
}
