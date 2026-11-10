#pragma once
#include "RecruitInterestPolicy.h"
#include "../Vendor/json.hpp"
#include "RecruitingUiResponse.h"
#include <map>
#include <functional>
#include <optional>
#include <set>
namespace recruitingfog::interest {
struct Knowledge {uint32_t league=0;int season=0,userTeam=-1;unsigned maxScoutingUnits=0;struct Seen{std::string first,last;std::map<int,unsigned> times;bool committed=false;std::set<int> boards;};std::map<uint32_t,Seen> recruits;};
inline std::optional<unsigned> scoutProgress(const Knowledge& data,uint32_t league,uint32_t recruit,int team){
 if(data.league!=league||team<0||!data.maxScoutingUnits||data.maxScoutingUnits>100)return {};
 const auto found=data.recruits.find(recruit);if(found==data.recruits.end()||found->second.committed||found->second.boards.count(team))return {};
 const auto units=found->second.times.find(team);if(units==found->second.times.end()||!units->second||units->second>data.maxScoutingUnits)return {};
 return unsigned(100*units->second/data.maxScoutingUnits);
}
inline const char* scoutingAsset(unsigned percent){constexpr const char* assets[]={"dynas_general_ScoutingEmpty","dynas_general_Scouting_10","dynas_general_Scouting_20","dynas_general_Scouting_30","dynas_general_Scouting_40","dynas_general_Scouting_50","dynas_general_Scouting_60","dynas_general_Scouting_70","dynas_general_Scouting_80","dynas_general_Scouting_90","dynas_general_Scouting"};return assets[(std::min)(100u,percent)/10];}
inline Knowledge knowledge(const nlohmann::json& raw){
 Knowledge out;out.league=static_cast<uint32_t>(raw.at("status").at("league").get<int64_t>());out.season=raw.at("status").at("season").at("CurrentSeasonYear");const auto& records=raw.at("records");
 auto resolve=[&](const nlohmann::json& ref)->const nlohmann::json*{if(!ref.is_string())return nullptr;auto it=records.find(ref.get<std::string>());return it==records.end()?nullptr:&it->at("v");};
 unsigned users=0;for(const auto& r:records)if(r.at("type")=="FranchiseUser"&&!r.at("v").value("IsRetired",false)){auto t=resolve(r.at("v").at("Team"));if(t){++users;out.userTeam=t->at("TeamIndex");}}if(users!=1)out.userTeam=-1;
 for(const auto& r:records)if(r.at("type")=="RecruitingTunables"){const auto maximum=r.at("v").value("MaxTimesScouted",0);if(maximum>0&&maximum<=100)out.maxScoutingUnits=unsigned(maximum);}
 for(const auto& r:records){if(r.at("type")!="ProspectInteraction")continue;const auto& v=r.at("v");const int times=v.value("TimesScouted",0);if(times<=0||times>100)continue;const auto team=resolve(v.at("Team")),recruit=resolve(v.at("Recruit"));if(!team||!recruit)continue;const auto player=resolve(recruit->at("Player"));if(!player)continue;
  const auto key=v.at("Recruit").get<std::string>();auto split=key.find(':');if(split==std::string::npos)continue;auto table=std::stoul(key.substr(0,split)),row=std::stoul(key.substr(split+1));if(table>=16384||row>=131072)continue;
  auto& known=out.recruits[uint32_t(table*131072+row)];known.first=player->at("FirstName");known.last=player->at("LastName");known.times[team->at("TeamIndex").get<int>()]=unsigned(times);
  const auto stage=recruit->value("RecruitStage","");known.committed=stage=="SoftCommitted"||stage=="HardCommitted"||stage=="Signed";
 }
 for(const auto& r:records)if(r.at("type")=="Team"){
  const auto& team=r.at("v");if(!team.contains("RecruitingBoard"))continue;const auto board=resolve(team.at("RecruitingBoard"));if(!board||!board->contains("Recruits"))continue;const auto targets=resolve(board->at("Recruits"));if(!targets)continue;
  for(const auto& ref:*targets){const auto target=resolve(ref);if(!target||!target->contains("Recruit"))continue;const auto key=target->at("Recruit").get<std::string>();const auto colon=key.find(':');if(colon==std::string::npos)continue;const auto table=std::stoul(key.substr(0,colon)),row=std::stoul(key.substr(colon+1));if(table>=16384||row>=131072)continue;auto found=out.recruits.find(uint32_t(table*131072+row));if(found!=out.recruits.end())found->second.boards.insert(team.at("TeamIndex").get<int>());
  }
 }
 return out;
}
struct TransformStats{unsigned recruits=0,schools=0,scouted=0,unsupported=0;};
// Only transient UI values. No Player/Recruit/ProspectInteraction DB writes.
// Numeric labels consumed by the native slider are quantized, then the native
// widget adapter removes that slider entirely. RankOnly leaves them untouched.
template<class View,class StringSetter,class BoolSetter,class NumberSetter,class Reorder>
TransformStats transform(View& view,uintptr_t root,Mode mode,const Knowledge* known,uint32_t league,StringSetter text,BoolSetter boolean,NumberSetter number,Reorder reorder,double presentationMax=100){
 TransformStats stats;std::vector<uintptr_t> recruits;if(!view.array(root,recruits,512)){++stats.unsupported;return stats;}
 for(auto recruit:recruits){
  std::map<std::string,uintptr_t> f;if(!view.members(recruit,f)){++stats.unsupported;continue;}
  auto string=[&](const char* name){std::string out;if(f.count(name))view.string(f.at(name),out);return out;};
  double rawId=0,progress=0;bool valid=f.count("RecordId")&&view.number(f.at("RecordId"),rawId)&&rawId>=0&&rawId<=UINT32_MAX&&rawId==std::floor(rawId);
  const auto first=string("FirstName"),last=string("LastName");if(f.count("PercentageScouted"))view.number(f.at("PercentageScouted"),progress);
  std::vector<uintptr_t> schoolValues;const bool schoolList=f.count("TopSchools")&&view.array(f.at("TopSchools"),schoolValues,32);
  std::vector<School> schools;std::map<int,uintptr_t> byId;int ownerTeam=-1;bool duplicate=false;
  struct Original {unsigned order;std::map<std::string,double> values;};std::map<int,Original> originals;
  for(auto school:schoolValues){std::map<std::string,uintptr_t> s;double team=0,total=0;std::string name;bool own=false;
   if(!view.members(school,s)||!s.count("TeamId")||!s.count("TeamName")||!s.count("InfluenceTotal")||!view.number(s.at("TeamId"),team)||team<0||team>254||team!=std::floor(team)||!view.number(s.at("InfluenceTotal"),total)||!view.string(s.at("TeamName"),name)){duplicate=true;break;}
   if(s.count("IsCurrentUserTeam")&&view.boolean(s.at("IsCurrentUserTeam"),own)&&own){if(ownerTeam!=-1)duplicate=true;ownerTeam=int(team);}
   Original original{unsigned(schools.size()),{}};
   for(const auto* key:{"InfluenceTotal","InfluenceNextTotal","InfluenceDelta","InfluenceMinDisplayValue","InfluenceMaxDisplayValue"}){double value=0;if(s.count(key)&&view.number(s.at(key),value))original.values[key]=value;}
   // Keep a bounded copy in the transient response itself, never in the DB.
   // Repeated rendering can reuse a response; classify from its original facts.
   // If native code has replaced any projected field, treat it as fresh data.
   std::string backup;if(s.count("BTFInterestOriginal")&&view.string(s.at("BTFInterestOriginal"),backup)&&backup.size()<1024)try{
    const auto saved=nlohmann::json::parse(backup);const auto projected=saved.at("projected").get<std::map<std::string,double>>();
    if(saved.at("version")==1&&projected==original.values){auto restored=saved.at("original").get<std::map<std::string,double>>();const auto ordinal=saved.at("order").get<unsigned>();
     bool safe=ordinal<32&&restored.size()==original.values.size();for(const auto& [key,value]:restored)if(!original.values.count(key)||!std::isfinite(value))safe=false;
     if(safe){original.order=ordinal;original.values=std::move(restored);total=original.values.at("InfluenceTotal");}
    }
   }catch(const std::exception&){}
   if(!byId.emplace(int(team),school).second)duplicate=true;originals[int(team)]=original;schools.push_back({int(team),name,total,Standing::Unknown,original.order});
  }
  bool previously=std::isfinite(progress)&&progress>0&&progress<=100;
  if(valid&&known&&known->league==league){const auto found=known->recruits.find(uint32_t(rawId));if(ownerTeam<0)ownerTeam=known->userTeam;
   if(found!=known->recruits.end()&&found->second.first==first&&found->second.last==last&&ownerTeam>=0&&found->second.times.count(ownerTeam))previously=true;
  }
  boolean(recruit,"BTFPreviouslyScouted",previously);text(recruit,"BTFScoutLabel",previously?(progress>0&&progress<=100?("SCOUTED "+std::to_string(int(progress))+"%").c_str():"PREVIOUSLY SCOUTED"):"");if(previously)++stats.scouted;
  text(recruit,"BTFInterestMode",mode==Mode::Feedback?"Feedback":mode==Mode::Blind?"Blind":mode==Mode::RankOnly?"RankOnly":"Native");
  if(!schoolList||duplicate){++stats.unsupported;continue;}++stats.recruits;
  if(mode!=Mode::Feedback&&mode!=Mode::Blind){std::stable_sort(schools.begin(),schools.end(),[](const School& a,const School& b){return a.nativeOrder<b.nativeOrder;});std::vector<uintptr_t> restored;
   for(const auto& school:schools){auto at=byId.at(school.team);std::map<std::string,uintptr_t> s;view.members(at,s);for(const auto& [key,value]:originals.at(school.team).values)number(s.at(key),value);if(s.count("BTFStanding"))text(at,"BTFStanding","");if(s.count("BTFInterestOriginal"))text(at,"BTFInterestOriginal","");restored.push_back(at);}
   reorder(f.at("TopSchools"),restored);text(recruit,"BTFStandingSort","5");continue;
  }
  uint64_t identity=valid?uint64_t(rawId):0;for(unsigned char c:first+"|"+last)identity=(identity^c)*1099511628211ull;
  classify(schools,identity,league);if(mode==Mode::Blind)for(auto& s:schools)s.standing=Standing::Unknown;group(schools);
  unsigned userStanding=5;for(const auto& school:schools)if(school.team==ownerTeam)userStanding=unsigned(school.standing);text(recruit,"BTFStandingSort",std::to_string(userStanding).c_str());
  std::vector<uintptr_t> sorted;for(const auto& school:schools){auto at=byId.at(school.team);std::map<std::string,uintptr_t> s;view.members(at,s);text(at,"BTFStanding",label(school.standing));
   std::map<std::string,double> projected;for(const auto& [key,value]:originals.at(school.team).values){(void)value;const double shown=key=="InfluenceMaxDisplayValue"?presentationMax:mode==Mode::Blind?0:(key=="InfluenceTotal"||key=="InfluenceNextTotal")?band(school.standing):0;projected[key]=shown;number(s.at(key),shown);}
   const auto& original=originals.at(school.team);text(at,"BTFInterestOriginal",nlohmann::json({{"version",1},{"order",original.order},{"original",original.values},{"projected",projected}}).dump().c_str());
   sorted.push_back(at);++stats.schools;
  }
  reorder(f.at("TopSchools"),sorted);
 }
 return stats;
}
}
