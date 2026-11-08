#pragma once
#include "SwellAudioRuntime.h"
#include <map>

namespace attendance::sparsecamera {
using sustained::Address;
using sustained::at;
struct Lease {Address match=0;unsigned epoch=0;uint64_t updated=0;bool suppress=false;};
inline bool fresh(const Lease& p,uint64_t now){return p.match&&now>=p.updated&&now-p.updated<=750;}
struct Api {
    using Get=Address(__fastcall*)(Address,Address*,int);
    using Find=int(__fastcall*)(Address,const char*);
    using Set=void(__fastcall*)(Address,int,bool);
    using Release=void(__fastcall*)(Address);
    Get get=nullptr;Find find=nullptr;Set set=nullptr;Release release=nullptr;
};
class Runtime {
    struct State {Address rows=0;Address match=0;unsigned epoch=0;uint64_t seen=0;int off=-1,high=-1,saved=-1;bool applied=false;};
    Address image_=0;Api api_;std::mutex mutex_;std::map<Address,State> states_;
    std::atomic<bool> healthy_{true},active_{false};std::atomic<unsigned> switches_{0},restores_{0},filtered_{0};
    static bool on(int code,int off){return code==off+1||code==off+3||code==off+5||code==off+7||code==off+9;}
    Address channel(Address camera,int off)const{
        Address b=at<Address>(camera+0x420),e=at<Address>(camera+0x428);
        if(!b||e<b||e-b>4096||(e-b)%16)return 0;
        for(Address p=b;p<e;p+=16)if(at<unsigned>(p+4)==static_cast<unsigned>(off)>>16)return p;
        return 0;
    }
    // Caller holds mutex_. The camera is borrowed from either CameraGet or
    // its native Set callback; stored addresses remain identity keys only.
    State* remember(Address camera,const Lease& p,uint64_t now){
        Address offValue=0,highValue=0;
        if(!sustained::obtain(offValue,api_.find,camera,"EVENT_HFA_DISABLED")||!sustained::obtain(highValue,api_.find,camera,"EVENT_HFA_TIER_3_ON")){healthy_=false;return nullptr;}
        int off=static_cast<int>(offValue),high=static_cast<int>(highValue);
        if(off<0||high-off!=9||(off>>16)!=(high>>16))return nullptr;
        Address row=channel(camera,off);
        if(states_.size()>=64&&!states_.count(camera)){
            auto stale=std::find_if(states_.begin(),states_.end(),[](const auto& item){return !item.second.applied;});
            if(stale==states_.end())return nullptr;states_.erase(stale);
        }
        auto& state=states_[camera];
        // A Set callback may precede creation of the channel row. Preserve
        // that intercepted native tier when the worker first sees the row.
        if((state.rows&&state.rows!=row)||state.match!=p.match||state.epoch!=p.epoch)state=State{};
        state.rows=row;state.match=p.match;state.epoch=p.epoch;state.seen=now;state.off=off;state.high=high;
        return &state;
    }
    void inspect(Address camera,const Lease& p,uint64_t now){
        std::lock_guard<std::mutex> lock(mutex_);
        auto remembered=remember(camera,p,now);if(!remembered||!remembered->rows)return;
        auto& state=*remembered;int off=state.off;
        int current=at<int>(state.rows+8);if(current!=off&&!on(current,off))return;
        bool suppress=healthy_&&fresh(p,now)&&p.suppress;
        if(suppress){
            if(current!=off){
                state.saved=current;state.applied=true;
                if(!sustained::invoke(api_.set,camera,off,false)){healthy_=false;return;}
                ++switches_;
            }
            if(state.applied)active_=true;
        }else if(state.applied){
            if(current==off&&(state.saved==off||on(state.saved,off))){
                if(!sustained::invoke(api_.set,camera,state.saved,false)){healthy_=false;return;}
                ++restores_;
            }
            state.applied=false;
        }
    }
public:
    void configure(Address image,const Api* custom=nullptr){
        image_=image;
        if(custom)api_=*custom;
        else{api_.get=reinterpret_cast<Api::Get>(image+bindings::CameraGet);api_.find=reinterpret_cast<Api::Find>(image+bindings::CameraFind);api_.set=reinterpret_cast<Api::Set>(image+bindings::CameraSet);}
    }
    void setOriginal(Api::Set set){api_.set=set;}
    int filter(Address camera,int code,const Lease& p,uint64_t now){
        std::lock_guard<std::mutex> lock(mutex_);
        auto found=states_.find(camera);
        if(found==states_.end()||found->second.match!=p.match||found->second.epoch!=p.epoch||now<found->second.seen||now-found->second.seen>750){
            // Intercept the first HFA event on a new live camera rather than
            // allowing one frame of native motion before the next tick.
            if(!healthy_||!fresh(p,now)||!p.suppress||at<Address>(camera)!=image_+bindings::CameraVtable||!remember(camera,p,now))return code;
            found=states_.find(camera);
        }
        auto& s=found->second;
        if(s.match!=p.match||s.epoch!=p.epoch||now<s.seen||now-s.seen>750||at<Address>(camera)!=image_+bindings::CameraVtable)return code;
        if(code!=s.off&&!on(code,s.off))return code; // Other presentation channels are untouched.
        s.saved=code;
        if(healthy_&&fresh(p,now)&&p.suppress&&on(code,s.off)){s.applied=true;++filtered_;active_=true;return s.off;}
        s.applied=false;return code;
    }
    void tick(Address owner,const Lease& p,uint64_t now){
        active_=false;
        if(!healthy_)return;
        if(!owner||!p.match){std::lock_guard<std::mutex> lock(mutex_);states_.clear();return;}
        // Each Get supplies a fresh owning reference, released on this callback.
        // Never hold a native camera reference across pause, replay or teardown.
        for(int index=1;index<=4;++index){
            Address camera=0,unused=0;
            if(!sustained::obtain(unused,api_.get,owner,&camera,index)){healthy_=false;return;}
            if(!camera)continue;
            // Get returned an owning interface reference even if its concrete
            // camera type is unsupported. Validate its release contract before
            // skipping the view; never inspect another class's HFA rows.
            Address table=at<Address>(camera),known=image_+bindings::CameraVtable;
            auto release=api_.release?api_.release:reinterpret_cast<Api::Release>(at<Address>(table+8));
            bool compatible=api_.release||table==known||(at<Address>(table)==at<Address>(known)&&at<Address>(table+8)==at<Address>(known+8));
            if(table!=known){
                // Broadcast/replay views legitimately have another concrete
                // class. Release compatible interfaces and keep filtering the
                // supported live views instead of disabling it for the game.
                if(!compatible||!release||!sustained::invoke(release,camera)){healthy_=false;return;}
                continue;
            }
            inspect(camera,p,now);
            if(!release||!sustained::invoke(release,camera)){healthy_=false;return;}
        }
    }
    void fail(){healthy_=false;}
    bool active()const{return active_;}bool healthy()const{return healthy_;}
    unsigned switches()const{return switches_;}unsigned restores()const{return restores_;}unsigned filtered()const{return filtered_;}
};
}
