#include "../Source/MotionSoundPairing.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    for(auto kind:{motion::Kind::Ordinary,motion::Kind::Critical,motion::Kind::LivePlay,motion::Kind::Touchdown,motion::Kind::Eruption,motion::Kind::Safety,motion::Kind::BlockedKick,motion::Kind::Clincher}){
        for(float fullness:{.5f,.8f,1.f}){
            motion::Cue cue{123,1,1400,1000,6000,kind,10,false,15};
            sustained::Intent out;out.active=true;out.match=123;out.epoch=1;out.updated=1400;
            auto paired=pairing::merge(out,cue,1400,fullness,1,true);
            assert(paired.intensity>0&&(paired.continuous||paired.reaction)&&paired.reason!=sustained::Reason::None);
            assert(!paired.liveCheer&&!paired.lineupCheer&&!paired.resultCheer); // A swell suffices; no invented cheer.
            out.reason=sustained::Reason::Lineup;out.continuous=true;out.intensity=0;
            paired=pairing::merge(out,cue,1400,fullness,1,true);assert(paired.intensity>0&&(paired.continuous||paired.reaction));
            out.reason=sustained::Reason::Eruption;out.celebration=8;
            paired=pairing::merge(out,cue,1400,fullness,1,true);assert(paired.intensity>0&&paired.celebration==8&&paired.reason==sustained::Reason::Eruption);
            pairing::SoundLease sound{123,1,1400,true};assert(pairing::soundReady(sound,cue,1400));
            sound.audible=false;assert(!pairing::soundReady(sound,cue,1400));sound.audible=true;
            bool earned=kind!=motion::Kind::Ordinary&&kind!=motion::Kind::Critical;
            bool scoring=kind==motion::Kind::Touchdown||kind==motion::Kind::Eruption;
            assert(!pairing::soundReady(sound,cue,1551));assert(!pairing::soundReady(sound,cue,1399));
            assert(!pairing::soundReady(sound,cue,1700));
            assert(!pairing::soundReady(sound,cue,2200));
            sound.audible=false;sound.audibleAt=1400;
            assert(!pairing::soundReady(sound,cue,1551));
            if(earned&&!scoring){float prior=1;for(uint64_t now=1550;now<=1700;now+=10){float next=pairing::soundWeight(sound,cue,now);assert(next<=prior);prior=next;}}
            sound.audible=true;sound.audibleAt=0;
            ++sound.epoch;assert(!pairing::soundReady(sound,cue,1400));--sound.epoch;++sound.match;assert(!pairing::soundReady(sound,cue,1400));
        }
    }
    // No historical voice proof may admit shake during a silent handoff.
    for(auto kind:{motion::Kind::Touchdown,motion::Kind::Eruption}){
        motion::Cue cue{123,2,1000,1000,12000,kind,16,false,20};
        for(uint64_t age=0;age<11000;age+=16){
            auto now=1000+age;cue.updated=now;bool voice=age%600<100;
            pairing::SoundLease sound{123,2,now,voice,now-age%600+(voice?age%600:96)};
            for(bool presentation:{false,true})assert(pairing::soundWeight(sound,cue,now,presentation)==(voice?1.f:0.f));
        }
        cue.updated=12000;pairing::SoundLease sound{123,2,12000,true,12000};
        assert(pairing::soundWeight(sound,cue,12000)==0); // Original expiry; no extension.
        cue.updated=cue.began=13000;cue.until=24000;
        sound={123,2,13500,false,12000};assert(pairing::soundWeight(sound,cue,13500)==0); // Old score cannot admit a new one.
        sound.audibleAt=13100;cue.updated=13500;assert(pairing::soundWeight(sound,cue,13500)==0);
        sound.audibleAt=0;assert(pairing::soundWeight(sound,cue,13500)==0); // Mute/inactive context clears proof.
        sound.audibleAt=13100;sound.updated=12749;assert(pairing::soundWeight(sound,cue,13500)==0); // Stale audio callback.
        sound.updated=13500;cue.updated=12749;assert(pairing::soundWeight(sound,cue,13500)==0); // Stale camera cue.
        cue.updated=13500;++sound.epoch;assert(pairing::soundWeight(sound,cue,13500)==0);
        --sound.epoch;++sound.match;assert(pairing::soundWeight(sound,cue,13500)==0);
    }
    // Visitor shake also requires current audible, current-play sound proof.
    for(auto kind:{motion::Kind::LivePlay,motion::Kind::Touchdown,motion::Kind::Eruption,motion::Kind::Safety,motion::Kind::BlockedKick}){
        motion::Cue cue{123,2,3000,2500,6000,kind,3,false,4};cue.visitor=true;cue.playSequence=19;
        pairing::SoundLease sound{123,2,3000,true,3000,19};
        assert(pairing::soundWeight(sound,cue,3000,true)==1);
        cue.tail=true;cue.began=2900;assert(pairing::soundWeight(sound,cue,3000,true)==1);
        ++sound.playSequence;assert(pairing::soundWeight(sound,cue,3000,true)==0);--sound.playSequence;
        sound.playSequence=0;assert(pairing::soundWeight(sound,cue,3000,true)==0);sound.playSequence=19;
        sound.audible=false;assert(pairing::soundWeight(sound,cue,3000,true)==0);sound.audible=true;
        sound.updated=2249;assert(pairing::soundWeight(sound,cue,3000,true)==0);sound.updated=3000;
        cue.updated=2249;assert(pairing::soundWeight(sound,cue,3000,true)==0);cue.updated=3000;
        assert(pairing::soundWeight(sound,cue,6000,true)==0);
        cue.updated=sound.updated=6000;assert(pairing::soundWeight(sound,cue,6000,true)==0);
        ++sound.epoch;assert(pairing::soundWeight(sound,cue,3000,true)==0);--sound.epoch;
        ++sound.match;assert(pairing::soundWeight(sound,cue,3000,true)==0);
    }
    std::puts("PASS: bounded sound companions, TD/eruption continuity and play-owned visitor handoffs; mute, stale, expiry, new-play and world isolation preserved.");
}
