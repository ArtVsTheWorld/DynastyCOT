#pragma once
#include "SwellAudioRuntime.h"
#include "CrowdPopulation.h"
#include <array>

namespace attendance::cheer {
using Address=uintptr_t;
using Receiver=Address(__fastcall*)(Address,Address);
enum class Source {Live,Lineup,Result,Eruption,Followup};
enum class Delivery {Sent,Failed,Expired,Registered,Recovery,Unregistered,NativePriority,PrioritySuppressed,Cancelled,ObservationUnavailable,PopulationSuppressed};
using Note=void(*)(Source,uint64_t,Delivery);
// Eruption, conversion and defensive-stop families passed listening tests. Their native
// owner limits each reaction to 25 seconds; no borrowed entity is retained.
inline bool emit(const sustained::Api& api,Receiver receiver,Address image,Address entity,int type=91){
    using namespace sustained;
    if(!population::reactionFamily(type))return false;
    Address asset=at<Address>(entity+0x38),system=at<Address>(asset+0x20)&~Address(4);
    Address list=at<Address>(system+0x20),description=0;
    unsigned count=at<unsigned>(list-4)&0x7fffffff;
    if(at<unsigned>(system+0x38)!=205964879||!list||count>128)return false;
    for(unsigned i=0;i<count;++i){
        Address d=at<Address>(list+i*8)&~Address(4);
        if(at<unsigned>(d+0x40)==205964879&&at<unsigned>(d+0x44)==3424066396u&&named(at<Address>(d+0x38),"Trigger_AreaReaction")){description=d;break;}
    }
    if(!description)return false;
#pragma warning(push)
#pragma warning(disable:4324)
    struct Owned {
        const Api& api;alignas(32) unsigned char arena[0x48]{},event[0xb0]{};bool ar=false,ev=false;
        explicit Owned(const Api& a):api(a){}
        ~Owned(){if(ev)invoke(api.eventDestroy,reinterpret_cast<Address>(event));if(ar)invoke(api.arenaDestroy,reinterpret_cast<Address>(arena));}
    } owned(api);
#pragma warning(pop)
    *reinterpret_cast<Address*>(owned.arena)=image+bindings::SwellArenaVtable;
    *reinterpret_cast<unsigned*>(owned.arena+8)=1;*reinterpret_cast<unsigned*>(owned.arena+12)=1;
    Address unused=0;
    if(!obtain(unused,api.arenaInit,reinterpret_cast<Address>(owned.arena+0x18)))return false;owned.ar=true;
    if(!obtain(unused,api.construct,reinterpret_cast<Address>(owned.event),reinterpret_cast<Address>(owned.arena),description))return false;owned.ev=true;
    Address event=reinterpret_cast<Address>(owned.event);
    if(at<unsigned char>(event+0xac)!=1||at<unsigned>(event+0x18)!=1)return false;
    Address value=at<Address>(at<Address>(event+0x20)),d=at<Address>(value+0x30);
    if(at<int>(d+0x40)!=1||at<unsigned char>(d+0x44)||!named(at<Address>(d+0x20),"reacType")||!invoke(api.set,value,type))return false;
    return obtain(unused,receiver,entity+0x40,event);
}
class Runtime {
    Address image_=0,match_=0;unsigned epoch_=0;uint64_t consumed_=0,result_=0;
    uint64_t lineup_=0;
    sustained::Api api_;Receiver receiver_=nullptr;std::mutex mutex_;
    std::atomic<bool> healthy_{true};std::atomic<unsigned> sent_{0};
    uint64_t liveConsumed_=0;std::atomic<uint64_t> liveDelivered_{0},liveResolved_{0};std::array<std::atomic<uint64_t>,32> liveOutcomes_{};Note note_=nullptr;
    std::atomic<Address> focusMatch_{0};std::atomic<unsigned> focusEpoch_{0};
    std::atomic<uint64_t> focusSequence_{0},focusBegan_{0},focusUntil_{0};
    void focus(const sustained::Intent& w,uint64_t now){
        if(!w.playSequence)return;
        focusMatch_=w.match;focusEpoch_=w.epoch;focusSequence_=w.playSequence;focusBegan_=now;focusUntil_=now+(w.spring?1500:700);
    }
    std::atomic<Address> audibleMatch_{0};std::atomic<unsigned> audibleEpoch_{0};
    std::atomic<uint64_t> audibleUpdated_{0},audibleSequence_{0};std::atomic<bool> audibleVoice_{false};
    struct NativeVoice {int handle=0,family=0;uint64_t first=0,sequence=0;};
    std::array<NativeVoice,256> nativeVoices_{};unsigned nativeCount_=0;
    Address nativeOwner_=0,nativeMatch_=0;unsigned nativeEpoch_=0;uint64_t nativeSample_=0;
    bool nativeSampled_=false,freshEquivalent_=false;
    struct VoiceEntry {int type=0,handle=0;uint32_t reserved=0;uint8_t stopped=0,padding[3]{};};
    static_assert(sizeof(VoiceEntry)==16);
    struct VoiceSnapshot {bool valid=false;unsigned count=0;std::array<VoiceEntry,256> entries{};};
    VoiceSnapshot currentVoices_;
    static VoiceSnapshot snapshot(Address entity){
        VoiceSnapshot out;std::array<Address,2> edges{};SIZE_T bytes=0;
        if(entity<0x10000||!ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(entity+0x120),edges.data(),sizeof(edges),&bytes)||bytes!=sizeof(edges))return out;
        const auto begin=edges[0],end=edges[1];if(end<begin||end-begin>256*16||(end-begin)%16)return out;
        out.count=unsigned((end-begin)/16);
        if(out.count&&(!ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(begin),out.entries.data(),out.count*16,&bytes)||bytes!=out.count*16))return out;
        out.valid=true;return out;
    }

    void sampleAudible(Address entity,const sustained::Intent& wanted,uint64_t now){
        audibleVoice_=false;freshEquivalent_=false;currentVoices_.valid=false;
        if(!sustained::fresh(wanted,wanted.match,wanted.epoch,now)||
           sustained::at<Address>(entity)!=image_+bindings::ReactionVtable)return;
        currentVoices_=snapshot(entity);if(!currentVoices_.valid)return;
        const bool continuity=nativeSampled_&&nativeOwner_==entity&&nativeMatch_==wanted.match&&nativeEpoch_==wanted.epoch&&
            now>=nativeSample_&&now-nativeSample_<=600;
        std::array<NativeVoice,256> observed{};unsigned count=0;bool playing=false;
        for(unsigned index=0;index<currentVoices_.count;++index)if(const auto& entry=currentVoices_.entries[index];!entry.stopped&&population::reactionFamily(entry.type)){
            playing=true;
            if(wanted.spring){
                NativeVoice v{entry.handle,entry.type/4,0,0};
                bool prior=false;
                if(continuity)for(unsigned i=0;i<nativeCount_;++i)if(nativeVoices_[i].handle==v.handle&&nativeVoices_[i].family==v.family){v=nativeVoices_[i];prior=true;break;}
                // An already present/unknown-age voice is not proof of a
                // reaction to this play. Only a newly observed positive native
                // handle in a continuous sample can suppress its equivalent.
                if(continuity&&!prior&&v.handle>0){v.first=now;v.sequence=wanted.playSequence;}
                // Generic YEAH/AWW/BOO registration is not an exact-event
                // duplicate. Only event-specific native families qualify.
                if(v.first&&v.sequence==wanted.playSequence&&now>=v.first&&now-v.first<=600&&v.family<22&&v.family==wanted.resultType/4)freshEquivalent_=true;
                observed[count++]=v;
            }
            if(!wanted.spring)break; // retain the ordinary audio callback's fast path
        }
        if(wanted.spring){nativeVoices_=observed;nativeCount_=count;nativeOwner_=entity;nativeMatch_=wanted.match;nativeEpoch_=wanted.epoch;nativeSample_=now;nativeSampled_=true;}
        audibleMatch_=wanted.match;audibleEpoch_=wanted.epoch;audibleSequence_=wanted.playSequence;
        audibleUpdated_=now;audibleVoice_=playing;
    }
    struct Followup {bool active=false;Source source=Source::Result;uint64_t id=0,sequence=0,due=0,until=0;int type=75;};
    Followup followup_;std::array<Followup,8> waiting_{};
    void clearFollowups(){followup_={};waiting_={};}
    void advanceFollowup(){followup_=waiting_[0];for(unsigned i=1;i<waiting_.size();++i)waiting_[i-1]=waiting_[i];waiting_.back()={};}
    struct Receipt {bool active=false,retried=false;Address owner=0;Source source=Source::Live;uint64_t id=0,sequence=0,began=0,until=0;int type=0;unsigned observations=0;std::array<int,256> priorHandles{};unsigned priorCount=0;bool spring=false;};
    std::array<Receipt,8> receipts_{};
    std::array<std::pair<int,int>,32> claimedVoices_{};unsigned claimedCount_=0;uint64_t receiptSequence_=0;
    void cancelReceipts(Delivery why){for(auto& r:receipts_)if(r.active){auto old=r;r={};report(old.source,old.id,why);}}
    bool receiptPending()const{for(const auto& r:receipts_)if(r.active&&(r.source==Source::Result||r.source==Source::Eruption))return true;return false;}
    std::atomic<bool> priorityPending_{false},observing_{false};
    std::atomic<uint64_t> populationDenied_{0}; // Scoped request ID; survives first tick's owner initialization.
    void receipt(Source source,uint64_t id,int type,Address entity,const sustained::Intent& wanted,uint64_t now){
        if(!wanted.playSequence||source==Source::Lineup)return;
        bool scoring=source==Source::Eruption||(source==Source::Result&&wanted.scoringReaction);
        Receipt* slot=nullptr;
        for(auto& r:receipts_)if(!r.active){slot=&r;break;}
        if(!slot){report(source,id,Delivery::ObservationUnavailable);return;}
        auto& r=*slot;r={true,false,entity,source,id,wanted.playSequence,now,now+(scoring?1200:600),type,0};r.spring=wanted.spring;
        const auto voices=snapshot(entity);
        if(voices.valid)for(unsigned i=0;i<voices.count;++i){const auto& v=voices.entries[i];if(!v.stopped&&v.type==type)r.priorHandles[r.priorCount++]=v.handle;}

    }
    void checkReceipt(Address entity,const sustained::Intent& wanted,uint64_t now){
        using namespace sustained;
        for(auto& r:receipts_)if(r.active){
            if(r.sequence!=wanted.playSequence||r.owner!=entity||now<r.began){auto old=r;r={};report(old.source,old.id,Delivery::Cancelled);continue;}
            if(!currentVoices_.valid){auto old=r;r={};report(old.source,old.id,Delivery::ObservationUnavailable);continue;}
            bool registered=false;int matchedHandle=0;
            for(unsigned j=0;j<currentVoices_.count;++j){const auto& voice=currentVoices_.entries[j];if(voice.stopped||voice.type!=r.type||voice.handle<=0)continue;
                bool existing=false;for(unsigned i=0;i<r.priorCount;++i)existing=existing||r.priorHandles[i]==voice.handle;for(unsigned i=0;i<claimedCount_;++i)existing=existing||(claimedVoices_[i].first==voice.type&&claimedVoices_[i].second==voice.handle);if(!existing&&claimedCount_<claimedVoices_.size()){registered=true;matchedHandle=voice.handle;break;}}
            ++r.observations;
            if(registered)claimedVoices_[claimedCount_++]={r.type,matchedHandle};
            if(registered||now>r.until){auto old=r;r={};report(old.source,old.id,registered?Delivery::Registered:Delivery::Unregistered);}
            // An inconclusive receipt never replays a sound. The native queue
            // owns its own attempts; each GDA event has one dispatch budget.
        }
    }

    void arm(Source source,uint64_t id,int type,const sustained::Intent& wanted,uint64_t now){
        if(!wanted.staggered||!wanted.playSequence||population::negative(type))return;
        Followup next{true,source,id,wanted.playSequence,now+900,now+1800,wanted.deepCheers?population::reactionType(75,wanted.populationSize):type};
        if(!followup_.active){followup_=next;return;}
        for(auto& waiting:waiting_)if(!waiting.active){waiting=next;return;}
        report(Source::Followup,id,Delivery::Expired);
    }
    bool follow(Address entity,const sustained::Intent& wanted,uint64_t now){
        if(!followup_.active)return false;
        bool same=followup_.sequence==wanted.playSequence&&wanted.staggered&&
            (followup_.source!=Source::Eruption||wanted.celebration==followup_.id);
        if(!same||now>=followup_.until||now+900<followup_.due){advanceFollowup();return follow(entity,wanted,now);}
        if(now<followup_.due)return false;
        if(sustained::at<Address>(entity)!=image_+bindings::ReactionVtable)return false;
        Address asset=sustained::at<Address>(entity+0x38);
        if(sustained::at<int>(asset+0x40)!=3||sustained::at<float>(asset+0x44)!=25)return false;
        Followup next=followup_;advanceFollowup(); // One bounded attempt, consumed before native dispatch.
        Address begin=sustained::at<Address>(entity+0x120),end=sustained::at<Address>(entity+0x128);
        if(end<begin||end-begin>256*16||(end-begin)%16){healthy_=false;report(Source::Followup,next.id,Delivery::Failed);return true;}
        // Every intended layer gets one dispatch even while native voices play.
        receipt(Source::Followup,next.id,next.type,entity,wanted,now);
        if(emit(api_,receiver_,image_,entity,next.type)){++sent_;report(Source::Followup,next.id,Delivery::Sent);}
        else{healthy_=false;report(Source::Followup,next.id,Delivery::Failed);}
        return true;
    }
    void report(Source source,uint64_t id,Delivery delivery){
        if(source==Source::Live){liveResolved_=id;liveOutcomes_[id%liveOutcomes_.size()]=id;}
        if(note_)note_(source,id,delivery);
    }
