#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include "GameBindings.h"

namespace attendance::agents {
inline bool musician(unsigned type){return type>=1&&type<=8;}
inline bool regular(unsigned type){return type==0||type==9;}
struct Tags {
    bool verified=false;
    std::vector<unsigned char> type,side,fixed;
    unsigned band=0;
    bool protectedBand(size_t i)const{return verified&&i<type.size()&&musician(type[i]);}
    bool protectedFan(size_t i)const{return verified&&i<fixed.size()&&fixed[i]!=0;}
    bool protectedGroup(size_t i)const{return protectedBand(i)||protectedFan(i);}
};
inline Tags decode(const std::vector<unsigned char>& records,size_t count){
    Tags out;if(count<512||count>200000||records.size()!=count*10)return out;
    out.type.resize(count);out.side.resize(count);out.fixed.assign(count,0);
    for(size_t i=0;i<count;++i){unsigned t=records[i*10+6]&31;
        if(t>11)return {}; // Unrecognized native layout/type: preserve old behavior.
        out.type[i]=static_cast<unsigned char>(t);out.side[i]=records[i*10]&1;out.fixed[i]=t==10?1:0;if(musician(t))++out.band;
    }
    out.verified=true;return out;
}
inline void protectStudents(Tags& tags,bool homeMilitary,bool awayMilitary){
    if(!tags.verified)return;
    for(size_t i=0;i<tags.type.size();++i)if(tags.type[i]==9&&(tags.side[i]?awayMilitary:homeMilitary))tags.fixed[i]=1;
}
// Read once when preparing a renderer. Never inspect native tag streams inside
// the per-agent visibility callback; these arrays can retire with their scene.
template<class Read> Tags readScene(Read read,uintptr_t image,uintptr_t scene,size_t count){
    auto q=[&](uintptr_t p){uintptr_t v=0;return read(p,&v,sizeof(v))?v:uintptr_t(0);};
    if(count<512||count>200000||q(scene)!=image+bindings::SceneCrowdVtable)return {};
    uintptr_t component=q(scene+0x908),map=q(scene+0x9f8);
    if(q(component)!=image+0xb484f98||q(component+0x48)!=map)return {};
    uintptr_t begin=q(map),end=q(map+8),value=0;
    if(begin<0x10000||end<=begin||(end-begin)%16||end-begin>128*16)return {};
    struct Entry {uintptr_t key,value;};std::vector<Entry> entries((end-begin)/16);
    if(!read(begin,entries.data(),entries.size()*sizeof(Entry)))return {};
    for(const auto& e:entries)if(e.key==image+0xb486033){if(value)return {};value=e.value;}
    uintptr_t holder=q(value+8),data=q(holder+8);
    if(!value||q(holder)!=count||data<0x10000)return {};
    std::vector<unsigned char> records(count*10);
    if(!read(data,records.data(),records.size()))return {};
    if(q(scene+0x908)!=component||q(component+0x48)!=map||q(value+8)!=holder||q(holder)!=count||q(holder+8)!=data)return {};
    return decode(records,count);
}
}
