#pragma once
#include <windows.h>
#include <atomic>
#include <algorithm>
#include <map>
#include <memory>
#include <mutex>
#include "ChantMixerPreset.h"
#include "GameBindings.h"

namespace attendance::chants {
using Create=Address(__fastcall*)(Address,Address);
using Release=void(__fastcall*)(Address,Address);
using Note=void(*)(const char*);
using Manager=Address(*)();
using Clock=unsigned long long(*)();
class Runtime {
    Address image_=0,manager_=0,handle_=0,asset_=0;
    Create create_=nullptr;Release release_=nullptr;Note note_=nullptr;
    Manager managerSource_=nullptr;
    Templates templates_;bool haveTemplates_=false;
    unsigned short selectedGroup_=14;bool muted_=false;
    float positiveLimit_=0;unsigned short positiveGroup_=9;
    Clock clock_=nullptr;unsigned long long lastPositiveChange_=0;
    std::map<int,std::unique_ptr<Preset>> presets_;
    std::mutex mutex_;std::atomic<bool> healthy_{true};
    std::atomic<float> db_{0};std::atomic<unsigned long long> creations_{0},releases_{0};
    template<class T>bool read(Address p,T& value){return copy(p,&value,sizeof(value));}
    static bool copy(Address p,void* value,size_t bytes){SIZE_T count=0;return p>=0x10000&&ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(p),value,bytes,&count)&&count==bytes;}
    template<class T>T at(Address p){T value{};return read(p,value)?value:T{};}
    Address ref(Address p){return at<Address>(p)&~Address(4);}
    bool named(Address p,const char* name){char text[128]{};size_t n=std::strlen(name)+1;return n<=sizeof(text)&&copy(p,text,n)&&!std::memcmp(text,name,n);}
    bool borrowedOwner(Address owner){
        return owner>=0x10040&&at<Address>(owner-0x40)==image_+0xb227098&&
            at<Address>(owner)==image_+0xb227178&&at<Address>(owner+8)==image_+0xb227188;
    }
    static bool tryLock(CRITICAL_SECTION* cs){
        __try {return TryEnterCriticalSection(cs)!=0;}
        __except(EXCEPTION_EXECUTE_HANDLER){return false;}
    }
    bool templates(Address manager){
        // The manager's own create/remove functions use this Windows critical
        // section. Copy only while held; never retain borrowed asset pointers.
        auto cs=reinterpret_cast<CRITICAL_SECTION*>(manager+0x30);
        if(!tryLock(cs))return false;
        bool result=false;
        Address begin=at<Address>(manager+0xc0),end=at<Address>(manager+0xc8);
        if(begin>=0x10000&&end>=begin&&end-begin<=32*512&&(end-begin)%32==0){
            for(Address p=begin;p<end&&!result;p+=32){
                Address handle=at<Address>(p);if(at<Address>(handle)!=image_+0xaf01718)continue;
                Address asset=ref(handle+8);
                Address name=at<Address>(asset+0x18);
                if(!named(name,"Sound/CFB/CoreAudio/Utility/Mixers/VolumeSettings_MXR")&&!named(name,"Sound/CFB/CoreAudio/Utility/Mixers/MasterMixer_MXR"))continue;
                Address graph=ref(asset+0x28),preset=ref(asset+0x20),groups=at<Address>(preset+0x28);
                unsigned count=at<unsigned>(groups-4)&0x7fffffff;if(count>128)continue;
                for(unsigned i=0;i<count;++i){
                    Address entry=groups+i*0x28,group=ref(entry);
                    if(at<unsigned short>(group+0x28)!=selectedGroup_)continue;
                    result=copy(asset-16,templates_.asset.data(),templates_.asset.size())&&
                        copy(graph-16,templates_.graph.data(),templates_.graph.size())&&
                        copy(preset-16,templates_.preset.data(),templates_.preset.size())&&
                        copy(group-16,templates_.group.data(),templates_.group.size())&&
                        copy(entry,templates_.entry.data(),templates_.entry.size());
                    templates_.groupIndex=selectedGroup_;templates_.parent=at<unsigned short>(group+0x2a);templates_.properties.fill(0);
                    Address label=at<Address>(group+0x20);std::memset(templates_.name,0,sizeof(templates_.name));
                    for(size_t j=0;j<sizeof(templates_.name)-1;++j){char c=0;if(!read(label+j,c)){result=false;break;}templates_.name[j]=c;if(!c)break;}
                    if(selectedGroup_==11&&(templates_.parent!=2||std::strcmp(templates_.name,"WeatherAndAmbience")!=0))result=false;
                    if(selectedGroup_==86&&(templates_.parent!=8||std::strcmp(templates_.name,"ReverbMaster")!=0))result=false;
                    if(selectedGroup_==32&&(templates_.parent!=3||std::strcmp(templates_.name,"PAAnnouncer")!=0))result=false;
                    if(selectedGroup_==12&&(templates_.parent!=9||std::strcmp(templates_.name,"Anticipation_Home_Pass")!=0))result=false;
                    if(selectedGroup_==14&&(templates_.parent!=9||std::strcmp(templates_.name,"Chants/Response")!=0))result=false;
                    if(selectedGroup_==16&&(templates_.parent!=9||std::strcmp(templates_.name,"StudentSection")!=0))result=false;
                    if(selectedGroup_==17&&(templates_.parent!=9||std::strcmp(templates_.name,"Sweeteners")!=0))result=false;
                    if(selectedGroup_==23&&(templates_.parent!=17||std::strcmp(templates_.name,"CrowdNIS")!=0))result=false;
                    if(selectedGroup_==77&&(templates_.parent!=5||std::strcmp(templates_.name,"DrumCadence")!=0))result=false;
                    if(selectedGroup_==13&&(templates_.parent!=9||std::strcmp(templates_.name,"Beds")!=0))result=false;
                    if(selectedGroup_==15&&(templates_.parent!=9||std::strcmp(templates_.name,"Reactions")!=0))result=false;
                    if(selectedGroup_==18&&(templates_.parent!=9||std::strcmp(templates_.name,"Swells")!=0))result=false;
                    if(selectedGroup_==9&&(templates_.parent!=2||std::strcmp(templates_.name,"Crowd")!=0))result=false;
                    if(selectedGroup_==74&&(templates_.parent!=5||std::strcmp(templates_.name,"BandAway")!=0))result=false;
                    if(selectedGroup_==75&&(templates_.parent!=5||std::strcmp(templates_.name,"BandHome")!=0))result=false;
                    break;
                }
            }
        }
        LeaveCriticalSection(cs);return result;
    }
    bool guardedCreate(Address manager,Address asset,Address& handle){
        __try {handle=create_(manager,asset);return handle!=0;}
        __except(EXCEPTION_EXECUTE_HANDLER){return false;}
    }
    bool guardedRelease(Address manager,Address handle){
        __try {release_(manager,handle);return true;}
        __except(EXCEPTION_EXECUTE_HANDLER){return false;}
    }
    void fault(){healthy_=false;if(note_)note_("mixer-fault: owned atmosphere gain layer stopped");}
public:
    bool snapshot(Address manager,unsigned short group,Templates& output){
        selectedGroup_=group;if(!templates(manager))return false;output=templates_;return true;
    }
    void configure(Address image,Note note=nullptr,Create create=nullptr,Release release=nullptr,Manager source=nullptr,float positiveLimit=0,Clock clock=nullptr,unsigned short positiveGroup=9){
        image_=image;note_=note;create_=create?create:reinterpret_cast<Create>(image+bindings::MixerCreate);
        release_=release?release:reinterpret_cast<Release>(image+bindings::MixerRelease);managerSource_=source;
        positiveLimit_=(std::clamp)(positiveLimit,0.0f,16.0f);
        clock_=clock;positiveGroup_=positiveGroup;
    }
    void tick(Address owner,bool active,float requestedDb,bool forceNeutral=false,bool mute=false,unsigned short group=14,bool immediateReduction=false){
        if(!borrowedOwner(owner)||!std::isfinite(requestedDb))return;
        if(group<1||group>87)return;
        std::lock_guard<std::mutex> lock(mutex_);
        Address manager=managerSource_?managerSource_():at<Address>(image_+bindings::MixerManager);
        // Manager teardown retires its handles. Do not dereference or release
        // an old handle through a new context, even if its asset remains ours.
        if(manager!=manager_){manager_=manager;handle_=asset_=0;db_=0;haveTemplates_=false;lastPositiveChange_=0;}
        if(!manager)return;
        // Range permits empty-crowd attenuation and stock preplay boost offsets.
        // This is an additive level, not an absolute output target.
        bool spatial=group==11||group==86;
        float ceiling=group==positiveGroup_&&(spatial||group==9||group==13||group==15||group==18)?(spatial?(std::min)(positiveLimit_,6.f):positiveLimit_):0;
        float desired=active?std::round((std::fmax)(-160.0f,(std::fmin)(ceiling,requestedDb))*4)*0.25f:0;
        bool wanted=healthy_&&active&&(desired!=0||forceNeutral||mute);
        auto time=clock_?clock_():GetTickCount64();
        // Native presets ease for 250 ms. Let each positive-gain transition
        // finish before replacing it again, rather than accumulating fades.
        // Disable, zero gain, group changes and owner teardown remain immediate.
        if(handle_&&(group==9||group==13||group==15||group==18)&&selectedGroup_==group&&desired>0&&db_>0&&wanted&&mute==muted_&&
           std::fabs(desired-db_.load())>=0.125f&&time-lastPositiveChange_<300&&!(group!=9&&desired<db_)&&!(immediateReduction&&desired<db_))return;
        if(handle_&&(!wanted||std::fabs(desired-db_.load())>=0.125f||group!=selectedGroup_||mute!=muted_)){
            if(at<Address>(handle_)!=image_+0xaf01718||ref(handle_+8)!=asset_||!guardedRelease(manager,handle_)){fault();return;}
            ++releases_;handle_=asset_=0;db_=0;
        }
        if(group!=selectedGroup_){selectedGroup_=group;haveTemplates_=false;}
        if(!wanted||handle_)return;
        if(!haveTemplates_){if(!templates(manager))return;haveTemplates_=true;if(note_)note_("group-layout: native additive preset templates verified");}
        int key=(int(group)<<12)|(mute?2048:0)|static_cast<int>(std::round((desired+160)*4));auto found=presets_.find(key);
        if(found==presets_.end())found=presets_.emplace(key,std::make_unique<Preset>(templates_,desired,uint64_t(key)+0x10000,mute)).first;
        Address asset=found->second->asset.address(),handle=0;
        if(!guardedCreate(manager,asset,handle)){fault();return;}
        if(at<Address>(handle)!=image_+0xaf01718||ref(handle+8)!=asset){fault();return;}
        handle_=handle;asset_=asset;db_=desired;muted_=mute;++creations_;
        if((group==9||group==13||group==15||group==18)&&desired>0)lastPositiveChange_=time;
        if(note_)note_("group-mixer: owned layer created for the selected group only");
    }
    bool healthy()const{return healthy_.load();}
    float db()const{return db_.load();}
    unsigned long long creations()const{return creations_.load();}
    unsigned long long releases()const{return releases_.load();}
};
}
