#pragma once
#include <array>
#include <cstdint>
#include <cstring>

namespace enhancements {
struct CreationScope {uintptr_t response=0,data=0;uint32_t league=0;explicit operator bool()const{return response&&data&&league&&league!=UINT32_MAX;}};
// The native initial Commissioner page owns a local response before the
// career service becomes active. LeagueID already belongs to that creation.
// Read-only validation; no extra hook, native call, database write or save name.
template<class Read>CreationScope creationScope(uintptr_t image,uintptr_t provider,Read read){
    CreationScope out;uintptr_t table=0,service=0,method=0;unsigned char flag=0,active=0;int mode=0,count=0,status=0;
    if(!provider||!read(provider,&table,8)||table!=image+0xbab9a08||
       !read(provider+0x420,&flag,1)||flag!=1||!read(image+0xd9eb808,&mode,4)||mode!=-1||
       !read(image+0xfa519b8,&service,8)||!service||!read(service,&table,8)||table!=image+0xbaa8998||
       !read(service+0x7b1,&active,1)||active!=0||
       !read(provider+0x418,&out.response,8)||out.response<0x10000||
       !read(out.response,&table,8)||table!=image+0xbd7d2b8||
       !read(table+0xa10,&method,8)||method!=image+0x8459ff0)return {};
    std::array<unsigned char,3> offline{};
    if(!read(method,offline.data(),offline.size())||offline!=std::array<unsigned char,3>{0x32,0xc0,0xc3}||
       !read(out.response+0xb1,&active,1)||active!=1||!read(out.response+0xb4,&count,4)||count<=0||
       !read(out.response+0xc4,&status,4)||status>0||
       !read(out.response+0x140,&out.data,8)||out.data<0x10000||
       !read(image+0xf55c998,&out.league,4)||!out.league||out.league==UINT32_MAX)return {};
    uintptr_t response=0,data=0;uint32_t league=0;
    if(!read(provider+0x418,&response,8)||response!=out.response||!read(response+0x140,&data,8)||data!=out.data||
       !read(provider+0x420,&flag,1)||flag!=1||!read(service+0x7b1,&active,1)||active!=0||
       !read(image+0xf55c998,&league,4)||league!=out.league)return {};
    return out;
}
}
