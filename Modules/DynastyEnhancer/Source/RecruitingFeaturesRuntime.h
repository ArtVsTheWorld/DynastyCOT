#pragma once
#include <windows.h>
#include "RecruitInterestResponse.h"
#include "RecruitScoutingFacts.h"
#include "RecruitSmallCard.h"
#include "RecruitInterestUiPrograms.h"
#include "RecruitNotesUiRuntime.h"
#include "EnhancementOptions.h"
#include "LiveTables.h"
namespace recruitingfog::features {
inline notesui::Runtime rowUi(interestui::rows()),progressUi(interestui::progress()),cutoffUi(interestui::cutoff()),scoutUi(interestui::scouts()),rankSortUi(interestui::rankSort()),interestSortUi(interestui::interestSort());
inline notesui::Runtime targetUi(interestui::targets());
inline notesui::Runtime smallCardUi(interestui::smallCards());
inline const std::array<notesui::Runtime*,8> banks{{&rowUi,&progressUi,&cutoffUi,&scoutUi,&rankSortUi,&interestSortUi,&targetUi,&smallCardUi}};
inline std::array<std::atomic<bool>,8> failed{};inline std::array<std::atomic<unsigned>,8> attached{};
inline std::atomic<bool> prepared{false};
inline std::shared_ptr<const interest::Knowledge> published;inline std::atomic<bool> requested{true};inline uint64_t nextCapture=0;inline uint32_t lastLeague=0;
inline std::shared_ptr<const interest::UiScoutingFacts> freshScouting;
inline std::atomic<bool> scoutingChanged{false};
inline std::set<std::string> fields(const std::string& n){
 if(n=="SeasonInfo")return {"CurrentSeasonYear","CurrentWeek","CurrentWeekType","CurrentStage"};if(n=="Franchise")return {"LeagueID"};if(n=="FranchiseServer_CareerFlow")return {"IsStageActive","IsStageAdvancing"};if(n=="Team")return {"TeamIndex","RecruitingBoard"};if(n=="RecruitingBoard")return {"Recruits"};if(n=="RecruitTarget"||n=="UserRecruitTarget")return {"Recruit"};if(n=="FranchiseUser")return {"Team","IsRetired","AdminLevel"};if(n=="Recruit")return {"Player","RecruitStage"};if(n=="Player")return {"FirstName","LastName"};if(n=="ProspectInteraction")return {"Team","Recruit","TimesScouted"};if(n=="RecruitingTunables")return {"MaxTimesScouted"};return {};
}
inline bool roots(const std::string& n){return n=="SeasonInfo"||n=="Franchise"||n=="FranchiseServer_CareerFlow"||n=="FranchiseUser"||n=="ProspectInteraction"||n=="RecruitingTunables";}
inline bool writeUiBytes(uintptr_t destination,const void* bytes,size_t count){__try{memcpy(reinterpret_cast<void*>(destination),bytes,count);return true;}__except(EXCEPTION_EXECUTE_HANDLER){return false;}}
inline void writeUiNumber(uintptr_t value,double number){if(!writeUiBytes(value+0x18,&number,sizeof(number)))throw std::runtime_error("Recruit UI value became unavailable");}
template<class Read>inline void reorderUi(uintptr_t value,const std::vector<uintptr_t>& order,Read read){
 uintptr_t begin=0,end=0;if(!read(value+0x18,&begin,8)||!read(value+0x20,&end,8)||end<begin||(end-begin)%8||(end-begin)/8!=order.size()||order.size()>32)throw std::runtime_error("Recruit UI list changed");
 std::vector<uintptr_t> existing(order.size());if(!read(begin,existing.data(),existing.size()*8))throw std::runtime_error("Recruit UI list unavailable");auto expected=order;std::sort(existing.begin(),existing.end());std::sort(expected.begin(),expected.end());if(existing!=expected||!writeUiBytes(begin,order.data(),order.size()*8))throw std::runtime_error("Recruit UI membership changed");
}
inline void prepare(){for(auto* bank:banks)bank->prepare();prepared=true;}
template<class Read>inline nlohmann::json tick(uintptr_t image,Read read){
 const auto league=enhancements::currentOptionsLeague.load();if(!league||!enhancements::presentationAllowed())return nullptr;
 if(league!=lastLeague){std::atomic_store(&published,std::shared_ptr<const interest::Knowledge>{});std::atomic_store(&freshScouting,std::shared_ptr<const interest::UiScoutingFacts>{});lastLeague=league;requested=true;nextCapture=0;}
 const bool edited=scoutingChanged.exchange(false);if((!edited&&GetTickCount64()<nextCapture)||!requested.exchange(false))return nullptr;nextCapture=GetTickCount64()+15000;
 try{
  auto q=[&](uintptr_t at){uintptr_t p=0;if(!read(at,&p,8))throw std::runtime_error("Scouting ownership unavailable");return p;};const auto binding=dynasty::live::activeSession(image,q);
  auto raw=dynasty::live::Capture(image,GetCurrentProcess(),fields,roots).run();auto data=std::make_shared<interest::Knowledge>(interest::knowledge(raw));
  if(data->league!=league||!(dynasty::live::activeSession(image,q)==binding))throw std::runtime_error("Scouting Dynasty changed");
  auto facts=std::atomic_load(&freshScouting);if(facts&&!facts->matches(*data))std::atomic_compare_exchange_strong(&freshScouting,&facts,std::shared_ptr<const interest::UiScoutingFacts>{});
  std::atomic_store(&published,std::shared_ptr<const interest::Knowledge>(data));return {{"event","btf_scout_cache_ready"},{"records",data->recruits.size()},{"league",league}};
 }catch(const std::exception& e){requested=true;return {{"event","btf_scout_cache_deferred"},{"reason",e.what()}};}
}
template<class Read>inline int beforeInvoke(void* object,Read read,bool fixedFrame=false,bool active=true){
 if(!prepared)return -1;
 const auto definition=*reinterpret_cast<const uintptr_t*>(static_cast<unsigned char*>(object)+0x58);const auto* h=reinterpret_cast<const unsigned char*>(definition);
 static const notesui::FrameDispatch<8> dispatch(banks);const auto candidates=dispatch.candidates(h,fixedFrame);if(!candidates)return -1;
 const auto scoped=active&&enhancements::presentationAllowed();auto selected=enhancements::options();const auto mode=interest::effective(selected.recruiting,unsigned(selected.scale),unsigned(selected.interest));
 for(size_t i=0;i<banks.size();++i)if((candidates&(uint64_t(1)<<i))&&!failed[i]&&(scoped||banks[i]->retainedDefinitionCount()))try{
  const auto scale=i==6?0:mode;const auto result=banks[i]->beforeInvoke(object,read,fixedFrame,scoped,scale,selected.colors,selected.roster,selected.team,selected.dollars,unsigned(selected.teamScale));
  if(result>=0){attached[i].fetch_add(1);return int(i);}
 }catch(...){failed[i]=true;}
 return -1;
}
template<class View,class StringSetter,class BoolSetter,class NumberSetter,class Reorder>
inline interest::TransformStats response(View& view,uintptr_t root,StringSetter text,BoolSetter boolean,NumberSetter number,Reorder reorder){
 if(!enhancements::presentationAllowed())return {};requested=true;auto data=std::atomic_load(&published);auto selected=enhancements::options();const auto league=enhancements::currentOptionsLeague.load();
 if(data&&data->league==league){auto previous=std::atomic_load(&freshScouting);auto next=interest::observeScouting(view,root,*data,previous);if(next&&next!=previous&&enhancements::currentOptionsLeague.load()==league&&std::atomic_compare_exchange_strong(&freshScouting,&previous,next))scoutingChanged=true;}
 return interest::transform(view,root,interest::Mode(interest::effective(selected.recruiting,unsigned(selected.scale),unsigned(selected.interest))),data.get(),league,text,boolean,number,reorder);
}
template<class View,class StringSetter,class BoolSetter,class NumberSetter,class Reorder>
inline interest::TransformStats prospectResponse(View& view,uintptr_t root,StringSetter text,BoolSetter boolean,NumberSetter number,Reorder reorder){
 if(!enhancements::presentationAllowed())return {};const auto selected=enhancements::options();const auto league=enhancements::currentOptionsLeague.load();
 return interest::transformSmallCard(view,root,interest::Mode(interest::effective(selected.recruiting,unsigned(selected.scale),unsigned(selected.interest))),league,text,boolean,number,reorder);
}
}
