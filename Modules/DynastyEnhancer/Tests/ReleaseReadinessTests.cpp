#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "../Source/EnhancementOptionsRuntime.h"
#include "../Source/RecruitResumeRuntime.h"
#include <cassert>
#include <iostream>
namespace recruitingfog::resume {
struct CacheTestAccess {
 static void staleWaits(){
    Cache c;auto now=GetTickCount64();
    for(int i=0;i<64;++i)c.waitingReports[std::to_string(i)]={now-9000,now-9000};
    NotesProspect p;p.identity="new";assert(c.queue(p)&&c.waitingReports.size()==1);
    for(int i=0;i<63;++i){p.identity=std::to_string(i);assert(c.queue(p));}
    p.identity="limit";assert(!c.queue(p)&&c.waitingReports.size()==64);
 }
 static void fail(Cache& c){c.backgroundFailed=true;c.waitingSince=GetTickCount64();}
 static void reportFailure(Cache& c){c.reportFailed=true;}
 static void delayed(Cache& c,const std::string& key){auto now=GetTickCount64();c.waitingReports[key]={now-9000,now};}
 static void ready(Cache& c,const NotesProspect& p){auto e=std::make_shared<Cache::ReportEdition>();e->league="42";e->season=2026;e->reports[p.identity]=evidence::obtain(Json(),p,evidence::Context{},std::nullopt);Cache::prepareView(*e,p.identity);std::atomic_store(&c.reportEdition,std::shared_ptr<const Cache::ReportEdition>(e));c.reportFailed=false;}
};
}
int main(){
 {using namespace recruitingfog::evidence;Json saved={{"claims",{{"speed",{{"text","Scouting observation: This remains a tentative read. He needs extra steps before reaching his stride."},{"quality",2}}}}},{"displayOrder",{"speed"}},{"flavor","Background note: his coach describes a quiet player."}};const auto original=saved.dump();const auto text=render(saved,25,false);assert(text.find("He needs extra steps")!=std::string::npos);assert(text.find("Fictional")==std::string::npos);assert(text.find("Scouting observation:")==std::string::npos);assert(saved.dump()==original);assert(render(saved,25,true).find("extra steps")==std::string::npos);}

 {using namespace recruitingfog::evidence;const std::string body="He needs extra steps before reaching his stride.";for(int quality=0;quality<3;++quality){assert(presentNote("Scouting observation: "+body,quality)==body);assert(presentNote("Coach's early assessment: This remains a tentative read. "+body,quality)==body);assert(presentNote("So far, he needs extra steps before reaching his stride.",quality)==body);}}
 {std::map<std::string,double> empty;assert(recruitingfog::measurables::estimate("speed",empty,"x",true).value=="Unavailable");assert(recruitingfog::measurables::estimate("speed",empty,"x",false).value=="?");recruitingfog::resume::NativeEntry e;e.prospect.ratings={{"SpeedRating",90},{"AccelerationRating",80}};recruitingfog::resume::Cache::complete(e,empty);assert(empty.at("speed")==90&&empty.at("acceleration")==80);}
 using namespace dynasty::live;using namespace recruitingfog;using namespace recruitingfog::resume;
 assert(decodedIntegerArray(0x80000000u,32,INT32_MIN)==0);
 assert(decodedIntegerArray(0x80000032u,32,INT32_MIN)==50);
 assert(decodedIntegerArray(0,32,INT32_MIN)==INT32_MIN);
 assert(decodedIntegerArray(UINT32_MAX,32,INT32_MIN)==INT32_MAX);
 bool rejected=false;try{decodedIntegerArray(1,16,0);}catch(...){rejected=true;}assert(rejected);
 enhancements::currentOptions=enhancements::Options{}.packed();enhancements::currentOptionsLeague=42;
 NotesProspect p;p.identity="Alex|Bell|Town|LA|123";p.position="TE";p.progress=80;p.observations={{"Catching",83,83,true},{"Run Block",61,61,true}};
 CacheTestAccess::staleWaits();
 Cache c;bool loading=false;
 auto first=c.compose({},p,123,"Alex","Bell",loading);assert(first.find("Preparing background")!=std::string::npos&&loading);
 CacheTestAccess::fail(c);auto failed=c.compose({},p,123,"Alex","Bell",loading);assert(failed.find("temporarily unavailable")!=std::string::npos&&!loading);
 auto scope=std::make_shared<Published>();scope->data.league="42";scope->data.season=2026;scope->backgroundReady=false;
 auto pending=c.compose(scope,p,123,"Alex","Bell",loading);assert(loading&&pending.find("Preparing scouting observations")!=std::string::npos&&pending.find("More scouting is needed")==std::string::npos);
 const auto key="ui|42|"+p.identity;CacheTestAccess::delayed(c,key);auto delayed=c.compose(scope,p,123,"Alex","Bell",loading);assert(!loading&&delayed.find("delayed")!=std::string::npos);
 CacheTestAccess::reportFailure(c);auto reportFail=c.compose(scope,p,123,"Alex","Bell",loading);assert(!loading&&reportFail.find("Reopen Notes to retry")!=std::string::npos);
 auto evidence=p;evidence.identity=key;CacheTestAccess::ready(c,evidence);auto ready=c.compose(scope,p,123,"Alex","Bell",loading);assert(!loading&&ready.find("Preparing scouting")==std::string::npos&&ready.find("Fictional scouting")==std::string::npos&&ready.find("SCOUTING NOTES")!=std::string::npos);
 enhancements::Options blind;blind.recruitingScale=enhancements::GradeScale::Blind;enhancements::currentOptions=blind.packed();auto hidden=c.compose(scope,p,123,"Alex","Bell",loading);assert(hidden.find("evaluation is hidden")==std::string::npos&&hidden.find("SCOUTING NOTES")!=std::string::npos&&!loading);
 auto athletic=p;athletic.measurementIdentity="42|athlete";athletic.observations.push_back({"Speed",90,90,true});athletic.observations.push_back({"Acceleration",80,80,true});
 enhancements::Options independent;independent.recruiting=false;independent.measurables=true;independent.recruitingScale=enhancements::GradeScale::Blind;enhancements::currentOptions=independent.packed();auto measured=c.compose(scope,athletic,123,"Alex","Bell",loading);assert(measured.find("40-Yard Dash")!=std::string::npos);
 independent.recruiting=true;enhancements::currentOptions=independent.packed();auto blindMeasured=c.compose(scope,athletic,123,"Alex","Bell",loading);assert(blindMeasured.find("40-Yard Dash")==std::string::npos&&blindMeasured.find("evaluation is hidden")==std::string::npos);
 auto other=std::make_shared<Published>();other->data.league="43";other->data.season=2026;auto isolated=c.compose(other,p,123,"Alex","Bell",loading);assert(isolated.find("Fictional scouting")==std::string::npos);
 std::cout<<"PASS typed integer arrays, rejected unknown encodings, Notes pending/failure/timeout/recovery, independent revealed observations, Blind and Dynasty isolation\n";
}
