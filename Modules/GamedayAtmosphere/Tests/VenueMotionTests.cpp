#include "../Source/DirectorShake.h"
#include "../Source/LineupAtmosphere.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    assert(std::abs(motion::venueScale(6060)-.80f)<.00001f);
    assert(std::abs(motion::venueScale(61888)-1.25f)<.00001f);
    assert(motion::venueScale(2039)<motion::venueScale(6060));
    assert(motion::venueScale(100000)==motion::venueScale(61888));
    assert(motion::venueScale(0)==0&&motion::venueScale(UINT64_MAX)==0);
    float previous=0;for(uint64_t n=512;n<200000;n+=31){auto scale=motion::venueScale(n);assert(scale>=previous&&scale>=.67f&&scale<=1.25f);previous=scale;}
    for(int family=0;family<7;++family)for(bool neutral:{false,true})for(uint64_t seats:{6060ull,30000ull,51424ull,61888ull}){
        motion::Settings s;motion::Model base,scaled;motion::Input a;a.match=123;a.epoch=2;a.allowed=a.venueEligible=true;a.phase=4;a.now=1000;
        if(family==0){a.phase=1;a.lineup=true;a.lineupId=1;}
        if(family==1){a.phase=1;a.lineup=true;a.lineupId=1;a.critical=true;}
        if(family==2)a.touchdown=1;
        if(family==3){a.eruption=1;a.eruptionAlive=true;}
        if(family==4){a.phase=3;a.duringPlay=1;}
        if(family==5){a.duringPlay=1;a.duringImpact=a.forcedImpact=true;}
        if(family==6){a.phase=1;a.lineup=true;a.lineupId=1;a.hostileOpening=s.hostileOpeningMaximum;}
        auto b=a;b.neutral=neutral;b.venueScale=motion::venueScale(seats);
        base.observe(s,a);scaled.observe(s,b);
        float factor=b.venueScale*(neutral?.75f:1.f);
        for(uint64_t now=1050;now<4500;now+=50){a.now=b.now=now;auto c=base.observe(s,a),d=scaled.observe(s,b);
            assert(std::abs(motion::strength(d,now)-motion::strength(c,now)*factor)<.002f);
        }
        // Size never admits motion when the atmosphere/world gate rejected it.
        b.allowed=false;b.now=4600;assert(scaled.observe(s,b).kind==motion::Kind::None);
        b.allowed=true;b.lineup=false;b.duringPlay=b.touchdown=b.eruption=0;b.now=4700;assert(scaled.observe(s,b).kind==motion::Kind::None);
    }
    // Qualification remains exactly the venue/fullness rule; size only changes strength.
    assert(motion::venueScale(51424)>motion::venueScale(6060));
    std::puts("PASS: catalog curve endpoints/clamps, monotonicity, all motion families scaled once, neutral combination and admission cleanup.");
}
