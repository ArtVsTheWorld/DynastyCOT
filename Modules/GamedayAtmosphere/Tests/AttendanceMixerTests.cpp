#define WIN32_LEAN_AND_MEAN
#include "../Source/AttendanceMixer.h"
#include <cassert>
#include <cstdio>
using attendance::chants::Address;
constexpr Address image=0x140000000;
template<class T>void put(Address p,T v){memcpy(reinterpret_cast<void*>(p),&v,sizeof(v));}
template<class T>T at(Address p){return attendance::sustained::at<T>(p);}
Address alloc(){auto p=reinterpret_cast<Address>(VirtualAlloc(nullptr,0x1000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));assert(p);return p;}
Address manager=0,handle=0;unsigned creates=0,releases=0;uint64_t now=1000;bool releaseFault=false,expectMute=false;
Address source(){return manager;}uint64_t clockValue(){return now;}
Address __fastcall create(Address m,Address asset){
 assert(m==manager);++creates;auto groups=at<Address>(at<Address>(asset+0x20)+0x28);assert(at<unsigned>(groups-4)==8);
 for(unsigned i=0;i<8;++i){auto entry=groups+i*0x28,group=at<Address>(entry),props=at<Address>(entry+8);
  assert(at<unsigned short>(group+0x28)==attendance::balance::Groups[i]);assert(at<unsigned short>(group+0x2a)==(i?9:2));
  assert(at<unsigned>(props-4)==1&&at<unsigned>(props)==0&&at<unsigned char>(entry+0x20)==1);
  assert(at<unsigned>(entry+0x18)==(expectMute&&i==0?1u:0u));
 }
 handle=alloc();put(handle,image+0xaf01718);put(handle+8,asset);return handle;
}
void __fastcall release(Address m,Address h){assert(m==manager&&h==handle);if(releaseFault){releaseFault=false;RaiseException(EXCEPTION_ACCESS_VIOLATION,0,0,nullptr);}++releases;VirtualFree(reinterpret_cast<void*>(h),0,MEM_RELEASE);handle=0;}
struct Fixture {
 std::array<attendance::chants::Templates,8> t;std::unique_ptr<attendance::balance::Preset> stock;Address m=alloc(),owner=alloc();
 Fixture(){for(unsigned i=0;i<8;++i){t[i].groupIndex=attendance::balance::Groups[i];t[i].parent=i?9:2;strcpy_s(t[i].name,attendance::balance::Names[i]);}
  stock=std::make_unique<attendance::balance::Preset>(t,attendance::balance::Mix{},123ull);stock->entries[0]->asset.put(0x18,reinterpret_cast<Address>("Sound/CFB/CoreAudio/Utility/Mixers/VolumeSettings_MXR"));
  InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m+0x30));put(m+0x200,image+0xaf01718);put(m+0x208,stock->address());put(m+0x300,m+0x200);put(m+0xc0,m+0x300);put(m+0xc8,m+0x320);
  put(owner,image+0xb227098);put(owner+0x40,image+0xb227178);put(owner+0x48,image+0xb227188);
 }
 ~Fixture(){DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m+0x30));VirtualFree(reinterpret_cast<void*>(m),0,MEM_RELEASE);VirtualFree(reinterpret_cast<void*>(owner),0,MEM_RELEASE);}
};
int main(){
 Fixture a,b;manager=a.m;attendance::balance::Runtime mix;mix.configure(image,create,release,source,clockValue);
 mix.tick(a.owner+0x40,true,attendance::balance::mix(.7f,6060));assert(!creates);
 auto sparse=attendance::balance::mix(.25f,6060);mix.tick(a.owner+0x40,true,sparse);assert(creates==1&&std::fabs(mix.netDb(1)-20*std::log10(.5f))<.0001f&&mix.chantDb()==mix.netDb(1));
 for(unsigned i=0;i<100;++i)mix.tick(a.owner+0x40,true,sparse);assert(creates==1);
 expectMute=true;mix.tick(a.owner+0x40,true,attendance::balance::mix(0,6060));assert(mix.muted()); // Mute bypasses transition throttle.
 expectMute=false;mix.tick(a.owner+0x40,false,sparse);assert(!handle&&creates==releases&&!mix.muted());
 now+=400;mix.tick(a.owner+0x40,true,sparse);VirtualFree(reinterpret_cast<void*>(handle),0,MEM_RELEASE);handle=0;manager=b.m;unsigned old=releases;
 mix.tick(b.owner+0x40,false,sparse);assert(releases==old); // Old manager owns its former handle retirement.
 now+=400;mix.tick(b.owner+0x40,true,attendance::balance::mix(1,61888));assert(mix.healthy()&&std::fabs(mix.netDb(1)-4.f)<.001f&&std::fabs(mix.netDb(3)-attendance::volume::boostDb(30))<.001f);
 releaseFault=true;mix.tick(b.owner+0x40,false,sparse);assert(!mix.healthy()&&handle);mix.tick(b.owner+0x40,false,sparse);assert(!handle); // Retry retirement after fault.
 assert(creates==releases+1);
 // An exception latches failure without disabling the native cleanup callback.
 {attendance::balance::Runtime failed;failed.configure(image,create,release,source,clockValue);now+=400;
  failed.tick(b.owner+0x40,true,sparse);assert(handle);auto made=creates;
  failed.fail();failed.tick(b.owner+0x40,true,sparse);assert(!handle&&!failed.healthy());
  now+=400;failed.tick(b.owner+0x40,true,sparse);assert(creates==made);}

 std::puts("PASS: one eight-group preset, synchronized gains, explicit zero mute, stable updates, manager replacement and retryable fault retirement.");
}
