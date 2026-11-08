#include "../Source/StartingAttendance.h"
#include <cassert>
#include <sstream>
#include <iostream>
using namespace attendance;
audience::Program custom(){audience::Program p;p.id=1801;p.row=29;p.year=0;p.custom=p.identityKnown=true;p.prestige=3;p.atmosphere=8;p.ap=71;return p;}
int main(){
    auto p=custom();p.history={audience::Season{12,0,0},audience::Season{12,0,0},audience::Season{12,0,0}};
    assert(audience::recent(p)==-1);auto seed=audience::foundation(p);assert(seed.peers&&seed.fans>20000&&seed.fans<35000);
    audience::Book book;audience::Foundation base;assert(book.resolve(-1649480587,p,base));double start=audience::demand(base,p);
    for(int n=0;n<20;++n){audience::Foundation repeat;assert(book.resolve(-1649480587,p,repeat));assert(audience::demand(repeat,p)==start);}assert(book.revision==1);
    p.year=1;p.history[0]={8,4,0};assert(std::abs(audience::recent(p)-2.f/3)<.0001f);
    p.year=2;p.history[1]={4,8,0};assert(std::abs(audience::recent(p)-(.5f*2/3+.3f/3)/.8f)<.0001f);
    p.year=3;p.history[2]={0,12,0};assert(std::abs(audience::recent(p)-(.5f*2/3+.3f/3))<.0001f);
    auto better=p;better.prestige=7;better.atmosphere=3;better.ap=12;better.history={audience::Season{11,2,0},audience::Season{10,3,0},audience::Season{9,4,0}};
    assert(audience::demand(base,better)>audience::demand(base,p));auto bad=p;bad.prestige=1;bad.atmosphere=11;bad.ap=138;bad.history={audience::Season{1,11,0},audience::Season{2,10,0},audience::Season{2,10,0}};
    assert(audience::demand(base,bad)<start);
    std::stringstream output;book.write(output);audience::Book restored;assert(restored.read(output));audience::Foundation after;assert(restored.resolve(-1649480587,better,after));assert(std::abs(audience::demand(after,better)-audience::demand(base,better))<.01);
    // Different dynasty and an earlier origin cannot inherit a future custom seed.
    audience::Foundation other;assert(restored.resolve(123,better,other)&&other.fans!=base.fans);
    auto rewind=custom();assert(restored.resolve(123,rewind,other));assert(std::abs(other.fans-base.fans)<.01);
    for(const char* text:{"bad", "DGA-AUDIENCE-1 8193", "DGA-AUDIENCE-1 1\n0 1 1 0 0 0 100 .5 .5 .5 .5", "DGA-AUDIENCE-1 0 junk"}){std::stringstream input(text);audience::Book corrupt;assert(!corrupt.read(input));}
    p=custom();p.id=1102;auto replaced=audience::foundation(p);assert(replaced.peers&&replaced.fans<35000);p.custom=false;auto native=audience::foundation(p);assert(!native.peers&&native.fans>90000); // IsTeamBuilder wins over an inherited ID.
    // A late first install must not erase earned success for a native school
    // whose original calibration had no historical seasons.
    p=custom();p.id=1233;p.custom=false;p.history={};auto early=audience::foundation(p);
    p.year=3;p.history={audience::Season{10,3,0},audience::Season{9,4,0},audience::Season{8,5,0}};auto late=audience::foundation(p);
    assert(early.fans==late.fans&&early.reference==late.reference&&audience::demand(early,p)==audience::demand(late,p));
    auto unavailable=custom();unavailable.ap=0;assert(audience::features(unavailable)[3]<0);
    auto unknownAP=audience::foundation(unavailable);assert(unknownAP.reference[3]>=0&&audience::demand(unknownAP,unavailable)>0);
    audience::Book withoutAP;assert(withoutAP.resolve(19,unavailable,unknownAP));std::stringstream missingAP;withoutAP.write(missingAP);audience::Book withoutAPReload;assert(withoutAPReload.read(missingAP));
    turnout::Seed s;s.home={1801,71,70,3,0,0,0,8,9,11,8,.416f};s.home.apRank=71;s.away={1251,138,138,0,0,0,1,12,12,12,12,.1f};s.away.apRank=138;
    s.home.audienceFans=25000;s.away.audienceFans=14000;s.stadium=s.homeStadium=650;s.capacity=65000;s.ea=44839;s.schools=138;s.regular=true;s.week=0;s.year=0;
    turnout::Launch l{1801,1251,650,65000,44839,1170,0,false,false};auto r=turnout::calculateAudience(s,l);assert(r.applied&&r.audienceModel&&r.openerBoost>=.1f&&r.nightBoost>=.05f);
    assert(r.kickoff==turnout::calculate(s,l).kickoff&&r.fillPercent==50);
    // Same fans and fixture: different capacity changes fullness, not demand.
    auto small=s;auto sl=l;small.capacity=sl.capacity=35000;auto sr=turnout::calculateAudience(small,sl);assert(sr.attendance==r.attendance&&sr.percent>r.percent);
    small.capacity=sl.capacity=100000;auto larger=turnout::calculateAudience(small,sl);assert(larger.attendance==r.attendance&&larger.percent<r.percent);
    auto fuller=turnout::calculateAudience(s,l,true);assert(std::abs(fuller.percent-r.percent-8)<.001&&fuller.kickoff==r.kickoff+7);
    l.rivalry=true;l.rivalryEvent=139;assert(turnout::calculateAudience(s,l).percent==100);
    l.rivalry=false;l.rivalryEvent=-1;l.neutral=true;auto neutral=turnout::calculateAudience(s,l);assert(neutral.applied&&neutral.audienceFans==19500);
    l.neutral=false;s.home.audienceFans=4000;assert(turnout::calculateAudience(s,l).percent<15); // No phantom percentage-floor audience.
    s.resumed=true;assert(!turnout::calculateAudience(s,l).applied);s.resumed=false;s.regular=false;assert(!turnout::calculateAudience(s,l).applied);
    std::cout<<"PASS: fixed audience foundations, 40/30/20/10 response, no repeated compounding, real completed-season recency, TeamBuilder provenance, restart/rewind/dynasty isolation, corrupt storage, venue invariance, preserved boosts/arrival rules/Fuller/showcase and unsupported-context guards.\n";
}
