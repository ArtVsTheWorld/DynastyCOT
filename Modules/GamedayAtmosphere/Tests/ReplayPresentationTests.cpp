#include "../Source/ReplayPresentation.h"
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
Address gate(){return logic(1,{function("257"),function("36"),function("38"),logic(0,{comparison(0,"128",0,2),comparison(5,"22",0,5)}),logic(0,{comparison(0,"128",0,3),comparison(5,"22",0,7)})});}
Address pair(const char* first,const char* second){return logic(0,{function(first),function(second)});}
Address quickOffense(){return logic(0,{function("570"),logic(1,{pair("19","20"),logic(0,{function("30"),comparison(2,"22",0,0)}),pair("60","20"),logic(1,{function("17"),pair("31","30")})})});}
Address tfl(){return logic(0,{function("30"),comparison(2,"22",0,0),comparison(0,"31",4,0)});}
}
int main(){
    attendance::timing::Counts measured;std::atomic<uint64_t> interval{0};
    {attendance::timing::ExclusiveScope<TestClock> work(measured,&interval);
        TestClock::ns+=2000;work.pause();work.pause();TestClock::ns+=100000000;
        work.resume();work.resume();TestClock::ns+=3000;
    }
    assert(measured.calls==1&&measured.maximumUs==5&&!measured.over4ms&&interval==5000);
    Memory arena;memory=&arena;View view(read,types);
    Address passComparison=comparison(5,"22",0,40),pass=logic(0,{function("1"),passComparison});
    assert(view.identify(pass).fullComparison==passComparison);
    Address runComparison=comparison(5,"22",0,35),noFumble=comparison(0,"14",4,0),run=logic(0,{function("0"),noFumble,runComparison});
    assert(view.identify(run).fullComparison==runComparison);
    // A bool constant's unused bytes must not change its meaning.
    arena.set(arena.get<Address>(noFumble+0x18)+8,uint64_t{0xaabbccddeeff0000});
    assert(view.identify(run).fullComparison==runComparison);
    arena.set(noFumble+0x28,1);assert(!view.identify(run).fullComparison);arena.set(noFumble+0x28,0);
    Address passGate=gate(),routinePass=logic(0,{passGate,function("1")});
    Address runGate=gate(),routineRun=logic(0,{runGate,function("0")});
    assert(view.identify(routinePass).insetGate==passGate&&view.identify(routineRun).insetGate==runGate);
    // Same milestones inside the defense's early-PIP selector are not a root match.
    Address defense=logic(0,{function("569"),logic(1,{routinePass,routineRun})});
    assert(!view.identify(defense).insetGate&&!view.identify(defense).fullComparison);
    assert(!view.identify(logic(0,{function("1"),comparison(5,"22",0,25)})).fullComparison);
    assert(!view.identify(logic(0,{function("1"),comparison(4,"22",0,40)})).fullComparison);
    assert(!view.identify(logic(0,{function("0"),comparison(5,"222",0,35)})).fullComparison);
    Address yards=arena.get<Address>(passComparison+8);arena.set(yards+8,19);arena.set(yards+0x10,0);
    assert(!view.fullEligible(passComparison,20));arena.set(yards+8,20);assert(view.fullEligible(passComparison,20));
    arena.set(yards+8,111);assert(!view.fullEligible(passComparison,20));arena.set(yards+8,25);arena.set(yards+0x10,7);assert(!view.fullEligible(passComparison,20));
    assert(!view.fullEligible(passComparison,0)&&!view.fullEligible(passComparison,61));
    arena.set(yards+0x10,0);for(int gain:{11,12,13}){arena.set(yards+8,gain);assert(view.fullEligible(passComparison,12)==(gain>=12));}
    Address result=arena.allocate(0x10);arena.set(result+8,4);arena.set(result,uint64_t{0xaabbccddeeff0000});
    assert(view.falseBoolean(result));arena.set(result,1);assert(!view.falseBoolean(result));
    assert(view.trueBoolean(result));arena.set(result,2);assert(!view.trueBoolean(result));
    arena.set(result,0);arena.set(result+8,0);assert(!view.falseBoolean(result));
    assert(!view.falseBoolean(0)&&!view.falseBoolean(UINTPTR_MAX));
    ThreadState state;
    {Scope root(state,view,routinePass,true,true,true);assert(state.depth==1&&state.selection.insetGate==passGate);
        {Scope child(state,view,passGate,false,false,false);assert(state.depth==2&&state.selection.insetGate==passGate);}
        assert(state.depth==1&&state.selection.insetGate==passGate);
    }
    assert(state.depth==0&&!state.selection.insetGate&&!state.selection.fullComparison);
    {Scope root(state,view,defense,true,true,true);{Scope child(state,view,routinePass,true,true,true);assert(!state.selection.insetGate);}}
    {Scope root(state,view,pass,false,true,true);assert(!state.selection.fullComparison);}
    {Scope root(state,view,pass,true,false,true);assert(!state.selection.fullComparison);}
    // Model the real Eval/GetValue contract: nested Eval returns validity, its
    // parent consumes the cached bool, and a comparison returns a Variant ptr.
    arena.set(passGate+0x38,0);arena.set(passGate+0x40,4);
    {Scope root(state,view,routinePass,true,true,true);
        {Scope nested(state,view,passGate,false,true,true);
            bool nativeValid=true;
            if(nativeValid&&state.selection.insetGate==passGate&&view.falseBoolean(passGate+0x38))arena.set(passGate+0x38,static_cast<unsigned char>(1));
        }
        assert(arena.get<unsigned char>(passGate+0x38)==1&&arena.get<int>(passGate+0x40)==4);
    }
    arena.set(yards+8,20);arena.set(yards+0x10,0);arena.set(result,0);arena.set(result+8,4);
    {Scope root(state,view,pass,true,true,true);
        if(state.selection.fullComparison==passComparison&&view.falseBoolean(result)&&view.fullEligible(passComparison,20))arena.set(result,static_cast<unsigned char>(1));
    }
    assert(arena.get<unsigned char>(result)==1&&arena.get<int>(yards+8)==20&&arena.get<int>(arena.get<Address>(passComparison+0x18)+8)==40);
    // No selection is retained between roots, disabled or otherwise.
    {Scope root(state,view,routineRun,true,true,false);assert(!state.selection.insetGate);}
    assert(state.depth==0&&!state.selection.fullComparison&&!state.selection.insetGate);
    // The defensive shortcuts are complete root selectors, not generic sack,
    // turnover, negative-yardage or safety facts. Native turnover roots stay native.
    Address quick=quickOffense(),loss=tfl();
    assert(view.identify(quick).denyRoot==quick&&view.identify(quick).defense==DefenseRoute::QuickOffense);
    assert(view.identify(loss).denyRoot==loss&&view.identify(loss).defense==DefenseRoute::TackleForLoss);
    assert(!view.identify(logic(0,{function("18")})).denyRoot&&!view.identify(logic(0,{function("14")})).denyRoot);
    assert(!view.identify(logic(0,{function("16"),logic(0,{comparison(0,"1042",4,0)})})).denyRoot);
    assert(!view.identify(logic(0,{function("30"),comparison(3,"22",0,0),comparison(0,"31",4,0)})).denyRoot);
    assert(!view.identify(logic(0,{function("30"),comparison(2,"22",0,0),comparison(0,"31",4,1)})).denyRoot);
    assert(!view.identify(logic(0,{function("569"),logic(1,{pair("19","20"),logic(0,{function("30"),comparison(2,"22",0,0)}),pair("60","20"),logic(1,{function("17"),pair("31","30")})})})).denyRoot);
    {Scope root(state,view,quick,true,true,true,false);assert(!state.selection.denyRoot);}
    {Scope root(state,view,loss,true,true,false,true);assert(!state.selection.denyRoot);}
    {Scope root(state,view,quick,true,false,false,true);assert(state.selection.denyRoot==quick);}
    {Scope root(state,view,loss,false,true,true,true);assert(!state.selection.denyRoot);}
    arena.set(loss+0x38,1);arena.set(loss+0x40,4);
    {Scope root(state,view,loss,true,true,true,true);
        assert(state.selection.denyRoot==loss);
        if(view.trueBoolean(loss+0x38))arena.set(loss+0x38,static_cast<unsigned char>(0));
        // Only selector output changed; the original negative-yardage and
        // non-safety conditions remain identical for the next native evaluation.
        assert(view.falseBoolean(loss+0x38)&&view.identify(loss).denyRoot==loss);
    }
    {Scope root(state,view,quick,true,true,true,true);{Scope nested(state,view,loss,true,true,true,true);assert(state.selection.denyRoot==quick);}}
    assert(!state.depth&&!state.selection.denyRoot);
    arena.set(yards+8,14);arena.set(yards+0x10,0);assert(!view.fullEligible(passComparison,15));
    arena.set(yards+8,15);assert(view.fullEligible(passComparison,15)&&!view.fullEligible(passComparison,20));
    // Major routes must match the complete authored shape; cached padding is ignored.
    auto tdRoot=logic(0,{function("27")});assert(view.major(tdRoot)==MajorRoute::Touchdown);
    assert(view.major(logic(1,{function("27")}))==MajorRoute::None);
    assert(view.major(logic(0,{function("270")}))==MajorRoute::None);
    assert(view.major(logic(0,{comparison(0,"556",4,1)}))==MajorRoute::QuickSkip);
    assert(view.major(logic(0,{comparison(0,"556",4,0)}))==MajorRoute::None);
    auto challenge=comparison(0,"1042",4,0),takeaway=logic(0,{function("16"),logic(0,{challenge})});
    assert(view.major(takeaway)==MajorRoute::Takeaway&&!view.challengeExcluded(takeaway));
    auto challengeFn=arena.get<Address>(challenge+8);arena.set(challengeFn+0x10,4);arena.set(challengeFn+8,uint64_t{0xaabbccddeeff0000});
    assert(view.challengeExcluded(takeaway));arena.set(challengeFn+8,1);assert(!view.challengeExcluded(takeaway));
    auto quickTd=logic(0,{logic(1,{comparison(4,"178",0,19),comparison(5,"540",0,20),comparison(5,"540",0,20),comparison(5,"540",0,20),comparison(5,"540",0,20),comparison(5,"540",0,20),comparison(5,"540",0,20),comparison(5,"540",0,20),comparison(5,"540",0,20)}),comparison(0,"157",4,0),comparison(0,"556",4,1)});
    assert(view.major(quickTd)==MajorRoute::QuickTouchdown);
    arena.set(quickTd+0x48,1);assert(view.major(quickTd)==MajorRoute::None);
    // Bad pointers, counts and node types fail closed.
    assert(!view.identify(0).insetGate&&!view.identify(UINTPTR_MAX).fullComparison);
    arena.set(passGate+0x10,UINTPTR_MAX);assert(!view.identify(routinePass).insetGate);
    std::cout<<"Replay presentation: offensive cutoffs, defensive routing, no false turnovers, native facts and scoped restore checks passed\n";
}
