#include "../Source/StartingAttendance.h"
#include "../Source/ArrivalRules.h"
#include "../Source/SettingsMenu.h"
#include <cassert>
#include <cstdio>
#include <cwchar>
#include <string>
using namespace attendance;
struct Labels {std::vector<std::string> labels;void text(int,const char* key,const char* value){if(std::string(key)=="display")labels.push_back(value);}void integer(int,const char*,int){}void boolean(int,const char*,bool){}};
int main(){
 const settingsmenu::Option* option=nullptr;for(const auto& o:settingsmenu::Options)if(std::wcscmp(o.key,L"AttendanceProfile")==0)option=&o;
 assert(option&&option->factory==1&&option->kind==settingsmenu::Kind::AttendanceProfile);
 Labels labels;settingsmenu::Choices choices(*option,1);settingsmenu::toggle(labels,*option,choices,1);
 assert(labels.labels==std::vector<std::string>({"Fuller Crowds","Default"}));
 for(int home:{1153,1218})for(float standing:{0.f,.05f,.5f,.8f,1.f})for(int rain:{0,1,2,3})for(bool night:{false,true})for(bool neutral:{false,true}){
  turnout::Seed s;s.home={home,70,70,4,3,3,2,8,8,8,8,standing};s.away={1193,80,80,3,3,3,2,8,8,8,8,.3f};s.regular=true;s.week=7;s.year=2026;s.schools=138;s.stadium=s.homeStadium=5;s.capacity=60000;s.ea=45000;
  turnout::Launch l{home,s.away.id,5,60000,45000,night?1200:720,rain,false,neutral};
  auto d=turnout::calculate(s,l),f=turnout::calculate(s,l,true);
  assert(d.applied&&f.applied&&!d.fuller&&f.fuller&&f.profileBoost==8&&d.profileBoost==0);
  assert(f.percent>=d.percent&&f.percent<=100&&f.percent-d.percent<=8.001f);
  assert(f.kickoff>=d.kickoff&&f.kickoff<=80&&f.fillPercent==d.fillPercent&&f.weather==d.weather);
  assert(f.attendance>=d.attendance&&f.attendance<=l.capacity);
  s.regular=false;assert(!turnout::calculate(s,l,true).applied);
  s.regular=true;s.resumed=true;assert(!turnout::calculate(s,l,true).applied);
 }
 arrivals::Settings d,f;State s{1,0,0,5,720,720};s.team0=1153;s.team1=1193;s.rank0=80;s.rank1=90;
 assert(arrivals::openingPercent(f,s)==arrivals::openingPercent(d,s)); // Unsupported/native-peak fallback stays unchanged.
 s.startingAttendance=true;s.plannedKickoff=43;s.plannedFill=75;
 assert(arrivals::openingPercent(f,s)==43); // A loaded plan always wins over the current menu setting.
 s.bowl=37;s.playoff=true;assert(arrivals::openingPercent(f,s)==arrivals::openingPercent(d,s));
 std::puts("PASS: Fuller above Default; default preserved; bounded peak/kickoff lift across weather/night/neutral/severe schools; no postseason/resume rewrite; saved arrival plans retained.");
}
