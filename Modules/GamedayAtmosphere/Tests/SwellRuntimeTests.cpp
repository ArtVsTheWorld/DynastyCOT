#define WIN32_LEAN_AND_MEAN
#include "../Source/SwellAudioRuntime.h"
#include "../Source/ReturnAtmosphere.h"
#include <cassert>
#include <cstdio>
#include <vector>
using namespace attendance::sustained;
constexpr Address image=0x140000000;
template<class T>void put(Address p,T value){memcpy(reinterpret_cast<void*>(p),&value,sizeof(value));}
template<size_t N>Address address(unsigned char (&p)[N]){return reinterpret_cast<Address>(p);}
unsigned char owner[0x300]{},resultObject[0x80]{},resultVtable[0x100]{},weak[0x20]{},endpoint[0x100]{},endpointVtable[0x100]{};
unsigned char descriptions[4][0x60]{},values[2][0x50]{},parameters[2][0x60]{};Address entries[2]{};
Address manager=0x111111;bool startEvent=false,queryMissing=false,badParameter=false,faultSend=false,reenter=false;
unsigned queries=0,queryEnds=0,weaks=0,weakEnds=0,arenas=0,arenaEnds=0,events=0,eventEnds=0,starts=0,stops=0;
struct Message {bool start;int size,attack,release;};std::vector<Message> messages;
Runtime* recursive=nullptr;Runtime* observer=nullptr;Intent request;uint64_t timeValue=1000;
Address __fastcall query(Address q,Address allocator,Address target,Address type,Address world){
    assert(q&&allocator==0x1234&&type==image+attendance::bindings::SwellQueryType&&world==0x2345);
    assert(at<unsigned>(target+8)==0x243a8b25);++queries;return q;
}
Address __fastcall get(Address,bool blocking){assert(!blocking);return queryMissing?0:address(resultObject);}
void __fastcall queryEnd(Address){++queryEnds;assert(weaks==weakEnds);}
void __fastcall weakGet(Address value,Address* out){assert(value==address(resultObject));*out=address(weak);++weaks;}
void __fastcall weakEnd(Address value,Address* out){assert(value==address(weak));*out=0;++weakEnds;}
Address __fastcall managerGet(Address value){assert(value==address(endpoint));return manager;}
Address __fastcall lookup(Address m,Address,const char* system,const char* name,Address* out){
    assert(m==manager&&!strcmp(system,"CORE_BG_CRWD_EventSystem"));startEvent=!strcmp(name,"Trigger_AreaPrePlaySwell")||!strcmp(name,"Trigger_AreaSwell");
    bool gameplay=!strcmp(name,"Trigger_AreaSwell")||!strcmp(name,"Stop_AreaSwell");
    assert(startEvent||!strcmp(name,"Stop_AreaPrePlaySwell")||!strcmp(name,"Stop_AreaSwell"));*out=address(descriptions[(gameplay?2:0)+(startEvent?0:1)]);return 0;
}
Address __fastcall init(Address a){++arenas;assert(at<Address>(a-0x18)==image+attendance::bindings::SwellArenaVtable);return a;}
void __fastcall arenaEnd(Address){++arenaEnds;assert(events==eventEnds);}
Address __fastcall construct(Address e,Address,Address description){
    put(e+0xa0,description);
    ++events;put<unsigned char>(e+0xac,1);put<unsigned>(e+0x18,startEvent?2u:1u);put(e+0x20,reinterpret_cast<Address>(entries));
    for(unsigned i=0;i<(startEvent?2u:1u);++i){
        entries[i]=address(values[i]);put(entries[i]+0x30,address(parameters[i]));
        const char* label=badParameter?"unknown":startEvent?(i==0?"reacSize":"swellAttack"):"swellRelease";
        put(address(parameters[i])+0x20,reinterpret_cast<Address>(label));put<int>(address(parameters[i])+0x40,1);
    }return e;
}
void __fastcall eventEnd(Address){++eventEnds;}
void __fastcall set(Address v,int value){put(v,value);}
Address __fastcall send(Address m,Address,Address event){
    assert(m==manager);Message message{startEvent,0,0,0};
    if(startEvent){++starts;message.size=at<int>(entries[0]);message.attack=at<int>(entries[1]);}
    else {++stops;message.release=at<int>(entries[0]);}
    messages.push_back(message);
    if(observer)observer->observeNativeStop(m,event,timeValue);
    if(reenter&&recursive)recursive->tick(address(owner)+0x40,request,timeValue);
    if(faultSend){faultSend=false;RaiseException(EXCEPTION_ACCESS_VIOLATION,0,0,nullptr);}return 0;
}
void balanced(){assert(queries==queryEnds&&weaks==weakEnds&&arenas==arenaEnds&&events==eventEnds);}
int main(){
    Address page=image+(attendance::bindings::SwellAllocator&~Address(0xffff));
    void* allocation=VirtualAlloc(reinterpret_cast<void*>(page),0x10000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);assert(reinterpret_cast<Address>(allocation)==page);
    put(image+attendance::bindings::SwellAllocator,Address(0x1234));
    put(address(owner),image+0xb227098);put(address(owner)+0x40,image+0xb227178);put(address(owner)+0x48,image+0xb227188);
    put(address(owner)+0x30,Address(0x2345));put(address(owner)+0x38,Address(0x3456));
    put(address(resultObject),address(resultVtable));put(address(resultVtable)+0x60,reinterpret_cast<Address>(weakGet));
    put(address(weak),address(endpoint)+8);put(address(endpoint),address(endpointVtable));put(address(endpointVtable)+0xe0,image+attendance::bindings::SwellManagerGetter);
    const unsigned ids[]={1690946487u,2466082807u,3736232244u,3732353268u};
    for(int i=0;i<4;++i){put<unsigned>(address(descriptions[i])+0x40,205964879);put<unsigned>(address(descriptions[i])+0x44,ids[i]);}
    Api api;api.query=query;api.get=get;api.queryDestroy=queryEnd;api.weakRelease=weakEnd;api.managerGet=managerGet;
    api.lookup=lookup;api.construct=construct;api.eventDestroy=eventEnd;api.set=set;api.send=send;api.arenaInit=init;api.arenaDestroy=arenaEnd;
    Runtime runtime;runtime.configure(image,nullptr,&api);recursive=&runtime;
    request={};request.match=123;request.epoch=1;request.updated=timeValue;request.active=true;request.reason=Reason::Opening;request.size=3;request.intensity=1000;
    auto tick=[&](uint64_t time){timeValue=time;request.updated=time;runtime.tick(address(owner)+0x40,request,time);balanced();};
    reenter=true;tick(1000);assert(runtime.playing()&&starts==1&&messages.back().size==3&&messages.back().attack==4000);
    assert(runtime.audible(request,1000));auto foreign=request;foreign.epoch++;assert(!runtime.audible(foreign,1000));
    reenter=false;
    for(uint64_t time=1100;time<21000;time+=100)tick(time);assert(starts==1&&queries==1); // No per-frame endpoint work while holding.
    tick(21000);assert(!runtime.playing()&&stops==1&&messages.back().release==4000);
    assert(runtime.audible(request,24999)&&!runtime.audible(request,25000));
    tick(28000);assert(starts==1);tick(29000);assert(starts==2);
    request.active=false;tick(29100);assert(stops==2&&!runtime.playing()); // Disable cleans up through the same live endpoint.
    request.active=true;tick(37100);assert(starts==3);runtime.tick(address(owner)+0x40,request,38000);balanced();assert(stops==3); // Expired lease.
    request.match=456;request.epoch=2;tick(46000);assert(starts==4);
    manager=0x222222;request.active=false;tick(46100);assert(stops==3&&!runtime.playing()); // Replacement manager never gets an old stop.
    request.active=true;queryMissing=true;tick(55000);assert(!runtime.playing());queryMissing=false;tick(56100);assert(runtime.playing());
    request.reaction=true;tick(56200);assert(!runtime.playing()&&stops==4);request.reaction=false;
    Runtime failed;failed.configure(image,nullptr,&api);badParameter=true;request.updated=65000;
    failed.tick(address(owner)+0x40,request,65000);balanced();assert(!failed.playing());badParameter=false;
    Runtime faulted;faulted.configure(image,nullptr,&api);faultSend=true;request.updated=68000;
    faulted.tick(address(owner)+0x40,request,68000);balanced();assert(!faulted.healthy()&&faulted.playing()&&!faulted.audible(request,68000));
    request.updated=70100;faulted.tick(address(owner)+0x40,request,70100);balanced();assert(!faulted.playing()); // Attempted send faults still receive a guarded stop.
    Runtime continuous;continuous.configure(image,nullptr,&api);request.continuous=true;request.releaseMs=20000;request.reason=Reason::Opening;request.updated=100000;
    continuous.tick(address(owner)+0x40,request,100000);balanced();assert(continuous.playing());
    unsigned char stockStop[0xb0]{};put<unsigned char>(address(stockStop)+0xac,1);put(address(stockStop)+0xa0,address(descriptions[1]));
    continuous.observeNativeStop(manager+1,address(stockStop));request.updated=125000;
    continuous.tick(address(owner)+0x40,request,125000);balanced();assert(continuous.playing()&&continuous.refreshes()==0); // No 20-second opening cutoff.
    continuous.observeNativeStop(manager,address(stockStop));request.updated=125100;
    continuous.tick(address(owner)+0x40,request,125100);balanced();assert(continuous.refreshes()==1&&messages.back().start&&messages.back().attack==500);
    request.updated=140000;request.offense=1;continuous.tick(address(owner)+0x40,request,140000);balanced();assert(continuous.playing());
    request.continuous=false;request.reason=Reason::None;request.updated=140100;
    continuous.tick(address(owner)+0x40,request,140100);balanced();assert(!continuous.playing()&&!messages.back().start&&messages.back().release==20000);
    // A fresh native maximum-size lineup roar is adopted rather than doubled.
    Runtime adopted;adopted.configure(image,nullptr,&api);
    unsigned char nativeStart[0xb0]{};startEvent=true;construct(address(nativeStart),0,0);set(entries[0],3);set(entries[1],4000);
    put(address(nativeStart)+0xa0,address(descriptions[0]));
    adopted.observeNativeStop(manager,address(nativeStart),150000);eventEnd(address(nativeStart));
    request={};request.match=789;request.epoch=3;request.updated=150100;request.active=true;request.reason=Reason::Lineup;request.priority=1;request.offense=0;request.size=3;request.releaseMs=8000;
    unsigned oldStarts=starts,oldStops=stops;
    adopted.tick(address(owner)+0x40,request,150100);balanced();assert(adopted.playing()&&adopted.adopted()==1&&starts==oldStarts);
    request.active=false;request.updated=150300;adopted.tick(address(owner)+0x40,request,150300);balanced();assert(stops==oldStops); // Native voice is not stopped by our cleanup.
    Runtime promoted;promoted.configure(image,nullptr,&api);request.active=true;request.updated=160000;
    promoted.tick(address(owner)+0x40,request,160000);assert(promoted.playing());
    request.reason=Reason::Eruption;request.priority=5;request.continuous=true;request.updated=160100;
    promoted.tick(address(owner)+0x40,request,160100);unsigned carriedStarts=starts;
    request.updated=195000;promoted.tick(address(owner)+0x40,request,195000);balanced();assert(promoted.playing()&&starts==carriedStarts); // Winning carry outlives the lineup cutoff without a restart.
    request.active=false;request.updated=195100;promoted.tick(address(owner)+0x40,request,195100);balanced();assert(!promoted.playing());
    // A new eruption reasserts an already playing smaller native channel once,
    // with a fast size-3 attack. It does not allocate an extra looping channel.
    Runtime burst;burst.configure(image,nullptr,&api);request={};request.active=true;request.match=789;request.epoch=3;request.reason=Reason::Lineup;request.continuous=true;request.size=2;request.updated=196000;
    burst.tick(address(owner)+0x40,request,196000);assert(messages.back().size==2);
    request.reason=Reason::Eruption;request.priority=6;request.celebration=1;request.size=3;request.attackMs=500;request.releaseMs=20000;request.updated=196100;
    oldStarts=starts;burst.tick(address(owner)+0x40,request,196100);assert(starts==oldStarts+1&&messages.back().size==3&&messages.back().attack==500);
    for(uint64_t now=196200;now<199000;now+=100){request.updated=now;burst.tick(address(owner)+0x40,request,now);}assert(starts==oldStarts+1);balanced();
    request.active=false;request.updated=199100;burst.tick(address(owner)+0x40,request,199100);assert(!burst.playing()&&messages.back().release==4000);balanced();
    Runtime extra;extra.configure(image,nullptr,&api,true);request.active=true;request.reason=Reason::LateDrive;request.continuous=true;request.attackMs=6000;request.releaseMs=12000;request.updated=200000;
    extra.tick(address(owner)+0x40,request,200000);balanced();assert(extra.playing()&&messages.back().start&&messages.back().attack==4000);
    put(address(stockStop)+0xa0,address(descriptions[1]));extra.observeNativeStop(manager,address(stockStop),201000);request.updated=202100;extra.tick(address(owner)+0x40,request,202100);assert(extra.refreshes()==0);
    put(address(stockStop)+0xa0,address(descriptions[3]));extra.observeNativeStop(manager,address(stockStop),203000);request.updated=204100;extra.tick(address(owner)+0x40,request,204100);balanced();assert(extra.refreshes()==1);
    request.reason=Reason::None;request.continuous=false;request.updated=205000;extra.tick(address(owner)+0x40,request,205000);balanced();assert(!extra.playing()&&!messages.back().start&&messages.back().release==12000);
    Runtime resultBridge;resultBridge.configure(image,nullptr,&api);request={};request.active=true;request.match=800;request.epoch=4;request.reason=Reason::Lineup;request.continuous=true;request.priority=1;request.size=2;request.updated=210000;
    resultBridge.tick(address(owner)+0x40,request,210000);assert(resultBridge.playing());oldStarts=starts;
    request.reason=Reason::Result;request.priority=5;request.resultCheer=1;request.resultCheerUntil=213000;request.size=3;request.releaseMs=8000;request.updated=210200;
    resultBridge.tick(address(owner)+0x40,request,210200);assert(starts==oldStarts+1&&messages.back().size==3);
    request.updated=210300;resultBridge.tick(address(owner)+0x40,request,210300);assert(starts==oldStarts+1); // One result refresh, never per frame.
    request.reaction=true;request.continuous=false;request.updated=218000;resultBridge.tick(address(owner)+0x40,request,218000);assert(!resultBridge.playing()&&messages.back().release==8000);
    request.updated=226100;resultBridge.tick(address(owner)+0x40,request,226100);assert(!resultBridge.playing()); // A fading result cannot restart even after its quiet gap.
    request.reaction=false;request.continuous=true;request.resultCheer=2;request.resultCheerUntil=228700;request.updated=226200;
    resultBridge.tick(address(owner)+0x40,request,226200);assert(resultBridge.playing());balanced();
    request.active=false;request.updated=226300;resultBridge.tick(address(owner)+0x40,request,226300);assert(!resultBridge.playing());balanced();
    Runtime quick;quick.configure(image,nullptr,&api);request={};request.active=true;request.match=900;request.epoch=1;request.reason=Reason::Lineup;request.lineup=42;request.continuous=true;request.updated=300000;
    quick.tick(address(owner)+0x40,request,300000);oldStarts=starts;
    put(address(stockStop)+0xa0,address(descriptions[1]));quick.observeNativeStop(manager,address(stockStop),300100);request.updated=300150;
    quick.tick(address(owner)+0x40,request,300150);assert(starts==oldStarts+1&&messages.back().attack==500); // Quick snap cannot leave the old two-second native-stop hole.
    quick.observeNativeStop(manager,address(stockStop),300200);request.updated=300300;quick.tick(address(owner)+0x40,request,300300);assert(starts==oldStarts+1); // One fast recovery per play token.
    request.active=false;request.updated=300400;quick.tick(address(owner)+0x40,request,300400);balanced();
    // Upset Watch uses the same adoption, quick recovery and one-token guard
    // for both the anticipation roar and its supporting gameplay swell.
    for(bool gameplay:{false,true}){
        Runtime upset;upset.configure(image,nullptr,&api,gameplay);
        request={};request.active=true;request.match=903;request.epoch=1;request.reason=Reason::Upset;request.lineup=88;request.continuous=true;request.size=2;request.updated=350000;
        upset.tick(address(owner)+0x40,request,350000);assert(upset.playing());oldStarts=starts;
        put(address(stockStop)+0xa0,address(descriptions[gameplay?3:1]));upset.observeNativeStop(manager,address(stockStop),350100);
        request.updated=350150;upset.tick(address(owner)+0x40,request,350150);assert(starts==oldStarts+1);
        upset.observeNativeStop(manager,address(stockStop),350200);request.updated=350300;upset.tick(address(owner)+0x40,request,350300);assert(starts==oldStarts+1);
        request.active=false;request.updated=350400;upset.tick(address(owner)+0x40,request,350400);assert(!upset.playing());balanced();
        Runtime nativeUpset;nativeUpset.configure(image,nullptr,&api,gameplay);
        startEvent=true;construct(address(nativeStart),0,0);set(entries[0],3);set(entries[1],4000);
        put(address(nativeStart)+0xa0,address(descriptions[gameplay?2:0]));nativeUpset.observeNativeStop(manager,address(nativeStart),360000);eventEnd(address(nativeStart));
        request.active=true;request.updated=360100;oldStarts=starts;nativeUpset.tick(address(owner)+0x40,request,360100);
        assert(nativeUpset.playing()&&nativeUpset.adopted()==1&&starts==oldStarts);
        request.active=false;request.updated=360300;nativeUpset.tick(address(owner)+0x40,request,360300);balanced();
    }
    // The production sound shape must hold BOTH native layers after the
    // whistle, then send one shortened release ending at the original time.
    for(bool gameplay:{false,true}){
        Runtime shaped;shaped.configure(image,nullptr,&api,gameplay);
        attendance::live::ReturnAtmosphere sound;attendance::energy::Play play;play.game={1,14,7,3,400,720};play.offense=0;
        unsigned beforeStarts=starts,beforeStops=stops;
        for(uint64_t now=400000;now<=405200;now+=25){
            play.game.phase=now<400200?3:4;
            sound.observe(play,901,1,now,true,now==400000?attendance::live::Outcome{attendance::live::Event::LongRun,1,0,20}:attendance::live::Outcome{},now<400200,false);
            Intent base;base.active=true;base.match=901;base.epoch=1;base.updated=now;
            auto voice=sound.merge(base,now,1);shaped.tick(address(owner)+0x40,voice,now);
            assert(starts==beforeStarts+1);
            if(now<401400)assert(shaped.playing()&&stops==beforeStops);
            else assert(!shaped.playing()&&stops==beforeStops+1&&messages.back().release==3800);
            balanced();
        }
    }
    // A timeout/penalty stop gets prompt recovery even if the matching native
    // cheer caused the one-shot request to be deduped. Result identity outlives
    // that request and does not collide with the previous lineup's token.
    Runtime interrupted;interrupted.configure(image,nullptr,&api);observer=&interrupted;
    request={};request.active=true;request.match=902;request.epoch=1;request.reason=Reason::Lineup;request.lineup=77;request.continuous=true;request.updated=500000;
    timeValue=500000;interrupted.tick(address(owner)+0x40,request,timeValue);oldStarts=starts;
    put(address(stockStop)+0xa0,address(descriptions[1]));interrupted.observeNativeStop(manager,address(stockStop),500100);
    request.reason=Reason::Result;request.priority=5;request.lineup=0;request.reactionId=77;request.resultCheer=request.liveCheer=0;request.updated=timeValue=500150;
    interrupted.tick(address(owner)+0x40,request,timeValue);assert(starts==oldStarts+1&&messages.back().attack==500);
    interrupted.observeNativeStop(manager,address(stockStop),500200);request.updated=timeValue=500250;
    interrupted.tick(address(owner)+0x40,request,timeValue);assert(starts==oldStarts+1); // Repeated stops cannot cause an event storm.
    auto nativeCount=interrupted.nativeStops();request.active=false;request.updated=timeValue=500350;
    interrupted.tick(address(owner)+0x40,request,timeValue);assert(!interrupted.playing()&&interrupted.nativeStops()==nativeCount); // Our own release is not a native interruption.
    request.active=true;request.reaction=true;request.continuous=false;request.updated=timeValue=500450;
    interrupted.tick(address(owner)+0x40,request,timeValue);assert(starts==oldStarts+1); // No replay of an already fading response.
    request.reaction=false;request.continuous=true;request.reactionId=78;request.updated=timeValue=500550;
    interrupted.tick(address(owner)+0x40,request,timeValue);assert(starts==oldStarts+2); // A new earned response may bypass the previous quiet gap without another cheer.
    request.active=false;request.updated=timeValue=500650;interrupted.tick(address(owner)+0x40,request,timeValue);observer=nullptr;balanced();
    // Real flight durations in the read-only capture were about 0.95, 1.95
    // and 3.12 seconds. The production callback must request a real gameplay
    // swell with a fast attack, even while the lineup roar already exists.
    for(bool warmSupport:{false,true}){
        Runtime primary,flightSupport;primary.configure(image,nullptr,&api);flightSupport.configure(image,nullptr,&api,true);
        attendance::live::PassFlight flight;attendance::energy::Play play;play.game={2,0,0,1,600,720};play.down=1;play.offense=0;
        Intent foundation;foundation.active=true;foundation.match=950;foundation.epoch=1;foundation.reason=Reason::Lineup;foundation.priority=1;foundation.continuous=true;foundation.lineup=99;foundation.size=1;foundation.attackMs=3000;foundation.releaseMs=12000;foundation.supporting=warmSupport;foundation.intensity=1000;foundation.updated=600000;
        primary.tick(address(owner)+0x40,foundation,600000);flightSupport.tick(address(owner)+0x40,supportingIntent(foundation),600000);
        flight.observe(play,950,1,600000,true,456,0,9,true);
        play.game.phase=3;flight.observe(play,950,1,600200,true,456,3,1,false);
        auto current=[&](uint64_t now){flight.observe(play,950,1,now,true,456,3,1,false);foundation.updated=now;return flight.merge(foundation,now,.8f);};
        auto inAir=current(600216);assert(inAir.supporting&&inAir.passFlight&&!inAir.liveCheer&&!inAir.shake);
        primary.tick(address(owner)+0x40,inAir,600216);flightSupport.tick(address(owner)+0x40,supportingIntent(inAir),600216);
        assert(primary.starts()==1&&primary.refreshes()==0&&flightSupport.starts()==1&&flightSupport.refreshes()==(warmSupport?1u:0u));
        assert(messages.back().start&&messages.back().attack==500&&messages.back().size==3);
        oldStarts=starts;
        for(uint64_t now=600250;now<=601100;now+=50){auto holding=current(now);primary.tick(address(owner)+0x40,holding,now);flightSupport.tick(address(owner)+0x40,supportingIntent(holding),now);assert(starts==oldStarts);}
        // Catch/incompletion proof releases flight ownership without guessing a
        // completion. A higher-priority result may override a carried flight id.
        play.game.phase=4;flight.observe(play,950,1,601150,true,456,3,4,false);foundation.updated=601150;auto ended=flight.merge(foundation,601150,.8f);
        auto score=ended;score.reason=Reason::Result;score.priority=5;score.reactionId=10;score.motionCompanion=true;
        assert(supportingIntent(score).reason==Reason::Result&&!passAnticipation(score));
        for(uint64_t now=601200;now<=602400;now+=50){flight.observe(play,950,1,now,true,456,3,4,false);foundation.updated=now;auto settling=flight.merge(foundation,now,.8f);flightSupport.tick(address(owner)+0x40,supportingIntent(settling),now);}
        assert(flightSupport.playing()==warmSupport);
        if(!warmSupport){
            assert(!messages.back().start&&messages.back().release==1200); // Do not inherit the lineup's 12-second release.
            // A subsequent verified pass can start promptly inside the previous
            // voice's quiet gap, but the same flight cannot start every frame.
            play.game.phase=1;flight.observe(play,950,1,602450,true,456,0,9,true);
            play.game.phase=3;flight.observe(play,950,1,602500,true,456,3,1,false);
            auto next=current(602516);flightSupport.tick(address(owner)+0x40,supportingIntent(next),602516);assert(flightSupport.playing()&&flightSupport.starts()==2);
            oldStarts=starts;auto same=current(602650);flightSupport.tick(address(owner)+0x40,supportingIntent(same),602650);assert(starts==oldStarts);
            put(address(stockStop)+0xa0,address(descriptions[3]));flightSupport.observeNativeStop(manager,address(stockStop),602700);
            auto resumed=current(602750);flightSupport.tick(address(owner)+0x40,supportingIntent(resumed),602750);assert(starts==oldStarts+1&&messages.back().attack==500);
            flightSupport.observeNativeStop(manager,address(stockStop),602800);auto repeated=current(602850);flightSupport.tick(address(owner)+0x40,supportingIntent(repeated),602850);assert(starts==oldStarts+1);
        }
        foundation.active=false;foundation.updated=603000;primary.tick(address(owner)+0x40,foundation,603000);flightSupport.tick(address(owner)+0x40,supportingIntent(foundation),603000);balanced();
    }
    // Full live-run voices join a TD/eruption without a new native attack.
    // A smaller voice or a stock stop still needs the normal fast refresh.
    for(bool gameplay:{false,true})for(bool erupt:{false,true})for(int initialSize:{2,3})for(bool nativeStop:{false,true}){
        Runtime joined;joined.configure(image,nullptr,&api,gameplay);
        request={};request.active=request.continuous=request.supporting=true;request.match=970;request.epoch=1;request.reason=Reason::Result;request.priority=4;request.reactionId=7;request.size=initialSize;request.updated=700000;
        joined.tick(address(owner)+0x40,request,700000);oldStarts=starts;oldStops=stops;
        if(nativeStop){put(address(stockStop)+0xa0,address(descriptions[gameplay?3:1]));joined.observeNativeStop(manager,address(stockStop),700100);}
        request.updated=700200;request.size=3;request.reason=erupt?Reason::Eruption:Reason::Result;request.priority=erupt?6:5;request.scoringReaction=!erupt;request.celebration=erupt?1:0;request.resultCheer=erupt?0:1;request.resultCheerUntil=702700;
        joined.tick(address(owner)+0x40,request,700200);
        unsigned expected=initialSize==3&&!nativeStop?0u:1u;
        assert(starts==oldStarts+expected&&stops==oldStops&&joined.playing());
        request.updated=700300;joined.tick(address(owner)+0x40,request,700300);assert(starts==oldStarts+expected&&stops==oldStops);
        request.active=false;request.updated=700400;joined.tick(address(owner)+0x40,request,700400);balanced();
    }
    // A fresh skill swell replaces the supporting voice once with its short
    // attack, even when a lineup swell is already playing. No retrigger loop.
    Runtime skills;skills.configure(image,nullptr,&api,true);
    request={};request.active=request.continuous=request.supporting=true;request.match=980;request.epoch=1;request.reason=Reason::Lineup;request.lineup=1;request.size=2;request.updated=800000;
    skills.tick(address(owner)+0x40,request,800000);oldStarts=starts;
    request.reason=Reason::Result;request.priority=4;request.skillAccent=1;request.reactionId=1;request.supportingOnly=request.motionCompanion=true;request.attackMs=500;request.releaseMs=1550;request.updated=800200;
    skills.tick(address(owner)+0x40,request,800200);assert(starts==oldStarts+1&&messages.back().attack==500);
    request.updated=800300;skills.tick(address(owner)+0x40,request,800300);assert(starts==oldStarts+1);
    request.active=false;request.updated=800400;skills.tick(address(owner)+0x40,request,800400);balanced();
    // A full-size ongoing reaction can hand off to the next live/result
    // reaction without restarting its native attack. A real stop still refreshes.
    for(bool gameplay:{false,true})for(bool nativeStop:{false,true}){
        Runtime continuity;continuity.configure(image,nullptr,&api,gameplay);
        request={};request.active=request.continuous=request.supporting=true;request.match=990;request.epoch=1;request.reason=Reason::Result;request.priority=4;request.reactionId=1;request.size=3;request.updated=900000;
        continuity.tick(address(owner)+0x40,request,900000);oldStarts=starts;
        if(nativeStop){put(address(stockStop)+0xa0,address(descriptions[gameplay?3:1]));continuity.observeNativeStop(manager,address(stockStop),900100);}
        request.reactionId=2;request.resultCheer=2;request.resultCheerUntil=902700;request.updated=900200;
        continuity.tick(address(owner)+0x40,request,900200);assert(starts==oldStarts+(nativeStop?1u:0u));
        request.updated=900300;continuity.tick(address(owner)+0x40,request,900300);assert(starts==oldStarts+(nativeStop?1u:0u));
        request.active=false;request.updated=900400;continuity.tick(address(owner)+0x40,request,900400);balanced();
    }
    // The distance-ramp voice upgrades once at a confirmed peak even when
    // there is no official-result cheer ID yet. Both native channels agree.
    for(bool gameplay:{false,true}){Runtime ramp;ramp.configure(image,nullptr,&api,gameplay);
        Intent gain;gain.match=1000;gain.epoch=1;gain.active=gain.continuous=true;gain.reason=Reason::Result;gain.priority=4;gain.reactionId=1;gain.size=2;gain.attackMs=500;gain.updated=950000;
        ramp.tick(address(owner)+0x40,gain,950000);oldStarts=starts;oldStops=stops;
        gain.earnedReaction=true;gain.size=3;gain.updated=950200;ramp.tick(address(owner)+0x40,gain,950200);
        assert(starts==oldStarts+1&&stops==oldStops&&messages.back().size==3&&messages.back().attack==500);
        gain.updated=950300;ramp.tick(address(owner)+0x40,gain,950300);assert(starts==oldStarts+1);
        gain.active=false;gain.updated=950400;ramp.tick(address(owner)+0x40,gain,950400);balanced();
    }
    // Baseline shares one voice with play reactions. No periodic cheer or
    // per-tick start; size/ownership changes refresh once without a stop gap.
    {Runtime background;background.configure(image,nullptr,&api);
        Intent bed;bed.match=1001;bed.epoch=1;bed.active=bed.continuous=true;bed.reason=Reason::Background;bed.size=1;bed.attackMs=4000;
        auto update=[&](uint64_t now){bed.updated=now;background.tick(address(owner)+0x40,bed,now);balanced();};
        update(1000000);oldStarts=starts;oldStops=stops;
        for(uint64_t now=1000050;now<=1001000;now+=50)update(now);
        assert(starts==oldStarts&&stops==oldStops&&background.playing());
        bed.size=2;update(1001100);assert(starts==oldStarts+1&&messages.back().attack==4000);
        update(1001200);assert(starts==oldStarts+1);
        bed.reason=Reason::Result;bed.priority=4;bed.reactionId=1;bed.earnedReaction=true;bed.size=3;bed.attackMs=500;
        update(1001300);assert(starts==oldStarts+2&&stops==oldStops&&messages.back().attack==500);
        update(1001400);assert(starts==oldStarts+2);
        bed.reason=Reason::Background;bed.priority=0;bed.reactionId=0;bed.earnedReaction=false;bed.size=1;bed.attackMs=4000;
        update(1001500);assert(starts==oldStarts+3&&stops==oldStops&&messages.back().attack==4000);
        put(address(stockStop)+0xa0,address(descriptions[1]));background.observeNativeStop(manager,address(stockStop),1001600);
        update(1001650);assert(starts==oldStarts+4&&messages.back().attack==500);
        update(1001750);assert(starts==oldStarts+4);
        bed.active=false;update(1001850);assert(stops==oldStops+1&&!background.playing());
    }
    assert(!ownedSwellDispatch);
    assert(VirtualFree(allocation,0,MEM_RELEASE));
    std::puts("PASS: native query/weak/event/arena cleanup, tested parameters, bounded playback, reentrancy, stale lease, disable, context replacement, unavailable assets and dispatch faults.");
}
