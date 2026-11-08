#include "../Source/AwayCrowd.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    using namespace visitors;
    Location akron{41.08,-81.52},columbus{39.96,-83.00},raleigh{35.78,-78.64},berkeley{37.87,-122.27};
    assert(miles(akron,columbus)>100&&miles(akron,columbus)<150);
    assert(miles(raleigh,berkeley)>2300&&miles(raleigh,berkeley)<2600);
    assert(std::abs(miles(raleigh,berkeley)-miles(berkeley,raleigh))<1e-6);
    assert(miles(akron,akron)==0&&!std::isfinite(miles({},akron)));
    {
        Demand d;d.homeFans=d.awayFans=30000;d.home=raleigh;d.away=berkeley;
        auto city=plan(d);assert(city.travel==Travel::City);
        d.homeState=raleigh;d.awayState=berkeley;d.home={};
        auto approximate=plan(d);assert(approximate.travel==Travel::State&&approximate.travelKnown);
        assert(std::abs(approximate.travelFactor-1)<std::abs(city.travelFactor-1));
        d.sameState=true;d.awayState=d.homeState;
        auto local=plan(d);assert(local.travel==Travel::Local&&!local.travelKnown&&!std::isfinite(local.distance)&&local.travelFactor==1.15);
        d.sameState=false;d.homeState={};d.awayState={};
        auto unknown=plan(d);assert(unknown.travel==Travel::Unknown&&unknown.travelFactor==1);
    }
    for(unsigned seed=0;seed<1000;++seed){
        Demand ordinary;ordinary.homeFans=ordinary.awayFans=30000;ordinary.home={35,-85};ordinary.away={37,-81};ordinary.seed=seed;
        auto p=plan(ordinary);assert(p.valid&&p.travelKnown&&p.share>=.03f&&p.share<=.12f);
        auto large=ordinary;large.homeFans=9000;large.awayFans=105000;large.home=akron;large.away=columbus;large.sameState=true;
        assert(plan(large).share>=.50f&&plan(large).share<=.70f);
        auto coast=ordinary;coast.home=raleigh;coast.away=berkeley;assert(plan(coast).share<p.share);
        auto rival=ordinary;rival.rivalry=true;assert(plan(rival).share>p.share);
        auto weak=ordinary;weak.awayFans=100;assert(plan(weak).share==.01f);
        auto neutral=ordinary;neutral.neutral=true;assert(plan(neutral).native&&plan(neutral).share==.5f);
        auto missing=ordinary;missing.home={};assert(plan(missing).valid&&!plan(missing).travelKnown);
        auto bad=ordinary;bad.awayFans=0;assert(!plan(bad).valid);
    }
    std::vector<seats::Position> positions(1024);std::vector<unsigned char> raw(1024*10);
    for(unsigned i=0;i<1024;++i){positions[i]={i&1?35.f:-35.f,static_cast<float>(i/128+1),static_cast<float>(static_cast<int>(i%128)-64),0};raw[i*10+6]=i<24?static_cast<unsigned char>(1+i%8):0;}
    auto tags=agents::decode(raw,1024);
    for(float share:{.01f,.03f,.05f,.07f,.25f,.5f,.60f,.70f}){
        auto x=assign(positions,tags,share,123),y=assign(positions,tags,share,123);assert(x==y);
        unsigned actual=0;for(unsigned i=0;i<x.size();++i){if(tags.protectedBand(i))assert(x[i]==0);else actual+=x[i];}
        assert(actual==static_cast<unsigned>(std::llround(1000*share)));
    }
    // The denominator is people actually attending, including native visitors,
    // never the authored number of seats or a stadium's advertised capacity.
    std::vector<unsigned char> occupied(1024,0);
    for(unsigned i=24;i<424;++i)occupied[i]=1;
    auto sparse=assign(positions,tags,.25f,123,&occupied);
    unsigned attending=0,visitors=0;
    for(unsigned i=0;i<1024;++i)if(occupied[i]){++attending;visitors+=sparse[i];}
    assert(attending==400&&visitors==100);
    for(unsigned i=424;i<1024;++i)assert(sparse[i]==tags.side[i]);
    occupied.pop_back();assert(assign(positions,tags,.25f,123,&occupied).empty());
    assert(assign(positions,tags,.701f,123).empty());
    // More support and a shorter trip increase the same seeded fixture's
    // visitor share smoothly; exceptional takeovers have an explicit ceiling.
    Demand sweep;sweep.homeFans=30000;sweep.awayFans=3000;sweep.home={35,-85};sweep.away={37,-81};
    float last=0;
    for(int fans=3000;fans<=3000000;fans+=3000){sweep.awayFans=fans;auto share=plan(sweep).share;assert(share>=last&&share<=.70f);last=share;}
    assert(last==.70f);
    for(unsigned n=1;n<1000;++n){assert(reactionScale(n,1000)>reactionScale(n-1,1000));assert(reactionScale(n,1000)<=1);}
    assert(reactionScale(0,100)==0&&reactionScale(100,100)==1);
    // Disabling revised turnout must not disable a verified crowd fixture.
    turnout::CrowdFixture fixture;fixture.verified=true;
    fixture.seed.home.id=fixture.launch.home=1153;fixture.seed.away.id=fixture.launch.away=1193;
    fixture.seed.stadium=fixture.launch.stadium=41;fixture.seed.capacity=fixture.launch.capacity=45000;
    fixture.seed.ea=fixture.launch.ea=30000;fixture.seed.home.audienceFans=fixture.seed.away.audienceFans=30000;
    turnout::Applied noTurnout;assert(!noTurnout.owns(1153,1193)&&fixture.owns(1153,1193)&&plan(fixture,1153,1193).valid);
    fixture.seed.regular=false;fixture.seed.resumed=true;assert(fixture.owns(1153,1193));
    assert(!plan(fixture,1193,1153).valid&&!plan(fixture,1153,1194).valid);
    auto wrong=fixture;++wrong.launch.stadium;assert(!plan(wrong,1153,1193).valid);
    wrong=fixture;wrong.verified=false;assert(!plan(wrong,1153,1193).valid);
    wrong=fixture;wrong.launch.capacity=0;assert(!plan(wrong,1153,1193).valid);
    fixture.launch.neutral=true;assert(plan(fixture,1153,1193).native);
    std::puts("PASS: distance/fanbase odds, 1-70% launch bounds, continuous takeover curve, native-inclusive occupied audience denominator, protected-band exclusion, missing-data fallback and symmetric supporter strength");
}
