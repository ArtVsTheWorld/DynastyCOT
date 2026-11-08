#include "../Source/NativeSkillSignals.h"
#include <unordered_map>
#include <cassert>
#include <cstdio>
using namespace attendance;
using A=uintptr_t;
std::unordered_map<A,unsigned char> memory;
bool read(A p,void* out,size_t n){for(size_t i=0;i<n;++i)if(!memory.count(p+i))return false;for(size_t i=0;i<n;++i)static_cast<unsigned char*>(out)[i]=memory[p+i];return true;}
template<class T>void put(A p,T v){for(size_t i=0;i<sizeof(T);++i)memory[p+i]=reinterpret_cast<unsigned char*>(&v)[i];}
struct Test {
    skills::Model m;skills::Sample sample;energy::Play p;uint64_t now=1000;A match=123;unsigned epoch=1;bool allowed=true,held=true;
    Test(){p.game={2,14,14,1,300,720};p.down=1;p.offense=0;sample.timesValid=sample.eventsValid=true;tick();p.game.phase=3;now+=100;tick();}
    live::Outcome tick(){return m.observe(p,match,epoch,now,allowed,held,sample);}
    void advance(unsigned n){while(n){unsigned step=(std::min)(n,100u);now+=step;tick();n-=step;}}
};
int main(){
    // Independent read-only fixture for both component and actor/event chains.
    constexpr A image=0x140000000,typeBuckets=0x100000,node=0x110000,type=0x120000,storage=0x130000,buckets=0x140000,entityNode=0x150000,component=0x160000,root=0x170000,actors=0x180000,actorBuckets=0x190000,actorNode=0x200000,actorArray=0x210000,actor=0x220000,data=0x230000;
    put(image+skills::TypeCount,1u);put(image+skills::TypeBuckets,typeBuckets);put(typeBuckets,node);put(node,0x3784eae8u);put(node+8,type);
    put(type+0xc8,storage);put(storage+0x20,A(1));put(storage+0x38,1u);put(storage+0x30,buckets);put(buckets,entityNode);put(entityNode,0x4f00000000000000ull);put(entityNode+8,component);
    for(unsigned i=0;i<6;++i)put(component+0x1640+i*8,double(i));
    put(image+skills::EventRoot,root);put(root,image+0xbc90128);for(unsigned i=0;i<45*32;++i)put(root+0x10+i,(unsigned char)0);
    put(root+0x10+16,50.0);put(root+0x10+24,52);put(root+0x10+28,0x30004u);
    put(image+skills::ActorRoot,actors);put(actors+0x170,actorBuckets);put(actors+0x178,1u);put(actors+0x28,actorArray);put(actors+0x30,actorArray+8);put(actorBuckets,actorNode);put(actorNode,0x30004u);put(actorNode+4,0);put(actorArray,actor);put(actor+0x18,data);put(data+0x44,(unsigned char)1);put(data+0x304,0);put(data+0x308,3);put(data+0x310,500);
    skills::View v(image,read);auto s=v.snapshot();assert(s.timesValid&&s.times[2]==2&&s.eventsValid&&s.events[0].side==0&&s.events[0].type==52);
    std::array<double,4> seen{0,0,50,0};assert(v.snapshot(seen).events[0].side==-1); // Old records need no actor lookup.
    put(data+0x308,4);assert(v.snapshot().events[0].side==-1);put(data+0x308,3);
    put(actors+0x178,100001u);assert(v.snapshot().events[0].side==-1);put(actors+0x178,1u);
    put(root,A(0));assert(!v.snapshot().eventsValid);put(root,image+0xbc90128);
    put<double>(component+0x1650,NAN);assert(!v.snapshot().timesValid);put(component+0x1650,2.0);
    put(image+skills::TypeCount,100001u);assert(!v.snapshot().timesValid);put(image+skills::TypeCount,1u);
    constexpr live::Event events[]{live::Event::Truck,live::Event::None,live::Event::StiffArm,live::Event::Juke,live::Event::Spin,live::Event::Hurdle};
    for(unsigned i=0;i<6;++i){Test t;t.now+=100;t.sample.times[i]=10;auto a=t.tick();assert(bool(a.id)==(i!=1));if(a.id)assert(a.event==events[i]);assert(!t.tick().id);}
    for(int kind:{31,35,52,54})for(int side:{0,1})for(bool neutral:{false,true}){Test t;t.p.game.neutral=neutral;t.now+=100;t.sample.events[0]={kind,side,10,4};auto a=t.tick();assert(bool(a.id)==(side==0||neutral));if(a.id)assert(a.event==(kind==31?live::Event::BrokenTackle:live::Event::HitStick));}
    for(int bad=0;bad<7;++bad){Test t;t.now+=100;t.sample.times[2]=10;
        if(bad==0)t.allowed=false;if(bad==1)t.held=false;if(bad==2)t.p.offense=1;if(bad==3)t.p.simulated=true;if(bad==4)t.epoch=2;if(bad==5)t.match=456;if(bad==6)t.now+=900;
        assert(!t.tick().id);}
    // New event storage appearing mid-play is baseline-only, not an old cheer.
    {Test t;t.m.reset();t.sample.eventsValid=false;t.p.game.phase=1;t.tick();t.p.game.phase=3;t.now+=100;t.tick();t.sample.eventsValid=true;t.sample.events[0]={52,0,80,4};t.now+=100;assert(!t.tick().id);t.sample.events[0].time=81;t.now+=100;assert(t.tick().id);}
    // Three spaced confirmed accents maximum; close records are consumed, not queued.
    {Test t;for(int i=0;i<5;++i){t.advance(900);t.sample.times[2]+=1;auto a=t.tick();assert(bool(a.id)==(i<3));t.sample.events[0]={31,0,double(20+i),4};assert(!t.tick().id);}}
    // Success + broken-tackle evidence on the same collision becomes one accent.
    {Test t;t.now+=100;t.sample.times[2]=10;t.sample.events[0]={31,0,20,4};assert(t.tick().event==live::Event::StiffArm);assert(!t.tick().id);}
    // The captured credited hurdle count settles 0.27s after the whistle,
    // without a fake-out timestamp. An old positive count cannot replay it.
    {Test t;t.m.reset();t.p.game.phase=1;t.sample.hurdleCount=0;t.tick();t.p.game.phase=3;t.now+=100;t.tick();
        t.p.game.phase=4;t.held=false;t.now+=100;t.tick();t.now+=275;t.sample.hurdleCount=1;
        assert(t.tick().event==live::Event::Hurdle);t.now+=100;assert(!t.tick().id);}
    for(int bad=0;bad<5;++bad){Test t;t.m.reset();t.p.game.phase=1;t.sample.hurdleCount=bad==0?1:0;t.tick();t.p.game.phase=3;t.now+=100;t.tick();
        t.p.game.phase=4;t.held=false;t.now+=100;t.tick();t.sample.hurdleCount=1;t.now+=275;
        if(bad==1)t.allowed=false;if(bad==2)t.p.offense=1;if(bad==3)t.now+=800;if(bad==4)t.match=456;
        assert(!t.tick().id);}
    // A fake-out timestamp followed by its credited count remains one hurdle.
    {Test t;t.sample.hurdleCount=0;t.now+=100;t.sample.times[5]=10;assert(t.tick().event==live::Event::Hurdle);
        t.p.game.phase=4;t.now+=100;t.tick();t.sample.hurdleCount=1;t.now+=275;assert(!t.tick().id);}
    std::puts("PASS: bounded read-only skill/actor mapping; success timestamps and fresh late credited hurdles; landed hits/broken tackles; side/neutral gates; attempt/stale/world/gap rejection; collision dedupe and three spaced accents per play.");
}
