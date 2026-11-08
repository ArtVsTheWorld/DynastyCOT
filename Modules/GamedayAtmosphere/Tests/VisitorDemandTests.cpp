#include "../Source/AwayCrowd.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    const char* names[]={"small/far","nearby rivals","major/weak host","major/strong host","top-ten showdown"};
    double sums[5]{},low[5]={1,1,1,1,1},high[5]{};
    for(unsigned seed=0;seed<1000;++seed){
        visitors::Demand base;base.home={30,-97};base.away={30+1000/69.0934,-97};base.seed=seed;
        base.homeFans=base.awayFans=100000;base.homeStrength=1;
        auto strong=visitors::plan(base);auto marquee=base;marquee.marquee=true;auto big=visitors::plan(marquee);
        assert(strong.share>=.06&&big.share>strong.share&&big.share<=strong.share*1.15f);
        auto small=base;small.homeFans=small.awayFans=25000;assert(visitors::plan(small).share<strong.share);
        auto weak=base;weak.homeFans=20000;weak.homeStrength=0;assert(visitors::plan(weak).share>big.share);
        auto near=base;near.away={30.5,-97};near.sameState=near.rivalry=true;
        auto distant=small;distant.away={30+2000/69.0934,-97};assert(visitors::plan(distant).share<.03);
        visitors::Demand cases[]={distant,near,weak,base,marquee};
        for(unsigned i=0;i<5;++i){auto p=visitors::plan(cases[i]);sums[i]+=p.share;low[i]=(std::min)(low[i],double(p.share));high[i]=(std::max)(high[i],double(p.share));}
        auto unknown=base;unknown.home=unknown.away={};assert(visitors::plan(unknown).travelFactor==1);
        unknown.neutral=true;assert(visitors::plan(unknown).native&&visitors::plan(unknown).share==.5f);
    }
    for(unsigned i=0;i<5;++i)std::printf("%s: %.2f-%.2f%% mean %.2f%%\n",names[i],low[i]*100,high[i]*100,sums[i]/10);
    std::puts("PASS: separate contingent/ticket resistance, national distance resilience, capped marquee demand, unknown and neutral behavior across 5000 matchups.");
}
