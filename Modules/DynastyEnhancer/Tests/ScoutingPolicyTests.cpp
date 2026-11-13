#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../Source/EnhancementOptionsRuntime.h"
#include "../Source/EnhancementSettingsMenu.h"
#include "../Source/ScoutingEvidence.h"
#include "../Source/RecruitingGradePolicy.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <set>
#include <deque>
#include "../Source/RecruitingUiResponse.h"
using Json=nlohmann::json;using A=uintptr_t;
struct Node {Json value;std::map<std::string,A> object;std::vector<A> array;};
struct View {
 std::deque<Node> nodes;
 A add(Json j){nodes.push_back({j,{},{}});A at=reinterpret_cast<A>(&nodes.back());if(j.is_object())for(auto i=j.begin();i!=j.end();++i)reinterpret_cast<Node*>(at)->object[i.key()]=add(i.value());if(j.is_array())for(auto& v:j)reinterpret_cast<Node*>(at)->array.push_back(add(v));return at;}
 bool members(A at,std::map<std::string,A>& out){auto* n=reinterpret_cast<Node*>(at);out=n->object;return n->value.is_object();}
 bool array(A at,std::vector<A>& out,size_t max){auto* n=reinterpret_cast<Node*>(at);out=n->array;return n->value.is_array()&&out.size()<=max;}
 bool number(A at,double& out){auto& j=reinterpret_cast<Node*>(at)->value;if(!j.is_number())return false;out=j.get<double>();return std::isfinite(out);}
 bool boolean(A at,bool& out){auto& j=reinterpret_cast<Node*>(at)->value;if(!j.is_boolean())return false;out=j.get<bool>();return true;}
 bool string(A at,std::string& out){auto& j=reinterpret_cast<Node*>(at)->value;if(!j.is_string())return false;out=j.get<std::string>();return true;}
 Json dump(A at){auto& n=*reinterpret_cast<Node*>(at);if(n.value.is_object()){Json j=Json::object();for(auto [k,v]:n.object)j[k]=dump(v);return j;}if(n.value.is_array()){Json j=Json::array();for(auto a:n.array)j.push_back(dump(a));return j;}return n.value;}
};

