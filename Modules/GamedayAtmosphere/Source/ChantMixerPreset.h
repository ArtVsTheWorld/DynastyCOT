#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <cmath>

namespace attendance::chants {
using Address=uintptr_t;
// Reflected from the game's MixGroupPropertyType enum. Reverb is property 3;
// changing it does not establish that the dry sound has changed volume.
enum class MixProperty : uint32_t { Gain=0, Pitch=1, Lfe=2, Reverb=3, Lpf=4, AttenuationDistanceScale=5 };
template<size_t N>struct alignas(16) Container {
    std::array<unsigned char,(16+N+15)&~size_t(15)> bytes{};
    Address address(){return reinterpret_cast<Address>(bytes.data()+16);}
    template<class T>void put(size_t offset,T value){std::memcpy(bytes.data()+16+offset,&value,sizeof(value));}
};
template<size_t N>struct alignas(8) Array {
    uint32_t reserved=0,count=0;std::array<unsigned char,N> data{};
    Address address(){return reinterpret_cast<Address>(data.data());}
};
struct Templates {
    unsigned short groupIndex=14,parent=9;char name[64]="Chants/Response";
    std::array<unsigned char,16+0x40> asset{};
    std::array<unsigned char,16+0x58> graph{};
    std::array<unsigned char,16+0x38> preset{};
    std::array<unsigned char,16+0x30> group{};
    std::array<unsigned char,0x28> entry{};
    std::array<unsigned char,24> properties{};
};

// An immutable, additive preset containing exactly one group. All arrays and
// containers are owned here; no game-heap pointer survives the template copy.
// Instances must stay at a stable address until their native mixer is retired.
struct Preset {
    Container<0x40> asset;Container<0x58> graph;Container<0x38> preset;Container<0x30> group;
    Array<8> empty;Array<8> presets;Array<0x28> groups;Array<24> properties;
    char groupName[64]{};
    Preset(const Templates& t,float db,uint64_t serial,bool mute=false){
        std::memcpy(asset.bytes.data(),t.asset.data(),t.asset.size());std::memcpy(graph.bytes.data(),t.graph.data(),t.graph.size());
        std::memcpy(preset.bytes.data(),t.preset.data(),t.preset.size());std::memcpy(group.bytes.data(),t.group.data(),t.group.size());
        // createMixer caches by the asset GUID at -16. Never reuse a stock GUID.
        const uint64_t guid[2]={0xa6ce61c7d1a6475bull^reinterpret_cast<uint64_t>(this),0xb827da0000000000ull^serial};
        std::memcpy(asset.bytes.data(),guid,16);
        const char* name="DynamicGamedayAtmosphere/OwnedCrowdGain";
        uint16_t flags=0;std::memcpy(&flags,asset.bytes.data()+16+0x14,2);asset.put(0x14,uint16_t(flags|0x100));
        asset.put(0x18,reinterpret_cast<Address>(name));asset.put(0x20,preset.address());
        asset.put(0x28,graph.address());asset.put(0x30,presets.address());asset.put(0x38,uint8_t(0));
        presets.count=1;Address p=preset.address();std::memcpy(presets.data.data(),&p,8);
        // Empty graph: gain comes from the preset, so no generic graph input,
        // expression, or other group's output is executed.
        for(size_t off:{size_t(0x18),size_t(0x20),size_t(0x28),size_t(0x30),size_t(0x40),size_t(0x48),size_t(0x50)})graph.put(off,empty.address());
        graph.put(0x38,uint16_t(0));graph.put(0x3a,uint16_t(0));
        preset.put(0x18,empty.address());preset.put(0x20,reinterpret_cast<Address>(name));preset.put(0x28,groups.address());
        std::memcpy(groupName,t.name,sizeof(groupName));groupName[63]=0;
        group.put(0x18,empty.address());group.put(0x20,reinterpret_cast<Address>(groupName));
        group.put(0x28,t.groupIndex);group.put(0x2a,t.parent);
        groups.count=1;groups.data=t.entry;properties.count=1;properties.data=t.properties;
        Address a=group.address(),b=properties.address();std::memcpy(groups.data.data(),&a,8);std::memcpy(groups.data.data()+8,&b,8);
        // State Normal, additive, non-dominant, priority zero, native smoothing.
        uint32_t priority=mute?1000u:0u,state=mute?1u:0u;float attack=0.25f,release=0.25f;
        std::memcpy(groups.data.data()+0x10,&priority,4);std::memcpy(groups.data.data()+0x18,&state,4);
        std::memcpy(groups.data.data()+0x14,&release,4);std::memcpy(groups.data.data()+0x1c,&attack,4);
        groups.data[0x20]=1;groups.data[0x21]=mute?1:0;
        // Gain only. Other level/loudness properties retain the game's mix.
        uint32_t property=static_cast<uint32_t>(MixProperty::Gain);std::memcpy(properties.data.data(),&property,4);
        std::memcpy(properties.data.data()+4,&db,4);properties.data[8]=0;
    }
    Preset(const Preset&)=delete;Preset& operator=(const Preset&)=delete;
};
inline float extraDb(float retainedFraction,int strength){
    if(!std::isfinite(retainedFraction)||strength<=0)return 0;
    retainedFraction=(std::fmax)(0.001f,(std::fmin)(1.0f,retainedFraction));
    float value=20.0f*std::log10(retainedFraction)*static_cast<float>((std::fmin)(100,strength))*0.01f;
    // Quarter-dB steps avoid rebuilding for insignificant numeric changes.
    return std::round(value*4)*0.25f;
}
}
