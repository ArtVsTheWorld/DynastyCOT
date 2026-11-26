#define NOMINMAX
#include <windows.h>
#include "../Source/RecruitingFeaturesRuntime.h"
#include "../Source/PreviouslyScoutedGrid.h"
#include <cassert>
#include <iostream>
int main(){
 using namespace recruitingfog;
 features::prepare();
 for(const auto* bank:features::banks)assert(bank->prepared&&(bank->methodCount()==0||!bank->frameKeys().empty()));
 const auto cutoff=interestui::cutoff();assert(cutoff.size()==1);
 const auto& divider=cutoff.front();
 assert(divider.offset==0x9a38-48&&divider.argc==0&&divider.registers==27&&divider.flags==41&&divider.size==748);
 assert(divider.copyAt==0&&divider.copyBytes==716&&divider.newRegisters==27&&divider.code.size()>748);
 // Native bytes are fingerprinted but not embedded in the generated source.
 assert(std::all_of(divider.code.begin(),divider.code.begin()+716,[](unsigned char b){return b==0;}));
 // A guarded jump from the copied native prefix reaches the row decorator.
 assert(divider.code[716]==0x99&&notesui::field<int32_t>(divider.code.data(),720)>24);
 const auto headerPrograms=interestui::offerHeader();
 assert(headerPrograms.size()==1&&headerPrograms.front().size==437&&headerPrograms.front().copyBytes==437);
 // Read live six-bit totals from a fixture: zero is distinct from missing,
 // and changing the packed count is visible without recapturing the class.
 dynasty::live::RecruitOfferTable table;
 table.data=0x1000;table.first=32;table.words=2;table.players={777,0};
 table.playerBits=0;table.offerBits=32+14;
 uint32_t livePlayer=777,total=0;
 auto readRow=[&](uintptr_t address,void* out,size_t size){
    if(size!=4)return false;
    uint32_t value=0;
    if(address==0x1080)value=livePlayer;
    else if(address==0x1084)value=(0xab000000u|(total<<14)|123u);
    else return false;
    memcpy(out,&value,4);return true;
 };
 for(auto count:{0u,1u,4u,63u}){total=count;assert(table.count(0,readRow)==count);}
 assert(!table.count(1,readRow)&&!table.count(2,readRow));
 livePlayer=555;assert(!table.count(0,readRow));
 const auto originals=currencytopbarui::recruitingPrograms(),composed=interestui::scouts();
 assert(composed.size()==3);const auto& actual=composed.front();bool matched=false;
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
 std::cout<<"PASS: native recruiting contracts, guarded live offer reads, composed recruiting currency bytes/flags preserved; protected Target states and unknown script forwarding. No game access or hooks.\n";
}
