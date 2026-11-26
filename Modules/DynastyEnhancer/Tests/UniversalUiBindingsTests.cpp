// Offline fixtures only. Never calls a plugin entry point or installs hooks.
#include "../Source/RecruitingFog.cpp"
#include <iostream>
using namespace recruitingfog;
using J=nlohmann::json;
void check(bool yes,const char* why){if(!yes)throw std::runtime_error(why);}
struct Arena {
    std::vector<void*> blocks;
    unsigned char* alloc(size_t n){auto p=static_cast<unsigned char*>(VirtualAlloc(nullptr,n,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));check(p!=nullptr,"allocation failed");blocks.push_back(p);return p;}
    ~Arena(){for(auto p:blocks)VirtualFree(p,0,MEM_RELEASE);}
};
struct Reader {
    size_t calls=0,bytes=0;
    bool operator()(uintptr_t p,void* out,size_t n){++calls;bytes+=n;SIZE_T got=0;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(p),out,n,&got)&&got==n;}
};
std::vector<unsigned char> binary(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"missing fixture");return {std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>()};}
notesui::Runtime& bank(const std::string& name){
    if(name=="grid.legacy")return runtime::playerUi;
    if(name=="grid.modern")return runtime::modernPlayerUi;
    const auto index=std::stoul(name.substr(name.find('.')+1));
    return name.rfind("card.",0)==0?*runtime::cardUiBanks.at(index):*features::banks.at(index);
}
int main(int argc,char** argv){try{
    check(argc==2,"private fixture folder required");const std::filesystem::path folder=argv[1];std::ifstream f(folder/"methods.json");J cases;f>>cases;
    runtime::configureUniversalUi();size_t methods=0,policies=0,cacheBytes=0;double coldWorst=0,warmMicros=0;
    for(const auto& scene:cases){
        const auto name=scene.at("bank").get<std::string>();auto& rt=bank(name);rt.prepare();cacheBytes+=rt.displayCacheBytes();Arena arena;Reader reader;
        std::map<std::string,unsigned char*> assets;std::vector<uintptr_t> defs;
        for(const auto& row:scene.at("methods")){
            const auto file=row.at("file").get<std::string>();auto& data=assets[file];if(!data){const auto bytes=binary(folder/file);data=arena.alloc(bytes.size());memcpy(data,bytes.data(),bytes.size());}
            const auto offset=row.at("offset").get<size_t>();auto h=data+offset;defs.push_back(reinterpret_cast<uintptr_t>(h));
            auto pool=row.at("poolHex").get<std::vector<std::string>>();for(auto& text:pool){std::string bytes;for(size_t i=0;i<text.size();i+=2)bytes+=static_cast<char>(std::stoul(text.substr(i,2),nullptr,16));text=std::move(bytes);}auto values=reinterpret_cast<uintptr_t*>(arena.alloc(pool.size()*8));
            for(size_t i=0;i<pool.size();++i){auto v=arena.alloc(24),s=arena.alloc(13+pool[i].size());const auto table=reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr))+0xbc416f0;memcpy(v,&table,8);const auto ptr=reinterpret_cast<uintptr_t>(s);memcpy(v+16,&ptr,8);const auto len=static_cast<unsigned short>(pool[i].size());memcpy(s+4,&len,2);memcpy(s+6,&len,2);memcpy(s+12,pool[i].c_str(),pool[i].size()+1);values[i]=reinterpret_cast<uintptr_t>(v);}
            const auto count=static_cast<unsigned>(pool.size());const auto poolPtr=reinterpret_cast<uintptr_t>(values);memcpy(h+32,&count,4);memcpy(h+40,&poolPtr,8);
            for(const auto& pointer:row.at("pointers")){const auto at=pointer.at("offset").get<size_t>();const auto value=reinterpret_cast<uintptr_t>(data)+pointer.at("value").get<uintptr_t>();memcpy(h+48+at,&value,8);}
            for(const auto& nested:row.at("nested")){
                auto n=h+48+nested.at("offset").get<size_t>();const auto nName=reinterpret_cast<uintptr_t>(data)+nested.at("name").get<uintptr_t>(),params=reinterpret_cast<uintptr_t>(data)+nested.at("params").get<uintptr_t>();memcpy(n,&nName,8);memcpy(n+16,&params,8);
                for(unsigned i=0;i<notesui::field<unsigned>(n,8);++i){auto a=reinterpret_cast<unsigned char*>(params)+i*16;const auto original=notesui::field<uintptr_t>(a,8);const auto absolute=reinterpret_cast<uintptr_t>(data)+original;memcpy(a+8,&absolute,8);}
                if(!nested.at("emptyPool").get<bool>()){memcpy(n+32,&count,4);memcpy(n+40,&poolPtr,8);}
            }
        }
        auto read=[&](uintptr_t p,void* out,size_t n){return reader(p,out,n);};
        if(name=="card.66"){
            check(defs.size()==1,"Stats producer must be an isolated one-method surface");
            const auto& program=rt.discoveryPrograms().front();
            check(program.size==452&&program.hash==0x0A5512E1CF3D4E6Eull&&program.newRegisters==program.registers,"Stats producer contract changed");
            // A changed body or a different native skill provider must not gain
            // rating permission merely by retaining the same callback frame.
            auto reject=[&](){
                notesui::Runtime isolated(playeringameui::statsPrograms());
                uidiscovery::configureAdapter("card.66",isolated);isolated.prepare();
                std::array<unsigned char,0x60> object{};memcpy(object.data()+0x58,&defs.front(),8);
                try{isolated.beforeInvoke(object.data(),read,false,true,0,true,true,true,false,0);}catch(const std::exception&){}
                check(notesui::field<uintptr_t>(object.data(),0x58)==defs.front(),"Unsupported Stats producer attached");
            };
            auto body=reinterpret_cast<unsigned char*>(defs.front())+48;const auto original=body[0];body[0]^=1;reject();body[0]=original;
            const auto values=reinterpret_cast<uintptr_t*>(notesui::field<uintptr_t>(reinterpret_cast<const unsigned char*>(defs.front()),40));
            bool checkedProvider=false;
            for(unsigned i=0;i<notesui::field<unsigned>(reinterpret_cast<const unsigned char*>(defs.front()),32);++i){
                auto text=reinterpret_cast<char*>(notesui::field<uintptr_t>(reinterpret_cast<const unsigned char*>(values[i]),16))+12;
                if(strcmp(text,"GetRelevantSkillsFromPlayer"))continue;
                text[0]='X';reject();text[0]='G';checkedProvider=true;break;
            }
            check(checkedProvider,"Stats fixture lacks the verified native skill provider");
        }
        // Navigation callbacks remain an all-or-nothing dependency group.
        if(name=="card.47"||name=="card.48")check(rt.admit(defs,read,0),"GPI dependency group rejected");
        for(size_t i=0;i<defs.size();++i){
            std::array<unsigned char,0x60> object{};memcpy(object.data()+0x58,&defs[i],8);const auto& p=rt.discoveryPrograms()[i];
            const auto begin=std::chrono::steady_clock::now();rt.beforeInvoke(object.data(),read,false,true,0,true,true,true,true,0);coldWorst=(std::max)(coldWorst,std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-begin).count());
            if(notesui::field<uintptr_t>(object.data(),0x58)==defs[i]){std::cerr<<name<<" method "<<i<<" frame "<<p.argc<<","<<p.registers<<","<<p.flags<<","<<p.size<<"\n";for(const auto& message:rt.takeDiscoveryMessages())std::cerr<<message<<'\n';throw std::runtime_error("symbolic method was not attached");}
            for(unsigned policy=0;policy<6;++policy){const unsigned scale=policy%5,teamScale=(policy+2)%5;const bool colors=policy%2!=0,fog=policy!=3,team=policy!=4,dollars=policy%2==0;
                rt.beforeInvoke(object.data(),read,false,true,scale,colors,fog,team,dollars,teamScale);
                const auto target=notesui::field<uintptr_t>(object.data(),0x58);const auto code=reinterpret_cast<const unsigned char*>(target)+48;
                for(const auto at:p.scaleOffsets)check(code[at]==scale,"roster/recruit scale mismatch");for(const auto at:p.teamScaleOffsets)check(code[at]==teamScale,"team scale mismatch");for(const auto at:p.colorOffsets)check(code[at]==static_cast<unsigned char>(colors),"colors mismatch");for(const auto at:p.fogOffsets)check(code[at]==static_cast<unsigned char>(fog),"fog mismatch");for(const auto at:p.teamOffsets)check(code[at]==static_cast<unsigned char>(team),"team fog mismatch");for(const auto at:p.dollarOffsets)check(code[at]==static_cast<unsigned char>(dollars),"dollars mismatch");
                check(!memcmp(code+p.copyAt,reinterpret_cast<const void*>(defs[i]+48),p.copyBytes),"native source instructions changed");++policies;
            }
            rt.beforeInvoke(object.data(),read,false,false);check(notesui::field<uintptr_t>(object.data(),0x58)==defs[i],"scope/Off did not restore native definition");
            if(name=="grid.legacy"||name=="grid.modern"){
                runtime::attachPlayerGridUi(object.data(),false);check(notesui::field<uintptr_t>(object.data(),0x58)!=defs[i],"grid setup candidate filter lost a supported method");
                rt.beforeInvoke(object.data(),read,false,false);
                if(p.newRegisters==p.registers){runtime::attachPlayerGridUi(object.data(),true,reinterpret_cast<const unsigned char*>(defs[i]));check(notesui::field<uintptr_t>(object.data(),0x58)!=defs[i],"fixed grid candidate filter lost a supported method");rt.beforeInvoke(object.data(),read,false,false);}
            }
            rt.beforeInvoke(object.data(),read,false,true,0,true,true,true,false,0);const auto before=reader.calls;const auto start=std::chrono::steady_clock::now();for(unsigned n=0;n<1000;++n)rt.beforeInvoke(object.data(),read,false,true,0,true,true,true,false,0);
            warmMicros+=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-start).count();check(reader.calls==before,"warm callback performed guarded reads");++methods;
        }
    }
    std::cout<<"PASS: "<<methods<<" real methods, "<<policies<<" independent policy combinations, exact native copy/Off/scope and zero warm guarded reads. Cache bytes "<<cacheBytes<<"; cold worst "<<coldWorst<<" us; warm average "<<warmMicros/(methods*1000)<<" us.\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
