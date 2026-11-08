#define WIN32_LEAN_AND_MEAN
#include "../Source/HandoffCamera.h"
#include <cassert>
#include <cstdio>
using namespace attendance::camera;
constexpr Address image=0x140000000;
template<class T>void put(Address p,T value){memcpy(reinterpret_cast<void*>(p),&value,sizeof(value));}
unsigned char cameras[2][0x700]{},vtable[16]{},rows[2][16]{},settings[0x500]{};
Address active=0;unsigned gets=0,releases=0,sets=0;bool fault=false,releaseFault=false;
constexpr int off=0x1c005c,high=off+9;int namedOff=off;
Address addr(void* p){return reinterpret_cast<Address>(p);}
Address __fastcall get(Address owner,Address* out,int index){assert(owner==123&&(index==1||index==4));*out=active;if(active)++gets;return addr(out);}
int __fastcall find(Address camera,const char* name){assert(camera==addr(cameras[0])||camera==addr(cameras[1]));return !strcmp(name,"EVENT_HFA_DISABLED")?namedOff:!strcmp(name,"EVENT_HFA_TIER_1_ON")?namedOff+5:!strcmp(name,"EVENT_HFA_TIER_2_ON")?namedOff+7:namedOff+9;}
void __fastcall set(Address camera,int code,bool force){assert((camera==addr(cameras[0])||camera==addr(cameras[1]))&&!force);++sets;put(at<Address>(camera+0x420)+8,code);if(fault){fault=false;RaiseException(EXCEPTION_ACCESS_VIOLATION,0,0,nullptr);}}
void __fastcall release(Address camera){assert(camera==addr(cameras[0])||camera==addr(cameras[1]));++releases;if(releaseFault){releaseFault=false;RaiseException(EXCEPTION_ACCESS_VIOLATION,0,0,nullptr);}}
void __fastcall addRef(Address){}
int main(){
    void* ownerPage=VirtualAlloc(reinterpret_cast<void*>(image+(attendance::bindings::CameraOwner&~Address(0xffff))),0x10000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    void* settingsPage=VirtualAlloc(reinterpret_cast<void*>(image+(attendance::bindings::CameraSettings&~Address(0xffff))),0x10000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    void* cameraVt=VirtualAlloc(reinterpret_cast<void*>(image+(attendance::bindings::CameraVtable&~Address(0xffff))),0x10000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    assert(ownerPage&&settingsPage&&cameraVt);
    put(image+attendance::bindings::CameraOwner,Address(123));put(image+attendance::bindings::CameraSettings,addr(settings));
    put(addr(settings),image+attendance::bindings::CameraSettingsVtable);
    put(addr(settings)+0x3e6,static_cast<unsigned short>(1));put(addr(settings)+0x41a,static_cast<unsigned short>(1));
    put(image+attendance::bindings::CameraVtable+8,reinterpret_cast<Address>(release));
    put(image+attendance::bindings::CameraVtable,reinterpret_cast<Address>(addRef));
    put(addr(vtable),reinterpret_cast<Address>(addRef));put(addr(vtable)+8,reinterpret_cast<Address>(release));
    for(int i=0;i<2;++i){put(addr(cameras[i]),image+attendance::bindings::CameraVtable);put(addr(cameras[i])+0x420,addr(rows[i]));put(addr(cameras[i])+0x428,addr(rows[i])+16);put(addr(rows[i])+4,unsigned(0x1c));put(addr(rows[i])+8,off);}

    active=addr(cameras[0]);HandoffRuntime runtime;Api api;api.get=get;api.find=find;api.set=set;runtime.configure(image,&api);
    attendance::sustained::Intent wanted;wanted.match=123;wanted.epoch=1;wanted.active=true;wanted.shake=true;wanted.shakeTier=1;
    auto tick=[&](uint64_t now,bool prime){wanted.updated=now;wanted.cameraPrime=prime;runtime.tick(wanted,123,1,now);assert(gets==releases+(runtime.armed()?1u:0u));};
    auto native=[&](int code,uint64_t now){wanted.updated=now;int selected=runtime.selectNative(active,code,wanted,123,1,now);set(active,selected,false);return selected;};
    tick(1000,false);assert(!runtime.armed()&&sets==0); // No late acquisition from a visible lineup.
    put(addr(rows[0])+8,-1);tick(1100,true);assert(runtime.armed()&&!runtime.playing()&&sets==0); // Unset camera during play selection is expected.
    tick(1200,true);assert(sets==0); // Preparing the camera never changes its preset.
    wanted.cameraPrime=false;assert(native(off,1250)==off+5&&runtime.playing());
    tick(1300,false);assert(sets==1); // A lineup frame cannot restart or switch the camera.
    wanted.shakeTier=3;tick(1350,false);assert(sets==1); // Stronger tier also waits for a native selection.
    assert(native(off,1400)==high);wanted.shake=false;tick(1500,false);assert(!runtime.armed()&&at<int>(addr(rows[0])+8)==off&&gets==releases);
    wanted.shake=true;wanted.shakeTier=1;tick(1600,true);assert(native(high,1650)==high&&!runtime.playing());
    wanted.active=false;tick(1700,false);assert(at<int>(addr(rows[0])+8)==high); // Preserve stronger native HFA.
    wanted.active=true;put(addr(rows[0])+8,off);tick(1800,true);native(off,1850);
    put(addr(settings)+0x41a,static_cast<unsigned short>(0));tick(1900,false);assert(!runtime.armed()&&at<int>(addr(rows[0])+8)==off);
    unsigned before=sets;tick(2000,true);assert(!runtime.armed()&&sets==before); // Respect either team's accessibility setting.
    put(addr(settings)+0x41a,static_cast<unsigned short>(1));tick(2100,true);native(off,2150);
    active=addr(cameras[1]);tick(2200,false);assert(!runtime.armed()&&at<int>(addr(rows[0])+8)==off&&at<int>(addr(rows[1])+8)==off);
    assert(native(off,2250)==off); // A replacement lineup never receives a late override.
    tick(2300,true);native(off,2350);assert(native(off+20,2400)==off+20&&!runtime.playing());wanted.active=false;tick(2450,false);assert(at<int>(addr(rows[1])+8)==off+20);
    wanted.active=true;put(addr(rows[1])+8,off);tick(2500,true);native(off,2550);runtime.tick(wanted,123,1,3400);assert(!runtime.armed()&&gets==releases&&at<int>(addr(rows[1])+8)==off); // Stale lease restores.
    tick(3500,true);native(off,3550);active=0;tick(3600,false);assert(!runtime.armed()&&at<int>(addr(rows[1])+8)==off&&gets==releases); // Lost active view restores retained old reference.
    active=addr(cameras[1]);tick(3700,true);native(off,3750);
    put(addr(cameras[0]),addr(vtable));active=addr(cameras[0]);tick(3760,false);
    assert(runtime.healthy()&&!runtime.armed()&&runtime.skippedViews()==1&&gets==releases);
    // A broadcast/replay camera shares native reference functions, but no HFA
    // methods may be called on it. A later play selection can arm normally.
    active=addr(cameras[1]);tick(3770,true);native(off,3780);assert(runtime.playing());
    fault=true;wanted.active=false;tick(3800,false);assert(!runtime.healthy()&&!runtime.armed()&&gets==releases&&!strcmp(runtime.failure(),"camera event restoration"));
    HandoffRuntime rebound;rebound.configure(image,&api);wanted.active=true;wanted.shake=true;wanted.cameraPrime=true;wanted.updated=4000;put(addr(rows[1])+8,-1);
    rebound.tick(wanted,123,1,4000);int chosen=rebound.selectNative(active,off,wanted,123,1,4001);set(active,chosen,false);assert(rebound.playing());
    before=sets;namedOff+=0x10000;wanted.active=false;wanted.updated=4100;rebound.tick(wanted,123,1,4100);
    assert(!rebound.armed()&&gets==releases&&sets==before); // A rebuilt channel namespace cannot receive an old event during restoration.
    HandoffRuntime lifecycle;lifecycle.configure(image,&api);namedOff=off;
    put(addr(cameras[0]),image+attendance::bindings::CameraVtable);put(addr(rows[0])+8,off);active=addr(cameras[0]);
    wanted.match=123;wanted.epoch=1;wanted.active=true;wanted.shake=true;wanted.cameraPrime=true;wanted.updated=5000;
    lifecycle.tick(wanted,123,1,5000);assert(lifecycle.armed());
    // Force-retire the old level allocation, then deliver an inactive new-
    // world lease. Cleanup must never call the old view, even before arming.
    put(addr(cameras[0]),Address(0));before=sets;unsigned oldReleases=releases;
    wanted.match=456;wanted.epoch=2;wanted.active=false;wanted.updated=5100;
    lifecycle.tick(wanted,456,2,5100);
    assert(lifecycle.healthy()&&!lifecycle.armed()&&lifecycle.retiredViews()==1&&sets==before&&releases==oldReleases);
    active=addr(cameras[1]);wanted.active=true;wanted.updated=5200;lifecycle.tick(wanted,456,2,5200);assert(lifecycle.armed());
    int selected=lifecycle.selectNative(active,off,wanted,456,2,5201);assert(selected==off+5);set(active,selected,false);
    // A reused match address is still a different level when its epoch changes.
    put(addr(cameras[1]),Address(0));before=sets;oldReleases=releases;wanted.epoch=3;wanted.active=false;wanted.updated=5300;
    lifecycle.tick(wanted,456,3,5300);assert(lifecycle.healthy()&&lifecycle.retiredViews()==2&&sets==before&&releases==oldReleases);
    put(addr(cameras[0]),image+attendance::bindings::CameraVtable);put(addr(rows[0])+8,off);active=addr(cameras[0]);wanted.active=true;wanted.updated=5400;
    lifecycle.tick(wanted,456,3,5400);assert(lifecycle.armed());
    // A scene camera can also be force-retired without changing the match.
    put(addr(cameras[0]),Address(0));wanted.active=false;wanted.updated=5500;before=sets;oldReleases=releases;
    lifecycle.tick(wanted,456,3,5500);assert(lifecycle.healthy()&&!lifecycle.armed()&&lifecycle.retiredViews()==3&&sets==before&&releases==oldReleases);
    put(addr(cameras[0]),image+attendance::bindings::CameraVtable);wanted.active=true;wanted.updated=5600;lifecycle.tick(wanted,456,3,5600);assert(lifecycle.armed());
    // World teardown may begin before match/epoch changes and while the old
    // vtable still looks valid. No native cleanup is safe in that window.
    before=sets;oldReleases=releases;wanted.active=false;wanted.updated=5700;
    lifecycle.tick(wanted,456,3,5700,false);assert(lifecycle.healthy()&&!lifecycle.armed()&&lifecycle.retiredViews()==4&&sets==before&&releases==oldReleases);
    assert(gets==releases+lifecycle.retiredViews());
    HandoffRuntime nativeFailure;nativeFailure.configure(image,&api);wanted.active=true;wanted.updated=5800;nativeFailure.tick(wanted,456,3,5800);assert(nativeFailure.armed());
    wanted.active=false;wanted.updated=5900;releaseFault=true;nativeFailure.tick(wanted,456,3,5900);
    assert(!nativeFailure.healthy()&&!strcmp(nativeFailure.failure(),"native camera release call")); // Real native failures still stop the optional feature.
    assert(VirtualFree(ownerPage,0,MEM_RELEASE)&&VirtualFree(settingsPage,0,MEM_RELEASE)&&VirtualFree(cameraVt,0,MEM_RELEASE));
    std::puts("PASS: offscreen camera arming, unset-channel handoff, no frame-driven preset switches, native stronger-tier preservation, no late/replacement override, accessibility, unknown events, stale leases, reference cleanup, force-retired levels, reused-match epochs, intra-match scene retirement and restoration faults.");
}
