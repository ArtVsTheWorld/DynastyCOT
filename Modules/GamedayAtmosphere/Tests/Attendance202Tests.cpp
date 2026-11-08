#include "../Source/StartingAttendance.h"
#include "../Source/ArrivalRules.h"
#include "../Source/LiveReactions.h"
#include "../Source/DefensiveMotion.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
turnout::Seed fixture(){turnout::Seed s;s.home={1233,120,120,2,1,5,3,11,12,12,8,.05f};s.away={1150,70,80,3,3,3,2,8,8,9,8,.25f};s.capacity=15000;s.stadium=s.homeStadium=100;s.schools=138;s.week=11;s.year=2026;s.regular=true;s.ea=10000;return s;}
turnout::Launch launch(const turnout::Seed& s){return {s.home.id,s.away.id,s.stadium,s.capacity,s.ea,720,0,false,false};}
int main(){
 auto s=fixture();auto l=launch(s);auto ordinary=turnout::calculate(s,l);assert(ordinary.applied&&ordinary.breakout==0&&ordinary.eventMultiplier==1&&ordinary.percent<60);
 for(int capacity:{6060,15000,30000,60000,100000,150000}){
  float bonus=turnout::accessBonus(capacity);assert(bonus>=0&&bonus<=8);if(capacity<=15000)assert(bonus==8);if(capacity>=100000)assert(bonus==0);
  for(int home:{1233,1218}){int base=turnout::openingPercent(.5f,home,70,80,false,false,false,false,0,false);int next=turnout::openingPercent(.5f,home,70,80,false,false,false,false,0,false,55,45,3,50,capacity);assert(next>=base&&next<=base+8);}
 }
 auto opener=s;opener.home.homeGames=0;auto o=turnout::calculate(opener,launch(opener));assert(o.openerBoost>=.10f&&o.openerBoost<=.25f&&o.percent>ordinary.percent&&o.percent<100);
 l.minutes=1200;auto night=turnout::calculate(s,l);assert(night.nightBoost>=.05f&&night.nightBoost<=.15f&&night.percent>ordinary.percent);
 l=launch(s);l.seniorKnown=l.senior=true;auto senior=turnout::calculate(s,l);assert(senior.seniorBoost==.10f&&senior.percent>ordinary.percent);l.seniorKnown=false;assert(turnout::calculate(s,l).seniorBoost==0);
 s.home.state=s.away.state=10;l=launch(s);auto local=turnout::calculate(s,l);assert(local.stateBoost>=.05f&&local.stateBoost<=.10f&&local.kickoff==ordinary.kickoff+2);s.away.state=-1;assert(turnout::calculate(s,l).stateBoost==0);
 for(int home:{1100,1108,1166})for(int away:{1100,1108,1166})if(home!=away){s=fixture();s.home.id=home;s.away.id=away;auto a=turnout::calculate(s,launch(s));assert(a.academyBoost>=.05f&&a.academyBoost<=.15f);}
 s=fixture();s.home.wins=10;s.home.losses=1;s.home.apRank=18;s.scheduleKnown=s.lastHome=true;l=launch(s);l.minutes=1200;auto breakout=turnout::calculate(s,l);assert(breakout.breakout>20&&breakout.percent>=80&&breakout.percent<=100);
 for(int year=2026;year<2126;++year){s.year=year;s.home.homeGames=0;s.home.state=s.away.state=10;l=launch(s);l.minutes=1200;auto a=turnout::calculate(s,l),b=turnout::calculate(s,l);assert(a.attendance==b.attendance&&a.eventMultiplier<=1.4f&&a.percent<=100&&a.kickoff<=80);}
 for(float peak:{.20f,.25f,.40f,.60f,1.f}){State g;g.startingAttendance=true;g.plannedKickoff=55;g.plannedFill=50;g.plannedCapacity=10000;g.plannedPeak=static_cast<int>(peak*10000);g.plannedStadium=100;g.plannedWeek=11;g.plannedYear=2026;g.precipitation=2;assert(peak*(100-lossCeiling(g))*.01f>=.15f-.00001f);}
 // Official results recover the exact reported nine-point-lead case once;
 // an unconfirmed first down or a disabled gate cannot supply motion.
 energy::Outcome r;r.accepted=r.completed=r.firstDown=true;r.sequence=27;r.event=energy::Event::FourthDown;r.side=0;r.yards=5;
 State g;g.phase=4;g.homeScore=26;g.awayScore=17;
 auto conversion=live::completedImpact(r,g,true,0,59);assert(conversion.event==live::Event::FourthConversion);assert(!live::completedImpact(r,g,true,27,59).id);assert(!live::completedImpact(r,g,false,0,59).id);
 r.firstDown=false;assert(!live::completedImpact(r,g,true,0,59).id);
 std::puts("PASS: stable bounded event demand, breakout season, rounded arrival access, senior/location/academy bonuses, departure capacity floor and confirmed fourth-down fallback.");
}
