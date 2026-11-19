#include "../Source/ScoutingEvidence.h"
#include "../Source/RecruitResumeNotes.h"
#include <iostream>
#include <limits>
#include <random>
#include <set>
#include <sstream>

using namespace recruitingfog;
using namespace recruitingfog::evidence;

#define CHECK(condition) do { if(!(condition)) throw std::runtime_error(#condition); } while(false)

static NotesProspect recruit(const char* position,std::vector<NotesObservation> observations){
 NotesProspect p;
 p.position=position;
 p.progress=100;
 p.observations=std::move(observations);
 // Use a max-pipeline identity whose reports are accurate for this fixture,
 // so the test can distinguish selection from the existing reliability rolls.
 for(unsigned i=0;;++i){
  p.identity="42|focused-selection|"+std::to_string(i);
  if(std::all_of(p.observations.begin(),p.observations.end(),[&](const auto& o){return notesHash(p.identity+"|"+family(p,o)+"|report-v1")%10000<9200;}))return p;
 }
}

static void verifySelection(){
 const auto context=Context{};
 auto tackle=recruit("RT",{{"pass block finesse",95,95,true},{"pass block power",53,53,true},
  {"run block",74,74,true},{"awareness",76,76,true},{"strength",76,76,true}});
 const auto prospect=tackle;
 tackle.progress=50;
 tackle.observations.erase(tackle.observations.begin()+1);
 const auto early=obtain(Json(),tackle,context,1.);
 const auto later=obtain(early,prospect,context,1.);
 const auto top=displayedNotes(later,100).front();
 CHECK(later.at("version")==3&&top.key=="pass block finesse"&&top.pairedWith=="pass block power");
 CHECK(top.text=="I trust his feet against speed. The bull rush is the one I'd send help for.");
 CHECK(render(early,50,false)==render(later,50,false));
 CHECK(later.at("claims").at("pass block finesse")==early.at("claims").at("pass block finesse"));
 CHECK(render(later,100,false)==render(Json::parse(later.dump()),100,false));
 CHECK(obtain(later,prospect,context,0.)==later);
 CHECK(displayedNotes(later,24).empty()&&render(later,100,true).find(top.text)==std::string::npos);
 auto priorTackle=later;
 priorTackle["version"]=2;
 priorTackle["flavor"]="An existing note from the staff.";
 CHECK(obtain(priorTackle,prospect,context,0.)==priorTackle);
 CHECK(displayedNotes(priorTackle,100).front().text==top.text);

 auto tightEnd=recruit("TE",{{"run block",52,52,true},{"catching",96,96,true},{"short route running",74,74,true}});
 auto report=obtain(Json(),tightEnd,context,1.);
 const auto wording=render(report,100,false);
 CHECK(wording.find("Flex him out")!=std::string::npos);
 CHECK(wording.find("trust him")!=std::string::npos||wording.find("gotta-have-it")!=std::string::npos);
 CHECK(wording.find_first_of("0123456789")==std::string::npos&&wording.find("modeled")==std::string::npos);

 auto receiver=recruit("WR",{{"catching",95,95,true},{"catch in traffic",54,54,true},{"short route running",74,74,true},
  {"medium route running",76,76,true},{"run block",75,75,true},{"awareness",75,75,true}});
 const auto first=obtain(Json(),receiver,context,1.);
 auto notes=displayedNotes(first,100);
 CHECK(notes.front().key=="catching"&&notes.front().pairedWith=="catch in traffic");
 std::set<std::string_view> topics;
 for(const auto& note:notes)CHECK(topics.insert(noteTopic(note.key)).second);
 receiver.observations[0]={"catching",74,74,true};
 receiver.observations[1]={"catch in traffic",75,75,true};
 receiver.observations[2]={"short route running",95,95,true};
 receiver.observations[3]={"medium route running",53,53,true};
 notes=displayedNotes(obtain(Json(),receiver,context,1.),100);
 CHECK(notes.front().key=="short route running"&&notes.front().pairedWith=="medium route running");

 // Wide or invalid ranges cannot trigger emphatic or combined evaluations.
 receiver.observations={{"catching",95,95,true},{"catch in traffic",55,82,false},
  {"speed",std::numeric_limits<double>::quiet_NaN(),60,false}};
 report=obtain(Json(),receiver,context,1.);
 CHECK(report.at("claims").size()==1&&displayedNotes(report,100).front().pairedWith.empty());

 // Keep old order and wording even when a more important new trait arrives.
 auto legacy=first;
 legacy["version"]=1;
 legacy.erase("position");
 for(auto& claim:legacy["claims"])claim.erase("emphasis");
 legacy["displayOrder"]={"run block","short route running","medium route running","awareness receiver","catch in traffic","catching"};
 const auto oldClaims=legacy.at("claims");
 const auto oldOrder=legacy.at("displayOrder");
 const auto legacyText=render(legacy,100,false);
 receiver.observations.push_back({"deep route running",95,95,true});
 const auto expanded=obtain(legacy,receiver,context,1.);
 for(auto it=oldClaims.begin();it!=oldClaims.end();++it)CHECK(expanded.at("claims").at(it.key())==it.value());
 CHECK(std::equal(oldOrder.begin(),oldOrder.end(),expanded.at("displayOrder").begin()));
 CHECK(expanded.at("displayOrder").back()=="deep route running"&&render(expanded,100,false)==legacyText);
 legacy.erase("displayOrder");
 CHECK(render(obtain(legacy,receiver,context,1.),100,false)==render(legacy,100,false));
 CHECK(naturalNotesText("Previous High School Season (modeled)")=="Previous High School Season");
 for(const char* pos:{"QB","CB","LG"}){
  auto other=recruit(pos,{{"speed",95,95,true}});
  CHECK(obtain(Json(),other,context,1.).at("version")==3);
 }
}

