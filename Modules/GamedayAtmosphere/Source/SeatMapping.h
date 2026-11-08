#pragma once
#include <cstdint>
#include <vector>
#include "GameBindings.h"
#include "SeatPattern.h"
namespace attendance::seats {
struct Mapping {uintptr_t holder=0,positions=0;size_t count=0;};
template<class Read>
bool mapping(Read read,uintptr_t image,uintptr_t scene,uintptr_t renderer,uintptr_t masks,size_t count,Mapping& output){
    using namespace attendance::bindings;
    if(scene<0x10000||renderer<0x10000||masks<0x10000||count<512||count>200000)return false;
    auto pointer=[&](uintptr_t address){uintptr_t value=0;if(!read(address,&value,sizeof(value)))return uintptr_t(0);return value;};
    if(pointer(scene)!=image+SceneCrowdVtable||pointer(scene+0x9f0)!=renderer||pointer(renderer)!=image+CrowdRendererVtable||
       pointer(renderer+0x188)!=masks||pointer(renderer+0x190)!=masks+count*2)return false;
    uintptr_t properties=pointer(scene+0x9f8),begin=pointer(properties),end=pointer(properties+8);
    if(begin<0x10000||end<begin||(end-begin)%16||end-begin>128*16)return false;
    struct Entry {uintptr_t key,value;};std::vector<Entry> entries((end-begin)/16);
    if(entries.empty()||!read(begin,entries.data(),entries.size()*sizeof(Entry)))return false;
    Mapping found;unsigned matches=0;
    for(const auto& entry:entries)if(entry.key==image+CrowdPositionKey){
        uintptr_t holder=pointer(entry.value+8),data=pointer(holder+8);
        if(holder<0x10000||data<0x10000||pointer(holder)!=count||data>0x7fffffffffff-count*sizeof(Position))return false;
        found={holder,data,count};++matches;
    }
    if(matches!=1||pointer(scene+0x9f0)!=renderer||pointer(renderer+0x188)!=masks)return false;
    output=found;return true;
}
template<class Read>
bool positions(Read read,uintptr_t image,uintptr_t scene,uintptr_t renderer,uintptr_t masks,size_t count,std::vector<Position>& output){
    Mapping before,after;if(!mapping(read,image,scene,renderer,masks,count,before))return false;
    std::vector<Position> values(count);if(!read(before.positions,values.data(),count*sizeof(Position)))return false;
    if(!mapping(read,image,scene,renderer,masks,count,after)||before.holder!=after.holder||before.positions!=after.positions||before.count!=after.count)return false;
    output=std::move(values);return true;
}
}
