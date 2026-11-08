#include "../Source/PresentationAtmosphere.h"
#include "../Source/AttendanceAudio.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
State opening(){State s{1,0,0,6,720,720};s.rank0=6;s.rank1=2;return s;}
int main(){
    core::Settings c;presentationaudio::Model m;auto s=opening();
    auto tick=[&](uint64_t now,float f=.9f,bool known=true,bool menu=false,bool allowed=true){return m.observe(c,s,123,1,now,allowed,known,f,56099,menu);};
    assert(!tick(1000,.17f).active&&!tick(1100,.9f,false).active);
    auto pre=tick(1200,.6f);assert(pre.active&&pre.reason==sustained::Reason::Opening&&pre.continuous&&pre.size==3&&pre.supporting);
    assert(!pre.shake&&!pre.cheers&&!pre.celebration&&!pre.resultCheer&&!pre.liveCheer&&pre.layerBoostPercent==0);
    assert(tick(2000,1).intensity==1000);
    c.opening=false;assert(!tick(2100).active);c.opening=true;
    s.rank1=50;assert(!tick(2200,.6f).active);assert(tick(2300,.9f).active); // Ordinary route still uses packed-house gate.
    s.rank1=2;assert(!tick(301000).active&&!tick(302000).active); // Bounded intro, no repeated restart.
    s.phase=1;assert(!tick(303000).active); // No duplicate play-call model.
    s.phase=7;s.quarter=6;s.seconds=0;s.homeScore=21;s.awayScore=14;
    auto post=tick(304000);assert(post.active&&post.reason==sustained::Reason::Postgame&&post.continuous);
    assert(!post.shake&&!post.cheers&&!post.celebration&&post.layerBoostPercent==0);
    assert(!tick(305000,.9f,true,true).active&&!tick(305100,.9f,true,false,false).active);
    s.homeScore=14;s.awayScore=21;assert(!tick(306000).active);
    s.neutral=true;assert(tick(306100).active);s.awayScore=14;assert(!tick(306200).active);
    s.homeScore=21;s.awayScore=0;assert(tick(306300).intensity<post.intensity);
    s.homeScore=42;s.awayScore=0;assert(!tick(306400).active);
    s.homeScore=21;s.awayScore=14;assert(!tick(424000).active); // Bounded ordinary outro; earned eruption is separate.
    auto fresh=m.observe(c,s,124,2,425000,true,true,.9f,56099,false);assert(fresh.active);
    assert(balance::audiblePhase(6,false,false)&&balance::audiblePhase(7,false,false));
    assert(!balance::audiblePhase(7,false,true)); // Interactive replay retains native control.
    assert(balance::audiblePhase(2,true,false)); // Live timeout transitions keep crowd quieting consistent.
    assert(!balance::audiblePhase(2,false,false)&&!balance::audiblePhase(2,true,true));
    std::puts("PASS: fullness-qualified intro/outro atmosphere, paused presentation quieting, original gates/volume caps, no fabricated play/cheer/shake, home/neutral winners, blowout/menu/context guards and bounded scene lifetimes.");
}
