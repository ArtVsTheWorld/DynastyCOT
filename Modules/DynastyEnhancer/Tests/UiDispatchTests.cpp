#include "../Source/RecruitingFog.cpp"
#include <chrono>
#include <iostream>
int main(){
    using namespace recruitingfog;
    std::vector<std::array<unsigned char,48>> headers;
    for(auto* bank:runtime::cardUiBanks){bank->prepare();for(const auto& key:bank->frameKeys()){
        std::array<unsigned char,48> h{};const auto regs=static_cast<unsigned short>(key[1]),flags=static_cast<unsigned short>(key[2]);
        memcpy(h.data()+8,&key[0],4);memcpy(h.data()+12,&regs,2);memcpy(h.data()+14,&flags,2);memcpy(h.data()+24,&key[3],4);headers.push_back(h);
    }}
    headers.push_back({});notesui::FrameDispatch<runtime::cardUiBanks.size()> index(runtime::cardUiBanks);
    using Mask=notesui::FrameDispatch<runtime::cardUiBanks.size()>::Mask;
    for(bool fixed:{false,true})for(const auto& h:headers){Mask expected;for(size_t i=0;i<runtime::cardUiBanks.size();++i)if(runtime::cardUiBanks[i]->acceptsFrame(h.data(),fixed))expected.set(i);
        if(index.candidates(h.data(),fixed)!=expected)throw std::runtime_error("Native/owned/fixed frame dispatch mismatch");
    }
    volatile uint64_t sink=0;const auto begin=std::chrono::steady_clock::now();
    for(unsigned n=0;n<10000;++n){const auto& h=headers[n%headers.size()];Mask mask;for(size_t i=0;i<runtime::cardUiBanks.size();++i)if(runtime::cardUiBanks[i]->acceptsFrame(h.data()))mask.set(i);sink=mask.count();}
    const auto middle=std::chrono::steady_clock::now();for(unsigned n=0;n<10000;++n)sink=index.candidates(headers[n%headers.size()].data()).count();const auto end=std::chrono::steady_clock::now();
    std::cout<<"PASS: "<<headers.size()<<" native/owned frame shapes, both fixed-frame modes, identical adapter candidates. 10000 dispatches: linear="<<std::chrono::duration<double,std::milli>(middle-begin).count()<<" ms; indexed="<<std::chrono::duration<double,std::milli>(end-middle).count()<<" ms; sink="<<sink<<'\n';
}
