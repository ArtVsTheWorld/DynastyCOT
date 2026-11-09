#pragma once
#include "DirectorShake.h"
#include "OfflineMatchScope.h"
#include "GameBindings.h"
#include "NativeRead.h"
#include <windows.h>
#include <atomic>

namespace attendance::motion {
class PointerHook {
    uintptr_t slot_=0,original_=0,replacement_=0;
    bool exchange(uintptr_t expected,uintptr_t desired){
        DWORD protection=0;if(!VirtualProtect(reinterpret_cast<void*>(slot_),sizeof(void*),PAGE_READWRITE,&protection))return false;
        auto previous=reinterpret_cast<uintptr_t>(InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(slot_),reinterpret_cast<void*>(desired),reinterpret_cast<void*>(expected)));
        DWORD unused=0;bool restored=VirtualProtect(reinterpret_cast<void*>(slot_),sizeof(void*),protection,&unused)!=0;
        if(!restored&&previous==expected){InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(slot_),reinterpret_cast<void*>(expected),reinterpret_cast<void*>(desired));VirtualProtect(reinterpret_cast<void*>(slot_),sizeof(void*),protection,&unused);}
        return previous==expected&&restored;
    }
public:
    bool install(uintptr_t slot,uintptr_t expected,uintptr_t replacement){if(slot_||!slot||slot%sizeof(void*)||!expected||!replacement)return false;slot_=slot;original_=expected;replacement_=replacement;if(exchange(expected,replacement))return true;slot_=0;return false;}
    bool restore(){if(!slot_)return true;bool result=exchange(replacement_,original_);slot_=0;return result;}
};
class Runtime {
    uintptr_t image_=0;
    inline static std::atomic<uint64_t> nextCacheId_{0};
    uint64_t cacheId_=++nextCacheId_;
    std::atomic<bool> healthy_{true};std::atomic<uint64_t> changed_{0},last_{0};std::atomic<float> percent_{0};std::atomic<Kind> kind_{Kind::None};
    std::atomic<uint64_t> appliedBegan_{0};std::atomic<float> zoomScale_{1},unscaled_{0},presetScale_{1};
    std::atomic<uint64_t> requestedBegan_{0},requestedAt_{0};std::atomic<Kind> requestedKind_{Kind::None};
    std::atomic<uint64_t> visitorChanged_{0};std::atomic<float> visitorPercent_{0},visitorPeak_{0};
    std::atomic<float> yawScale_{1},focalY_{0},framingScale_{1},screenPitch_{0},screenYaw_{0};
    std::atomic<uint64_t> cached_{0},invalid_{0},clipped_{0};std::atomic<uintptr_t> view_{0};
    static bool read(uintptr_t p,void* out,size_t size){SIZE_T n=0;return p>=0x10000&&ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(p),out,size,&n)&&n==size;}
    template<class T>static T at(uintptr_t p){return attendance::nativeValue<T>(p,read);}
    bool world(const Cue& c,uintptr_t view)const{
        if(at<uintptr_t>(at<uintptr_t>(image_+bindings::MatchWrapper)+16)!=c.match||at<uintptr_t>(c.match)!=image_+0xb47e908||!attendance::scope::matchMode(at<int>(image_+0xd9eb808))||at<int>(image_+0xf3fddbc)!=3)return false;
        auto configuration=at<uintptr_t>(image_+0xf63f9f0);
        if(!configuration||at<unsigned char>(configuration+0x84)!=0)return false;
        if(at<uintptr_t>(view)!=image_+bindings::CameraVtable)return false;
        auto pause=at<uintptr_t>(image_+bindings::PauseFlag),replay=at<uintptr_t>(image_+bindings::ReplayManager);
        if(!pause||at<unsigned char>(pause)||at<uintptr_t>(replay)!=image_+bindings::ReplayVtable||at<unsigned char>(replay+0x38))return false;
        auto settings=at<uintptr_t>(image_+bindings::CameraSettings);
        if(at<uintptr_t>(settings)!=image_+bindings::CameraSettingsVtable||!at<unsigned short>(settings+0x3e6)||!at<unsigned short>(settings+0x41a))return false;
        // Native CameraGet(context=1) selects this gameplay view directly.
        // Prepare the lineup envelope early, but never apply its movement to
        // the separate presentation views behind the play-selection menu.
        // Earned reactions below still follow all current live render views.
        if(c.kind==Kind::Ordinary||c.kind==Kind::Critical){
            // The snap can replace the slot-1 lineup camera. Its short fade
            // must follow the new live view instead of disappearing at the cut.
            int phase=at<int>(at<uintptr_t>(image_+bindings::GameStates)+0xa0);
            if(c.tail&&phase==3)return true;
            auto owner=at<uintptr_t>(image_+bindings::CameraOwner),views=at<uintptr_t>(at<uintptr_t>(owner)+0x1f0);
            return views&&at<uintptr_t>(views+0x68)==view;
        }
        // Earned effects also follow native player cutaways whose borrowed
        // view is outside the four gameplay slots. The hook's verified camera
        // vtable, live match, pause/replay and cue leases remain mandatory.
        // Each output still receives its own projection compensation.
        return true;
    }
