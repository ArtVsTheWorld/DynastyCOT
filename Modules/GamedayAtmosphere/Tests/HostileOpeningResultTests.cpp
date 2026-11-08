#include "../Source/HostileOpeningResult.h"
#include "../Source/CrowdPopulation.h"
#include <cassert>
#include <iostream>
using namespace attendance;
int main(){
    energy::Play p;p.game={1,0,0,1,710,720};p.down=1;p.offense=1;
    energy::Outcome result;openingresult::Model m;
    assert(!m.observe(p,p,result,123,1,1,1000,true,true,0,false));
    auto snap=p;p.game.phase=3;assert(!m.observe(p,snap,result,123,1,2,1100,false,true,0,false));
    p.game.phase=4;result.accepted=result.completed=true;result.sequence=2;result.yards=2;
    assert(m.observe(p,snap,result,123,1,2,2000,false,true,0,false));
    assert(!m.observe(p,snap,result,123,1,2,2100,false,true,0,false));
    sustained::Intent out;out.active=true;out.match=123;out.epoch=1;out.updated=2000;out.intensity=750;out.size=1;
    auto positive=m.merge(out,p,2200);assert(positive.resultCheer&&positive.resultType==91&&positive.hostileOpeningResult&&!positive.shake);
    assert(m.merge(out,p,2300).resultCheer==positive.resultCheer);
    auto scaled=population::layers(positive,6060,.85f);assert(scaled.populationSize==3&&scaled.size==1&&scaled.resultType==91);
    auto existing=out;existing.resultCheer=17;existing.resultCheerUntil=4400;assert(m.merge(existing,p,2200).resultCheer==17);
    assert(!m.merge(out,p,4500).resultCheer);m.suspend();assert(!m.merge(out,p,2200).resultCheer);
    for(int reject=0;reject<12;++reject){
        openingresult::Model test;auto call=snap;auto r=result;auto after=p;
        test.observe(call,call,{},123,1,1,1000,true,true,0,false);auto live=call;live.game.phase=3;
        test.observe(live,call,{},123,1,2,1100,false,true,0,false);
        bool allowed=true;int kind=0;bool pass=false;uint64_t sequence=2,now=2000;
        if(reject==0)r.firstDown=true;if(reject==1)r.awayPoints=6;if(reject==2)r.homePoints=6;
        if(reject==3)r.takeaway=true;if(reject==4)r.yards=3;if(reject==5)r.yards=-1000;
        if(reject==6)after.game.neutral=true;if(reject==7)after.resumed=true;if(reject==8)after.simulated=true;
        if(reject==9)sequence=3;if(reject==10)now=50000;if(reject==11)allowed=false;
        assert(!test.observe(after,call,r,123,1,sequence,now,false,allowed,kind,pass));
    }
    for(int kind:{4,5}){
        openingresult::Model test;test.observe(snap,snap,{},123,1,1,1000,true,true,0,false);auto live=snap;live.game.phase=3;
        test.observe(live,snap,{},123,1,2,1100,false,true,3,true);auto r=result;r.yards=-1000;
        assert(test.observe(p,snap,r,123,1,2,2000,false,true,kind,true));
        auto eruption=out;eruption.celebration=1;assert(!test.merge(eruption,p,2200).resultCheer);
    }
    openingresult::Model late;assert(!late.observe(p,snap,result,123,1,2,2000,false,true,4,true));
    std::cout<<"PASS: qualified hostile first away snap, short stops and verified incompletions, largest one-shot selector at small occupancy, result-slot promotion, no new shake/roar size, no repeat/late/resume/sim/neutral/conversion/score/takeaway responses.\n";
}
