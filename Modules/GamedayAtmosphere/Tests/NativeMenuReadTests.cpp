#include "../Source/NativeSettingsMenu.h"
#include <cassert>
#include <cstdio>
int main(){
    using namespace attendance::settingsmenu::native;
    SYSTEM_INFO info{};GetSystemInfo(&info);auto page=static_cast<size_t>(info.dwPageSize);
    auto memory=static_cast<unsigned char*>(VirtualAlloc(nullptr,page*2,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));assert(memory);
    std::memset(memory,0xff,page*2);DWORD old=0;assert(VirtualProtect(memory+page,page,PAGE_NOACCESS,&old));
    assert(peek<unsigned>(reinterpret_cast<A>(memory))==0xffffffffu);
    assert(peek<uint64_t>(reinterpret_cast<A>(memory+page-4))==0); // Partial copy cannot produce an apparently valid pointer.
    assert(peek<uint64_t>(reinterpret_cast<A>(memory+page))==0&&peek<unsigned>(0)==0);
    assert(VirtualFree(memory,0,MEM_RELEASE));
    std::puts("PASS: complete native-menu reads, inaccessible pages, partial cross-page reads and null pointers fail closed without partial values.");
}
