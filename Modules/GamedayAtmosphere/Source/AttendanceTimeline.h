#pragma once
#include <cstdint>
#include <deque>
#include <iomanip>
#include <istream>
#include <ostream>
#include <vector>
#include "DepartureModel.h"
#include "ArrivalRules.h"

namespace attendance::timeline {
struct ReplayPolicy {bool historical=false,suspendAtmosphere=false;};
inline ReplayPolicy replayPolicy(bool nativeActive,bool worldPaused,bool finalMenu){
    // Automatic presentation clips share the native recording timeline. Their
    // visual history must not pause the live celebration/atmosphere model.
    return {nativeActive,nativeActive&&(worldPaused||finalMenu)};
}
inline bool ownsReplayCrowd(bool verifiedGame,uintptr_t current,uintptr_t initialized,bool contextReady,bool currentSession=true){
    return verifiedGame&&contextReady&&currentSession&&current&&current==initialized;
}
inline bool recordingRestarted(bool validSnapshot,double recording,double lastRecording,const State& current,const State& previous){
    return validSnapshot&&lastRecording>=0&&recording+2<lastRecording&&elapsedGameTime(current)+1<elapsedGameTime(previous);
}
inline bool recordingTime(bool bindingAvailable,bool snapshotValid,double nativeTime,double gameTime,double& result){
    // Recording time includes presentation; live game-clock time does not.
    // A transient missing manager must not mix those two time bases in history.
    if(bindingAvailable&&!snapshotValid)return false;
    double chosen=bindingAvailable?nativeTime:gameTime;
    if(!std::isfinite(chosen)||chosen<0||chosen>86400)return false;
    result=chosen;return true;
}
struct Point {
    double time=0;State state;int loss=0,temporary=0,periodStart=0;History history;
    float homeLoss=-1,awayLoss=-1,originalAwayShare=-1; // v6 supporter history; -1 for older records.
    bool sharedBudget=true; // v7: temporary percentage includes protected spectators.
    int total()const{return (std::max)(loss,temporary);}
};
struct Game {uint64_t id=0;State context;std::vector<Point> points;bool resumeReady=false;};
inline bool legacyProgress(const std::vector<Point>& points){
    // Formats 1-4 did not store phases or readiness. Require clock progression
    // between two midgame samples, not the loading jump from the opening clock.
    for(size_t i=1;i<points.size();++i){
        const auto& a=points[i-1];const auto& b=points[i];
        if((a.state.quarter>1||a.state.seconds<a.state.quarterSeconds*.95f)&&
           (b.state.quarter>1||b.state.seconds<b.state.quarterSeconds*.95f)&&
           b.time>a.time&&elapsedGameTime(b.state)>elapsedGameTime(a.state)+.01f)return true;
    }
    return false;
}
struct Archive {
    static constexpr size_t MaxGames=8,MaxPoints=4096;
    std::deque<Game> games;uint64_t active=0,revision=0;
    void begin(uint64_t id,const State& context,bool restored=false){
        active=id;games.push_back(Game{id,context,{},restored});while(games.size()>MaxGames)games.pop_front();++revision;
    }
    bool record(const Point& p,bool checkpoint=false){
        if(games.empty()||games.back().id!=active||!valid(p.state)||!std::isfinite(p.time)||p.time<0||p.loss<0||p.loss>100||p.temporary<0||p.temporary>100||p.periodStart<0||p.periodStart>p.loss)return false;
        Point owned=p;copyCrowdPlan(games.back().context,owned.state);
        auto& v=games.back().points;
        if(!v.empty()&&p.time+0.01<v.back().time)return false;
        // Phase 3 is an actual live play. Pregame, lineup, play selection and
        // loading-time clock jumps cannot establish a new resume source.
        if(!games.back().resumeReady&&p.state.phase==3){games.back().resumeReady=true;++revision;}
        if(!v.empty()){
            const auto& last=v.back();
            if(p.loss==last.loss&&p.temporary==last.temporary&&p.sharedBudget==last.sharedBudget&&p.state.quarter==last.state.quarter&&p.state.homeScore==last.state.homeScore&&p.state.awayScore==last.state.awayScore&&
               (checkpoint?std::abs(p.state.seconds-last.state.seconds)<.01f:p.time-last.time<5))return false;
        }
        if(v.size()>=MaxPoints){
            // Keep the opening and all crowd changes; compact equal-target runs.
            std::vector<Point> compact;compact.reserve(v.size());compact.push_back(v.front());
            for(size_t i=1;i+1<v.size();++i)if(i%2==0||v[i].total()!=v[i-1].total()||v[i].total()!=v[i+1].total())compact.push_back(v[i]);
            compact.push_back(v.back());v=std::move(compact);
            if(v.size()>=MaxPoints)v.erase(v.begin()+1);
        }
        v.push_back(owned);++revision;return true;
    }
    bool replay(double time,Point& out)const{
        if(games.empty()||games.back().id!=active||games.back().points.empty()||!std::isfinite(time)||time<0)return false;
        const auto& v=games.back().points;auto at=std::upper_bound(v.begin(),v.end(),time+0.01,[](double t,const Point& p){return t<p.time;});
        out=at==v.begin()?v.front():*std::prev(at);return true;
    }
    bool resume(const State& s,Point& out,uint64_t sourceGame=0)const{
        // Without a stable Midgame Save ID, reuse only a close clock/score match
        // in a recorded game of these teams. Otherwise estimate explicitly.
        for(auto g=games.rbegin();g!=games.rend();++g){
            if(sourceGame&&g->id!=sourceGame)continue;
            if(!g->resumeReady)continue;
            if(g->context.team0!=s.team0||g->context.team1!=s.team1||g->context.quarterSeconds!=s.quarterSeconds||g->context.playoff!=s.playoff||g->context.conferenceChampionship!=s.conferenceChampionship||g->context.bowl!=s.bowl||
                g->context.rivalry!=s.rivalry||g->context.neutral!=s.neutral||g->context.spring!=s.spring)continue;
            if(s.spring&&(g->context.springDynasty!=s.springDynasty||g->context.springSchool!=s.springSchool||g->context.springNumber!=s.springNumber||g->context.plannedYear!=s.plannedYear||g->context.plannedWeek!=s.plannedWeek))continue;
            // Native ranks and weather can refresh while a saved match loads.
            // A fixture-owned source ID removes that ambiguity; legacy callers
            // without ownership still need the stricter historical heuristic.
            if(!sourceGame&&(g->context.rank0!=s.rank0||g->context.rank1!=s.rank1||g->context.precipitation!=s.precipitation))continue;
            if(s.plannedStadium>=0&&g->context.plannedStadium>=0&&s.plannedStadium!=g->context.plannedStadium)continue;
            if(crowdPlan(s)&&crowdPlan(g->context)&&(s.plannedWeek!=g->context.plannedWeek||s.plannedYear!=g->context.plannedYear))continue;
            // Equal-density history is sampled every five seconds. Native save
            // clock rounding must not reject the last real sample at 3.1s away.
            const Point* best=nullptr;float distance=5.01f;
            for(const auto& p:g->points)if(p.state.quarter==s.quarter&&p.state.homeScore==s.homeScore&&p.state.awayScore==s.awayScore){
                float d=std::abs(p.state.seconds-s.seconds);if(d<=distance){distance=d;best=&p;}
            }
            if(best){out=*best;return true;}
        }return false;
    }
};
struct Bootstrap {departures::Model departures;arrivals::Model arrivals;arrivals::Halftime halftime;History history;int loss=0,temporary=0;};
inline Bootstrap estimate(const Settings& c,const arrivals::Settings& a,State s,const Point* saved=nullptr,int halftimeExtra=100){
    Bootstrap out;if(!valid(s))return out;float elapsed=elapsedGameTime(s);
    if(saved){
        s.variationSeed=saved->state.variationSeed;copyCrowdPlan(saved->state,s);
        out.loss=(std::clamp)(saved->loss,0,100);out.temporary=(std::clamp)(saved->temporary,0,100);out.history=saved->history;
        out.history.elapsed=elapsed;out.history.lastHome=s.homeScore;out.history.lastAway=s.awayScore;
        out.departures.periodStartLoss=(std::clamp)(saved->periodStart,0,out.loss);
    }else{
        // An estimate, not invented play history: ramp the present score over
        // elapsed game time so a late deficit is not backdated to kickoff.
        History synthetic;float end=(std::min)(elapsed,4*s.quarterSeconds);
        float step=s.quarterSeconds/60.0f;
        for(float t=0;;t=(std::min)(end,t+step)){
            State x=s;x.phase=5;x.quarter=(std::min)(4,1+static_cast<int>(t/s.quarterSeconds));x.seconds=x.quarter*s.quarterSeconds-t;
            x.offense=x.homeTimeouts=-1;x.toGoal=-1; // The present possession does not describe unrecorded earlier drives.
            float f=end>0?t/end:0;x.homeScore=static_cast<int>(std::lround(s.homeScore*f));x.awayScore=static_cast<int>(std::lround(s.awayScore*f));
            synthetic.observe(c,x);out.departures.observe(c,x,synthetic);out.loss=out.departures.settle(x,out.loss,0,c.quarterCap).loss;
            if(t>=end)break;
        }
        // Only real observed state establishes comeback history going forward.
        out.history.observe(c,s,true);
    }
    if(!c.enabled){out.loss=out.temporary=0;}
    auto& d=out.departures;d.seen=true;d.automatic=c.enabled!=0;d.elapsed=elapsed;d.length=s.quarterSeconds;d.previous=s;d.priorHistory=out.history;
    d.hazard=d.committedHazard=-std::log((std::max)(0.000001,1-out.loss/100.0));d.committedLoss=out.loss;
    d.capPeriod=(std::min)(4,1+static_cast<int>(elapsed/s.quarterSeconds+0.00001f));d.minimumStart=elapsed<2*s.quarterSeconds?(std::max)(elapsed,s.quarterSeconds):-1;
    d.plannedLoss=out.loss;d.plannedPeriod=d.capPeriod;d.plannedStart=d.periodStartLoss;
    // Restore the current arrival curve without replaying a fictional kickoff.
    auto& ar=out.arrivals;ar.seen=true;ar.startMissing=100-arrivals::openingPercent(a,s);ar.fillSeconds=s.quarterSeconds*arrivals::finishFraction(a,s);
    ar.elapsed=s.quarterSeconds-s.seconds;ar.missing=a.enabled&&c.enabled&&s.quarter==1?arrivals::missingAt(ar.startMissing,ar.elapsed,ar.fillSeconds,s):0;
    if(saved&&a.enabled&&c.enabled&&s.quarter==1)ar.missing=(std::min)(ar.startMissing,out.temporary);
    ar.active=ar.missing>0;ar.done=!ar.active;
    auto breakTiming=context::halftimeTiming(s);
    if(c.enabled&&((s.quarter==2&&s.seconds<s.quarterSeconds*breakTiming.departure)||(s.quarter==3&&s.quarterSeconds-s.seconds<s.quarterSeconds*breakTiming.lastReturn)))out.halftime.active=true;
    State temp=s;temp.phase=5;int bathroom=c.enabled?out.halftime.observe(temp,out.loss,halftimeExtra):0;
    if(saved&&s.quarter==3){
        // A recorded return must not restart the longer new break window or
        // invent extra absences when loading history from an older version.
        auto& half=out.halftime;half.seen=true;half.highTime=elapsed;
        half.lastAbsent=(std::max)(0,out.temporary-out.loss);half.temporary=out.temporary;half.lastQuarter=3;
        half.breakPercent=context::halftime(s).temporary;
        half.active=half.lastAbsent>0;half.done=!half.active;
    }
    if(!saved)out.temporary=(std::max)(ar.missing,bathroom);
    if(spring::verified(s)){ar.missing=0;ar.active=false;ar.done=true;}
    out.history.warningPending=false;out.history.warningDone=s.quarter>=4&&s.seconds<=120;
    for(int q=2;q<=s.quarter&&q<=4;++q)out.history.quarterMask|=1u<<q;
    return out;
}
// Text format with explicit fields and bounded reads; never dumps native structs.
inline void write(std::ostream& f,const Archive& a){
    f<<"DGA-HISTORY-8 "<<a.games.size()<<'\n'<<std::setprecision(17);
    for(const auto& g:a.games){
        const auto& s=g.context;f<<g.id<<' '<<s.team0<<' '<<s.team1<<' '<<s.quarterSeconds<<' '<<s.playoff<<' '<<s.bowl<<' '<<s.rank0<<' '<<s.rank1<<' '<<s.rivalry<<' '<<s.neutral<<' '<<s.precipitation<<' '<<g.points.size()<<' '<<s.variationSeed<<' '<<s.conferenceChampionship;
        bool plan=crowdPlan(s);f<<' '<<plan<<' '<<s.night;
        if(plan)f<<' '<<s.plannedKickoff<<' '<<s.plannedFill<<' '<<s.plannedPeak<<' '<<s.plannedCapacity<<' '<<s.plannedStadium<<' '<<s.plannedWeek<<' '<<s.plannedYear<<' '<<s.exceptionalOpening;
        f<<' '<<g.resumeReady;
        f<<' '<<s.spring<<' '<<s.springSchool<<' '<<s.springDynasty<<' '<<s.springNumber;
        if(s.spring)f<<' '<<s.plannedWeek<<' '<<s.plannedYear;
        f<<'\n';
        for(const auto& p:g.points){const auto& h=p.history;
            f<<p.time<<' '<<p.state.quarter<<' '<<p.state.seconds<<' '<<p.state.homeScore<<' '<<p.state.awayScore<<' '<<p.loss<<' '<<p.temporary<<' '<<p.periodStart<<' '
             <<h.comeback<<' '<<h.released<<' '<<h.maxDeficit<<' '<<h.bestDeficit<<' '<<h.watchStage<<' '<<h.comebackSide<<' '<<h.maxAwayDeficit<<' '<<h.deteriorationSince<<' '<<p.homeLoss<<' '<<p.awayLoss<<' '<<p.originalAwayShare<<' '<<p.sharedBudget<<'\n';}
    }
}
inline bool read(std::istream& f,Archive& target){
    Archive a;std::string magic;size_t games=0;if(!(f>>magic>>games)||magic.size()!=13||magic.compare(0,12,"DGA-HISTORY-")||magic[12]<'1'||magic[12]>'8'||games>Archive::MaxGames)return false;
    const int version=magic[12]-'0';
    for(size_t i=0;i<games;++i){Game g;size_t count=0;int playoff=0,rivalry=0,neutral=0;
        if(!(f>>g.id>>g.context.team0>>g.context.team1>>g.context.quarterSeconds>>playoff>>g.context.bowl>>g.context.rank0>>g.context.rank1>>rivalry>>neutral>>g.context.precipitation>>count)||!g.id||count>Archive::MaxPoints||g.context.team0<0||g.context.team0>=2047||g.context.team1<0||g.context.team1>=2047||playoff<0||playoff>1||rivalry<0||rivalry>1||neutral<0||neutral>1||g.context.rank0< -1||g.context.rank0>256||g.context.rank1< -1||g.context.rank1>256||g.context.precipitation<0||g.context.precipitation>2||g.context.bowl<0||g.context.bowl>255||!std::isfinite(g.context.quarterSeconds)||g.context.quarterSeconds<60||g.context.quarterSeconds>1200)return false;
        if(version>=2){uint64_t seed=0;if(!(f>>seed)||seed>0xffffffffull)return false;g.context.variationSeed=static_cast<unsigned>(seed);}
        if(version>=3){int conference=0;if(!(f>>conference)||conference<0||conference>1)return false;g.context.conferenceChampionship=conference!=0;}
        if(version>=4){
            int plan=0,night=0,exceptional=0;if(!(f>>plan>>night)||plan<0||plan>1||night<0||night>1)return false;g.context.night=night!=0;
            if(plan){auto& s=g.context;s.startingAttendance=true;
                if(!(f>>s.plannedKickoff>>s.plannedFill>>s.plannedPeak>>s.plannedCapacity>>s.plannedStadium>>s.plannedWeek>>s.plannedYear>>exceptional)||exceptional<0||exceptional>1)return false;
                s.exceptionalOpening=exceptional!=0;if(!crowdPlan(s))return false;
            }
        }
        if(version>=5){int ready=0;if(!(f>>ready)||ready<0||ready>1)return false;g.resumeReady=ready!=0;}
        if(version>=8){auto& s=g.context;int springFlag=0;
            if(!(f>>springFlag>>s.springSchool>>s.springDynasty>>s.springNumber)||springFlag<0||springFlag>1)return false;
            s.spring=springFlag!=0;
            if(s.spring&&(!(f>>s.plannedWeek>>s.plannedYear)||!spring::verified(s)||s.plannedWeek<0||s.plannedWeek>31||s.plannedYear<0||s.plannedYear>4095))return false;
        }
        g.context.playoff=playoff!=0;g.context.rivalry=rivalry!=0;g.context.neutral=neutral!=0;double last=-1;
        for(size_t j=0;j<count;++j){Point p;p.sharedBudget=false;p.state=g.context;auto& h=p.history;int comeback=0,released=0;
            if(!(f>>p.time>>p.state.quarter>>p.state.seconds>>p.state.homeScore>>p.state.awayScore>>p.loss>>p.temporary>>p.periodStart>>comeback>>released>>h.maxDeficit>>h.bestDeficit>>h.watchStage>>h.comebackSide>>h.maxAwayDeficit>>h.deteriorationSince))return false;
            if(!valid(p.state)||!std::isfinite(p.time)||p.time<0||p.time<last||p.time>86400||p.loss<0||p.loss>100||p.temporary<0||p.temporary>100||p.periodStart<0||p.periodStart>p.loss||comeback<0||comeback>1||released<0||released>1||h.maxDeficit<0||h.maxDeficit>255||h.bestDeficit< -255||h.bestDeficit>255||h.watchStage<0||h.watchStage>3||h.comebackSide< -1||h.comebackSide>1||h.maxAwayDeficit<0||h.maxAwayDeficit>255||!std::isfinite(h.deteriorationSince))return false;
            if(version>=6){
                if(!(f>>p.homeLoss>>p.awayLoss>>p.originalAwayShare)||!std::isfinite(p.homeLoss)||!std::isfinite(p.awayLoss)||!std::isfinite(p.originalAwayShare)||p.homeLoss< -1||p.homeLoss>1||p.awayLoss< -1||p.awayLoss>1||p.originalAwayShare< -1||p.originalAwayShare>1)return false;
                bool missing=p.homeLoss==-1&&p.awayLoss==-1&&p.originalAwayShare==-1;bool known=p.homeLoss>=0&&p.awayLoss>=0&&p.originalAwayShare>=0;if(!missing&&!known)return false;
            }
            if(version>=7){int shared=0;if(!(f>>shared)||shared<0||shared>1)return false;p.sharedBudget=shared!=0;}
            last=p.time;h.seen=true;h.comeback=comeback!=0;h.released=released!=0;h.elapsed=elapsedGameTime(p.state);h.lastHome=p.state.homeScore;h.lastAway=p.state.awayScore;h.upsetWatch=h.watchStage>0;g.points.push_back(p);
        }if(version<5)g.resumeReady=legacyProgress(g.points);a.games.push_back(std::move(g));
    }
    f>>std::ws;if(!f.eof())return false;target=std::move(a);return true;
}
}
