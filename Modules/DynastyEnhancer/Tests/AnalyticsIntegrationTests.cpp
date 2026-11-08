#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <atomic>
#include <array>
#include <map>
#include <mutex>
#include <optional>
#include <tuple>
#include <iostream>
#include <chrono>
#include <fstream>
#include "../Source/AnalyticsCapture.h"
#include "../Source/AnalyticsBridge.h"
using Json=nlohmann::json;
namespace fixture {
uintptr_t base=0;std::atomic<bool> disabled{false};std::atomic<unsigned> inflight{0};
struct CallGuard{CallGuard(){++inflight;}~CallGuard(){--inflight;}};
bool read(uintptr_t at,void* out,size_t n){SIZE_T got=0;return at>=0x10000&&ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(at),out,n,&got)&&got==n;}
void note(Json){}
bool replace(void*,void*,uintptr_t,uintptr_t){throw std::runtime_error("Tests must not install hooks");}
#include "../Source/AnalyticsRuntime.h"
Json collected;int reserved=0,forwarded=0;std::vector<unsigned> encoded;
template<class T>void __fastcall accept(void*,unsigned row,const char* key,T value){collected[std::to_string(row)][key]=value;}
void __fastcall reserve(void*,int n){reserved=n;}
void __fastcall scalar(void*,const char* value){collected["key"]=value;}
int __fastcall colsCount(void*){return 6;}
int __fastcall colAt(void*,int n){return n+2;}
bool __fastcall sortFallback(void*,int,int column){forwarded=column;return true;}
int __fastcall countFallback(void*,int){++forwarded;return 123;}
void __fastcall encode(unsigned value,void* out,int*){encoded.push_back(value);strcpy_s(*reinterpret_cast<char**>(reinterpret_cast<char*>(out)+8),256,"native-shaped-test-key");}
int checks=0;
void check(bool result,const char* label){++checks;if(!result)throw std::runtime_error(label);}
Json input(){
 Json raw={{"status",{{"league",4294967295u},{"season",{{"CurrentYear",0},{"CurrentSeasonYear",2026},{"CurrentWeek",1},{"CurrentWeekType","RegularSeason"}}}}},{"records",Json::object()}};
 auto record=[&](const char* key,const char* type,Json v,int capacity=150){raw["records"][key]={{"key",key},{"type",type},{"capacity",capacity},{"v",v}};};
 record("1:0","Franchise",{{"LeagueID",4294967295u},{"FCSTeams","2:0"}},1);record("2:0","Team[]",{{"Member0","3:2"}});
 record("3:0","Team",{{"TeamIndex",14},{"DisplayName","Test University"},{"TEAM_LOGO",14}});record("3:1","Team",{{"TeamIndex",15},{"DisplayName","Opponent"},{"TEAM_LOGO",15}});record("3:2","Team",{{"TeamIndex",254},{"DisplayName","FCS Test"}});
 record("4:0","FranchiseUser",{{"Team","3:0"},{"AdminLevel","Owner"},{"IsRetired",false}},32);
 record("5:0","Player",{{"FirstName","Longfirstname"},{"LastName","Longlastname-Suffix"},{"TeamIndex",14},{"Position","QB"},{"GameStats","6:0"},{"OverallRating",99},{"TraitDevelopment","Elite"},{"Height",73},{"Weight",40},{"SchoolYear","Junior"}});
 record("5:1","Player",{{"FirstName","No"},{"LastName","Snaps"},{"TeamIndex",14},{"Position","WR"},{"GameStats",nullptr}});
 record("6:0","GameStats[]",{{"Member0","7:0"},{"Member1","7:0"},{"Member2",nullptr}});
 Json stats={{"SeasonGame","8:0"},{"OpposingTeam","3:1"},{"DOWNSPLAYED",60},{"GAMERATING",99},{"PASSYARDS",300},{"PASSCOMPLETED",22},{"PASSATTEMPTS",34},{"PASSTDS",3},{"PASSINTS",1},{"PASSSACKED",1},{"RUSHYARDS",15},{"RUSHTDS",0},{"RUSHFUMBLES",0}};
 record("7:0","GameOffensiveStats",stats);
 record("8:0","SeasonGame",{{"IsPractice",false},{"SeasonYear",0},{"SeasonWeek",1},{"SeasonWeekType","RegularSeason"},{"GameStatus","HomeWon"},{"HomeTeam","3:0"},{"AwayTeam","3:1"}});
 return raw;
}
}
int main(int argc,char** argv){using namespace fixture;using namespace fixture::nativeanalytics;
 try{
  std::ifstream calibrationFile(std::filesystem::path(__FILE__).parent_path().parent_path()/"Source/analytics-baselines.json");Json calibration;calibrationFile>>calibration;check(analytics::calibration()==calibration,"Embedded calibration differs from the imported frozen source");
  auto raw=input(),unchanged=raw;auto result=analytics::live::dataset(raw);check(raw==unchanged,"Adapter changed captured records");
  check(result["leagueId"]==4294967295u&&result["userTeamIndex"]==14,"Unsigned LeagueID/user team mapping changed");
  check(result["rows"].size()==1&&result["issues"].empty(),"Repeated stat references duplicated games");check(result["teams"][2]["isFcs"]==true,"Native FCSTeams references were ignored");
  const auto grade=result["rows"][0]["analytics"];check(grade["grade"].is_number(),"Verified completed QB performance is not graded");check(!result["rows"][0]["stats"].contains("GAMERATING"),"Native GAMERATING entered grading input");
  auto direct=analytics::grade(result["rows"][0],analytics::calibration(),Json(4294967295u).dump());check(direct==grade,"Native adapter changed the frozen standalone grade model");
  auto changed=raw;changed["records"]["5:0"]["v"]["OverallRating"]=10;changed["records"]["5:0"]["v"]["TraitDevelopment"]="Normal";changed["records"]["7:0"]["v"]["GAMERATING"]=0;
  check(analytics::live::dataset(changed)["rows"][0]["analytics"]==grade,"Hidden/native ratings influence production grade");
  changed=raw;changed["records"]["7:0"]["v"].erase("PASSINTS");check(analytics::live::dataset(changed)["rows"][0]["analytics"]["grade"].is_null(),"Missing stats were fabricated");
  changed=raw;changed["records"]["7:0"]["v"]["DOWNSPLAYED"]=0;check(analytics::live::dataset(changed)["rows"][0]["analytics"]["grade"].is_null(),"Zero snaps generated a grade");
  changed=raw;changed["records"]["8:0"]["v"]["GameStatus"]="Scheduled";check(analytics::live::dataset(changed)["rows"][0]["analytics"]["grade"].is_null(),"Unplayed games generated grades");
  changed=raw;changed["records"]["8:0"]["v"]["IsPractice"]=true;check(analytics::live::dataset(changed)["rows"].empty(),"Practice entered Dynasty grades");
  changed=raw;changed["records"]["7:0"]["v"]["OpposingTeam"]="3:2";auto foreign=analytics::live::dataset(changed);check(foreign["rows"].empty()&&foreign["issues"].size()==1,"Foreign opponent entered game stats");
  auto prepared=prepareSnapshot(result);useSnapshot(std::move(prepared));startOnUserTeam();opened=true;source=7;
  check(teams.front().first==analytics::NationalTeam&&teams.size()==3,"National team choices include an FCS team");check(selectedTeam==14&&page["rows"].size()==2,"Native table does not start on user roster");
  check(page["rows"][0]["grade"].is_number()&&page["rows"][1]["grade"].is_null(),"Missing participation does not remain N/A");
  check(playerCards.at((5u<<17))["height"]=="6' 1\""&&playerCards.at((5u<<17))["weight"]==200,"Native card dimensions changed");
  check(recordKey("5:0")=="000A0000","Native player-reference packing changed");bool rejected=false;try{recordKey("5:131072");}catch(...){rejected=true;}check(rejected,"Out-of-range record accepted");
  void* vt[]={nullptr,reinterpret_cast<void*>(reserve),nullptr,reinterpret_cast<void*>(accept<bool>),reinterpret_cast<void*>(accept<int>),reinterpret_cast<void*>(scalar),reinterpret_cast<void*>(accept<const char*>)};struct Target{void** vt;}target{vt};
  collected=Json::object();columnHook(nullptr,&target,0);check(collected.size()==8&&collected["5"]["display"]=="PFF GRADE"&&collected["6"]["display"]=="CLASSIFICATION","Native columns missing production grades/classification");
  void* cvt[]={nullptr,reinterpret_cast<void*>(colsCount),reinterpret_cast<void*>(colAt)};Target cols{cvt};collected=Json::object();rowHook(nullptr,&target,7,0,&cols);check(reserved==6&&collected["3"]["display"].is_string(),"Native row collector ABI changed");
  encodePrefix=encodeValue=encodeSuffix=encode;collected=Json::object();keyHook(nullptr,&target,7,0);check(encoded.size()==3&&encoded[0]==255&&encoded[1]==(5u<<17)&&encoded[2]==255,"Native serial-key identity changed");
  nativeSort=sortFallback;check(sortHook(nullptr,7,5)&&forwarded==2,"Native sort refresh does not use valid donor column");check(page["rows"].back()["grade"].is_null(),"N/A sorts before real production grades");
  count=countFallback;check(countHook(nullptr,8)==123,"Unrelated table was intercepted");disabled=true;check(countHook(nullptr,7)==123,"Disabled module retained analytics table");disabled=false;
  auto season=analytics::teamPage(result,analytics::NationalTeam,0,analytics::SeasonWeek,"Season");check(season["rows"].size()==2&&season["rows"][0]["grade"].is_number(),"Season/National aggregation changed");
  auto doubleStat=result;auto duplicate=doubleStat["rows"][0];duplicate["statType"]="GameOffensiveKPReturnStats";duplicate["analytics"]["grade"]=nullptr;doubleStat["rows"].push_back(duplicate);check(analytics::teamPage(doubleStat,14,0,1,"RegularSeason")["rows"].size()==2,"Return stats duplicate a player's primary game row");
  doubleStat["rows"][1]["statType"]="GameDefensiveStats";check(analytics::teamPage(doubleStat,14,0,1,"RegularSeason")["rows"].size()==2,"Incompatible position stats entered the weekly grade table");
  auto missingOwner=raw;missingOwner["records"]["4:0"]["v"]["IsRetired"]=true;rejected=false;try{analytics::live::dataset(missingOwner);}catch(...){rejected=true;}check(rejected,"Retired/missing user used a previous Dynasty");
  snapshotReady=true;nativeReady=true;check(!available()&&!open(),"Absent native Dynasty admitted a stale snapshot");leave();check(!opened&&source==-1,"Leaving native Analytics retained source ownership");
  for(const auto& binding:bindings){check(!(binding.vt==0xbacd668&&(binding.offset==0x68||binding.offset==0x60))&&!(binding.vt==0xbaa78b8&&binding.offset==0x70)&&binding.vt!=0xbac8c40,"Analytics competes with a shared COT hook");}
  std::map<uintptr_t,std::vector<unsigned char>> nativeBytes;constexpr uintptr_t image=0x140000000;
  for(const auto& gate:analyticsGates)nativeBytes[image+gate.rva]={gate.bytes,gate.bytes+32};
  for(const auto& binding:bindings){auto address=image+binding.rva;std::vector<unsigned char> bytes(8);memcpy(bytes.data(),&address,8);nativeBytes[image+binding.vt+binding.offset]=bytes;}
  auto query=[&](uintptr_t at,void* out,size_t n){auto found=nativeBytes.find(at);if(found==nativeBytes.end()||found->second.size()!=n)return false;memcpy(out,found->second.data(),n);return true;};validateContract(image,query);check(true,"Pristine native contract rejected");
  for(auto& [at,bytes]:nativeBytes)for(size_t offset=0;offset<bytes.size();++offset){bytes[offset]^=1;bool refused=false;try{validateContract(image,query);}catch(...){refused=true;}check(refused,"Changed instruction or foreign slot passed Analytics gates");bytes[offset]^=1;}
  rejected=false;try{validateContract(image,[](uintptr_t,void*,size_t){return false;});}catch(...){rejected=true;}check(rejected,"Unreadable native code passed Analytics gates");
  check(inflight==0,"UI callbacks leaked ownership drain counts");
  if(argc==2){std::ifstream file(argv[1]);Json captured;file>>captured;const auto before=captured;auto start=std::chrono::steady_clock::now();auto dataset=analytics::live::dataset(captured);auto snapshot=prepareSnapshot(dataset);const auto end=std::chrono::steady_clock::now();check(captured==before,"Real read-only fixture changed");size_t graded=0;for(const auto& row:dataset.at("rows"))if(row.at("analytics").at("grade").is_number())++graded;check(graded>0,"Actual Dynasty fixture has no graded rows");std::cout<<"Actual native adapter: "<<dataset.at("roster").size()<<" roster, "<<dataset.at("rows").size()<<" game records, "<<graded<<" grades, "<<dataset.at("issues").size()<<" rejected linkages; prepare="<<std::chrono::duration<double,std::milli>(end-start).count()<<" ms (offline fixture, not live menu).\n";}
  std::cout<<"PASS: "<<checks<<" analytics integration checks; no game hooks installed.\n";return 0;
 }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
