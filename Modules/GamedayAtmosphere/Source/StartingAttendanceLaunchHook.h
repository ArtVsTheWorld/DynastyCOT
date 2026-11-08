#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>
namespace attendance::turnout::launchhook {
// Both the ordinary branch (7f1463c -> 7f146f8) and optional rivalry
// publication join here. All launch options are final; team setup follows.
inline constexpr uintptr_t Site=0x7f182c8,NativeTeamSetup=0x7f20c60;
inline constexpr size_t SourceBytes=17,TrampolineBytes=42;
// lea r8,[r13+9f0]; mov edx,edi; mov rcx,r14; call NativeTeamSetup.
inline constexpr std::array<unsigned char,SourceBytes> Prefix={0x4d,0x8d,0x85,0xf0,0x09,0,0,0x8b,0xd7,0x49,0x8b,0xce,0xe8,0x87,0x89,0,0};
inline std::array<unsigned char,TrampolineBytes> trampoline(uintptr_t setup,uintptr_t resume){
    std::array<unsigned char,TrampolineBytes> out{};std::memcpy(out.data(),Prefix.data(),12);
    // Indirect call followed by a jump over its address, without borrowing
    // a register or imposing a +/-2GB allocation requirement.
    const unsigned char call[]={0xff,0x15,2,0,0,0,0xeb,8};std::memcpy(out.data()+12,call,8);std::memcpy(out.data()+20,&setup,8);
    out[28]=0xff;out[29]=0x25;std::memcpy(out.data()+34,&resume,8);return out;
}
inline std::vector<unsigned char> gate(uintptr_t callback,uintptr_t original){
    std::vector<unsigned char> out;out.reserve(192);
    auto add=[&](std::initializer_list<unsigned char> b){out.insert(out.end(),b);};
    auto address=[&](uintptr_t p){for(unsigned i=0;i<8;++i)out.push_back(static_cast<unsigned char>(p>>(i*8)));};
    // LaunchCopy pushes seven registers and allocates 3a0h: RSP is 16-byte
    // aligned at this mid-function site. Save flags, volatile GPRs and XMM0-5;
    // retain the caller's shadow space and all original stack arguments.
    add({0x9c,0x50,0x51,0x52,0x41,0x50,0x41,0x51,0x41,0x52,0x41,0x53,0x48,0x81,0xec,0x80,0,0,0});
    for(unsigned i=0;i<6;++i)add({0xf3,0x0f,0x7f,static_cast<unsigned char>(0x44+i*8),0x24,static_cast<unsigned char>(0x20+i*16)});
    add({0x48,0xb8});address(callback);add({0xff,0xd0});
    for(unsigned i=0;i<6;++i)add({0xf3,0x0f,0x6f,static_cast<unsigned char>(0x44+i*8),0x24,static_cast<unsigned char>(0x20+i*16)});
    add({0x48,0x81,0xc4,0x80,0,0,0,0x41,0x5b,0x41,0x5a,0x41,0x59,0x41,0x58,0x5a,0x59,0x58,0x9d,0xff,0x25,0,0,0,0});address(original);return out;
}
}
