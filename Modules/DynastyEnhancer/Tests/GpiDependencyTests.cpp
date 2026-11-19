// Own-process native-layout fixtures only; never opens or patches the game.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef GPI_BEFORE_HEADER
#include GPI_BEFORE_HEADER
#else
#include "UiMethodDiscovery.h"
#endif
#include "AnalyticsActionsUiPrograms.h"
#include "UiAdapterContracts.generated.h"
#include "../Vendor/json.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>
#include <algorithm>
using namespace recruitingfog;using J=nlohmann::json;
void require(bool yes,const char* why){if(!yes)throw std::runtime_error(why);}
struct Arena {
 std::vector<void*> blocks;
 unsigned char* alloc(size_t n){auto p=static_cast<unsigned char*>(VirtualAlloc(nullptr,n,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));require(p!=nullptr,"fixture allocation failed");blocks.push_back(p);return p;}
 ~Arena(){for(auto p:blocks)VirtualFree(p,0,MEM_RELEASE);}
};
struct Reader {size_t calls=0,bytes=0;bool operator()(uintptr_t p,void* out,size_t n){++calls;bytes+=n;SIZE_T got=0;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(p),out,n,&got)&&got==n;}};
struct Fixture {
 uintptr_t asset;std::vector<uintptr_t> defs;J rows;
 Fixture(Arena& a,const std::filesystem::path& folder,const char* name,uintptr_t game){
  std::ifstream meta(folder/(std::string(name)+".json"));meta>>rows;std::ifstream f(folder/(std::string(name)+".bin"),std::ios::binary);require(bool(f),"fixture missing");std::vector<unsigned char> bytes{std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>()};require(bytes.size()<2*1024*1024,"fixture exceeds opening cap");auto blob=a.alloc(8*1024*1024);memcpy(blob,bytes.data(),bytes.size());asset=reinterpret_cast<uintptr_t>(blob);
  for(const auto& r:rows){auto h=blob+r.at("offset").get<size_t>();defs.push_back(reinterpret_cast<uintptr_t>(h));const auto pool=r.at("pool").get<std::vector<std::string>>();auto values=reinterpret_cast<uintptr_t*>(a.alloc(pool.size()*8));
   for(size_t i=0;i<pool.size();++i){auto v=a.alloc(24),s=a.alloc(13+pool[i].size());const auto vt=game+0xbc416f0,p=reinterpret_cast<uintptr_t>(s);memcpy(v,&vt,8);memcpy(v+16,&p,8);const auto len=static_cast<unsigned short>(pool[i].size());memcpy(s+4,&len,2);memcpy(s+6,&len,2);memcpy(s+12,pool[i].c_str(),pool[i].size()+1);values[i]=reinterpret_cast<uintptr_t>(v);}
   const auto count=static_cast<unsigned>(pool.size());const auto ptr=reinterpret_cast<uintptr_t>(values);memcpy(h+32,&count,4);memcpy(h+40,&ptr,8);
   for(const auto& p:r.at("pointers")){const auto at=p[0].get<size_t>(),value=asset+p[1].get<uintptr_t>();memcpy(h+48+at,&value,8);}
   for(const auto& n:r.at("nested")){auto nh=h+48+n.at("offset").get<size_t>();const auto namePtr=asset+n.at("name").get<uintptr_t>(),params=asset+n.at("params").get<uintptr_t>();memcpy(nh,&namePtr,8);memcpy(nh+16,&params,8);for(unsigned i=0;i<notesui::field<unsigned>(nh,8);++i){auto arg=reinterpret_cast<unsigned char*>(params)+i*16;const auto value=asset+notesui::field<uintptr_t>(arg,8);memcpy(arg+8,&value,8);}}
  }
 }
};
struct Context {
 notesui::Runtime standard{analyticsactions::hubMainPrograms()},extended{analyticsactions::currentMainPrograms()};uidiscovery::Service service;std::vector<J> events;Reader reader;
 Context(){auto a=uidiscovery::configureAdapter("card.47",standard,true),b=uidiscovery::configureAdapter("card.48",extended,true);standard.prepare();extended.prepare();
#ifdef GPI_BEFORE_HEADER
  service.registerDependency("GPI standard",a,standard);service.registerDependency("GPI extended",b,extended);
#else
  service.registerDependency("GPI standard",a,standard,"GPI navigation");service.registerDependency("GPI extended",b,extended,"GPI navigation");
#endif
 }
 uintptr_t invoke(uintptr_t def,bool dynasty=true){std::array<unsigned char,0x60> o{};memcpy(o.data()+0x58,&def,8);uidiscovery::Header h{};memcpy(h.data(),reinterpret_cast<void*>(def),48);service.invokeSnapshot(o.data(),def,h,[&](uintptr_t p,void* b,size_t n){return reader(p,b,n);},false,dynasty,0,0,0,true,true,true,true,true);return notesui::field<uintptr_t>(o.data(),0x58);}
 void tick(){service.tick([&](uintptr_t p,void* b,size_t n){return reader(p,b,n);},reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr)),[&](const auto& surface,const auto& state,const auto& reason,uintptr_t asset){events.push_back({{"surface",surface},{"state",state},{"reason",reason},{"asset",asset}});});}
 unsigned failures(){unsigned n=0;for(const auto& e:events)if(e.at("state")=="unavailable"||e.at("state")=="unsupported"||e.at("state")=="waiting")++n;return n;}
};
int main(int argc,char** argv){try{
 require(argc==2,"private fixture folder required");const std::filesystem::path folder=argv[1];const auto game=reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));J results=J::array();
 for(const auto* name:{"standard","extended","relocated"}){
  Arena a;Fixture f(a,folder,name,game);Context c;const auto t=std::chrono::steady_clock::now();require(c.invoke(f.defs[0])==f.defs[0],"preparation changed an ordinary native receiver");const auto coldUs=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-t).count();const auto coldBytes=c.reader.bytes;c.tick();auto& winner=std::string(name)=="standard"?c.standard:c.extended;auto& loser=std::string(name)=="standard"?c.extended:c.standard;(void)loser;
  for(auto def:f.defs){uidiscovery::Header h{};memcpy(h.data(),reinterpret_cast<void*>(def),48);require(winner.admitted(def,h),"selected complete alternative not admitted atomically");}