public:
    void configure(Address image,const sustained::Api* api=nullptr,Receiver receiver=nullptr,Note note=nullptr){
        image_=image;api_=api?*api:sustained::Api::native(image);
        receiver_=receiver?receiver:reinterpret_cast<Receiver>(image+bindings::ReactionReceive);note_=note;
    }
    void tick(Address entity,const sustained::Intent& input,uint64_t now){
        auto wanted=input;
        if(wanted.supporterSize>0)wanted.populationSize=(std::min)(wanted.populationSize,wanted.supporterSize);
        wanted.liveType=population::reactionType(wanted.liveType,wanted.populationSize);
        wanted.resultType=population::reactionType(wanted.resultType,wanted.populationSize);
        wanted.lineupType=population::reactionType(wanted.lineupType,wanted.populationSize);
        using namespace sustained;
        static thread_local bool executing=false;if(executing)return;
        struct Guard {bool& value;explicit Guard(bool& v):value(v){value=true;}~Guard(){value=false;}} guard(executing);
        std::lock_guard<std::mutex> lock(mutex_);
        struct PriorityGuard {Runtime& runtime;~PriorityGuard(){runtime.priorityPending_=runtime.receiptPending();bool active=false;for(const auto& r:runtime.receipts_)active=active||r.active;runtime.observing_=active;}} pending{*this};
        if(!healthy_||!fresh(wanted,wanted.match,wanted.epoch,now)){audibleVoice_=false;nativeSampled_=false;clearFollowups();cancelReceipts(Delivery::Cancelled);return;}
        if(receiptSequence_!=wanted.playSequence||match_!=wanted.match||epoch_!=wanted.epoch){claimedCount_=0;receiptSequence_=wanted.playSequence;}
        sampleAudible(entity,wanted,now);
        if(match_!=wanted.match||epoch_!=wanted.epoch){match_=wanted.match;epoch_=wanted.epoch;consumed_=result_=lineup_=liveConsumed_=0;liveDelivered_=liveResolved_=0;clearFollowups();cancelReceipts(Delivery::Cancelled);for(auto& outcome:liveOutcomes_)outcome=0;}
        bool victory=wanted.celebration&&now<wanted.celebrationUntil&&consumed_!=wanted.celebration;
        bool priorityResult=!wanted.celebration&&wanted.scoringReaction&&wanted.resultCheer&&now<wanted.resultCheerUntil;
        if(victory||(priorityResult&&result_!=wanted.resultCheer))clearFollowups();
        checkReceipt(entity,wanted,now);if(!healthy_)return;
        if(wanted.liveCheer&&liveConsumed_!=wanted.liveCheer&&now>=wanted.liveCheerUntil){liveConsumed_=wanted.liveCheer;report(Source::Live,wanted.liveCheer,Delivery::Expired);}
        if(wanted.resultCheer&&result_!=wanted.resultCheer&&now>=wanted.resultCheerUntil){result_=wanted.resultCheer;report(Source::Result,wanted.resultCheer,Delivery::Expired);}
        if(wanted.celebration&&consumed_!=wanted.celebration&&now>=wanted.celebrationUntil){consumed_=wanted.celebration;report(Source::Eruption,wanted.celebration,Delivery::Expired);}
        if(wanted.liveCheer&&populationDenied_.load()==wanted.liveCheer)liveConsumed_=wanted.liveCheer;
        bool during=wanted.liveCheer&&now<wanted.liveCheerUntil&&liveConsumed_!=wanted.liveCheer;
        // A late live-run accent must not take the receiver/receipt slot from
        // a fresh scoring cheer. Leave its ID unconsumed; it expires normally.
        bool scoring=priorityResult||(wanted.celebration&&now<wanted.celebrationUntil);
        if(during&&scoring){liveConsumed_=wanted.liveCheer;report(Source::Live,wanted.liveCheer,Delivery::PrioritySuppressed);}
        if(during&&!scoring){
            if(!population::reactionFamily(wanted.liveType))return;
            if(at<Address>(entity)!=image_+bindings::ReactionVtable)return;
            Address asset=at<Address>(entity+0x38);if(at<int>(asset+0x40)!=3||at<float>(asset+0x44)!=25)return;
            Address begin=at<Address>(entity+0x120),end=at<Address>(entity+0x128);
            if(end<begin||end-begin>256*16||(end-begin)%16){healthy_=false;return;}
            liveConsumed_=wanted.liveCheer;
            receipt(Source::Live,wanted.liveCheer,wanted.liveType,entity,wanted,now);
            if(emit(api_,receiver_,image_,entity,wanted.liveType)){++sent_;focus(wanted,now);liveDelivered_=wanted.liveCheer;report(Source::Live,wanted.liveCheer,Delivery::Sent);if(wanted.majorLive)arm(Source::Live,wanted.liveCheer,wanted.liveType,wanted,now);}
            else{cancelReceipts(Delivery::Cancelled);healthy_=false;report(Source::Live,wanted.liveCheer,Delivery::Failed);}
            if(!healthy_)return;
        }
        bool result=!wanted.celebration&&wanted.resultCheer&&now<wanted.resultCheerUntil&&result_!=wanted.resultCheer;
        bool lineup=false;
        if(!victory&&!result&&follow(entity,wanted,now))return;
        if(!victory&&!result){
            if(!wanted.cheers||!wanted.lineupCheer||wanted.lineupCheer==lineup_||now<wanted.lineupCheerBegan)return;
            uint64_t age=now-wanted.lineupCheerBegan;
            // One early opportunity, never a delayed cheer after waiting at the
            // line. An expired opportunity is never backfilled.
            if(now>=wanted.lineupCheerUntil||age>=1500){lineup_=wanted.lineupCheer;report(Source::Lineup,lineup_,Delivery::Expired);return;}
            lineup=true;
        }
        if(at<Address>(entity)!=image_+bindings::ReactionVtable)return;
        Address asset=at<Address>(entity+0x38);
        if(at<int>(asset+0x40)!=3||at<float>(asset+0x44)!=25)return;
        // Consume before dispatch, including uncertain native failures. A score
        // may cross several entity updates/scenes but receives one bounded burst.
        if(victory)consumed_=wanted.celebration;
        else if(result){result_=wanted.resultCheer;}
        else{lineup_=wanted.lineupCheer;}
        if(wanted.spring&&result&&freshEquivalent_){clearFollowups();report(Source::Result,wanted.resultCheer,Delivery::NativePriority);return;}
        unsigned sentNow=0;
        auto send=[&](int type){
            type=population::reactionType(type,wanted.populationSize);
            // Re-read after each send: the native instance array may reallocate.
            Address begin=at<Address>(entity+0x120),end=at<Address>(entity+0x128);
            if(end<begin||end-begin>256*16||(end-begin)%16){healthy_=false;return;}
            // Observe the primary, including rushing-TD selectors. A secondary
            // layer must not overwrite its receipt merely because families match.
            receipt(victory?Source::Eruption:result?Source::Result:Source::Lineup,victory?wanted.celebration:result?wanted.resultCheer:wanted.lineupCheer,type,entity,wanted,now);
            if(emit(api_,receiver_,image_,entity,type)){++sent_;++sentNow;if(!lineup&&sentNow==1)focus(wanted,now);}else{cancelReceipts(Delivery::Cancelled);healthy_=false;}
        };
        send(result?wanted.resultType:lineup?wanted.lineupType:wanted.celebrationType);
        // Keep the immediate primary; the optional second layer has one
        // bounded delayed opportunity, never a repeated cheer barrage.
        bool stagger=wanted.staggered&&(victory||(result&&wanted.deepCheers));
        if((victory||lineup||result)&&wanted.deepCheers&&!stagger&&healthy_)send(75);
        if(stagger&&healthy_&&sentNow)arm(victory?Source::Eruption:Source::Result,victory?wanted.celebration:wanted.resultCheer,result?wanted.resultType:wanted.celebrationType,wanted,now);
        report(victory?Source::Eruption:result?Source::Result:Source::Lineup,victory?wanted.celebration:result?wanted.resultCheer:wanted.lineupCheer,
            !healthy_?Delivery::Failed:sentNow?Delivery::Sent:Delivery::Failed);
    }
    void fail(){healthy_=false;}
    bool healthy()const{return healthy_;}unsigned sent()const{return sent_;}
    bool observing()const{return observing_.load(std::memory_order_relaxed);}
    void suppress(uint64_t id){if(id&&liveResolved_.exchange(id)!=id)report(Source::Live,id,Delivery::PrioritySuppressed);}
    void suppressPopulation(uint64_t id){if(id&&populationDenied_.exchange(id)!=id)report(Source::Live,id,Delivery::PopulationSuppressed);}
    bool priorityPending()const{return priorityPending_.load();}
    bool eventFocus(Address match,unsigned epoch,uint64_t sequence,uint64_t now)const{
        return healthy_&&match&&sequence&&focusMatch_.load()==match&&focusEpoch_.load()==epoch&&focusSequence_.load()==sequence&&now>=focusBegan_.load()&&now<focusUntil_.load();
    }
    bool liveDelivered(uint64_t id)const{return id&&liveDelivered_.load()==id;}
    bool audible(Address match,unsigned epoch,uint64_t sequence,uint64_t now)const{
        auto updated=audibleUpdated_.load();
        return audibleVoice_.load()&&match&&audibleMatch_.load()==match&&audibleEpoch_.load()==epoch&&
            audibleSequence_.load()==sequence&&updated&&now>=updated&&now-updated<=150;
    }
    bool liveResolved(uint64_t id)const{return id&&liveOutcomes_[id%liveOutcomes_.size()].load()==id;}
};
}
