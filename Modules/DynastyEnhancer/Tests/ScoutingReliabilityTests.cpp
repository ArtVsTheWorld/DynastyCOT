#include "../Source/ScoutingEvidence.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
using namespace recruitingfog;
using namespace recruitingfog::evidence;
Json sample(){
 Json r=Json::object();auto add=[&](const char* key,const char* type,Json v){r[key]={{"type",type},{"v",v}};};
 add("user","FranchiseUser",{{"IsRetired",false},{"Team","team"}});
 add("team","Team",{{"CoachTalentEffects","effects"},{"SchoolPipelineInfluenceList","list"}});
 add("effects","CoachTalentEffects",{{"Recruiting_ScoutingBoost_Start","a"},{"Recruiting_BonusHours","b"},{"Recruiting_InfluenceBoost_Start","c"}});
 Json zero=Json::object();for(int i=0;i<8;i++)zero["Member"+std::to_string(i)]=0;
 add("a","int[]",zero);add("b","int[]",zero);add("c","int[]",zero);
 add("list","SchoolPipelineInfluence[]",{{"Member0","pipe"}});add("pipe","SchoolPipelineInfluence",{{"Pipeline","Tidewater"},{"InfluenceLevel","Popular"}});
 return {{"records",r}};
}
int main(int argc,char** argv){
 constexpr double accuracy[]={.35,.43,.52,.60,.77,.92};
 auto raw=sample();auto captured=context(raw);assert(captured.staffData==StaffData::CapturedZeroEffects&&captured.pipelinesKnown);
 assert(captured.staffData!=StaffData::VerifiedNoRelevantSkills&&pipelineFor(captured,"Tidewater")==.6);
 assert(pipelineFor(captured,"OtherVerifiedNativePipeline")==0.);
 assert(!pipelineFor(captured,"").has_value());
 raw["records"]["b"]["v"]["Member0"]=15;captured=context(raw);assert(captured.staffData==StaffData::CapturedEffects);
 raw["records"].erase("c");auto missing=context(raw);assert(missing.staffData==StaffData::Unavailable&&missing.pipelinesKnown);
 assert(reliability(pipelineFor(missing,"Tidewater")).accurate==.6);
 raw["records"].erase("list");missing=context(raw);assert(!missing.pipelinesKnown&&!pipelineFor(missing,"Tidewater"));assert(reliability(pipelineFor(missing,"Tidewater")).accurate==.6);
 assert(reliability(std::nullopt).accurate==.6&&context(Json()).staffData==StaffData::Unavailable);
 auto unsupported=sample();unsupported["records"]["pipe"]["v"]["InfluenceLevel"]="UnknownLayout";assert(!context(unsupported).pipelinesKnown);
 auto ambiguous=sample();ambiguous["records"]["secondUser"]=ambiguous["records"]["user"];assert(!context(ambiguous).pipelinesKnown);
 auto partial=sample();partial["records"]["a"]["v"].erase("Member7");assert(context(partial).staffData==StaffData::Unavailable);
 auto empty=sample();empty["records"]["list"]["v"]=Json::object();assert(context(empty).pipelinesKnown&&pipelineFor(context(empty),"Tidewater")==0.);
 NotesProspect p;p.position="QB";p.progress=75;p.observations={{"Speed",80,80,true},{"Short Accuracy",61,61,true},{"Awareness",72,75,false}};
 for(int tier=0;tier<6;tier++){
  auto chance=reliability(tier/5.);assert(std::abs(chance.accurate-accuracy[tier])<1e-12&&std::abs(chance.accurate+chance.exaggerated+chance.misleading-1)<1e-12);
  unsigned accurate=0,total=0;for(unsigned i=0;i<20000;i++){p.identity="42|calibration|"+std::to_string(i);auto report=obtain(Json(),p,captured,tier/5.);for(const auto& claim:report["claims"]){++total;if(claim["quality"]==0)++accurate;assert(std::abs(claim["truthBand"].get<int>()-claim["claimBand"].get<int>())<=1);if(claim["truthBand"]==2)assert(claim["claimBand"]!=0);}}
  const auto rate=double(accurate)/total;assert(std::abs(rate-accuracy[tier])<.02);std::cout<<"tier "<<tier<<" expected="<<accuracy[tier]<<" observed="<<rate<<" exaggerated="<<chance.exaggerated<<" misleading="<<chance.misleading<<'\n';
 }
 p.identity="42|legacy";auto old=obtain(Json(),p,captured,0.);for(auto& claim:old["claims"])claim.erase("calibration");const auto prior=old["claims"];auto improved=obtain(old,p,Context{},1.);assert(improved["claims"]==prior);p.observations.push_back({"Throw Power",83,83,true});auto expanded=obtain(old,p,Context{},std::nullopt);for(auto i=prior.begin();i!=prior.end();++i)assert(expanded["claims"].at(i.key())==i.value());assert(expanded["claims"]["throw power"]["calibration"]=="pipeline-v2"&&!expanded["claims"]["throw power"]["pipelineKnown"].get<bool>());
 if(argc==2){std::ifstream input(argv[1]);Json actual;input>>actual;auto& records=actual.at("records");std::string user;for(auto i=records.begin();i!=records.end();++i)if(i.value().at("type")=="FranchiseUser"){user=i.key();break;}assert(!user.empty());std::set<int> tiers;unsigned teams=0,relationships=0;for(auto i=records.begin();i!=records.end();++i)if(i.value().at("type")=="Team"){
  const auto teamKey=i.key();records[user]["v"]["Team"]=teamKey;records[user]["v"]["IsRetired"]=false;auto c=context(actual);if(!c.pipelinesKnown)continue;++teams;for(const auto& pair:c.pipelines){const auto tier=int(std::lround(pair.second*5));tiers.insert(tier);++relationships;assert(std::abs(reliability(pipelineFor(c,pair.first)).accurate-accuracy[tier])<1e-12);}
 }assert(tiers.size()==6&&teams>100);std::cout<<"PASS actual read-only Dynasty fixture: "<<teams<<" teams, "<<relationships<<" native relationships, all six tiers; staff effects recorded without inventing purchase/stacking/max-skill proof\n";}
 std::cout<<"PASS pipeline-v2 calibration, unavailable/zero/valid coaching distinction, missing/partial/ambiguous pipelines, neutral fallback, plausible errors and immutable previous claims\n";
}
