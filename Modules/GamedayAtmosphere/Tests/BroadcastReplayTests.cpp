#include "../Source/BroadcastReplay.h"
#include "../Source/ReplayRouteAudit.h"
#include "../Source/LogPolicy.h"
#include "../Source/CallbackTiming.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <initializer_list>
using namespace attendance::replaypresentation;
namespace {
struct TestClock {
    using duration=std::chrono::nanoseconds;using time_point=std::chrono::time_point<TestClock>;
    static int64_t ns;static time_point now(){return time_point(duration(ns));}
};
int64_t TestClock::ns=0;
struct Memory {
    std::vector<unsigned char> bytes=std::vector<unsigned char>(0x20000);
    Address next=0x10000;
    Address allocate(size_t count){Address address=next;next+=count;assert(next<0x30000);return address;}
    template<class T>void set(Address address,T value){std::memcpy(bytes.data()+address-0x10000,&value,sizeof(value));}
    template<class T>T get(Address address)const{T value{};std::memcpy(&value,bytes.data()+address-0x10000,sizeof(value));return value;}
};
Memory* memory=nullptr;
bool read(Address address,void* out,size_t count){if(!memory||address<0x10000||address>=0x30000||count>0x30000-address)return false;std::memcpy(out,memory->bytes.data()+address-0x10000,count);return true;}
const Types types{0x100,0x200,0x300,0x400,0x500};
Address function(const char* id){Address node=memory->allocate(0x50);memory->set(node,types.function);std::memcpy(memory->bytes.data()+node-0x10000+0x48,id,std::strlen(id)+1);return node;}
Address value(int type,int number){Address node=memory->allocate(0x18);memory->set(node,types.constant);memory->set(node+8,number);memory->set(node+0x10,type);return node;}
Address comparison(int op,const char* id,int type,int number){Address node=memory->allocate(0x30);memory->set(node,types.comparison);memory->set(node+8,function(id));memory->set(node+0x18,value(type,number));memory->set(node+0x28,op);return node;}
Address logic(int op,std::initializer_list<Address> children){Address node=memory->allocate(0x50),entries=memory->allocate(children.size()*16);memory->set(node,types.logic);memory->set(node+8,entries);memory->set(node+0x10,entries+children.size()*16);memory->set(node+0x48,op);size_t index=0;for(Address child:children)memory->set(entries+16*index++,child);return node;}
}
int main(){
    Memory arena;memory=&arena;View view(read,types);
    auto path=[&](std::initializer_list<std::pair<const char*,int>> list){
        std::vector<Address> c;for(auto x:list)c.push_back(comparison(0,x.first,4,x.second));
        Address node=arena.allocate(0x50),entries=arena.allocate(c.size()*16);arena.set(node,types.logic);arena.set(node+8,entries);arena.set(node+0x10,entries+c.size()*16);arena.set(node+0x48,0);
        for(size_t i=0;i<c.size();++i)arena.set(entries+i*16,c[i]);return node;
    };
    Address td=comparison(0,"27",4,0),fg=comparison(0,"29",4,0),yes=value(4,1);
    Address guard=logic(0,{td,fg,yes});
    Address cases=logic(1,{
        path({{"14",1}}),path({{"14",0},{"18",1}}),path({{"14",0},{"18",0},{"17",1}}),
        path({{"14",0},{"18",0},{"17",0},{"1",1},{"20",1}}),
        path({{"14",0},{"18",0},{"17",0},{"20",0},{"1",1},{"21",1}}),
        path({{"14",0},{"18",0},{"17",0},{"20",0},{"21",0},{"0",1}})});
    Address root=logic(0,{guard,cases});
    using namespace attendance::broadcastreplay;
    assert(handoff(view,root));
    arena.set(yes+8,uint64_t{0xaabbccddeeff0001});assert(handoff(view,root));
    assert(!handoff(view,guard)&&!handoff(view,cases)&&!handoff(view,0)&&!handoff(view,UINTPTR_MAX));
    arena.set(root+0x48,1);assert(!handoff(view,root));arena.set(root+0x48,0);
    arena.set(cases+0x48,0);assert(!handoff(view,root));arena.set(cases+0x48,1);
    arena.set(yes+8,0);assert(!handoff(view,root));arena.set(yes+8,1);
    for(Address node:{td,fg}){
        Address val=arena.get<Address>(node+0x18);arena.set(val+8,1);assert(!handoff(view,root));arena.set(val+8,0);
    }
    // Every branch and leaf is part of ownership, including unused branches.
    auto entries=arena.get<Address>(cases+8);
    for(unsigned i=0;i<6;++i){
        Address branch=arena.get<Address>(entries+i*16),items=arena.get<Address>(branch+8),end=arena.get<Address>(branch+0x10);
        arena.set(branch+0x10,end+16);assert(!handoff(view,root));arena.set(branch+0x10,end);
        for(Address at=items;at<end;at+=16){
            auto cmp=arena.get<Address>(at),fn=arena.get<Address>(cmp+8),val=arena.get<Address>(cmp+0x18);
            auto old=arena.get<int>(val+8);arena.set(val+8,1-old);assert(!handoff(view,root));arena.set(val+8,old);
            auto first=arena.get<unsigned char>(fn+0x48);arena.set(fn+0x48,static_cast<unsigned char>('9'));assert(!handoff(view,root));arena.set(fn+0x48,first);
        }
    }
    auto routine=[](){return logic(1,{function("257"),function("36"),function("38"),logic(0,{comparison(0,"128",0,2),comparison(5,"22",0,5)}),logic(0,{comparison(0,"128",0,3),comparison(5,"22",0,7)})});};
    auto passShortcut=logic(0,{routine(),function("1")}),runShortcut=logic(0,{routine(),function("0")});
    auto defense=logic(0,{function("569"),logic(1,{passShortcut,runShortcut})});
    assert(defensiveInset(view,defense)&&!defensiveInset(view,root)&&!defensiveInset(view,passShortcut)&&!defensiveInset(view,runShortcut));
    auto defenseFn=arena.get<Address>(arena.get<Address>(defense+8));arena.set(defenseFn+0x48,static_cast<unsigned char>('8'));
    assert(!defensiveInset(view,defense));arena.set(defenseFn+0x48,static_cast<unsigned char>('5'));assert(defensiveInset(view,defense));
    auto nativeRoutine=arena.get<Address>(arena.get<Address>(passShortcut+8));arena.set(nativeRoutine+0x48,0);assert(!defensiveInset(view,defense));arena.set(nativeRoutine+0x48,1);
    assert(defensiveInset(view,defense));
    attendance::replayrouteaudit::Gate logGate;attendance::replayrouteaudit::Record logRow;
    assert(!logGate.accept(logRow));logRow.match=100;logRow.sequence=1;logRow.route=1;assert(logGate.accept(logRow));
    for(int i=0;i<1000;++i)assert(!logGate.accept(logRow));
    for(int i=1;i<16;++i){logRow.yards=i;assert(logGate.accept(logRow));}
    logRow.yards=100;assert(!logGate.accept(logRow));++logRow.sequence;assert(logGate.accept(logRow));
    ++logRow.epoch;assert(logGate.accept(logRow));++logRow.match;assert(logGate.accept(logRow));
    assert(attendance::importantLogTag("replay-route","")&&attendance::importantLogTag("visitor-demand-components",""));
    attendance::energy::Tracker t;t.serial=1;t.resultUsed=true;t.startStamp=10;
    t.start.down=2;t.start.offense=0;t.start.toGoal=70;t.start.selectedType=1;
    attendance::energy::Play result=t.start;result.resultStamp=11;result.toGoal=55;
    assert(exclusions(t,result,false,15)==Pending);
    assert(eligible(t,result,true,15)&&!eligible(t,result,true,16)&&!eligible(t,result,false,15));
    for(int gain:{11,12,13}){auto boundary=result;boundary.toGoal=t.start.toGoal-gain;assert(eligible(t,boundary,true,12)==(gain>=12));}
    for(int minimum:{0,9,61})assert(!eligible(t,result,true,minimum));
    result.toGoal=56;assert(!eligible(t,result,true,15));result.toGoal=55;
    for(int test=0;test<19;++test){auto bad=t;auto r=result;
        switch(test){
        case 0:bad.serial=0;break;case 1:bad.resultUsed=false;break;case 2:bad.start.down=0;break;
        case 3:r.down=0;break;case 4:r.offense=1;break;case 5:bad.start.selectedType=37;break;
        case 6:bad.start.selectedType=38;break;case 7:bad.start.specialTeams=true;break;case 8:r.specialTeams=true;break;
        case 9:bad.start.simulated=true;break;case 10:r.simulated=true;break;case 11:r.resumed=true;break;
        case 12:bad.sawPick=true;break;case 13:bad.sawFumble=true;break;case 14:r.interception=true;break;
        case 15:r.fumble=true;break;case 16:r.resultStamp=10;break;case 17:r.game.homeScore=6;break;case 18:r.game.awayScore=6;break;
        }
        assert(!eligible(bad,r,true,15));
    }
    result.offense=t.start.offense=1;assert(eligible(t,result,true,15));
    result.toGoal=-1;t.start.toGoal=-1;assert(!eligible(t,result,true,15));
    t.start.fieldSpot=20;t.start.attackDirection=1;result.fieldSpot=35;assert(eligible(t,result,true,15));
    t.start.attackDirection=-1;result.fieldSpot=5;assert(eligible(t,result,true,15));
    std::cout<<"PASS: exact Broadcast Universe handoff shape, complete ownership/rejection, exact defensive inset shortcut, bounded per-snap diagnostics, same-snap gain threshold, TD/turnover/special-team/simulation exclusions and native-field direction checks\n";
}
