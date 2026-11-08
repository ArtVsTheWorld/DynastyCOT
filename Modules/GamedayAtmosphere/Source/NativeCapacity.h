#pragma once
#include "StadiumOccupancy.h"
#include "SwellAudioRuntime.h"
namespace attendance::occupancy {
using sustained::Address;
struct Variant {Address vt,type,data;};
struct Api {
    using Query=bool(__fastcall*)(Address,unsigned,Variant*,Address,Address);
    using Capacity=int(__fastcall*)(int,int*);
    Query query=nullptr;Capacity capacity=nullptr;
};
inline bool nativeSnapshot(Address image,Address match,unsigned epoch,Snapshot& out,const Api* custom=nullptr){
    using sustained::at;
    Address service=at<Address>(image+0xe2e53e8),weather=at<Address>(image+0xf506000);
    auto current=[&](){return at<Address>(at<Address>(image+bindings::MatchWrapper)+16);};
    if(!match||current()!=match||at<Address>(match)!=image+bindings::MatchVtable||
       at<Address>(service)!=image+0xb29ee30||at<Address>(at<Address>(service)+0x68)!=image+bindings::OptionQuery||
       at<Address>(weather)!=image+0xb32a290)return false;
    int stadium=at<int>(weather+0x358);if(stadium<0||stadium>=1023)return false;
    Variant value{image+0xb135818,at<Address>(image+0xfc90fb8),0};if(!value.type)return false;
    Api api=custom?*custom:Api{reinterpret_cast<Api::Query>(image+bindings::OptionQuery),reinterpret_cast<Api::Capacity>(image+bindings::StadiumCapacity)};
    if(!api.query||!api.capacity)return false;
    Address okay=0;
    if(!sustained::obtain(okay,api.query,service,bindings::AttendanceOption,&value,Address(0),Address(0))||!okay||!value.data)return false;
    int attendance=-1;SIZE_T got=0;
    if(!ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(value.data),&attendance,sizeof(attendance),&got)||got!=sizeof(attendance))return false;
    int capacity=-1;
    if(!sustained::obtain(okay,api.capacity,stadium,&capacity)||okay!=0)return false;
    Snapshot next{match,epoch,stadium,attendance,capacity};
    if(!next.valid()||current()!=match||at<int>(weather+0x358)!=stadium)return false;
    out=next;return true;
}
}