static void verifyRolesAndBackground(){
 auto sample=[](const NotesProspect& p){return obtain(Json(),p,Context{},1.);};
 auto qb=recruit("QB",{{"short accuracy",96,96,true},{"medium accuracy",78,78,true},{"deep accuracy",53,53,true}});
 auto qbNotes=displayedNotes(sample(qb),100);
 CHECK(qbNotes.size()==1&&qbNotes[0].pairedWith=="deep accuracy"&&qbNotes[0].text.find("quick game")!=std::string::npos);

 auto snapper=recruit("LS",{{"long snapping",94,94,true},{"kicking power",95,95,true},{"kick power",95,95,true},{"agility",90,90,true}});
 auto snapperReport=sample(snapper);
 CHECK(snapperReport.at("claims").size()==1&&snapperReport.at("claims").contains("long snap"));
 CHECK(!hasNewEvidence(snapperReport,snapper));
 snapper.observations.push_back({"awareness",72,77,false});
 CHECK(hasNewEvidence(snapperReport,snapper));

 auto athlete=recruit("ATH",{{"Man Coverage",96,96,true},{"Zone Coverage",94,94,true},{"Power Moves",97,97,true}});
 auto athleteReport=sample(athlete);
 auto athleteNotes=displayedNotes(athleteReport,100);
 CHECK(athleteNotes.size()==1&&athleteNotes[0].text.find("reps in the secondary")!=std::string::npos);
 CHECK(!hasNewEvidence(athleteReport,athlete));
 athlete.activeTransfer=true;
 CHECK(sample(athlete).at("flavor")=="");
 CHECK(render(athleteReport,100,false).find(athleteNotes[0].text)!=std::string::npos);

 const auto talkingPoints=Json{{"role","Regular starter"},{"games",10},{"stats",{{"specialTeamsSnaps",75},{"badSnaps",0}}}};
 std::set<std::string> snaps;
 for(unsigned seed=0;seed<12;++seed)snaps.insert(resume::seasonBackground("LS",talkingPoints,seed));
 CHECK(snaps.size()==6);
 auto noSample=talkingPoints;
 noSample["games"]=0;
 noSample["stats"]=Json::object();
 CHECK(resume::seasonBackground("LS",noSample,0).find("No game work")==0);
 resume::Prospect prospect;
 prospect.identity="42|background-stability";prospect.position="WR";prospect.stars=4;prospect.name="Fixture";
 auto profile=resume::generate(prospect).publicData;
 const auto text=profile.at("background");
 CHECK(resume::generate(prospect).publicData==profile);
 profile["background"]="This is the saved version of the season entry.";
 resume::addResumeDetails(profile,prospect);
 CHECK(profile.at("background")=="This is the saved version of the season entry.");
 CHECK(resume::renderResume(profile).find("(MODELED)")==std::string::npos);
 CHECK(text!=profile.at("background"));
}

static void variety(unsigned seed){
 std::mt19937 rng(seed);
 for(std::string role:{"QB","HB","WR","TE","LT","DT","MIKE","CB","FS","K"})for(int i=0;i<120;++i){
  NotesProspect p;
  p.identity="42|variety|"+role+"|"+std::to_string(i);
  p.position=role;
  std::vector<NotesObservation> early,full;
  std::set<std::string> added;
  for(const auto& entry:resume::prose){
   std::string key=entry.attribute;
   if(key.rfind("awareness",0)==0)key="awareness";
   if(!added.insert(key).second||!relevant(role,family(p,NotesObservation{key})))continue;
   const double rating=std::clamp(65+(i%6)*4+int(rng()%29)-14,45,99);
   full.push_back({key,rating,rating,true});
   if(rng()%100<62){const double width=rng()%5;early.push_back({key,rating-width,std::min(100.,rating+width),width==0});}
  }
  Json saved;
  for(int stage:{50,100}){
   p.progress=stage;
   p.observations=stage==50?early:full;
   saved=obtain(saved,p,Context{},.6);
   Json notes=Json::array();
   std::istringstream lines(render(saved,stage,false));
   std::string line;
   std::getline(lines,line);
   for(const auto& note:displayedNotes(saved,stage)){
    if(!std::getline(lines,line)||line!=note.text)throw std::runtime_error("Variety fixture diverged from rendered Notes");
    notes.push_back({note.key,line,note.pairedWith});
   }
   std::cout<<Json::array({role,i,stage,notes}).dump()<<'\n';
  }
 }
}

int main(int argc,char** argv) try {
 verifySelection();
 verifyRolesAndBackground();
 if(argc==3&&std::string_view(argv[1])=="--variety")variety(std::stoul(argv[2]));
 else std::cout<<"PASS focused Notes ranking, reveal gates, combined wording, deterministic reload and legacy preservation\n";
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
