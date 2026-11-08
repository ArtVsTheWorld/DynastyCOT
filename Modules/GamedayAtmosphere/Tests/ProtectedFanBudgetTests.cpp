#include "../Source/ProtectedFanBudget.h"
#include "../Source/AwayCrowd.h"
#include "../Source/AttendanceTimeline.h"
#include <cassert>
#include <cstdio>
#include <sstream>
using namespace attendance;
int main(){
    using namespace fanbudget;
    std::vector<Layout> layouts={{10,{12000,0,2000,0}},{20,{6000,2000,0,0}}};
    for(int missingPercent=0;missingPercent<=100;++missingPercent){
        auto p=build(layouts,{},missingPercent);assert(p.valid);
        uint64_t target=(std::min)(uint64_t(18000),uint64_t(20000)*missingPercent/100);
        assert(p.missing.total()==target&&p.fans.total()-target>=2000);
        uint64_t h=0,a=0;for(size_t i=0;i<p.layouts.size();++i){const auto& row=p.layouts[i];h+=row.missing.home;a+=row.missing.away;
            assert(row.missing.home<=layouts[i].fans.home-layouts[i].fans.homeProtected&&row.missing.away<=layouts[i].fans.away-layouts[i].fans.awayProtected);}
        assert(h==p.missing.home&&a==p.missing.away);
    }
    auto kickoff=build(layouts,{},50);assert(kickoff.fans.total()-kickoff.missing.total()==10000);
    assert(build(layouts,{},100).fans.total()-build(layouts,{},100).missing.total()==2000);
    // A separate ALL-protected section cannot add fans above the stadium budget.
    std::vector<Layout> academy={{1,{2000,0,2000,0}},{2,{16000,2000,0,0}}};
    auto split=build(academy,{},50);assert(split.valid&&split.missing.total()==10000&&split.layouts[0].missing.total()==0);
    // Halftime is an additional absence only until the TOTAL target is met.
    Hidden gone{3500,500};auto halftime=build(layouts,gone,35);
    assert(halftime.valid&&halftime.missing.total()==7000&&halftime.gone.total()==4000);
    auto returned=build(layouts,gone,0);assert(returned.missing.total()==4000);
    for(size_t i=0;i<halftime.layouts.size();++i){assert(halftime.layouts[i].missing.home>=halftime.layouts[i].gone.home);assert(halftime.layouts[i].missing.away>=halftime.layouts[i].gone.away);}
    // Permanent loss allocation must remain nested across uneven layout sizes.
    std::vector<Layout> odd={{17,{1001,31,900,0}},{3,{731,127,0,20}},{90,{123,1009,0,0}},{6,{512,51,10,0}}};
    std::vector<Selection> prior;
    for(uint64_t h=0;h<=1457;++h){auto p=build(odd,{h,h/2},35);assert(p.valid);
        if(!prior.empty())for(size_t i=0;i<prior.size();++i){assert(p.layouts[i].gone.home>=prior[i].gone.home&&p.layouts[i].gone.away>=prior[i].gone.away);}
        prior=p.layouts;
    }
    // Ordering and native addresses do not determine tie allocation.
    auto swapped=layouts;std::reverse(swapped.begin(),swapped.end());auto other=build(swapped,gone,35);
    for(const auto& row:halftime.layouts)for(const auto& x:other.layouts)if(row.key==x.key)assert(row.missing.home==x.missing.home&&row.gone.home==x.gone.home&&row.missing.away==x.missing.away);
    visitors::Losses exits;uint64_t prevHome=0,prevAway=0;
    for(int percent=0;percent<=100;++percent){exits.advance(18000,2000,percent,0,56,4,2000,0);auto p=build(layouts,{exits.home,exits.away},0);assert(p.valid);
        assert(p.gone.home>=prevHome&&p.gone.away>=prevAway);prevHome=p.gone.home;prevAway=p.gone.away;
        assert(p.gone.total()==(std::min)(uint64_t(18000),uint64_t(20000)*percent/100));}
    // History tags the policy, preserving old filmed/paused temporary crowds.
    State s{3,7,28,1,620,720};s.team0=1100;s.team1=1166;
    timeline::Archive archive;archive.begin(91,s,true);timeline::Point point;point.state=s;point.time=20;point.loss=20;point.temporary=35;point.homeLoss=3500.f/18000;point.awayLoss=.25f;point.originalAwayShare=.1f;
    assert(archive.record(point));std::stringstream disk;timeline::write(disk,archive);assert(disk.str().find("DGA-HISTORY-7")==0);
    timeline::Archive loaded;assert(timeline::read(disk,loaded));timeline::Point saved;assert(loaded.resume(s,saved)&&saved.sharedBudget);
    visitors::Losses restored;restored.restore(18000,2000,2000,0,saved.homeLoss,saved.awayLoss);
    auto resume=build(layouts,{restored.home,restored.away},saved.temporary);assert(resume.valid);
    for(size_t i=0;i<halftime.layouts.size();++i)assert(resume.layouts[i].missing.home==halftime.layouts[i].missing.home&&resume.layouts[i].missing.away==halftime.layouts[i].missing.away);
    auto recording=loaded;recording.active=recording.games.back().id;assert(recording.replay(20,saved)&&saved.sharedBudget);
    std::string old=disk.str(); // stream contents remain available after reading
    old.replace(0,13,"DGA-HISTORY-6");old.erase(old.rfind(' '),old.size()-old.rfind(' ')-1);
    std::stringstream legacy(old);assert(timeline::read(legacy,loaded)&&!loaded.games.front().points.front().sharedBudget);
    auto bad=disk.str();bad[bad.size()-2]='2';std::stringstream corrupt(bad);assert(!timeline::read(corrupt,loaded));
    assert(!build({},gone,50).valid&&!build({{1,{1,0,2,0}}},{},50).valid&&!build({layouts[0],layouts[0]},gone,50).valid);
    assert(!build({{1,{UINT64_MAX,1,0,0}}},{},50).valid);
    std::puts("PASS: exact shared kickoff/halftime/departure budgets across separate protected layouts, mandatory headcount floor, side constraints, monotonic rounding, deterministic ordering and v7/v6 save/replay policy restoration");
}
