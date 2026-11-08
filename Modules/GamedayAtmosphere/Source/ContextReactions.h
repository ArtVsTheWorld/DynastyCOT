#pragma once
#include "ExcitementRules.h"
#include "PackedHouse.h"
namespace attendance::contextreaction {
inline bool premium(const State& s,float fullness,uint64_t positions){
    return bigGameMatchup(s)&&packed::qualifies(positions,fullness)&&std::abs(s.homeScore-s.awayScore)<=16;
}
inline float strength(const energy::Play& snap,const State& current,bool qualified){
    if(!qualified)return 1;
    float close=1-(std::clamp)((std::abs(snap.game.homeScore-snap.game.awayScore)-3)/13.f,0.f,1.f);
    float clock=current.quarter>=4?1-(std::clamp)(current.seconds/(std::max)(1.f,current.quarterSeconds*.25f),0.f,1.f):0;
    float goal=snap.toGoal>=0&&snap.toGoal<=10?1-snap.toGoal/15.f:0;
    float down=snap.down>=3?.5f:0;
    return 1+.05f+.10f*close+.10f*clock+.05f*(std::max)(goal,down);
}
inline bool kneelable(const energy::Play& after,int side){
    const auto& s=after.game;
    if(s.quarter!=4||s.seconds<0||s.seconds>120||after.offense!=side)return false;
    int timeouts=side==0?after.awayTimeouts:after.homeTimeouts;
    return timeouts>=0&&timeouts<=2&&s.seconds<=(3-timeouts)*40.f;
}
inline energy::Outcome apply(energy::Outcome r,const energy::Play& snap,const energy::Play& after,float fullness,uint64_t positions,bool enabled=true){
    if(!r.accepted||!r.completed||!r.sequence||!energy::normalDown(snap)||energy::clockManagement(snap))return r;
    bool own=snap.offense==1||after.game.neutral;
    bool noScore=!r.homePoints&&!r.awayPoints;
    bool stopped=noScore&&!r.firstDown&&!snap.specialTeams&&(after.offense!=snap.offense||after.down>snap.down);
    if(own&&snap.toGoal>=0&&snap.toGoal<=5&&stopped){
        if(snap.down==3){r.thirdGoalStop=true;r.side=1-snap.offense;r.event=energy::preferred(r.event,energy::Event::ThirdStop);}
        if(snap.down==4&&after.offense!=snap.offense){r.goalLineStop=true;r.side=1-snap.offense;r.event=energy::preferred(r.event,energy::Event::GoalLine);}
    }
    bool qualified=enabled&&premium(snap.game,fullness,positions);
    r.contextStrength=strength(snap,after.game,qualified);
    if(!qualified||r.side<0||r.side>1||(!after.game.neutral&&r.side!=0)||!noScore)return r;
    int lead=r.side==0?after.game.homeScore-after.game.awayScore:after.game.awayScore-after.game.homeScore;
    if(lead<=0||lead>8||std::abs(snap.game.homeScore-snap.game.awayScore)>8)return r;
    bool final=after.game.phase==7&&after.game.quarter>=4;
    bool ownsKneels=kneelable(after,r.side)&&after.down==1;
    bool defense=r.takeaway||r.goalLineStop||r.sack||r.blockedFieldGoal||r.event==energy::Event::FourthStop;
    r.contextEruption=defense&&(final||ownsKneels);
    r.contextClincher=r.firstDown&&snap.offense==r.side&&!snap.specialTeams&&(final||ownsKneels);
    if(r.contextEruption)r.event=energy::preferred(r.event,energy::Event::GameSealingStop);
    else if(r.contextClincher)r.event=energy::preferred(r.event,energy::Event::GameSealingFirstDown);
    return r;
}
// A clock-expiring sack/stop can settle before the world announces Final.
// Retain its evidence briefly; only an actual final state can upgrade it.
class Model {
    uintptr_t match_=0;unsigned epoch_=0;uint64_t sequence_=0,until_=0,last_=0;
    energy::Outcome candidate_;energy::Play snap_;int home_=0,away_=0;
public:
    void reset(){*this=Model{};}
    energy::Outcome observe(energy::Outcome r,const energy::Play& snap,const energy::Play& after,uint64_t sequence,uintptr_t match,unsigned epoch,uint64_t now,float fullness,uint64_t positions,bool enabled){
        if(match!=match_||epoch!=epoch_){reset();match_=match;epoch_=epoch;}
        if(!enabled||!match||after.simulated||now<last_){until_=0;last_=now;return apply(r,snap,after,fullness,positions,false);}last_=now;
        if(after.game.phase==3||sequence!=sequence_||now>until_)until_=0;
        r=apply(r,snap,after,fullness,positions,true);
        if(r.accepted&&r.completed&&r.sequence==sequence&&normalCandidate(r)&&premium(snap.game,fullness,positions)&&snap.game.quarter==4&&after.game.seconds<=3){
            candidate_=r;snap_=snap;sequence_=sequence;until_=now+20000;home_=after.game.homeScore;away_=after.game.awayScore;
            if(r.contextEruption||r.contextClincher)until_=0;
        }
        if(until_&&after.game.phase==7&&sequence==sequence_&&after.game.homeScore==home_&&after.game.awayScore==away_){
            auto final=apply(candidate_,snap_,after,fullness,positions,true);until_=0;
            if(final.contextEruption||final.contextClincher)return final;
        }
        return r;
    }
private:
    static bool normalCandidate(const energy::Outcome& r){return !r.homePoints&&!r.awayPoints&&(r.sack||r.takeaway||r.goalLineStop||r.thirdGoalStop||r.blockedFieldGoal||r.event==energy::Event::FourthStop||r.firstDown);}
};
}
