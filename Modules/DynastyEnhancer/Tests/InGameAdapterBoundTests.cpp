#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../Source/RecruitNotesUiRuntime.h"
#include "../Source/PlayerInGameUiPrograms.h"
#include <cassert>
#include <iostream>
#include <iterator>
int main(int argc,char** argv){
    assert(argc==2);std::ifstream f(argv[1],std::ios::binary);std::vector<unsigned char> body((std::istreambuf_iterator<char>(f)),{});
    auto programs=recruitingfog::playeringameui::programs();const auto& p=programs.front();
    assert(p.offset==229352&&p.hash==11921223581905478919ull&&body.size()==674);
    std::array<unsigned char,48> h{};auto set=[&](size_t off,auto value){memcpy(h.data()+off,&value,sizeof(value));};
    set(8,p.argc);set(12,static_cast<unsigned short>(p.registers));set(14,static_cast<unsigned short>(p.flags));set(24,p.size);set(32,1u);set(40,reinterpret_cast<uintptr_t>(body.data()));
    auto result=recruitingfog::notesui::makeDefinition(p,h,body);assert(result.size()==48+p.code.size());
    assert(std::equal(body.begin(),body.end(),result.begin()+48+p.copyAt));
    auto reject=[&](const auto& candidate,const auto& header,const auto& source){bool threw=false;try{recruitingfog::notesui::makeDefinition(candidate,header,source);}catch(const std::runtime_error&){threw=true;}assert(threw);};
    auto corrupt=body;corrupt[0]^=1;reject(p,h,corrupt);auto changed=h;changed[12]^=1;reject(p,changed,body);
    auto oversized=p;oversized.code.resize(32769);reject(oversized,h,body);
    for(const auto& candidate:programs)assert(candidate.code.size()<=32768&&candidate.newRegisters<=64);
    std::cout<<"PASS: exact native bank6 method accepted at "<<p.code.size()<<" bytes; changed native body/header and oversized programs rejected; native prefix copied intact. No hooks or game process used.\n";
}
