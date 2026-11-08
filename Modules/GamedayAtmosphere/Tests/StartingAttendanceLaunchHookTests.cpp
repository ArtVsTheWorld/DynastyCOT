#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cassert>
#include <cstring>
#include <cstdio>
#include "../Source/StartingAttendanceLaunchHook.h"
using A=uintptr_t;
int stage=0,visits=0;A expectedManager=0,expectedRoster=0;int expectedTeam=0;
void boundary(){assert(stage==0);stage=1;++visits;}
A setup(A manager,int team,A roster){assert(stage==1&&manager==expectedManager&&team==expectedTeam&&roster==expectedRoster);stage=2;return 0x123456789abcdef0ull;}
int main(){
    using namespace attendance::turnout::launchhook;
    int32_t displacement=0;std::memcpy(&displacement,Prefix.data()+13,4);
    assert(static_cast<intptr_t>(Site+SourceBytes)+displacement==static_cast<intptr_t>(NativeTeamSetup));
    auto memory=static_cast<unsigned char*>(VirtualAlloc(nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));assert(memory);
    auto code=trampoline(reinterpret_cast<A>(setup),reinterpret_cast<A>(memory+512));
    assert(!std::memcmp(code.data(),Prefix.data(),12));
    std::memcpy(memory+256,code.data(),code.size());
    auto entry=gate(reinterpret_cast<A>(boundary),reinterpret_cast<A>(memory+256));assert(entry.size()<256);
    std::memcpy(memory,entry.data(),entry.size());
    // Resume the synthetic native frame, leaving its native return value intact.
    const unsigned char finish[]={0x48,0x83,0xc4,0x20,0x5f,0x41,0x5e,0x41,0x5d,0xc3};std::memcpy(memory+512,finish,sizeof(finish));
    // A native caller frame with seven/equivalent alignment pushes in the real
    // function. Three pushes here produce the same aligned mid-function RSP.
    std::vector<unsigned char> wrapper={0x41,0x55,0x41,0x56,0x57,0x48,0x83,0xec,0x20};
    auto add=[&](std::initializer_list<unsigned char> b){wrapper.insert(wrapper.end(),b);};
    auto address=[&](A p){for(unsigned i=0;i<8;++i)wrapper.push_back(static_cast<unsigned char>(p>>(8*i)));};
    expectedManager=reinterpret_cast<A>(&stage);expectedRoster=reinterpret_cast<A>(&visits)+0x9f0;expectedTeam=1161;
    add({0x49,0xbd});address(expectedRoster-0x9f0);add({0x49,0xbe});address(expectedManager);add({0xbf,0x89,0x04,0,0});
    // Both routes join at the same block: test ecx,ecx; jz shared; nop.
    add({0x85,0xc9,0x74,0x01,0x90,0x48,0xb8});address(reinterpret_cast<A>(memory));add({0xff,0xe0});
    std::memcpy(memory+768,wrapper.data(),wrapper.size());DWORD old=0;assert(VirtualProtect(memory,4096,PAGE_EXECUTE_READ,&old));FlushInstructionCache(GetCurrentProcess(),memory,4096);
    auto call=reinterpret_cast<A(*)(int)>(memory+768);
    for(int rivalry:{0,1}){stage=0;int before=visits;assert(call(rivalry)==0x123456789abcdef0ull&&stage==2&&visits==before+1);}
    assert(VirtualFree(memory,0,MEM_RELEASE));
    std::puts("PASS: ordinary/rivalry paths join before native setup, exactly one callback, relocated native call with correct arguments/return, balanced executable gate and stack restoration.");
}
