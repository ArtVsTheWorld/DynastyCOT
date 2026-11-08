#include "../Source/Showboating.h"
#include <cassert>
#include <cstdio>
#include <unordered_map>
using namespace attendance;
std::unordered_map<uintptr_t,unsigned char> memory;
template<class T>void put(uintptr_t p,T value){for(size_t n=0;n<sizeof(T);++n)memory[p+n]=reinterpret_cast<unsigned char*>(&value)[n];}
bool read(uintptr_t p,void* out,size_t size){for(size_t n=0;n<size;++n){auto i=memory.find(p+n);if(i==memory.end())return false;static_cast<unsigned char*>(out)[n]=i->second;}return true;}
struct Case {
    showboating::Model model;energy::Play snap,p;showboating::Sample s{0x100000,0,0x10004,true,{10,20,30},0};
    uintptr_t match=123;unsigned epoch=1;uint64_t now=1000,sequence=1;bool allowed=true,held=true;
    Case(int side=0){p.game={2,14,14,1,300,720};p.down=1;p.offense=side;p.selectedType=103;s.side=side;s.actor=0x10004|(side<<8);snap=p;tick(1);tick(3);}
    showboating::Event tick(int phase){now+=100;p.game.phase=phase;return model.observe(snap,p,match,epoch,sequence,now,allowed,held,s);}
    void animate(){s.tags[0]=40;s.celebration=1;}
};
int main(){
    constexpr uintptr_t image=0x140000000,root=0x110000,control=0x120000,player=0x130000,tag=0x140000,value=0x150000,text=0x160000;
    constexpr unsigned char signature[]{0xb8,0x1f,0,0x40,0,0xc3};for(size_t n=0;n<sizeof(signature);++n)put(image+bindings::ShowboatPlayerType+n,signature[n]);
    put(image+bindings::BallControl,root);put(root+0x10,control);put(control,image+0xb47bb10);put(control+0x1a0,player);put(control+0x168,9);
    put(player,image+bindings::ShowboatPlayerVtable);put(player+0x64,0x80004u);put(player+0x138,0);put(player+0x13c,8);
    put(player+0x1418,tag);put(player+0x1420,uintptr_t(0));put(player+0x1428,uintptr_t(0));
    put(tag,image+bindings::ShowboatAudioTagVtable);put(tag+0x48,value);put(value,image+bindings::ShowboatEnumValueVtable);
    put(value+0x30,text);put(value+0x38,2);constexpr char name[]="PChat_Loco_Celebration";
    for(size_t n=0;n<sizeof(name);++n)put(text+n,name[n]);
    auto native=showboating::snapshot(image,read);assert(native.valid&&native.carrier==player&&native.side==0&&native.actor==0x80004&&native.celebration==1);
    put(control+0x168,1);assert(!showboating::snapshot(image,read).valid);put(control+0x168,9);
    put(text,'X');assert(showboating::snapshot(image,read).celebration==0);put(text,name[0]);
    put(value+0x38,9);assert(showboating::snapshot(image,read).celebration==0);put(value+0x38,2);
    put(player+0x138,1);assert(!showboating::snapshot(image,read).valid);put(player+0x138,0);
    put(player+0x13c,9);assert(!showboating::snapshot(image,read).valid);put(player+0x13c,8);
    put(image+bindings::ShowboatPlayerType,(unsigned char)0x90);assert(!showboating::snapshot(image,read).valid);put(image+bindings::ShowboatPlayerType,signature[0]);
    for(int side:{0,1}){Case c(side);c.animate();auto event=c.tick(3);assert(event.id&&event.side==side);
        auto start=c.model.buildup(c.now);assert(bool(start.id)==(side==0));
        for(int n=0;n<30;++n){assert(!c.tick(3).id);auto buildup=c.model.buildup(c.now);if(side==0)assert(buildup.progress>0&&buildup.progress<=1);}
        if(side==0)assert(c.model.buildup(c.now).progress==1);else assert(!c.model.buildup(c.now).id);
        c.tick(4);assert(!c.model.buildup(c.now).id);
    }
    {Case c;c.animate();c.tick(3);for(int n=0;n<45;++n)c.tick(3);assert(!c.model.buildup(c.now).id);} // Bounded start event, not an endless cached tag.
    {Case c;c.s.celebration=1;c.model.reset();c.tick(1);assert(!c.tick(3).id);} // Old celebration is baseline only.
    {Case c;c.animate();c.allowed=false;assert(!c.tick(3).id);c.allowed=true;assert(!c.tick(3).id);}
    {Case c;c.animate();assert(!c.tick(4).id);assert(!c.tick(3).id);} // Post-play tags never become live events later.
    {Case c;c.animate();c.tick(3);c.s.tags={20,40,30};c.s.celebration=2;assert(!c.tick(3).id);} // Moving an existing tag between channels is not new proof.
    {Case c;c.animate();c.model.suspend();assert(!c.tick(3).id);++c.sequence;c.tick(1);c.tick(3);c.s.tags[0]=50;assert(c.tick(3).id);}
    {Case c;c.held=false;c.tick(3);c.animate();c.held=true;assert(!c.tick(3).id);} // A stale offscreen tag cannot arrive with reacquired possession.
    for(int bad=0;bad<12;++bad){Case c;c.animate();
        if(bad==0)c.held=false;if(bad==1)c.s.valid=false;if(bad==2)c.p.simulated=true;if(bad==3)c.p.resumed=true;
        if(bad==4)c.p.fumble=true;if(bad==5)c.p.offense=1;if(bad==6)c.snap.specialTeams=true;if(bad==7)c.snap.selectedType=37;
        if(bad==8)c.snap.selectedType=38;if(bad==9)c.now+=800;if(bad==10)c.snap.down=-1;if(bad==11)++c.epoch;
        assert(!c.tick(3).id);
    }
    for(int type:{22,23,24}){Case c;c.snap.specialTeams=true;c.snap.selectedType=type;c.snap.offense=1;c.animate();assert(c.tick(3).id);} // Verified receiving-side carrier, not the kicker.
    {Case c;c.snap.offense=1;c.p.interception=true;c.animate();assert(c.tick(3).id);} // Home showboat on an interception return.
    {Case c;c.animate();c.tick(3);c.s.carrier=0x200000;c.tick(3);assert(!c.model.buildup(c.now).id);} // Carrier replacement clears buildup.
    std::puts("PASS: guarded captured carrier/animation asset layout; live fresh home/away proof; slow bounded buildup; stale/postplay/kick/clock/owner/sim/gate cleanup and interception/kick-return support.");
}
