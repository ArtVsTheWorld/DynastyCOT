#define WIN32_LEAN_AND_MEAN
#include "../Source/NativeCapacity.h"
#include "../Source/AttendanceAudio.h"
#include <cassert>
#include <cstdio>
#include <vector>
using namespace attendance;
using A=uintptr_t;
constexpr A image=0x140000000;
unsigned char service[16]{},vtable[0x80]{},weather[0x400]{},matchData[16]{},wrapper[32]{};
int nativeAttendance=9000,nativeCapacity=30000,code=0;unsigned queries=0,lookups=0;bool queryOkay=true,moveMatch=false,moveStadium=false,badPointer=false;
template<class T>void put(A p,T value){memcpy(reinterpret_cast<void*>(p),&value,sizeof(value));}
template<size_t N>A addr(unsigned char (&p)[N]){return reinterpret_cast<A>(p);}
bool __fastcall query(A object,unsigned key,occupancy::Variant* value,A a,A b){
    ++queries;assert(object==addr(service)&&key==bindings::AttendanceOption&&!a&&!b&&value->vt==image+0xb135818);
    value->data=badPointer?1:reinterpret_cast<A>(&nativeAttendance);return queryOkay;
}
int __fastcall capacity(int stadium,int* out){
    ++lookups;assert(stadium==501);*out=nativeCapacity;if(moveMatch)put(addr(wrapper)+16,A(0));if(moveStadium)put(addr(weather)+0x358,502);return code;
}
int main(){
    std::vector<void*> pages;
    for(A offset:{A(0xe2e53e8),A(0xf506000),A(0xf85f700),A(0xfc90fb8)}){
        A p=image+(offset&~A(0xffff));void* page=VirtualAlloc(reinterpret_cast<void*>(p),0x10000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);assert(reinterpret_cast<A>(page)==p);pages.push_back(page);
    }
    put(image+0xe2e53e8,addr(service));put(addr(service),image+0xb29ee30);
    // Native service vtable stays verified; the injected API replaces only calls.
    void* svcPage=VirtualAlloc(reinterpret_cast<void*>(image+0xb290000),0x10000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);assert(svcPage);pages.push_back(svcPage);
    put(image+0xb29ee30+0x68,image+bindings::OptionQuery);
    put(image+0xf506000,addr(weather));put(addr(weather),image+0xb32a290);put(addr(weather)+0x358,501);
    put(image+bindings::MatchWrapper,addr(wrapper));put(addr(wrapper)+16,addr(matchData));put(addr(matchData),image+bindings::MatchVtable);put(image+0xfc90fb8,A(123));
    occupancy::Api api{query,capacity};occupancy::Snapshot sample;
    assert(occupancy::nativeSnapshot(image,addr(matchData),7,sample,&api));assert(sample.attendance==9000&&sample.capacity==30000&&sample.stadium==501);
    assert(std::fabs(occupancy::fraction(sample,addr(matchData),7,.8f)-.24f)<.00001f);
    assert(balance::crowdBoost(occupancy::fraction(sample,addr(matchData),7,1))==0);
    assert(occupancy::fraction(sample,addr(matchData),8,.8f)==.8f); // A recycled match address cannot reuse the previous epoch's ratio.
    assert(occupancy::fraction(sample,999,7,.8f)==.8f);
    assert(occupancy::fraction(sample,addr(matchData),7,NAN)==0&&occupancy::fraction(sample,addr(matchData),7,-1)==0);
    auto saved=sample;nativeAttendance=30001;assert(!occupancy::nativeSnapshot(image,addr(matchData),7,sample,&api)&&sample.attendance==saved.attendance);
    nativeAttendance=-1;assert(!occupancy::nativeSnapshot(image,addr(matchData),7,sample,&api));nativeAttendance=9000;
    nativeCapacity=0;assert(!occupancy::nativeSnapshot(image,addr(matchData),7,sample,&api));nativeCapacity=30000;
    code=1;assert(!occupancy::nativeSnapshot(image,addr(matchData),7,sample,&api));code=0;
    queryOkay=false;auto oldLookups=lookups;assert(!occupancy::nativeSnapshot(image,addr(matchData),7,sample,&api)&&lookups==oldLookups);queryOkay=true;
    badPointer=true;assert(!occupancy::nativeSnapshot(image,addr(matchData),7,sample,&api));badPointer=false;
    moveMatch=true;assert(!occupancy::nativeSnapshot(image,addr(matchData),7,sample,&api));moveMatch=false;put(addr(wrapper)+16,addr(matchData));
    moveStadium=true;assert(!occupancy::nativeSnapshot(image,addr(matchData),7,sample,&api));moveStadium=false;put(addr(weather)+0x358,501);
    put(addr(weather)+0x358,1023);auto oldQueries=queries;assert(!occupancy::nativeSnapshot(image,addr(matchData),7,sample,&api)&&queries==oldQueries);put(addr(weather)+0x358,501);
    nativeAttendance=0;assert(occupancy::nativeSnapshot(image,addr(matchData),7,sample,&api)&&sample.peak()==0);
    sample={123,1,501,60000,100000};assert(std::fabs(occupancy::fraction(sample,123,1,.8f)-.48f)<.00001f);
    for(int size:{5000,30000,100000})for(int count:{0,1,25,50,75,100})for(int retained:{0,30,60,80,100}){
        sample={123,1,501,size*count/100,size};float f=occupancy::fraction(sample,123,1,retained*.01f);
        assert(f>=0&&f<=1&&std::fabs(f-count*.01f*retained*.01f)<.00021f);assert(balance::crowdBoost(f)<=25&&balance::mix(f,6060).net(1)==balance::mix(f,6060).net(2));
    }
    for(void* page:pages)assert(VirtualFree(page,0,MEM_RELEASE));
    std::puts("PASS: native attendance option/capacity lookup, verified ownership/types, read faults and DB failures, transition rejection, zero crowds, exact occupancy math and recycled-match isolation.");
}
