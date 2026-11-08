#include "../Source/CrowdDispatch.h"
#include "../Source/CrowdVisibility.h"
#include "../Source/CrowdPartResolver.h"
#include <cassert>
#include <iostream>
#include <algorithm>
using namespace attendance;
struct Crowd {
    bool ready=false;std::vector<bool> native,current,original,owned;
    Crowd():native(10000),current(10000),original(10000),owned(10000){
        for(unsigned i=0;i<10000;i++)native[i]=current[i]=i>=8000;
    }
};
// Intentionally contains no renderer or mask layout. This is the real boundary
// that failed: a small scene controller resolves a separate crowd object.
struct Controller {Crowd* crowd;bool live=true;};
unsigned partLookups=0;
unsigned __fastcall validPart(uintptr_t,unsigned part){++partLookups;return part;}
unsigned __fastcall retiredPart(uintptr_t,unsigned){RaiseException(EXCEPTION_ACCESS_VIOLATION,0,0,nullptr);return 0;}
int main(){
    unsigned bit=0;
    assert(!visibility::resolvePart(validPart,0,3,bit)&&partLookups==0);
    assert(!visibility::resolvePart(nullptr,0x10000,3,bit));
    assert(visibility::resolvePart(validPart,0x10000,15,bit)&&bit==15);
    assert(!visibility::resolvePart(validPart,0x10000,16,bit));
    assert(!visibility::resolvePart(retiredPart,0x10000,3,bit));
    assert(visibility::resolvePart(validPart,0x10000,3,bit)&&bit==3); // A later valid scene still works.
    visibility::PassParts cache;partLookups=0;cache.begin();
    for(unsigned fan=0;fan<34393;++fan)for(unsigned part=0;part<3;++part)assert(cache.resolve(validPart,0x10000,part,bit)&&bit==part);
    assert(partLookups==3); //103179 lookups become3; identical bit result.
    assert(!cache.resolve(retiredPart,0x10000,1,bit));
    assert(cache.resolve(validPart,0x20000,1,bit)&&partLookups==4);
    cache.end();assert(cache.resolve(validPart,0x10000,1,bit)&&partLookups==5);
    cache.begin();assert(cache.resolve(validPart,0x10000,1,bit)&&partLookups==6);
    for(unsigned part=3;part<80;++part)assert(!cache.resolve(validPart,0x10000,part+16,bit));
    cache.end();
    Crowd crowd;Controller controller{&crowd},dead{&crowd,false};
    std::vector<uintptr_t> controllers{reinterpret_cast<uintptr_t>(&controller),reinterpret_cast<uintptr_t>(&dead)};
    int permanent=0,temporary=40;unsigned capturePasses=0,applyPasses=0;
    auto valid=[&](uintptr_t p){return reinterpret_cast<Controller*>(p)->live;};
    auto update=[&](uintptr_t p,bool capture){
        auto& target=*reinterpret_cast<Controller*>(p)->crowd;
        visibility::Pass r;
        if(capture)++capturePasses;else ++applyPasses;
        for(unsigned i=0;i<target.current.size();++i){
            ++r.calls;if(!target.ready)continue;++r.verified;
            bool native=target.native[i];
            if(capture){target.original[i]=target.current[i]=native;target.owned[i]=false;continue;}
            bool left=departed(i,permanent),held=departed(i,temporary),wasOwned=target.owned[i];
            bool hidden=hidePart(native,target.original[i],target.current[i],left||held,false,true,wasOwned);
            if(hidden&&!native)++r.changed;
            if(held&&!left&&!native&&!target.original[i])target.owned[i]=true;else if(!held)target.owned[i]=false;
            target.current[i]=hidden;
        }
        return r;
    };
    auto waiting=visibility::dispatch(controllers,false,true,valid,update);
    assert(waiting.controllers==1&&waiting.calls==10000&&waiting.verified==0&&waiting.changed==0);
    crowd.ready=true;
    auto result=visibility::dispatch(controllers,false,true,valid,update);
    assert(result.controllers==1&&result.verified==10000&&result.changed>2500);
    unsigned visible=0,expected=0;
    for(unsigned i=0;i<crowd.current.size();++i){
        if(!crowd.current[i])++visible;
        if(!crowd.native[i]&&!departed(i,40))++expected;
        if(crowd.native[i])assert(crowd.current[i]);
    }
    assert(visible==expected&&visible>4500&&visible<5100); // About 60% of original 8,000 fans.
    unsigned originalCaptureCount=capturePasses;
    temporary=20;visibility::dispatch(controllers,false,false,valid,update);
    unsigned filled=static_cast<unsigned>(std::count(crowd.current.begin(),crowd.current.end(),false));assert(filled>visible);
    permanent=50;temporary=60;visibility::dispatch(controllers,false,false,valid,update);
    unsigned beforeReturn=static_cast<unsigned>(std::count(crowd.current.begin(),crowd.current.end(),false));
    temporary=0;visibility::dispatch(controllers,false,false,valid,update);
    unsigned returned=static_cast<unsigned>(std::count(crowd.current.begin(),crowd.current.end(),false));
    assert(returned>beforeReturn&&returned<filled&&capturePasses==originalCaptureCount);
    for(unsigned i=0;i<crowd.current.size();++i)if(crowd.native[i]||departed(i,permanent))assert(crowd.current[i]);
    unsigned previousApplies=applyPasses;
    auto capture=visibility::dispatch(controllers,true,false,valid,update);
    assert(capture.verified==10000&&applyPasses==previousApplies);
    // Simulate additional native hiding after the original population was
    // captured. Diagnostics must separate it from the plugin's fan selection.
    VisibilityCount tally;
    for(unsigned i=0;i<8000;++i){
        bool selected=departed(i,12),native=i<600,other=i>=600&&i<650;
        unsigned short current=static_cast<unsigned short>(selected||native||other);
        countFan(tally,i,1,0,current,static_cast<unsigned short>(native),12);
    }
    assert(tally.original==8000&&tally.expected>6900&&tally.expected<7150);
    assert(tally.nativeExtra>450&&tally.otherExtra>30&&tally.extraVisible==0);
    assert(tally.visible+tally.nativeExtra+tally.otherExtra==tally.expected);
    VisibilityCount empties;countFan(empties,0,1,1,1,1,0);assert(empties.original==0); // Native starting empties were never in the denominator.
    countFan(empties,0,1,0,0,0,100);assert(empties.extraVisible==1); // Flag a missing override too.
    controller.live=false;
    auto retired=visibility::dispatch(controllers,false,true,valid,update);
    assert(retired.controllers==0&&retired.verified==0);
    std::cout<<"PASS: retiring part-table guards, controller/renderer dispatch, retries, measured 60% geometry, native empty seats, arrival returns and permanent departures.\n";
}
