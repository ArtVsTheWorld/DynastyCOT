// Read numeric exports only; no save parser, game access or modification.
// Stage-independent support/ranks and actual schedule pairings, with controlled
// day/night brackets. Central turnout omits the unverified venue hash variation.
#include "../Source/StartingAttendance.h"
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <iostream>
using namespace attendance::turnout;
int main(int argc,char** argv){
    if(argc!=4)return 2;std::ifstream teamFile(argv[1]),schedule(argv[2]);std::ofstream out(argv[3]);std::string line;std::vector<Team> cohort;std::map<int,Team> teams;
    while(std::getline(teamFile,line)){std::replace(line.begin(),line.end(),',',' ');std::istringstream in(line);Team t;
        if(!(in>>t.id>>t.rank>>t.toughest>>t.prestige>>t.wins>>t.losses>>t.homeGames>>t.atmosphere>>t.tradition>>t.brand>>t.conference>>t.apRank))return 3;
        teams[t.id]=t;if(!fcs(t.id))cohort.push_back(t);}
    if(!assignStandings(cohort))return 4;for(const auto& t:cohort)teams[t.id]=t;
    out<<"game_row,home,away,week,rivalry,neutral_inferred,condition,central_peak_pct,variation_low,variation_high,kickoff_share_of_peak,kickoff_capacity_pct\n";
    unsigned cases=0;std::getline(schedule,line);
    while(std::getline(schedule,line)){std::replace(line.begin(),line.end(),',',' ');std::istringstream in(line);
        int row=0,home=0,away=0,week=0,year=0,ordinal=0,rival=0,neutral=0,precip=0,weather=0,savedTime=0;
        if(!(in>>row>>home>>away>>week>>year>>ordinal>>rival>>neutral>>precip>>weather>>savedTime)||!teams.count(home)||!teams.count(away))return 5;
        for(int condition=0;condition<3;++condition){if(condition==2&&precip<0)continue;
            Seed seed;seed.home=teams[home];seed.away=teams[away];seed.home.homeGames=ordinal;seed.capacity=100000;seed.ea=100000;seed.stadium=seed.homeStadium=0;seed.schools=static_cast<int>(cohort.size());seed.week=week;seed.year=year;seed.regular=true;
            Launch launch{home,away,0,100000,100000,condition==1?1200:720,condition==2?precip:0,rival!=0,neutral!=0};
            auto r=calculate(seed,launch);if(!r.applied)return 6;
            float raw=(r.demand+r.momentum+r.appeal+r.adjustmentLimit+r.breakout)*r.eventMultiplier+r.weather;
            float floor=r.weather<0?20.f:25.f,central=(std::clamp)(raw,floor,100.f),low=(std::clamp)(raw-3,floor,100.f),high=(std::clamp)(raw+3,floor,100.f);
            int kickoff=openingPercent(r.standing,home,seed.home.rank,seed.away.rank,neutral!=0,r.night,rival!=0,ordinal==0,launch.precipitation,false,55,45,3,central);
            out<<row<<','<<home<<','<<away<<','<<week<<','<<rival<<','<<neutral<<','<<(condition==0?"clear_day":condition==1?"clear_night":"saved_weather_day")<<','<<central<<','<<low<<','<<high<<','<<kickoff<<','<<central*kickoff*.01f<<'\n';++cases;
        }
    }
    if(!teamFile.eof()||!schedule.eof()||!out)return 7;std::cout<<"PASS: "<<cases<<" scheduled launch cases; percentages only, controlled day/night, unknown venue variation bracketed.\n";
}
