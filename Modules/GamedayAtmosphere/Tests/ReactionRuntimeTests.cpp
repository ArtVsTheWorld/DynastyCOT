#define WIN32_LEAN_AND_MEAN
#include "../Source/ReactionAudioRuntime.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <set>
using namespace attendance::sustained;
constexpr Address image=0x140000000;
unsigned char entity[0x200]{},asset[0x80]{},systemData[0x60]{},list[0x20]{},description[0x60]{},reactionValue[0x60]{},parameter[0x60]{},instances[4096]{};
std::vector<int> received;
std::vector<attendance::cheer::Delivery> reports;
void report(attendance::cheer::Source,uint64_t,attendance::cheer::Delivery delivery){reports.push_back(delivery);}
Address entries[1]{};unsigned arenas=0,arenaEnds=0,events=0,eventEnds=0,sends=0;
bool immediateVoice=false;
int voiceOnType=-1;
bool badParameter=false,fault=false,reenter=false;attendance::cheer::Runtime* recursive=nullptr;Intent wanted;
template<size_t N>Address addr(unsigned char (&p)[N]){return reinterpret_cast<Address>(p);}
template<class T>void put(Address p,T v){memcpy(reinterpret_cast<void*>(p),&v,sizeof(v));}
Address __fastcall arenaInit(Address a){++arenas;assert(at<Address>(a-0x18)==image+attendance::bindings::SwellArenaVtable);return a;}
void __fastcall arenaEnd(Address){++arenaEnds;assert(events==eventEnds);}
Address __fastcall construct(Address e,Address,Address d){
    assert(d==addr(description));++events;put<unsigned char>(e+0xac,1);put<unsigned>(e+0x18,1);
    entries[0]=addr(reactionValue);put(e+0x20,reinterpret_cast<Address>(entries));put(addr(reactionValue)+0x30,addr(parameter));
    put<int>(addr(parameter)+0x40,1);put<unsigned char>(addr(parameter)+0x44,0);
    put(addr(parameter)+0x20,reinterpret_cast<Address>(badParameter?"wrong":"reacType"));return e;
}
void __fastcall eventEnd(Address){++eventEnds;}
void __fastcall set(Address v,int n){assert(attendance::population::reactionFamily(n));put(v,n);}
Address __fastcall receive(Address target,Address){
    assert(target==addr(entity)+0x40);int type=at<int>(addr(reactionValue));assert(attendance::population::reactionFamily(type));received.push_back(type);++sends;
    if(immediateVoice||type==voiceOnType){put(addr(entity)+0x120,addr(instances));put(addr(entity)+0x128,addr(instances)+16);put<int>(addr(instances),type);put<int>(addr(instances)+4,7777);put<unsigned char>(addr(instances)+12,0);}
    if(reenter&&recursive)recursive->tick(addr(entity),wanted,wanted.updated);
    if(fault){fault=false;RaiseException(EXCEPTION_ACCESS_VIOLATION,0,0,nullptr);}return 0;
}
void balanced(){assert(arenas==arenaEnds&&events==eventEnds);}
int main(){
    put(addr(entity),image+attendance::bindings::ReactionVtable);put(addr(entity)+0x38,addr(asset));
    put(addr(asset)+0x40,3);put(addr(asset)+0x44,25.f);put(addr(asset)+0x20,addr(systemData));
    put<unsigned>(addr(systemData)+0x38,205964879);put(addr(systemData)+0x20,addr(list)+8);
    put<unsigned>(addr(list)+4,1);put(addr(list)+8,addr(description));
    put<unsigned>(addr(description)+0x40,205964879);put<unsigned>(addr(description)+0x44,3424066396u);
    put(addr(description)+0x38,reinterpret_cast<Address>("Trigger_AreaReaction"));
    Api api;api.arenaInit=arenaInit;api.arenaDestroy=arenaEnd;api.construct=construct;api.eventDestroy=eventEnd;api.set=set;
    auto voices=[&](int n){put(addr(entity)+0x120,n?addr(instances):Address(0));put(addr(entity)+0x128,n?addr(instances)+16*n:Address(0));
        for(int i=0;i<n;++i){put<int>(addr(instances)+16*i,i%2?59:83);put<int>(addr(instances)+16*i+4,100+i);put<unsigned char>(addr(instances)+16*i+12,0);}};
    auto intent=[&](){Intent w;w.active=true;w.match=123;w.epoch=1;w.updated=1000;return w;};
    unsigned checkId=0;auto check=[&](unsigned n,auto action){++checkId;unsigned before=sends;action();balanced();if(sends!=before+n)std::fprintf(stderr,"check %u: expected %u sends, got %u\n",checkId,n,sends-before);assert(sends==before+n);};
    // Dispatch alone is not sound proof. Only a fresh active voice can admit shake.
    voices(0);attendance::cheer::Runtime paired;paired.configure(image,&api,receive);
    wanted=intent();wanted.playSequence=44;wanted.liveCheer=1;wanted.liveCheerUntil=1500;wanted.liveType=83;
    check(1,[&]{paired.tick(addr(entity),wanted,1000);});assert(!paired.audible(123,1,44,1000));
    voices(1);wanted.updated=1030;check(0,[&]{paired.tick(addr(entity),wanted,1030);});
    assert(paired.audible(123,1,44,1030)&&!paired.audible(123,1,45,1030)&&!paired.audible(124,1,44,1030)&&!paired.audible(123,2,44,1030));
    assert(!paired.audible(123,1,44,1181));
    put<unsigned char>(addr(instances)+12,1);wanted.updated=1060;check(0,[&]{paired.tick(addr(entity),wanted,1060);});assert(!paired.audible(123,1,44,1060));
    voices(1);wanted.active=false;check(0,[&]{paired.tick(addr(entity),wanted,1060);});assert(!paired.audible(123,1,44,1060));
    // Every source dispatches with zero, matching or multiple native voices.
    // A new intended event can overlap; the same event cannot repeat.
    for(int source=0;source<4;++source)for(int n:{0,1,2,4}){
        voices(n);attendance::cheer::Runtime runtime;runtime.configure(image,&api,receive,report);wanted=intent();
        if(source==0){wanted.liveCheer=1;wanted.liveCheerUntil=1500;wanted.liveType=83;}
        if(source==1){wanted.lineupCheer=1;wanted.lineupCheerBegan=1000;wanted.lineupCheerUntil=2500;}
        if(source==2){wanted.resultCheer=1;wanted.resultCheerUntil=2500;wanted.resultType=59;}
        if(source==3){wanted.celebration=1;wanted.celebrationUntil=2500;}
        recursive=&runtime;reenter=true;check(1,[&]{runtime.tick(addr(entity),wanted,1000);});reenter=false;
        check(0,[&]{for(int i=0;i<20;++i)runtime.tick(addr(entity),wanted,1000);});
        if(source==0)++wanted.liveCheer;if(source==1)++wanted.lineupCheer;if(source==2)++wanted.resultCheer;if(source==3)++wanted.celebration;
        wanted.updated=1100;check(1,[&]{runtime.tick(addr(entity),wanted,1100);});
    }
    for(int type:{11,91,95}){voices(4);for(int i=0;i<4;++i)put<int>(addr(instances)+16*i,type);
        attendance::cheer::Runtime runtime;runtime.configure(image,&api,receive);wanted=intent();wanted.liveCheer=1;wanted.liveCheerUntil=1500;wanted.liveType=type;
        check(1,[&]{runtime.tick(addr(entity),wanted,1000);});assert(received.back()==type);check(0,[&]{runtime.tick(addr(entity),wanted,1000);});}
    voices(4);attendance::cheer::Runtime burst;burst.configure(image,&api,receive);wanted=intent();wanted.celebration=1;wanted.celebrationUntil=3000;wanted.deepCheers=true;
    check(2,[&]{burst.tick(addr(entity),wanted,1000);});assert(received[received.size()-2]==91&&received.back()==75);check(0,[&]{burst.tick(addr(entity),wanted,1100);});
    attendance::cheer::Runtime stagger;stagger.configure(image,&api,receive);wanted=intent();wanted.liveCheer=1;wanted.liveCheerUntil=1500;wanted.liveType=83;wanted.majorLive=wanted.staggered=true;wanted.playSequence=wanted.reactionId=20;wanted.reason=Reason::Result;
    check(1,[&]{stagger.tick(addr(entity),wanted,1000);});wanted.updated=1899;check(0,[&]{stagger.tick(addr(entity),wanted,1899);});wanted.updated=1900;check(1,[&]{stagger.tick(addr(entity),wanted,1900);});check(0,[&]{stagger.tick(addr(entity),wanted,1900);});
    wanted.liveCheer=2;wanted.liveCheerUntil=3500;wanted.updated=3000;check(1,[&]{stagger.tick(addr(entity),wanted,3000);});wanted.active=false;check(0,[&]{stagger.tick(addr(entity),wanted,3100);});wanted.active=true;wanted.updated=3900;check(0,[&]{stagger.tick(addr(entity),wanted,3900);});
    wanted.liveCheer=3;wanted.liveCheerUntil=5500;wanted.updated=5000;check(1,[&]{stagger.tick(addr(entity),wanted,5000);});++wanted.playSequence;wanted.updated=5900;check(0,[&]{stagger.tick(addr(entity),wanted,5900);});
    // A scoring eruption takes priority over an unconsumed live accent.
    voices(4);attendance::cheer::Runtime simultaneous;simultaneous.configure(image,&api,receive);
    wanted=intent();wanted.liveCheer=9;wanted.liveCheerUntil=1500;wanted.liveType=11;wanted.celebration=1;wanted.celebrationUntil=3000;
    check(1,[&]{simultaneous.tick(addr(entity),wanted,1000);});assert(received.back()==91);
    check(0,[&]{simultaneous.tick(addr(entity),wanted,1100);});
    // A scoring result cancels the old run accent's queued followup/recovery,
    // and the score owns the primary dispatch even when both arrive together.
    voices(0);attendance::cheer::Runtime scorePriority;scorePriority.configure(image,&api,receive);
    wanted=intent();wanted.playSequence=120;wanted.staggered=wanted.majorLive=true;
    wanted.liveCheer=71;wanted.liveCheerUntil=2500;wanted.liveType=83;
    check(1,[&]{scorePriority.tick(addr(entity),wanted,1000);});
    wanted.updated=1100;wanted.liveCheer=72;wanted.scoringReaction=true;wanted.resultCheer=90;wanted.resultCheerUntil=3600;wanted.resultType=75;
    check(1,[&]{scorePriority.tick(addr(entity),wanted,1100);});assert(received.back()==75);
    assert(scorePriority.priorityPending());
    wanted.updated=1200;check(0,[&]{scorePriority.tick(addr(entity),wanted,1200);});
    wanted.updated=1300;check(0,[&]{scorePriority.tick(addr(entity),wanted,1300);});
    wanted.updated=1900;check(1,[&]{scorePriority.tick(addr(entity),wanted,1900);});assert(received.back()!=83&&scorePriority.priorityPending());
    wanted.updated=2000;check(0,[&]{scorePriority.tick(addr(entity),wanted,2000);});assert(scorePriority.priorityPending());
    wanted.updated=2401;check(0,[&]{scorePriority.tick(addr(entity),wanted,2401);});assert(!scorePriority.priorityPending());
    // A later result must not cancel the earlier valid major-play followup.
    voices(4);attendance::cheer::Runtime pending;pending.configure(image,&api,receive);wanted=intent();
    wanted.playSequence=wanted.reactionId=90;wanted.reason=Reason::Result;wanted.staggered=wanted.majorLive=true;
    wanted.liveCheer=1;wanted.liveCheerUntil=1500;wanted.liveType=83;check(1,[&]{pending.tick(addr(entity),wanted,1000);});
    wanted.updated=1100;wanted.resultCheer=1;wanted.resultCheerUntil=1600;wanted.deepCheers=true;
    check(1,[&]{pending.tick(addr(entity),wanted,1100);});
    wanted.updated=1900;check(1,[&]{pending.tick(addr(entity),wanted,1900);});
    wanted.updated=2000;check(1,[&]{pending.tick(addr(entity),wanted,2000);});
    check(0,[&]{pending.tick(addr(entity),wanted,2000);});
    // Preserve fresh leases, early lineup windows, native shape and fault guards.
    voices(0);attendance::cheer::Runtime guards;guards.configure(image,&api,receive);wanted=intent();wanted.lineupCheer=1;wanted.lineupCheerBegan=2000;wanted.lineupCheerUntil=5000;
    check(0,[&]{guards.tick(addr(entity),wanted,1000);});wanted.updated=3500;check(0,[&]{guards.tick(addr(entity),wanted,3500);});
    wanted.lineupCheer=2;wanted.lineupCheerBegan=4000;wanted.updated=4000;wanted.active=false;check(0,[&]{guards.tick(addr(entity),wanted,4000);});wanted.active=true;
    put(addr(asset)+0x44,30.f);check(0,[&]{guards.tick(addr(entity),wanted,4000);});put(addr(asset)+0x44,25.f);check(1,[&]{guards.tick(addr(entity),wanted,4000);});
    attendance::cheer::Runtime failed;failed.configure(image,&api,receive);badParameter=true;check(0,[&]{failed.tick(addr(entity),wanted,4000);});assert(!failed.healthy());badParameter=false;
    attendance::cheer::Runtime faulted;faulted.configure(image,&api,receive);fault=true;check(1,[&]{faulted.tick(addr(entity),wanted,4000);});assert(!faulted.healthy());check(0,[&]{faulted.tick(addr(entity),wanted,4000);});
    // One bounded queue-recovery request, despite other active native voices.
    voices(4);attendance::cheer::Runtime receipt;receipt.configure(image,&api,receive,report);wanted=intent();wanted.playSequence=1;wanted.liveCheer=1;wanted.liveCheerUntil=1500;wanted.liveType=83;
    auto tick=[&](uint64_t now){wanted.updated=now;receipt.tick(addr(entity),wanted,now);};
    check(1,[&]{tick(1000);});check(0,[&]{tick(1030);tick(1060);});check(1,[&]{tick(1100);});assert(reports.back()==attendance::cheer::Delivery::Recovery);
    check(0,[&]{tick(1130);tick(1160);tick(1200);});assert(reports.back()==attendance::cheer::Delivery::Unregistered);
    // Snapshot every old handle: a third pre-existing matching voice cannot
    // falsely establish that the new request registered. Only a new handle can.
    voices(4);for(int i=0;i<4;++i)put<int>(addr(instances)+16*i,83);
    wanted.liveCheer=2;wanted.liveCheerUntil=2500;check(1,[&]{tick(2000);});check(0,[&]{tick(2030);});assert(reports.back()==attendance::cheer::Delivery::Sent);
    put<int>(addr(instances)+3*16+4,777);check(0,[&]{tick(2060);});assert(reports.back()==attendance::cheer::Delivery::Registered);
    // Still queued means no recovery; ownership/sequence changes cancel it.
    voices(0);unsigned char queue[8]{};put<int>(addr(queue),83);wanted.liveCheer=3;wanted.liveCheerUntil=3500;check(1,[&]{tick(3000);});put(addr(entity)+0xd0,addr(queue));put(addr(entity)+0xd8,addr(queue)+8);
    check(0,[&]{tick(3030);tick(3060);tick(3100);});++wanted.playSequence;check(0,[&]{tick(3130);});put<Address>(addr(entity)+0xd0,0);put<Address>(addr(entity)+0xd8,0);
    voices(4);attendance::cheer::Runtime synchronous;synchronous.configure(image,&api,receive,report);wanted=intent();wanted.playSequence=10;wanted.liveCheer=1;wanted.liveCheerUntil=1500;wanted.liveType=83;
    immediateVoice=true;check(1,[&]{synchronous.tick(addr(entity),wanted,1000);});immediateVoice=false;wanted.updated=1030;check(0,[&]{synchronous.tick(addr(entity),wanted,1030);});assert(reports.back()==attendance::cheer::Delivery::Registered);voices(0);
    // Rushing-TD is now also a primary family. Receipt ownership follows
    // dispatch order rather than mistaking every family18 voice for a followup.
    voices(0);attendance::cheer::Runtime rushing;rushing.configure(image,&api,receive,report);wanted=intent();wanted.playSequence=11;wanted.celebration=1;wanted.celebrationUntil=2500;wanted.celebrationType=75;
    immediateVoice=true;check(1,[&]{rushing.tick(addr(entity),wanted,1000);});immediateVoice=false;wanted.updated=1030;check(0,[&]{rushing.tick(addr(entity),wanted,1030);});assert(reports.back()==attendance::cheer::Delivery::Registered);voices(0);
    // Every listening-approved selector reaches the same native construction
    // path; excluded and untested selectors are rejected before allocation.
    // Reproduce native rejection of an event selector while a generic
    // same-size equivalent is available. Keep the original request immediate,
    // then allow exactly one fallback and verify ITS new handle.
    for(int type:{11,59,67,83,95}){
        voices(0);attendance::cheer::Runtime fallback;fallback.configure(image,&api,receive,report);
        wanted=intent();wanted.playSequence=50;wanted.liveCheer=1;wanted.liveCheerUntil=1500;wanted.liveType=type;
        int generic=(attendance::population::negative(type)?92:88)+(type%4);
        if(generic==type)--generic;
        voiceOnType=generic;check(1,[&]{fallback.tick(addr(entity),wanted,1000);});assert(received.back()==type);
        for(uint64_t time:{1030u,1060u,1100u}){wanted.updated=time;fallback.tick(addr(entity),wanted,time);}
        assert(received.back()==generic);wanted.updated=1130;check(0,[&]{fallback.tick(addr(entity),wanted,1130);});
        assert(reports.back()==attendance::cheer::Delivery::Registered);voiceOnType=-1;
    }
    unsigned valid=0;
    for(int id=0;id<112;++id){bool heard=id%4!=0&&id!=13&&id!=14&&id!=15&&id!=23;
        if(heard){check(1,[&]{assert(attendance::cheer::emit(api,receive,image,addr(entity),id));});assert(received.back()==id);++valid;}
        else check(0,[&]{assert(!attendance::cheer::emit(api,receive,image,addr(entity),id));});
    }assert(valid==80);
    for(int invalid:{-1,112,999})assert(!attendance::cheer::emit(api,receive,image,addr(entity),invalid));
    std::puts("PASS: all earned layers dispatch over matching/multiple native voices, once-per-event and reentrant guards, bounded followups/recovery, complete prior-handle receipts, ownership/expiry/callback/fault safety and native family validation.");
}
