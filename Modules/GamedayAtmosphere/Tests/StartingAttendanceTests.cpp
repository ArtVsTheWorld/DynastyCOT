#include "../Source/StartingAttendance.h"
#include "../Source/ArrivalRules.h"
#include <cassert>
#include <cstdio>
using namespace attendance::turnout;
bool near(float a,float b){return std::fabs(a-b)<.001f;}
Seed sample(){Seed s;s.home={1153,50,70,6,3,3,3,6,6,6,6,.5f};s.away={1193,60,80,5,3,3,3,6,6,6,6,.4f};s.stadium=s.homeStadium=553;s.capacity=60000;s.schools=138;s.week=8;s.year=2026;s.ea=45000;s.regular=true;return s;}
Launch launch(const Seed& s){return {s.home.id,s.away.id,s.stadium,s.capacity,s.ea,720,0,false,false};}
int main(){
    assert(lateArrivalSchool(1223));
    for(int id:{1212,1216}){
      auto school=sample();school.home.id=id;auto launchOptions=launch(school);auto ordinary=calculate(school,launchOptions);
      assert(ordinary.applied&&ordinary.hardToFillPenalty>8&&ordinary.percent<85&&lateArrivalSchool(id)&&ordinary.fillPercent==75);
      school.home.standing=1;school.home.rank=school.home.apRank=1;school.home.wins=10;school.home.losses=0;school.home.homeGames=0;
      school.away.rank=school.away.apRank=1;launchOptions.minutes=1200;launchOptions.seniorKnown=launchOptions.senior=true;launchOptions.rivalry=true;
      auto exceptional=calculate(school,launchOptions);assert(exceptional.percent==100); // Soft discount, not a cap.
      launchOptions.rivalryEvent=139;auto showcase=calculate(school,launchOptions);assert(showcase.percent==100&&showcase.kickoff>=70);
      launchOptions.neutral=true;assert(calculate(school,launchOptions).hardToFillPenalty==0);
      school.regular=false;assert(!calculate(school,launchOptions).applied);
    }
    Seed s=sample();Launch l=launch(s);auto base=calculate(s,l);assert(base.applied&&near(base.demand,87.86f)&&base.kickoff==static_cast<int>(std::lround(55+accessBonus(s.capacity)))&&base.fillPercent==50);
    assert(near(support(s.home,138),.35f*.5f+.25f*.6f+.20f*.5f+.10f*.5f+.05f*.5f));
    for(int member=0;member<4;++member){auto t=s.home;int* fields[]={&t.atmosphere,&t.tradition,&t.brand,&t.conference};*fields[member]=0;assert(support(t,138)>support(s.home,138));*fields[member]=12;assert(support(t,138)<support(s.home,138));}
    auto ranked=s.home;ranked.apRank=1;assert(support(ranked,138)>support(s.home,138));ranked.rank=1;assert(near(support(ranked,138),support(s.home,138)+.05f));
    ranked=s.home;ranked.prestige=10;assert(near(support(ranked,138)-support(s.home,138),.10f));
    std::vector<Team> cohort;for(int i=0;i<138;++i)cohort.push_back(Team{200+i,138-i,138-i,0,0,0,1,12-i*12/137,12-i*12/137,12-i*12/137,12-i*12/137});
    for(int i=0;i<138;++i)cohort[i].apRank=138-i;
    assert(assignStandings(cohort)&&near(cohort.front().standing,0)&&near(cohort.back().standing,1));
    auto reversed=cohort;std::reverse(reversed.begin(),reversed.end());assert(assignStandings(reversed));for(int i=0;i<138;++i)assert(near(cohort[i].standing,reversed[137-i].standing));
    auto tie=cohort;for(auto& t:tie){t.rank=t.apRank=69;t.atmosphere=t.tradition=t.brand=t.conference=6;}assert(assignStandings(tie));for(auto t:tie)assert(near(t.standing,.5f));
    auto invalid=cohort;invalid[0].brand=13;assert(!assignStandings(invalid));invalid=cohort;invalid[1].id=invalid[0].id;assert(!assignStandings(invalid));
    invalid=cohort;invalid[0].prestige=11;assert(!assignStandings(invalid));invalid=cohort;invalid[0].apRank=-1;assert(!assignStandings(invalid));
    auto unranked=cohort;for(auto& t:unranked)t.apRank=0;assert(assignStandings(unranked));
    float last=0;for(int i=0;i<=1000;++i){float d=demandCurve(i/1000.f);assert(d>=last&&d>=25&&d<109);last=d;}assert(demandCurve(.8f)==100);
    l.minutes=1139;assert(!calculate(s,l).night);l.minutes=1140;auto night=calculate(s,l);assert(night.night&&night.nightBoost>=.05f&&night.nightBoost<=.15f&&night.percent>=base.percent&&night.kickoff==base.kickoff+5);l.minutes=720;
    for(int rain=1;rain<=3;++rain){l.precipitation=rain;auto wet=calculate(s,l);assert(near(base.percent-wet.percent,rain==1?5.f:rain==2?9.f:14.f));assert(wet.kickoff>=35&&wet.kickoff<base.kickoff);}
    s.roof=1;assert(near(calculate(s,l).percent,base.percent)&&calculate(s,l).kickoff==base.kickoff);s.roof=0;l.precipitation=0;
    l.rivalry=true;assert(near(calculate(s,l).percent-base.percent,6));l.rivalry=false;
    auto winning=s;winning.home.wins=6;winning.home.losses=0;assert(calculate(winning,l).momentum==6);
    auto losing=s;losing.home.wins=0;losing.home.losses=6;assert(calculate(losing,l).momentum==-6);losing.home.losses=1;assert(calculate(losing,l).momentum==-1);
    auto fcsSeed=s;fcsSeed.away.id=1500;fcsSeed.away.rank=fcsSeed.away.toughest=255;l=launch(fcsSeed);assert(calculate(fcsSeed,l).applied&&calculate(fcsSeed,l).appeal==-10&&calculate(fcsSeed,l).kickoff>=40);
    s=sample();l=launch(s);l.neutral=true;l.rivalry=true;auto neutral=calculate(s,l);std::swap(s.home,s.away);l.home=s.home.id;l.away=s.away.id;auto reverse=calculate(s,l);assert(near(neutral.percent,reverse.percent)&&neutral.attendance==reverse.attendance&&neutral.kickoff==reverse.kickoff);
    for(int rank:{1,25,50,138})for(float standing:{0.f,.05f,.5f,.95f,1.f})for(int record:{0,3,6})for(int rain:{0,1,2,3})for(int time:{720,1140})for(int home:{1153,1218}){
        s=sample();s.home.rank=rank;s.home.standing=standing;s.home.id=home;s.home.wins=record;s.home.losses=6-record;l=launch(s);l.precipitation=rain;l.minutes=time;
        auto a=calculate(s,l),b=calculate(s,l);assert(a.applied&&a.percent>=(rain?20:25)&&a.percent<=100&&a.attendance<=l.capacity&&a.attendance==b.attendance&&a.variation>=-3&&a.variation<=3);
        assert(a.kickoff>=(rain?35:40)&&a.kickoff<=80);if(time==720&&home==1218&&rain==0)assert(a.kickoff==static_cast<int>(std::lround(45+accessBonus(s.capacity))));
        if(rank>25&&time==720&&rain==0)assert(a.kickoff<=70);
    }
    assert(openingPercent(1,1153,1,2,false,true,true,true,0,false)==70);
    assert(openingPercent(0,1218,135,137,false,false,false,false,3,false)==37);
    assert(openingPercent(0,1218,135,137,true,false,false,false,0,false)==50);
    assert(openingPercent(.5f,1153,50,60,false,false,false,false,0,false,50)==50);
    assert(openingPercent(.5f,1153,50,60,false,false,false,false,0,false,60)==60);
    assert(openingPercent(.5f,1153,25,40,false,false,true,false,0,false,55,45,0)==55);
    s=sample();l=launch(s);for(int reason=0;reason<8;++reason){auto bad=s;auto wrong=l;if(reason==0)bad.regular=false;if(reason==1)bad.resumed=true;if(reason==2)wrong.minutes=1440;if(reason==3)wrong.precipitation=5;if(reason==4)bad.home.brand=13;if(reason==5)bad.home.standing=-1;if(reason==6)wrong.away++;if(reason==7)wrong.ea++;auto r=calculate(bad,wrong);assert(!r.applied&&r.attendance==wrong.ea);}
    Seeds cache;Seed found;cache.remember(s,1000);assert(cache.find(l,1100,found));auto serial=found.serial;assert(cache.find(l,1101,found)&&found.serial==serial);assert(!cache.find(l,121001,found));cache.remember(s,130000);assert(cache.find(l,130010,found)&&found.serial!=serial);cache.clear();assert(!cache.find(l,130011,found));
    attendance::State game;game.startingAttendance=true;game.plannedKickoff=37;game.plannedFill=75;attendance::arrivals::Settings arrivals;assert(attendance::arrivals::openingPercent(arrivals,game)==37&&attendance::arrivals::finishFraction(arrivals,game)==.75f);
    game.playoff=true;game.bowl=37;assert(attendance::arrivals::openingPercent(arrivals,game)>=70&&attendance::arrivals::finishFraction(arrivals,game)==.2f);game.bowl=47;assert(attendance::arrivals::openingPercent(arrivals,game)>=80);
    std::puts("PASS: actual-grade weighting, empirical demand distribution, FBS-only cohort/ties/order, rank input, bounded modifiers, Utah, opening/weather/cap limits, arrivals, neutral symmetry and protected/fallback/stale contexts.");
}
