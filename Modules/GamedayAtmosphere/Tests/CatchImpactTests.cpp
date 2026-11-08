#include "../Source/DirectorShake.h"
#include <cassert>
#include <cstdio>
using namespace attendance::motion;
int main(){
    Settings settings;assert(settings.catchImpact==12);
    for(int phase:{4,5}){Model m;Input x;x.match=123;x.epoch=1;x.allowed=x.duringAllowed=x.duringImpact=x.duringCatch=true;x.phase=phase;x.now=1000;x.duringPlay=1;x.profile=Profile::Catch;x.leverage=.5f;
        auto hit=m.observe(settings,x);assert(hit.impactOnly&&hit.catchReaction&&hit.percent==12&&hit.until==3700);
        x.now=1200;hit=m.observe(settings,x);assert(strength(hit,1200)>11.9f&&strength(hit,1200)<=12);
        x.now=3700;assert(m.observe(settings,x).kind==Kind::None);
    }
    for(unsigned delay:{16u,100u,350u,351u,1000u}){Model m;Input x;x.match=123;x.epoch=1;x.allowed=x.duringAllowed=x.duringCatch=true;x.phase=3;x.now=1000;x.duringPlay=1;x.profile=Profile::Catch;x.leverage=.5f;
        m.observe(settings,x);x.duringPlay=0;
        for(unsigned update=100;update<delay;update+=100){x.now=1000+update;m.observe(settings,x);}
        x.phase=4;x.now=1000+delay;auto ended=m.observe(settings,x);
        assert(ended.impactOnly==(delay<=350));assert(ended.percent==(delay<=350?12:11));
        if(delay<=350){assert(ended.until==3700);x.now+=250;assert(strength(m.observe(settings,x),x.now)<=12.001f);}
    }
    {Model m;Input x;x.match=123;x.epoch=1;x.allowed=x.duringAllowed=x.duringImpact=true;x.phase=4;x.now=1000;x.duringPlay=1;x.profile=Profile::Catch;x.leverage=.5f;
        assert(m.observe(settings,x).percent==11); // Sack/other impacts keep their own tested strength.
        x.touchdown=1;x.now=1100;assert(m.observe(settings,x).kind==Kind::Touchdown);
        x.eruption=1;x.eruptionAlive=true;x.now=1200;assert(m.observe(settings,x).kind==Kind::Eruption);}
    {Model m;Input x;x.match=123;x.epoch=1;x.phase=4;x.now=1000;x.duringPlay=1;x.profile=Profile::Catch;x.leverage=.5f;x.duringCatch=true;assert(m.observe(settings,x).kind==Kind::None);}
    std::puts("PASS: separate 12% immediate-catch impact, bounded whistle classification, continuous caught-and-run strength, 2.7-second duration, sack separation, priority and disabled guards.");
}
