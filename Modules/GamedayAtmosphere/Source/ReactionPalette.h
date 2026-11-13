#pragma once
#include "ReactionCatalog.h"
#include "CoreAtmosphere.h"
#include "LiveReactions.h"
#include "SituationalReactions.h"
namespace attendance::palette {
inline int tier(const energy::Play&,const State&,bool major=false){
 // Play importance selects the variant; crowd size remains a separate ceiling.
 return major?3:2; // Population is the sole sparse-crowd downsize.
}
inline int generic(uint64_t key,int level=3){return catalog::choose(std::array<int,2>{22,26},key,level);}
inline int liveType(const live::Outcome& o,const energy::Play& snap,const State& s,uint64_t sequence,unsigned epoch,bool negative=false){
 using E=live::Event;uint64_t key=catalog::hash(sequence,static_cast<uint64_t>(o.event)+1,epoch);int level=tier(snap,s,o.yards>=40);
 switch(o.event){
 case E::Truck:case E::HitStick:case E::HitStickStop:return catalog::type(negative?23:2,3);
 case E::StiffArm:case E::Hurdle:case E::Juke:case E::Spin:return catalog::type(negative?1:0,3);
 case E::BrokenTackle:return catalog::type(negative?1:0,level);
 case E::LongCatch:case E::CompletedPass:return catalog::type(negative?5:4,tier(snap,s,o.yards>=20));
 case E::LongRun:case E::RunAfterCatch:case E::KickReturn:case E::PuntReturn:return catalog::type(negative?21:20,tier(snap,s,o.yards>=15));
 case E::Interception:return negative?catalog::type(13):catalog::choose(std::array<int,2>{12,8},key);
 case E::FumbleRecovery:case E::ForcedFumble:return catalog::type(negative?9:8);
 case E::Sack:return catalog::type(negative?15:14);
 case E::PassBreakup:case E::CatchKnockout:return negative?catalog::type(11,level):catalog::choose(std::array<int,2>{10,24},key,level);
 case E::OvertimeFieldGoal:return catalog::type(negative?7:6);
 case E::OvertimeMissedFieldGoal:return catalog::type(negative?7:6);
 case E::TackleForLoss:return negative?catalog::type(1,level):generic(key,level);
 case E::GoalLineStop:case E::ThirdGoalStop:case E::CriticalStop:case E::DownsStop:case E::OvertimeStop:case E::OvertimeConversionStop:return negative?catalog::type(11,3):catalog::type(10,3);
 default:return negative?catalog::type(23,level):generic(key,level);
 }
}
inline bool positiveOutcome(const energy::Outcome& r,const energy::Play& snap){
 return r.safety||r.homePoints>=6||r.awayPoints>=6||((r.homePoints==3||r.awayPoints==3)&&r.event==energy::Event::MadeFieldGoal)||
  r.homePoints==2||r.awayPoints==2||core::positiveResult(r.event)||(r.firstDown&&(r.yards>=9.99f||snap.down>=3));
}
inline int resultType(const energy::Outcome& r,const energy::Play& snap,const State& s,bool pass,uint64_t sequence,unsigned epoch,bool negative=false){
 uint64_t key=catalog::hash(sequence,static_cast<uint64_t>(r.event)+31,epoch);int points=r.side==1?r.awayPoints:r.homePoints;
 if(points>=6){if(r.takeaway)return catalog::type(negative?9:8);if(pass)return catalog::type(negative?17:16);if(energy::normalDown(snap)&&!snap.specialTeams)return catalog::type(negative?19:18);return negative?catalog::type(23):generic(key);}
 if(points==3)return catalog::type(negative?7:6);
 if(r.takeaway){if(r.event==energy::Event::FumbleRecovery)return catalog::type(negative?9:8);return negative?catalog::type(13):catalog::choose(std::array<int,2>{12,8},key);}
 if(r.forcedFumble)return catalog::type(negative?9:8);
 if(r.sack)return catalog::type(negative?15:14);
 if(r.event==energy::Event::ThirdStop||r.event==energy::Event::FourthStop||r.event==energy::Event::ConversionStop||r.turnoverOnDowns||r.goalLineStop)return catalog::type(negative?11:10,tier(snap,s,r.turnoverOnDowns||r.goalLineStop||r.event==energy::Event::FourthStop||r.event==energy::Event::ConversionStop));
 if(r.firstDown||r.event==energy::Event::ExplosiveGain){int level=tier(snap,s,r.yards>=20);return negative?catalog::type(21,level):pass?catalog::type(4,level):catalog::type(20,level);}
 if(r.fiveYardLoss)return negative?catalog::type(1,tier(snap,s)):generic(key,tier(snap,s));
 return negative?catalog::type(23,tier(snap,s)):generic(key,tier(snap,s));
}
inline int situationType(const situational::Outcome& o,const energy::Play& snap,const State& s,uint64_t sequence,unsigned epoch,const energy::Outcome* result=nullptr,bool pass=false){
 using E=situational::Event;auto key=catalog::hash(sequence,static_cast<uint64_t>(o.event)+100,epoch);int level=tier(snap,s);
 switch(o.event){
 case E::DeepIncomplete:return catalog::choose(std::array<int,2>{5,11},key,level);
 case E::DeepDefensiveIncomplete:return catalog::choose(std::array<int,2>{10,24},key,level);
 case E::DeepVisitorCatch:return catalog::type(5,level);
 case E::HomeInterception:return catalog::type(13);
 case E::HomeTurnover:return catalog::type(9);
 case E::MissedHomeFieldGoal:return catalog::type(7);
 case E::MissedVisitorFieldGoal:return catalog::type(6);
 case E::FailedConversion:return catalog::type(11,3);
 case E::AwayShowboat:return catalog::type(25,3);
 case E::OvertimeVisitorScore:return result?resultType(*result,snap,s,pass,sequence,epoch,true):catalog::type(23);
 case E::OvertimeDefensiveConversionStop:return catalog::type(10);
 default:return o.type;
 }
}
inline int lineup(uint64_t play,uint64_t id,unsigned epoch){return catalog::choose(std::array<int,3>{22,26,27},catalog::hash(play,id,epoch));}
}
