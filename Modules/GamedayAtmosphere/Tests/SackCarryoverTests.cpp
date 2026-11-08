#include "../Source/LiveReactions.h"
#include "../Source/NativeSackFacts.h"
#include <cassert>
#include <cstdio>
#include <map>
#include <vector>
using namespace attendance;
std::map<uintptr_t,std::vector<unsigned char>> memory;
bool read(uintptr_t address,void* output,size_t size){auto at=memory.find(address);if(at==memory.end()||at->second.size()<size)return false;std::memcpy(output,at->second.data(),size);return true;}
template<class T>void put(uintptr_t address,T value){auto& bytes=memory[address];bytes.resize(sizeof(value));std::memcpy(bytes.data(),&value,sizeof(value));}
energy::Play snap(int side,int type=206){
    energy::Play p;p.game={1,0,0,1,495,720};p.game.neutral=true;p.offense=side;p.down=3;
    p.fieldSpot=-20;p.attackDirection=1;p.toFirst=13;p.toGoal=70;p.selectedType=type;p.resultStamp=10;
    p.sackSampled=true;return p;
}
int main(){
    unsigned cases=0;
    for(bool strip:{false,true})for(bool reset:{false,true})for(bool sampled:{false,true})
    for(int phase:{1,4,5})for(int side:{0,1})for(float loss:{-2.f,-6.f}){
        energy::Settings rules;energy::Tracker tracker;auto p=snap(side,103);
        tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);
        p.game.phase=4;p.game.seconds=490;p.resultStamp=11;p.sack=true;p.fumble=p.forcedFumble=strip;
        if(strip){p.offense=1-side;p.down=1;}else p.down=4;
        auto first=tracker.observe(rules,p);assert(first.completed&&(strip?first.takeaway:first.sack));
        // The next offense selects a new ordinary run, but the presentation's
        // sack predicate from the previous play is still true at its snap.
        p.game.phase=1;p.game.seconds=488;p.offense=strip?1-side:side;p.down=1;
        p.fieldSpot=-20;p.toFirst=10;p.toGoal=70;p.selectedType=206;p.fumble=p.forcedFumble=false;
        tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);
        p.sack=!reset;p.sackSampled=sampled;tracker.observe(rules,p);
        p.game.phase=phase;p.game.seconds=480;p.down=2;p.fieldSpot+=loss;p.toGoal-=loss;p.toFirst-=loss;
        p.resultStamp=12;p.sack=true;p.sackSampled=true;
        auto second=tracker.observe(rules,p);assert(second.completed&&second.yards==loss);
        assert(second.sack==(reset&&sampled));
        assert(second.fiveYardLoss==(loss<=-5));
        if(!reset||!sampled){
            assert(second.event!=energy::Event::Sack&&!tracker.sawSack);
            if(loss==-2)assert(!live::completedImpact(second,p.game,true,0,83).id);
        }
        ++cases;
    }
    // A fresh first sack remains eligible. Clearing a carried predicate at a
    // later postplay phase can also establish genuine delayed native proof.
    energy::Settings rules;energy::Tracker tracker;auto p=snap(1);
    tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);
    p.game.phase=4;p.game.seconds=490;p.resultStamp=11;p.sack=true;p.down=4;p.fieldSpot-=2;
    assert(tracker.observe(rules,p).sack);
    tracker.reset();p=snap(1);p.sack=true;tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);
    p.game.phase=4;p.sack=false;p.sackSampled=true;tracker.observe(rules,p);
    p.game.phase=5;p.game.seconds=490;p.resultStamp=11;p.sack=true;p.down=4;p.fieldSpot-=2;
    assert(tracker.observe(rules,p).sack);
    // The native selector is stronger evidence than an apparent reset: a false
    // value for old play41 cannot authorize a later true value for old play41.
    for(bool advanced:{false,true})for(bool apparentReset:{false,true})for(int phase:{1,4,5})for(int side:{0,1}){
        tracker.reset();p=snap(side);p.down=1;p.sack=true;p.sackPlay=41;
        tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);
        if(apparentReset){p.sack=false;p.sackSampled=true;tracker.observe(rules,p);}
        p.game.phase=phase;p.game.seconds=488;p.resultStamp=11;p.down=2;p.fieldSpot-=2;
        p.sack=true;p.sackPlay=advanced?42:41;
        auto result=tracker.observe(rules,p);assert(result.completed&&result.sack==advanced);
        ++cases;
    }
    // Test the production read-only selector adapter and all its fail-closed guards.
    for(bool nextSack:{false,true}){
        tracker.reset();p=snap(0);p.sackPlay=40;tracker.observe(rules,p);
        p.game.phase=3;tracker.observe(rules,p);p.game.phase=4;p.game.seconds=490;
        p.resultStamp=11;p.sack=true;p.sackPlay=41;p.fumble=p.forcedFumble=true;p.offense=1;p.down=1;
        assert(tracker.observe(rules,p).takeaway);
        p.game.phase=1;p.game.seconds=488;p.sack=false;p.fumble=p.forcedFumble=false;p.fieldSpot=-20;
        tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);
        p.game.phase=4;p.game.seconds=480;p.resultStamp=12;p.sack=true;p.sackPlay=nextSack?42:41;p.down=2;p.fieldSpot-=2;
        assert(tracker.observe(rules,p).sack==nextSack); // An intervening false value cannot revive an old sack ID.
        ++cases;
    }
    constexpr uintptr_t image=0x140000000,stats=0x20000,selector=0x30000;
    put(stats+0xce0,selector);put(selector,image+bindings::SackPlaySelectorVtable);
    put(image+bindings::SackPlaySelectorVtable+0x68,image+bindings::SackPlayIndexGetter);
    memory[image+bindings::SackPlayIndexGetter]={0x8b,0x41,0x10,0xc3};
    for(int index:{0,2,2998}){put(selector+0x10,index);assert(sackfacts::selectedPlay(image,stats,read)==index);}
    for(int index:{-1,2999,3000}){put(selector+0x10,index);assert(sackfacts::selectedPlay(image,stats,read)==-1);}
    put(selector+0x10,2);memory[image+bindings::SackPlayIndexGetter][0]=0x90;assert(sackfacts::selectedPlay(image,stats,read)==-1);
    memory[image+bindings::SackPlayIndexGetter][0]=0x8b;put(image+bindings::SackPlaySelectorVtable+0x68,image+bindings::SackPlayIndexGetter+1);assert(sackfacts::selectedPlay(image,stats,read)==-1);
    put(image+bindings::SackPlaySelectorVtable+0x68,image+bindings::SackPlayIndexGetter);put(selector,image+bindings::SackPlaySelectorVtable+8);assert(sackfacts::selectedPlay(image,stats,read)==-1);
    put(selector,image+bindings::SackPlaySelectorVtable);memory.erase(selector+0x10);assert(sackfacts::selectedPlay(image,stats,read)==-1);
    assert(sackfacts::selectedPlay(image,0,read)==-1);
    std::printf("PASS: %u sack/strip-sack carryover cases, ownership changes, fresh result hashes, run losses, unknown samples, verified rearming and genuine consecutive sacks\n",cases);
}
