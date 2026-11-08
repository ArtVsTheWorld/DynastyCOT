#include "../Source/AwayCrowd.h"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iomanip>
using namespace attendance;
struct Case {const char* name;double home,away,distance;bool state,rivalry;float low,high;};
static double oldShare(const visitors::Demand& d){
    double distance=visitors::miles(d.home,d.away);
    double travel=(std::clamp)(1.35*std::exp(-distance/2200.),.35,1.35);
    double share=.05*std::pow(d.awayFans/d.homeFans,.9)*travel*(.9+.2*seats::random({static_cast<int>(d.seed%100000),407}));
    if(d.sameState)share+=.005*travel;
    if(d.rivalry)share+=(.015+.01*seats::random({static_cast<int>(d.seed%100000),409}))*travel;
    return (std::clamp)(share,.01,.5);
}
int main(int argc,char** argv){
    const Case cases[]={
        {"Distant limited support",40000,28000,2500,false,false,.01f,.03f},
        {"Ordinary comparable fanbases",30000,30000,700,false,false,.03f,.10f},
        {"Nearby comparable fanbases",30000,30000,100,false,false,.10f,.20f},
        {"Strong traveling fanbase",30000,75000,600,false,false,.10f,.20f},
        {"Large fanbase at less-supported host",15000,75000,400,false,false,.20f,.35f},
        {"Exceptional nearby rivalry takeover",9000,105000,125,true,true,.35f,.70f},
        {"Extreme fanbase imbalance",2000,200000,125,true,true,.69f,.70f},
        {"Comparable in-state rivalry",30000,30000,40,true,true,.15f,.22f}
    };
    std::ofstream csv;if(argc>1){csv.open(argv[1]);assert(csv);csv<<"Situation,Home audience,Away audience,Distance miles,Old low %,Old high %,New low %,New high %,New average %,Visitors among 20000 attendees\n"<<std::fixed<<std::setprecision(2);}
    for(const auto& c:cases){
        double lo=1,hi=0,sum=0,oldLo=1,oldHi=0;
        for(unsigned seed=0;seed<1000;++seed){
            visitors::Demand d;d.homeFans=c.home;d.awayFans=c.away;d.home={20,-85};d.away={20+c.distance/69.0934,-85};d.sameState=c.state;d.rivalry=c.rivalry;d.seed=seed;
            auto p=visitors::plan(d);assert(p.valid&&!p.native&&p.travelKnown);
            assert(p.share>=c.low-.00001f&&p.share<=c.high+.00001f);
            lo=(std::min)(lo,static_cast<double>(p.share));hi=(std::max)(hi,static_cast<double>(p.share));sum+=p.share;
            auto old=oldShare(d);oldLo=(std::min)(oldLo,old);oldHi=(std::max)(oldHi,old);
        }
        if(csv)csv<<c.name<<','<<c.home<<','<<c.away<<','<<c.distance<<','<<oldLo*100<<','<<oldHi*100<<','<<lo*100<<','<<hi*100<<','<<sum/10<<','<<std::llround(sum/1000*20000)<<'\n';
    }
    std::puts("PASS: 8000 seeded visitor scenarios cover distant, ordinary, nearby, traveling, weak-host, 70%-capped takeover and in-state rivalry ranges; spectator-count examples exclude empty capacity");
}
