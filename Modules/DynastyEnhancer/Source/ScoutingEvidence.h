#pragma once
#include "RecruitNotesProse.h"
#include "RecruitNotes.h"
#include "../Vendor/json.hpp"
#include "ScoutingTechniqueLibrary.h"
#include <optional>
#include "NotesPresentation.h"
#include "ScoutingNoteSelection.h"
#include "RecruitBackgroundNotes.h"
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
inline int band(const NotesObservation& o){
 if(!std::isfinite(o.low)||!std::isfinite(o.high)||o.low<0||o.high>100||o.low>o.high)return -1;
 return o.low>=80?0:o.low>=70&&o.high<80?1:o.high<70?2:-1;
}
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

inline int currentReportVersion(NoteRole role){return role==NR::Unknown?1:3;}
inline std::string observationKey(const NotesProspect& p,const NotesObservation& o,int version,NoteRole role){
 if(band(o)<0)return {};
 auto key=family(p,o);
 if(version<3)return key!="jumping"&&relevant(p.position,key)?key:std::string();
 if(key.rfind("awareness",0)==0){
  if(role==NR::Snapper)key="awareness specialist";
  if(role==NR::Athlete)key="awareness";
 }
 if(key=="long snapping")key="long snap";
 if(key=="bc vision"||key=="ball-carrier vision")key="ball carrier vision";
 return roleEligible(role,key)?key:std::string();
}

inline bool hasNewEvidence(const Json& saved,const NotesProspect& p){
 if(p.progress<25)return false;
 const NoteRole role=noteRole(p.position);
 const auto version=saved.value("version",1);
 const auto& claims=saved.at("claims");
 for(const auto& o:p.observations){auto key=observationKey(p,o,version,role);if(!key.empty()&&!claims.contains(key))return true;}
 return false;
}
inline std::string storedBackground(const Json& saved){
 if(!saved.is_object())return {};
 for(const auto* key:{"backgroundNote","athleticBackground","flavor"}){
  const auto field=saved.find(key);
  if(field==saved.end()||!field->is_string())continue;
  std::string line;
  noteLines(field->get_ref<const std::string&>(),[&](std::string_view part){
   line=presentNote(std::string(part));
   return line.empty();
  });
  if(!line.empty())return line;
 }
 return {};
}
// Append immutable claims as evidence is obtained. Existing keys/wording survive
// better scouting, staff changes and reloads; no native records are changed.
inline Json obtain(Json saved,const NotesProspect& p,const Context& context,std::optional<double> pipeline){
 const auto role=noteRole(p.position);
 const bool hadReport=saved.is_object();
 if(!hadReport){
  const auto version=currentReportVersion(role);
  saved={{"version",version},{"claims",Json::object()},{"flavor",""}};
  if(version>=2)saved["position"]=p.position;
 }
 const auto version=saved.value("version",1);
 const bool focused=version>=2;
 const bool preserveOrder=hadReport&&!focused;
 if(!saved.contains("displayOrder")){
  auto& order=saved["displayOrder"]=Json::array();
  const auto& claims=saved.at("claims");
  if(preserveOrder)for(auto it=claims.begin();it!=claims.end();++it)order.push_back(it.key());
 }
 if(p.progress<25)return saved;
 if(revealBackground(p)&&storedBackground(saved).empty())saved["backgroundNote"]=std::string(newBackground(p));
 const auto chances=reliability(pipeline);
 for(const auto& o:p.observations){
  const auto f=observationKey(p,o,version,role);
  if(f.empty()||saved["claims"].contains(f))continue;
  const auto b=band(o);
  const auto h=notesHash(p.identity+"|"+f+"|report-v1");const auto roll=double(h%10000)/10000.;
  const int quality=roll<chances.accurate?0:roll<chances.accurate+chances.exaggerated?1:2;
  const int claim=quality==0?b:quality==1?(std::max)(0,b-1):b==0?1:b==1?2:1;
  const auto emphasis=focused?noteEmphasis(o.low,o.high,b,claim):0;
  const char* phrasing=nullptr;
  if(version>=3){
   if((noteKickers|roles(NR::Snapper))&roles(role))phrasing=roleVariant(role,f,claim,h);
   else phrasing=fieldVariant(role,f,claim,emphasis!=0,h);
  }
  if(focused&&!phrasing){
   phrasing=roleNote(f,p.position,claim,emphasis,h);
   if(!phrasing&&emphasis&&(version==2||focusedNotes(p.position)))phrasing=emphaticNote(f,p.position,claim,h);
  }
  std::string text;
  if(phrasing)text=phrasing;
  else for(const auto& entry:resume::prose)if(f==entry.attribute){
   constexpr const char* sources[]={"Film assessment: ","Coach's early assessment: ","Unverified camp impression: ","Scouting observation: "};
   if(version<3)text=sources[(h>>18)%4];
   const auto specific=technique(p.position,f,claim,h);text+=specific?specific:entry.bands[claim][(h>>24)%3];break;
  }
  if(text.empty())continue;
  saved["claims"][f]={{"text",text},{"truthBand",b},{"claimBand",claim},{"quality",quality},{"priority",universal(f)?40:100},{"order",h},{"calibration","pipeline-v2"},{"staffData",staffDataName(context.staffData)},{"pipelineKnown",bool(pipeline)&&std::isfinite(*pipeline)},{"pipeline",pipeline&&std::isfinite(*pipeline)?Json(*pipeline):Json()},{"progress",p.progress}};
  if(focused)saved["claims"][f]["emphasis"]=emphasis;
 }
 std::vector<std::string> order;for(auto i=saved.at("claims").begin();i!=saved.at("claims").end();++i)order.push_back(i.key());
 std::stable_sort(order.begin(),order.end(),[&](const std::string& a,const std::string& b){const auto& x=saved.at("claims").at(a);const auto& y=saved.at("claims").at(b);const auto xp=x.value("priority",40),yp=y.value("priority",40);return xp!=yp?xp>yp:x.value("order",uint64_t(0))<y.value("order",uint64_t(0));});
 if(preserveOrder){
  auto& prior=saved.at("displayOrder");
  for(const auto& key:order)if(std::find(prior.begin(),prior.end(),key)==prior.end())prior.push_back(key);
 }else saved["displayOrder"]=order;
 return saved;
}

