#define NOMINMAX
#include <windows.h>
#include "../Source/RecruitingFeaturesRuntime.h"
#include "../Source/PreviouslyScoutedGrid.h"
#include <cassert>
#include <iostream>
int main(){
 using namespace recruitingfog;
 features::prepare();
 for(const auto* bank:features::banks)assert(bank->prepared&&!bank->frameKeys().empty());
 const auto cutoff=interestui::cutoff();assert(cutoff.size()==1);
 const auto& divider=cutoff.front();
 assert(divider.offset==0x9a38-48&&divider.argc==0&&divider.registers==27&&divider.flags==41&&divider.size==748);
 assert(divider.copyAt==0&&divider.copyBytes==748&&divider.newRegisters==27&&divider.code.size()>748);
 // Native bytes are fingerprinted but not embedded in the generated source.
 assert(std::all_of(divider.code.begin(),divider.code.begin()+748,[](unsigned char b){return b==0;}));
 const auto originals=currencytopbarui::recruitingPrograms(),composed=interestui::scouts();
 assert(composed.size()==1);const auto& actual=composed.front();bool matched=false;
 for(const auto& original:originals)if(original.offset==actual.offset){
  assert(actual.hash==original.hash&&actual.argc==original.argc&&actual.registers==original.registers&&actual.flags==original.flags);
  assert(actual.copyAt==original.copyAt&&actual.copyBytes==original.copyBytes&&actual.dollarOffsets==original.dollarOffsets);
  assert(actual.code.size()>original.code.size()&&original.code.back()==0x3e&&actual.code[original.code.size()-1]==0x17);
  assert(std::equal(original.code.begin(),original.code.end()-1,actual.code.begin()));matched=true;
 }
 assert(matched&&interestui::targets().size()==1);
 for(const auto* native:{"TargetTargeted","Target,61","TargetVisiting","TargetReadyVisit","Alex Bell","40","Unknown"})assert(!features::scoutingTarget(native));
 for(const auto* candidate:{"TargetNone","TargetLockedOut","TargetDealbreaker","TargetInterested"})assert(features::scoutingTarget(candidate));
 std::array<unsigned char,96> object{};std::array<unsigned char,48> header{};
 const auto definition=reinterpret_cast<uintptr_t>(header.data());memcpy(object.data()+0x58,&definition,8);
 auto unusedRead=[](uintptr_t,void*,size_t){assert(false);return false;};
 assert(features::beforeInvoke(object.data(),unusedRead)==-1);
 std::cout<<"PASS: eight nonempty native UI banks; composed recruiting currency bytes/flags preserved; protected Target states and unknown script forwarding. No game access or hooks.\n";
}
