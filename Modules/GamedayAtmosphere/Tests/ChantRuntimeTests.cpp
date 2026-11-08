#define WIN32_LEAN_AND_MEAN
#include "../Source/ChantMixerRuntime.h"
#include "../Source/ChantAudioModel.h"
#include <cassert>
#include <cstdio>
using namespace attendance::chants;
constexpr Address image=0x140000000;
Address currentManager=0,currentHandle=0;unsigned created=0,released=0;bool failCreate=false;
unsigned long long gainTime=1000;unsigned long long clockSource(){return gainTime;}
template<class T>void put(Address p,T v){std::memcpy(reinterpret_cast<void*>(p),&v,sizeof(v));}
template<class T>T get(Address p){T v{};std::memcpy(&v,reinterpret_cast<void*>(p),sizeof(v));return v;}
Address alloc(){auto p=reinterpret_cast<Address>(VirtualAlloc(nullptr,0x1000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));assert(p);return p;}
Address source(){return currentManager;}
Address __fastcall create(Address manager,Address asset){
    assert(manager==currentManager);++created;if(failCreate)return 0;
    currentHandle=alloc();put(currentHandle,image+0xaf01718);put(currentHandle+8,asset);
    Address preset=get<Address>(asset+0x20),groupEntries=get<Address>(preset+0x28);
    assert(get<unsigned>(groupEntries-4)==1);
    Address props=get<Address>(groupEntries+8);assert(get<unsigned>(props-4)==1&&get<unsigned>(props)==static_cast<unsigned>(MixProperty::Gain));
    return currentHandle;
}
void __fastcall release(Address manager,Address handle){
    assert(manager==currentManager&&handle==currentHandle);++released;
    assert(VirtualFree(reinterpret_cast<void*>(handle),0,MEM_RELEASE));currentHandle=0;
}
struct Fixture {
    Templates blank;Preset stock{blank,0,999};Address manager=alloc(),owner=alloc();
    Fixture(){
        InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(manager+0x30));
        const char* name="Sound/CFB/CoreAudio/Utility/Mixers/VolumeSettings_MXR";
        stock.asset.put(0x18,reinterpret_cast<Address>(name));stock.properties.count=2;
        put(stock.properties.address(),unsigned(0));put(stock.properties.address()+12,unsigned(3));
        put(manager+0x200,image+0xaf01718);put(manager+0x208,stock.asset.address());
        put(manager+0x300,manager+0x200);put(manager+0xc0,manager+0x300);put(manager+0xc8,manager+0x320);
        put(owner,image+0xb227098);put(owner+0x40,image+0xb227178);put(owner+0x48,image+0xb227188);
    }
    ~Fixture(){DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(manager+0x30));VirtualFree(reinterpret_cast<void*>(manager),0,MEM_RELEASE);VirtualFree(reinterpret_cast<void*>(owner),0,MEM_RELEASE);}
};
int main(){
    Fixture first,second;currentManager=first.manager;
    Runtime runtime;runtime.configure(image,nullptr,create,release,source);
    runtime.tick(first.owner+0x40,false,0);runtime.tick(first.owner+0x40,true,0);assert(created==0);
    runtime.tick(first.owner+0x40,true,-6);assert(runtime.healthy()&&created==1&&released==0&&runtime.db()==-6);
    for(int i=0;i<500;i++)runtime.tick(first.owner+0x40,true,-6);assert(created==1&&released==0);
    runtime.tick(first.owner+0x40,true,-9);assert(created==2&&released==1&&runtime.db()==-9);
    runtime.tick(first.owner+0x40,false,-9);assert(released==2&&runtime.db()==0);
    unsigned oldCreated=created,oldReleased=released;
    runtime.tick(first.owner+0x40,true,0,false,true);assert(created==oldCreated+1);
    runtime.tick(first.owner+0x40,true,0,false,true);assert(created==oldCreated+1);
    runtime.tick(first.owner+0x40,false,0);assert(released==oldReleased+1);
    // Keep the following ownership checks' counters independent of this mute pass.
    --created;--released;
    runtime.tick(first.owner+0x40,true,0,true);assert(created==3);runtime.tick(first.owner+0x40,false,0);assert(released==3);
    put(first.owner+0x48,Address(0));runtime.tick(first.owner+0x40,true,-6);assert(created==3);put(first.owner+0x48,image+0xb227188);
    runtime.tick(first.owner+0x40,true,-6);assert(created==4);
    // Simulate destruction of an entire manager and its owned mixer. The next
    // callback must not inspect or release the retired handle through a new manager.
    assert(VirtualFree(reinterpret_cast<void*>(currentHandle),0,MEM_RELEASE));currentHandle=0;currentManager=second.manager;
    runtime.tick(second.owner+0x40,false,0);assert(released==3);
    runtime.tick(second.owner+0x40,true,-6);assert(created==5);runtime.tick(second.owner+0x40,false,0);assert(released==4);
    runtime.tick(second.owner+0x40,true,-120);assert(runtime.db()==-120);
    Address deepProps=get<Address>(get<Address>(get<Address>(currentHandle+8)+0x20)+0x28);
    assert(get<float>(get<Address>(deepProps+8)+4)==-120);
    runtime.tick(second.owner+0x40,false,0);
    // Production wiring: quarter attendance targets select the calibrated
    // preset. Repeated/replay callbacks reuse it, and disabling retires it.
    auto mixerLevel=[](float remaining){return attendance::volume::nativeDb(1-attendancePercent(remaining)*.01f);};
    const float remainingValues[]={1.0f,0.99f,0.94f,0.5f,0.22f},expectedLevels[]={0,0,0,-2,-6};
    for(int i=0;i<5;++i){
        float target=mixerLevel(remainingValues[i]);
        runtime.tick(second.owner+0x40,true,target);
        assert(runtime.db()==expectedLevels[i]);
    }
    unsigned stableCreated=created,stableReleased=released;
    for(int replay=0;replay<500;++replay)runtime.tick(second.owner+0x40,true,mixerLevel(0.22f));
    assert(created==stableCreated&&released==stableReleased&&runtime.db()==-6);
    runtime.tick(second.owner+0x40,false,mixerLevel(0.22f));assert(released==stableReleased+1&&runtime.db()==0);
    runtime.tick(second.owner+0x40,true,mixerLevel(1));assert(created==stableCreated);
    // Positive gain is an explicit opt-in for a specific crowd group. Chants
    // and unrelated groups keep their attenuation-only ceiling.
    Runtime boost;boost.configure(image,nullptr,create,release,source,12,clockSource);
    put(second.stock.group.address()+0x28,static_cast<unsigned short>(9));
    put(second.stock.group.address()+0x2a,static_cast<unsigned short>(2));strcpy_s(second.stock.groupName,"Crowd");
    boost.tick(second.owner+0x40,true,3,false,false,9);assert(boost.db()==3);
    Address boostedGroups=get<Address>(get<Address>(get<Address>(currentHandle+8)+0x20)+0x28);
    assert(get<float>(get<Address>(boostedGroups+8)+4)==3);
    gainTime+=50;boost.tick(second.owner+0x40,true,8,false,false,9);assert(boost.db()==3); // No overlapping rapid fades.
    gainTime+=300;boost.tick(second.owner+0x40,true,8,false,false,9);assert(boost.db()==8);
    gainTime+=300;boost.tick(second.owner+0x40,true,99,false,false,9);assert(boost.db()==12);
    gainTime+=300;boost.tick(second.owner+0x40,true,3.02f,false,false,9);assert(boost.db()==3);
    boost.tick(second.owner+0x40,false,3,false,false,9);assert(boost.db()==0);
    boost.configure(image,nullptr,create,release,source,16,clockSource);
    gainTime+=300;boost.tick(second.owner+0x40,true,15,false,false,9);assert(boost.db()==15);
    gainTime+=300;boost.tick(second.owner+0x40,true,99,false,false,9);assert(boost.db()==16);
    gainTime+=300;boost.tick(second.owner+0x40,true,8,false,false,9);assert(boost.db()==8);
    gainTime+=10;boost.tick(second.owner+0x40,true,6,false,false,9,true);assert(boost.db()==6); // A decisive tail cannot retain an expired higher target through cooldown.
    boost.tick(second.owner+0x40,false,16,false,false,9);assert(boost.db()==0);
    boost.tick(second.owner+0x40,true,3,false,false,14);assert(boost.db()==0);
    boost.tick(second.owner+0x40,true,3,false,false,31);assert(boost.db()==0); // Crowd runtime cannot boost commentary.
    put(second.stock.group.address()+0x28,static_cast<unsigned short>(13));
    put(second.stock.group.address()+0x2a,static_cast<unsigned short>(9));
    std::memcpy(second.stock.groupName,"Beds",5);
    Runtime bed;bed.configure(image,nullptr,create,release,source,3,clockSource,13);
    gainTime+=300;bed.tick(second.owner+0x40,true,1,false,false,13);assert(bed.db()==1);
    gainTime+=10;bed.tick(second.owner+0x40,true,0.25f,false,false,13);assert(bed.db()==0.25f); // Headroom reductions cannot wait on the positive cooldown.
    bed.tick(second.owner+0x40,false,0,false,false,13);assert(bed.db()==0);
    bed.tick(second.owner+0x40,true,3,false,false,9);assert(bed.db()==0); // Cannot boost a second group.
    bed.tick(second.owner+0x40,true,3,false,false,31);assert(bed.db()==0);
    for(auto spec:{std::pair<unsigned short,const char*>{static_cast<unsigned short>(15),"Reactions"},{static_cast<unsigned short>(18),"Swells"}}){
        put(second.stock.group.address()+0x28,spec.first);std::memset(second.stock.groupName,0,sizeof(second.stock.groupName));std::memcpy(second.stock.groupName,spec.second,std::strlen(spec.second)+1);
        Runtime layer;layer.configure(image,nullptr,create,release,source,4,clockSource,spec.first);
        gainTime+=300;layer.tick(second.owner+0x40,true,3.5f,false,false,spec.first);assert(layer.db()==3.5f);
        gainTime+=10;layer.tick(second.owner+0x40,true,.5f,false,false,spec.first);assert(layer.db()==.5f); // Shared headroom shrinks without waiting on a fade cooldown.
        layer.tick(second.owner+0x40,false,0,false,false,spec.first);assert(layer.db()==0);
        layer.tick(second.owner+0x40,true,3,false,false,31);assert(layer.db()==0);
    }
    put(second.stock.group.address()+0x28,static_cast<unsigned short>(14));
    strcpy_s(second.stock.groupName,"Chants/Response"); // Restore the identity after testing other groups.
    Runtime failed;failed.configure(image,nullptr,create,release,source);failCreate=true;
    failed.tick(second.owner+0x40,true,-6);assert(!failed.healthy());unsigned attempts=created;
    failed.tick(second.owner+0x40,true,-6);assert(created==attempts);
    std::puts("Chant runtime activation, native ownership, stable updates, retirement, controller replacement and failure checks passed.");
}
