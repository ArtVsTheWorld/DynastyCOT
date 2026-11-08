#include "../Source/CrowdPopulation.h"
#include "../Source/LineupAtmosphere.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    assert(dense::layers(0,1).multiplier()==1&&dense::layers(61888,NAN).multiplier()==1);
    assert(dense::layers(6060,.9f).multiplier()==1&&dense::layers(61888,.7f).multiplier()==1);
    assert(std::fabs(dense::layers(29271,.88f).multiplier()-1.2f)<.0001f);
    assert(std::fabs(dense::layers(61888,.95f).multiplier()-1.32f)<.0001f);
    for(uint64_t seats:{6060ull,20000ull,30000ull,50000ull,61888ull}){float last=1,threshold=.55f;
        for(int i=0;i<=1000;++i){auto d=dense::layers(seats,i/1000.f);assert(d.multiplier()>=last-.00001f&&d.multiplier()<=1.321f&&d.multiplier()-last<.02f&&d.swellMinimum()<=threshold+.00001f);last=d.multiplier();threshold=d.swellMinimum();}}
    sustained::Intent base;base.active=true;base.reason=sustained::Reason::Lineup;base.intensity=700;base.continuous=true;base.size=1;base.lineup=11;base.lineupCheer=7;
    auto mid=population::layers(base,29271,.88f),big=population::layers(base,61888,.95f);
    float old=700*population::presence(29271,.88f)/std::sqrt(.88f);
    assert(std::fabs(mid.intensity/old-1.2f)<.0001f&&big.intensity>mid.intensity&&big.size==2);
    float largeOld=700*population::presence(61888,.95f)/std::sqrt(.95f);
    assert(std::fabs(big.intensity/largeOld-1.32f)<.0001f&&big.intensity<=1000);
    base.intensity=1000;assert(population::layers(base,61888,1).intensity==1000);base.intensity=700;
    assert(mid.lineup==11&&mid.lineupCheer==7&&mid.liveCheer==0&&mid.resultCheer==0);
    base.reason=sustained::Reason::Result;base.size=3;base.majorLive=true;
    assert(population::layers(base,30000,.85f).size==3);base.majorLive=false;assert(population::layers(base,30000,.85f).size==2);
    base.active=false;assert(population::layers(base,61888,1).intensity==base.intensity);
    // Lower supporting-swell hurdle changes neither admission nor cue IDs.
    core::Settings c;energy::Play p;p.game={2,14,0,1,400,720};p.game.rank0=6;p.game.rank1=11;p.down=1;p.offense=0;p.toGoal=70;
    lineup::Model model;model.observe(c,p,1,1,1000,true,.88f,true,nullptr,29271);model.observe(c,p,1,1,6000,true,.88f,true,nullptr,29271);
    sustained::Intent idle;idle.active=true;idle.match=1;idle.epoch=1;idle.updated=6000;
    auto admitted=model.merge(idle,c,p,6000,.88f);assert(admitted.reason==sustained::Reason::Lineup&&admitted.supporting);
    auto serial=model.serial;model.observe(c,p,1,1,6500,true,.88f,true,nullptr,29271);assert(model.serial==serial);
    model.suspend();assert(model.positionsForSound==0);
    model.observe(c,p,1,1,7000,false,.95f,true,nullptr,61888);assert(!model.merge(idle,c,p,7000,.95f).supporting);
    model.reset();model.observe(c,p,1,1,8000,true,.4f,true,nullptr,61888);assert(!model.layeredActive());
    std::puts("PASS: smooth medium/very-large density tiers, sparse invariance, sound-only lift, major-only size upgrade, unchanged IDs/admission and suspension cleanup.");
}
