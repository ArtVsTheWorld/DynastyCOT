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
 static void savedReport(Cache& c,const std::string& identity,const Json& report){
    auto edition=std::make_shared<Cache::ReportEdition>();
    edition->league="42";edition->season=2026;edition->reports[identity]=report;
    Cache::prepareView(*edition,identity);
    std::atomic_store(&c.reportEdition,std::shared_ptr<const Cache::ReportEdition>(edition));
 }
};
}

static void fullAthCard(){
    using namespace recruitingfog;
    using namespace recruitingfog::resume;
    using namespace recruitingfog::measurables;
    enhancements::Options selected;selected.measurables=true;
    enhancements::currentOptions=selected.packed();enhancements::currentOptionsLeague=42;
    Prospect profile;profile.position="FB";profile.height=74;profile.stars=5;profile.state="NorthCarolina";profile.town="Winston-Salem";
    profile.alternatePositions={"CB"};profile.ratings={{"OverallRating",88},{"SpeedRating",85},
        {"ManCoverageRating",75},{"ZoneCoverageRating",73},{"CatchingRating",70},{"PlayRecognitionRating",70}};
    Json background;
    for(int i=0;i<100;++i){
        profile.identity="full-ath-"+std::to_string(i);
        background=generate(profile).publicData;
        if(background.contains("modeledSecondary"))break;
    }
    assert(background.contains("modeledSecondary"));
    background["background"]="Split reps against a stronger schedule.";
    const auto original=background;
    const std::string flavor="His coach says the transition to a new spot went smoothly.";
    const std::vector<std::string> observations={
        "He can finish when his approach and angle are sound.",
        "The chase loses efficiency when the runner changes course.",
        "Handles his release well before the first defender makes contact.",
        "Needs more time to find his man when the front changes late."
    };
    Json claims=Json::object(),order=Json::array();
    for(size_t i=0;i<observations.size();++i){
        const auto key="observation-"+std::to_string(i);
        claims[key]={{"text",observations[i]}};
        order.push_back(key);
    }
    const Json journal={{"version",1},{"claims",claims},{"displayOrder",order},
        {"athleticBackground",flavor+"\nAnother saved line."},{"flavor","A second saved Background field."},{"athleticBackgroundChecked",true}};
    auto published=std::make_shared<Published>();
    published->data.league="42";published->data.season=2026;
    NativeEntry entry;entry.nativeId=123;entry.first="Alex";entry.last="Bell";entry.identity=profile.identity;
    entry.highSchool=true;entry.prospect.position="ATH";entry.prospect.alternatePositions={"FB"};entry.saved=background;
    entry.prospect.ratings={{"SpeedRating",88},{"AccelerationRating",85},{"StrengthRating",65},
        {"ChangeOfDirectionRating",80},{"AgilityRating",83},{"JumpingRating",79}};
    published->data.entries.emplace(123,entry);
    NotesProspect notes;notes.identity="Alex|Bell|Winston-Salem|NC|123";notes.measurementIdentity=notes.identity;
    notes.position="ATH";notes.progress=100;
    notes.observations={{"Agility",83,83,true}};
    const auto subject=seedIdentity(42,notes.measurementIdentity);
    const auto lift=estimate(Event::Bench,revealedInputs({{"strength",65}}),subject);
    Cache cache;CacheTestAccess::savedReport(cache,entry.identity,journal);
    bool loading=false;
    const auto text=cache.compose(published,notes,123,"Alex","Bell",loading);
    assert(text.find("Split reps")==std::string::npos&&text.find("schedule")==std::string::npos);
    assert(text.find("Rushing: ")!=std::string::npos&&text.find("Receiving: ")!=std::string::npos&&text.find("Run blocking: ")!=std::string::npos);
    assert(text.find("Also played: DB | Defense: ")!=std::string::npos&&text.find("\n\nSCOUTING NOTES\n- ")!=std::string::npos);
    assert(text.find("\n\nATHLETIC MEASURABLES\n40-Yard Dash: ")!=std::string::npos);
    assert(text.find("\n\nBACKGROUND\n- "+flavor+"\n\nATHLETIC MEASURABLES")!=std::string::npos);
    assert(text.find("Bench / Squat: "+lift.value+"\n")!=std::string::npos);
    assert(text.substr(text.find("ATHLETIC MEASURABLES")).find('(')==std::string::npos);
    const auto lastLine=text.substr(text.find_last_of('\n')+1);
    const auto jump=estimate(Event::Vertical,revealedInputs({{"jumping",79}}),subject);
    assert(lastLine=="Vertical Jump: "+jump.value);
    assert(text.size()<1500&&std::count(text.begin(),text.end(),'\n')+1==22);
    assert(published->data.entries.at(123).saved==original);
    auto incomplete=notes;incomplete.progress=99;
    const auto partial=cache.compose(published,incomplete,123,"Alex","Bell",loading);
    assert(partial.find("20-Yard Shuttle")!=std::string::npos&&partial.find("Bench / Squat")==std::string::npos);
    assert(partial.find("\n\nBACKGROUND")==std::string::npos);
    auto& candidate=published->data.entries.at(123);
    candidate.highSchool=false;
    candidate.juniorCollege=true;
    assert(cache.compose(published,notes,123,"Alex","Bell",loading).find("\n\nBACKGROUND\n- "+flavor)!=std::string::npos);
    candidate.juniorCollege=false;
    candidate.saved={{"gpa",3.2}};
    candidate.actual={{"schools",Json::array()},{"rows",Json::array()}};
    assert(cache.compose(published,notes,123,"Alex","Bell",loading).find("\n\nBACKGROUND")==std::string::npos);
    candidate=entry;
    published->data.unavailable.emplace(125,std::make_pair("Alex","Bell"));
    published->data.unavailableSchoolRecruits.insert(125);
    assert(cache.compose(published,notes,125,"Alex","Bell",loading).find("\n\nBACKGROUND\n- ")!=std::string::npos);
    published->data.unavailableSchoolRecruits.clear();
    published->data.unavailableTransfers.insert(125);
    assert(cache.compose(published,notes,125,"Alex","Bell",loading).find("\n\nBACKGROUND")==std::string::npos);
    const auto otherRecruit=cache.compose(published,notes,123,"Alex","Jones",loading);
    assert(otherRecruit.find("Bench / Squat")==std::string::npos&&otherRecruit.find("\n\nBACKGROUND")==std::string::npos);
    selected.recruitingScale=enhancements::GradeScale::Blind;
    enhancements::currentOptions=selected.packed();
    const auto blind=cache.compose(published,notes,123,"Alex","Bell",loading);
    assert(blind.find("SCOUTING NOTES\n- ")!=std::string::npos&&blind.find("ATHLETIC MEASURABLES")==std::string::npos&&blind.find("\n\nBACKGROUND\n- "+flavor)!=std::string::npos);
    const auto concealed=renderResume(background,false);
    assert(concealed.find("Also played")==std::string::npos&&concealed.find("Shared reps")==std::string::npos);
    std::cout<<"FULL ATH CARD BEGIN\n"<<text<<"\nFULL ATH CARD END\n";
}

int main(){
 fullAthCard();
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