#ifdef GPI_BEFORE_HEADER
  require(c.failures()>0,"before fixture did not reproduce false sibling warning");
#else
  require(loser.ownerCount()==0,"unused alternative redirected an object");require(c.failures()==0,"supported alternative logged a false failure");
#endif
  const auto before=c.reader.bytes;const auto warmStart=std::chrono::steady_clock::now();for(unsigned n=0;n<20;++n)c.invoke(f.defs[0]);const auto nativeWarmUs=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-warmStart).count()/20;const auto warmBytes=c.reader.bytes-before;
#ifndef GPI_BEFORE_HEADER
  require(warmBytes==0,"warm native Init repeated catalogue/constant reads");
#endif
  std::array<unsigned char,0x60> obj{};auto def=f.defs[0];memcpy(obj.data()+0x58,&def,8);auto read=[&](uintptr_t p,void* b,size_t n){return c.reader(p,b,n);};winner.beforeInvoke(obj.data(),read,false,true);const auto owned=notesui::field<uintptr_t>(obj.data(),0x58);require(owned!=def,"verified GPI Init failed to attach");const auto native=winner.discoveryPrograms()[0];require(!memcmp(reinterpret_cast<void*>(owned+48+native.copyAt),reinterpret_cast<void*>(def+48),native.copyBytes),"native Init prefix changed");const auto bytesBefore=c.reader.bytes;for(unsigned n=0;n<1000;++n)require(c.invoke(owned)==owned,"preparation rewrote an owned callback");c.tick();const auto ownedWarmBytes=c.reader.bytes-bytesBefore;
#ifndef GPI_BEFORE_HEADER
  require(ownedWarmBytes==0,"owned callback queued a redundant asset search");uidiscovery::Header h{};memcpy(h.data(),reinterpret_cast<void*>(owned),48);require(winner.admitted(owned,h,true),"owned immutable definition not recognized");h[24]^=1;require(!winner.admitted(owned,h,true),"owned pointer accepted a changed header");
