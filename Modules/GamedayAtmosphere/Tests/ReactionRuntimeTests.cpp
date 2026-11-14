#define WIN32_LEAN_AND_MEAN
#include "../Source/ReactionAudioRuntime.h"
#include "../Source/SpringReactions.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <set>
using namespace attendance::sustained;
constexpr Address image=0x140000000;
unsigned char entity[0x200]{},asset[0x80]{},systemData[0x60]{},list[0x20]{},description[0x60]{},reactionValue[0x60]{},parameter[0x60]{},instances[4096]{};
std::vector<int> received;
std::vector<attendance::cheer::Delivery> reports;
std::vector<std::pair<uint64_t,attendance::cheer::Delivery>> receipts;
void report(attendance::cheer::Source,uint64_t id,attendance::cheer::Delivery delivery){reports.push_back(delivery);receipts.emplace_back(id,delivery);}
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
    // A visiting cheer rejected on the first population sample stays consumed if
    // the percentage changes before its original window expires. A new event
    // has its own dispatch opportunity.
    {
        for(bool seenEarlier:{false,true}){
        voices(0);attendance::cheer::Runtime visitors;visitors.configure(image,&api,receive,report);
        wanted=intent();wanted.playSequence=91;
        if(seenEarlier)visitors.tick(addr(entity),wanted,1000);
        visitors.suppressPopulation(10019);
        wanted.liveCheer=10019;wanted.liveCheerUntil=2000;wanted.liveType=83;
        wanted.updated=1100;check(0,[&]{visitors.tick(addr(entity),wanted,1100);});
        wanted.liveCheer=10020;wanted.updated=1200;check(1,[&]{visitors.tick(addr(entity),wanted,1200);});
        }
    }
    // Dispatch alone is not sound proof. Only a fresh active voice can admit shake.
    voices(0);attendance::cheer::Runtime paired;paired.configure(image,&api,receive);
    wanted=intent();wanted.playSequence=44;wanted.liveCheer=1;wanted.liveCheerUntil=1500;wanted.liveType=83;
    check(1,[&]{paired.tick(addr(entity),wanted,1000);});assert(!paired.audible(123,1,44,1000));
    assert(paired.eventFocus(123,1,44,1000)&&paired.eventFocus(123,1,44,1699));
    assert(!paired.eventFocus(123,1,44,1700)&&!paired.eventFocus(124,1,44,1100)&&!paired.eventFocus(123,2,44,1100)&&!paired.eventFocus(123,1,45,1100)&&!paired.eventFocus(123,1,44,999));
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
        check(1,[&]{runtime.tick(addr(entity),wanted,1000);});assert(received.back()==attendance::population::reactionType(type,3));check(0,[&]{runtime.tick(addr(entity),wanted,1000);});}
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
    wanted.updated=1900;check(0,[&]{scorePriority.tick(addr(entity),wanted,1900);});assert(received.back()!=83&&scorePriority.priorityPending());
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
    check(1,[&]{tick(1000);});check(0,[&]{tick(1030);tick(1060);});check(0,[&]{tick(1100);});assert(reports.back()==attendance::cheer::Delivery::Sent);
    check(0,[&]{tick(1130);tick(1160);tick(1601);});assert(reports.back()==attendance::cheer::Delivery::Unregistered);
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
        voiceOnType=generic;check(1,[&]{fallback.tick(addr(entity),wanted,1000);});assert(received.back()==attendance::population::reactionType(type,3));
        for(uint64_t time:{1030u,1060u,1100u}){wanted.updated=time;fallback.tick(addr(entity),wanted,time);}
        assert(received.back()==attendance::population::reactionType(type,3));wanted.updated=1601;check(0,[&]{fallback.tick(addr(entity),wanted,1601);});
        assert(reports.back()==attendance::cheer::Delivery::Unregistered);voiceOnType=-1;
    }
    // Relaxed spring coverage uses the same owner once, yields to already
    // playing native reactions, and does not arm ordinary recovery duplicates.
    for(int n:{0,1,4}){voices(n);attendance::cheer::Runtime springRuntime;springRuntime.configure(image,&api,receive,report);
        wanted=intent();wanted.spring=true;wanted.resultCheer=900;wanted.resultCheerUntil=1600;wanted.playSequence=77;wanted.populationSize=1;
        check(1,[&]{springRuntime.tick(addr(entity),wanted,1000);});assert(received.back()==89);wanted.updated=1030;check(0,[&]{springRuntime.tick(addr(entity),wanted,1030);});
        wanted.updated=1200;check(0,[&]{springRuntime.tick(addr(entity),wanted,1200);});
    }
    // Positive voices, AWW, old same-family handles and unknown-age voices
    // cannot consume the one small GEN_BOO opportunity. It never retries or
    // adds a continuous layer; a truly fresh equivalent native voice still wins.
    for(int nativeType:{83,95,101}){
        voices(1);put<int>(addr(instances),nativeType);attendance::cheer::Runtime springBoo;springBoo.configure(image,&api,receive,report);
        wanted=intent();wanted.spring=true;wanted.resultCheer=901;wanted.resultCheerUntil=1600;wanted.playSequence=78;wanted.populationSize=1;wanted.resultType=101;
        check(1,[&]{springBoo.tick(addr(entity),wanted,1000);});assert(received.back()==101);
        for(uint64_t now:{1030u,1100u,1300u,1601u}){wanted.updated=now;check(0,[&]{springBoo.tick(addr(entity),wanted,now);});}
    }
    for(bool positive:{false,true}){
        voices(0);attendance::cheer::Runtime matching;matching.configure(image,&api,receive,report);wanted=intent();wanted.spring=true;wanted.playSequence=79;wanted.populationSize=1;wanted.resultType=positive?89:101;
        check(0,[&]{matching.tick(addr(entity),wanted,1000);});
        voices(1);put<int>(addr(instances),positive?91:103); // generic family cannot prove an exact-event duplicate
        wanted.updated=1030;wanted.resultCheer=902;wanted.resultCheerUntil=1630;
        check(1,[&]{matching.tick(addr(entity),wanted,1030);});assert(reports.back()==attendance::cheer::Delivery::Sent);
        // A carried-over native handle is not a new play's reaction.
        ++wanted.playSequence;++wanted.resultCheer;wanted.updated=1060;
        check(1,[&]{matching.tick(addr(entity),wanted,1060);});assert(received.back()==(positive?89:101));
    }
    {voices(0);attendance::cheer::Runtime exact;exact.configure(image,&api,receive,report);wanted=intent();wanted.spring=true;wanted.playSequence=79;wanted.populationSize=2;wanted.resultType=58;
     check(0,[&]{exact.tick(addr(entity),wanted,1000);});voices(1);put<int>(addr(instances),59);wanted.updated=1030;wanted.resultCheer=902;wanted.resultCheerUntil=1630;
     check(0,[&]{exact.tick(addr(entity),wanted,1030);});assert(reports.back()==attendance::cheer::Delivery::NativePriority);}
    for(int discontinuity:{0,1,2,3}){
        voices(0);attendance::cheer::Runtime runtime;runtime.configure(image,&api,receive,report);wanted=intent();wanted.spring=true;wanted.playSequence=80;wanted.populationSize=1;wanted.resultType=101;
        check(0,[&]{runtime.tick(addr(entity),wanted,1000);});voices(1);put<int>(addr(instances),101);
        if(discontinuity==0)wanted.updated=1700; // sample gap
        if(discontinuity==1){wanted.updated=1030;++wanted.epoch;}
        if(discontinuity==2)wanted.updated=900; // clock rewind
        if(discontinuity==3){wanted.active=false;check(0,[&]{runtime.tick(addr(entity),wanted,1010);});wanted.active=true;wanted.updated=1030;}
        wanted.resultCheer=903;wanted.resultCheerUntil=wanted.updated+600;
        check(1,[&]{runtime.tick(addr(entity),wanted,wanted.updated);});assert(received.back()==101);
    }
    // A result is seen once, then the ordinary update stream contains no
    // result. The later sound callback must still receive the retained lease.
    {voices(0);attendance::spring::Reactions lease;attendance::energy::Play p;p.game={2,7,7,4,40,180};p.game.spring=true;p.game.springDynasty=77;p.game.springNumber=250;p.game.team0=1502;p.game.team1=p.game.springSchool=1145;p.offense=1;
     attendance::energy::Outcome r;r.accepted=r.completed=true;r.sequence=100;r.yards=15;
     auto selected=lease.observe(p,r,123,1,100,1000,true,1);assert(selected.selected);r={};
     auto later=lease.observe(p,r,123,1,100,1300,true,1);assert(later.intent.resultCheer==100);
     attendance::cheer::Runtime runtime;runtime.configure(image,&api,receive,report);immediateVoice=true;check(1,[&]{runtime.tick(addr(entity),later.intent,1300);});immediateVoice=false;
     later=lease.observe(p,r,123,1,100,1330,true,1);check(0,[&]{runtime.tick(addr(entity),later.intent,1330);});assert(reports.back()==attendance::cheer::Delivery::Registered);
     voices(0);attendance::cheer::Runtime unregistered;unregistered.configure(image,&api,receive,report);later.intent.resultCheer=101;later.intent.updated=2000;later.intent.resultCheerUntil=2600;later.intent.playSequence=101;
     check(1,[&]{unregistered.tick(addr(entity),later.intent,2000);});for(uint64_t now:{2030u,2100u,2200u,2500u,2601u}){later.intent.updated=now;check(0,[&]{unregistered.tick(addr(entity),later.intent,now);});}assert(reports.back()==attendance::cheer::Delivery::Unregistered);}
    // Two requests from one play keep independent receipts after the source
    // IDs change. No inferred failure may replay either sound.
    voices(0);receipts.clear();attendance::cheer::Runtime ledger;ledger.configure(image,&api,receive,report);
    wanted=intent();wanted.playSequence=777;wanted.liveCheer=201;wanted.liveCheerUntil=1500;wanted.liveType=2;
    check(1,[&]{ledger.tick(addr(entity),wanted,1000);});wanted.resultCheer=202;wanted.resultType=74;wanted.resultCheerUntil=2200;wanted.scoringReaction=true;wanted.updated=1050;
    check(1,[&]{ledger.tick(addr(entity),wanted,1050);});voices(2);put<int>(addr(instances),2);put<int>(addr(instances)+16,74);wanted.liveCheer=0;wanted.updated=1080;
    check(0,[&]{ledger.tick(addr(entity),wanted,1080);});
    assert(std::find(receipts.begin(),receipts.end(),std::make_pair(uint64_t(201),attendance::cheer::Delivery::Registered))!=receipts.end());
    assert(std::find(receipts.begin(),receipts.end(),std::make_pair(uint64_t(202),attendance::cheer::Delivery::Registered))!=receipts.end());
    assert(!ledger.observing());voices(0);
    // Missing eligible callbacks and intentional scoring suppression are named.
    wanted.liveCheer=203;wanted.liveCheerUntil=1090;wanted.updated=1100;check(0,[&]{ledger.tick(addr(entity),wanted,1100);});assert(reports.back()==attendance::cheer::Delivery::Expired);
    wanted.liveCheer=204;wanted.liveCheerUntil=1600;wanted.updated=1130;check(0,[&]{ledger.tick(addr(entity),wanted,1130);});assert(reports.back()==attendance::cheer::Delivery::PrioritySuppressed);
    // Production uses independent receivers for a verified skill accent and
    // its score. Both families can register without overwriting either ticket.
    voices(0);receipts.clear();attendance::cheer::Runtime touchdown,hurdle;touchdown.configure(image,&api,receive,report);hurdle.configure(image,&api,receive,report);
    wanted=intent();wanted.playSequence=800;wanted.resultCheer=301;wanted.resultCheerUntil=1700;wanted.resultType=74;wanted.scoringReaction=true;
    check(1,[&]{touchdown.tick(addr(entity),wanted,1000);});auto accent=wanted;accent.resultCheer=0;accent.scoringReaction=false;accent.liveCheer=302;accent.liveCheerUntil=1500;accent.liveType=2;
    check(1,[&]{hurdle.tick(addr(entity),accent,1000);});voices(2);put<int>(addr(instances),74);put<int>(addr(instances)+16,2);wanted.updated=accent.updated=1030;
    check(0,[&]{touchdown.tick(addr(entity),wanted,1030);hurdle.tick(addr(entity),accent,1030);});assert(!touchdown.observing()&&!hurdle.observing());
    wanted.updated=accent.updated=1100;check(0,[&]{touchdown.tick(addr(entity),wanted,1100);hurdle.tick(addr(entity),accent,1100);});
    // Same-selector requests cannot both claim one native voice. Matching
    // remains independent even when both dispatch before the first registers.
    voices(0);receipts.clear();attendance::cheer::Runtime sharedVoice;sharedVoice.configure(image,&api,receive,report);wanted=intent();wanted.playSequence=900;wanted.liveCheer=401;wanted.liveCheerUntil=1500;wanted.liveType=90;
    check(1,[&]{sharedVoice.tick(addr(entity),wanted,1000);});wanted.liveCheer=402;wanted.updated=1030;check(1,[&]{sharedVoice.tick(addr(entity),wanted,1030);});voices(1);put<int>(addr(instances),90);wanted.updated=1060;check(0,[&]{sharedVoice.tick(addr(entity),wanted,1060);});
    auto registeredCount=[&]{return std::count_if(receipts.begin(),receipts.end(),[](const auto& p){return p.second==attendance::cheer::Delivery::Registered;});};assert(registeredCount()==1);wanted.updated=1090;check(0,[&]{sharedVoice.tick(addr(entity),wanted,1090);});assert(registeredCount()==1);
    wanted.updated=1700;check(0,[&]{sharedVoice.tick(addr(entity),wanted,1700);});assert(reports.back()==attendance::cheer::Delivery::Unregistered);
    unsigned valid=0;
    for(int id=0;id<112;++id){bool heard=id%4!=0&&id!=13&&id!=14&&id!=15&&id!=23;
        if(heard){check(1,[&]{assert(attendance::cheer::emit(api,receive,image,addr(entity),id));});assert(received.back()==id);++valid;}
        else check(0,[&]{assert(!attendance::cheer::emit(api,receive,image,addr(entity),id));});
    }assert(valid==80);
    for(int invalid:{-1,112,999})assert(!attendance::cheer::emit(api,receive,image,addr(entity),invalid));
    // Spring focus is separate from the unchanged short dispatch deadline.
    voices(0);attendance::cheer::Runtime springFocus;springFocus.configure(image,&api,receive);wanted=intent();wanted.spring=true;wanted.playSequence=999;wanted.resultCheer=999;wanted.resultCheerUntil=1600;wanted.resultType=102;wanted.size=wanted.populationSize=2;
    springFocus.tick(addr(entity),wanted,1000);assert(springFocus.eventFocus(123,1,999,2499)&&!springFocus.eventFocus(123,1,999,2500));assert(!springFocus.eventFocus(123,1,1000,1100));
    std::puts("PASS: all earned layers dispatch over matching/multiple native voices, once-per-event and reentrant guards, bounded followups without receipt-based replay, complete prior-handle receipts, ownership/expiry/callback/fault safety and native family validation.");
}
