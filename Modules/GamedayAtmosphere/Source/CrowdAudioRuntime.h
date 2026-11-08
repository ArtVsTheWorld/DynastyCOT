#pragma once
#include <windows.h>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include "CrowdAudioModel.h"
#include "AudioIntent.h"
#include "GameBindings.h"

namespace attendance::audio {
using Address=uintptr_t;
struct alignas(16) Parameter {uint32_t id,padding[3];float value;uint32_t tail[3];};
static_assert(sizeof(Parameter)==32);
using Update=Address(__fastcall*)(Address,Address,Address,Address);
using Post=void(__fastcall*)(Address,const Parameter*);
using Request=Wanted(*)();
using Note=void(*)(const char*);
using Clock=unsigned long long(*)();

// MixerBridge's native update borrows its +0x40 interface for this call.
// No mixer address is dereferenced outside that owning native callback.
class Runtime {
    struct Entry {
        Gain gain;Address match=0;unsigned epoch=0;unsigned long long tick=0,posted=0;
        int volume=-1;float output=-1;bool modified=false;
    };
    Address image_=0;Update original_=nullptr;Post post_=nullptr;Request request_=nullptr;Note note_=nullptr;Clock clock_=nullptr;
    std::mutex mutex_;std::unordered_map<Address,Entry> entries_;
    bool legacyCuts_=false; // Production is bounded to 25%; legacy behavior is research-only.
    std::atomic<bool> healthy_{true};
    std::atomic<unsigned long long> callbacks_{0},writes_{0};
    std::atomic<float> shownGain_{1},shownTarget_{1};
    static unsigned long long now(){return GetTickCount64();}
    template<class T>static bool read(Address p,T& value){
        SIZE_T n=0;return p>=0x10000&&ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(p),&value,sizeof(value),&n)&&n==sizeof(value);
    }
    bool owner(Address tickThis,Address& primary,int& volume){
        if(tickThis<0x10040)return false;
        primary=tickThis-0x40;Address a=0,b=0,c=0;
        return read(primary,a)&&a==image_+0xb227098&&read(tickThis,b)&&b==image_+0xb227178&&
            read(primary+0x48,c)&&c==image_+0xb227188&&read(primary+0x1f4,volume)&&volume>=0&&volume<=100;
    }
    bool guardedPost(Address primary,float value){
        Parameter parameter{0x9f7d7b40,{0,0,0},value,{0,0,0}};
        __try {post_(primary+0x140,&parameter);return true;}
        __except(EXCEPTION_EXECUTE_HANDLER){return false;}
    }
    void update(Address tickThis){
        ++callbacks_;
        if(!healthy_)return;
        Address primary=0;int volume=0;if(!owner(tickThis,primary,volume))return;
        Wanted wanted=request_();
        if(!legacyCuts_)wanted.strength=(std::clamp)(wanted.strength,0.f,.25f);
        if(!std::isfinite(wanted.remaining)||!std::isfinite(wanted.strength)||!std::isfinite(wanted.emphasis)||!std::isfinite(wanted.arrivalFraction)||!std::isfinite(wanted.reductionStart))wanted.active=false;
        const auto time=clock_();
        std::lock_guard<std::mutex> lock(mutex_);
        auto found=entries_.find(primary);
        if(found==entries_.end()){
            // Keys distinguish histories only. Their addresses are never used
            // to call or read a controller on a later callback.
            if(entries_.size()>=32)entries_.clear();
            Entry e;e.match=wanted.match;e.epoch=wanted.epoch;e.tick=time;
            if(wanted.active){if(legacyCuts_)e.gain.attendance(wanted.remaining,wanted.strength,wanted.emphasis,wanted.arrivalFraction,wanted.reductionStart);else e.gain.sparse((std::min)(wanted.remaining,wanted.arrivalFraction),wanted.strength,wanted.reductionStart);e.gain.advance(6);}
            found=entries_.emplace(primary,e).first;
            if(note_)note_("audio-controller: native mixer update observed");
        }
        Entry& e=found->second;
        if(e.epoch!=wanted.epoch||e.match!=wanted.match){
            e.gain.reset();e.epoch=wanted.epoch;e.match=wanted.match;
        }
        if(wanted.active){
            if(legacyCuts_)e.gain.attendance(wanted.remaining,wanted.strength,wanted.emphasis,wanted.arrivalFraction,wanted.reductionStart);
            else e.gain.sparse((std::min)(wanted.remaining,wanted.arrivalFraction),wanted.strength,wanted.reductionStart);
        }else if(legacyCuts_)e.gain.restore();else e.gain.reset();
        float dt=static_cast<float>(time-e.tick)*0.001f;e.tick=time;
        e.gain.advance((std::min)(dt,0.1f));
        const float gain=e.gain.value(),output=e.gain.linear(volume*0.01f);
        shownGain_=gain;shownTarget_=e.gain.target();
        // A full crowd adds no queue traffic. An attenuated crowd is refreshed
        // at most 20 times/second and always derives from the unscaled slider.
        bool needed=gain<1||e.modified;
        bool due=time-e.posted>=50||volume!=e.volume;
        if(!needed||!due)return;
        if(guardedPost(primary,output)){
            ++writes_;e.output=output;e.volume=volume;e.posted=time;e.modified=gain<1;
        }else {
            healthy_=false;if(note_)note_("audio-fault: native queue update failed; attenuation disabled");
        }
    }
public:
    void configure(Address image,Update original,Request request,Note note=nullptr,Post post=nullptr,Clock clock=nullptr,bool legacyCuts=false){
        image_=image;original_=original;request_=request;note_=note;legacyCuts_=legacyCuts;
        post_=post?post:reinterpret_cast<Post>(image+bindings::CrowdQueue);clock_=clock?clock:now;
    }
    Address tick(Address tickThis,Address a,Address b,Address c){
        // Forward every integer argument and the original return value. Native
        // processing always runs, even if our optional audio stage is disabled.
        Address result=original_(tickThis,a,b,c);
        try {update(tickThis);}catch(...){healthy_=false;}
        return result;
    }
    unsigned long long callbacks() const{return callbacks_.load();}
    unsigned long long writes() const{return writes_.load();}
    float gain() const{return shownGain_.load();}
    float target() const{return shownTarget_.load();}
    bool healthy() const{return healthy_.load();}
};
}
