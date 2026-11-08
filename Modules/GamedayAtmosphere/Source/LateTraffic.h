#pragma once
#include "AttendanceContext.h"
namespace attendance::traffic {
inline float window(const State& s){
    int deficit=s.awayScore-s.homeScore;
    // Keep the existing postseason and smaller-deficit policy. Clear home
    // blowouts begin in the last third; 28+ can start gently halfway through Q4.
    return s.quarterSeconds*(s.playoff||deficit<17?.25f:deficit>=28?.5f:1.f/3);
}
inline bool lateHomeGame(const State& s){return valid(s)&&!s.neutral&&s.quarter==4&&s.phase!=7&&s.seconds>=0&&s.seconds<=window(s);}
inline float recentScore(const State& s,float scoredAt){
    float now=elapsedGameTime(s);
    return scoredAt>=0&&now>=scoredAt?std::exp(-(now-scoredAt)/(s.quarterSeconds/8.f)):0;
}
// A recovery outlook for attendance, not a published win-probability estimate.
// Unknown possession/timeouts receive the more patient interpretation.
inline float recovery(const State& s,float recent=0){
    int deficit=s.awayScore-s.homeScore;if(deficit<=0)return 1;
    int scores=(deficit+7)/8;
    int timeouts=s.homeTimeouts>=0&&s.homeTimeouts<=3?s.homeTimeouts:3;
    float opponentPossession=s.offense==1?40.f-10.f*timeouts:0;
    float field=s.offense==0&&s.toGoal>=0&&s.toGoal<=20?25.f:0;
    float needed=scores*65.f+opponentPossession-12.f*timeouts-field;
    // Score/possession costs use the 12-minute reference, like the traffic
    // window itself. Short or long quarters must not change the outlook for
    // an otherwise identical fraction of a game remaining.
    float referenceSeconds=s.seconds*720.f/s.quarterSeconds;
    float chance=context::smooth((referenceSeconds-needed+45.f)/90.f);
    float lift=.25f*(std::clamp)(recent,0.f,1.f)*(1-chance)*context::smooth(chance*2);
    return (std::clamp)(chance+lift,0.f,1.f);
}
inline float activation(const State& s){
    if(!lateHomeGame(s))return 0;
    int deficit=s.awayScore-s.homeScore;float main=s.quarterSeconds/3;
    if(!s.playoff&&deficit>=28){
        if(s.seconds>main)return .35f*context::smooth((window(s)-s.seconds)/(window(s)-main));
        return .35f+.65f*context::smooth((main-s.seconds)/(main*.10f));
    }
    return context::smooth((window(s)-s.seconds)/(window(s)*(!s.playoff&&deficit>=17?.10f:.20f)));
}
inline float blowoutUrgency(const State& s,float scoredAt=-1){
    if(!lateHomeGame(s)||s.playoff||s.awayScore-s.homeScore<17)return 0;
    return activation(s)*context::smooth((s.awayScore-s.homeScore-16)/19.f)*(1-recovery(s,recentScore(s,scoredAt)));
}
inline int extraQuarterAllowance(const State& s,float scoredAt,int maximum){
    int deficit=s.awayScore-s.homeScore;
    if(!lateHomeGame(s)||s.playoff||deficit<28)return 0;
    float severity=.6f+.4f*context::smooth((deficit-28)/14.f);
    float hopeless=1-recovery(s,recentScore(s,scoredAt));
    return static_cast<int>(std::lround((std::clamp)(maximum,0,10)*severity*hopeless*activation(s)));
}
inline float lossTarget(const State& s,float scoredAt=-1){
    int deficit=s.awayScore-s.homeScore;
    if(!lateHomeGame(s)||deficit<=8||(s.playoff&&deficit<17))return 0;
    float hopeless=1-recovery(s,recentScore(s,scoredAt));
    float target=30+45*context::smooth((deficit-8)/27.f);
    if(s.playoff)target=lossCeiling(s)*(target/75.f)*(championship(s)?.35f:.60f);
    else if(deficit>=17)target+=(lossCeiling(s)-target)*context::smooth((deficit-16)/19.f)*hopeless;
    return target*std::pow(hopeless,.8f)*activation(s);
}
}
