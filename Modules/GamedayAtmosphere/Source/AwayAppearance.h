#pragma once
#include <windows.h>
#include <intrin.h>
#include "AwayCrowd.h"

namespace attendance::visitors {
using Address=uintptr_t;
inline constexpr Address Generation=0x3628a00;
inline const std::vector<unsigned char> GenerationPrefix{0x4c,0x8b,0xdc,0x55,0x53,0x57,0x49,0x8d,0xab,0xc8,0xfc,0xff,0xff,0x48,0x81,0xec,0x20,0x04,0,0};
struct Stream {Address value=0,holder=0,data=0;size_t count=0;};
template<class Read> Stream property(Read read,Address image,Address map,Address key){
    auto q=[&](Address p){Address v=0;return read(p,&v,sizeof(v))?v:Address(0);};
    Address begin=q(map),end=q(map+8);Stream found;
    if(begin<0x10000||end<=begin||(end-begin)%16||end-begin>128*16)return {};
    struct Entry {Address key,value;};std::vector<Entry> entries((end-begin)/16);
    if(!read(begin,entries.data(),entries.size()*sizeof(Entry)))return {};
    for(const auto& e:entries)if(e.key==image+key){
        if(found.value)return {};found.value=e.value;found.holder=q(e.value+8);
        found.count=q(found.holder);found.data=q(found.holder+8);
    }
    return found;
}
inline bool writable(Address address,size_t bytes){
    if(address<0x10000||!bytes||address>UINTPTR_MAX-bytes)return false;
    Address end=address+bytes;
    while(address<end){MEMORY_BASIC_INFORMATION m{};
        if(!VirtualQuery(reinterpret_cast<void*>(address),&m,sizeof(m))||m.State!=MEM_COMMIT||
           (m.Protect&0xff)!=PAGE_READWRITE||(m.Protect&PAGE_GUARD))return false;
        Address next=reinterpret_cast<Address>(m.BaseAddress)+m.RegionSize;
        if(next<=address)return false;address=(std::min)(next,end);
    }return true;
}
// Change the single native team-appearance bit atomically; don't copy back a
// whole packed record and overwrite unrelated native flags or instrument tags.
inline bool writeSides(Address data,const agents::Tags& tags,const std::vector<unsigned char>& sides)noexcept{
    if(tags.type.size()!=sides.size()||!writable(data,tags.type.size()*10))return false;
    __try{
        for(size_t i=0;i<tags.type.size();++i)if(agents::regular(tags.type[i])&&!tags.protectedFan(i)){
            auto p=reinterpret_cast<volatile char*>(data+i*10);
            char current=*p;
            for(;;){char wanted=static_cast<char>((static_cast<unsigned char>(current)&0xfe)|(sides[i]&1));
                char observed=_InterlockedCompareExchange8(p,wanted,current);if(observed==current)break;current=observed;}
        }return true;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
struct AppearanceResult {
    bool applied=false,nativeFallback=false;unsigned positions=0,band=0,visitors=0;
    bool occupiedVerified=false;unsigned spectators=0,occupiedVisitors=0;
    Address component=0,map=0,coordinates=0;Stream stream;
    agents::Tags native;std::vector<unsigned char> desired;
};
// Restore only the one bit we temporarily owned, preserving other native
// writes. If native generation changed the side itself, leave that new value.
inline bool restoreSides(Address data,const agents::Tags& tags,const std::vector<unsigned char>& desired)noexcept{
    if(tags.side.size()!=desired.size()||!writable(data,desired.size()*10))return false;
    __try{
        for(size_t i=0;i<desired.size();++i)if(agents::regular(tags.type[i])&&!tags.protectedFan(i)){
            auto p=reinterpret_cast<volatile char*>(data+i*10);char current=*p;
            for(;;){if((static_cast<unsigned char>(current)&1)!=(desired[i]&1))break;
                char wanted=static_cast<char>((static_cast<unsigned char>(current)&0xfe)|(tags.side[i]&1));
                char observed=_InterlockedCompareExchange8(p,wanted,current);if(observed==current)break;current=observed;}
        }return true;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
template<class Read> bool restoreAppearance(Read read,Address image,const AppearanceResult& prepared){
    if(!prepared.applied)return true;
    auto q=[&](Address p){Address v=0;return read(p,&v,sizeof(v))?v:Address(0);};
    if(q(prepared.component)!=image+0xb484f98||q(prepared.component+0x48)!=prepared.map)return false;
    auto current=property(read,image,prepared.map,0xb486033);const auto& previous=prepared.stream;
    if(current.value!=previous.value||current.holder!=previous.holder||current.data!=previous.data||current.count!=previous.count)return false;
    return restoreSides(previous.data,prepared.native,prepared.desired);
}
// A generated outfit outlives its temporary source selector. Retain only its
// supporter identity for DGA's population/departure model, not a game object.
struct GeneratedAppearance {
    Address component=0,map=0,data=0,coordinates=0;int home=-1,away=-1;
    std::vector<unsigned char> side;
    template<class Read> bool apply(Read read,Address image,Address scene,int homeTeam,int awayTeam,agents::Tags& tags)const{
        if(home!=homeTeam||away!=awayTeam||!tags.verified||tags.side.size()!=side.size())return false;
        auto q=[&](Address p){Address v=0;return read(p,&v,sizeof(v))?v:Address(0);};
        if(q(scene+0x908)!=component||q(component)!=image+0xb484f98||q(component+0x48)!=map)return false;
        auto current=property(read,image,map,0xb486033),points=property(read,image,map,bindings::CrowdPositionKey);
        if(current.data!=data||current.count!=side.size()||points.data!=coordinates||points.count!=side.size())return false;
        tags.side=side;return true;
    }
};
// Called only as part of EA's own native generation callback, never from a
// worker or per-seat visibility callback. Loaded palettes, jerseys and crests
// remain EA's responsibility, including TeamBuilder assets and neutral designs.
template<class Read> AppearanceResult appearance(Read read,Address image,Address component,const Plan& plan,unsigned seed,bool homeMilitary=false,bool awayMilitary=false,const std::vector<unsigned char>* occupied=nullptr,int singleSchoolSide=-1){
    AppearanceResult result;
    if(singleSchoolSide<0&&(!plan.valid||plan.native))return result;
    if(singleSchoolSide>1)return result;
    auto q=[&](Address p){Address v=0;return read(p,&v,sizeof(v))?v:Address(0);};
    if(q(component)!=image+0xb484f98)return result;
    Address map=q(component+0x48);auto tags=property(read,image,map,0xb486033),coords=property(read,image,map,bindings::CrowdPositionKey);
    if(tags.count<512||tags.count>200000||coords.count!=tags.count)return result;
    std::vector<unsigned char> raw(tags.count*10);
    std::vector<seats::Position> positions(tags.count);
    if(!read(tags.data,raw.data(),raw.size())||!read(coords.data,positions.data(),positions.size()*sizeof(seats::Position)))return result;
    auto types=agents::decode(raw,tags.count);if(!types.verified)return result;
    agents::protectStudents(types,homeMilitary,awayMilitary);
    std::vector<unsigned> ordinary;for(unsigned i=0;i<types.type.size();++i)if(agents::regular(types.type[i]))ordinary.push_back(i);
    if(!seats::plausible(positions,ordinary))return result;
    std::vector<unsigned char> desired;
    if(singleSchoolSide>=0){desired=types.side;for(unsigned i:ordinary)if(!types.protectedFan(i))desired[i]=static_cast<unsigned char>(singleSchoolSide);}
    else desired=assign(positions,types,plan.share,seed,occupied,plan.layoutVersion,plan.ceiling==.70,&result.nativeFallback);
    if(desired.size()!=tags.count)return result;
    auto current=property(read,image,map,0xb486033);
    if(q(component+0x48)!=map||current.value!=tags.value||current.holder!=tags.holder||current.data!=tags.data||current.count!=tags.count)return result;
    if(!writeSides(tags.data,types,desired)){
        if(q(component+0x48)==map&&q(tags.holder+8)==tags.data)restoreSides(tags.data,types,desired);
        return result;
    }
    result.applied=true;result.positions=static_cast<unsigned>(tags.count);result.band=types.band;
    result.occupiedVerified=occupied!=nullptr;
    if(occupied)for(unsigned i=0;i<tags.count;++i)if((*occupied)[i]&&!types.protectedBand(i)){++result.spectators;result.occupiedVisitors+=desired[i];}
    for(unsigned i:ordinary)result.visitors+=desired[i];
    result.component=component;result.map=map;result.coordinates=coords.data;result.stream=tags;
    result.native=std::move(types);result.desired=std::move(desired);return result;
}
}
