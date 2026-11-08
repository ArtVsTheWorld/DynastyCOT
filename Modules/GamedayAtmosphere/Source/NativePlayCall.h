#pragma once
#include "GameBindings.h"
#include "NativeRead.h"
#include <cstddef>
namespace attendance::playcall {
using Address=uintptr_t;using Read=bool(*)(Address,void*,size_t);
class View {
    Address image_;Read read_;
    template<class T>T at(Address p)const{return attendance::nativeValue<T>(p,read_);}
public:
    View(Address image,Read read):image_(image),read_(read){}
    int type(Address match,int side)const{
        if(side<0||side>1||at<Address>(match)!=image_+bindings::MatchVtable)return -1;
        auto service=at<Address>(image_+bindings::PlayCallService);
        if(at<Address>(service)!=image_+bindings::PlayCallServiceVtable)return -1;
        Address team=service+0x10+static_cast<Address>(side)*0xde8;
        if(at<Address>(team)!=image_+bindings::PlayCallTeamVtable||at<Address>(team+8)!=image_+bindings::PlayCallSelectedVtable||at<Address>(team+16)!=image_+bindings::PlayCallVtable)return -1;
        int current=-1,fallback=-1;
        if(!read_(team+16+0x2c,&current,4)||!read_(team+16+0x30,&fallback,4))return -1;
        int result=current!=-1?current:fallback;return result>0&&result<256?result:-1;
    }
};
}
