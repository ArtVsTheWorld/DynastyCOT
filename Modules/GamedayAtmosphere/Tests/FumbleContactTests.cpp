#include "../Source/LiveFumble.h"
#include <cassert>
#include <cstdio>
#include <unordered_map>
using namespace attendance;
using A=uintptr_t;
std::unordered_map<A,unsigned char> memory;
template<class T>void put(A p,T value){for(size_t n=0;n<sizeof(T);++n)memory[p+n]=reinterpret_cast<unsigned char*>(&value)[n];}
bool torn=false;
bool read(A p,void* out,size_t size){for(size_t n=0;n<size;++n){if(!memory.count(p+n))return false;static_cast<unsigned char*>(out)[n]=memory[p+n];}if(torn&&size==32)static_cast<unsigned char*>(out)[16]^=1;return true;}
struct Case {
    fumblecontact::Model detector;live::Fumbles reactions;energy::Play p;fumblecontact::Sample sample{999,true,0,0,-1,-1};
    A match=123;unsigned epoch=1;uint64_t now=1000;bool allowed=true;int ball=2;bool held=true,fumble=false;live::ForcedEvent native{777,0,true};
    fumblecontact::Proof proof;
    Case(int owner=1,bool neutral=false,int down=1){p.game={2,14,14,1,300,720};p.game.neutral=neutral;p.offense=owner;p.down=down;tick(1);tick(3);}
    live::Outcome tick(int phase){now+=50;p.game.phase=phase;proof=detector.observe(p,match,epoch,now,allowed,sample,ball,held,fumble);return reactions.observe(p,match,epoch,now,allowed,native,0,ball,held,fumble,proof);}
    void collision(){sample={999,true,508.8,508.8,p.offense,1-p.offense};ball=0;held=false;assert(!tick(3).id);}
    void wait(unsigned ms,int phase=3){for(unsigned n=0;n<ms;n+=50)assert(!tick(phase).id);}
    live::Outcome confirmed(int phase=3,bool recovered=true){fumble=true;ball=5;held=recovered;return tick(phase);}
};
int main(){
    // Captured native contact records: 8 = home carrier 0x4,
    // 9 = away contact player 0x50104, both at native time 508.8.
    constexpr A image=0x140000000,root=0x100000,actors=0x110000,buckets=0x120000,nodes=0x130000,array=0x140000,actor=0x150000,data=0x160000;
    put(image+skills::EventRoot,root);put(root,image+0xbc90128);for(unsigned n=0;n<45*32;++n)put(root+16+n,(unsigned char)0);
    put(image+skills::ActorRoot,actors);put(actors+0x170,buckets);put(actors+0x178,1u);put(actors+0x28,array);put(actors+0x30,array+16);put(buckets,nodes);
    for(unsigned n=0;n<2;++n){A node=nodes+n*0x40,object=actor+n*0x100,player=data+n*0x400;uint32_t key=n?0x50104u:4u;
        put(node,key);put(node+4,static_cast<int>(n));put(node+8,n?A(0):nodes+0x40);put(array+n*8,object);put(object+0x18,player);
        put(player+0x44,(unsigned char)1);put(player+0x304,static_cast<int>(n));put(player+0x308,n?5:0);put(player+0x310,100+static_cast<int>(n));
        put(root+16+n*32+16,508.8);put(root+16+n*32+24,n?9:8);put(root+16+n*32+28,key);
    }
    auto sample=fumblecontact::snapshot(image,read);assert(sample.valid&&sample.took==508.8&&sample.made==508.8&&sample.carrier==0&&sample.defender==1);
    assert(fumblecontact::snapshot(image,read,509).carrier==-1);torn=true;assert(!fumblecontact::snapshot(image,read).valid);torn=false;
    put(root,A(0));assert(!fumblecontact::snapshot(image,read).valid);put(root,image+0xbc90128);
    // Mirrored home/away cases: the away defender caused the captured home
    // fumble, so that case must prove ownership without a positive home shake.
    for(int owner:{0,1})for(bool neutral:{false,true})for(int down:{0,1,2,3,4})for(int phase:{3,4,5}){
        Case c(owner,neutral,down);c.collision();c.wait(850);auto out=c.confirmed(phase);
        assert(c.proof.id&&c.proof.owner==owner);assert(bool(out.id)==(owner==1||neutral));
        if(out.id)assert(out.event==live::Event::ForcedFumble&&out.side==1-owner&&live::cameraReaction(out.event));
        assert(!c.confirmed(phase).id);
    }
    // Home recovery of the opponent's loose ball cannot steal the original
    // fumbler's ownership. The impact represents the earlier contact.
    {Case c;c.collision();c.wait(850);c.p.offense=0;auto out=c.confirmed();assert(out.event==live::Event::ForcedFumble&&out.side==0&&c.proof.owner==1);}
    {Case c;c.collision();c.tick(4);c.wait(200,4);assert(c.confirmed(5).event==live::Event::ForcedFumble);} // Late authoritative fumble state.
    // Native and fallback proofs share the same one-force-per-play latch.
    {Case c;c.native.type=32;c.held=false;c.ball=0;assert(c.tick(3).event==live::Event::ForcedFumble);c.sample={999,true,508.8,508.8,1,0};c.tick(3);assert(!c.confirmed().id);}
    {Case c;c.collision();assert(c.confirmed().event==live::Event::ForcedFumble);c.native.type=32;assert(!c.tick(3).id);}
    // Ordinary hits can release a carrier at the whistle. No fresh native
    // fumble state means no force reaction, even with contact and loose ball.
    {Case c;c.collision();c.wait(1400);assert(!c.tick(4).id&&!c.proof.id);}
    {Case c;c.held=false;c.ball=0;c.tick(3);assert(!c.confirmed().id&&!c.proof.id);} // Unforced drop.
    {Case c;assert(!c.confirmed().id&&!c.proof.id);} // Recovery/old flag alone.
    {Case c;c.sample={999,true,508.8,508.8,1,0};c.tick(3);assert(!c.confirmed().id&&!c.proof.id);} // Contact but no loose ball.
    for(int bad=0;bad<12;++bad){Case c;c.collision();
        if(bad==0)c.allowed=false;if(bad==1)c.p.simulated=true;if(bad==2)c.p.resumed=true;if(bad==3)c.now+=800;
        if(bad==4)++c.epoch;if(bad==5)++c.match;if(bad==6)c.wait(1550);
        if(bad==7){c.sample.object=1001;c.tick(3);}if(bad==8){c.detector.reset();}
        if(bad==9)c.wait(800,4);if(bad==10)c.p.game.phase=7;
        int phase=bad==10?7:3;if(bad==11)phase=8;
        auto rejected=c.confirmed(phase);if(rejected.id||c.proof.id)std::fprintf(stderr,"Invalid contact case %d emitted proof %llu\n",bad,static_cast<unsigned long long>(c.proof.id));assert(!rejected.id&&!c.proof.id);
    }
    // Never attribute an airborne pass/knockout to the quarterback who last
    // held the ball seconds earlier. Grounded carrier proof must be recent.
    {Case c;c.held=false;c.ball=1;c.wait(500);c.collision();assert(!c.confirmed().id&&!c.proof.id);}
    for(int bad=0;bad<3;++bad){Case c;c.sample={999,true,508.8,508.8,bad==0?0:1,bad==1?1:0};
        if(bad==2)c.sample.valid=false;c.ball=0;c.held=false;c.tick(3);assert(!c.confirmed().id&&!c.proof.id);}
    // Matching contact records may publish in adjacent frames.
    {Case c;c.sample={999,true,508.8,0,1,-1};c.ball=0;c.held=false;c.tick(3);c.sample={999,true,508.8,508.8,1,0};c.tick(3);assert(c.confirmed().event==live::Event::ForcedFumble);}
    // Nearby earlier contact cannot consume a half-published newer pair.
    {Case c;c.sample={999,true,508.766,508.766,1,1};c.tick(3);
        c.sample={999,true,508.8,508.766,1,1};c.ball=0;c.held=false;c.tick(3);
        c.sample={999,true,508.8,508.8,1,0};c.tick(3);assert(c.confirmed().event==live::Event::ForcedFumble);}
    {Case c;c.collision();c.p.game.homeScore+=6;assert(!c.confirmed(4).id&&!c.proof.id);}
    // A delayed force fact can enrich a nonscoring result, but never relabel
    // a touchdown or the other play's accepted result.
    for(int points:{0,6}){energy::Outcome result;result.accepted=result.completed=true;result.sequence=1;
        result.homePoints=points;result.event=points?energy::Event::HomeScore:energy::Event::ThirdStop;
        live::noteForcedResult(result,2,{live::Event::ForcedFumble,1,0,0});assert(!result.forcedFumble);
        live::noteForcedResult(result,1,{live::Event::ForcedFumble,1,0,0});assert(result.forcedFumble==(points==0));
        assert(result.event==(points?energy::Event::HomeScore:energy::Event::ForcedFumble));}
    std::puts("PASS: captured delayed fumble state after opposing contact/loose ball; home/away/neutral ownership; all downs; late whistle confirmation; own-pickup and native/fallback dedupe; ordinary tackle/drop/recovery/airborne/stale/world/sim rejection.");
}
