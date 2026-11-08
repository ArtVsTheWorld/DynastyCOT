#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include "../Source/ReplayUpdateHook.h"
#include "../Source/GameBindings.h"
int main(){
    using namespace attendance::replayhook;
    int32_t displacement=0;std::memcpy(&displacement,Prefix.data()+13,4);
    assert(static_cast<intptr_t>(attendance::bindings::ReplayUpdate+SourceBytes)+displacement==static_cast<intptr_t>(attendance::bindings::ReplayManager));
    uintptr_t manager=0x123456789abc;auto memory=static_cast<unsigned char*>(VirtualAlloc(nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));assert(memory);
    // Resume after the expanded prologue; return the freshly loaded manager.
    const unsigned char finish[]={0x48,0x8b,0xc3,0x48,0x8b,0x5c,0x24,0x30,0x48,0x83,0xc4,0x20,0x5f,0xc3};
    auto code=trampoline(reinterpret_cast<uintptr_t>(&manager),reinterpret_cast<uintptr_t>(memory+128));
    assert(!std::memcmp(code.data(),Prefix.data(),10));
    std::memcpy(memory,code.data(),code.size());std::memcpy(memory+128,finish,sizeof(finish));
    DWORD old=0;assert(VirtualProtect(memory,4096,PAGE_EXECUTE_READ,&old));FlushInstructionCache(GetCurrentProcess(),memory,4096);
    auto call=reinterpret_cast<uintptr_t(*)()>(memory);assert(call()==manager);
    manager=0xabcdef123456;assert(call()==manager); // Reads current global, never a stale captured manager.
    assert(VirtualFree(memory,0,MEM_RELEASE));
    std::cout<<"PASS: executable replay-update trampoline, live manager indirection, complete prologue and preserved stack/registers\n";
}
