#define WIN32_LEAN_AND_MEAN
#include "../Source/PresentationCamera.h"
#include <cassert>
#include <cstdio>
using namespace attendance::camera;
constexpr Address image=0x140000000;
template<class T>void put(Address p,T value){memcpy(reinterpret_cast<void*>(p),&value,sizeof(value));}
unsigned char cameras[2][0x700]{},vtable[16]{},rows[2][16]{},settings[0x500]{};
Address active=0;unsigned gets=0,releases=0,sets=0;bool fault=false;
constexpr int off=0x1c005c,high=off+9;
Address addr(void* p){return reinterpret_cast<Address>(p);}
Address __fastcall get(Address owner,Address* out,int index){assert(owner==123&&index==4);*out=active;if(active)++gets;return addr(out);}
int __fastcall find(Address camera,const char* name){assert(camera==active);return !strcmp(name,"EVENT_HFA_DISABLED")?off:!strcmp(name,"EVENT_HFA_TIER_1_ON")?off+5:!strcmp(name,"EVENT_HFA_TIER_2_ON")?off+7:high;}
void __fastcall set(Address camera,int code,bool force){assert((camera==addr(cameras[0])||camera==addr(cameras[1]))&&!force);++sets;put(at<Address>(camera+0x420)+8,code);if(fault){fault=false;RaiseException(EXCEPTION_ACCESS_VIOLATION,0,0,nullptr);}}
void __fastcall release(Address camera){assert(camera==addr(cameras[0])||camera==addr(cameras[1]));++releases;}
int main(){
    void* ownerPage=VirtualAlloc(reinterpret_cast<void*>(image+(attendance::bindings::CameraOwner&~Address(0xffff))),0x10000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    void* settingsPage=VirtualAlloc(reinterpret_cast<void*>(image+(attendance::bindings::CameraSettings&~Address(0xffff))),0x10000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    void* cameraVt=VirtualAlloc(reinterpret_cast<void*>(image+(attendance::bindings::CameraVtable&~Address(0xffff))),0x10000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    assert(ownerPage&&settingsPage&&cameraVt);
    put(image+attendance::bindings::CameraOwner,Address(123));put(image+attendance::bindings::CameraSettings,addr(settings));
    put(addr(settings),image+attendance::bindings::CameraSettingsVtable);
    put(addr(settings)+0x3e6,static_cast<unsigned short>(1));put(addr(settings)+0x41a,static_cast<unsigned short>(1));
    put(image+attendance::bindings::CameraVtable+8,reinterpret_cast<Address>(release));
    for(int i=0;i<2;++i){put(addr(cameras[i]),image+attendance::bindings::CameraVtable);put(addr(cameras[i])+0x420,addr(rows[i]));put(addr(cameras[i])+0x428,addr(rows[i])+16);put(addr(rows[i])+4,unsigned(0x1c));put(addr(rows[i])+8,off);}
    active=addr(cameras[0]);Runtime runtime;Api api;api.get=get;api.find=find;api.set=set;runtime.configure(image,&api,0);
    attendance::sustained::Intent wanted;wanted.match=123;wanted.epoch=1;wanted.active=true;wanted.shake=true;wanted.lineup=1;
    auto tick=[&](uint64_t now){wanted.updated=now;runtime.tick(wanted,123,1,now);assert(gets==releases+(runtime.playing()?1u:0u));};
    tick(1000);assert(runtime.playing()&&sets==1&&at<int>(addr(rows[0])+8)==high);
    tick(1100);assert(sets==1); // Does not restart the camera every frame.
    runtime.observeNative(active,off+5,1150);put(addr(rows[0])+8,off+5);tick(1200);assert(sets==2);
    wanted.shake=false;tick(1300);assert(!runtime.playing()&&at<int>(addr(rows[0])+8)==off+5&&runtime.restores()==1);
    wanted.shake=true;tick(1400);runtime.observeNative(active,high,1450);wanted.active=false;tick(1500);assert(at<int>(addr(rows[0])+8)==high); // Preserve a native tier-3 takeover too.
    put(addr(rows[0])+8,off);wanted.active=true;tick(1600);
    put(addr(settings)+0x41a,static_cast<unsigned short>(0));tick(1700);assert(!runtime.playing()&&at<int>(addr(rows[0])+8)==off);
    unsigned before=sets;tick(1800);assert(sets==before); // Either team's accessibility setting disables added shake.
    put(addr(settings)+0x41a,static_cast<unsigned short>(1));tick(1900);
    active=addr(cameras[1]);wanted.active=false;tick(2000);assert(!runtime.playing()&&at<int>(addr(rows[1])+8)==off&&at<int>(addr(rows[0])+8)==off); // Restore/release the owned old view without writing into its replacement.
    wanted.active=true;tick(2100);put(addr(rows[1])+8,0x1c0099);wanted.active=false;tick(2200);assert(at<int>(addr(rows[1])+8)==0x1c0099); // Unknown native channel event wins.
    put(addr(rows[1])+8,off);wanted.active=true;tick(2300);runtime.tick(wanted,123,1,3100);assert(!runtime.playing()&&gets==releases); // Stale lease restores.
    tick(3150);active=0;wanted.shake=false;tick(3175);assert(!runtime.playing()&&at<int>(addr(rows[1])+8)==off); // Cleanup also works when the active view disappears.
    active=addr(cameras[1]);wanted.shake=true;
    wanted.active=true;fault=true;tick(3200);assert(!runtime.healthy());tick(3300);assert(!runtime.playing()&&at<int>(addr(rows[1])+8)==off);
    Runtime softer;softer.configure(image,&api,0);wanted.active=true;wanted.shake=true;wanted.shakeTier=1;wanted.updated=4000;softer.tick(wanted,123,1,4000);assert(at<int>(addr(rows[1])+8)==off+5);
    wanted.shakeTier=3;wanted.updated=4100;softer.tick(wanted,123,1,4100);assert(at<int>(addr(rows[1])+8)==high);
    wanted.shakeTier=1;wanted.updated=4200;softer.tick(wanted,123,1,4200);assert(at<int>(addr(rows[1])+8)==off+5);
    wanted.active=false;wanted.updated=4300;softer.tick(wanted,123,1,4300);assert(at<int>(addr(rows[1])+8)==off&&gets==releases);
    put(addr(rows[1])+8,high);wanted.active=true;wanted.updated=4400;softer.tick(wanted,123,1,4400);assert(at<int>(addr(rows[1])+8)==high); // A weaker plugin baseline does not suppress native shake.
    wanted.active=false;wanted.updated=4500;softer.tick(wanted,123,1,4500);assert(at<int>(addr(rows[1])+8)==high&&gets==releases);
    Runtime settling;settling.configure(image,&api);active=addr(cameras[0]);put(addr(rows[0])+8,off);put(addr(rows[1])+8,off);wanted.active=true;wanted.shake=true;
    auto settledTick=[&](uint64_t now){wanted.updated=now;settling.tick(wanted,123,1,now);assert(gets==releases+(settling.playing()?1u:0u));};
    before=sets;settledTick(5000);assert(sets==before&&!settling.playing());
    put(addr(rows[0])+8,off+5);settledTick(5300);settledTick(5500);assert(sets==before); // A changing native event restarts the wait.
    active=addr(cameras[1]);settledTick(5600);settledTick(6099);assert(sets==before); // A replacement view must settle too.
    settledTick(6100);assert(sets==before+1&&settling.playing()&&at<int>(addr(rows[1])+8)==off+5);
    wanted.active=false;settledTick(6200);assert(!settling.playing()&&at<int>(addr(rows[1])+8)==off&&gets==releases);
    assert(VirtualFree(ownerPage,0,MEM_RELEASE)&&VirtualFree(settingsPage,0,MEM_RELEASE)&&VirtualFree(cameraVt,0,MEM_RELEASE));
    std::puts("PASS: native camera-only events, reference cleanup, accessibility, native takeover including tier 3, snap/replay/lease restoration, view/event settling, view replacement and setter faults.");
}
