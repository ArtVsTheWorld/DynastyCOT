#pragma once
#include <array>
#include <cstdint>
#include <cstring>
namespace attendance::replayhook {
inline constexpr size_t SourceBytes=17,TrampolineBytes=37;
// Complete instructions through mov rbx,[rip+ReplayManager]. The replay entity's
// native updater consumes only RCX and continues even when gameplay is paused.
inline constexpr std::array<unsigned char,SourceBytes> Prefix={0x48,0x89,0x5c,0x24,0x08,0x57,0x48,0x83,0xec,0x20,0x48,0x8b,0x1d,0x17,0x91,0xae,0x0b};
inline std::array<unsigned char,TrampolineBytes> trampoline(uintptr_t managerGlobal,uintptr_t resume){
    std::array<unsigned char,TrampolineBytes> code{};std::memcpy(code.data(),Prefix.data(),10);
    // Expand the RIP-relative load, so the trampoline can live anywhere. Its
    // input instruction boundaries 0,5,6,10 remain valid for thread relocation.
    code[10]=0x48;code[11]=0xbb;std::memcpy(code.data()+12,&managerGlobal,8);
    code[20]=0x48;code[21]=0x8b;code[22]=0x1b;
    code[23]=0xff;code[24]=0x25;std::memcpy(code.data()+29,&resume,8);return code;
}
}
