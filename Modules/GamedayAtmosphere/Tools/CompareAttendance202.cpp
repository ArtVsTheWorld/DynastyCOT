// Compile once against the archived 2.0.1 header and once against Source.
// Controlled inputs isolate this pass; no game/save writes or future outcomes.
#include "StartingAttendance.h"
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <iostream>
#include <stdexcept>
using namespace attendance::turnout;
void emit(std::ostream& out,const std::string& label,Seed s,int minutes=720,bool senior=false,bool local=false,bool rivalry=false){
    Launch l{s.home.id,s.away.id,s.stadium,s.capacity,s.ea,minutes,0,rivalry,false};
#ifndef DGA_BEFORE202
    l.seniorKnown=true;l.senior=senior;
    if(local)s.home.state=s.away.state=43;
#else
    (void)senior;(void)local;
#endif
    auto r=calculate(s,l);
    if(!r.applied)throw std::runtime_error("Invalid comparison fixture");
    out<<label<<','<<s.home.id<<','<<s.away.id<<','<<s.capacity<<','<<minutes<<','<<r.percent<<','<<r.kickoff<<','<<r.percent*r.kickoff*.01f;
#ifndef DGA_BEFORE202
    out<<','<<r.breakout<<','<<r.eventMultiplier;
#else
    out<<",0,1";
#endif
    out<<'\n';
}
int main(int argc,char** argv){
    if(argc!=2&&argc!=4)return 2;std::ofstream out(argv[argc-1]);
    out<<"case,home,away,capacity,local_minutes,peak_pct,kickoff_share,kickoff_fullness,breakout_points,event_multiplier\n";
    Seed s;s.home={1153,50,70,6,3,3,3,6,6,6,6,.5f};s.away={1193,60,80,5,3,3,3,6,6,6,6,.4f};s.stadium=s.homeStadium=553;s.capacity=60000;s.schools=138;s.week=8;s.year=2026;s.regular=true;s.ea=10000;
    if(argc==2){
        for(int seats:{15000,40000,100000}){s.capacity=seats;emit(out,"Ordinary daytime "+std::to_string(seats),s);auto late=s;late.home.id=1218;emit(out,"Severe-late daytime "+std::to_string(seats),late);}
        s.capacity=15000;s.home={1233,120,125,2,1,5,3,11,12,12,8,.05f};s.away.rank=70;
        emit(out,"Low-support ordinary",s);emit(out,"Low-support night",s,1200);emit(out,"Low-support in-state",s,720,false,true);emit(out,"Low-support senior day",s,720,true);
        auto opener=s;opener.home.wins=opener.home.losses=opener.home.homeGames=0;emit(out,"Low-support home opener",opener);emit(out,"Low-support opener night",opener,1200);
        auto breakout=s;breakout.home.wins=10;breakout.home.losses=1;breakout.home.apRank=18;
        emit(out,"Low-support 10-1 AP18 daytime",breakout);emit(out,"Low-support 10-1 AP18 night senior",breakout,1200,true);
        s.home.id=1108;s.away.id=1166;emit(out,"Low-support academy rivalry",s,720,false,false,true);
    }else{
        std::ifstream teamsFile(argv[1]),schedule(argv[2]);std::string line;std::vector<Team> cohort;std::map<int,Team> teams;
        while(std::getline(teamsFile,line)){std::replace(line.begin(),line.end(),',',' ');std::istringstream in(line);Team t;
            if(!(in>>t.id>>t.rank>>t.toughest>>t.prestige>>t.wins>>t.losses>>t.homeGames>>t.atmosphere>>t.tradition>>t.brand>>t.conference>>t.apRank))return 3;
            teams[t.id]=t;if(!fcs(t.id))cohort.push_back(t);}
        if(!assignStandings(cohort))return 4;for(auto& t:cohort)teams[t.id]=t;
        struct Game {int row,home,away,week,year,ordinal,rival,neutral,precip,weather,time;};std::vector<Game> games;std::map<int,int> lastHome;
        std::getline(schedule,line);while(std::getline(schedule,line)){std::replace(line.begin(),line.end(),',',' ');std::istringstream in(line);Game g;
            if(!(in>>g.row>>g.home>>g.away>>g.week>>g.year>>g.ordinal>>g.rival>>g.neutral>>g.precip>>g.weather>>g.time))return 5;
            if(!g.neutral){games.push_back(g);lastHome[g.home]=(std::max)(lastHome[g.home],g.week);}}
        // Shared 60k comparison venue isolates demand changes. This does not
        // claim actual capacities, school locations, kickoff timezones or future form.
        for(auto g:games){if(!teams.count(g.home)||!teams.count(g.away))return 6;
            s.home=teams[g.home];s.away=teams[g.away];s.home.homeGames=g.ordinal;s.capacity=60000;s.schools=static_cast<int>(cohort.size());s.week=g.week;
            for(int time:{720,1200})emit(out,"row"+std::to_string(g.row)+(time==720?" day":" night"),s,time,g.week==lastHome[g.home],false,g.rival!=0);}
    }
    if(!out)return 7;std::cout<<"PASS: matched production turnout comparisons exported.\n";
}
