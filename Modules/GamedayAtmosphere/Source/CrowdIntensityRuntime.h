#pragma once
#include <map>
#include "SwellAudioRuntime.h"

namespace attendance::sustained {
class IntensityRuntime {
public:
    using Lock=void(__fastcall*)(Address);
    using Publish=void(__fastcall*)(Address,const float*,bool);
private:
    struct State {Address asset=0,publisher=0;bool applied=false;float weight=0;uint64_t last=0;Address match=0;unsigned epoch=0;};
    Address image_=0;Lock lock_=nullptr,unlock_=nullptr;Publish publish_=nullptr;
    std::map<Address,State> states_;std::mutex mutex_;
    std::atomic<float> native_{0},output_{0};std::atomic<unsigned> restores_{0};std::atomic<bool> healthy_{true};
public:
    void configure(Address image,Lock lock=nullptr,Lock unlock=nullptr,Publish publish=nullptr){
        image_=image;lock_=lock?lock:reinterpret_cast<Lock>(image+bindings::IntensityLock);
        unlock_=unlock?unlock:reinterpret_cast<Lock>(image+bindings::IntensityUnlock);
        publish_=publish?publish:reinterpret_cast<Publish>(image+bindings::IntensityPublish);
    }
    void tick(Address entity,const Intent& wanted,Address match,unsigned epoch,uint64_t now){
        static thread_local bool executing=false;if(executing)return;
        struct Guard {bool& flag;explicit Guard(bool& value):flag(value){flag=true;}~Guard(){flag=false;}} executingGuard(executing);
        // Only the native update's borrowed entity is touched. Stored addresses
        // are identity keys, never worker-thread objects or owning references.
        if(at<Address>(entity)!=image_+bindings::IntensityVtable)return;
        std::lock_guard<std::mutex> guard(mutex_);
        if(!invoke(lock_,entity+0x90)){healthy_=false;return;}
        struct Unlock {Lock fn;Address address;~Unlock(){invoke(fn,address);}} release{unlock_,entity+0x90};
        Address asset=at<Address>(entity+0x38),publisher=at<Address>(entity+0x80);
        float native=at<float>(entity+0xc4);
        // Native decay can overshoot its endpoint by a fraction (observed
        // 800.107). Preserve it rather than skipping a pending restoration.
        if(!asset||!publisher||!std::isfinite(native)||native<0||native>1001||at<float>(asset+0x3c)!=0||at<float>(asset+0x40)!=1000)return;
        auto found=states_.find(entity);
        if(found==states_.end()){
            if(states_.size()>=64){
                // Never discard a live override just to admit another entity.
                auto unused=std::find_if(states_.begin(),states_.end(),[](const auto& entry){return !entry.second.applied;});
                if(unused==states_.end())return;states_.erase(unused);
            }
            found=states_.emplace(entity,State{}).first;
        }
        State& state=found->second;
        if(state.asset!=asset||state.publisher!=publisher||state.match!=match||state.epoch!=epoch){
            bool sameBinding=state.asset==asset&&state.publisher==publisher;bool applied=state.applied;
            state=State{};state.applied=sameBinding&&applied;state.asset=asset;state.publisher=publisher;state.last=now;state.match=match;state.epoch=epoch;
        }
        bool active=healthy_&&fresh(wanted,match,epoch,now)&&wanted.intensity>0;
        float desired=active?(std::clamp)(wanted.intensity,0.0f,1000.0f):0;
        float dt=state.last&&now>=state.last?static_cast<float>((std::min)(now-state.last,250ull))*0.001f:0;
        state.last=now;
        // Smooth raises. A lost lease or departed gameplay context restores the
        // current native output immediately rather than keeping a stale lift.
        float attack=wanted.liveCheer?.05f:wanted.reason==Reason::PassFlight?.15f:wanted.reason==Reason::Result||wanted.reason==Reason::Eruption||lineupReason(wanted.reason)?.25f:2.0f;
        state.weight=active?(std::min)(1.0f,state.weight+dt/attack):0;
        // Density is mixed on native buses. This path only raises earned
        // reactions; it never suppresses the native excitement envelope.
        float output=native+((std::max)(native,desired)-native)*state.weight;
        bool override=std::fabs(output-native)>0.01f;
        if(!override)output=native; // Finish restoration at the exact native value.
        if(override||state.applied){
            bool wasApplied=state.applied;state.applied=true; // A fault may occur after the engine consumed the value.
            if(!invoke(publish_,entity+0x80,&output,true)){healthy_=false;return;}
            if(!override&&wasApplied)++restores_;
            state.applied=override;
        }
        native_=native;output_=output;
    }
    float native()const{return native_;}float output()const{return output_;}
    unsigned restores()const{return restores_;}bool healthy()const{return healthy_;}
    void fail(){healthy_=false;}
};
}
