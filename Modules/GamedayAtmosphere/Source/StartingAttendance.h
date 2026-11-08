#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>
#include <array>
#include "StartingAttendanceTransition.h"
#include "MatchupClass.h"
#include "ShowcaseAttendance.h"
#include "AudienceModel.h"

namespace attendance::turnout {
// Expected settled turnout, not the number already in their seats at kickoff.
// Inputs are frozen at launch. All adjustments below are capacity percentage points.
struct Team {
    int id=-1,rank=0,toughest=0,prestige=0,wins=0,losses=0,homeGames=0;
    // LetterGrade is A+=0 through F=12; 13 is Incomplete, never an A+ fallback.
    int atmosphere=-1,tradition=-1,brand=-1,conference=-1;
    float standing=-1;
    int apRank=0; // MediaPoll_CurrentRank; separate from the game's general TeamRank.
    int state=-1;
    audience::Program program;
    double audienceFans=0;
    float opponentRetention=0; // Evidence-based home support; custom/unmapped defaults to none.
    float latitude=NAN,longitude=NAN; // Loaded City coordinates; optional, never school-ID geography.
    double stateLatitude=NAN,stateLongitude=NAN; // Optional, resolved by verified StateName metadata.
    bool militaryKnown=false,military=false; // Native Team.IsMilitary, not an academy-ID list.
    int recordRow=-1; // Owned native Team row; also available without history/grades.
};
struct Seed {
    Team home,away;
    int stadium=-1,homeStadium=-1,homeAlternate=-1,capacity=0,schools=0,week=-1,year=0,ea=0,roof=0;
    bool regular=false,resumed=false,lastHome=false,scheduleKnown=false;
    uint64_t serial=0,observed=0,dynastyVerifiedAt=0,dynastyGeneration=0;
    int dynasty=0;
};
struct Launch {
    int home=-1,away=-1,stadium=-1,capacity=0,ea=0,minutes=-1,precipitation=-1;
    bool rivalry=false,neutral=false,senior=false,seniorKnown=false;
    bool dynamicWeather=false,weatherKnown=true;int configuredPrecipitation=-1;
    int rivalryEvent=-1;
};
struct Result {
    bool applied=false,night=false,exceptional=false,fuller=false;
    int attendance=0,kickoff=55,fillPercent=50;
    float standing=0,score=0,adjustmentLimit=0;
    float demand=0,momentum=0,appeal=0,nightBonus=0,weather=0,variation=0,limits=0,percent=0;
    float hardToFillPenalty=0;
    float breakout=0,eventMultiplier=1,openerBoost=0,nightBoost=0,seniorBoost=0,stateBoost=0,academyBoost=0,accessBonus=0,profileBoost=0;
    int showcaseEvent=-1;
    bool audienceModel=false;double audienceFans=0;
    float opponentRelief=0;
};
inline bool lateArrivalSchool(int team){return team==1212||team==1158||team==1137||team==1156||team==1218||team==1216||team==1223;}
inline bool fcs(int id){return id>=1500&&id<=1504;}
inline bool grade(int value){return value>=0&&value<=12;}
inline bool grades(const Team& t){return grade(t.atmosphere)&&grade(t.tradition)&&grade(t.brand)&&grade(t.conference);}
inline float rankStrength(int rank,int schools){return rank>=1&&rank<=schools?1.f-static_cast<float>(rank-1)/(schools-1):0.f;}
inline float support(const Team& t,int schools){
    // Atmosphere already represents the toughest-place/home-field metric: do
    // not count ToughestPlacesRank again. Prestige is the native 0..10 half-star scale.
    return .35f*(12-t.atmosphere)/12.f+.25f*t.prestige/10.f+.20f*(12-t.tradition)/12.f+
        .10f*(12-t.brand)/12.f+.05f*(12-t.conference)/12.f+.05f*rankStrength(t.apRank,schools);
}
inline bool assignStandings(std::vector<Team>& teams){
    int n=static_cast<int>(teams.size());if(n<100||n>256)return false;
    for(int i=0;i<n;++i){if(!grades(teams[i])||teams[i].rank<1||teams[i].rank>n||teams[i].prestige<0||teams[i].prestige>10||teams[i].apRank<0||teams[i].apRank>256||fcs(teams[i].id))return false;
        for(int j=0;j<i;++j)if(teams[i].id==teams[j].id)return false;}
    for(auto& t:teams){float score=support(t,n);int lower=0,equal=0;
        for(const auto& other:teams){float delta=support(other,n)-score;if(delta<-.000001f)++lower;else if(std::fabs(delta)<=.000001f)++equal;}
        t.standing=(lower+.5f*(equal-1))/(n-1); // Equal inputs receive equal demand, independent of IDs/order.
    }return true;
}
inline float demandCurve(float standing){
    // Aggregate quantiles of the user-supplied 2025 table (136 capacity ratios).
    // >100 is demand headroom; the final rendered count is capped at capacity.
    constexpr float p[]={0,.05f,.10f,.20f,.25f,.40f,.50f,.60f,.75f,.80f,.90f,.95f,1};
    constexpr float y[]={25.76f,38.5775f,47.12f,58.89f,66.145f,81.15f,87.86f,91.56f,97.99f,100,101.435f,103.385f,108.49f};
    float q=(std::clamp)(standing,0.f,1.f);
    for(unsigned i=1;i<std::size(p);++i)if(q<=p[i])return y[i-1]+(y[i]-y[i-1])*(q-p[i-1])/(p[i]-p[i-1]);
    return y[std::size(y)-1];
}
inline bool top(int rank,int cutoff){return rank>=1&&rank<=cutoff;}
inline bool big(int home,int away,bool rivalry){return matchup::big(home,away,rivalry);}
inline bool exceptional(float peak,float standing,int home,int rank0,int rank1,bool neutral,bool night,bool rivalry,bool opener,int bigBonus=3){
    return std::isfinite(peak)&&peak>=95&&standing>=.8f&&bigBonus>0&&
        (neutral||!lateArrivalSchool(home))&&(night||(!neutral&&opener))&&
        ((top(rank0,10)&&top(rank1,10))||(rivalry&&top(rank0,25)&&top(rank1,25)));
}
inline float accessBonus(int capacity){
    if(capacity<1000||capacity>200000)return 0;
    float t=(std::clamp)(std::log((std::max)(15000,capacity)/15000.f)/std::log(100000.f/15000.f),0.f,1.f);
    return 8*(1-t*t*(3-2*t));
}
inline int openingPercent(float standing,int home,int rank0,int rank1,bool neutral,bool night,bool rivalry,bool opener,int rain,bool indoor,int normalBase=55,int severeBase=45,int bigBonus=3,float peak=0,int capacity=0,bool sameState=false,bool fuller=false){
    float start=!neutral&&lateArrivalSchool(home)?static_cast<float>((std::clamp)(severeBase,40,50)):
        (std::clamp)(normalBase+10.f*((std::clamp)(standing,0.f,1.f)-.5f),50.f,60.f);
    start+=(!neutral?accessBonus(capacity)+(sameState?2.f:0.f):0.f);
    start+=(night?5.f:0.f)+(opener&&!neutral?1.f:0.f)+(big(rank0,rank1,rivalry)?static_cast<float>((std::clamp)(bigBonus,0,3)):0.f)+(top(rank0,10)&&top(rank1,10)?1.f:0.f);
    if(fuller)start+=7;
    start=(std::clamp)(start,40.f,70.f);
    if(exceptional(peak,standing,home,rank0,rank1,neutral,night,rivalry,opener,bigBonus))
        start=(std::max)(start,70.f+(5.f+5.f*(std::clamp)((standing-.8f)/.2f,0.f,1.f))*(std::clamp)(bigBonus,0,3)/3.f);
    constexpr float deduction[]={0,3,5,8};if(!indoor&&rain>=1&&rain<=3)start-=deduction[rain];
    return (std::clamp)(static_cast<int>(std::lround(start)),35,80);
}
inline bool validTeam(const Team& t,int schools){
    return t.id>=0&&t.id<2047&&t.toughest>=0&&t.toughest<=(fcs(t.id)?256:schools)&&
        t.rank>=0&&t.rank<=256&&t.prestige>=0&&t.prestige<=10&&t.apRank>=0&&t.apRank<=256&&
        t.wins>=0&&t.losses>=0&&t.wins+t.losses<=30&&t.homeGames>=0&&t.homeGames<=20;
}
inline float momentum(const Team& t){
    int games=t.wins+t.losses;if(!games)return 0;
    float confidence=(std::min)(1.f,games/6.f);
    return (std::clamp)((t.wins/static_cast<float>(games)-.5f)*12.f*confidence,-6.f,6.f);
}
inline float opponent(const Team& t){
    if(fcs(t.id))return -10;
    if(t.rank>=1&&t.rank<=25)return 2.f+4.f*(26-t.rank)/25.f;
    if(t.rank>100)return -6;
    return 0;
}
inline bool sameState(const Seed& s){return !fcs(s.home.id)&&!fcs(s.away.id)&&s.home.state>=0&&s.home.state<=100&&s.home.state==s.away.state;}
inline bool academy(int id){return id==1100||id==1108||id==1166;}
inline float breakoutLift(const Team& t,float base){
    int games=t.wins+t.losses;if(games<4)return 0;
    float confidence=(std::clamp)((games-3)/6.f,0.f,1.f);
    float win=(std::clamp)((t.wins/static_cast<float>(games)-.5f)/.4f,0.f,1.f);win=win*win*(3-2*win);
    float poll=top(t.apRank,25)?(26-t.apRank)/25.f:0.f;
    float target=68+17*poll;
    return (std::max)(0.f,target-base)*confidence*win;
}
inline uint32_t fixtureHash(const Seed& s,bool neutral=false){
    uint32_t h=2166136261u;
    int a=s.home.id,b=s.away.id;if(neutral&&a>b)std::swap(a,b);
    for(int n:{a,b,s.stadium,s.week,s.year}){h^=static_cast<uint32_t>(n);h*=16777619u;}
    return h;
}
inline void applyShowcase(const Seed& s,const Launch& l,Result& r){
    if(!r.applied||!l.rivalry||!showcase::event(l.rivalryEvent))return;
    r.showcaseEvent=l.rivalryEvent;r.limits+=100-r.percent;r.attendance=l.capacity;r.percent=100;
    r.kickoff=showcase::opening(l.rivalryEvent,s.week,s.year);r.fillPercent=30;r.exceptional=true;
}
inline Result calculate(const Seed& s,const Launch& l,bool fuller=false){
    Result r;r.attendance=l.ea;
    if(!s.regular||s.resumed||s.week<0||s.week>31||s.year<0||s.year>4095||s.schools<100||s.schools>512||!validTeam(s.home,s.schools)||!validTeam(s.away,s.schools)||
       s.home.id==s.away.id||l.home!=s.home.id||l.away!=s.away.id||l.stadium!=s.stadium||l.ea!=s.ea||
       l.capacity<1000||l.capacity>200000||l.ea<1||l.ea>l.capacity*2||l.minutes<0||l.minutes>=1440||l.precipitation<0||l.precipitation>3||
       !grades(s.home)||!std::isfinite(s.home.standing)||s.home.standing<0||s.home.standing>1||
       (l.neutral&&(!grades(s.away)||!std::isfinite(s.away.standing)||s.away.standing<0||s.away.standing>1)))return r;
    r.night=l.minutes>=1140; // Verified FUNCTIONKEY_ISNIGHTGAME: stadium-local kickoff >=19:00.
    r.standing=s.home.standing;r.score=support(s.home,s.schools);r.momentum=momentum(s.home);
    r.appeal=opponent(s.away)+(l.rivalry?6.f:0.f);
    if(l.neutral){
        r.standing=.5f*(r.standing+s.away.standing);r.score=.5f*(r.score+support(s.away,s.schools));
        r.momentum=.5f*(r.momentum+momentum(s.away));
        r.appeal=.5f*(opponent(s.home)+opponent(s.away))+(l.rivalry?6.f:0.f);
    }
    r.demand=demandCurve(r.standing);
    float changes=r.momentum+r.appeal;
    r.adjustmentLimit=(std::clamp)(changes,-15.f,12.f)-changes;
    float ordinary=r.demand+changes+r.adjustmentLimit;
    r.breakout=breakoutLift(s.home,ordinary);
    if(l.neutral)r.breakout=.5f*(breakoutLift(s.home,ordinary)+breakoutLift(s.away,ordinary));
    uint32_t hash=fixtureHash(s,l.neutral);
    auto varying=[&](unsigned salt,float low,float high){uint32_t n=hash^(salt*0x9e3779b9u);n^=n>>16;n*=0x7feb352du;n^=n>>15;return low+(high-low)*(n%10001)/10000.f;};
    if(!l.neutral&&s.home.homeGames==0)r.openerBoost=varying(1,.10f,.25f);
    if(r.night)r.nightBoost=varying(2,.05f,.15f);
    if(!l.neutral&&((l.seniorKnown&&l.senior)||(!l.seniorKnown&&s.scheduleKnown&&s.lastHome)))r.seniorBoost=.10f;
    if(sameState(s))r.stateBoost=varying(3,.05f,.10f);
    if(academy(s.home.id)&&academy(s.away.id))r.academyBoost=varying(4,.05f,.15f);
    float local=r.stateBoost*(l.rivalry?.5f:1.f);
    r.eventMultiplier=(std::min)(1.40f,(1+r.openerBoost)*(1+r.nightBoost)*(1+r.seniorBoost)*(1+local)*(1+r.academyBoost));
    r.nightBonus=(ordinary+r.breakout)*r.nightBoost;
    const float weather[]={0.f,5.f,9.f,14.f};r.weather=s.roof>=1&&s.roof<=3?0.f:-weather[l.precipitation];
    r.variation=static_cast<float>(static_cast<int>(hash%7)-3);
    r.fuller=fuller;r.profileBoost=fuller?8.f:0.f;
    // Soft demand discount before the capacity clamp, never a school-specific cap.
    // Special event boosts retain their headroom; protected rivalry policy follows below.
    float hardToFill=!l.neutral?(s.home.id==1212?.14f:s.home.id==1216?.10f:0.f):0.f;
    r.hardToFillPenalty=(ordinary+r.breakout)*r.eventMultiplier*hardToFill;
    float raw=(ordinary+r.breakout)*r.eventMultiplier-r.hardToFillPenalty+r.weather+r.variation+r.profileBoost;
    r.percent=(std::clamp)(raw,r.weather<0?20.f:25.f,100.f);r.limits=r.percent-raw;
    r.attendance=(std::clamp)(static_cast<int>(std::lround(l.capacity*r.percent*.01f)),1,l.capacity);
    r.exceptional=exceptional(r.percent,r.standing,s.home.id,s.home.rank,s.away.rank,l.neutral,r.night,l.rivalry,s.home.homeGames==0);
    r.kickoff=openingPercent(r.standing,s.home.id,s.home.rank,s.away.rank,l.neutral,r.night,l.rivalry,s.home.homeGames==0,l.precipitation,s.roof>=1&&s.roof<=3,55,45,3,r.percent,l.capacity,sameState(s),fuller);
    r.accessBonus=!l.neutral?accessBonus(l.capacity):0;
    r.fillPercent=!l.neutral&&lateArrivalSchool(s.home.id)?75:big(s.home.rank,s.away.rank,l.rivalry)?30:50;
    r.applied=true;applyShowcase(s,l,r);return r;
}
// Absolute audience model. Legacy calculation supplies unchanged arrival,
// identity, event and showcase policies; it is not a fallback peak formula.
inline Result calculateAudience(const Seed& s,const Launch& l,bool fuller=false){
    auto r=calculate(s,l,fuller);
    if(!r.applied||!std::isfinite(s.home.audienceFans)||s.home.audienceFans<=0||
       (l.neutral&&(!std::isfinite(s.away.audienceFans)||s.away.audienceFans<=0))){r.applied=false;return r;}
    double fans=l.neutral?.5*(s.home.audienceFans+s.away.audienceFans):s.home.audienceFans;
    r.audienceModel=true;r.audienceFans=fans;
    // Preserve matchup/current-season point strengths on a fixed 75-point
    // reference, so changing the venue cannot create demand for those bonuses.
    auto soften=[](float appeal,float retention){return appeal<0&&std::isfinite(retention)?appeal*(1-.75f*(std::clamp)(retention,0.f,1.f)):appeal;};
    float appeal=soften(opponent(s.away),s.home.opponentRetention);
    if(l.neutral)appeal=.5f*(appeal+soften(opponent(s.home),s.away.opponentRetention));
    appeal+=l.rivalry?6.f:0.f;r.opponentRelief=appeal-r.appeal;r.appeal=appeal;
    float changes=(std::clamp)(r.momentum+r.appeal,-15.f,12.f);
    double temporary=1+changes/75.0;
    auto breakout=[](const Team& t){int games=t.wins+t.losses;if(games<4)return 0.f;
        float confidence=(std::clamp)((games-3)/6.f,0.f,1.f),win=(std::clamp)((t.wins/static_cast<float>(games)-.5f)/.4f,0.f,1.f);
        win=win*win*(3-2*win);float poll=top(t.apRank,25)?(26-t.apRank)/25.f:0;
        return .15f*confidence*win*(.5f+.5f*poll);};
    float extra=l.neutral?.5f*(breakout(s.home)+breakout(s.away)):breakout(s.home);
    double base=fans*(temporary+extra),weatherFactor=1+r.weather/100.0,variationFactor=1+r.variation/100.0;
    double hard=!l.neutral?(s.home.id==1212?.14:s.home.id==1216?.10:0):0;
    double planned=base*r.eventMultiplier*(1-hard)*weatherFactor*variationFactor;
    r.demand=static_cast<float>(100*fans/l.capacity);r.breakout=static_cast<float>(100*fans*extra/l.capacity);
    r.hardToFillPenalty=static_cast<float>(100*base*r.eventMultiplier*hard/l.capacity);
    r.nightBonus=static_cast<float>(100*base*r.nightBoost/l.capacity);
    // Fuller is an explicit user-selected capacity override, not audience growth.
    double raw=100*planned/l.capacity+r.profileBoost;
    r.percent=static_cast<float>((std::clamp)(raw,0.0,100.0));r.limits=static_cast<float>(r.percent-raw);
    r.attendance=(std::clamp)(static_cast<int>(std::lround(l.capacity*r.percent*.01)),1,l.capacity);
    r.kickoff=openingPercent(r.standing,s.home.id,s.home.rank,s.away.rank,l.neutral,r.night,l.rivalry,s.home.homeGames==0,l.precipitation,s.roof>=1&&s.roof<=3,55,45,3,r.percent,l.capacity,sameState(s),fuller);
    r.exceptional=exceptional(r.percent,r.standing,s.home.id,s.home.rank,s.away.rank,l.neutral,r.night,l.rivalry,s.home.homeGames==0);
    applyShowcase(s,l,r);return r;
}
// Server simulation calls can interleave with a playable launch. Match the actual
// launch IDs, native count and stadium; never use a reused match pointer as identity.
class Seeds {
    std::vector<Seed> recent_;uint64_t serial_=0;
public:
    void remember(Seed s,uint64_t now){
        s.serial=++serial_;s.observed=now;recent_.push_back(s);
        if(recent_.size()>32)recent_.erase(recent_.begin());
    }
    void clear(){recent_.clear();}
    bool find(const Launch& l,uint64_t now,Seed& out)const{
        for(auto p=recent_.rbegin();p!=recent_.rend();++p){
            if(p->home.id!=l.home||p->away.id!=l.away||p->stadium!=l.stadium||p->ea!=l.ea)continue;
            if(now<p->observed||now-p->observed>120000)return false;
            out=*p;return true; // Native loading may copy the same launch options twice.
        }
        return false;
    }
};
inline bool capturedLaunchProof(const Seed& s,const Launch& l,uint64_t now,uint64_t generation){
    return s.serial&&s.dynastyGeneration==generation&&generation&&s.dynastyVerifiedAt&&s.dynastyVerifiedAt<=s.observed&&now>=s.observed&&
        now-s.dynastyVerifiedAt<=ModeWitness::Lifetime&&s.home.id==l.home&&s.away.id==l.away&&s.stadium==l.stadium&&s.ea==l.ea;
}
inline bool transitionProof(const Seed& s,const Launch& l,uint64_t now,uint64_t generation){return s.regular&&!s.resumed&&capturedLaunchProof(s,l,now,generation);}
struct Applied {
    Seed seed;Launch launch;Result result;
    bool owns(int home,int away)const{return result.applied&&launch.home==home&&launch.away==away;}
};
// Appearance/supporter identity must not depend on whether DGA replaced EA's
// attendance count. Publish only after the shared launch ownership checks.
struct CrowdFixture {
    Seed seed;Launch launch;bool verified=false;
    bool owns(int home,int away)const{return verified&&home>=0&&away>=0&&home!=away&&
        seed.home.id==home&&seed.away.id==away&&launch.home==home&&launch.away==away&&
        seed.stadium>=0&&seed.stadium==launch.stadium&&seed.ea>0&&seed.ea==launch.ea&&
        seed.capacity>=1000&&seed.capacity<=200000&&seed.capacity==launch.capacity;}
};
}
