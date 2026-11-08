#include "../Source/CatchKnockout.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <unordered_map>
using namespace attendance;
std::unordered_map<uintptr_t,unsigned char> memory;
template<class T>void put(uintptr_t p,T value){for(size_t n=0;n<sizeof(T);++n)memory[p+n]=reinterpret_cast<unsigned char*>(&value)[n];}
bool read(uintptr_t p,void* out,size_t size){for(size_t n=0;n<size;++n){if(!memory.count(p+n))return false;static_cast<unsigned char*>(out)[n]=memory[p+n];}return true;}
struct Case {
    knockout::Model model;energy::Play p;knockout::Sample sample{1234,true,false};uint64_t now=1000;uintptr_t match=123;unsigned epoch=1;bool allowed=true;
    Case(int down=3,int side=1,bool neutral=false){p.game={2,14,14,1,300,720};p.game.neutral=neutral;p.down=down;p.offense=side;tick(1,0);tick(3,0);}
    live::Outcome tick(int phase,int kind,bool held=false){now+=100;p.game.phase=phase;return model.observe(p,match,epoch,now,allowed,sample,kind,held);}
};
int main(){
    constexpr uintptr_t image=0x140000000,object=0x100000;
    put(image+bindings::CatchSummary,object);put(object,image+bindings::CatchSummaryVtable);put(object+0x175,static_cast<unsigned char>(1));
    assert(knockout::snapshot(image,read).knockout);put(object+0x175,static_cast<unsigned char>(2));assert(!knockout::snapshot(image,read).valid);
    put(object+0x175,static_cast<unsigned char>(1));put(object,uintptr_t{0});assert(!knockout::snapshot(image,read).valid);
    for(int down:{1,2,3,4})for(int side:{0,1})for(bool neutral:{false,true}){
        Case c(down,side,neutral);c.sample.knockout=true;auto out=c.tick(3,4);
        bool expected=down>=3&&(side==1||neutral);assert(bool(out.id)==expected);
        if(expected)assert(out.event==live::Event::CatchKnockout&&out.side==1-side&&live::cheerType(out)==59);
        assert(!c.tick(4,4).id);
    }
    {Case c;c.tick(4,3);c.sample.knockout=true;assert(c.tick(4,4).id);} // Late drop publication.
    for(int kind:{4,5}){Case c;c.sample.knockout=true;assert(c.tick(3,kind).event==live::Event::CatchKnockout);assert(!c.tick(4,kind).id);}
    {Case c;assert(!c.tick(3,5).id);} // Kind 5 alone is only an incompletion.
    {Case c;c.sample.knockout=true;assert(!c.tick(3,3).id);assert(c.tick(4,5).id);} // Captured airborne -> knockout/incomplete order.
    for(int bad=0;bad<10;++bad){Case c;c.sample.knockout=true;int kind=4;bool held=false;
        if(bad==0)c.allowed=false;if(bad==1)c.p.simulated=true;if(bad==2)c.now+=800;if(bad==3)++c.epoch;
        if(bad==4)c.match=456;if(bad==5)c.sample.object=4321;if(bad==6)c.sample.valid=false;
        if(bad==7)c.p.interception=true;if(bad==8)kind=1;if(bad==9)held=true;
        assert(!c.tick(3,kind,held).id);
    }
    {Case c;c.model.reset();c.sample.knockout=true;c.tick(1,0);c.tick(3,0);assert(!c.tick(4,4).id);} // Positive old flag.
    {Case c;assert(!c.tick(3,4).id);} // An ordinary dropped pass is not knockout proof.
    {Case c;c.tick(4,3);c.now+=1100;c.sample.knockout=true;assert(!c.tick(4,4).id);}
    {Case c;c.tick(4,3);for(int n=0;n<20;++n)c.tick(4,3);c.sample.knockout=true;assert(c.tick(4,5).event==live::Event::CatchKnockout);}
    {Case c;c.tick(4,3);for(int n=0;n<51;++n)c.tick(4,3);c.sample.knockout=true;assert(!c.tick(4,5).id);}
    std::puts("PASS: read-only knockout flag validation, third/fourth home defense and neutral scope, fresh clear/drop proof, postplay settling, dedupe and ordinary-drop/score-context/stale/world/sim rejection.");
}
