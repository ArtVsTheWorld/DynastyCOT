#pragma once
#include "AttendanceContext.h"
#include "StartingAttendance.h"
namespace attendance::arrivals {
struct Settings {
    int enabled=1,startPercent=55,lateStartPercent=45;
    int finishPercent=50,lateFinishPercent=75;
    int bigGameBonus=3,badWeatherPenalty=5;
    int openingOverride=-1; // Internal, fresh Play Now opening; no INI/profile migration.
};
enum class Group { Standard,Severe };
inline bool fcsTeam(int team){return attendance::fcsTeam(team);}
inline bool fcsOpponent(const State& s){return fcsTeam(s.team1)||(s.neutral&&fcsTeam(s.team0));}
inline Group schoolGroup(const State& s){
    if(s.neutral)return Group::Standard;
    return turnout::lateArrivalSchool(s.team0)?Group::Severe:Group::Standard;
}
inline bool lateSchool(const State& s){return schoolGroup(s)==Group::Severe;}
inline int baselinePercent(const Settings& c,const State& s);
inline float adjustment(const Settings& c,const State& raw){
    State s=ranked(raw);
    int dry=turnout::openingPercent(.5f,s.team0,s.rank0,s.rank1,s.neutral,s.night,s.rivalry,false,0,false,c.startPercent,c.lateStartPercent,c.bigGameBonus);
    return static_cast<float>(dry-baselinePercent(c,s)-(badWeather(s)?(std::clamp)(c.badWeatherPenalty,0,8):0));
}
inline int baselinePercent(const Settings& c,const State& s){
    return schoolGroup(s)==Group::Severe?(std::clamp)(c.lateStartPercent,40,50):(std::clamp)(c.startPercent,50,60);
}
inline bool postseasonOpening(const State& s){return s.playoff||s.conferenceChampionship||s.bowl!=127;}
inline int openingPercent(const Settings& c,const State& s){
    if(c.openingOverride>=0)return (std::clamp)(c.openingOverride,0,100);
    if(postseasonOpening(s)){
        int low=championship(s)||s.conferenceChampionship?80:70;
        float appeal=.5f*(turnout::rankStrength(s.rank0,138)+turnout::rankStrength(s.rank1,138));
        return (std::clamp)(low+static_cast<int>(std::lround(10*appeal))-(badWeather(s)?3:0),low,low+10);
    }
    if(arrivalPlan(s))return s.plannedKickoff;
    // Verified rivalry event identity remains available when the richer
    // Dynasty launch capture fails. Preserve its opening policy, rather than
    // sending a protected rivalry through the severe-late fallback.
    if(s.rivalry&&showcase::event(s.rivalryEvent))return showcase::opening(s.rivalryEvent,0,0);
    return (std::clamp)(static_cast<int>(std::lround(baselinePercent(c,s)+adjustment(c,s))),badWeather(s)?35:40,70);
}
inline float finishFraction(const Settings& c,const State& s){
    if(postseasonOpening(s))return .20f;
    if(arrivalPlan(s))return s.plannedFill*.01f;
    if(s.rivalry&&showcase::event(s.rivalryEvent))return .30f;
    int base=schoolGroup(s)==Group::Severe?c.lateFinishPercent:turnout::big(s.rank0,s.rank1,s.rivalry)?30:c.finishPercent;
    return (std::clamp)(base,10,100)*.01f;
}
inline int missingAt(int start,float elapsed,float fill,const State& s){
    if(fill<=0)return 0;
    float progress=(std::max)(0.0f,elapsed/fill);
    (void)s;progress=(std::clamp)(progress,0.0f,1.0f);
    return (std::clamp)(static_cast<int>(std::ceil(start*std::pow(1-progress,1.6f))),0,start);
}
struct Change {bool changed=false,starting=false,finished=false;int missing=0;};
struct CompletionNotice {
    bool pending=false;
    void reset(){pending=false;}
    void targetReached(){pending=true;}
    bool settled(int visibleAbsent,int permanent,bool applied){
        if(!pending||!applied||visibleAbsent>permanent)return false;
        pending=false;return true;
    }
};
struct Model {
    bool seen=false,active=false,done=false;int missing=0,startMissing=0;float elapsed=0,fillSeconds=0;
    void reset(){*this=Model{};}
    Change finish(){Change r;r.changed=missing>0;r.finished=active;r.missing=0;missing=0;active=false;done=true;return r;}
    Change observe(const Settings& c,const State& s){
        Change r;r.missing=missing;
        if(!valid(s)||(s.phase!=1&&s.phase!=5&&s.phase!=6))return r;
        if(done)return r;
        if(!c.enabled||s.quarter>1)return finish();
        if(!seen){
            seen=true;
            // An update loaded halfway through a game must never empty seats
            // to simulate a kickoff that already happened.
            if(s.quarter!=1||s.seconds<s.quarterSeconds*0.95f||s.homeScore||s.awayScore)return finish();
            int start=openingPercent(c,s);
            startMissing=100-start;missing=startMissing;
            fillSeconds=s.quarterSeconds*finishFraction(c,s);
            active=missing>0;done=!active;r.starting=active;r.changed=active;r.missing=missing;
        }
        float now=s.quarterSeconds-s.seconds;
        // A restored visible crowd can lag its ideal curve until the next
        // hidden update. Reloading or pausing at the same game clock must not
        // spend that catch-up immediately and change the saved crowd.
        if(now<=elapsed+0.01f)return r;
        elapsed=now;
        int next=missingAt(startMissing,elapsed,fillSeconds,s);
        next=(std::clamp)(next,0,missing);
        r.changed=r.changed||next!=missing;missing=next;r.missing=missing;
        if(missing==0&&active){active=false;done=true;r.finished=true;}
        return r;
    }
};
// These are temporarily absent fans, separate from permanent departures. Only
// plugin-owned seats may return. Game-clock high water prevents replay cycling.
struct Halftime {
    bool seen=false,active=false,done=false;float highTime=0,breakPercent=-1;int temporary=0,lastAbsent=0,lastQuarter=0,absenceLimit=100;
    bool waitingForBreak=false,releaseAtBreak=false;
    float returnScale=1;
    void reset(){*this=Halftime{};}
    int observe(const State& s,int permanent,int extra=100,bool packedHouse=false){
        releaseAtBreak=false;
        if(!valid(s)||(s.phase!=1&&s.phase!=5)||done)return 0;
        float now=(s.quarter-1)*s.quarterSeconds+s.quarterSeconds-s.seconds;
        if(seen&&now+0.01f<highTime)return temporary;
        bool crossing=seen&&lastQuarter<3&&s.quarter==3&&highTime<2*s.quarterSeconds-0.01f;
        bool enteredThird=seen&&lastQuarter<3&&s.quarter==3;
        bool patient=std::abs(s.homeScore-s.awayScore)<=8&&(marqueeMatchup(s)||packedHouse);
        if(s.quarter==2&&s.seconds>.01f)waitingForBreak=patient;
        else if(s.quarter==2&&patient)waitingForBreak=true;
        bool release=waitingForBreak&&((s.quarter==2&&s.seconds<=.01f)||enteredThird);
        seen=true;highTime=now;lastQuarter=s.quarter;
        if(extra<=0){active=false;temporary=0;return 0;}
        auto timing=context::halftimeTiming(s);
        const float before=s.quarterSeconds*timing.departure,after=s.quarterSeconds*timing.lastReturn;
        if(s.quarter>3||(s.quarter==3&&s.quarterSeconds-s.seconds>=after)){done=true;active=false;temporary=0;return 0;}
        if((s.quarter==2&&s.seconds<=before)||crossing)active=true;
        // Smooth both directions. A late attach in Q3 never invents a seat break.
        float amount=s.quarter==2?context::smooth(1-s.seconds/before):context::halftimeReturnWeight(s);
        if(s.quarter==2&&s.seconds>.01f&&patient)amount*=.20f;
        bool initializing=breakPercent<0;
        if(s.quarter==2||initializing)breakPercent=context::halftime(s).temporary;
        int missing=static_cast<int>(std::lround((100-permanent)*breakPercent*0.0001f*(std::clamp)(extra,0,200)*amount*returnScale));
        if(active&&s.quarter==2)missing=(std::max)(missing,lastAbsent);
        if(active)absenceLimit=(std::min)(absenceLimit,(std::max)(0,lossCeiling(s)-permanent));
        missing=(std::min)(missing,absenceLimit);
        if(s.quarter==3&&!initializing&&!crossing&&!release)missing=(std::min)(missing,lastAbsent);
        temporary=active?(std::max)(permanent,(std::min)(lossCeiling(s),permanent+missing)):0;
        // Remember the actual absence after floors, not the larger request.
        lastAbsent=(std::max)(0,temporary-permanent);
        releaseAtBreak=release&&active;
        if(s.quarter==3)waitingForBreak=false;
        return temporary;
    }
    void convert(const State& s,int before,int after,int total,int extra){
        int remaining=(std::max)(0,total-after);
        float amount=s.quarter==3?context::halftimeReturnWeight(s):1;
        float raw=(100-after)*breakPercent*.0001f*(std::clamp)(extra,0,200)*amount;
        returnScale=raw>0?(std::clamp)(remaining/raw,0.f,1.f):0;
        lastAbsent=remaining;temporary=(std::max)(after,total);
        (void)before;
    }
};
}
