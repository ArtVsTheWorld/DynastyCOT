#include "../Source/RecruitResumeNotes.h"
#include "../Source/ScoutingEvidence.h"
#include <cassert>
#include <iostream>
#include <set>
#include <chrono>
using namespace recruitingfog;
int main(){
 using namespace resume;
 {Prospect p;p.identity="legacy-proof";p.position="QB";p.stars=5;auto stored=generate(p).publicData;stored["heading"]="Previous high school season (modeled)";stored["background"]="Shared a role in his modeled previous season.";const auto canonical=stored.dump();const auto shown=renderResume(stored);assert(shown.find("MODELED")==std::string::npos&&shown.find("modeled")==std::string::npos&&shown.find("Shared reps last season.")!=std::string::npos);assert(stored.dump()==canonical);assert(evidence::presentNote("Fictional scouting reports; assessments may be mistaken.").empty());}
 std::vector<std::string> positions={"QB","HB","WR","TE","LT","DT","CB","K","P"};
 auto started=std::chrono::steady_clock::now();
 for(const auto& pos:positions)for(int stars=1;stars<=5;++stars){unsigned reserve=0,starts=0;std::vector<int> yards;
  for(int i=0;i<600;++i){Prospect p;p.identity="quality-"+pos+std::to_string(stars)+"-"+std::to_string(i);p.position=pos;p.stars=stars;
   p.ratings={{"OverallRating",55.+stars*6},{"AwarenessRating",55.+stars*6}};auto result=generate(p);validate(result.publicData);assert(generate(p).publicData==result.publicData);
   const auto& v=result.publicData;reserve+=v.at("starts")==0;starts+=v.at("starts").get<int>();yards.push_back(v.at("stats").value(pos=="QB"?"passingYards":pos=="HB"?"rushingYards":"receivingYards",0));
  }std::sort(yards.begin(),yards.end());if(stars==5)assert(reserve<12);
  std::cout<<"production|"<<pos<<'|'<<stars<<'|'<<reserve<<'|'<<starts/600.<<'|'<<yards[60]<<'|'<<yards[300]<<'|'<<yards[540]<<'\n';
 }
 unsigned roles=0;for(int i=0;i<1000;++i){Prospect p;p.identity="ath-"+std::to_string(i);p.position="QB";p.stars=5;p.height=75;p.alternatePositions={"CB"};p.ratings={{"OverallRating",80},{"SpeedRating",85},{"ManCoverageRating",75},{"ZoneCoverageRating",73},{"CatchingRating",70},{"PlayRecognitionRating",70}};
  auto v=generate(p).publicData;validate(v);if(v.contains("modeledSecondary")){++roles;assert(v["modeledSecondary"]["position"]=="DB");assert(renderResume(v,false).find("Also played")==std::string::npos);assert(renderResume(v).find("Also played: DB")!=std::string::npos);}
  p.ratings.erase("ManCoverageRating");assert(!generate(p).publicData.contains("modeledSecondary"));p.alternatePositions.clear();assert(!generate(p).publicData.contains("modeledSecondary"));
 }assert(roles>450&&roles<700);std::cout<<"twoWay|eligible1000|"<<roles<<'\n';
 evidence::Context context;std::map<std::string,unsigned> repeated;std::set<std::string> different;unsigned displayed=0;
 for(int i=0;i<1000;++i){NotesProspect p;p.identity="notes-"+std::to_string(i);p.position=positions[i%positions.size()];p.progress=100;
  p.observations={{"Speed",85,85,true},{"Acceleration",80,80,true},{"Catching",75,75,true},{"Awareness",65,65,true},{"Pass Block",80,80,true},{"Tackle",70,70,true}};
  auto v=evidence::obtain(nullptr,p,context,.6);const auto immutable=v;auto text=evidence::render(v,100,false);assert(evidence::obtain(v,p,context,.6)==immutable);
  std::istringstream lines(text);std::string line;std::set<std::string> local;while(std::getline(lines,line))if(!line.empty()&&line!="SCOUTING NOTES"){assert(local.insert(line).second);++displayed;++repeated[line];different.insert(line);}
  auto old=v;old["athleticBackground"]="Background lead: the report mentions qualifying for a regional track meet. No official result is attached.";old["flavor"]="Background lead: he reportedly competes in track. No verified meet result is included in this scouting file.";
  const auto canonical=old;const auto transfer=evidence::render(old,100,false,true);assert(transfer.find("regional track meet")==std::string::npos&&transfer.find("runs track")==std::string::npos&&old==canonical);
 }
 unsigned worst=0;for(const auto& item:repeated)worst=(std::max)(worst,item.second);
 std::vector<std::pair<unsigned,std::string>> frequent;for(const auto& item:repeated)frequent.emplace_back(item.second,item.first);std::sort(frequent.rbegin(),frequent.rend());for(unsigned i=0;i<5&&i<frequent.size();++i)std::cout<<"frequent|"<<frequent[i].first<<'|'<<frequent[i].second<<'\n';
 std::cout<<"notes|reports1000|distinct "<<different.size()<<"|lines "<<displayed<<"|maxFrequency "<<worst<<'\n';
 std::cout<<"workerBatchMs|"<<std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count()<<'\n';
 std::cout<<"PASS quality/position distributions, exact consistency, stable modeled roles, missing-input gates, compact varied Notes, immutable legacy transfer filtering; no game access.\n";
}
