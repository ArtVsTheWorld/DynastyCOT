#include "../Source/SeatMapping.h"
#include <cassert>
#include <iostream>
#include <map>
using namespace attendance;
struct Memory {
    std::map<uintptr_t,unsigned char> bytes;
    template<class T>void put(uintptr_t p,T value){auto raw=reinterpret_cast<const unsigned char*>(&value);for(size_t k=0;k<sizeof(T);++k)bytes[p+k]=raw[k];}
    bool read(uintptr_t p,void* out,size_t n)const{auto b=static_cast<unsigned char*>(out);for(size_t k=0;k<n;++k){auto i=bytes.find(p+k);if(i==bytes.end())return false;b[k]=i->second;}return true;}
};
int main(){
    std::vector<seats::Position> xyz;
    for(int side:{-1,1})for(int row=0;row<32;++row)for(int seat=0;seat<24;++seat)
        xyz.push_back({side*(28.0f+row*.9f),1.0f+row*.65f,-48.0f+seat*4.0f,0});
    std::vector<unsigned short> baseline(xyz.size());constexpr unsigned short parts=3;
    for(unsigned i=0;i<baseline.size();++i)if(i%7==0)baseline[i]=parts;
    auto pattern=seats::build(xyz,baseline,parts);assert(pattern.policy==seats::Policy::Position);
    auto repeat=seats::build(xyz,baseline,parts);assert(repeat.rank==pattern.rank);
    unsigned original=0;for(auto v:baseline)original+=v!=parts;assert(pattern.original==original);
    for(int loss=0;loss<=100;++loss){unsigned hidden=0;
        for(unsigned i=0;i<xyz.size();++i){bool selected=pattern.departed(i,loss);hidden+=selected;
            if(loss)assert(!pattern.departed(i,loss-1)||selected);
            if(baseline[i]==parts)assert(!selected);
        }
        assert(hidden==static_cast<uint64_t>(original)*static_cast<unsigned>(loss)/100);
    }
    assert(!pattern.departed(0,-1));assert(pattern.departed(1,200));assert(!pattern.departed(100000,50));
    std::vector<bool> current(xyz.size()),owned(xyz.size());
    auto update=[&](int permanent,int temporary,bool historical=false,int historicalLoss=0,bool restore=false){
        unsigned visible=0;
        for(unsigned i=0;i<xyz.size();++i){bool native=baseline[i]==parts,left=pattern.departed(i,permanent),held=pattern.departed(i,temporary);
            bool result=hidePart(native,native,current[i],left||held,restore,true,owned[i]);
            if(!restore&&historical)result=historicalSelection(native,native,pattern.departed(i,historicalLoss));
            if(held&&!left&&!native)owned[i]=true;else if(!held)owned[i]=false;
            current[i]=result;visible+=!result;if(native)assert(result);
        }return visible;
    };
    assert(update(0,55)==original-original*55/100);
    assert(update(0,25)==original-original*25/100);assert(update(0,0)==original);
    assert(update(35,45)==original-original*45/100);assert(update(35,35)==original-original*35/100);
    assert(update(35,0)==original-original*35/100);
    for(int historical:{5,70,20,0,55,10})assert(update(35,0,true,historical)==original-original*historical/100);
    assert(update(35,0,true,35)==original-original*35/100);
    assert(update(35,55,true,55)==original-original*55/100);
    assert(update(35,35,true,35)==original-original*35/100);
    assert(update(35,0,true,35,true)==original); // Release old masks before a new scenario, even after exact replay.
    auto resumed=seats::build(xyz,baseline,parts);assert(resumed.rank==pattern.rank);
    VisibilityCount tally;for(unsigned i=0;i<xyz.size();++i){bool selected=pattern.departed(i,35);auto mask=static_cast<unsigned short>(baseline[i]|(selected?parts:0));countSelection(tally,parts,baseline[i],mask,baseline[i],selected);}
    assert(tally.original==original&&tally.visible==original-original*35/100&&tally.visible==tally.expected&&tally.otherExtra==0&&tally.extraVisible==0);
    auto other=xyz;for(auto& p:other)p.y+=15;auto deck=seats::build(other,baseline,parts);assert(deck.policy==seats::Policy::Position);
    for(auto bad:{std::vector<seats::Position>(10),std::vector<seats::Position>(1024)}){
        auto fallback=seats::build(bad,std::vector<unsigned short>(bad.size()),parts);assert(fallback.policy==seats::Policy::Scattered);
        for(unsigned i=0;i<bad.size();++i)assert(fallback.departed(i,35)==departed(i,35));
    }
    auto bad=xyz;bad[1].x=NAN;assert(seats::build(bad,baseline,parts).policy==seats::Policy::Scattered);
    assert(seats::build(xyz,{},parts).policy==seats::Policy::Scattered);
    std::vector<unsigned short> empty(xyz.size(),parts);assert(seats::build(xyz,empty,parts).policy==seats::Policy::Scattered);
    Memory m;uintptr_t image=0x140000000,scene=0x20000,renderer=0x30000,masks=0x40000,props=0x50000,begin=0x60000,value=0x70000,holder=0x80000,data=0x90000;size_t count=1024;
    m.put(scene,image+bindings::SceneCrowdVtable);m.put(scene+0x9f0,renderer);m.put(renderer,image+bindings::CrowdRendererVtable);m.put(renderer+0x188,masks);m.put(renderer+0x190,masks+count*2);
    m.put(scene+0x9f8,props);m.put(props,begin);m.put(props+8,begin+16);m.put(begin,image+bindings::CrowdPositionKey);m.put(begin+8,value);m.put(value+8,holder);m.put(holder,count);m.put(holder+8,data);
    for(size_t i=0;i<count;++i)m.put(data+i*16,xyz[i]);
    auto read=[&](uintptr_t p,void* out,size_t n){return m.read(p,out,n);};seats::Mapping mapping;std::vector<seats::Position> copied;
    assert(seats::mapping(read,image,scene,renderer,masks,count,mapping));assert(seats::positions(read,image,scene,renderer,masks,count,copied)&&copied.size()==count);
    m.put(holder,count-1);assert(!seats::positions(read,image,scene,renderer,masks,count,copied));m.put(holder,count);
    m.put(props+8,begin+17);assert(!seats::mapping(read,image,scene,renderer,masks,count,mapping));m.put(props+8,begin+16);
    bool switched=false;auto retiring=[&](uintptr_t p,void* out,size_t n){bool ok=m.read(p,out,n);if(p==data){m.put(renderer+0x188,masks+16);switched=true;}return ok;};
    assert(!seats::positions(retiring,image,scene,renderer,masks,count,copied)&&switched);
    std::cout<<"PASS: deterministic area pattern, exact nested cuts, original sparse seats, arrivals/halftime, replay seeks/restoration, resumed scenarios, multiple renderers, fallbacks and retiring mapping.\n";
}
