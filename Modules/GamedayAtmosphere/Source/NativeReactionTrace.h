#pragma once
#include <algorithm>
#include <array>
#include <cstdint>

namespace attendance::nativevoices {
struct Voice {int type=0,handle=-1;};
struct Snapshot {std::array<Voice,16> voices{};unsigned count=0;};
// Debug-only observation of engine-owned instances. No borrowed pointer is
// dereferenced here and no game state is changed. Voice ages are deliberately
// excluded so ordinary playback does not produce a line every frame.
class Trace {
    struct Slot {uintptr_t entity=0,asset=0,match=0;unsigned epoch=0;uint64_t last=0;Snapshot value;};
    std::array<Slot,32> slots_{};
public:
    bool observe(uintptr_t entity,uintptr_t asset,uintptr_t match,unsigned epoch,uint64_t now,Snapshot value){
        if(!entity||!asset||!match||value.count>value.voices.size())return false;
        std::sort(value.voices.begin(),value.voices.begin()+value.count,[](const Voice& a,const Voice& b){return a.type<b.type||(a.type==b.type&&a.handle<b.handle);});
        Slot* slot=nullptr;
        for(auto& s:slots_)if(s.entity==entity){slot=&s;break;}
        if(!slot){for(auto& s:slots_)if(!s.entity||now<s.last||now-s.last>30000){slot=&s;break;}}
        if(!slot)return false;
        bool same=slot->entity==entity&&slot->asset==asset&&slot->match==match&&slot->epoch==epoch&&now>=slot->last&&now-slot->last<=750;
        bool changed=!same?value.count!=0:slot->value.count!=value.count;
        if(same&&!changed)for(unsigned i=0;i<value.count;++i)if(value.voices[i].type!=slot->value.voices[i].type||value.voices[i].handle!=slot->value.voices[i].handle){changed=true;break;}
        *slot={entity,asset,match,epoch,now,value};return changed;
    }
};
}
