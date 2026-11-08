// Own policy with generic examples. Optional private cohort CSV contains numeric
// inputs only: id,rank,toughest,prestige,wins,losses,homeGames,atmosphere,tradition,brand,conference,apRank (actual MediaPoll_CurrentRank; do not substitute TeamRank).
#include "../Source/StartingAttendance.h"
#include <cstdio>
#include <fstream>
#include <sstream>
using namespace attendance::turnout;
float eaPercent(const Seed& s,bool rivalry,bool forceFull){
    if(forceFull)return 100;
    float p=20.f+.6f*(s.schools-s.home.toughest+60-s.home.rank)+.33f*(60-(std::min)(s.schools,s.away.rank))+(rivalry?30.f:0.f);
    if(s.home.homeGames==0&&p<100)p+=.6f*(100-p);
    return (std::clamp)(p,30.f,100.f);
}
float previous(const Seed& s,int minutes,int rain,bool rivalry,bool neutral){
    auto demand=[&](const Team& t){return .8f*(45+50*std::pow(1.f-static_cast<float>(t.toughest-1)/(s.schools-1),.65f))+.2f*(45+5*t.prestige);};
    auto form=[](const Team& t){int n=t.wins+t.losses;return n?(std::clamp)((t.wins/static_cast<float>(n)-.5f)*20*(std::min)(1.f,n/6.f),-10.f,8.f):0.f;};
    auto appeal=[](const Team& t){return fcs(t.id)?-8.f:top(t.rank,25)?2+6*(26-t.rank)/25.f:t.rank>100?-4.f:0.f;};
    float base=demand(s.home),moment=form(s.home),event=appeal(s.away)+(rivalry?10.f:0.f)+(s.home.homeGames==0?4.f:0.f);
    if(neutral){base=.5f*(base+demand(s.away));moment=.5f*(moment+form(s.away));event=.5f*(appeal(s.home)+appeal(s.away))+(rivalry?10.f:0.f);}
    constexpr float weather[]={0,5,9,14};return (std::clamp)(base+moment+(std::clamp)(event,-8.f,15.f)+(minutes>=1140?7.f:0.f)-(s.roof>=1&&s.roof<=3?0:weather[rain])+static_cast<int>(fixtureHash(s,neutral)%7)-3,30.f,100.f);
}
void example(const char* name,Seed s,int time=720,int rain=0,bool rivalry=false,bool neutral=false,bool forceFull=false){
    float ea=eaPercent(s,rivalry,forceFull);s.ea=static_cast<int>(s.capacity*ea*.01f);
    Launch l{s.home.id,s.away.id,s.stadium,s.capacity,s.ea,time,rain,rivalry,neutral};auto d=calculate(s,l);
    float old=s.regular?previous(s,time,rain,rivalry,neutral):ea;
    int oldOpening=s.regular?(!neutral&&lateArrivalSchool(s.home.id)&&s.home.id!=1218?(time>=1140?88:78):(time>=1140?97:90)):60;
    std::printf("%s,%.1f,%.1f,%.1f,%.1f,%d,%.1f,%d,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n",name,ea,old,d.percent,old*oldOpening*.01f,d.kickoff,d.percent*d.kickoff*.01f,d.fillPercent,d.standing*100,d.demand,d.momentum,d.appeal,d.nightBonus,d.weather);
}
int main(int argc,char** argv){
    if(argc>1){
        std::ifstream file(argv[1]);std::string line;std::vector<Team> teams;
        while(std::getline(file,line)){std::replace(line.begin(),line.end(),',',' ');std::istringstream in(line);Team t;
            if(in>>t.id>>t.rank>>t.toughest>>t.prestige>>t.wins>>t.losses>>t.homeGames>>t.atmosphere>>t.tradition>>t.brand>>t.conference>>t.apRank)teams.push_back(t);}
        if(!assignStandings(teams)){std::fputs("Invalid FBS cohort\n",stderr);return 2;}
        std::vector<float> peaks,old,ea;for(const auto& home:teams)for(const auto& away:teams)if(home.id!=away.id){
            Seed s;s.home=home;s.away=away;s.schools=static_cast<int>(teams.size());s.stadium=s.homeStadium=553;s.capacity=60000;s.regular=true;s.week=8;s.year=2026;s.ea=40000;
            Launch l{home.id,away.id,553,60000,40000,720,0,false,false};auto r=calculate(s,l);if(!r.applied)return 3;
            peaks.push_back(r.percent);old.push_back(previous(s,720,0,false,false));ea.push_back(eaPercent(s,false,false));}
        auto report=[](const char* name,std::vector<float> values){std::sort(values.begin(),values.end());double total=0;unsigned full=0,low=0;for(float v:values){total+=v;if(v>=99.99f)++full;if(v<40)++low;}
            std::printf("%s,count=%zu,mean=%.2f,min=%.2f,p10=%.2f,p25=%.2f,median=%.2f,p75=%.2f,max=%.2f,full_pct=%.2f,below40_pct=%.2f\n",name,values.size(),total/values.size(),values.front(),values[values.size()/10],values[values.size()/4],values[values.size()/2],values[values.size()*3/4],values.back(),100.*full/values.size(),100.*low/values.size());};
        report("EA",ea);report("DGA181",old);report("DGA-current",peaks);return 0;
    }
    std::puts("Example,EA peak pct,DGA181 peak pct,DGA183 peak pct,DGA181 kickoff capacity pct,DGA183 kickoff peak pct,DGA183 kickoff capacity pct,Arrivals finish Q1 pct,Support percentile,Demand,Season,Matchup,Night,Weather");
    Seed s;s.home={1153,50,70,6,3,3,3,6,6,6,6,.5f};s.away={1193,60,80,5,3,3,3,6,6,6,6,.4f};s.stadium=s.homeStadium=553;s.capacity=60000;s.schools=138;s.week=8;s.year=2026;s.regular=true;
    example("Middle-support ordinary",s);example("Same matchup - night",s,1200);example("Same matchup - heavy rain",s,720,3);
    auto rival=s;rival.away.rank=10;example("Middle-support ranked rivalry",rival,720,0,true);
    auto weak=s;weak.home={1164,120,125,3,1,6,4,12,12,12,8,.05f};weak.away.rank=115;example("Struggling bottom-five-percent support",weak);
    auto weakest=weak;weakest.home.rank=135;weakest.home.toughest=138;weakest.home.prestige=2;weakest.home.wins=0;weakest.home.losses=6;weakest.home.standing=0;weakest.away.rank=137;example("Weakest support",weakest);
    auto elite=s;elite.home={1104,3,1,10,6,0,3,0,0,0,0,.95f};elite.away.rank=55;example("Elite support ordinary",elite);
    auto late=elite;late.home.id=1218;example("Utah severe-late arrival",late);
    auto opener=elite;opener.home.wins=opener.home.losses=opener.home.homeGames=0;opener.home.standing=1;opener.away.rank=8;example("Exceptional Top10 opener rivalry night",opener,1200,0,true);
}
