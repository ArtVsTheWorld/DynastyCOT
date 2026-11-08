#pragma once
#include "SwellAudioRuntime.h"
#include "CrowdPopulation.h"
#include <array>

namespace attendance::cheer {
using Address=uintptr_t;
using Receiver=Address(__fastcall*)(Address,Address);
enum class Source {Live,Lineup,Result,Eruption,Followup};
enum class Delivery {Sent,Failed,Expired,Registered,Recovery,Unregistered};
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
    uint64_t liveConsumed_=0;std::atomic<uint64_t> liveDelivered_{0},liveResolved_{0};Note note_=nullptr;
    std::atomic<Address> audibleMatch_{0};std::atomic<unsigned> audibleEpoch_{0};
    std::atomic<uint64_t> audibleUpdated_{0},audibleSequence_{0};std::atomic<bool> audibleVoice_{false};
    void sampleAudible(Address entity,const sustained::Intent& wanted,uint64_t now){
        audibleVoice_=false;
        if(!sustained::fresh(wanted,wanted.match,wanted.epoch,now)||
           sustained::at<Address>(entity)!=image_+bindings::ReactionVtable)return;
        auto begin=sustained::at<Address>(entity+0x120),end=sustained::at<Address>(entity+0x128);
        if(end<begin||end-begin>256*16||(end-begin)%16)return;
        bool playing=false;
        for(Address p=begin;p<end;p+=16)if(!sustained::at<unsigned char>(p+12)&&population::reactionFamily(sustained::at<int>(p))){playing=true;break;}
        audibleMatch_=wanted.match;audibleEpoch_=wanted.epoch;audibleSequence_=wanted.playSequence;
        audibleUpdated_=now;audibleVoice_=playing;
    }
    struct Followup {bool active=false;Source source=Source::Result;uint64_t id=0,sequence=0,due=0,until=0;int type=75;};
    Followup followup_;std::array<Followup,8> waiting_{};
    void clearFollowups(){followup_={};waiting_={};}
    void advanceFollowup(){followup_=waiting_[0];for(unsigned i=1;i<waiting_.size();++i)waiting_[i-1]=waiting_[i];waiting_.back()={};}
    struct Receipt {bool active=false,retried=false;Address owner=0;Source source=Source::Live;uint64_t id=0,sequence=0,began=0,until=0;int type=0;unsigned observations=0;std::array<int,256> priorHandles{};unsigned priorCount=0;};
    Receipt receipt_;
    std::atomic<bool> priorityPending_{false};
    void receipt(Source source,uint64_t id,int type,Address entity,const sustained::Intent& wanted,uint64_t now){
        if(!wanted.playSequence||source==Source::Lineup)return;
        bool scoring=source==Source::Eruption||(source==Source::Result&&wanted.scoringReaction);
        receipt_={true,false,entity,source,id,wanted.playSequence,now,now+(scoring?1200:600),type,0};
        auto begin=sustained::at<Address>(entity+0x120),end=sustained::at<Address>(entity+0x128);
        if(end>=begin&&end-begin<=256*16&&(end-begin)%16==0)for(Address p=begin;p<end&&receipt_.priorCount<receipt_.priorHandles.size();p+=16)
            if(!sustained::at<unsigned char>(p+12)&&sustained::at<int>(p)==type)receipt_.priorHandles[receipt_.priorCount++]=sustained::at<int>(p+4);
    }
    void checkReceipt(Address entity,const sustained::Intent& wanted,uint64_t now){
        using namespace sustained;
        if(!receipt_.active)return;
        auto& r=receipt_;
        bool same=r.source==Source::Live?wanted.liveCheer==r.id:r.source==Source::Result?wanted.resultCheer==r.id:wanted.celebration==r.id;
        if(!same||r.sequence!=wanted.playSequence||r.owner!=entity||now<r.began){receipt_={};return;}
        Address begin=at<Address>(entity+0x120),end=at<Address>(entity+0x128),queue=at<Address>(entity+0xd0),queueEnd=at<Address>(entity+0xd8);
        if(end<begin||end-begin>256*16||(end-begin)%16||queueEnd<queue||queueEnd-queue>256*8||(queueEnd-queue)%8){receipt_={};return;}
        bool queued=false;
        for(Address p=begin;p<end;p+=16){if(at<unsigned char>(p+12))continue;int type=at<int>(p);
            bool existing=false;for(unsigned i=0;i<r.priorCount;++i)existing=existing||r.priorHandles[i]==at<int>(p+4);
            if(type==r.type&&!existing){auto done=r;receipt_={};report(done.source,done.id,Delivery::Registered);return;}
        }
        for(Address p=queue;p<queueEnd;p+=8){int type=at<int>(p);if(type==r.type)queued=true;}
        ++r.observations;auto age=now-r.began;
        if(now>r.until){auto done=r;receipt_={};report(done.source,done.id,Delivery::Unregistered);return;}
        // Receiver success means queued, not played. Recover only after the
        // native three-attempt queue has discarded this type, on the same
        // fresh play/owner, once, before any delayed cheer would be intrusive.
        bool scoring=r.source==Source::Eruption||(r.source==Source::Result&&wanted.scoringReaction);
        if(!queued&&age>=(scoring?350u:100u)&&r.observations>=3){
            if(r.retried){if(scoring)return;auto done=r;receipt_={};report(done.source,done.id,Delivery::Unregistered);return;}
            // The native queue has already exhausted the event-specific
            // selector. Retrying that same unavailable/full selector repeats
            // its failure. Use one listening-verified generic equivalent at
            // the SAME crowd size and valence; leave native voices untouched.
            int fallback=(population::negative(r.type)?92:88)+(std::clamp)(r.type%4,1,3);
            if(fallback==r.type)fallback=(population::negative(r.type)?92:88)+(std::max)(1,r.type%4-1);
            r.type=fallback;r.priorCount=0;
            for(Address p=begin;p<end&&r.priorCount<r.priorHandles.size();p+=16)
                if(!at<unsigned char>(p+12)&&at<int>(p)==r.type)r.priorHandles[r.priorCount++]=at<int>(p+4);
            r.retried=true;r.began=now;r.observations=0;
            if(emit(api_,receiver_,image_,entity,r.type)){++sent_;report(r.source,r.id,Delivery::Recovery);}
            else{auto done=r;receipt_={};healthy_=false;report(done.source,done.id,Delivery::Failed);}
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
        if(emit(api_,receiver_,image_,entity,next.type)){++sent_;report(Source::Followup,next.id,Delivery::Sent);}
        else{healthy_=false;report(Source::Followup,next.id,Delivery::Failed);}
        return true;
    }
    void report(Source source,uint64_t id,Delivery delivery){
        if(source==Source::Live)liveResolved_=id;
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
        struct PriorityGuard {std::atomic<bool>& flag;const Receipt& receipt;bool scoring;~PriorityGuard(){flag=scoring&&receipt.active;}} pending{priorityPending_,receipt_,wanted.scoringReaction||wanted.celebration!=0};
        if(!healthy_||!fresh(wanted,wanted.match,wanted.epoch,now)){audibleVoice_=false;clearFollowups();receipt_={};return;}
        sampleAudible(entity,wanted,now);
        if(match_!=wanted.match||epoch_!=wanted.epoch){match_=wanted.match;epoch_=wanted.epoch;consumed_=result_=lineup_=liveConsumed_=0;liveDelivered_=liveResolved_=0;clearFollowups();receipt_={};}
        bool victory=wanted.celebration&&now<wanted.celebrationUntil&&consumed_!=wanted.celebration;
        bool priorityResult=!wanted.celebration&&wanted.scoringReaction&&wanted.resultCheer&&now<wanted.resultCheerUntil;
        if(victory||(priorityResult&&result_!=wanted.resultCheer)){receipt_={};clearFollowups();}
        checkReceipt(entity,wanted,now);if(!healthy_)return;
        bool during=wanted.liveCheer&&now<wanted.liveCheerUntil&&liveConsumed_!=wanted.liveCheer;
        // A late live-run accent must not take the receiver/receipt slot from
        // a fresh scoring cheer. Leave its ID unconsumed; it expires normally.
        bool scoring=priorityResult||(wanted.celebration&&now<wanted.celebrationUntil);
        if(during&&scoring)liveConsumed_=wanted.liveCheer;
        if(during&&!scoring){
            if(!population::reactionFamily(wanted.liveType))return;
            if(at<Address>(entity)!=image_+bindings::ReactionVtable)return;
            Address asset=at<Address>(entity+0x38);if(at<int>(asset+0x40)!=3||at<float>(asset+0x44)!=25)return;
            Address begin=at<Address>(entity+0x120),end=at<Address>(entity+0x128);
            if(end<begin||end-begin>256*16||(end-begin)%16){healthy_=false;return;}
            liveConsumed_=wanted.liveCheer;
            receipt(Source::Live,wanted.liveCheer,wanted.liveType,entity,wanted,now);
            if(emit(api_,receiver_,image_,entity,wanted.liveType)){++sent_;liveDelivered_=wanted.liveCheer;report(Source::Live,wanted.liveCheer,Delivery::Sent);if(wanted.majorLive)arm(Source::Live,wanted.liveCheer,wanted.liveType,wanted,now);}
            else{receipt_={};healthy_=false;report(Source::Live,wanted.liveCheer,Delivery::Failed);}
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
        unsigned sentNow=0;
        auto send=[&](int type){
            type=population::reactionType(type,wanted.populationSize);
            // Re-read after each send: the native instance array may reallocate.
            Address begin=at<Address>(entity+0x120),end=at<Address>(entity+0x128);
            if(end<begin||end-begin>256*16||(end-begin)%16){healthy_=false;return;}
            // Observe the primary, including rushing-TD selectors. A secondary
            // layer must not overwrite its receipt merely because families match.
            if(!sentNow)receipt(victory?Source::Eruption:result?Source::Result:Source::Lineup,victory?wanted.celebration:result?wanted.resultCheer:wanted.lineupCheer,type,entity,wanted,now);
            if(emit(api_,receiver_,image_,entity,type)){++sent_;++sentNow;}else{receipt_={};healthy_=false;}
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
    bool priorityPending()const{return priorityPending_.load();}
    bool liveDelivered(uint64_t id)const{return id&&liveDelivered_.load()==id;}
    bool audible(Address match,unsigned epoch,uint64_t sequence,uint64_t now)const{
        auto updated=audibleUpdated_.load();
        return audibleVoice_.load()&&match&&audibleMatch_.load()==match&&audibleEpoch_.load()==epoch&&
            audibleSequence_.load()==sequence&&updated&&now>=updated&&now-updated<=150;
    }
    bool liveResolved(uint64_t id)const{return id&&liveResolved_.load()==id;}
};
}
