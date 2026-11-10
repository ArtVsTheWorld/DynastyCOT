#include "../Source/RecruitInterestResponse.h"
#include "../Source/RecruitScoutingFacts.h"
#include "../Source/RecruitSmallCard.h"
#include <cassert>
#include <iostream>
#include <deque>
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
int main(){using namespace recruitingfog::interest;unsigned checks=0;
 for(unsigned choice=0;choice<3;++choice)for(unsigned scale=0;scale<5;++scale){assert(effective(false,scale,choice)==0);assert(effective(true,scale,choice)==(scale==4?3:choice));checks+=2;}
 for(unsigned seed=0;seed<1000;++seed){std::vector<School> a{{3,"Zulu",100},{1,"Alpha",99},{2,"Beta",1}},b=a;classify(a,seed,123);classify(b,seed,123);assert(a[0].standing==Standing::Favorite&&a[1].standing==Standing::Favorite&&a[2].standing==Standing::LongShot);group(a);group(b);assert(a[0].team==1&&a[1].team==3&&a[2].team==2);assert(a[0].standing==b[0].standing);checks+=3;}
 Knowledge k;k.league=123;k.userTeam=7;k.recruits[99]={"Test","Recruit",{{7,1}}};
 Json recruit={{"RecordId",99},{"FirstName","Test"},{"LastName","Recruit"},{"PercentageScouted",0},{"CurrentNILOffer",10},{"Hours",30},{"UserRanking",2},{"TopSchools",Json::array({{{"TeamId",7},{"TeamName","Zulu"},{"InfluenceTotal",100},{"InfluenceDelta",12},{"InfluenceMaxDisplayValue",500},{"InfluenceMinDisplayValue",0},{"RankChange",2},{"IsCurrentUserTeam",true},{"VisitValue","Week 12"}},{{"TeamId",2},{"TeamName","Alpha"},{"InfluenceTotal",99},{"InfluenceDelta",5},{"InfluenceMaxDisplayValue",500},{"InfluenceMinDisplayValue",0},{"RankChange",1},{"IsCurrentUserTeam",false},{"VisitValue","---"}}})}};
 for(auto mode:{Mode::Native,Mode::Feedback,Mode::RankOnly,Mode::Blind})for(unsigned league:{123,456}){
  View v;auto root=v.add(Json::array({recruit}));auto stats=transform(v,root,mode,&k,league,[&](A at,const char* key,const char* text){reinterpret_cast<Node*>(at)->object[key]=v.add(text);},[&](A at,const char* key,bool b){reinterpret_cast<Node*>(at)->object[key]=v.add(b);},[&](A at,double n){reinterpret_cast<Node*>(at)->value=n;},[&](A at,const std::vector<A>& values){reinterpret_cast<Node*>(at)->array=values;});auto out=v.dump(root)[0];
  assert(stats.recruits==1&&out["BTFPreviouslyScouted"]==(league==123));assert(out["CurrentNILOffer"]==10&&out["Hours"]==30&&out["UserRanking"]==2&&out["PercentageScouted"]==0);checks+=2;
  if(mode==Mode::Native||mode==Mode::RankOnly){assert(out["TopSchools"]==recruit["TopSchools"]);++checks;}else{assert(out["TopSchools"][0]["TeamName"]=="Alpha"&&out["TopSchools"][0]["VisitValue"]=="---"&&out["TopSchools"][0]["RankChange"]==1);assert(out["TopSchools"][0]["InfluenceTotal"]==(mode==Mode::Blind?0:90));checks+=2;}
 }

 {View v;const auto root=v.add(Json::array({recruit}));
  auto apply=[&](Mode mode){return transform(v,root,mode,&k,123,[&](A at,const char* key,const char* text){reinterpret_cast<Node*>(at)->object[key]=v.add(text);},[&](A at,const char* key,bool b){reinterpret_cast<Node*>(at)->object[key]=v.add(b);},[&](A at,double n){reinterpret_cast<Node*>(at)->value=n;},[&](A at,const std::vector<A>& values){reinterpret_cast<Node*>(at)->array=values;});};
  apply(Mode::Feedback);const auto first=v.dump(root);for(int n=0;n<50;++n){apply(Mode::Feedback);assert(v.dump(root)==first);++checks;}
  apply(Mode::Blind);apply(Mode::RankOnly);auto restored=v.dump(root)[0]["TopSchools"];for(auto& school:restored){school.erase("BTFInterestOriginal");school.erase("BTFStanding");}assert(restored==recruit["TopSchools"]);++checks;
  apply(Mode::Feedback);apply(Mode::Native);restored=v.dump(root)[0]["TopSchools"];for(auto& school:restored){school.erase("BTFInterestOriginal");school.erase("BTFStanding");}assert(restored==recruit["TopSchools"]);++checks;
 }
 {View v;auto sparse=recruit;sparse.erase("TopSchools");const auto root=v.add(Json::array({sparse}));auto result=transform(v,root,Mode::Feedback,&k,123,[&](A at,const char* key,const char* text){reinterpret_cast<Node*>(at)->object[key]=v.add(text);},[&](A at,const char* key,bool b){reinterpret_cast<Node*>(at)->object[key]=v.add(b);},[](A,double){assert(false);},[](A,const std::vector<A>&){assert(false);});assert(result.scouted==1&&result.unsupported==1&&v.dump(root)[0]["BTFPreviouslyScouted"]==true);++checks;}

 {Knowledge data;data.league=123;data.userTeam=7;data.maxScoutingUnits=5;data.recruits[99]={"Alex","Bell",{{7,2}}};assert(scoutProgress(data,123,99,7)==40u);assert(std::string(scoutingAsset(40))=="dynas_general_Scouting_40");++checks;
  data.recruits[99].boards.insert(7);assert(!scoutProgress(data,123,99,7));data.recruits[99].boards.clear();data.recruits[99].committed=true;assert(!scoutProgress(data,123,99,7));data.recruits[99].committed=false;assert(!scoutProgress(data,456,99,7)&&!scoutProgress(data,123,99,8));data.maxScoutingUnits=0;assert(!scoutProgress(data,123,99,7));checks+=4;
  for(unsigned percentage=0;percentage<=100;++percentage){assert(std::string(scoutingAsset(percentage)).find("dynas_general_Scouting")==0);++checks;}
 }

 {Knowledge history=k;history.season=2026;history.maxScoutingUnits=5;history.recruits[99].times[7]=2;history.recruits[99].boards.insert(7);
  View view;auto native=recruit;native["PercentageScouted"]=80;auto root=view.add(Json::array({native}));auto fresh=observeScouting(view,root,history,{});assert(fresh&&fresh->board.count(99));
  // No timer, capture or page flip: the just-returned native percentage wins
  // and the current off-board cell overrides the earlier board snapshot.
  assert(currentScouting(history,fresh,123,99,7,"TargetNone")==80u);assert(!currentScouting(history,fresh,123,99,7,"TargetTargeted"));assert(!currentScouting(history,fresh,123,99,7,"Target,61"));assert(!currentScouting(history,fresh,456,99,7,"TargetNone"));checks+=4;
  auto unchanged=observeScouting(view,root,history,fresh);assert(unchanged==fresh);++checks;
  View empty;auto removed=observeScouting(empty,empty.add(Json::array()),history,fresh);assert(removed&&!removed->board.count(99)&&currentScouting(history,removed,123,99,7,"TargetLockedOut")==80u);++checks;
  native["PercentageScouted"]=100;View complete;auto finished=observeScouting(complete,complete.add(Json::array({native})),history,removed);assert(currentScouting(history,finished,123,99,7,"TargetNone")==100u);assert(!currentScouting(history,finished,123,99,7,"TargetLockedOut"));checks+=2;
  native["PercentageScouted"]=0;View reset;auto unscouted=observeScouting(reset,reset.add(Json::array({native})),history,finished);assert(!currentScouting(history,unscouted,123,99,7,"TargetNone"));++checks;
  native["PercentageScouted"]=101;View bad;assert(!observeScouting(bad,bad.add(Json::array({native})),history,finished));++checks;
  auto other=history;other.league=456;other.recruits.clear();assert(!currentScouting(other,finished,456,99,7,"TargetNone"));++checks;
 }

 {std::vector<School> all;for(int i=0;i<10;++i)all.push_back({i,std::to_string(i),100.-i*10});auto five=std::vector<School>(all.begin(),all.begin()+5);classify(all,99,123);classify(five,99,123);for(size_t i=0;i<five.size();++i){assert(all[i].standing==five[i].standing);++checks;}}
 {Json native={{"commitScore",700},{"recordKey",99},{"firstName","Test"},{"lastName","Recruit"},{"DesiredNILValue",35},{"TargetedState",7},{"topSchoolsSubpanelInfo",Json::array()}};
  for(const auto& school:recruit["TopSchools"]){native["topSchoolsSubpanelInfo"].push_back({{"teamLogoId",school["TeamId"]},{"teamName",school["TeamName"]},{"influenceTotal",school["InfluenceTotal"]},{"infuenceThisWeek",school["InfluenceTotal"].get<double>()+1},{"influenceDelta",school["InfluenceDelta"]},{"influenceMaxDisplayValue",school["InfluenceMaxDisplayValue"]},{"influenceMinDisplayValue",school["InfluenceMinDisplayValue"]},{"RankChange",school["RankChange"]},{"IsCurrentUserTeam",school["IsCurrentUserTeam"]},{"offerMade",true}});}
  View v;auto root=v.add(native);auto apply=[&](Mode mode){return transformSmallCard(v,root,mode,123,[&](A at,const char* key,const char* text){reinterpret_cast<Node*>(at)->object[key]=v.add(text);},[&](A at,const char* key,bool b){reinterpret_cast<Node*>(at)->object[key]=v.add(b);},[&](A at,double n){reinterpret_cast<Node*>(at)->value=n;},[&](A at,const std::vector<A>& values){reinterpret_cast<Node*>(at)->array=values;});};
  auto result=apply(Mode::Feedback);const auto displayed=v.dump(root);assert(result.schools==2&&displayed["topSchoolsSubpanelInfo"][0]["teamName"]=="Alpha"&&displayed["topSchoolsSubpanelInfo"][0]["influenceMaxDisplayValue"]==100.125);assert(displayed["DesiredNILValue"]==35&&displayed["TargetedState"]==7);checks+=2;
  assert(displayed["commitScore"]==700);for(const auto& school:displayed["topSchoolsSubpanelInfo"]){assert(school["infuenceThisWeek"]==school["influenceTotal"]);++checks;}
  apply(Mode::Feedback);assert(v.dump(root)==displayed);++checks;
  apply(Mode::Blind);assert(v.dump(root)["topSchoolsSubpanelInfo"][0]["influenceTotal"]==0);++checks;
  apply(Mode::Native);auto restored=v.dump(root)["topSchoolsSubpanelInfo"];for(auto& school:restored){school.erase("BTFStanding");school.erase("BTFInterestOriginal");}assert(restored==native["topSchoolsSubpanelInfo"]&&v.dump(root)["commitScore"]==700);++checks;
 }
 std::cout<<"PASS: "<<checks<<" interest/response checks: defaults and Blind precedence, multiple favorites, deterministic boundary uncertainty, alphabetical ties, native/rank-only passthrough, untouched budgets/hours/scouting, native arrows/visits and Dynasty-isolated prior scouting.\n";
}
