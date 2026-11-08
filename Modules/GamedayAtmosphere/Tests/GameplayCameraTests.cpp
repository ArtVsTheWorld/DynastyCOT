#include "../Source/GameplayCamera.h"
#include <map>
#include <vector>
#include <cstring>
#include <cassert>
#include <limits>
#include <iostream>
using namespace attendance;using namespace attendance::gamecamera;
using A=uintptr_t;
int main(){
    std::array<float,5> percent{{100,100,100,100,100}};assert(!custom(percent));
    for(int kind=0;kind<5;++kind){
        assert(known(kind)&&scale(percent,kind)==Baseline[kind]);
        for(float value:{0.f,50.f,100.f,125.f,200.f}){
            auto candidate=percent;candidate[kind]=value;
            assert(std::fabs(scale(candidate,kind)-Baseline[kind]*value*.01f)<.00001f);
            for(int other=0;other<5;++other)if(other!=kind)assert(scale(candidate,other)==Baseline[other]);
        }
    }
    assert(!known(-1)&&!known(5)&&scale(percent,-1)==1&&scale(percent,5)==1);
    percent[0]=std::numeric_limits<float>::quiet_NaN();assert(scale(percent,0)==1);percent[0]=300;assert(scale(percent,0)==2.f);percent[0]=-1;assert(scale(percent,0)==0);
    constexpr A image=0x10000000,settings=0x20000000,manager=0x21000000,match=0x22000000;
    std::map<A,std::vector<unsigned char>> memory;
    auto put=[&](A address,const auto& value){std::vector<unsigned char> bytes(sizeof(value));std::memcpy(bytes.data(),&value,sizeof(value));memory[address]=bytes;};
    unsigned reads=0;auto read=[&](A address,void* out,size_t n){++reads;auto found=memory.find(address);if(found==memory.end()||found->second.size()!=n)return false;std::memcpy(out,found->second.data(),n);return true;};
    put(image+bindings::CameraSettings,settings);put(settings,image+bindings::CameraSettingsVtable);
    put(image+bindings::TeamInputManager,manager);put(manager,image+bindings::TeamInputManagerVtable);put(match,image+A(0xb47e908));
    std::array<int,22> records{};records[0]=0;records[1]=0;records[5]=35;records[11]=-1;put(manager+0x148,records);
    auto selection=[&](unsigned short offense,unsigned short defense,int user,int possession){
        put(settings+0x68,offense);put(settings+0x6a,defense);records[1]=user;put(manager+0x148,records);put(match+0x1c8,possession);return sample(image,match,read);
    };
    for(unsigned short offense=0;offense<5;++offense)for(unsigned short defense=0;defense<5;++defense)for(int user:{0,1})for(int possession:{0,1}){
        auto result=selection(offense,defense,user,possession);assert(result.offense==offense&&result.defense==defense&&result.active==(user==possession?offense:defense));
    }
    // Exercise the production fast path: default sliders need no native reads.
    percent.fill(100);selection(0,1,0,0);reads=0;sample(image,match,read);unsigned baselineReads=reads;assert(baselineReads>0);
    reads=0;for(unsigned i=0;i<10000;++i)assert(preference(percent,image,match,read)==1);assert(reads==0);
    percent[0]=150;assert(preference(percent,image,match,read)==1.5f&&reads==baselineReads);
    percent[0]=0;assert(preference(percent,image,match,read)==0);percent.fill(100);
    std::cout<<"Default camera lookup: "<<baselineReads<<" native reads per output before; 0 after; 10000 default outputs checked.\n";
    assert(selection(0,1,0,-1).active==-1);assert(selection(99,1,0,0).active==-1);assert(selection(99,1,0,1).active==1);
    selection(0,1,0,0);records[5]=9;put(manager+0x148,records);assert(sample(image,match,read).active==-1); // Player-lock is not a normal team camera.
    records[5]=35;records[11]=1;records[12]=1;records[16]=35;put(manager+0x148,records);assert(sample(image,match,read).active==-1);
    put(settings+0x6a,static_cast<unsigned short>(0));assert(sample(image,match,read).active==0); // Same preset is unambiguous for both users.
    put(manager,A(0));assert(sample(image,match,read).active==-1);put(manager,image+bindings::TeamInputManagerVtable);
    memory.erase(settings+0x68);assert(sample(image,match,read).active==-1);
    std::cout<<"PASS: all five native camera IDs, 100 offense/defense/possession combinations, independent scale defaults/bounds, unknown/partial reads, player-lock and ambiguous multiplayer fallback. No game access.\n";
}
