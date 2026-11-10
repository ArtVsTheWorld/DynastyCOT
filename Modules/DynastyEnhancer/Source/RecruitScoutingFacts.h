#pragma once
#include "RecruitInterestResponse.h"
#include <memory>
#include <atomic>
namespace recruitingfog::interest {
struct UiScoutingFacts {
 uint32_t league=0;int season=0,team=-1;
 struct Recruit {std::string first,last;unsigned percent=0;bool operator==(const Recruit& other)const{return first==other.first&&last==other.last&&percent==other.percent;}};
 std::map<uint32_t,Recruit> records;std::set<uint32_t> board;
 bool matches(const Knowledge& data)const{return league==data.league&&season==data.season&&team==data.userTeam;}
};
// Only the owned native GetTeamRecruitList response may populate this cache.
// Unlike the background history capture, this response already contains the
// just-completed scouting edit. Keep it on the same synchronous UI path.
template<class View>std::shared_ptr<const UiScoutingFacts> observeScouting(View& view,uintptr_t root,const Knowledge& data,const std::shared_ptr<const UiScoutingFacts>& previous){
 if(!data.league||data.userTeam<0)return {};std::vector<uintptr_t> rows;if(!view.array(root,rows,512))return {};
 auto next=std::make_shared<UiScoutingFacts>();if(previous&&previous->matches(data))*next=*previous;
 next->league=data.league;next->season=data.season;next->team=data.userTeam;next->board.clear();
 for(auto row:rows){std::map<std::string,uintptr_t> f;double id=0,percent=0;std::string first,last;
  if(!view.members(row,f)||!f.count("RecordId")||!f.count("FirstName")||!f.count("LastName")||!f.count("PercentageScouted")||!view.number(f.at("RecordId"),id)||id<=0||id>UINT32_MAX||id!=std::floor(id)||!view.string(f.at("FirstName"),first)||!view.string(f.at("LastName"),last)||first.empty()||last.empty()||!view.number(f.at("PercentageScouted"),percent)||percent<0||percent>100||percent!=std::floor(percent)||!next->board.insert(uint32_t(id)).second)return {};
  next->records[uint32_t(id)]={first,last,unsigned(percent)};
 }
 // A malformed or unexpectedly large history never evicts arbitrary entries
 // or silently changes the displayed record. A later native capture can retry.
 if(next->records.size()>4096)return {};
 if(previous&&previous->matches(data)&&previous->board==next->board&&previous->records==next->records)return previous;
 return next;
}
inline std::optional<unsigned> currentScouting(const Knowledge& data,const std::shared_ptr<const UiScoutingFacts>& fresh,uint32_t league,uint32_t record,int team,const std::string& nativeTarget){
 if(data.league!=league||team<0)return {};
 if(nativeTarget!="TargetNone"&&nativeTarget!="TargetLockedOut"&&nativeTarget!="TargetDealbreaker"&&nativeTarget!="TargetInterested")return {};
 const bool nativeOffBoard=nativeTarget=="TargetNone";
 if(fresh&&fresh->matches(data)&&fresh->league==league&&fresh->team==team){
  const auto found=fresh->records.find(record);
  // The current native TargetNone cell is stronger than the earlier board
  // response: it is produced after removal, before that response may refresh.
  if(found!=fresh->records.end()){
   if(!nativeOffBoard&&fresh->board.count(record))return {};
   return found->second.percent?std::optional<unsigned>(found->second.percent):std::nullopt;
  }
 }
 if(nativeOffBoard){auto known=data.recruits.find(record);if(known==data.recruits.end()||known->second.committed||!data.maxScoutingUnits||data.maxScoutingUnits>100)return {};const auto found=known->second.times.find(team);if(found==known->second.times.end()||!found->second||found->second>data.maxScoutingUnits)return {};return 100*found->second/data.maxScoutingUnits;}
 return scoutProgress(data,league,record,team);
}
}