struct DisplayedNote {std::string key,pairedWith,text;};

inline std::vector<DisplayedNote> focusedDisplay(const Json& saved,unsigned limit){
 struct Choice {
  std::string_view key,paired;
  const Json* claim=nullptr;
  const char* combined=nullptr;
  int score=0;
  unsigned directions=0; // Bit 0: standout strength. Bit 1: standout concern.
  uint64_t order=0;
 };
 const auto& claims=saved.at("claims");
 const auto pos=saved.at("position").get_ref<const std::string&>();
 const bool expanded=saved.value("version",1)>=3;
 const auto role=noteRole(pos);
 auto topicFor=[&](std::string_view key){return expanded?roleTopic(role,key):noteTopic(key);};
 std::map<std::string_view,Choice> available,topics;
 auto offer=[&](const Choice& choice){
  auto [it,inserted]=topics.try_emplace(topicFor(choice.key),choice);
  if(!inserted&&(choice.score>it->second.score||(choice.score==it->second.score&&choice.order<it->second.order)))it->second=choice;
 };
 for(auto it=claims.begin();it!=claims.end();++it){
  if(noteLimit(it->value("progress",0.))>limit)continue;
  const std::string_view key=it.key();
  const auto topic=topicFor(key);
  const auto band=it->at("claimBand").get<int>();
  const auto emphasis=it->value("emphasis",0);
  int score=it->value("priority",40)+(band==1?0:20)+emphasis*34;
  if(tackleNotes(pos)&&(topic=="speed"||topic=="change of direction"||key=="strength"))score-=30;
  if(key=="carrying")score-=30;
  if(pos=="WR"&&key=="run block"&&!emphasis)score-=24;
  if(pos=="TE"&&(key=="run block"||key=="catch in traffic"))score+=12;
  if(expanded)score+=roleWeight(role,key,emphasis);
  const Choice choice{key,{},&*it,nullptr,score,emphasis&&band!=1?(band==0?1u:2u):0u,it->at("order").get<uint64_t>()};
  available.emplace(key,choice);
  offer(choice);
 }
 auto offerPair=[&](std::string_view firstKey,std::string_view secondKey,const char* const* lines){
  const auto a=available.find(firstKey),b=available.find(secondKey);
  if(a==available.end()||b==available.end())return;
  const auto& x=a->second;
  const auto& y=b->second;
  const auto directions=x.directions|y.directions;
  const auto first=x.claim->at("claimBand").get<int>(),second=y.claim->at("claimBand").get<int>();
  if(!directions||first==1||second==1)return;
  const auto variant=first==second?(first==0?0:1):(first==0?2:3);
  offer({x.key,y.key,nullptr,lines[variant],(std::max)(x.score,y.score)+7,directions,(std::min)(x.order,y.order)});
 };
 for(const auto& pair:coupledNotes){
  if(!expanded){if(pair.tackle==tackleNotes(pos))offerPair(pair.first,pair.second,pair.lines);continue;}
  if(pair.tackle?(noteLine&roles(role)):(role==NR::Receiver||role==NR::TightEnd))offerPair(pair.first,pair.second,pair.lines);
 }
 if(expanded)for(const auto& pair:rolePairs)if(pair.positions&roles(role))offerPair(pair.first,pair.second,pair.lines);
 std::vector<Choice> sorted;
 for(const auto& topic:topics)sorted.push_back(topic.second);
 auto better=[](const Choice& a,const Choice& b){return a.score!=b.score?a.score>b.score:a.order!=b.order?a.order<b.order:a.key<b.key;};
 std::sort(sorted.begin(),sorted.end(),better);
 const auto count=(std::min)(sorted.size(),size_t(limit));
 // If the best few subjects are all positive or all negative, make space
 // for the other side only when there is a clear, distinct standout.
 unsigned directions=0;
 for(size_t i=0;i<count;++i)directions|=sorted[i].directions;
 if(count>=2&&(directions==1||directions==2)){
  const auto other=std::find_if(sorted.begin()+count,sorted.end(),[&](const Choice& c){return (c.directions&~directions)!=0;});
  if(other!=sorted.end())sorted[count-1]=*other;
 }
 std::vector<DisplayedNote> result;
 for(size_t i=0;i<count;++i){
  const auto& choice=sorted[i];
  result.push_back({std::string(choice.key),std::string(choice.paired),choice.combined?choice.combined:presentNote(choice.claim->at("text").get<std::string>())});
 }
 return result;
}

