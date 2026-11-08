#include "../Source/DirectorShake.h"
#include <cassert>
#include <cstdio>
using namespace attendance::motion;
Input input(){Input x;x.match=1;x.epoch=1;x.allowed=true;x.phase=3;x.now=1000;x.runBuildup=1;x.smoothRunBuildup=true;x.leverage=1;return x;}
float ramp(unsigned step){Settings s;Model m;auto x=input();x.runProgress=1;m.observe(s,x);for(x.now=1000+step;x.now<1800;x.now+=step)m.observe(s,x);x.now=1800;return strength(m.observe(s,x),x.now);}
int main(){
    Settings s;Model m;auto x=input();x.runProgress=.99f;
    auto cue=m.observe(s,x);assert(cue.smoothRunBuildup&&cue.percent<=1.01f);
    float previous=0;
    for(x.now=1016;x.now<=1800;x.now+=16){cue=m.observe(s,x);float level=strength(cue,x.now);assert(level>=previous&&level-previous<.5f&&level<=12);previous=level;}
    assert(previous>11.5f);assert(std::fabs(ramp(16)-ramp(33))<.001f&&std::fabs(ramp(33)-12)<.001f);
    // Reaching 20 yards confirms the existing cue without a fresh attack.
    x.now=1800;x.runProgress=1;x.duringPlay=1;x.profile=Profile::Gain;cue=m.observe(s,x);auto began=cue.began;
    x.now=1816;x.duringPlay=0;cue=m.observe(s,x);assert(cue.began==began&&cue.confirmedRun);
    // A tackle during the attack settles only from movement actually reached.
    m.reset();x=input();x.runProgress=1;m.observe(s,x);x.now=1100;cue=m.observe(s,x);float atWhistle=strength(cue,x.now);
    x.phase=4;x.now=1110;float before=strength(cue,x.now);cue=m.observe(s,x);assert(cue.tail&&std::fabs(cue.percent-before)<.001f&&cue.percent<2&&atWhistle<2);
    x.now=1200;assert(strength(m.observe(s,x),x.now)<=before);
    // Score handoff rises from the reached movement rather than restarting.
    m.reset();x=input();x.runProgress=1;m.observe(s,x);for(x.now=1050;x.now<=1450;x.now+=50)cue=m.observe(s,x);
    x.phase=4;x.now=1500;x.touchdown=1;auto td=m.observe(s,x);assert(td.kind==Kind::Touchdown&&td.scoreBuildup&&strength(td,x.now)>=strength(cue,x.now));
    x.now=1800;assert(strength(m.observe(s,x),x.now)==20);
    // Existing return/showboat timing and opt-outs are unchanged.
    m.reset();x=input();x.smoothRunBuildup=false;x.runProgress=1;cue=m.observe(s,x);assert(cue.percent==12);x.now=1200;assert(strength(m.observe(s,x),x.now)==12);
    m.reset();auto off=s;off.live=0;x=input();x.runProgress=1;for(int i=0;i<20;++i){x.now+=50;assert(strength(m.observe(off,x),x.now)==0);}
    x.allowed=false;assert(m.observe(s,x).kind==Kind::None);
    std::puts("PASS: late first sample, 800ms gain attack, cadence independence, whistle from reached level, one-shot confirmation, score handoff, return timing and opt-outs.");
}
