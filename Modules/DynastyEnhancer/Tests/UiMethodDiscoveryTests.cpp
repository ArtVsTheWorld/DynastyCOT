#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../Source/UiMethodDiscovery.h"
#include "../Vendor/json.hpp"
#include <fstream>
#include <iostream>
#include <chrono>
using namespace recruitingfog;
using Json=nlohmann::json;
void require(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
struct Arena {
    std::vector<void*> blocks;
    unsigned char* allocate(size_t size){auto p=static_cast<unsigned char*>(VirtualAlloc(nullptr,size,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));require(p!=nullptr,"fixture allocation");blocks.push_back(p);return p;}
    ~Arena(){for(auto p:blocks)VirtualFree(p,0,MEM_RELEASE);}
};
struct Reader {
    size_t bytes=0,calls=0;
    bool operator()(uintptr_t at,void* out,size_t size){SIZE_T actual=0;++calls;bytes+=size;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(at),out,size,&actual)&&actual==size;}
};
std::vector<unsigned char> binary(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);require(bool(f),"fixture missing");return {std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>()};}
struct Fixture {
    uintptr_t asset=0;std::vector<uintptr_t> definitions;
    Fixture(Arena& arena,const std::filesystem::path& file,const Json& rows,uintptr_t game){
        const auto bytes=binary(file);auto memory=arena.allocate(8*1024*1024);memcpy(memory,bytes.data(),bytes.size());asset=reinterpret_cast<uintptr_t>(memory);
        for(const auto& row:rows){const auto at=row.at("offset").get<size_t>();definitions.push_back(asset+at);
            const auto pool=row.at("pool").get<std::vector<std::string>>();auto values=reinterpret_cast<uintptr_t*>(arena.allocate(pool.size()*8));
            for(size_t i=0;i<pool.size();++i){auto value=arena.allocate(24);auto text=arena.allocate(pool[i].size()+13);const auto vtable=game+0xbc41750;
                memcpy(value,&vtable,8);const auto ptr=reinterpret_cast<uintptr_t>(text);memcpy(value+16,&ptr,8);
                const auto length=static_cast<unsigned short>(pool[i].size());memcpy(text+4,&length,2);memcpy(text+6,&length,2);memcpy(text+12,pool[i].c_str(),pool[i].size()+1);values[i]=reinterpret_cast<uintptr_t>(value);
            }
            const auto count=static_cast<unsigned>(pool.size());const auto address=reinterpret_cast<uintptr_t>(values);memcpy(memory+at+32,&count,4);memcpy(memory+at+40,&address,8);
            for(const auto& p:row.at("pointers")){const auto dest=p[0].get<size_t>();const auto ptr=asset+p[1].get<uintptr_t>();memcpy(memory+at+48+dest,&ptr,8);}
        }
    }
};
int run(int argc,char** argv){
    require(argc==2,"private fixture directory argument required");const std::filesystem::path folder=argv[1];const auto game=reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
    size_t scenes=0;double worstInvoke=0;
    {uidiscovery::Service service;uintptr_t definition=0;uidiscovery::Header h{};
        require(!uidiscovery::invocationSnapshot(nullptr,definition,h),"null invocation accepted");
        require(!uidiscovery::invocationSnapshot(reinterpret_cast<void*>(1),definition,h),"unreadable invocation accepted");
        // The cheap filter must keep every native and owned callback shape,
        // including fixed-frame access and later external dependency groups.
        for(const auto& group:service.surfaces())for(bool fixed:{false,true})for(const auto& key:group->runtime.frameKeys(fixed)){
            h.fill(0);const auto argc_=std::get<0>(key),size=std::get<3>(key);const auto regs=static_cast<unsigned short>(std::get<1>(key)),flags=static_cast<unsigned short>(std::get<2>(key));
            memcpy(h.data()+8,&argc_,4);memcpy(h.data()+12,&regs,2);memcpy(h.data()+14,&flags,2);memcpy(h.data()+24,&size,4);
            require(service.interested(h,fixed),"indexed filter dropped supported frame");
        }
    }
    for(const auto* label:{"vanilla","aio","zeo"}){
        std::ifstream meta(folder/(std::string(label)+".json"));Json rows;meta>>rows;Arena arena;Reader reader;
        Fixture notes(arena,folder/(std::string(label)+"-notes.bin"),rows.at("notes"),game);
        Fixture team(arena,folder/(std::string(label)+"-team.bin"),rows.at("team"),game);
        Fixture currency(arena,folder/(std::string(label)+"-currency.bin"),rows.at("currency"),game);
        uidiscovery::Service service;std::vector<Json> events;
        for(const auto& group:service.surfaces())if(group->recruiting)for(const auto& p:group->programs)require(p.newRegisters==p.registers,"Notes changed a native frame");
        auto read=[&](uintptr_t at,void* out,size_t n){return reader(at,out,n);};
        auto report=[&](const auto& surface,const auto& state,const auto& reason,uintptr_t asset){events.push_back({{"surface",surface},{"state",state},{"reason",reason},{"asset",asset}});std::cout<<events.back().dump()<<std::endl;};
        auto call=[&](uintptr_t def,unsigned scale,bool dynasty=true,unsigned interestMode=0){std::array<unsigned char,0x60> receiver{};memcpy(receiver.data()+0x58,&def,8);uintptr_t current=0;uidiscovery::Header h{};require(uidiscovery::invocationSnapshot(receiver.data(),current,h),"live header snapshot failed");if(service.interested(h,false))service.invokeSnapshot(receiver.data(),current,h,read,false,dynasty,scale,scale,scale,true,true,true,true,true,interestMode);return notesui::field<uintptr_t>(receiver.data(),0x58);};
        uidiscovery::Header first{};read(notes.definitions[0],first.data(),48);
        require(uidiscovery::poolMatches(service.surfaces()[0]->contracts[0],first,read,game),"initial pool contract mismatch");
        require(uidiscovery::declarationMatches(notes.definitions[0],service.surfaces()[0]->contracts[0],read),"initial declaration mismatch");
        require(call(notes.definitions[0],0,false)==notes.definitions[0],"loading prewarm changed native navigation outside Dynasty");
        const auto coldBegin=std::chrono::steady_clock::now();service.tick(read,game,report);service.tick(read,game,report);
        const auto coldMs=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-coldBegin).count();
        for(size_t i=0;i<notes.definitions.size();++i){call(notes.definitions[i],0);service.tick(read,game,report);const auto owned=call(notes.definitions[i],0);require(owned!=notes.definitions[i],"relocated Notes callback not admitted");
            require(call(owned,0,false)==notes.definitions[i],"outside Dynasty failed to restore native Notes method");
            std::array<unsigned char,0x60> receiver{};memcpy(receiver.data()+0x58,&notes.definitions[i],8);
            service.invoke(receiver.data(),read,true,true,0,0,0,true,true,true,true,true);
            require(notesui::field<uintptr_t>(receiver.data(),0x58)!=notes.definitions[i],"fixed-frame path failed to attach Notes");
        }
        call(team.definitions[0],0);service.tick(read,game,report);
        for(unsigned scale=0;scale<=4;++scale){const auto owned=call(team.definitions[0],scale);require(owned!=team.definitions[0],"team body family not admitted");
            const auto sourceSize=notesui::field<unsigned>(reinterpret_cast<unsigned char*>(team.definitions[0]),24);
            require(!memcmp(reinterpret_cast<void*>(team.definitions[0]+48),reinterpret_cast<void*>(owned+48),sourceSize),"team native Draw/currency/layout modified");
            require(call(owned,scale,false)==team.definitions[0],"outside Dynasty failed to restore team native method");
        }
        for(const auto def:currency.definitions){call(def,0);service.tick(read,game,report);const auto owned=call(def,0);require(owned!=def,"relocated recruiting currency callback not admitted");
            uidiscovery::Header h{};read(def,h.data(),48);bool copied=false;
            for(const auto& g:service.surfaces())for(const auto& p:g->programs)if(notesui::headerMatches(p,h)&&!memcmp(reinterpret_cast<void*>(def+48),reinterpret_cast<void*>(owned+48+p.copyAt),p.copyBytes))copied=true;
            require(copied,"recruiting currency original implementation changed");
            for(const auto& group:service.surfaces())if(group->interestPolicy&&notesui::headerMatches(group->programs[0],h)){
                const auto& p=group->programs[0];require(!p.scaleOffsets.empty(),"currency row lost its interest policy");
                for(unsigned mode=0;mode<4;++mode){const auto changed=call(def,2,true,mode);const auto body=reinterpret_cast<unsigned char*>(changed)+48;for(const auto at:p.scaleOffsets)require(body[at]==mode,"interest mode incorrectly followed roster/recruit grade scale");require(call(changed,2,false,mode)==def,"composed currency row escaped Dynasty scope");}
            }
        }
        // No scanning/constant decoding after preparation on invocation paths.
        const auto owned=call(team.definitions[0],2);const auto begin=std::chrono::steady_clock::now();const auto before=reader.bytes;
        for(unsigned n=0;n<10000;++n)require(call(owned,2)==owned,"stable redraw changed definition");
        const auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();worstInvoke=(std::max)(ms,worstInvoke);
        require(reader.bytes==before,"guarded reads leaked into prepared callback snapshots");
        // Foreign class, same frame/body, or a changed source instruction must
        // never gain admission. Original unresolved vanilla copies stay inert.
        const auto& contract=service.surfaces()[0]->contracts[0];uidiscovery::Header h{};read(notes.definitions[0],h.data(),48);
        auto wrong=contract;wrong.prefix[1]="unrelated";require(!uidiscovery::poolMatches(wrong,h,read,game),"foreign class admitted");
        wrong=contract;++wrong.methodIndex;require(!uidiscovery::declarationMatches(notes.definitions[0],wrong,read),"foreign method admitted");
        auto unreadable=[](uintptr_t,void*,size_t){return false;};require(!uidiscovery::poolMatches(contract,h,unreadable,game),"unavailable pool accepted");
        for(const auto& event:events)require(event.at("state")=="ready","valid fixture not ready");
        require(events.size()>=2,"missing surface readiness diagnostics");
        ++scenes;std::cout<<label<<": all 13 Notes methods, team scales/Blind, native restore, exact Draw copy; cold worker "<<coldMs<<" ms; 10000 warm invokes "<<ms<<" ms\n";
    }
    // Reproduce a real first opening: Navigation has already executed native
    // before the worker. Board.Init must verify all dependencies on that same
    // opening, then its prefix can repair the already-created three-tab array.
    for(const auto* label:{"vanilla","aio","zeo"})for(bool fixed:{false,true}){
        std::ifstream f(folder/(std::string(label)+".json"));Json rows;f>>rows;Arena arena;Reader r;Fixture notes(arena,folder/(std::string(label)+"-notes.bin"),rows.at("notes"),game);uidiscovery::Service service;
        auto read=[&](uintptr_t p,void* b,size_t n){return r(p,b,n);};
        auto call=[&](uintptr_t def){std::array<unsigned char,0x60> o{};memcpy(o.data()+0x58,&def,8);service.invoke(o.data(),read,fixed,true,0,0,0,true,true,true,true,true);return notesui::field<uintptr_t>(o.data(),0x58);};
        std::array<unsigned char,0x60> preload{};memcpy(preload.data()+0x58,&notes.definitions[0],8);
        service.invoke(preload.data(),read,fixed,false,0,0,0,true,true,true,true,true);
        require(notesui::field<uintptr_t>(preload.data(),0x58)==notes.definitions[0],"preload changed native navigation outside Dynasty");
        const auto begin=std::chrono::steady_clock::now();const auto owned=call(notes.definitions[1]);
        const auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();
        require(owned!=notes.definitions[1],"first Init still depended on a worker tick");
        for(size_t i=0;i<10;++i)require(call(notes.definitions[i])!=notes.definitions[i],"opening admitted a partial dependency group");
        const auto& init=service.surfaces()[0]->programs[1];require(!memcmp(reinterpret_cast<void*>(notes.definitions[1]+48),reinterpret_cast<void*>(owned+48+init.copyAt),init.size),"opening changed native Board.Init implementation");
        const auto warmBytes=r.bytes;for(unsigned n=0;n<1000;++n)require(call(owned)==owned,"warm Init changed definition");
        require(r.bytes-warmBytes<=1000*256,"opening catalogue ran again on a warm method");
        std::cout<<label<<" first opening (fixed="<<fixed<<"): "<<ms<<" ms; full dependency group ready before Init, no worker tick\n";
        uidiscovery::Service navigation;std::array<unsigned char,0x60> nativeNavigation{};memcpy(nativeNavigation.data()+0x58,&notes.definitions[0],8);
        navigation.invoke(nativeNavigation.data(),read,fixed,true,0,0,0,true,true,true,true,true);
        require(notesui::field<uintptr_t>(nativeNavigation.data(),0x58)!=notes.definitions[0],"first eligible Navigation still depended on a worker tick");
    }
    {std::ifstream f(folder/"zeo.json");Json rows;f>>rows;Arena arena;Reader r;Fixture notes(arena,folder/"zeo-notes.bin",rows.at("notes"),game);uidiscovery::Service service;
        *reinterpret_cast<unsigned char*>(notes.definitions[3]+48)^=1;
        auto read=[&](uintptr_t p,void* b,size_t n){return r(p,b,n);};std::array<unsigned char,0x60> o{};memcpy(o.data()+0x58,&notes.definitions[1],8);
        service.invoke(o.data(),read,true,true,0,0,0,true,true,true,true,true);require(notesui::field<uintptr_t>(o.data(),0x58)==notes.definitions[1],"opening bypassed a failed required fingerprint");
        *reinterpret_cast<unsigned char*>(notes.definitions[3]+48)^=1;*reinterpret_cast<unsigned char*>(notes.definitions[10]+48)^=1;
        uidiscovery::Service optional;optional.invoke(o.data(),read,true,true,0,0,0,true,true,true,true,true);require(notesui::field<uintptr_t>(o.data(),0x58)!=notes.definitions[1],"optional attribute failure blocked first opening");
    }
    {std::ifstream f(folder/"zeo.json");Json rows;f>>rows;Arena arena;Reader r;Fixture notes(arena,folder/"zeo-notes.bin",rows.at("notes"),game);uidiscovery::Service service;
        const auto count=rows.at("notes")[0].at("size").get<size_t>();
        memcpy(reinterpret_cast<void*>(notes.asset+1024*1024),reinterpret_cast<void*>(notes.definitions[0]-16),16+48+count);
        auto read=[&](uintptr_t p,void* b,size_t n){return r(p,b,n);};std::array<unsigned char,0x60> o{};memcpy(o.data()+0x58,&notes.definitions[1],8);
        service.invoke(o.data(),read,true,true,0,0,0,true,true,true,true,true);require(notesui::field<uintptr_t>(o.data(),0x58)==notes.definitions[1],"ambiguous first-opening declaration admitted");
    }
    // Loader placeholders recover when the actual class is resolved. A fresh
    // generation is independently checked; an optional grade failure cannot
    // disable Notes. All memory in this fixture belongs to this test process.
    {std::ifstream f(folder/"zeo.json");Json rows;f>>rows;Arena arena;Reader r;Fixture notes(arena,folder/"zeo-notes.bin",rows.at("notes"),game);uidiscovery::Service service;
        auto read=[&](uintptr_t p,void* b,size_t n){return r(p,b,n);};std::array<unsigned char,0x60> owner{};auto invoke=[&](uintptr_t def){memcpy(owner.data()+0x58,&def,8);service.invoke(owner.data(),read,false,true,0,0,0,true,true,true,true,true);return notesui::field<uintptr_t>(owner.data(),0x58);};
        auto tick=[&](){service.tick(read,game,[](const auto&,const auto&,const auto&,uintptr_t){});};
        unsigned saved=0;memcpy(&saved,reinterpret_cast<void*>(notes.definitions[0]+32),4);const unsigned placeholder=0x98765432;memcpy(reinterpret_cast<void*>(notes.definitions[0]+32),&placeholder,4);
        invoke(notes.definitions[0]);tick();require(invoke(notes.definitions[0])==notes.definitions[0],"unresolved pool admitted");
        memcpy(reinterpret_cast<void*>(notes.definitions[0]+32),&saved,4);*reinterpret_cast<unsigned char*>(notes.definitions[10]+48)^=1;
        invoke(notes.definitions[0]);tick();require(invoke(notes.definitions[0])!=notes.definitions[0],"resolved pool did not recover or optional grade blocked Notes");
        Fixture fresh(arena,folder/"zeo-notes.bin",rows.at("notes"),game);invoke(fresh.definitions[0]);tick();require(invoke(fresh.definitions[0])!=fresh.definitions[0],"fresh asset generation reused stale negative result");
    }
    // Reproduce the live startup burst: 64 broad shape-only candidates must
    // not discard the one-time Navigation/Draw definitions at the tail.
    {std::ifstream f(folder/"zeo.json");Json rows;f>>rows;Arena arena;Reader r;Fixture notes(arena,folder/"zeo-notes.bin",rows.at("notes"),game);uidiscovery::Service service;
        auto read=[&](uintptr_t p,void* b,size_t n){return r(p,b,n);};
        auto call=[&](uintptr_t def,bool dynasty=true){std::array<unsigned char,0x60> o{};memcpy(o.data()+0x58,&def,8);service.invoke(o.data(),read,false,dynasty,0,0,0,true,true,true,true,true);return notesui::field<uintptr_t>(o.data(),0x58);};
        for(unsigned i=0;i<64;++i){auto b=arena.allocate(4096);unsigned argc_=1,size=7;unsigned short regs=3,flags=41;memcpy(b+8,&argc_,4);memcpy(b+12,&regs,2);memcpy(b+14,&flags,2);memcpy(b+24,&size,4);call(reinterpret_cast<uintptr_t>(b));}
        Fixture team(arena,folder/"zeo-team.bin",rows.at("team"),game);
        call(notes.definitions[0],false);call(team.definitions[0]);require(service.preemptedCount()==2,"exact one-shot callbacks were discarded behind broad candidates");
        service.tick(read,game,[](const auto&,const auto&,const auto&,uintptr_t){});require(call(notes.definitions[0])!=notes.definitions[0],"startup burst still prevented Notes preparation");
        require(call(team.definitions[0])!=team.definitions[0],"startup burst still prevented the team header");
    }
    // Any reviewed recruiting callback can seed the asset catalogue. Notes
    // must be ready even when its own navigation callback was never observed.
    {std::ifstream f(folder/"zeo.json");Json rows;f>>rows;Arena arena;Reader r;auto combined=rows.at("notes");for(const auto& row:rows.at("currency"))combined.push_back(row);
        Fixture all(arena,folder/"zeo-notes.bin",combined,game);uidiscovery::Service service;auto read=[&](uintptr_t p,void* b,size_t n){return r(p,b,n);};
        auto call=[&](uintptr_t def){std::array<unsigned char,0x60> o{};memcpy(o.data()+0x58,&def,8);service.invoke(o.data(),read,false,true,0,0,0,true,true,true,true,true);return notesui::field<uintptr_t>(o.data(),0x58);};
        call(all.definitions[14]);service.tick(read,game,[](const auto&,const auto&,const auto&,uintptr_t){});
        require(call(all.definitions[0])!=all.definitions[0],"currency asset seed did not prepare unobserved Notes dependency group");
    }
    // A single required dependency mutation blocks Notes as a group, while
    // unrelated team headers and optional grade focus remain independent.
    {std::ifstream f(folder/"zeo.json");Json rows;f>>rows;Arena arena;Reader r;Fixture notes(arena,folder/"zeo-notes.bin",rows.at("notes"),game);uidiscovery::Service service;
        *reinterpret_cast<unsigned char*>(notes.definitions[3]+48)^=1;std::array<unsigned char,0x60> owner{};memcpy(owner.data()+0x58,&notes.definitions[0],8);
        auto read=[&](uintptr_t p,void* b,size_t n){return r(p,b,n);};service.invoke(owner.data(),read,false,true,0,0,0,true,true,true,true,true);
        bool rejected=false;service.tick(read,game,[&](const auto&,const auto& state,const auto&,uintptr_t){rejected|=std::string(state)=="unsupported";});
        require(rejected,"changed callback dependency not diagnosed");service.invoke(owner.data(),read,false,true,0,0,0,true,true,true,true,true);require(notesui::field<uintptr_t>(owner.data(),0x58)==notes.definitions[0],"partial Notes bundle attached");
    }
    // A different verified Dynasty class in CoreNode can seed the catalogue
    // before TeamModuleHorizontal.Draw is ever called. Namespace recognition
    // must not relax the target's exact body/class/constant validation.
    {std::ifstream f(folder/"zeo.json"),seedFile(folder/"zeo-team-seed.json");Json rows,seed;f>>rows;seedFile>>seed;auto combined=rows.at("team");combined.push_back(seed);
        Arena arena;Reader r;Fixture all(arena,folder/"zeo-team.bin",combined,game);uidiscovery::Service service;
        auto read=[&](uintptr_t p,void* b,size_t n){return r(p,b,n);};auto call=[&](uintptr_t def){std::array<unsigned char,0x60> owner{};memcpy(owner.data()+0x58,&def,8);service.invoke(owner.data(),read,false,true,0,0,0,true,true,true,true,true);return notesui::field<uintptr_t>(owner.data(),0x58);};
        call(all.definitions[1]);service.tick(read,game,[](const auto&,const auto&,const auto&,uintptr_t){});require(call(all.definitions[0])!=all.definitions[0],"CoreNode seed did not prepare unobserved team header");
    }
    // TeamRatings marks only team data, then the generic circular control's
    // Draw and animation callbacks can format that explicit marker. Each is
    // pointer-free, independently verified and frame-preserving on first use.
    {std::ifstream f(folder/"circles.json");Json rows;f>>rows;Arena arena;Reader r;uidiscovery::Service service;
        auto read=[&](uintptr_t p,void* b,size_t n){return r(p,b,n);};
        auto call=[&](uintptr_t def,bool dynasty=true){std::array<unsigned char,0x60> o{};memcpy(o.data()+0x58,&def,8);service.invoke(o.data(),read,true,dynasty,4,4,1,true,false,false,true,true);return notesui::field<uintptr_t>(o.data(),0x58);};
        for(size_t i=0;i<rows.size();++i){Fixture fixture(arena,folder/("circle-"+std::to_string(i)+".bin"),Json::array({rows[i]}),game);const auto native=fixture.definitions[0];
            const auto coldBegin=std::chrono::steady_clock::now();
            require(call(native,false)==native,"cold circle changed a native non-Dynasty method");
            const auto coldMs=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-coldBegin).count();
            const auto owned=call(native);require(owned!=native,"first-use circle did not attach before its one-time callback");
            const auto& p=service.surfaces()[service.surfaces().size()-3+i]->programs[0];
            require(p.newRegisters==p.registers,"circle changed native frame");
            require(!memcmp(reinterpret_cast<void*>(native+48),reinterpret_cast<void*>(owned+48+p.copyAt),p.size),"circle native implementation changed");
            require(call(owned,false)==native,"circle failed native scope restoration");
            const auto begin=std::chrono::steady_clock::now();const auto before=r.bytes;
            for(unsigned n=0;n<10000;++n)require(call(owned)==owned,"warm circle changed definition");
            require(r.bytes-before<=10000*256,"warm circle decoded constants or scanned an asset");
            const auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();
            std::cout<<"Circle "<<rows[i].at("method")<<": first-use verification "<<coldMs<<" ms; 10000 warm callbacks "<<ms<<" ms\n";
            Fixture changed(arena,folder/("circle-"+std::to_string(i)+".bin"),Json::array({rows[i]}),game);
            *reinterpret_cast<unsigned char*>(changed.definitions[0]+48)^=1;
            require(call(changed.definitions[0])==changed.definitions[0],"changed circle fingerprint admitted");
        }
    }
    // The loader can resolve a pool entry in place, leaving the method header
    // identical. Such temporarily unavailable constants must remain retryable.
    {std::ifstream f(folder/"zeo.json");Json rows;f>>rows;Arena arena;Reader r;Fixture notes(arena,folder/"zeo-notes.bin",rows.at("notes"),game);uidiscovery::Service service;
        auto read=[&](uintptr_t p,void* b,size_t n){return r(p,b,n);};
        auto call=[&](){std::array<unsigned char,0x60> o{};memcpy(o.data()+0x58,&notes.definitions[0],8);service.invoke(o.data(),read,false,true,0,0,0,true,true,true,true,true);return notesui::field<uintptr_t>(o.data(),0x58);};
        const auto pool=notesui::field<uintptr_t>(reinterpret_cast<unsigned char*>(notes.definitions[0]),40);uintptr_t original=0;memcpy(&original,reinterpret_cast<void*>(pool),8);memset(reinterpret_cast<void*>(pool),0,8);
        call();service.tick(read,game,[](const auto&,const auto&,const auto&,uintptr_t){});
        require(call()==notes.definitions[0],"unreadable in-place constant admitted");
        memcpy(reinterpret_cast<void*>(pool),&original,8);Sleep(1100);call();service.tick(read,game,[](const auto&,const auto&,const auto&,uintptr_t){});
        require(call()!=notes.definitions[0],"in-place pool resolution remained negatively cached");
    }
    // The previous 1024-entry cache could churn through startup candidates on
    // every callback. Pending and dropped work must remain bounded and cached.
    {Arena arena;Reader r;uidiscovery::Service service;auto slab=arena.allocate(2048*64);
        auto read=[&](uintptr_t p,void* b,size_t n){return r(p,b,n);};
        auto call=[&](uintptr_t def){std::array<unsigned char,0x60> o{};memcpy(o.data()+0x58,&def,8);service.invoke(o.data(),read,false,false,0,0,0,true,true,true,true,true);};
        for(unsigned i=0;i<2048;++i){auto b=slab+i*64;unsigned argc_=1,size=7;unsigned short regs=3,flags=41;memcpy(b+8,&argc_,4);memcpy(b+12,&regs,2);memcpy(b+14,&flags,2);memcpy(b+24,&size,4);call(reinterpret_cast<uintptr_t>(b));}
        const auto dropped=service.droppedCount();require(dropped==2048-64,"unexpected bounded queue capacity");
        for(unsigned repeat=0;repeat<4;++repeat)for(unsigned i=0;i<2048;++i)call(reinterpret_cast<uintptr_t>(slab+i*64));
        require(service.droppedCount()==dropped,"unchanged pending/dropped definitions repeatedly flooded queue");
    }
    // Practice banners use their actual typed owner, including relocated and
    // newly loaded copies. Neither another school scale nor a saved offset wins.
    for(const auto* name:{"week-module","weekly-tile"})for(bool fixed:{false,true}){
        std::ifstream meta(folder/(std::string(name)+".json"));Json rows;meta>>rows;Arena arena;Reader reader;
        Fixture fixture(arena,folder/(std::string(name)+".bin"),rows,game);uidiscovery::Service service;
        auto read=[&](uintptr_t p,void* out,size_t n){return reader(p,out,n);};
        const auto source=fixture.definitions[0];const auto n=notesui::field<unsigned>(reinterpret_cast<unsigned char*>(source),24);
        const auto relocated=fixture.asset+2*1024*1024;memcpy(reinterpret_cast<void*>(relocated-16),reinterpret_cast<void*>(source-16),16+48+n);
        for(const auto definition:{source,relocated}){
            std::array<unsigned char,0x60> object{};memcpy(object.data()+0x58,&definition,8);
            service.invoke(object.data(),read,fixed,true,4,3,0,false,false,false,true,false);
            const auto attached=notesui::field<uintptr_t>(object.data(),0x58);require(attached!=definition,"practice banner relocated/first-use binding failed");
            require(!memcmp(reinterpret_cast<void*>(attached+48),reinterpret_cast<void*>(definition+48),n),"practice banner native instructions changed");
            service.invoke(object.data(),read,fixed,false,4,3,0,false,false,false,true,false);
            require(notesui::field<uintptr_t>(object.data(),0x58)==definition,"practice banner escaped Dynasty scope");
        }
    }
    std::cout<<"PASS: "<<scenes<<" real asset families, bounded discovery, dependency isolation, scope restore, first-use circles and 2048-candidate cache. Worst warm callback "<<worstInvoke/10000<<" ms\n";
    return 0;
}
int main(int argc,char** argv){try{return run(argc,argv);}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
