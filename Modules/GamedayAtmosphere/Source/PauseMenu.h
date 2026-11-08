#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include "GameBindings.h"

namespace attendance::pausemenu {
// This is the named TextMedia output in GetListTiles, immediately before the
// native string writer. It is deliberately separate from the shared writer
// used by other plugins. No menu object, action, localization or layout is edited.
inline constexpr uintptr_t Site=bindings::PauseMenuText,Writer=bindings::MenuStringWriter,TileVtable=bindings::MenuTileVtable;
inline constexpr std::array<unsigned char,25> Prefix={
    0x41,0x8b,0xc1,0x48,0x8d,0x54,0x24,0x78,0xc1,0xe8,0x07,0xa8,0x01,
    0x49,0x0f,0x45,0xd0,0x4c,0x8b,0xc1,0x48,0x8b,0x4c,0x24,0x20};
inline bool labelField(const char* action,const char* key){
    return action&&key&&!std::strcmp(action,"RESUME_GAME")&&!std::strcmp(key,"display");
}
struct Context {
    bool enabled=false,offlineDynasty=false,paused=false,replay=false,leaving=false;
    uintptr_t match=0,measuredMatch=0;unsigned epoch=0,measuredEpoch=0;
    bool mapped=false;double peak=0;int loss=0,arrivalMissing=0;
};
// -2 leaves native text alone; -1 explicitly reports an unavailable measurement.
inline int percent(const Context& c){
    if(!c.enabled||!c.offlineDynasty||!c.paused||c.replay||c.leaving||!c.match)return -2;
    if(!c.mapped||c.measuredMatch!=c.match||c.measuredEpoch!=c.epoch||
       !std::isfinite(c.peak)||c.peak<0||c.peak>1||c.loss<0||c.loss>100||c.arrivalMissing<0||c.arrivalMissing>100)return -1;
    int absent=c.loss>c.arrivalMissing?c.loss:c.arrivalMissing;
    return static_cast<int>(std::floor(c.peak*(100-absent)+.5));
}
class Labels {
    std::array<std::array<char,64>,102> text_{};
public:
    Labels(){
        for(int i=0;i<=100;++i)std::snprintf(text_[i].data(),text_[i].size(),"Resume \xe2\x80\x93 Stadium Fullness: %d%%",i);
        std::snprintf(text_[101].data(),text_[101].size(),"Resume \xe2\x80\x93 Stadium Fullness: --");
    }
    const char* get(int value)const{return value>=0&&value<=100?text_[value].data():value==-1?text_[101].data():nullptr;}
};
// Original RSP is 16-byte aligned at this mid-function call site. Preserve all
// volatile registers, flags and XMM0-5 around the C++ selector. A non-null return
// substitutes only R8 (the serialized string); the original native writer runs.
inline std::vector<unsigned char> trampoline(uintptr_t selector,uintptr_t resume){
    std::vector<unsigned char> out(Prefix.begin(),Prefix.end());out.reserve(256);
    auto add=[&](std::initializer_list<unsigned char> b){out.insert(out.end(),b);};
    auto address=[&](uintptr_t p){for(unsigned i=0;i<8;++i)out.push_back(static_cast<unsigned char>(p>>(i*8)));};
    add({0x9c,0x50,0x51,0x52,0x41,0x50,0x41,0x51,0x41,0x52,0x41,0x53,0x48,0x81,0xec,0x80,0,0,0});
    for(unsigned i=0;i<6;++i)add({0xf3,0x0f,0x7f,static_cast<unsigned char>(0x44+i*8),0x24,static_cast<unsigned char>(0x20+i*16)});
    // Original [RSP+28h] is the current tile, including copies made for a menu draw.
    add({0x48,0x8b,0x8c,0x24,0xe8,0,0,0,0x48,0xb8});address(selector);add({0xff,0xd0});
    add({0x48,0x85,0xc0,0x74,0x08,0x48,0x89,0x84,0x24,0x98,0,0,0});
    for(unsigned i=0;i<6;++i)add({0xf3,0x0f,0x6f,static_cast<unsigned char>(0x44+i*8),0x24,static_cast<unsigned char>(0x20+i*16)});
    add({0x48,0x81,0xc4,0x80,0,0,0,0x41,0x5b,0x41,0x5a,0x41,0x59,0x41,0x58,0x5a,0x59,0x58,0x9d,0xff,0x25,0,0,0,0});address(resume);
    return out;
}
}
