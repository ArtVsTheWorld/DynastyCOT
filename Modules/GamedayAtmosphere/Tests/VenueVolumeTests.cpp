#include "../Source/CrowdAudioModel.h"
#include <cassert>
#include <cstdio>
using namespace attendance::audio;
int main(){
    for(uint64_t n:{0ull,511ull,12001ull,48530ull,61888ull,12800001ull})assert(!volumeCutVenue(n));
    for(uint64_t n:{512ull,6060ull,10000ull,11999ull,12000ull})assert(volumeCutVenue(n));
    for(float fullness:{0.f,.10f,.25f,.49f,.50f,.75f,1.f})for(uint64_t n:{6060ull,12000ull,12001ull,48530ull}){
        Gain volume;volume.sparse(fullness,volumeCutVenue(n)?.25f:0,.5f);volume.advance(6);
        assert(volume.value()>=.75f);if(n>12000||fullness>=.5f)assert(volume.value()==1);
    }
    Gain scene;scene.sparse(0,.25f,.5f);scene.advance(6);assert(scene.value()==.75f);scene.reset();assert(scene.value()==1); // Large/new scene releases old managed cut.
    std::puts("PASS: verified 12,000-position volume-cut boundary, unknown/large exclusion, 25% cap, no cut above 50%, unchanged native base and owned-cut restoration.");
}
