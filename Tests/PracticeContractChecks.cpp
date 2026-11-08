#include "../Compatibility/PracticeTextSetter.generated.h"
#include "../Build/Compatibility/PracticeContractFixtures.h"
#include "../Modules/DynastyEnhancer/Source/CurrentBindings.h"
#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>
namespace p=cotcompat::practice;
struct Memory {
 std::map<uintptr_t,std::vector<unsigned char>> regions;
 bool read(uintptr_t at,void* output,size_t size)const {
  auto i=regions.upper_bound(at);if(i==regions.begin())return false;--i;
  if(at<i->first||size>i->second.size()||at-i->first>i->second.size()-size)return false;
  std::memcpy(output,i->second.data()+at-i->first,size);return true;
 }
};
static int checks=0;
void check(bool value,const char* name){if(!value)throw std::runtime_error(name);++checks;}
template<size_t N>void putEntry(Memory& m,uintptr_t at,uintptr_t target) {
 auto& bytes=m.regions[at];bytes.assign(N,0x90);bytes[0]=0xff;bytes[1]=0x25;
 bytes[2]=bytes[3]=bytes[4]=bytes[5]=0;std::memcpy(bytes.data()+6,&target,8);
}
int main(){
 const unsigned char* native=nullptr;
 for(const auto& g:socialGates)if(g.rva==p::site14)native=g.bytes;
 check(native!=nullptr,"Missing independent native gate");
 for(unsigned variant=0;variant<4;++variant){
  uintptr_t image=0x140000000ull+((variant&1)?0x10000000:0),code=0x300000000ull;
  Memory m;putEntry<p::original0.size()>(m,image+p::site0,code);
  putEntry<p::original13.size()>(m,image+p::site13,code+13*1024);
  putEntry<p::original14.size()>(m,image+p::site14,code+14*1024);
  m.regions[image+p::site14].resize(32);
  std::memcpy(m.regions[image+p::site14].data()+p::original14.size(),native+p::original14.size(),32-p::original14.size());
  m.regions[code]={fixture0[variant].begin(),fixture0[variant].end()};
  m.regions[code+13*1024]={fixture13[variant].begin(),fixture13[variant].end()};
  m.regions[code+14*1024]={fixture14[variant].begin(),fixture14[variant].end()};
  auto accept=[&](){
   unsigned char local[32]{};
   if(!m.read(image+p::site14,local,32))return false;
   if(std::memcmp(local,native,32))p::normalize(image,p::site14,local,native,32,[&](uintptr_t a,void* o,size_t n){return m.read(a,o,n);});
   return std::memcmp(local,native,32)==0;
  };
  check(accept(),"Exact known Practice hook rejected");
  auto initial=m.regions;
  check(accept()&&m.regions==initial,"Validation rewrote actual memory");
  for(uintptr_t at:{code,code+13*1024,code+14*1024,image+p::site0,image+p::site13,image+p::site14}){
   auto saved=m.regions[at];
   for(size_t i=0;i<saved.size();++i){
    m.regions[at][i]^=1;check(!accept(),"Changed body/entry/tail accepted");m.regions[at][i]^=1;
   }
   check(m.regions[at]==saved,"Probe changed mock memory");
  }
  auto saved=m.regions[code];m.regions.erase(code);check(!accept(),"Unreadable attestation accepted");m.regions[code]=saved;
  m.regions[image+p::site14]={native,native+32};check(accept(),"Original native bytes rejected");
 }
 std::cout<<"PASS: "<<checks<<" exact Practice contract checks; relocation variants, all body/entry byte mutations, unreadable code and native passthrough. No process access.\n";
}
