#include "../Source/SpringTurnout.h"
#include <cassert>
#include <iostream>
using namespace attendance;
int main(){turnout::Seed s;s.spring=true;s.dynasty=9;s.springSchool=100;s.springSide=1;s.away.id=100;s.away.prestigeKnown=true;s.away.audienceFans=100000;s.away.prestige=7;
 turnout::Launch l;l.ea=12483;l.capacity=24877;l.minutes=900;l.precipitation=0;
 auto a=spring::audienceTurnout(s,l);assert(a.applied&&a.percent>=58&&a.percent<=60&&a.attendance>l.ea); // 60% ceiling, not 50% native cap.
 for(int cap:{10000,24877,50000,100000})for(double fans:{4000.,15000.,30000.,70000.,120000.}){
  s.away.audienceFans=fans;l.capacity=cap;auto x=spring::audienceTurnout(s,l);assert(x.applied&&x.attendance<=cap*.60+1);
  assert(x.attendance<=fans+1);
  l.ea=cap/4;assert(spring::audienceTurnout(s,l).attendance==x.attendance);l.ea=cap/2;
  std::cout<<"FIXTURE|"<<cap<<"|"<<fans<<"|"<<x.attendance<<"\n";
 }
 s.away.audienceFans=10000;l.capacity=50000;l.minutes=1200;auto dry=spring::audienceTurnout(s,l).attendance;assert(dry>0);
 l.precipitation=3;assert(spring::audienceTurnout(s,l).attendance<dry);s.roof=1;assert(spring::audienceTurnout(s,l).attendance==dry);
 l.ea=0;assert(!spring::audienceTurnout(s,l).applied);l.ea=25000;s.resumed=true;assert(!spring::audienceTurnout(s,l).applied);s.resumed=false;
 s.away.prestigeKnown=false;assert(!spring::audienceTurnout(s,l).applied);s.away.prestigeKnown=true;l.weatherKnown=false;assert(!spring::audienceTurnout(s,l).applied);
 std::cout<<"PASS spring audience, actual-side, verified-input, empty/resume fallbacks, discount/weather/evening and no supplied-50%-ceiling checks\n";
}
