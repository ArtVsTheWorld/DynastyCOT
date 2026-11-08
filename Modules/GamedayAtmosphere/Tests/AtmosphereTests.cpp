#include "../Source/AtmosphereRules.h"
#include <cassert>
#include <iostream>
using namespace attendance;
int main(){
    Settings c;State s;History h;
    s={2,0,28,1,720,720};assert(h.observe(c,s)&&h.maxDeficit==28&&!h.comeback);
    s.quarter=3;s.homeScore=14;assert(h.observe(c,s)&&h.comeback);
    s.awayScore=35;s.seconds=700;h.observe(c,s);assert(h.comeback&&h.deteriorationSince>=0); // A full touchdown starts deterioration.
    s.awayScore=36;s.seconds=680;h.observe(c,s);assert(h.comeback);
    s.seconds=680;for(int i=0;i<500;i++)h.observe(c,s);assert(h.comeback); // Pauses consume no game time.
    s.seconds=641;h.observe(c,s);assert(h.comeback);s.seconds=640;h.observe(c,s);assert(!h.comeback);
    h.observe(c,s);assert(!h.comeback); // Cannot instantly rearm after deterioration.
    s.homeScore=30;s.seconds=610;h.observe(c,s);assert(h.comeback); // Within one possession.
    s.homeScore=0;s.seconds=710;assert(!h.observe(c,s));assert(h.comeback&&h.maxDeficit==28); // Replay rejected.
    h.reset();s={4,0,35,1,130,300};h.observe(c,s);s.seconds=119;h.observe(c,s);
    assert(h.takeWarning()&&!h.takeWarning());s.seconds=180;assert(!h.observe(c,s));s.seconds=110;h.observe(c,s);assert(!h.takeWarning());
    h.reset();s.seconds=110;h.observe(c,s);assert(!h.takeWarning()); // Attaching after warning does not invent a crossing.
    h.reset();s={4,0,0,1,120,120};h.observe(c,s);s.seconds=119;h.observe(c,s);assert(!h.takeWarning());
    h.reset();s={3,0,0,3,300,300};assert(!h.observe(c,s));assert(h.observe(c,s,true));
    assert(h.takeQuarter(2,3)&&!h.takeQuarter(2,3));s.quarter=4;assert(h.observe(c,s,true)&&h.takeQuarter(3,4));
    s.quarter=3;assert(!h.observe(c,s,true)); // Rewound native period event also rejected.
    s={4,24,21,1,180,300};s.rivalry=true;
    assert(situationEmphasis(s,15,10)==1.15f);s.awayScore=42;assert(situationEmphasis(s,15,10)==1.0f);
    s.rivalry=false;s.rank0=1;s.rank1=25;assert(situationEmphasis(s,15,10)==1.1f);
    s={2,14,21,1,180,300};s.rank0=132;s.rank1=10;h.reset();h.observe(c,s);
    assert(h.watchStage==1&&upsetDiscrepancy(s)>0.7f);
    float bigGap=upsetDiscrepancy(s);s.rank0=26;assert(bigGap>upsetDiscrepancy(s));
    s.rank0=132;s.quarter=4;s.homeScore=21;h.reset();h.observe(c,s);assert(h.watchStage==3);
    s.awayScore=24;h.observe(c,s);assert(h.watchStage==3);s.awayScore=28;h.observe(c,s);assert(h.watchStage==2);
    s.awayScore=40;h.observe(c,s);assert(h.watchStage==0);
    s.rank0=0;assert(upsetDiscrepancy(s)==0&&upsetStage(s)==0);
    s={4,21,21,1,180,300};s.neutral=true;s.rank0=3;s.rank1=132;
    assert(upsetStage(s)==3&&upsetDiscrepancy(s)>0.7f);
    h.reset();h.observe(c,s);
    History other;State reverse=reversed(s);other.observe(c,reverse);
    assert(other.watchStage==h.watchStage&&upsetDiscrepancy(s)==upsetDiscrepancy(reverse));
    h.reset();s={2,28,0,1,300,300};s.neutral=true;h.observe(c,s);
    s.quarter=3;s.awayScore=14;h.observe(c,s);assert(h.comeback&&h.comebackSide==1);
    for(int minutes=1;minutes<=20;++minutes){
        h.reset();float q=static_cast<float>(minutes*60),patience=60*q/720;
        s={2,0,28,5,q,q};assert(h.observe(c,s));
        s.quarter=3;s.homeScore=14;assert(h.observe(c,s)&&h.comeback);
        s.awayScore=36;s.seconds=q-1;assert(h.observe(c,s)&&h.comeback);
        s.seconds=q-1-patience+0.1f;assert(h.observe(c,s)&&h.comeback);
        s.seconds=q-1-patience-0.1f;assert(h.observe(c,s)&&!h.comeback);
    }
    std::cout<<"PASS: full-rank upsets, comeback recovery/deterioration, no refill, replay rejection, once-only warning, short quarters, neutral symmetry and audio situations.\n";
}
