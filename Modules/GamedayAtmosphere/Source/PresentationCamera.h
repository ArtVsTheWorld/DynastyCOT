#pragma once
#include "SwellAudioRuntime.h"

namespace attendance::camera {
using sustained::Address;
using sustained::at;
struct Api {
    using Get=Address(__fastcall*)(Address,Address*,int);
    using Find=int(__fastcall*)(Address,const char*);
    using Set=void(__fastcall*)(Address,int,bool);
    Get get=nullptr;Find find=nullptr;Set set=nullptr;
};
// Only camera events are selected. Gameplay HFA, player modifiers, the Pulse
// meter and clocks are never written. All camera access is on presentation update.
class Runtime {
    Address image_=0,camera_=0,match_=0;unsigned epoch_=0;
    int saved_=-1,off_=-1,high_=-1,maximum_=-1;uint64_t seen_=0;
    Address candidate_=0,candidateMatch_=0;unsigned candidateEpoch_=0;int candidateEvent_=-1;uint64_t candidateAt_=0,settleMs_=500;
    Api api_;std::mutex mutex_;
    std::atomic<bool> applied_{false},healthy_{true};
    std::atomic<unsigned> starts_{0},restores_{0};
    void releaseRef(Address camera){
        Address vt=at<Address>(camera);
        auto release=reinterpret_cast<void(__fastcall*)(Address)>(at<Address>(vt+8));
        if(vt!=image_+bindings::CameraVtable||!release||!sustained::invoke(release,camera))healthy_=false;
    }
    Address channel(Address camera,int code)const{
        Address b=at<Address>(camera+0x420),e=at<Address>(camera+0x428);
        if(!b||e<b||e-b>4096||(e-b)%16)return 0;
        for(Address p=b;p<e;p+=16)if(at<unsigned>(p+4)==static_cast<unsigned>(code)>>16)return p;
        return 0;
    }
    void clear(){Address owned=camera_;camera_=match_=candidate_=0;epoch_=0;saved_=off_=high_=maximum_=-1;seen_=0;applied_=false;if(owned)releaseRef(owned);}
    bool restoreOwned(){
        if(!camera_)return true;
        Address row=channel(camera_,off_);
        if(row&&at<int>(row+8)==high_&&saved_>=off_&&saved_<=maximum_){
            if(!sustained::invoke(api_.set,camera_,saved_,false)){healthy_=false;return false;}
            ++restores_;
        }
        clear();return true;
    }
    bool accessibility()const{
        Address settings=at<Address>(image_+bindings::CameraSettings);
        if(at<Address>(settings)!=image_+bindings::CameraSettingsVtable)return false;
        // IsAccessibilityCameraEffectsEnabled(team) -> setting 0x15 in the
        // native two-team accessibility array. Match the authored AND gate.
        return at<unsigned short>(settings+0x3bc+0x15*2)!=0&&
            at<unsigned short>(settings+0x3bc+(0x1a+0x15)*2)!=0;
    }
    bool inspect(Address camera,const sustained::Intent& wanted,Address match,unsigned epoch,uint64_t now){
        std::lock_guard<std::mutex> guard(mutex_);
        // A native reference is retained for the duration of an override. This
        // lets replay/menu transitions restore the old view before releasing
        // it, without ever dereferencing a retired non-owning pointer.
        bool active=healthy_&&sustained::fresh(wanted,match,epoch,now)&&wanted.lineup&&wanted.shake&&accessibility();
        bool sameContext=match_==match&&epoch_==epoch;
        if(camera_&&(camera_!=camera||!active||!sameContext))if(!restoreOwned())return false;
        if(!active){candidate_=0;return false;}
        Address offValue=0,highValue=0;
        if(!sustained::obtain(offValue,api_.find,camera,"EVENT_HFA_DISABLED")||
           !sustained::obtain(highValue,api_.find,camera,"EVENT_HFA_TIER_3_ON")){healthy_=false;return false;}
        int off=static_cast<int>(offValue),high=static_cast<int>(highValue);
        Address row=off>=0?channel(camera,off):0;
        if(!row||off<0||high<=off||high-off!=9||(off>>16)!=(high>>16)){clear();return false;}
        Address tierValue=0;
        int tier=(std::clamp)(wanted.shakeTier>0?wanted.shakeTier:3,1,3);
        const char* tierName=tier==1?"EVENT_HFA_TIER_1_ON":tier==2?"EVENT_HFA_TIER_2_ON":"EVENT_HFA_TIER_3_ON";
        if(!sustained::obtain(tierValue,api_.find,camera,tierName)){healthy_=false;return false;}
        int desired=static_cast<int>(tierValue);if(desired<off||desired>high)return false;
        int current=at<int>(row+8);
        bool known=current>=off&&current<=high;
        if(!known){clear();return false;}
        // Do not fire an HFA transition while the engine is still replacing
        // the lineup view or changing its native camera event. Candidate
        // addresses are identity keys only; each tick owns a fresh reference.
        if(!camera_&&settleMs_){
            if(candidate_!=camera||candidateMatch_!=match||candidateEpoch_!=epoch||candidateEvent_!=current||now<candidateAt_){
                candidate_=camera;candidateMatch_=match;candidateEpoch_=epoch;candidateEvent_=current;candidateAt_=now;return false;
            }
            if(now-candidateAt_<settleMs_)return false;
        }
        bool retain=!camera_;
        if(retain){camera_=camera;match_=match;epoch_=epoch;off_=off;maximum_=high;saved_=current;++starts_;}
        else if(current!=high_)saved_=current;
        // Preserve stronger camera shake selected by the game itself.
        if(saved_==off+5||saved_==off+7||saved_==off+9)desired=(std::max)(desired,saved_);
        high_=desired;
        seen_=now;applied_=true; // Also restore after a partially completed call.
        if(current!=high_&&!sustained::invoke(api_.set,camera,high_,false))healthy_=false;
        return retain;
    }
public:
    void configure(Address image,const Api* custom=nullptr,uint64_t settleMs=500){
        image_=image;settleMs_=settleMs;
        if(custom)api_=*custom;
        else {api_.get=reinterpret_cast<Api::Get>(image+bindings::CameraGet);api_.find=reinterpret_cast<Api::Find>(image+bindings::CameraFind);api_.set=reinterpret_cast<Api::Set>(image+bindings::CameraSet);}
    }
    // Called by the native setter observer. Our calls use its original
    // trampoline, so the saved value always comes from the game itself.
    void observeNative(Address camera,int code,uint64_t now){
        if(!applied_)return;
        std::lock_guard<std::mutex> guard(mutex_);
        if(applied_&&camera==camera_&&now>=seen_&&now-seen_<750&&code>=off_&&code<=maximum_)saved_=code;
    }
    void tick(const sustained::Intent& wanted,Address match,unsigned epoch,uint64_t now){
        if(!applied_&&(!healthy_||!wanted.active||!wanted.shake)){std::lock_guard<std::mutex> guard(mutex_);candidate_=0;return;}
        Address owner=at<Address>(image_+bindings::CameraOwner),camera=0,unused=0;
        if(!owner||!sustained::obtain(unused,api_.get,owner,&camera,4)||!camera){std::lock_guard<std::mutex> guard(mutex_);restoreOwned();return;}
        Address vt=at<Address>(camera);
        if(vt!=image_+bindings::CameraVtable){
            healthy_=false;std::lock_guard<std::mutex> guard(mutex_);restoreOwned();return;
        }
        if(!inspect(camera,wanted,match,epoch,now))releaseRef(camera);
    }
    bool playing()const{return applied_;}bool healthy()const{return healthy_;}
    unsigned starts()const{return starts_;}unsigned restores()const{return restores_;}
    void fail(){healthy_=false;}
};
}