public:
    void configure(uintptr_t image){image_=image;cacheId_=++nextCacheId_;}
    bool prepare(uintptr_t view,const Cue& cue,uintptr_t match,unsigned epoch,uint64_t now){
        if(!healthy_||cue.match!=match||cue.epoch!=epoch||strength(cue,now)<=0||!world(cue,view))return false;
        requestedBegan_=cue.began;requestedKind_=cue.kind;requestedAt_=now;return true;
    }
    void tick(uintptr_t view,float* destination,const Cue& cue,uintptr_t match,unsigned epoch,uint64_t now,bool widePlayerCompensation=false,float presetScale=1){
        if(!std::isfinite(presetScale)||presetScale<=0||presetScale>2.f)return;
        if(!prepare(view,cue,match,epoch,now))return;
        auto address=reinterpret_cast<uintptr_t>(destination);
        if(address<view+0x800&&address+sizeof(float)*16>view)return;
        std::array<float,16> p{},output{};
        if(!read(view+0x2b4,p.data(),sizeof(p)))p={};
        static thread_local projection::History history;
        auto optical=history.observe(cacheId_,view,match,epoch,now,p,widePlayerCompensation);
        if(!optical.valid){++invalid_;return;}
        float level=strength(cue,now),preference=presetScale*projection::ReferenceStrength*optical.framing;
        if(!read(address,output.data(),sizeof(output))||!apply(output,cue,now,optical.pitch,optical.yaw,preference))return;
        SIZE_T n=0;if(!WriteProcessMemory(GetCurrentProcess(),destination,output.data(),sizeof(output),&n)||n!=sizeof(output))return;
        ++changed_;appliedBegan_=cue.began;unscaled_=level;zoomScale_=optical.pitch;presetScale_=presetScale;percent_=level*preference*optical.pitch;kind_=cue.kind;last_=now;
        yawScale_=optical.yaw;focalY_=optical.fy;framingScale_=optical.framing;view_=view;
        // Peak small-angle displacement estimates in percent of frame height /
        // width. They describe this output, not proof it was finally rendered.
        screenPitch_=.5f*optical.fy*.65f*.01745329252f*level*preference*optical.pitch;
        screenYaw_=.5f*optical.fx*.45f*.01745329252f*level*preference*optical.yaw;
        if(optical.cached)++cached_;if(optical.clipped)++clipped_;
        if(cue.visitor){
            ++visitorChanged_;float applied=level*preference*optical.pitch;visitorPercent_=applied;
            float peak=visitorPeak_.load();while(peak<applied&&!visitorPeak_.compare_exchange_weak(peak,applied)){}
        }
    }
    bool healthy()const{return healthy_;}void fail(){healthy_=false;}
    bool playing(uint64_t now)const{auto last=last_.load();return healthy_&&last&&now>=last&&now-last<150;}
    bool playing(const Cue& cue,uint64_t now)const{return playing(now)&&appliedBegan_==cue.began&&kind_==cue.kind;}
    bool requested(const Cue& cue,uint64_t now)const{auto at=requestedAt_.load();return at&&now>=at&&now-at<150&&requestedBegan_==cue.began&&requestedKind_==cue.kind;}
    float zoomScale()const{return zoomScale_;}float unscaled()const{return unscaled_;}
    float presetScale()const{return presetScale_;}
    float framingScale()const{return framingScale_;}
    float yawScale()const{return yawScale_;}float fov()const{float f=focalY_;return f>0?2.f*std::atan(1.f/f)*57.295779513f:0;}
    float screenPitch()const{return screenPitch_;}float screenYaw()const{return screenYaw_;}
    uintptr_t view()const{return view_;}uint64_t cached()const{return cached_;}uint64_t invalid()const{return invalid_;}uint64_t clipped()const{return clipped_;}
    Kind kind()const{return kind_;}float percent()const{return percent_;}uint64_t changed()const{return changed_;}
    uint64_t visitorChanged()const{return visitorChanged_;}float visitorPercent()const{return visitorPercent_;}float visitorPeak()const{return visitorPeak_;}
};
}
