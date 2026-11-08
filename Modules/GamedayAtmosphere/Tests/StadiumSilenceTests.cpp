#define WIN32_LEAN_AND_MEAN
#include "../Source/StadiumSilenceRuntime.h"
#include <cassert>
#include <cstdio>
using attendance::chants::Address;
constexpr Address image=0x140000000;
template<class T>void put(Address p,T v){memcpy(reinterpret_cast<void*>(p),&v,sizeof(v));}
template<class T>T at(Address p){T v{};memcpy(&v,reinterpret_cast<void*>(p),sizeof(v));return v;}
Address alloc(){auto p=reinterpret_cast<Address>(VirtualAlloc(nullptr,0x1000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));assert(p);return p;}
Address manager=0;unsigned creates=0,releases=0;bool releaseFault=false;
std::map<Address,unsigned short> handles;
Address source(){return manager;}
Address __fastcall create(Address m,Address asset){
    assert(m==manager);auto groups=at<Address>(at<Address>(asset+0x20)+0x28);assert(at<unsigned>(groups-4)==1);
    auto group=at<Address>(groups),props=at<Address>(groups+8);auto id=at<unsigned short>(group+0x28);
    assert(id==9||id==12||id==13||id==14||id==15||id==16||id==17||id==18||id==23||id==74||id==75||id==77||id==32||id==11||id==86);
    assert(at<unsigned short>(group+0x2a)==(id==9||id==11?2:id==86?8:id>=74?5:id==32?3:id==23?17:9));
    assert(at<unsigned>(props-4)==1&&at<float>(props+4)==(id==17?-3.f:id==11?6.f:id==86?3.f:-60.f));assert(at<unsigned>(groups+0x18)==(id==17||id==11||id==86?0u:1u));
    Address h=alloc();put(h,image+0xaf01718);put(h+8,asset);handles.emplace(h,id);++creates;return h;
}
void __fastcall release(Address m,Address h){
    assert(m==manager&&handles.count(h));
    if(releaseFault){releaseFault=false;RaiseException(EXCEPTION_ACCESS_VIOLATION,0,0,nullptr);}
    ++releases;handles.erase(h);VirtualFree(reinterpret_cast<void*>(h),0,MEM_RELEASE);
}
struct Fixture {
    std::array<std::unique_ptr<attendance::chants::Preset>,15> stocks;
    Address m=alloc(),owner=alloc();
    Fixture(){
        constexpr unsigned short ids[]{12,13,14,15,16,18,23,74,75,77,32,17,11,86,9};const char* names[]{"Anticipation_Home_Pass","Beds","Chants/Response","Reactions","StudentSection","Swells","CrowdNIS","BandAway","BandHome","DrumCadence","PAAnnouncer","Sweeteners","WeatherAndAmbience","ReverbMaster","Crowd"};
        for(unsigned i=0;i<15;++i){
            attendance::chants::Templates t;t.groupIndex=ids[i];t.parent=ids[i]==9||ids[i]==11?2:ids[i]==86?8:ids[i]>=74?5:ids[i]==32?3:ids[i]==23?17:9;strcpy_s(t.name,names[i]);
            stocks[i]=std::make_unique<attendance::chants::Preset>(t,0.f,100+i);
            stocks[i]->asset.put(0x18,reinterpret_cast<Address>("Sound/CFB/CoreAudio/Utility/Mixers/VolumeSettings_MXR"));
            put(m+0x200+i*0x20,image+0xaf01718);put(m+0x208+i*0x20,stocks[i]->asset.address());put(m+0x500+i*0x20,m+0x200+i*0x20);
        }
        InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m+0x30));put(m+0xc0,m+0x500);put(m+0xc8,m+0x6e0);
        put(owner,image+0xb227098);put(owner+0x40,image+0xb227178);put(owner+0x48,image+0xb227188);
    }
    ~Fixture(){DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m+0x30));VirtualFree(reinterpret_cast<void*>(m),0,MEM_RELEASE);VirtualFree(reinterpret_cast<void*>(owner),0,MEM_RELEASE);}
};
int main(){
    Fixture a,b;manager=a.m;attendance::silence::Runtime mute;mute.configure(image,nullptr,create,release,source);
    mute.tick(a.owner+0x40,false);assert(!creates&&!mute.active());
    mute.tick(a.owner+0x40,true);assert(creates==14&&handles.size()==14&&mute.active());
    for(unsigned i=0;i<3;++i)assert(mute.muted(i));assert(mute.drumMuted()&&mute.paMuted()&&mute.detailDb()==-3&&mute.ambienceDb()==6&&mute.spaceDb()==3);
    for(unsigned i=0;i<100;++i)mute.tick(a.owner+0x40,true);assert(creates==14);
    mute.tick(a.owner+0x40,false);assert(creates==releases&&handles.empty()&&!mute.active());
    mute.tick(a.owner+0x40,true);assert(creates==28);
    // Manager teardown retires its handles; never release them via the new manager.
    for(auto [h,id]:handles){(void)id;VirtualFree(reinterpret_cast<void*>(h),0,MEM_RELEASE);}handles.clear();manager=b.m;
    mute.tick(b.owner+0x40,false);assert(releases==14&&!mute.active());
    mute.tick(b.owner+0x40,true);assert(creates==42&&mute.healthy());
    releaseFault=true;mute.tick(b.owner+0x40,false);assert(!mute.healthy()&&mute.active()&&handles.size()==1);
    mute.tick(b.owner+0x40,false);assert(!mute.active()&&handles.empty());
    unsigned before=creates;mute.tick(b.owner+0x40,true);assert(creates==before+14&&mute.detailDb()==-60); // Failed collective child stays silent through the root fallback.
    mute.tick(b.owner+0x40,false);assert(handles.empty());
    // A different music group with an accidentally reused ID cannot be muted.
    strcpy_s(b.stocks[8]->groupName,"PAMusic");
    attendance::silence::Runtime strict;strict.configure(image,nullptr,create,release,source);before=creates;
    strict.tick(b.owner+0x40,true);assert(creates==before+13&&!strict.muted(2));strict.tick(b.owner+0x40,false);
    strcpy_s(b.stocks[8]->groupName,"BandHome");
    attendance::silence::Runtime field;field.configure(image,nullptr,create,release,source);field.tick(b.owner+0x40,true);
    assert(field.ambienceDb()==6&&field.spaceDb()==3&&handles.size()==14);
    field.tick(b.owner+0x40,true,false);assert(field.ambienceDb()==0&&field.spaceDb()==0&&field.drumMuted()&&field.paMuted()&&handles.size()==12);
    field.tick(b.owner+0x40,false);assert(handles.empty());
    std::puts("PASS: collective crowd, both bands and drum cadence mute; quiet native detail remains; stable leases, identity checks, context reset and fault retirement preserve unrelated audio.");
}
