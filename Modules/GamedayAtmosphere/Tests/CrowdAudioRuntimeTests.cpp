#define WIN32_LEAN_AND_MEAN
#include "../Source/CrowdAudioRuntime.h"
#include <cassert>
#include <cstring>
#include <vector>
#include <cstdio>
using namespace attendance::audio;
constexpr Address base=0x140000000;
Wanted wanted;unsigned long long timeValue=1000,originalCalls=0;
struct Message {Address queue;float value;};std::vector<Message> messages;
Wanted productionRequest(){return wanted;}
Wanted request(){auto custom=wanted;custom.reductionStart=.8f;return custom;}
unsigned long long clockValue(){return timeValue;}
Address __fastcall original(Address,Address a,Address b,Address c){assert(a==11&&b==22&&c==33);++originalCalls;return 0x12345678;}
void __fastcall post(Address queue,const Parameter* p){assert(p->id==0x9f7d7b40);messages.push_back({queue,p->value});}
unsigned faultCalls=0;
void __fastcall faultPost(Address,const Parameter*){++faultCalls;RaiseException(EXCEPTION_ACCESS_VIOLATION,0,0,nullptr);}
template<class T>void put(Address address,T value){memcpy(reinterpret_cast<void*>(address),&value,sizeof(value));}
Address make(){
    auto p=reinterpret_cast<Address>(VirtualAlloc(nullptr,0x1000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));assert(p);
    put(p,base+0xb227098);put(p+0x40,base+0xb227178);put(p+0x48,base+0xb227188);put<int>(p+0x1f4,100);return p;
}
void expectClose(float a,float b){assert(std::fabs(a-b)<0.00001f);}
int main(){
    Runtime nativeOnly;nativeOnly.configure(base,original,productionRequest,nullptr,post,clockValue);
    Address unchanged=make();wanted={1234,1,true,0.0f,1.0f};
    for(int i=0;i<90;i++){timeValue+=100;nativeOnly.tick(unchanged+0x40,11,22,33);}
    expectClose(messages.back().value,.75f);expectClose(nativeOnly.gain(),.75f); // Hard 25% limit, including zero fullness and an excessive request.
    wanted.remaining=.175f;for(int i=0;i<70;++i){timeValue+=100;nativeOnly.tick(unchanged+0x40,11,22,33);}
    expectClose(messages.back().value,.875f); // Production follows current density, including arrivals and earlier replay clips.
    put<int>(unchanged+0x1f4,50);timeValue+=100;nativeOnly.tick(unchanged+0x40,11,22,33);expectClose(messages.back().value,.4375f);
    put<int>(unchanged+0x1f4,0);timeValue+=100;nativeOnly.tick(unchanged+0x40,11,22,33);expectClose(messages.back().value,0);
    put<int>(unchanged+0x1f4,100);wanted.active=false;timeValue+=100;nativeOnly.tick(unchanged+0x40,11,22,33);expectClose(messages.back().value,1);
    auto restoredCount=messages.size();timeValue+=100;nativeOnly.tick(unchanged+0x40,11,22,33);assert(messages.size()==restoredCount);
    assert(VirtualFree(reinterpret_cast<void*>(unchanged),0,MEM_RELEASE));
    messages.clear();
    originalCalls=0;
    Runtime runtime;runtime.configure(base,original,request,nullptr,post,clockValue,true);
    Address p=make();wanted={1234,1,true,1,.2f};
    auto tick=[&](Address owner){timeValue+=100;assert(runtime.tick(owner+0x40,11,22,33)==0x12345678);};
    tick(p);assert(messages.empty()); // Full crowd does not override native volume.
    wanted.remaining=0.8f;for(int i=0;i<80;i++)tick(p);assert(messages.empty()); // No cut or extra queue traffic at the threshold.
    wanted.remaining=0.4f;for(int i=0;i<80;i++)tick(p);
    assert(!messages.empty());assert(messages.back().queue==p+0x140);expectClose(messages.back().value,0.9f);
    assert(*reinterpret_cast<int*>(p+0x1f4)==100); // User setting was never written.
    size_t count=messages.size();for(int i=0;i<100;i++)tick(p);expectClose(messages.back().value,0.9f);assert(messages.size()>count);
    wanted.remaining=0.9f;tick(p);expectClose(messages.back().value,0.9f); // No comeback refill.
    put<int>(p+0x1f4,50);tick(p);expectClose(messages.back().value,0.45f); // Slider changes use the new base.
    put<int>(p+0x1f4,0);tick(p);expectClose(messages.back().value,0);
    put<int>(p+0x1f4,80);wanted.active=false;
    for(int i=0;i<80;i++)tick(p);expectClose(messages.back().value,0.8f);
    count=messages.size();for(int i=0;i<10;i++)tick(p);assert(messages.size()==count);
    // Old controller memory is gone. A replay/new world supplies its live owner;
    // the runtime must never read or call the retired address from its map.
    assert(VirtualFree(reinterpret_cast<void*>(p),0,MEM_RELEASE));
    Address next=make();wanted={1234,1,true,0.4f,.2f};tick(next);for(int i=0;i<80;i++)tick(next);
    expectClose(messages.back().value,0.9f);assert(messages.back().queue==next+0x140);
    wanted={5678,2,true,1,.2f};tick(next);expectClose(messages.back().value,1); // Fresh match restores full level.
    count=messages.size();put(next+0x48,Address(0));tick(next);assert(messages.size()==count); // Bad owner is ignored.
    assert(originalCalls==runtime.callbacks());
    Runtime gentle;gentle.configure(base,original,request,nullptr,post,clockValue,true);
    put(next+0x48,base+0xb227188);wanted={7890,3,true,0.39f,0.35f};
    gentle.tick(next+0x40,11,22,33);expectClose(messages.back().value,0.820625f);
    wanted.remaining=0.8f;timeValue+=100;gentle.tick(next+0x40,11,22,33);
    expectClose(messages.back().value,0.820625f);
    put<int>(next+0x1f4,50);timeValue+=100;gentle.tick(next+0x40,11,22,33);
    expectClose(messages.back().value,0.5f*0.820625f);
    wanted.strength=0;for(int i=0;i<80;i++){timeValue+=100;gentle.tick(next+0x40,11,22,33);}
    expectClose(messages.back().value,0.5f); // Changing strength never changes the saved slider.
    wanted.strength=0.35f;wanted.remaining=0.39f;wanted.emphasis=1.25f;
    for(int i=0;i<80;i++){timeValue+=100;gentle.tick(next+0x40,11,22,33);}
    expectClose(messages.back().value,0.5f); // Emphasis can soften a cut, never exceed the user's slider.
    wanted.emphasis=1;for(int i=0;i<80;i++){timeValue+=100;gentle.tick(next+0x40,11,22,33);}
    expectClose(messages.back().value,0.5f*0.820625f);
    wanted.emphasis=1.35f;wanted.remaining=1;wanted.epoch++;
    for(int i=0;i<80;i++){timeValue+=100;gentle.tick(next+0x40,11,22,33);}
    expectClose(messages.back().value,0.5f); // Boost respects the player's slider.
    wanted.remaining=0;for(int i=0;i<80;i++){timeValue+=100;gentle.tick(next+0x40,11,22,33);}
    expectClose(messages.back().value,0); // Empty crowd never gets boosted.
    wanted={9100,4,true,1,0.35f,1,0.85f};
    for(int i=0;i<80;i++){timeValue+=100;gentle.tick(next+0x40,11,22,33);}
    expectClose(messages.back().value,0.5f); // No cut for 85% attendance.
    wanted.arrivalFraction=0.6f;for(int i=0;i<80;i++){timeValue+=100;gentle.tick(next+0x40,11,22,33);}
    expectClose(messages.back().value,0.45625f);
    wanted.arrivalFraction=1;for(int i=0;i<80;i++){timeValue+=100;gentle.tick(next+0x40,11,22,33);}
    expectClose(messages.back().value,0.5f); // Audio can grow for authorized arrivals.
    wanted.remaining=0.7f;for(int i=0;i<80;i++){timeValue+=100;gentle.tick(next+0x40,11,22,33);}
    wanted.remaining=1;for(int i=0;i<80;i++){timeValue+=100;gentle.tick(next+0x40,11,22,33);}
    expectClose(messages.back().value,0.478125f); // Permanent loss still wins.
    Runtime failed;failed.configure(base,original,request,nullptr,faultPost,clockValue,true);
    put(next+0x48,base+0xb227188);wanted={5678,2,true,0.4f,.2f};
    failed.tick(next+0x40,11,22,33);assert(!failed.healthy()&&faultCalls==1);
    failed.tick(next+0x40,11,22,33);assert(faultCalls==1); // Never retry a failed native queue.
    Runtime legacy;legacy.configure(base,original,productionRequest,nullptr,post,clockValue,true);
    put<int>(next+0x1f4,100);wanted={9900,5,true,.125f};
    timeValue+=100;legacy.tick(next+0x40,11,22,33);expectClose(messages.back().value,.125f/.35f);
    wanted.remaining=0;for(int n=0;n<70;++n){timeValue+=100;legacy.tick(next+0x40,11,22,33);}expectClose(messages.back().value,0);
    wanted.epoch++;wanted.remaining=1;wanted.arrivalFraction=.125f;
    for(int n=0;n<70;++n){timeValue+=100;legacy.tick(next+0x40,11,22,33);}expectClose(messages.back().value,.125f/.35f);
    wanted.arrivalFraction=.35f;for(int n=0;n<70;++n){timeValue+=100;legacy.tick(next+0x40,11,22,33);}expectClose(messages.back().value,1);
    assert(VirtualFree(reinterpret_cast<void*>(next),0,MEM_RELEASE));
    std::puts("PASS: production 25% cut limit, current density, slider/mute preservation, immediate restoration; legacy attenuation, ownership, forwarding and fault regressions.");
}
