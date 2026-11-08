// Exercise the production preparation/selection paths against private objects.
// No game process, injected preview or installed plugin is opened or changed.
#include "../Source/NativeRuntime.cpp"
#include <cassert>
#include <iostream>
#include <sstream>
namespace {
void fresh(){
    startingCrowds.clear();sharedFanExits={};sharedFanBasis={};sharedFanExitsSeeded=false;
    activeLoss=arrivalMissing=0;replayLoss=-1;replayHomeLoss=replayAwayLoss=resumeHomeLoss=resumeAwayLoss=-1;
    replaySharedBudget=true;resumeLegacyBudget=false;sharedFanBudgetApplied=false;shownQuarter=1;shownScore0=shownScore1=0;
}
void add(uint64_t key,unsigned total,unsigned cadets,unsigned visitors,unsigned band=0,unsigned empty=0){
    StartingCrowd b;b.layoutKey=key;b.geometry=static_cast<A>(key);b.parts=1;b.hidden.assign(total+band+empty,0);
    std::vector<unsigned char> raw(b.hidden.size()*10);
    for(unsigned i=0;i<raw.size()/10;++i){raw[i*10]=(i>=cadets&&i<cadets+visitors)?1:0;raw[i*10+6]=i<cadets?9:i>=total&&i<total+band?1:0;if(i>=total+band)b.hidden[i]=1;}
    b.agents=attendance::agents::decode(raw,b.hidden.size());attendance::agents::protectStudents(b.agents,true,false);
    b.selection.policy=attendance::seats::Policy::Scattered;b.supporters=attendance::visitors::groups(b.selection,b.hidden,1,b.agents);
    assert(b.supporters.verified);startingCrowds.emplace(static_cast<A>(key),std::move(b));
}
struct Count {uint64_t original=0,visible=0,protectedFans=0,band=0,gone=0;};
Count count(){
    Count c;std::set<uint64_t> seen;
    for(const auto& item:startingCrowds){const auto& b=item.second;if(!seen.insert(b.layoutKey).second)continue;
        for(unsigned i=0;i<b.hidden.size();++i){
            if(b.agents.protectedBand(i)){++c.band;assert(!selectedSupporter(b,i,2,0));continue;}
            if(b.hidden[i])continue;++c.original;
            bool missing=selectedSupporter(b,i,2,0),gone=selectedSupporter(b,i,0,0);c.visible+=!missing;c.gone+=gone;
            if(b.agents.protectedFan(i)){++c.protectedFans;assert(!missing&&!gone);}
        }
    }return c;
}
}
int main(){
    fresh();add(1,2000,2000,0,105);add(2,18000,0,2000,0,2000);
    // A duplicate streamed renderer of the same native layout adds no budget.
    startingCrowds.emplace(3,startingCrowds.at(2));
    uint64_t previous=0;
    for(int absent=50;absent>=0;--absent){arrivalMissing=absent;prepareVisitorLosses();auto c=count();
        assert(sharedFanBudgetApplied&&c.original==20000&&c.protectedFans==2000&&c.band==105);
        assert(c.visible==20000-20000*static_cast<unsigned>(absent)/100&&c.visible>=previous);previous=c.visible;
    }
    // Halftime around already departed supporters, then return only the absentees.
    shownQuarter=2;shownScore0=0;shownScore1=42;activeLoss=20;arrivalMissing=35;prepareVisitorLosses();auto half=count();
    assert(half.visible==13000&&half.gone==4000);
    const auto homeGone=sharedFanExits.home,awayGone=sharedFanExits.away;
    for(int absent=34;absent>=0;--absent){arrivalMissing=absent;prepareVisitorLosses();auto c=count();
        assert(c.gone==4000&&c.visible==20000-(std::max)(4000u,20000u*static_cast<unsigned>(absent)/100));}
    assert(count().visible==16000);
    // A recorded earlier picture does not alter live exit counters.
    replayLoss=35;replayPermanentLoss=20;replayVisitorArrivals=35;
    replayHomeLoss=static_cast<float>(static_cast<double>(homeGone)/18000);replayAwayLoss=static_cast<float>(static_cast<double>(awayGone)/2000);
    replaySharedBudget=true;prepareVisitorLosses();assert(count().visible==half.visible);
    assert(sharedFanExits.home==homeGone&&sharedFanExits.away==awayGone);
    replayLoss=-1;prepareVisitorLosses();assert(count().visible==16000);
    // Fresh-process equivalent: only fixture counts, saved fractions and target.
    fresh();add(1,2000,2000,0,105);add(2,18000,0,2000,0,2000);
    resumeHomeLoss=static_cast<float>(static_cast<double>(homeGone)/18000);resumeAwayLoss=static_cast<float>(static_cast<double>(awayGone)/2000);
    shownQuarter=3;shownScore0=0;shownScore1=42;activeLoss=20;arrivalMissing=35;prepareVisitorLosses();assert(count().visible==half.visible&&count().gone==4000);
    // Preserve v6 temporary accounting while viewing an old clip.
    replayLoss=50;replayHomeLoss=replayAwayLoss=0;replayVisitorArrivals=50;replaySharedBudget=false;prepareVisitorLosses();assert(count().visible==11000);
    replayLoss=-1;prepareVisitorLosses();assert(count().visible==13000);
    // An old Midgame Save completes its current interval under its old policy.
    fresh();add(1,2000,2000,0,105);add(2,18000,0,2000);resumeLegacyBudget=true;arrivalMissing=50;
    prepareVisitorLosses();assert(count().visible==11000&&!sharedFanBudgetApplied);
    arrivalMissing=0;prepareVisitorLosses();assert(!resumeLegacyBudget&&sharedFanBudgetApplied&&count().visible==20000);
    activeLoss=100;prepareVisitorLosses();assert(count().visible==2000&&count().protectedFans==2000);
    fresh();std::cout<<"PASS: production multi-layout arrivals/halftime/returns/departures, band and native-hole exclusion, duplicate-layout isolation, live/replay separation, fresh-process-equivalent resume and legacy-policy transition; self-process fixtures only\n";
}
