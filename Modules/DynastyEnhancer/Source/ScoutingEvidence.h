#pragma once
#include "RecruitNotesProse.h"
#include "RecruitNotes.h"
#include "../Vendor/json.hpp"
#include "ScoutingTechniqueLibrary.h"
#include <optional>
#include "NotesPresentation.h"
namespace recruitingfog::evidence {
using Json=nlohmann::json;
struct Reliability {double accurate=0,exaggerated=0,misleading=0;};
// Native skill names and role values have been audited, but purchase-to-
// aggregate stacking/caps are not yet verified. Never normalize mixed units
// into an invented staff score. This candidate deliberately uses pipeline only.
inline Reliability reliability(std::optional<double> pipeline){
 constexpr double accurateByTier[]={.35,.43,.52,.60,.77,.92};
 double accurate=.60; // Explicit neutral fallback, NOT an assumed weak staff.
 if(pipeline&&std::isfinite(*pipeline)){const auto tier=(std::clamp)(*pipeline,0.,1.)*5.;const auto lower=static_cast<unsigned>(tier);accurate=lower>=5?accurateByTier[5]:accurateByTier[lower]+(accurateByTier[lower+1]-accurateByTier[lower])*(tier-lower);}
 return {accurate,.8*(1-accurate),.2*(1-accurate)};
}
enum class StaffData {Unavailable,NotCapturedForPipelineModel,CapturedZeroEffects,CapturedEffects,VerifiedNoRelevantSkills};
inline const char* staffDataName(StaffData value){switch(value){case StaffData::NotCapturedForPipelineModel:return "not-captured-pipeline-only";case StaffData::CapturedZeroEffects:return "captured-zero-effects";case StaffData::CapturedEffects:return "captured-effects";case StaffData::VerifiedNoRelevantSkills:return "verified-no-relevant-skills";default:return "unavailable";}}
struct Context {StaffData staffData=StaffData::Unavailable;bool pipelinesKnown=false;std::map<std::string,std::vector<int>> effects;std::map<std::string,double> pipelines;};
inline std::optional<double> pipelineFor(const Context& context,const std::string& recruitPipeline){
 if(!context.pipelinesKnown||recruitPipeline.empty())return std::nullopt;
 const auto found=context.pipelines.find(recruitPipeline);
 // A complete native relationship list and a verified recruit pipeline name
 // establish an absent relationship. A missing LIST or recruit name does not.
 return found==context.pipelines.end()?0.:found->second;
}
inline Context context(const Json& raw){
 Context out;if(!raw.is_object()||!raw.contains("records")||!raw.at("records").is_object())return out;const auto& records=raw.at("records");
 auto resolve=[&](const Json& ref)->const Json*{if(!ref.is_string())return nullptr;auto i=records.find(ref.get<std::string>());return i==records.end()||!i->contains("v")||!i->at("v").is_object()?nullptr:&i->at("v");};
 const Json* team=nullptr;unsigned users=0;
 for(const auto& row:records)if(row.is_object()&&row.value("type",std::string())=="FranchiseUser"&&row.contains("v")&&row.at("v").is_object()){const auto& v=row.at("v");if(!v.contains("IsRetired")||!v.at("IsRetired").is_boolean())return out;if(!v.at("IsRetired").get<bool>()){auto t=resolve(v.value("Team",Json()));if(!t)return out;team=t;++users;}}
 if(users!=1||!team)return out;
 const auto* effects=resolve(team->value("CoachTalentEffects",Json()));
 if(effects){bool valid=true,nonzero=false;for(auto key:{"Recruiting_ScoutingBoost_Start","Recruiting_BonusHours","Recruiting_InfluenceBoost_Start"}){
  const auto* values=resolve(effects->value(key,Json()));if(!values||!values->is_object()||values->size()!=8){valid=false;continue;}
  std::vector<int> vector;for(unsigned i=0;i<8;++i){const auto item=values->find("Member"+std::to_string(i));if(item==values->end()||!item->is_number_integer()||*item<0||*item>65535){valid=false;break;}int n=item->get<int>();vector.push_back(n);nonzero=nonzero||n!=0;}if(vector.size()==8)out.effects[key]=std::move(vector);
 }if(valid)out.staffData=nonzero?StaffData::CapturedEffects:StaffData::CapturedZeroEffects;}
 // Zero aggregate values are not proof that no relevant skills were bought.
 // VerifiedNoRelevantSkills is reserved for a verified talent-ownership proof.
 const auto* list=resolve(team->value("SchoolPipelineInfluenceList",Json()));
 if(list&&list->is_object()){out.pipelinesKnown=true;for(const auto& ref:*list){auto v=resolve(ref);if(!v||!v->contains("Pipeline")||!v->at("Pipeline").is_string()||!v->contains("InfluenceLevel")||!v->at("InfluenceLevel").is_string()){out.pipelinesKnown=false;break;}
  const auto name=v->at("Pipeline").get<std::string>(),level=v->at("InfluenceLevel").get<std::string>();constexpr const char* levels[]={"Unrecognized","NicheInterest","Respected","Popular","HouseholdName","CulturalPillar"};bool known=false;
  for(int i=0;i<6;++i)if(level==levels[i]&&!name.empty()){if(out.pipelines.count(name)&&out.pipelines.at(name)!=i/5.){out.pipelinesKnown=false;break;}out.pipelines[name]=i/5.;known=true;}if(!known){out.pipelinesKnown=false;break;}
 }if(!out.pipelinesKnown)out.pipelines.clear();}
 return out;
}
inline int band(const NotesObservation& o){return o.low>=80?0:o.low>=70&&o.high<80?1:o.high<70?2:-1;}
inline std::string family(const NotesProspect& p,const NotesObservation& o){const auto k=notesKey(o.name);if(k=="tackling")return "tackle";if(k!="awareness")return k;
 const auto pos=notesKey(p.position);if(pos=="qb")return "awareness qb";if(pos=="wr"||pos=="te")return "awareness receiver";if(pos=="hb"||pos=="rb"||pos=="fb")return "awareness back";if(pos=="lt"||pos=="lg"||pos=="c"||pos=="rg"||pos=="rt")return "awareness line";if(pos=="k"||pos=="p")return "awareness specialist";return "awareness defense";
}
inline bool universal(const std::string& f){return f.rfind("awareness",0)==0||f=="speed"||f=="acceleration"||f=="strength"||f=="agility"||f=="change of direction"||f=="jumping";}
inline bool relevant(const std::string& position,const std::string& f){
 if(universal(f))return true;const auto pos="|"+position+"|";std::string allowed;
 if(position=="QB")allowed="|throw power|short accuracy|medium accuracy|deep accuracy|throw on run|under pressure|break sack|carrying|";
 else if(std::string("|HB|RB|FB|").find(pos)!=std::string::npos)allowed="|carrying|break tackle|trucking|elusiveness|catching|pass block|run block|";
 else if(position=="WR"||position=="TE")allowed="|catching|catch in traffic|spectacular catch|short route running|medium route running|deep route running|run block|carrying|";
 else if(std::string("|LT|LG|C|RG|RT|").find(pos)!=std::string::npos)allowed="|run block|run block power|run block finesse|pass block|pass block power|pass block finesse|impact blocking|";
 else if(position=="K"||position=="P"||position=="LS")allowed="|kick power|kick accuracy|";
 else if(position=="CB"||position=="FS"||position=="SS")allowed="|man coverage|zone coverage|press|tackle|hit power|pursuit|play recognition|catching|";
 else allowed="|tackle|hit power|block shedding|power moves|finesse moves|pursuit|play recognition|";
 return allowed.find("|"+f+"|")!=std::string::npos;
}
inline const char* flavor(const std::string& position,uint64_t seed){
 constexpr const char* common[]={
 "Background note: he keeps a notebook of corrections rather than just watching the highlights.",
 "Background note: his coach describes a quiet player who asks detailed questions after practice.",
 "Background note: the staff's early impression is of a talkative competitor who likes to set the tone.",
 "Development note: he reportedly learned a second position before settling into this role.",
 "Development note: the camp report emphasizes repeatable technique over his best single rep.",
 "Availability note: the scouting file does not establish a medical history. Durability remains unverified.",
 "Camp note: the early account favors his effort; the staff still wants a broader sample.",
 "Background note: he reportedly balances football with another school activity. The details are unconfirmed.",
 "Coach's account: he takes correction well, though that impression still needs checking.",
 "Development note: the report describes a late start in organized football, rather than years in one system.",
 "Camp note: one strong session should not outweigh the rest of the evaluation.",
 "Background note: teammates reportedly lean on his sense of humor when practice gets repetitive."};
 const auto pos=notesKey(position);
 if(seed%5==0){if(pos=="lt"||pos=="lg"||pos=="c"||pos=="rg"||pos=="rt")return "Background lead: he reportedly also wrestles. Treat the claimed leverage benefit as something to verify on football film.";
 if(pos=="wr"||pos=="cb"||pos=="hb"||pos=="rb")return "Background lead: he reportedly competes in track. No verified meet result is included in this scouting file.";
 if(pos=="qb")return "Background lead: his coach mentions another throwing sport. That does not establish football placement under pressure.";
 if(pos=="k"||pos=="p")return "Background lead: the report mentions soccer experience. Repeatable football contact still needs its own evaluation.";
 return "Background lead: he reportedly also plays basketball. The staff wants to see how the footwork carries over to football.";}
 return common[seed%(sizeof(common)/sizeof(*common))];
}
// Append immutable claims as evidence is obtained. Existing keys/wording survive
// better scouting, staff changes and reloads; no native records are changed.
inline Json obtain(Json saved,const NotesProspect& p,const Context& context,std::optional<double> pipeline){
 if(!saved.is_object())saved={{"version",1},{"claims",Json::object()},{"flavor",p.activeTransfer?"Film review continues.":flavor(p.position,notesHash(p.identity+"|background"))}};
 if(!saved.contains("displayOrder"))saved["displayOrder"]=Json::array();
 if(p.progress<25)return saved;
 const auto chances=reliability(pipeline);
 for(const auto& o:p.observations){const auto b=band(o);if(b<0||o.low<0||o.high>100||o.low>o.high)continue;const auto f=family(p,o);if(!relevant(p.position,f)||saved["claims"].contains(f))continue;
  for(const auto& entry:resume::prose)if(f==entry.attribute){
   const auto h=notesHash(p.identity+"|"+f+"|report-v1");const auto roll=double(h%10000)/10000.;
   const int quality=roll<chances.accurate?0:roll<chances.accurate+chances.exaggerated?1:2;
   const int claim=quality==0?b:quality==1?(std::max)(0,b-1):b==0?1:b==1?2:1;
   constexpr const char* sources[]={"Film assessment: ","Coach's early assessment: ","Unverified camp impression: ","Scouting observation: "};
   std::string text=sources[(h>>18)%4];
   const auto specific=technique(p.position,f,claim,h);text+=specific?specific:entry.bands[claim][(h>>24)%3];
   saved["claims"][f]={{"text",text},{"truthBand",b},{"claimBand",claim},{"quality",quality},{"priority",universal(f)?40:100},{"order",h},{"calibration","pipeline-v2"},{"staffData",staffDataName(context.staffData)},{"pipelineKnown",bool(pipeline)&&std::isfinite(*pipeline)},{"pipeline",pipeline&&std::isfinite(*pipeline)?Json(*pipeline):Json()},{"progress",p.progress}};break;
  }
 }
 if(!p.activeTransfer&&p.progress>=50&&!saved.contains("athleticBackgroundChecked")){
  saved["athleticBackgroundChecked"]=true;const auto h=notesHash(p.identity+"|athletic-background");
  if(h%11==0)for(const auto& o:p.observations)if(o.low>=85&&o.high<=100){
   const auto key=notesKey(o.name);
   if(key=="speed"||key=="acceleration"){saved["athleticBackground"]="Background lead: the report mentions qualifying for a regional track meet. No official result is attached.";break;}
   if(key=="strength"){saved["athleticBackground"]="Background lead: his coach mentions reaching a school weight-room milestone. The reported lift has not been independently checked.";break;}
  }
 }
 std::vector<std::string> order;for(auto i=saved.at("claims").begin();i!=saved.at("claims").end();++i)order.push_back(i.key());
 std::stable_sort(order.begin(),order.end(),[&](const std::string& a,const std::string& b){const auto& x=saved.at("claims").at(a);const auto& y=saved.at("claims").at(b);const auto xp=x.value("priority",40),yp=y.value("priority",40);return xp!=yp?xp>yp:x.value("order",uint64_t(0))<y.value("order",uint64_t(0));});saved["displayOrder"]=order;
 return saved;
}
inline std::string render(const Json& saved,double progress,bool blind,bool transfer=false){
 if(blind)return "SCOUTING NOTES\nPlayer evaluation is hidden by Recruiting Blind.";
 std::string out="SCOUTING NOTES";
 if(!saved.is_object()||progress<25)return out+"\nScout further for individual observations.";
 unsigned count=0,limit=progress>=75?4:progress>=50?3:2;
 if(saved.contains("displayOrder")){for(const auto& key:saved.at("displayOrder"))if(count++<limit)out+="\n"+presentNote(saved.at("claims").at(key.get<std::string>()).at("text").get<std::string>());}
 else for(const auto& claim:saved.at("claims"))if(count++<limit)out+="\n"+presentNote(claim.at("text").get<std::string>());
 if(!count)out+="\nNeed a closer look before judging individual traits.";
 if(!transfer&&progress>=50&&saved.contains("athleticBackground"))out+="\n"+presentNote(saved.at("athleticBackground").get<std::string>());
 if(!transfer&&progress>=50&&notesHash(saved.at("flavor").get<std::string>())%3==0)out+="\n"+presentNote(saved.at("flavor").get<std::string>());
 return out;
}
}
