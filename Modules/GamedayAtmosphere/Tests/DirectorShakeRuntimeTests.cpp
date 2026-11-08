#include "../Source/DirectorShakeRuntime.h"
#include "../Source/NativePlayCall.h"
#include "../Source/LineupAtmosphere.h"
#include <vector>
#include <cassert>
#include <cstring>
#include <iostream>
using namespace attendance;using namespace attendance::motion;
using A=uintptr_t;
bool closeEnough(float a,float b){return std::fabs(a-b)<.001f;}
template<class T>void put(A address,T value){std::memcpy(reinterpret_cast<void*>(address),&value,sizeof(value));}
template<class T>A addr(T& object){return reinterpret_cast<A>(&object);}
bool memoryRead(A p,void* out,size_t n){SIZE_T read=0;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(p),out,n,&read)&&read==n;}
int main(){
    constexpr A image=0x10000000;std::vector<void*> pages;
    auto global=[&](A rva,A value,bool integer=false){A p=(image+rva)&~A(0xffff);bool exists=false;for(auto v:pages)if(addr(*static_cast<char*>(v))==p)exists=true;if(!exists){auto v=VirtualAlloc(reinterpret_cast<void*>(p),65536,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);assert(reinterpret_cast<A>(v)==p);pages.push_back(v);}if(integer)put(image+rva,static_cast<int>(value));else put(image+rva,value);};
    std::vector<A> wrapper(4),match(0x200/8),replay(0x80/8),settings(0x500/8),owner(2),data(0x300/8),views(4*0x68/8),view(0x800/8),foreign(0x800/8);
    std::vector<A> gameStates(0x200/8);put(addr(gameStates[0])+0xa0,1);global(bindings::GameStates,addr(gameStates[0]));
    unsigned char paused=0;match[0]=image+0xb47e908;wrapper[2]=addr(match[0]);
    replay[0]=image+bindings::ReplayVtable;settings[0]=image+bindings::CameraSettingsVtable;
    put(addr(settings[0])+0x3e6,static_cast<unsigned short>(1));put(addr(settings[0])+0x41a,static_cast<unsigned short>(1));
    owner[0]=addr(data[0]);put(addr(data[0])+0x1f0,addr(views[0]));views[0]=addr(foreign[0]);views[0x68/8]=addr(view[0]);view[0]=foreign[0]=image+bindings::CameraVtable;
    global(bindings::MatchWrapper,addr(wrapper[0]));global(0xd9eb808,14,true);global(0xf3fddbc,3,true);
    global(bindings::PauseFlag,addr(paused));global(bindings::ReplayManager,addr(replay[0]));global(bindings::CameraSettings,addr(settings[0]));global(bindings::CameraOwner,addr(owner[0]));
    std::vector<A> selectedService(0x1c00/8);selectedService[0]=image+bindings::PlayCallServiceVtable;global(bindings::PlayCallService,addr(selectedService[0]));
    for(int side=0;side<2;++side){A team=addr(selectedService[0])+0x10+static_cast<A>(side)*0xde8;
        put(team,image+bindings::PlayCallTeamVtable);put(team+8,image+bindings::PlayCallSelectedVtable);put(team+16,image+bindings::PlayCallVtable);
        put(team+16+0x2c,side?22:3);put(team+16+0x30,12);
    }
    playcall::View plays(image,memoryRead);assert(plays.type(addr(match[0]),0)==3&&plays.type(addr(match[0]),1)==22);
    put(addr(selectedService[0])+0x20+0x2c,-1);assert(plays.type(addr(match[0]),0)==12);
    put(addr(selectedService[0])+0x20,A(0));assert(plays.type(addr(match[0]),0)==-1&&plays.type(addr(match[0]),-1)==-1);
    core::Settings venueSettings;State venue;venue.rank0=37;venue.rank1=34;
    assert(!lineup::cameraVenue(venueSettings,venue,.84f,true,26101));assert(lineup::cameraVenue(venueSettings,venue,.85f,true,26101));
    assert(!lineup::cameraVenue(venueSettings,venue,.90f,false));venue.rivalry=true;
    assert(lineup::cameraVenue(venueSettings,venue,.60f,true,53913)); // Both Top50 rivalry now qualifies.
    venue.rank1=51;assert(!lineup::cameraVenue(venueSettings,venue,.60f,true,53913));
    venue.rivalry=false;venue.rank0=6;venue.rank1=11;assert(lineup::cameraVenue(venueSettings,venue,.60f,true,53913));
    Runtime r;r.configure(image);Cue cue{addr(match[0]),2,2000,1000,0,Kind::Ordinary,5,false};
    std::array<float,16> source{1,0,0,0,0,1,0,0,0,0,1,0,4,12,20,1},output=source;
    std::array<float,16> projection{};projection[0]=2;projection[5]=3.1715948f;projection[11]=1;
    std::memcpy(reinterpret_cast<void*>(addr(view[0])+0x2b4),projection.data(),sizeof(projection));
    std::memcpy(reinterpret_cast<void*>(addr(view[0])+0x274),source.data(),sizeof(source));auto original=view;
    std::memcpy(reinterpret_cast<void*>(addr(foreign[0])+0x2b4),projection.data(),sizeof(projection));
    r.tick(addr(view[0]),output.data(),cue,cue.match,2,2000);assert(output!=source&&view==original&&r.changed()==1&&r.playing(2000)&&closeEnough(r.percent(),5.775f)&&closeEnough(r.framingScale(),1.05f));
    // The selection prelude is already building, but only its gameplay view
    // receives motion. All three presentation slots stay still for both
    // ordinary and critical buildup, including the new hostile opener.
    for(auto kind:{Kind::Ordinary,Kind::Critical})for(unsigned slot:{0u,2u,3u}){
        views[slot*0x68/8]=addr(foreign[0]);Cue next=cue;next.kind=kind;next.hostileOpening=15;
        output=source;auto before=r.changed();r.tick(addr(foreign[0]),output.data(),next,cue.match,2,2000);assert(output==source&&r.changed()==before);
    }
    // A missing gameplay view does not fall back to a presentation cutaway.
    views[0x68/8]=0;output=source;auto before=r.changed();r.tick(addr(foreign[0]),output.data(),cue,cue.match,2,2000);assert(output==source&&r.changed()==before);views[0x68/8]=addr(view[0]);
    // An actual snap changes render slots. The existing 900 ms lineup tail
    // follows that live view without changing phase, timer or native matrix.
    for(auto kind:{Kind::Ordinary,Kind::Critical}){
        motion::Model model;motion::Settings config;motion::Input input;input.match=cue.match;input.epoch=2;input.allowed=input.lineup=input.venueEligible=true;
        input.phase=1;input.lineupId=1;input.critical=kind==Kind::Critical;input.now=1000;model.observe(config,input);
        input.now=2000;auto lineupBefore=model.observe(config,input);float peak=strength(lineupBefore,input.now);
        input.phase=3;input.lineup=false;input.now=2100;auto tail=model.observe(config,input);
        put(addr(gameStates[0])+0xa0,3);
        for(unsigned age:{0u,225u,450u,675u}){
            input.now=2100+age;tail=model.observe(config,input);assert(tail.tail&&tail.until==3000);
            output=source;auto countBefore=r.changed();r.tick(addr(foreign[0]),output.data(),tail,cue.match,2,input.now);
            assert(output!=source&&r.changed()==countBefore+1&&r.unscaled()<=peak);
        }
        // At non-live presentation cuts the old slot restriction still applies.
        put(addr(gameStates[0])+0xa0,5);output=source;auto prior=r.changed();r.tick(addr(foreign[0]),output.data(),tail,cue.match,2,input.now);assert(output==source&&r.changed()==prior);
        input.now=3000;assert(model.observe(config,input).kind==Kind::None);
        put(addr(gameStates[0])+0xa0,1);
    }
    auto skipped=[&](A target,A currentMatch,unsigned epoch,uint64_t now){output=source;auto count=r.changed();r.tick(target,output.data(),cue,currentMatch,epoch,now);assert(output==source&&r.changed()==count&&view==original);};
    paused=1;skipped(addr(view[0]),cue.match,2,2000);paused=0;
    put(addr(replay[0])+0x38,static_cast<unsigned char>(1));skipped(addr(view[0]),cue.match,2,2000);put(addr(replay[0])+0x38,static_cast<unsigned char>(0));
    skipped(addr(view[0]),cue.match,3,2000);skipped(addr(view[0]),cue.match+8,2,2000);skipped(addr(view[0]),cue.match,2,2800);
    put(addr(settings[0])+0x41a,static_cast<unsigned short>(0));skipped(addr(view[0]),cue.match,2,2000);put(addr(settings[0])+0x41a,static_cast<unsigned short>(1));
    // Both native Dynamic Camera Effects profiles block every added category,
    // including earned close-ups, until re-enabled; no native setting is written.
    for(unsigned offset:{0x3e6u,0x41au})for(auto kind:{Kind::Ordinary,Kind::Critical,Kind::LivePlay,Kind::Touchdown,Kind::Eruption,Kind::Safety,Kind::BlockedKick,Kind::Clincher,Kind::Cannon}){
        auto gated=cue;gated.kind=kind;gated.until=17000;put(addr(settings[0])+offset,static_cast<unsigned short>(0));
        output=source;auto priorCount=r.changed();r.tick(addr(view[0]),output.data(),gated,cue.match,2,2000);
        assert(output==source&&r.changed()==priorCount);
        put(addr(settings[0])+offset,static_cast<unsigned short>(1));
        output=source;r.tick(addr(view[0]),output.data(),gated,cue.match,2,2000);assert(output!=source&&r.changed()==priorCount+1);
    }
    skipped(addr(foreign[0]),cue.match,2,2000);wrapper[2]=0;skipped(addr(view[0]),cue.match,2,2000);wrapper[2]=cue.match;
    auto count=r.changed();r.tick(addr(view[0]),reinterpret_cast<float*>(addr(view[0])+0x274),cue,cue.match,2,2000);assert(view==original&&count==r.changed());
    output=source;output[0]=2;r.tick(addr(view[0]),output.data(),cue,cue.match,2,2000);assert(count==r.changed());
    // The projection is read only; optical correction bounds close-up motion.
    std::memcpy(reinterpret_cast<void*>(addr(foreign[0])+0x2b4),projection.data(),sizeof(projection));
    for(auto kind:{Kind::LivePlay,Kind::Touchdown,Kind::Eruption,Kind::Safety,Kind::BlockedKick,Kind::Clincher,Kind::Cannon})for(unsigned slot:{0u,2u,3u}){
        views[slot*0x68/8]=addr(foreign[0]);Cue earned=cue;earned.kind=kind;earned.until=17000;
        output=source;auto countBefore=r.changed();r.tick(addr(foreign[0]),output.data(),earned,cue.match,2,2000);assert(output!=source&&r.changed()==countBefore+1);
        assert(earned.began==cue.began&&earned.updated==cue.updated); // Filtering never rewrites an earned cue's clock.
    }
    std::memcpy(reinterpret_cast<void*>(addr(view[0])+0x2b4),projection.data(),sizeof(projection));
    Cue celebration=cue;celebration.kind=Kind::Eruption;celebration.percent=22.5f;celebration.impact=30;celebration.until=17000;
    Runtime zoom;zoom.configure(image);output=source;auto unchanged=view;zoom.tick(addr(view[0]),output.data(),celebration,cue.match,2,2000);assert(closeEnough(zoom.percent(),34.65f)&&view==unchanged);
    projection[5]=12;std::memcpy(reinterpret_cast<void*>(addr(view[0])+0x2b4),projection.data(),sizeof(projection));unchanged=view;
    for(auto kind:{Kind::Ordinary,Kind::Critical}){Cue buildup=cue;buildup.kind=kind;
        output=source;auto changed=r.changed();r.tick(addr(view[0]),output.data(),buildup,cue.match,2,2000);assert(r.changed()==changed+1&&closeEnough(r.percent(),5*motion::projection::ReferenceStrength*motion::projection::framingGain(projection[5])*3.1715948f/12)&&view==unchanged);}
    output=source;zoom.tick(addr(view[0]),output.data(),celebration,cue.match,2,2000);assert(closeEnough(zoom.percent(),30*motion::projection::ReferenceStrength*motion::projection::framingGain(projection[5])*3.1715948f/12)&&view==unchanged);
    Cue defensive=cue;defensive.kind=Kind::LivePlay;defensive.impactOnly=true;defensive.percent=10;defensive.began=1000;defensive.until=3400;
    output=source;zoom.tick(addr(view[0]),output.data(),defensive,cue.match,2,2000);assert(zoom.percent()>0&&zoom.percent()<4.3f&&view==unchanged);
    // A camera cut swaps the owned render-view pointer; no borrowed camera
    // reference survives it. The same cue clock follows the new live view.
    views[0]=addr(foreign[0]);projection[5]=3.1715948f;
    std::memcpy(reinterpret_cast<void*>(addr(foreign[0])+0x2b4),projection.data(),sizeof(projection));
    auto nextOriginal=foreign;output=source;zoom.tick(addr(foreign[0]),output.data(),celebration,cue.match,2,2000);
    assert(output!=source&&closeEnough(zoom.percent(),34.65f)&&foreign==nextOriginal);
    // Live gain reactions also compensate for a suddenly zoomed-in camera.
    projection[5]=12;std::memcpy(reinterpret_cast<void*>(addr(foreign[0])+0x2b4),projection.data(),sizeof(projection));nextOriginal=foreign;
    Cue running=cue;running.kind=Kind::LivePlay;running.percent=10;
    output=source;zoom.tick(addr(foreign[0]),output.data(),running,cue.match,2,2000);assert(closeEnough(zoom.percent(),10*motion::projection::ReferenceStrength*motion::projection::framingGain(projection[5])*3.1715948f/12)&&foreign==nextOriginal);
    // Exercise the real renderer for every category, not just its pure scale
    // helper. Close/wide compensation leaves source data and cue timing intact.
    for(bool widePlayer:{false,true})for(auto kind:{Kind::Ordinary,Kind::Critical,Kind::LivePlay,Kind::Touchdown,Kind::Eruption,Kind::Safety,Kind::BlockedKick,Kind::Clincher,Kind::Cannon})for(float focal:{12.f,1.5857974f,1.0264003277f,.11f}){
        auto tested=cue;tested.kind=kind;tested.until=17000;projection[5]=focal;
        projection[0]=.5773501992f; // Captured default RTG horizontal focal length.
        std::memcpy(reinterpret_cast<void*>(addr(view[0])+0x2b4),projection.data(),sizeof(projection));auto saved=view;
        output=source;zoom.tick(addr(view[0]),output.data(),tested,cue.match,2,2000,widePlayer);
        assert(output!=source&&view==saved&&zoom.playing(tested,2000));
        const float ratio=3.1715948f/focal,expected=(std::min)(ratio,widePlayer?6.f:4.f);
        assert(std::fabs(zoom.unscaled()-5)<.001f&&std::fabs(zoom.zoomScale()-expected)<.001f);
        assert(std::fabs(zoom.percent()-5*motion::projection::ReferenceStrength*motion::projection::framingGain(projection[5])*zoom.zoomScale())<.001f);
        if(focal>=1&&focal<=3.1715948f&&widePlayer)assert(std::fabs(zoom.percent()*focal-5*motion::projection::ReferenceStrength*motion::projection::framingGain(projection[5])*3.1715948f)<.001f);
        auto other=tested;++other.began;assert(!zoom.playing(other,2000));
        assert(tested.began==1000&&tested.until==17000&&tested.percent==5);
    }
    // The captured wide RTG view keeps the strongest profile distinct from a
    // regular reaction, including the original impact-to-hold progression.
    projection[5]=1.0264003277f;std::memcpy(reinterpret_cast<void*>(addr(view[0])+0x2b4),projection.data(),sizeof(projection));
    unchanged=view;output=source;zoom.tick(addr(view[0]),output.data(),celebration,cue.match,2,2000,true);
    assert(zoom.unscaled()==30&&closeEnough(zoom.percent(),30*motion::projection::ReferenceStrength*motion::projection::framingGain(projection[5])*3.1715948f/1.0264003277f)&&view==unchanged);
    celebration.updated=3000;output=source;zoom.tick(addr(view[0]),output.data(),celebration,cue.match,2,3000,true);
    assert(zoom.unscaled()==22.5f&&closeEnough(zoom.percent(),22.5f*motion::projection::ReferenceStrength*motion::projection::framingGain(projection[5])*3.1715948f/1.0264003277f)&&view==unchanged);
    // The captured wide view now normalizes automatically. The extended
    // setting only changes the bound for more extreme projections.
    auto miami=cue;miami.kind=Kind::Critical;miami.percent=7.2f;
    output=source;zoom.tick(addr(view[0]),output.data(),miami,cue.match,2,2000);
    assert(std::fabs(zoom.unscaled()-7.2f)<.001f&&closeEnough(zoom.zoomScale(),3.1715948f/1.0264003277f)&&closeEnough(zoom.percent(),7.2f*motion::projection::ReferenceStrength*motion::projection::framingGain(projection[5])*3.1715948f/1.0264003277f));
    output=source;zoom.tick(addr(view[0]),output.data(),miami,cue.match,2,2000,true);
    assert(closeEnough(zoom.percent(),7.2f*motion::projection::ReferenceStrength*motion::projection::framingGain(projection[5])*3.1715948f/1.0264003277f));
    // Earned effects follow a verified cutaway outside all four cached views.
    views[0]=views[2*0x68/8]=views[3*0x68/8]=0;
    output=source;zoom.tick(addr(foreign[0]),output.data(),running,cue.match,2,2000);assert(output!=source);
    views[0]=addr(view[0]);
    std::memcpy(reinterpret_cast<void*>(addr(view[0])+0x2b4),reinterpret_cast<const unsigned char*>(original.data())+0x2b4,sizeof(projection));
    // Personal settings multiply once, independently of optics and framing.
    Runtime scaled;scaled.configure(image);projection[0]=2;projection[5]=3.1715948f;projection[11]=1;
    std::memcpy(reinterpret_cast<void*>(addr(view[0])+0x2b4),projection.data(),sizeof(projection));
    Cue fixed{cue.match,2,2000,1000,10000,Kind::LivePlay,10,false};
    for(float factor:{.5f,1.f,1.25f,2.f}){
        output=source;scaled.tick(addr(view[0]),output.data(),fixed,cue.match,2,2000,false,factor);
        assert(output!=source&&scaled.zoomScale()==1&&scaled.presetScale()==factor&&std::fabs(scaled.percent()-strength(fixed,2000)*motion::projection::ReferenceStrength*motion::projection::framingGain(projection[5])*factor)<.001f);
    }
    auto unchangedCount=scaled.changed();output=source;scaled.tick(addr(view[0]),output.data(),fixed,cue.match,2,2000,false,0);assert(output==source&&scaled.changed()==unchangedCount);
    output=source;scaled.tick(addr(view[0]),output.data(),fixed,cue.match,2,2000,false,3);assert(output==source&&scaled.changed()==unchangedCount);
    // Actual output follows the captured broad/tight lenses and a custom
    // intermediate lens without a preset lookup. All axes get the bias once,
    // across effect categories and independent user preferences.
    Runtime framed;framed.configure(image);
    for(auto sample:{std::array<float,2>{1.9437481f,.675f},std::array<float,2>{2.9128082f,1.05f},std::array<float,2>{2.42827815f,.8625f}}){
        projection[5]=sample[0];projection[0]=sample[0]*9.f/16;
        std::memcpy(reinterpret_cast<void*>(addr(view[0])+0x2b4),projection.data(),sizeof(projection));auto saved=view;
        for(auto kind:{Kind::Ordinary,Kind::Critical,Kind::LivePlay,Kind::Touchdown,Kind::Eruption,Kind::Safety,Kind::BlockedKick,Kind::Clincher,Kind::Cannon})for(float preference:{.5f,1.f,1.25f,2.f}){
            auto tested=fixed;tested.kind=kind;auto expected=source;output=source;
            assert(apply(expected,tested,2000,3.1715948f/sample[0],3.1715948f/sample[0],1.10f*sample[1]*preference));
            framed.tick(addr(view[0]),output.data(),tested,cue.match,2,2000,false,preference);
            for(size_t i=0;i<output.size();++i)assert(std::fabs(output[i]-expected[i])<.00001f);
            assert(view==saved&&closeEnough(framed.framingScale(),sample[1])&&framed.presetScale()==preference);
            assert(closeEnough(framed.percent(),strength(tested,2000)*1.10f*sample[1]*preference*3.1715948f/sample[0]));
        }
    }
    projection[0]=2;projection[5]=3.1715948f;
    std::memcpy(reinterpret_cast<void*>(addr(view[0])+0x2b4),projection.data(),sizeof(projection));
    // Invalid projections may briefly reuse only this runtime/view/world's
    // last good lens. Expiry and epoch changes must leave the output untouched.
    Runtime lensLease;lensLease.configure(image);auto leased=fixed;
    output=source;lensLease.tick(addr(view[0]),output.data(),leased,cue.match,2,2000);
    std::array<float,16> missing{};std::memcpy(reinterpret_cast<void*>(addr(view[0])+0x2b4),missing.data(),sizeof(missing));
    leased.updated=2100;output=source;lensLease.tick(addr(view[0]),output.data(),leased,cue.match,2,2100);
    assert(output!=source&&lensLease.cached()==1);
    leased.updated=2300;output=source;auto validOutputs=lensLease.changed();lensLease.tick(addr(view[0]),output.data(),leased,cue.match,2,2300);
    assert(output==source&&lensLease.changed()==validOutputs&&lensLease.invalid()==1);
    leased.epoch=3;leased.updated=2200;output=source;lensLease.tick(addr(view[0]),output.data(),leased,cue.match,3,2200);
    assert(output==source&&lensLease.invalid()==2);
    lensLease.configure(image);leased.epoch=2;leased.updated=2100;output=source;lensLease.tick(addr(view[0]),output.data(),leased,cue.match,2,2100);
    assert(output==source&&lensLease.invalid()==3);
    std::memcpy(reinterpret_cast<void*>(addr(view[0])+0x2b4),projection.data(),sizeof(projection));
    // Distinguish a prepared visitor request from an actual matrix change.
    Runtime visitor;visitor.configure(image);auto away=fixed;away.visitor=true;away.percent=3;away.playSequence=19;
    assert(visitor.prepare(addr(view[0]),away,cue.match,2,2000)&&visitor.visitorChanged()==0);
    output=source;visitor.tick(addr(view[0]),output.data(),away,cue.match,2,2000,false,1.1f);
    assert(output!=source&&visitor.visitorChanged()==1&&closeEnough(visitor.visitorPercent(),3*motion::projection::ReferenceStrength*motion::projection::framingGain(projection[5])*1.1f)&&visitor.visitorPeak()==visitor.visitorPercent());
    away.percent=2;output=source;visitor.tick(addr(view[0]),output.data(),away,cue.match,2,2000);
    assert(visitor.visitorChanged()==2&&closeEnough(visitor.visitorPercent(),2.31f)&&closeEnough(visitor.visitorPeak(),3*motion::projection::ReferenceStrength*motion::projection::framingGain(projection[5])*1.1f));
    paused=1;output=source;visitor.tick(addr(view[0]),output.data(),away,cue.match,2,2000);
    assert(output==source&&visitor.visitorChanged()==2);paused=0;
    r.fail();skipped(addr(view[0]),cue.match,2,2000);assert(!r.healthy());
    // Vtable-pointer ownership: restore only the pointer we installed.
    auto page=VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);assert(page);auto slot=reinterpret_cast<A*>(page);*slot=123;
    DWORD old=0;assert(VirtualProtect(page,4096,PAGE_READONLY,&old));PointerHook h;
    assert(!h.install(addr(*slot),456,789)&&*slot==123);assert(h.install(addr(*slot),123,789)&&*slot==789);
    MEMORY_BASIC_INFORMATION info{};assert(VirtualQuery(page,&info,sizeof(info))&&info.Protect==PAGE_READONLY);
    assert(h.restore()&&*slot==123);assert(h.install(addr(*slot),123,789));
    assert(VirtualProtect(page,4096,PAGE_READWRITE,&old));*slot=999;assert(VirtualProtect(page,4096,PAGE_READONLY,&old));
    assert(!h.restore()&&*slot==999);assert(VirtualQuery(page,&info,sizeof(info))&&info.Protect==PAGE_READONLY);
    VirtualFree(page,0,MEM_RELEASE);for(auto p:pages)VirtualFree(p,0,MEM_RELEASE);
    std::cout<<"PASS: gameplay-only ordinary/critical/hostile buildup, earned reaction continuity across presentation views, output-only motion, source/world guards, pause/replay/accessibility, stale cue rejection and owner-safe vtable hook restoration.\n";
}
