#include "../Source/SettingsMenu.h"
#include "../Source/SettingsAptRouting.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
using namespace attendance::settingsmenu;
namespace route=attendance::settingsmenu::routing;
struct Movie {
    std::vector<unsigned char> data=std::vector<unsigned char>(0x7000);
    template<class T>void put(size_t at,T value){assert(at+sizeof(value)<=data.size());std::memcpy(data.data()+at,&value,sizeof(value));}
    void pointer(size_t at,size_t value){put<uint64_t>(at,value);}
    bool read(size_t at,void* out,size_t n)const{if(at>data.size()||n>data.size()-at)return false;std::memcpy(out,data.data()+at,n);return true;}
    Movie(){
        std::memcpy(data.data(),"Apt Data:1:7:8\x1a\0",16);
        put<uint32_t>(0x14a8,9);put<uint32_t>(0x14c8,1);pointer(0x14d0,0x2000);
        put<uint32_t>(0x2000,69);pointer(0x2008,0x2100);pointer(0x2100+37*8,0x2400);
        put<uint32_t>(0x2400,1);pointer(0x2408,0x3000);
        data[0x3000]=0x88;put<uint32_t>(0x3008,668);pointer(0x3010,0x1800);
    }
    size_t method(size_t at){
        data[at++]=0x8e;at=(at+7)&~size_t(7);
        pointer(at,0x1700);put<uint32_t>(at+8,1);put<uint32_t>(at+12,0x002a0003);
        pointer(at+16,0x1710);put<uint32_t>(at+24,static_cast<uint32_t>(route::BodySize));
        size_t body=at+48;
        data[body]=0x59;data[body+1]=0x87;
        std::copy(route::StockFallback.begin(),route::StockFallback.end(),data.begin()+body+route::FallbackOffset);
        data[body+route::BodySize-1]=0x3e;return body;
    }
    bool locate(size_t& offset,uint64_t base=0)const{
        return route::detail::locate([&](size_t at,void* out,size_t n){return read(at,out,n);},categoryFallback(),offset,
            [](const route::Body& body){return body[0]==0x59&&body[1]==0x87&&body.back()==0x3e;},base);
    }
};
int main(int argc,char** argv){
    Movie stock;size_t body=stock.method(0x3018),offset=99;
    assert(stock.locate(offset)&&offset==body+route::FallbackOffset);
    Movie initAction=stock;initAction.put<uint32_t>(0x2400,8);initAction.pointer(0x2410,0x3000);
    assert(initAction.locate(offset)&&offset==body+route::FallbackOffset);
    auto patched=stock;const auto replacement=categoryFallback();
    std::copy(replacement.begin(),replacement.end(),patched.data.begin()+offset);
    assert(patched.locate(offset)); // repeated menu opening is idempotent
    Movie moved;const size_t active=moved.method(0x5018);
    std::copy_n(moved.data.begin()+0x3000,24,moved.data.begin()+0x5000);
    moved.pointer(0x2408,0x5000);moved.method(0x3018); // stale, unreachable stock initializer remains
    assert(moved.locate(offset)&&offset==active+route::FallbackOffset);
    Movie duplicate=stock;duplicate.method(body+route::BodySize);assert(!duplicate.locate(offset));
    Movie foreign=stock;foreign.data[body+route::FallbackOffset]=0x71;assert(!foreign.locate(offset));
    Movie bad=stock;bad.data[body]=0x17;assert(!bad.locate(offset));
    Movie unknown=stock;unknown.data[body+route::BodySize]=0xba;assert(!unknown.locate(offset));
    Movie truncated=stock;truncated.data.resize(body+route::BodySize-1);assert(!truncated.locate(offset));
    Movie pointers=stock;pointers.pointer(0x2408,route::MaxOffset-1);assert(!pointers.locate(offset));
    pointers=stock;pointers.pointer(0x14d0,UINT64_MAX);assert(!pointers.locate(offset));
    Movie fixed=stock;constexpr uint64_t base=0x100000000ull;
    for(size_t at:{size_t(0x14d0),size_t(0x2008),size_t(0x2100+37*8),size_t(0x2408),body-48,body-32}){
        uint64_t value=0;assert(fixed.read(at,&value,8));fixed.put<uint64_t>(at,base+value);
    }
    assert(fixed.locate(offset,base)&&offset==body+route::FallbackOffset);
    assert(!fixed.locate(offset,base+route::MaxOffset));
    Movie unsupported=stock;unsupported.put<uint32_t>(0x2000,70);assert(!unsupported.locate(offset));
    route::Body synthetic{};assert(!route::verifiedBody(synthetic));
    // Optional read-only integration against locally extracted stock/modded APT
    // files. No game or third-party mod assets are included in the repository.
    for(int arg=1;arg<argc;++arg){
        std::ifstream file(argv[arg],std::ios::binary);assert(file);
        std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)),{});assert(!bytes.empty());
        auto read=[&](size_t at,void* out,size_t n){if(at>bytes.size()||n>bytes.size()-at)return false;std::memcpy(out,bytes.data()+at,n);return true;};
        const auto original=bytes;size_t actual=0;
        assert(route::locate(read,replacement,actual));
        std::copy(replacement.begin(),replacement.end(),bytes.begin()+actual);
        size_t again=0;assert(route::locate(read,replacement,again)&&again==actual);
        size_t changed=0;for(size_t i=0;i<bytes.size();++i)if(bytes[i]!=original[i]){assert(i>=actual&&i<actual+32);++changed;}
        assert(changed>0);bytes[actual-1]^=1;assert(!route::locate(read,replacement,again));
        std::cout<<"PASS: active settings routing at 0x"<<std::hex<<actual<<std::dec<<"; 32-byte edit only, existing switch and other UI data preserved: "<<argv[arg]<<"\n";
    }
    std::cout<<"PASS: stock/relocated active initializer selection, native pointer fixups, stale-copy exclusion, idempotence, ambiguous/foreign/unknown/truncated rejection and bounded structural reads\n";
}