inline std::vector<DisplayedNote> displayedNotes(const Json& saved,double progress){
 const auto limit=noteLimit(progress);
 if(!limit||!saved.is_object())return {};
 if(saved.value("version",1)>=2)return focusedDisplay(saved,limit);
 const auto& claims=saved.at("claims");
 std::vector<DisplayedNote> result;
 auto add=[&](const std::string& key){result.push_back({key,{},presentNote(claims.at(key).at("text").get<std::string>())});};
 if(saved.contains("displayOrder")){
  for(const auto& key:saved.at("displayOrder")){if(result.size()==limit)break;add(key.get_ref<const std::string&>());}
 }else for(auto it=claims.begin();it!=claims.end()&&result.size()<limit;++it)add(it.key());
 return result;
}

inline std::string render(const Json& saved,double progress,bool blind){
 if(blind)return "SCOUTING NOTES\n- Player evaluation is hidden by Recruiting Blind.";
 std::string out="SCOUTING NOTES";
 if(!saved.is_object()||progress<25)return out+"\n- Scout further for individual observations.";
 const auto notes=displayedNotes(saved,progress);
 for(const auto& note:notes)appendNoteBullets(out,note.text);
 if(notes.empty())out+="\n- Need a closer look before judging individual traits.";
 return out;
}
}
