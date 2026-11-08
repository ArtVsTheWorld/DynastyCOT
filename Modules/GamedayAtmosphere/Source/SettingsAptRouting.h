#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace attendance::settingsmenu::routing {
// Follow the named common movie's active GenericSettings initializer. Some UI
// mods append a replacement initializer and leave the old code intact. Patching
// the old fixed offset then leaves a visible DGA tab routed to native settings.
// This is a bounded structural walk, not a scan of the heap or other UI movies.
inline constexpr size_t MaxOffset=2*1024*1024,BodySize=663,FallbackOffset=0x274;
inline constexpr std::array<unsigned char,32> StockFallback={
    0xad,5,1,3,4,5,0x6e,0xa3,0x49,2,0x47,0xb9,2,0x47,0xa2,0xc5,
    0x47,0x74,0xb5,2,0xad,3,0x60,0x1f,0x61,0xb2,0xc6,0x99,0,0,0,0};
using Body=std::array<unsigned char,BodySize>;
inline bool verifiedBody(const Body& body){
    // Every byte outside DGA's 32-byte fallback must match the verified native
    // switch, including all existing cases, branch targets and return code.
    uint64_t hash=14695981039346656037ull;
    for(size_t i=0;i<body.size();++i)if(i<FallbackOffset||i>=FallbackOffset+32)hash=(hash^body[i])*1099511628211ull;
    return hash==0x24a4e862c370dcd1ull;
}
template<class Read>class Cursor {
    Read& read_;uint64_t base_;
public:
    explicit Cursor(Read& read,uint64_t base):read_(read),base_(base){}
    bool bytes(size_t p,void* out,size_t n){return p<=MaxOffset&&n<=MaxOffset-p&&read_(p,out,n);}
    template<class T>bool value(size_t p,T& out){return bytes(p,&out,sizeof(out));}
    static size_t aligned(size_t p,size_t n=8){return (p+n-1)&~(n-1);}
    bool pointer(size_t p,size_t& out){
        uint64_t n=0;if(!value(p,n))return false;
        // Disk offsets and native fixups must both remain inside this movie.
        if(n>=MaxOffset){if(!base_||n<base_||n-base_>=MaxOffset)return false;n-=base_;}
        out=static_cast<size_t>(n);return true;
    }
};
// Return only the active method's patch offset. Do not write anything until the
// entire initializer has parsed, one verified route was found and its fallback
// is stock or already ours. Unknown layouts/foreign fallback edits fail closed.
namespace detail {
template<class Read,class Verify>bool locate(Read read,const std::array<unsigned char,32>& replacement,size_t& patch,Verify verify,uint64_t base=0){
    Cursor<Read> c(read,base);std::array<char,16> header{};
    if(!c.bytes(0,header.data(),header.size())||std::memcmp(header.data(),"Apt Data:1:7:8\x1a\0",16))return false;
    uint32_t type=0,frames=0,controls=0;size_t frame=0,array=0,control=0,p=0;
    // This root/control layout is shared by the supported native common movie
    // and AIO Broadcast Beta1. Its action pointer is allowed to move.
    if(!c.value(0x14a8,type)||type!=9||!c.value(0x14c8,frames)||frames!=1||
       !c.pointer(0x14d0,frame)||!c.value(frame,controls)||controls!=69||
       !c.pointer(frame+8,array)||!c.pointer(array+37*8,control)||
       !c.value(control,type)||(type!=1&&type!=8)||!c.pointer(control+(type==8?16:8),p))return false;
    unsigned char first=0;uint32_t constants=0;
    if(!c.value(p,first)||first!=0x88||!c.value(c.aligned(p+1),constants)||constants!=668)return false;
    size_t candidate=0;unsigned matches=0;
    for(unsigned steps=0;steps<4096;++steps){
        unsigned char op=0;if(!c.value(p++,op))return false;
        if(op==0){if(matches!=1)return false;patch=candidate;return true;}
        if(op==0x8e||op==0x9b){
            p=c.aligned(p);size_t name=0,parameters=0;uint32_t argc=0,size=0,registers=0;
            if(!c.pointer(p,name)||!c.value(p+8,argc)||argc>32)return false;
            p+=12;if(op==0x8e){if(!c.value(p,registers))return false;p+=4;}
            p=c.aligned(p);if(!c.pointer(p,parameters))return false;p+=8;
            if(!c.value(p,size)||size>MaxOffset)return false;
            p=c.aligned(p+4);p=c.aligned(p+4)+8;
            if(p>MaxOffset||size>MaxOffset-p)return false;
            if(op==0x8e&&argc==1&&registers==0x002a0003&&size==BodySize){
                Body body{};if(!c.bytes(p,body.data(),body.size()))return false;
                if(verify(body)){
                    if(std::memcmp(body.data()+FallbackOffset,StockFallback.data(),32)&&
                       std::memcmp(body.data()+FallbackOffset,replacement.data(),32))return false;
                    candidate=p+FallbackOffset;++matches;
                }
            }
            p+=size;continue;
        }
        switch(op){
        case 0x87:case 0x99:case 0x9d:p=c.aligned(p)+4;break;
        case 0x88:p=c.aligned(c.aligned(p)+4)+8;break;
        case 0xa1:p=c.aligned(p)+8;break;
        case 0xac:case 0xad:{unsigned char n=0;if(!c.value(p++,n))return false;p+=n;break;}
        case 0xa2:case 0xaf:case 0xb5:case 0xb9:++p;break;
        case 0xa3:case 0xb6:p+=2;break;
        case 0x17:case 0x3d:case 0x40:case 0x4f:case 0x59:case 0x5a:case 0x69:case 0x71:case 0x75:break;
        default:return false;
        }
    }
    return false;
}
}
template<class Read>bool locate(Read read,const std::array<unsigned char,32>& replacement,size_t& patch,uint64_t base=0){
    return detail::locate(read,replacement,patch,verifiedBody,base);
}
}
