#pragma once
#include <algorithm>
#include <cstdint>
namespace attendance::variety {
inline uint64_t mix(uint64_t x){x+=0x9e3779b97f4a7c15ull;x=(x^(x>>30))*0xbf58476d1ce4e5b9ull;x=(x^(x>>27))*0x94d049bb133111ebull;return x^(x>>31);}
struct Profile {
    unsigned attackMs=3000,envelopeAttackMs=2500,roarMs=28000,releaseMs=12000;
    unsigned supportDelayMs=1500;
    int sizeChoice=0;
    unsigned totalMs()const{return roarMs+releaseMs;}
    int size(float strength)const{
        int base=strength>=.65f?3:strength>=.35f?2:1;
        // Full opening/OT energy keeps the largest profile. Only borderline
        // situations move one size, and settled ambience stays restrained.
        if(base==3&&strength<.9f&&sizeChoice==0)return 2;
        if(base==2&&strength>=.5f&&sizeChoice==3)return 3;
        if(base==2&&strength<.5f&&sizeChoice==0)return 1;
        return base;
    }
};
inline Profile choose(uint64_t seed,uint64_t sequence,uint64_t cycle=0){
    uint64_t h=mix(seed^mix(sequence)^mix(cycle+0x524f4152ull));
    auto pick=[&](unsigned lo,unsigned hi){h=mix(h);return lo+static_cast<unsigned>(h%(hi-lo+1));};
    Profile p;p.attackMs=pick(2500,3500);p.envelopeAttackMs=pick(2200,2800);
    p.releaseMs=pick(10000,14000);p.roarMs=40000-p.releaseMs;
    p.supportDelayMs=pick(1200,2000);p.sizeChoice=static_cast<int>(pick(0,3));return p;
}
}
