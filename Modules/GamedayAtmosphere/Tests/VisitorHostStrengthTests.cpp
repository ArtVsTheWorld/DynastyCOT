#include "../Source/AwayCrowd.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    double low=1,high=0,sum=0;
    for(unsigned seed=0;seed<1000;++seed){
        visitors::Demand d;d.homeFans=d.awayFans=75000;d.home={40.2,-111.65};d.away={40.77,-111.85};d.sameState=d.rivalry=true;d.seed=seed;
        auto ordinary=visitors::plan(d);float previous=ordinary.share;
        for(unsigned n=1;n<=20;++n){d.homeStrength=n/20.;auto defended=visitors::plan(d);assert(defended.share<=previous);previous=defended.share;}
        auto strong=visitors::plan(d);assert(strong.share<ordinary.share&&strong.share>=.07&&strong.share<=.13);
        low=(std::min)(low,double(strong.share));high=(std::max)(high,double(strong.share));sum+=strong.share;
        d.neutral=true;assert(visitors::plan(d).native&&visitors::plan(d).share==.5f);
    }
    std::printf("PASS: 20000 host-strength comparisons; nearby equal-fanbase rivalry at a strong host: %.2f-%.2f%% (mean %.2f%%)\n",low*100,high*100,sum/10);
}
