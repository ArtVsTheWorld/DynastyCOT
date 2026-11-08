#pragma once
#include "PresentationCamera.h"

namespace attendance::camera {
// Arm behind play selection, then alter only a native camera selection. Never
// switch to a shake preset from a later frame tick: the preset also changes
// framing, and a late switch produces a visible camera jump.
class HandoffRuntime {
    Address image_=0,camera_=0,match_=0;unsigned epoch_=0;
    int off_=-1,high_=-1,saved_=-1;uint64_t seen_=0;
    Api api_;std::mutex mutex_;
    std::atomic<bool> healthy_{true},applied_{false},armed_{false};
    std::atomic<Address> publishedView_{0};
    std::atomic<unsigned> starts_{0},restores_{0},skipped_{0},retired_{0},fault_{0};
    void fault(unsigned code){unsigned empty=0;fault_.compare_exchange_strong(empty,code);healthy_=false;}
    Address row(Address view,int code)const{
        Address b=at<Address>(view+0x420),e=at<Address>(view+0x428);
        if(!b||e<b||e-b>4096||(e-b)%16)return 0;
        for(Address p=b;p<e;p+=16)if(at<unsigned>(p+4)==static_cast<unsigned>(code)>>16)return p;
        return 0;
    }
    void releaseRef(Address view){
        Address vt=at<Address>(view);
        auto release=reinterpret_cast<void(__fastcall*)(Address)>(at<Address>(vt+8));
        // CameraGet returns a referenced native interface, including broadcast
        // and replay views. Such views are not HFA cameras, but their acquired
        // reference still needs releasing. Require the exact same native
        // AddRef/Release functions before using the shared interface contract.
        Address known=image_+bindings::CameraVtable;
        bool compatible=vt==known||(at<Address>(known)&&at<Address>(vt)==at<Address>(known)&&
            reinterpret_cast<Address>(release)==at<Address>(known+8));
        if(!compatible||!release)fault(1);
        else if(!sustained::invoke(release,view))fault(5);
    }
    bool accessibility()const{
        Address s=at<Address>(image_+bindings::CameraSettings);
        return at<Address>(s)==image_+bindings::CameraSettingsVtable&&
            at<unsigned short>(s+0x3e6)&&at<unsigned short>(s+0x41a);
    }
    bool eligible(const sustained::Intent& w,Address match,unsigned epoch,uint64_t now)const{
        return healthy_&&sustained::fresh(w,match,epoch,now)&&w.shake&&accessibility();
    }
    bool binding(Address view,int off,int high){
        Address disabled=0,tier3=0;
        if(!sustained::obtain(disabled,api_.find,view,"EVENT_HFA_DISABLED")||
           !sustained::obtain(tier3,api_.find,view,"EVENT_HFA_TIER_3_ON")){fault(2);return false;}
        return static_cast<int>(disabled)==off&&static_cast<int>(tier3)==high;
    }
    void forget(){camera_=match_=0;epoch_=0;off_=high_=saved_=-1;seen_=0;applied_=armed_=false;publishedView_=0;}
    void retire(){
        // A level transition can retire its camera allocation despite an old
        // interface reference. Once match/epoch ownership changes, the address
        // is only an obsolete identity: never restore, look up or release it.
        if(camera_)++retired_;forget();
    }
    void clear(){
        if(!camera_)return;
        // A rebuilt scene can also retire a view within the same match. Do
        // not interpret its old channels or dispatch through a changed type.
        if(at<Address>(camera_)!=image_+bindings::CameraVtable){retire();return;}
        Address r=row(camera_,off_);
        int current=r?at<int>(r+8):-1;
        if(applied_&&r&&(current==off_+5||current==off_+7||current==high_)&&saved_>=off_&&saved_<=high_&&binding(camera_,off_,high_)){
            if(sustained::invoke(api_.set,camera_,saved_,false))++restores_;else fault(3);
        }
        Address old=camera_;forget();releaseRef(old);
    }
public:
    void configure(Address image,const Api* custom=nullptr){
        image_=image;
        if(custom)api_=*custom;
        else {api_.get=reinterpret_cast<Api::Get>(image+bindings::CameraGet);api_.find=reinterpret_cast<Api::Find>(image+bindings::CameraFind);api_.set=reinterpret_cast<Api::Set>(image+bindings::CameraSet);}
    }
    void tick(const sustained::Intent& w,Address match,unsigned epoch,uint64_t now,bool worldCurrent=true){
        std::lock_guard<std::mutex> guard(mutex_);
        if(!worldCurrent){retire();return;}
        // Check ownership BEFORE an inactive/loading lease tries to restore
        // the previous world. New games can reuse the same match pointer.
        if(camera_&&(match_!=match||epoch_!=epoch))retire();
        if(!eligible(w,match,epoch,now)){clear();return;}
        // 1 is the gameplay view behind play selection. 4 requests the game's
        // automatic active view; it is not an array slot.
        Address owner=at<Address>(image_+bindings::CameraOwner),view=0,unused=0;
        if(!owner||!sustained::obtain(unused,api_.get,owner,&view,w.cameraPrime?1:4)||!view){clear();return;}
        if(at<Address>(view)!=image_+bindings::CameraVtable){
            // Automatic camera selection legitimately visits other native
            // presentation classes. Skip them instead of disabling shake for
            // every later game in this process. Never inspect their HFA data.
            ++skipped_;releaseRef(view);clear();return;
        }
        if(camera_&&camera_!=view)clear();
        if(camera_&&!binding(camera_,off_,high_))clear();
        if(camera_){seen_=now;releaseRef(view);return;}
        if(!w.cameraPrime){releaseRef(view);return;} // A missed handoff gets no late override.
        Address disabled=0,tier3=0;
        if(!sustained::obtain(disabled,api_.find,view,"EVENT_HFA_DISABLED")||
           !sustained::obtain(tier3,api_.find,view,"EVENT_HFA_TIER_3_ON")){fault(2);releaseRef(view);return;}
        int off=static_cast<int>(disabled),high=static_cast<int>(tier3);
        if(off<0||high-off!=9||(off>>16)!=(high>>16)||!row(view,off)){releaseRef(view);return;}
        camera_=view;match_=match;epoch_=epoch;off_=off;high_=high;seen_=now;armed_=true;publishedView_=view;
        // An unset channel is normal while the gameplay camera is rebuilt.
        // Wait for a real native event instead of inventing an initial value.
        saved_=at<int>(row(view,off)+8);
    }
    int selectNative(Address view,int code,const sustained::Intent& w,Address match,unsigned epoch,uint64_t now){
        std::lock_guard<std::mutex> guard(mutex_);
        if(view!=camera_)return code;
        if(code<off_||code>high_){applied_=false;return code;}
        saved_=code;
        if(match_!=match||epoch_!=epoch||!eligible(w,match,epoch,now)||now<seen_||now-seen_>750){applied_=false;return code;}
        if(!row(view,off_)||!binding(view,off_,high_)){applied_=false;return code;}
        int tier=(std::clamp)(w.shakeTier>0?w.shakeTier:1,1,3),desired=off_+3+2*tier;
        if(code==off_+5||code==off_+7||code==high_)desired=(std::max)(desired,code);
        if(desired!=code&&!applied_)++starts_;
        applied_=desired!=code;return desired;
    }
    bool playing()const{return applied_;}bool armed()const{return armed_;}bool healthy()const{return healthy_;}
    bool owns(Address view)const{return view&&publishedView_==view;}
    unsigned starts()const{return starts_;}unsigned restores()const{return restores_;}
    unsigned skippedViews()const{return skipped_;}
    unsigned retiredViews()const{return retired_;}
    const char* failure()const{
        switch(fault_.load()){
        case 1:return "camera reference validation";case 2:return "camera event lookup";
        case 3:return "camera event restoration";case 4:return "unexpected camera exception";
        case 5:return "native camera release call";
        default:return "none";
        }
    }
    void fail(){fault(4);}
};
}
