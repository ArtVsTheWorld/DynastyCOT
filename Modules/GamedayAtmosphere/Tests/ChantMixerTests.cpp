#include "../Source/ChantMixerPreset.h"
#include "../Source/ChantAudioModel.h"
#include <cassert>
#include <cstdio>
using namespace attendance::chants;
template<class T>T at(Address p){T v{};std::memcpy(&v,reinterpret_cast<void*>(p),sizeof(v));return v;}
int main(){
    assert(attendancePercent(1)==0&&attendancePercent(0.93f)==0&&attendancePercent(0.90f)==0&&attendancePercent(0.80f)==0);
    for(int percent=70;percent<=100;percent++)assert(attendancePercent(percent*0.01f)==0);
    assert(attendancePercent(0.75f)==0&&attendancePercent(0.70f)==0);
    assert(std::fabs(attendancePercent(0.65f)-1.714678f)<.0001f&&std::fabs(attendancePercent(0.5f)-20.85048f)<.0001f);
    assert(attendancePercent(0.22f)==50&&attendancePercent(0.01f)==50&&attendancePercent(0)==100);
    assert(std::fabs(attendancePercent(0.5f,50)-10.42524f)<.0001f&&attendancePercent(0.5f,0)==0);
    assert(attendancePercent(NAN)==0&&attendancePercent(2)==0&&attendancePercent(-1)==100);
    assert(attendancePercent(0.5f,100,50,1.25f)<attendancePercent(0.5f));
    assert(attendancePercent(0,100,50,1.35f)==100&&attendancePercent(1,100,50,1.35f)==0);
    float previous=0;
    for(int percent=100;percent>=0;--percent){
        float cut=attendancePercent(percent*0.01f);assert(cut>=previous);previous=cut;
    }
    Templates t;Preset p(t,-6,1),q(t,-12,1);
    assert(std::memcmp(p.asset.bytes.data(),q.asset.bytes.data(),16)!=0);
    Address a=p.asset.address(),g=at<Address>(a+0x28),s=at<Address>(a+0x20);
    assert(at<uint16_t>(a+0x14)&0x100);
    assert(at<uint32_t>(at<Address>(a+0x30)-4)==1);
    for(size_t off:{size_t(0x18),size_t(0x20),size_t(0x28),size_t(0x30),size_t(0x40),size_t(0x48),size_t(0x50)})assert(at<uint32_t>(at<Address>(g+off)-4)==0);
    assert(at<uint16_t>(g+0x38)==0&&at<uint16_t>(g+0x3a)==0);
    Address groups=at<Address>(s+0x28);assert(at<uint32_t>(groups-4)==1);
    Address group=at<Address>(groups),props=at<Address>(groups+8);
    assert(at<uint16_t>(group+0x28)==14&&at<uint16_t>(group+0x2a)==9);
    assert(at<uint32_t>(props-4)==1&&at<uint8_t>(groups+0x20)==1&&at<uint8_t>(groups+0x21)==0);
    assert(at<uint32_t>(groups+0x10)==0&&at<uint32_t>(groups+0x18)==0);
    static_assert(static_cast<uint32_t>(MixProperty::Gain)==0);
    static_assert(static_cast<uint32_t>(MixProperty::Reverb)==3);
    assert(at<uint32_t>(props)==0&&at<float>(props+4)==-6&&at<uint8_t>(props+8)==0);
    assert(extraDb(1,35)==0&&extraDb(0.5f,0)==0&&extraDb(0.5f,35)==-2);
    assert(extraDb(0.22f,35)==-4.5f&&extraDb(0,100)==-60);
    Preset muted(t,0,2,true);Address muteGroups=at<Address>(muted.preset.address()+0x28);
    assert(at<unsigned>(muteGroups+0x18)==1&&at<unsigned>(muteGroups+0x10)==1000&&at<uint8_t>(muteGroups+0x21)==1);
    Templates broad=t;broad.groupIndex=9;broad.parent=2;std::memcpy(broad.name,"Crowd",6);
    Preset crowdMute(broad,0,3,true);assert(at<unsigned short>(crowdMute.group.address()+0x28)==9&&at<unsigned short>(crowdMute.group.address()+0x2a)==2);
    std::puts("Chant-only additive preset, owned storage, unique IDs and attenuation checks passed.");
}
