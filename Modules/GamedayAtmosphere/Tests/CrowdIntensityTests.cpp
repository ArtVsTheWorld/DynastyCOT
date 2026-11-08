#define WIN32_LEAN_AND_MEAN
#include "../Source/CrowdIntensityRuntime.h"
#include <cassert>
#include <cstdio>
using namespace attendance::sustained;
constexpr Address image=0x140000000;
unsigned locks=0,unlocks=0,publishes=0;float last=0;bool fault=false,reentrant=false;
IntensityRuntime* recursive=nullptr;Intent intent;Address current=0;uint64_t timeValue=1000;
template<class T>void put(Address p,T value){memcpy(reinterpret_cast<void*>(p),&value,sizeof(value));}
void __fastcall lock(Address p){assert(p==current+0x90);++locks;}
void __fastcall unlock(Address p){assert(p==current+0x90);++unlocks;}
void __fastcall publish(Address p,const float* value,bool force){
    assert(p==current+0x80&&force&&locks==unlocks+1);last=*value;++publishes;
    if(reentrant)recursive->tick(current,intent,123,1,timeValue);
    if(fault){fault=false;RaiseException(EXCEPTION_ACCESS_VIOLATION,0,0,nullptr);}
}
int main(){
    Address entity=reinterpret_cast<Address>(VirtualAlloc(nullptr,0x1000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));assert(entity);
    unsigned char asset[0x100]{};Address a=reinterpret_cast<Address>(asset);
    put(entity,image+attendance::bindings::IntensityVtable);put(entity+0x38,a);put(entity+0x80,Address(0x12345));put(entity+0xc4,200.0f);
    put(a+0x3c,0.0f);put(a+0x40,1000.0f);
    IntensityRuntime runtime;runtime.configure(image,lock,unlock,publish);recursive=&runtime;current=entity;
    intent.match=123;intent.epoch=1;intent.active=true;intent.intensity=1000;
    auto tick=[&](uint64_t now){timeValue=now;intent.updated=now;runtime.tick(current,intent,123,1,now);assert(locks==unlocks);};
    reentrant=true;for(uint64_t now=1000;now<=3100;now+=100)tick(now);reentrant=false;
    assert(std::fabs(last-1000)<.01f&&runtime.native()==200);
    assert(at<float>(entity+0xc4)==200); // Native envelope and timers were never changed.
    put(entity+0xc4,990.0f);intent.intensity=800;tick(3200);assert(last==990); // Never suppress a native eruption.
    intent.intensity=1000;tick(3300);put(entity+0xc4,340.0f);
    runtime.tick(current,intent,123,1,4100);assert(last==340&&runtime.restores()>=1&&locks==unlocks); // Expired lease restores contemporaneous native intensity.
    intent.active=true;for(uint64_t now=4200;now<=6400;now+=100)tick(now);assert(last==1000);
    intent.active=false;tick(6500);assert(last==340); // Pause, replay, disabled audio and unsupported context share this gate.
    put(entity+0xc4,1000.107f);tick(6550);assert(runtime.native()>1000&&runtime.output()==runtime.native()); // A fractional native overshoot must still permit restoration.
    put(entity+0xc4,340.0f);
    intent.active=true;for(uint64_t now=6600;now<=8800;now+=100)tick(now);
    put(entity+0x80,Address(0x23456));put(entity+0xc4,700.0f);unsigned before=publishes;tick(8900);assert(publishes==before); // Fresh binding starts from native.
    tick(9000);assert(last>=700&&last<=1000);
    fault=true;tick(9100);assert(!runtime.healthy()&&locks==unlocks);
    put(entity+0xc4,410.0f);tick(9200);assert(last==410&&locks==unlocks); // A publish fault still restores safely on its owning callback.
    assert(VirtualFree(reinterpret_cast<void*>(entity),0,MEM_RELEASE));
    current=reinterpret_cast<Address>(VirtualAlloc(nullptr,0x1000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));assert(current);
    put(current,image+attendance::bindings::IntensityVtable);put(current+0x38,a);put(current+0x80,Address(0x34567));put(current+0xc4,250.0f);
    tick(9400);assert(runtime.native()==250&&runtime.output()==250); // No access to the retired object.
    IntensityRuntime reaction;reaction.configure(image,lock,unlock,publish);intent.active=true;intent.reason=Reason::Result;intent.intensity=1000;
    intent.updated=9500;reaction.tick(current,intent,123,1,9500);
    intent.updated=9750;reaction.tick(current,intent,123,1,9750);assert(last==1000&&at<float>(current+0xc4)==250);
    put(current+0xc4,1000.1f);intent.intensity=800;intent.updated=9800;reaction.tick(current,intent,123,1,9800);assert(last==1000.1f); // A natural cheer always keeps its stronger intensity.
    intent.active=false;intent.updated=9900;reaction.tick(current,intent,123,1,9900);assert(last==1000.1f&&locks==unlocks);
    IntensityRuntime lineup;lineup.configure(image,lock,unlock,publish);put(current+0xc4,400.0f);intent.active=true;intent.reason=Reason::Lineup;intent.intensity=950;
    intent.updated=10000;lineup.tick(current,intent,123,1,10000);intent.updated=10250;lineup.tick(current,intent,123,1,10250);
    assert(last==950&&at<float>(current+0xc4)==400); // The early cheer has excited voices before the slower roar attack finishes.
    intent.active=false;intent.updated=10300;lineup.tick(current,intent,123,1,10300);assert(last==400&&locks==unlocks);
    // Even a tiny/sparse venue cannot impose an excitement ceiling now.
    IntensityRuntime nativeOnly;nativeOnly.configure(image,lock,unlock,publish);intent.active=false;
    for(float value:{0.f,99.f,350.f,700.f,1000.f,1000.1f}){
        put(current+0xc4,value);nativeOnly.tick(current,intent,123,1,10000);
        assert(nativeOnly.native()==value&&nativeOnly.output()==value&&at<float>(current+0xc4)==value);
    }
    assert(VirtualFree(reinterpret_cast<void*>(current),0,MEM_RELEASE));
    std::puts("PASS: upward-only earned excitement, native floors, lease restoration, no sparse/size limiter, reentrancy and fault cleanup.");
}