bool outside(){return false;}
int main(){
 using namespace enhancements;using namespace recruitingfog;
 for(unsigned r=0;r<5;++r)for(unsigned p=0;p<5;++p)for(unsigned t=0;t<5;++t)for(unsigned flags=0;flags<8;++flags){
  Options o;o.recruitingScale=GradeScale(r);o.scale=GradeScale(p);o.teamScale=GradeScale(t);o.recruiting=flags&1;o.roster=flags&2;o.team=flags&4;
  currentOptions=o.packed();assert(Options::validBits(o.packed())&&configuredOptions().packed()==o.packed());assert(display(Visibility::Revealed,91)==gradeLabel(91,o.recruitingScale));assert(ratingDisplay(91)==gradeLabel(91,o.scale));
 }
 nlohmann::json saved={{"format",2},{"league",42},{"socialMedia",false},{"recruitingFog",true},{"rosterFog",false},{"teamFog",true},{"playerGradeScale",3},{"teamGradeScale",4},{"dollarValues",true},{"recruitInterestDisplay",2}};
 auto migrated=decodeOptions(saved,42);assert(migrated.recruitingScale==GradeScale::Descriptive&&!migrated.roster&&migrated.teamScale==GradeScale::Blind&&migrated.dollars&&!migrated.social&&!migrated.measurables);
 saved["athleticMeasurables"]=true;assert(decodeOptions(saved,42).measurables);saved["athleticMeasurables"]=false;assert(!decodeOptions(saved,42).measurables);assert(!Options{}.measurables&&!Options::unpack(0).measurables);currentOptions=migrated.packed();presentationGuard=outside;assert(!options().recruiting&&!options().roster&&!options().team&&!options().measurables);presentationGuard=nullptr;
 currentOptionsLeague=42;
 assert(measurables::interpolate(40,95,500)==95&&measurables::interpolate(99,95,500)==500&&measurables::interpolate(-5,95,500)==95);
 std::map<std::string,double> inputs={{"speed",90},{"acceleration",80},{"strength",78},{"agility",86},{"change of direction",85},{"jumping",80}};
 const auto dash=measurables::estimate("speed",inputs,"recruit").value;assert(dash==measurables::estimate("acceleration",inputs,"recruit").value);inputs.erase("acceleration");assert(measurables::estimate("speed",inputs,"recruit").value=="?");inputs["acceleration"]=80;
 std::set<std::string> dashValues;double minimum=10,maximum=0,mean=0;
 for(unsigned n=0;n<10000;++n){const auto id=std::to_string(n);const auto v=measurables::estimate("speed",inputs,id);dashValues.insert(v.value);const auto value=std::stod(v.value);mean+=value;minimum=(std::min)(minimum,value);maximum=(std::max)(maximum,value);assert(std::abs(value-measurables::interpolate(86,6.5,4.21))<=.086);
  auto lifts=measurables::estimate("strength",inputs,id).value;assert(std::stoi(lifts)%5==0);
 }
 assert(dashValues.size()>=15);assert(std::abs(mean/10000-measurables::interpolate(86,6.5,4.21))<.01);
 currentOptionsLeague=43;assert(measurables::estimate("speed",inputs,"recruit").value!=dash);currentOptionsLeague=42;assert(measurables::estimate("speed",inputs,"recruit").value==dash);
 for(int tier=0;tier<=5;++tier){auto r=evidence::reliability(tier/5.);assert(std::abs(r.accurate+r.exaggerated+r.misleading-1)<1e-9);std::cout<<"pipeline tier "<<tier<<" "<<r.accurate<<' '<<r.exaggerated<<' '<<r.misleading<<'\n';}
 assert(evidence::reliability(0).accurate<.5&&evidence::reliability(1).accurate>.9&&evidence::reliability(.6).accurate==.6&&evidence::reliability(std::nullopt).accurate==.6);

 // Native per-attribute reveal is authoritative, not the progress percentage.
 for(int progress:{0,20,40,80,100})for(bool blind:{false,true}){
  View v;auto attr=[&](const char* name,int rating,bool range){return Json{{"Name",name},{"HaveSlider",range},{"SliderStart",rating},{"SliderEnd",range?100:rating}};};
  Json recruitUi={{"FirstName","Alex"},{"LastName","Test"},{"Hometown","Town"},{"State","OH"},{"RecordId",77},{"PercentageScouted",progress},{"ScoutingAttributes",Json::array({attr("Speed",90,false),attr("Acceleration",80,progress<80),attr("Strength",0,false)})}};
  auto root=v.add(Json::array({recruitUi}));Options opt;opt.measurables=true;opt.recruitingScale=blind?GradeScale::Blind:GradeScale::PlusMinus;currentOptions=opt.packed();
  convertRecruitingResponse(v,root,[&](A at,const char* k,const char* text){reinterpret_cast<Node*>(at)->object[k]=v.add(text);},[&](A at,const char* k,bool b){reinterpret_cast<Node*>(at)->object[k]=v.add(b);});auto out=v.dump(root)[0]["ScoutingAttributes"];
  if(blind)assert(out[0]["SliderStart"]==u8"–");else if(progress<80)assert(out[0]["SliderStart"]=="?");else assert(out[0]["SliderStart"]==out[1]["SliderStart"]&&out[0]["Name"]=="40-Yard Dash (est.)");
  if(progress<100)assert(out[2]["SliderStart"]==0);
 }
 {View v;auto root=v.add(Json::array({{{"PercentageScouted",100},{"ScoutingAttributes",Json::array({{{"Name","Speed"},{"HaveSlider",false},{"SliderStart",90},{"SliderEnd",90}}})}}}));Options off;off.recruiting=false;off.measurables=true;currentOptions=off.packed();bool changed=false;convertRecruitingResponse(v,root,[&](A,const char*,const char*){changed=true;},[&](A,const char*,bool){changed=true;});assert(changed);}
 // Independent controls, including a stale saved Blind scale while fog is Off.
 for(bool fog:{false,true})for(bool measurements:{false,true})for(bool blind:{false,true}){
  View v;auto attr=[](const char* name,int rating){return Json{{"Name",name},{"HaveSlider",false},{"SliderStart",rating},{"SliderEnd",rating}};};
  auto root=v.add(Json::array({{{"FirstName","Test"},{"LastName","Recruit"},{"RecordId",77},{"PercentageScouted",100},{"ScoutingAttributes",Json::array({attr("Speed",90),attr("Acceleration",80),attr("Catching",75)})}}}));
  Options opt;opt.recruiting=fog;opt.measurables=measurements;opt.recruitingScale=blind?GradeScale::Blind:GradeScale::PlusMinus;currentOptions=opt.packed();
  convertRecruitingResponse(v,root,[&](A at,const char* k,const char* text){reinterpret_cast<Node*>(at)->object[k]=v.add(text);},[&](A at,const char* k,bool b){reinterpret_cast<Node*>(at)->object[k]=v.add(b);});auto out=v.dump(root)[0]["ScoutingAttributes"];
  if(!fog){assert(out[2]["SliderStart"]==75);if(measurements){assert(out[0]["Name"]=="40-Yard Dash (est.)"&&out[0]["SliderStart"].is_string());}else assert(out[0]["SliderStart"]==90);}
  if(fog&&blind)assert(out[0]["SliderStart"]==u8"–");
 }
 for(int progress:{40,100})for(bool concealed:{false,true}){
  View v;auto attr=[](const char* name,int rating,bool range=false){return Json{{"Name",name},{"HaveSlider",range},{"SliderStart",rating},{"SliderEnd",range?100:rating}};};
  Json attributes=Json::array({attr("Speed",90)});if(concealed)attributes.push_back(attr("Acceleration",0,true));
  auto root=v.add(Json::array({{{"PercentageScouted",progress},{"ScoutingAttributes",attributes}}}));Options opt;opt.measurables=true;currentOptions=opt.packed();int calls=0;
  convertRecruitingResponse(v,root,[&](A at,const char* key,const char* text){reinterpret_cast<Node*>(at)->object[key]=v.add(text);},[&](A at,const char* key,bool value){reinterpret_cast<Node*>(at)->object[key]=v.add(value);},[&](auto&,const auto&,auto& extra){++calls;extra["acceleration"]=80;});
  const auto value=v.dump(root)[0]["ScoutingAttributes"][0]["SliderStart"];
  assert(calls==(progress==100?1:0));if(progress<100)assert(value=="?");else if(concealed)assert(value=="Unavailable");else assert(value.is_string()&&value.get<std::string>().find(" s")!=std::string::npos);
 }
 evidence::Context context;context.staffData=evidence::StaffData::CapturedEffects;NotesProspect recruit;recruit.identity="42|test";recruit.position="QB";recruit.progress=40;recruit.observations={{"Throw Power",83,83,true},{"Awareness",70,75,false}};
 assert(!evidence::relevant("QB","kick power")&&!evidence::relevant("LT","short accuracy")&&evidence::relevant("K","kick accuracy")&&evidence::relevant("DT","tackle"));
 const auto report=evidence::obtain(nullptr,recruit,context,.5),again=evidence::obtain(report,recruit,context,1);assert(report==again);recruit.progress=100;recruit.observations.push_back({"Short Accuracy",60,60,true});auto expanded=evidence::obtain(report,recruit,context,1);assert(expanded["claims"].size()==3&&expanded["claims"]["throw power"]==report["claims"]["throw power"]);
 assert(evidence::render(expanded,100,true).find("zip")==std::string::npos);
 auto begin=std::chrono::steady_clock::now();size_t sink=0;for(unsigned i=0;i<10000;++i)sink+=evidence::render(expanded,100,false).size();auto end=std::chrono::steady_clock::now();
 std::cout<<"PASS: 1000 domain combinations, legacy migration, native scope, measurement reveal/noise/rounding/Dynasty isolation, reliability/stable evidence. Cached Notes render 10000: "<<std::chrono::duration<double,std::milli>(end-begin).count()<<" ms; "<<sink<<" bytes; dash distinct="<<dashValues.size()<<" range="<<minimum<<'-'<<maximum<<"\n";
}
