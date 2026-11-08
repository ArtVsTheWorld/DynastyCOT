#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <intrin.h>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <limits>
#include "../Source/PauseMenu.h"
using A=uintptr_t;
namespace {
A observedTile=0,observedText=0;bool substitute=true;unsigned selections=0;
using Clobber=void(*)();Clobber clobber=nullptr;
const char replacement[]="Resume \xe2\x80\x93 Stadium Fullness: 48%";
const char* __fastcall select(A tile,A key,A text){
    assert((reinterpret_cast<A>(_AddressOfReturnAddress())&15)==8);
    assert(!std::strcmp(reinterpret_cast<const char*>(key),"display"));
    observedTile=tile;observedText=text;++selections;clobber();return substitute?replacement:nullptr;
}
struct Capture {A tile=0,context=0,text=0,rax=1,rcx=0,rdx=0,r8=0,r9=0,r10=0,r11=0,flags=0;unsigned char xmm[96]{};};
void executable(){
    using namespace attendance::pausemenu;
    auto memory=static_cast<unsigned char*>(VirtualAlloc(nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));assert(memory);
    std::vector<unsigned char> start;
    auto add=[&](std::initializer_list<unsigned char> b){start.insert(start.end(),b);};
    auto q=[&](A p){for(unsigned i=0;i<8;++i)start.push_back(static_cast<unsigned char>(p>>(i*8)));};
    // Model the verified mid-function frame without calling any game code.
    add({0x55,0x56,0x57,0x48,0x81,0xec,0,1,0,0,0x48,0x8b,0xf9,
         0x48,0x8b,0x07,0x48,0x89,0x44,0x24,0x28,
         0x48,0x8b,0x47,0x08,0x48,0x89,0x44,0x24,0x20,
         0x48,0x8b,0x77,0x10,0x48,0x8d,0x4e,0x38,0x48,0xb8});q(0x0079616c70736964ull);
    add({0x48,0x89,0x44,0x24,0x78,0x41,0xb9,0x0f,0,0,0,0x45,0x33,0xc0,0x49,0xba});q(0x11223344);
    add({0x49,0xbb});q(0x55667788);
    for(unsigned i=0;i<6;++i)add({0x66,0x0f,0xef,static_cast<unsigned char>(0xc0+i*9)});
    add({0x48,0xb8});q(reinterpret_cast<A>(memory+512));add({0xff,0xe0});
    assert(start.size()<256);memcpy(memory,start.data(),start.size());
    auto gate=trampoline(reinterpret_cast<A>(select),reinterpret_cast<A>(memory+1024));assert(gate.size()<512);memcpy(memory+512,gate.data(),gate.size());
    start.clear();
    auto store=[&](unsigned char mod,unsigned char reg,size_t offset){add({0x48,0x89,static_cast<unsigned char>(mod+reg*8)});start.push_back(static_cast<unsigned char>(offset));};
    store(0x47,0,offsetof(Capture,rax));store(0x47,1,offsetof(Capture,rcx));store(0x47,2,offsetof(Capture,rdx));
    add({0x4c,0x89,0x47,static_cast<unsigned char>(offsetof(Capture,r8)),0x4c,0x89,0x4f,static_cast<unsigned char>(offsetof(Capture,r9)),
         0x4c,0x89,0x57,static_cast<unsigned char>(offsetof(Capture,r10)),0x4c,0x89,0x5f,static_cast<unsigned char>(offsetof(Capture,r11)),
         0x9c,0x58,0x48,0x89,0x47,static_cast<unsigned char>(offsetof(Capture,flags))});
    for(unsigned i=0;i<6;++i){add({0xf3,0x0f,0x7f,static_cast<unsigned char>(0x87+i*8)});uint32_t offset=static_cast<uint32_t>(offsetof(Capture,xmm)+16*i);for(unsigned n=0;n<4;++n)start.push_back(static_cast<unsigned char>(offset>>(8*n)));}
    add({0x48,0x81,0xc4,0,1,0,0,0x5f,0x5e,0x5d,0xc3});memcpy(memory+1024,start.data(),start.size());
    start.clear();
    for(unsigned i=0;i<6;++i)add({0x66,0x0f,0x76,static_cast<unsigned char>(0xc0+i*9)});
    add({0x49,0xba});q(0xdeadbeef);add({0x49,0xbb});q(0xabcdef);add({0x33,0xc0,0xc3});memcpy(memory+1536,start.data(),start.size());
    DWORD old=0;assert(VirtualProtect(memory,4096,PAGE_EXECUTE_READ,&old));FlushInstructionCache(GetCurrentProcess(),memory,4096);
    clobber=reinterpret_cast<Clobber>(memory+1536);char text[128]{};strcpy_s(text+0x38,sizeof(text)-0x38,"Resume");
    Capture capture;capture.tile=0x12345678;capture.context=0x23456789;capture.text=reinterpret_cast<A>(text);
    auto run=reinterpret_cast<void(*)(Capture*)>(memory);
    for(bool enabled:{true,false}){
        substitute=enabled;run(&capture);
        assert(observedTile==capture.tile&&observedText==capture.text+0x38&&capture.rcx==capture.context);
        assert(capture.r8==(enabled?reinterpret_cast<A>(replacement):capture.text+0x38));
        assert(capture.rax==0&&capture.r9==15&&capture.r10==0x11223344&&capture.r11==0x55667788);
        assert((capture.flags&0x8c5)==0x44);for(auto x:capture.xmm)assert(x==0);
    }
    assert(selections==2);assert(VirtualFree(memory,0,MEM_RELEASE));
}
}
int main(){
    using namespace attendance::pausemenu;Context c;c.enabled=c.offlineDynasty=c.paused=c.mapped=true;
    c.match=c.measuredMatch=123;c.epoch=c.measuredEpoch=7;c.peak=.8;c.arrivalMissing=40;
    assert(percent(c)==48);c.loss=65;assert(percent(c)==28);c.arrivalMissing=0;c.loss=0;assert(percent(c)==80);
    c.peak=1;assert(percent(c)==100);c.loss=100;assert(percent(c)==0);
    for(int mismatch=0;mismatch<5;++mismatch){auto x=c;if(mismatch==0)x.measuredMatch++;if(mismatch==1)x.measuredEpoch++;if(mismatch==2)x.mapped=false;if(mismatch==3)x.peak=std::numeric_limits<double>::quiet_NaN();if(mismatch==4)x.loss=101;assert(percent(x)==-1);}
    for(int off=0;off<6;++off){auto x=c;if(off==0)x.enabled=false;if(off==1)x.offlineDynasty=false;if(off==2)x.paused=false;if(off==3)x.replay=true;if(off==4)x.leaving=true;if(off==5)x.match=0;assert(percent(x)==-2);}
    Labels labels;for(int p=-2;p<=100;++p){auto label=labels.get(p);assert((label!=nullptr)==(p>=-1));if(label)assert(strlen(label)<64);}
    assert(!strcmp(labels.get(-1),"Resume \xe2\x80\x93 Stadium Fullness: --"));
    assert(labelField("RESUME_GAME","display")&&!labelField("CHALLENGE_PLAY","display")&&!labelField("RESUME_GAME","SubText"));
    executable();std::cout<<"PASS: native pause fullness scope, measured seating/arrivals, stale ownership, disabled/replay/exit guards, unavailable labels and executable selector ABI/register preservation.\n";
}
