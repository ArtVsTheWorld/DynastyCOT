#define WIN32_LEAN_AND_MEAN
#include "../Source/NativeAtmosphereContext.h"
#include "../Source/ArrivalRules.h"
#include <cassert>
#include <array>
#include "../Source/DepartureModel.h"
#include <cstdio>
using namespace attendance;
using sustained::Address;
constexpr Address image=0x140000000;
template<class T>void put(Address p,T v){memcpy(reinterpret_cast<void*>(p),&v,sizeof(v));}
Address addr(void* p){return reinterpret_cast<Address>(p);}
int main(){
    void* a=VirtualAlloc(reinterpret_cast<void*>(image+(bindings::MatchVisualManager&~Address(0xffff))),0x10000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    void* b=VirtualAlloc(reinterpret_cast<void*>(image+(bindings::HfaSelection&~Address(0xffff))),0x10000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);assert(a&&b);
    unsigned char manager[0x100]{},reference[32]{},visual[64]{},meter[0x100]{},selection[0x200]{},entry[64]{};Address list[]={addr(entry)};
    put(image+bindings::MatchVisualManager,addr(manager));put(addr(manager)+0x20,addr(reference));put(addr(reference)+8,addr(visual));put(addr(visual),image+bindings::MatchVisualVtable);
    bool conference=true;assert(nativecontext::conference(image,false,false,conference)&&!conference);put<unsigned char>(addr(visual)+0x27,1);assert(nativecontext::conference(image,false,false,conference)&&conference);
    State cc;cc.quarter=1;cc.quarterSeconds=720;cc.seconds=720;cc.team0=100;postseason(cc,false,conference);State cfp=cc;cfp.conferenceChampionship=false;cfp.bowl=37;
    assert(cc.playoff&&lossCeiling(cc)==40&&lossCeiling(cfp)==40);cc.precipitation=1;assert(lossCeiling(cc)==50);cc.precipitation=0;
    arrivals::Settings settings;assert(arrivals::baselinePercent(settings,cc)==arrivals::baselinePercent(settings,cfp));assert(arrivals::finishFraction(settings,cc)==arrivals::finishFraction(settings,cfp));
    History history;Settings retention;
    for(int q=2;q<=4;++q)for(int margin:{0,14,21,35,49})for(int weather:{0,1,2}){
        cc.quarter=cfp.quarter=q;cc.precipitation=cfp.precipitation=weather;cc.homeScore=cfp.homeScore=0;cc.awayScore=cfp.awayScore=margin;cc.seconds=cfp.seconds=360;
        float elapsed=elapsedGameTime(cc);assert(std::fabs(departures::rate(retention,cc,history,elapsed)-departures::rate(retention,cfp,history,elapsed))<.0001);
        assert(context::halftime(cc).temporary==context::halftime(cfp).temporary);
    }
    cc.bowl=47;assert(!championship(cc)&&lossCeiling(cc)>=40); // Conference games never inherit the national-title floor.
    assert(!nativecontext::conference(image,true,false,conference));put<unsigned char>(addr(visual)+0x28,1);assert(!nativecontext::conference(image,false,false,conference));put<unsigned char>(addr(visual)+0x28,0);put<unsigned char>(addr(visual)+0x27,2);assert(!nativecontext::conference(image,false,false,conference));
    put(image+bindings::HfaSelection,addr(selection));put(addr(selection),image+bindings::HfaSelectionVtable);put(image+bindings::HfaMeter,addr(meter));put(addr(meter),image+bindings::HfaMeterVtable);put(addr(meter)+0x80,reinterpret_cast<Address>(list));put(addr(meter)+0x88,reinterpret_cast<Address>(list)+8);
    put(addr(entry)+0x18,1.f);put(addr(entry)+0x1c,2);put(addr(entry)+0x20,0);auto original=std::array<unsigned char,sizeof(meter)>{};memcpy(original.data(),meter,sizeof(meter));assert(nativecontext::hfa(image));assert(!memcmp(original.data(),meter,sizeof(meter)));
    put<unsigned char>(addr(selection)+0x1f0,1);assert(!nativecontext::hfa(image));put<unsigned char>(addr(selection)+0x1f0,0);put(addr(entry)+0x20,-1);assert(!nativecontext::hfa(image));
    assert(VirtualFree(a,0,MEM_RELEASE)&&VirtualFree(b,0,MEM_RELEASE));
    std::puts("PASS: dedicated conference flag, inconsistent/unknown data rejection, playoff-equivalent arrivals/floors and read-only native HFA observation without camera feedback.");
}