#endif
  winner.beforeInvoke(obj.data(),read,false,false);require(notesui::field<uintptr_t>(obj.data(),0x58)==def,"outside Dynasty did not restore native Init");
  results.push_back({{"fixture",name},{"coldUs",coldUs},{"coldReadBytes",coldBytes},{"nativeWarmUs",nativeWarmUs},{"nativeWarmReadBytes",warmBytes},{"ownedWarmAndTickReadBytes",ownedWarmBytes},{"warningEvents",c.failures()}});
 }
#ifndef GPI_BEFORE_HEADER
 for(const auto* name:{"ambiguous","duplicate"}){Arena a;Fixture f(a,folder,name,game);Context c;c.invoke(f.defs[0]);c.tick();uidiscovery::Header h{};memcpy(h.data(),reinterpret_cast<void*>(f.defs[0]),48);require(!c.standard.admitted(f.defs[0],h)&&!c.extended.admitted(f.defs[0],h),"ambiguous dependency family was admitted");require(c.failures()>0,"ambiguous family lacked actionable diagnostics");}
 for(bool changedSize:{false,true}){Arena a;Fixture f(a,folder,"ambiguous",game);Context c;
  if(changedSize){auto size=notesui::field<unsigned>(reinterpret_cast<unsigned char*>(f.defs[1]),24)-1;memcpy(reinterpret_cast<void*>(f.defs[1]+24),&size,4);}else *reinterpret_cast<unsigned char*>(f.defs[1]+48)^=1;
  c.invoke(f.defs[0]);c.tick();uidiscovery::Header h{};memcpy(h.data(),reinterpret_cast<void*>(f.defs[0]),48);require(!c.standard.admitted(f.defs[0],h)&&!c.extended.admitted(f.defs[0],h),"a valid sibling hid an unknown second Input owner");require(c.failures()>0,"unknown duplicate owner was not diagnosed");
 }
 {Arena a;Fixture f(a,folder,"extended",game);Context c;*reinterpret_cast<unsigned char*>(f.defs[1]+48)^=1;c.invoke(f.defs[0]);c.tick();uidiscovery::Header h{};memcpy(h.data(),reinterpret_cast<void*>(f.defs[0]),48);require(!c.standard.admitted(f.defs[0],h)&&!c.extended.admitted(f.defs[0],h),"modified Input was admitted");require(c.failures()>0,"modified Input was silently hidden");}
 {Arena a;Fixture f(a,folder,"extended",game);Context c;const auto pool=notesui::field<uintptr_t>(reinterpret_cast<unsigned char*>(f.defs[1]),40);memset(reinterpret_cast<void*>(f.defs[1]+40),0,8);c.invoke(f.defs[0]);c.tick();uidiscovery::Header h{};memcpy(h.data(),reinterpret_cast<void*>(f.defs[0]),48);require(!c.extended.admitted(f.defs[0],h),"partial group published");memcpy(reinterpret_cast<void*>(f.defs[1]+40),&pool,8);Sleep(1100);c.invoke(f.defs[0]);c.tick();require(c.extended.admitted(f.defs[0],h),"in-place loading recovery failed");}
 {Arena a;Fixture x(a,folder,"extended",game),y(a,folder,"standard",game);Context c;c.invoke(x.defs[0]);c.invoke(y.defs[0]);c.tick();const auto bytes=c.reader.bytes;c.invoke(x.defs[0]);c.invoke(y.defs[0]);c.tick();require(c.reader.bytes==bytes,"selected family leaked across asset generations");require(c.failures()==0,"valid generation switch logged false warnings");}
 {Arena a;Fixture f(a,folder,"extended",game);Context c;c.invoke(f.defs[0],false);uidiscovery::Header h{};memcpy(h.data(),reinterpret_cast<void*>(f.defs[0]),48);require(!c.extended.admitted(f.defs[0],h),"Dynasty-only opening preparation escaped scope");}
 {Arena a;Fixture f(a,folder,"extended",game);Context c;c.invoke(f.defs[2]);c.tick();require(c.failures()==0,"worker selected unused sibling as failed requirement");const auto bytes=c.reader.bytes;for(unsigned i=0;i<25;++i)c.tick();require(c.reader.bytes==bytes,"worker retained a false incomplete-family retry");}
#endif
 std::cout<<J{{"pass",true},{"fixtures",results},{"gameAccess",false},{"fingerprintsBypassed",false}}.dump(2)<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
