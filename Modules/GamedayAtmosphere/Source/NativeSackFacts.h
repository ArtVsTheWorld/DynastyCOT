#pragma once
#include "GameBindings.h"
#include <cstddef>
#include <cstring>
namespace attendance::sackfacts {
using Address=uintptr_t;using Read=bool(*)(Address,void*,size_t);
// SackPredicate itself gets the selected DB play through vtable slot68h. The
// guarded getter is mov eax,[rcx+10h]; ret, so reading that field is equivalent.
inline int selectedPlay(Address image,Address tracker,Read read){
    Address selector=0,table=0,getter=0;
    if(!tracker||!read(tracker+0xce0,&selector,8)||!selector||!read(selector,&table,8)||table!=image+bindings::SackPlaySelectorVtable||
       !read(table+0x68,&getter,8)||getter!=image+bindings::SackPlayIndexGetter)return -1;
    unsigned char code[4]{};const unsigned char expected[]={0x8b,0x41,0x10,0xc3};
    int index=-1;
    return read(getter,code,4)&&!std::memcmp(code,expected,4)&&read(selector+0x10,&index,4)&&index>=0&&index<2999?index:-1;
}
}
