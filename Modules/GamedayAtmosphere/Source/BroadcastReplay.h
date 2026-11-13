#pragma once
#include "ReplayPresentation.h"
#include "ExcitementRules.h"

namespace attendance::broadcastreplay {
// Broadcast Universe 0.7-beta1's ESPN_CP6_TwoPlayerHandoff selector, observed
// read-only in its loaded CFB post-play tree. Match the complete expression;
// matching a run/pass fragment alone would also capture unrelated cutaways.
template<size_t N>bool path(const replaypresentation::View& view,uintptr_t node,
    const std::array<const char*,N>& ids,const std::array<int,N>& values){
    std::array<uintptr_t,N> c{};if(!view.children(node,0,c))return false;
    for(size_t i=0;i<N;++i)if(!view.compare(c[i],0,ids[i],4,values[i]))return false;
    return true;
}
inline bool handoff(const replaypresentation::View& view,uintptr_t node){
    std::array<uintptr_t,2> root{};std::array<uintptr_t,3> guard{};std::array<uintptr_t,6> cases{};
    return view.children(node,0,root)&&view.children(root[0],0,guard)&&
        view.compare(guard[0],0,"27",4,0)&&view.compare(guard[1],0,"29",4,0)&&view.constant(guard[2],4,1)&&
        view.children(root[1],1,cases)&&
        path<1>(view,cases[0],{"14"},{1})&&
        path<2>(view,cases[1],{"14","18"},{0,1})&&
        path<3>(view,cases[2],{"14","18","17"},{0,0,1})&&
        path<5>(view,cases[3],{"14","18","17","1","20"},{0,0,0,1,1})&&
        path<6>(view,cases[4],{"14","18","17","20","1","21"},{0,0,0,0,1,1})&&
        path<6>(view,cases[5],{"14","18","17","20","21","0"},{0,0,0,0,0,1});
}
// The native defensive inset shortcut precedes Standard Flow as well. Decline
// the WHOLE shortcut on verified big gains; never change its nested first-down,
// pass/run or milestone facts. Short opponent plays keep their native inset.
inline bool defensiveInset(const replaypresentation::View& view,uintptr_t node){
    std::array<uintptr_t,2> root{},plays{},pass{},run{};
    return view.children(node,0,root)&&view.function(root[0],"569")&&
        view.children(root[1],1,plays)&&view.children(plays[0],0,pass)&&
        view.routineGate(pass[0])&&view.function(pass[1],"1")&&
        view.children(plays[1],0,run)&&view.routineGate(run[0])&&view.function(run[1],"0");
}
// Completed, same-snap, unchanged-possession scrimmage gains only. Native
// score/turnover/special-teams/clock-management presentation remains in charge.
enum Exclusion : unsigned {Pending=1,NoSnap=2,Unsettled=4,InvalidDown=8,Possession=16,
    ClockPlay=32,Special=64,Simulated=128,Resumed=256,Turnover=512,StaleResult=1024,Score=2048,Gain=4096};
inline unsigned exclusions(const energy::Tracker& tracker,const energy::Play& result,bool completed,int minimum){
    const auto& start=tracker.start;unsigned flags=0;
    if(!completed)flags|=Pending;if(!tracker.serial)flags|=NoSnap;if(!tracker.resultUsed)flags|=Unsettled;
    if(!energy::normalDown(start)||!energy::normalDown(result))flags|=InvalidDown;
    if(start.offense!=result.offense)flags|=Possession;
    if(energy::clockManagement(start))flags|=ClockPlay;
    if(start.specialTeams||result.specialTeams)flags|=Special;
    if(start.simulated||result.simulated)flags|=Simulated;
    if(start.resumed||result.resumed)flags|=Resumed;
    if(tracker.sawPick||tracker.sawFumble||result.interception||result.fumble)flags|=Turnover;
    if(!result.resultStamp||result.resultStamp==tracker.startStamp)flags|=StaleResult;
    if(start.game.homeScore!=result.game.homeScore||start.game.awayScore!=result.game.awayScore)flags|=Score;
    float yards=energy::gainedYards(start,result);
    if(minimum<10||minimum>60||!std::isfinite(yards)||yards<minimum||yards>110)flags|=Gain;
    return flags;
}
inline bool eligible(const energy::Tracker& tracker,const energy::Play& result,bool completed,int minimum){return exclusions(tracker,result,completed,minimum)==0;}
}
