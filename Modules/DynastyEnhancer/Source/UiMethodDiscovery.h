#pragma once
#include "RecruitNotesUiRuntime.h"
#include "PlayerCardUiPrograms.h"
#include "AioCurrencyUiPrograms.h"
#include "TeamCircleUiPrograms.h"
#include "WeeklyMatchupUiPrograms.h"
#include "WeekModuleUiPrograms.h"
#include <deque>
#include <functional>
#include <chrono>
#include <algorithm>
namespace recruitingfog::uidiscovery {
struct Contract {std::string method;std::vector<std::string> prefix;unsigned constants;uint64_t poolHash;unsigned methodIndex;};
}
#include "UiMethodContracts.generated.h"
namespace recruitingfog::uidiscovery {
using Header=std::array<unsigned char,48>;
// Called only with the engine's live ScriptFunction2 receiver. This snapshot
// is a candidate filter, never permission to bind an unverified method.
inline bool invocationSnapshot(void* object,uintptr_t& definition,Header& header){
    if(!object)return false;
    __try{definition=*reinterpret_cast<const uintptr_t*>(static_cast<unsigned char*>(object)+0x58);
        if(definition<0x10000)return false;memcpy(header.data(),reinterpret_cast<const void*>(definition),header.size());return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
// Verified AptValueString / AptString layouts for this supported game build.
// This is deliberately separate from the unrelated native UI-response values.
template<class Read>bool constantString(uintptr_t value,std::string& out,Read read,uintptr_t game){
    std::array<unsigned char,24> v{};std::array<unsigned char,12> s{};
    if(value<0x10000||!read(value,v.data(),v.size())||notesui::field<uintptr_t>(v.data(),0)!=game+0xbc41750)return false;
    const auto text=notesui::field<uintptr_t>(v.data(),16);
    if(text<0x10000||!read(text,s.data(),s.size()))return false;
    const auto length=notesui::field<unsigned short>(s.data(),4),capacity=notesui::field<unsigned short>(s.data(),6);
    if(length>1024||capacity<length)return false;
    out.resize(size_t(length)+1);if(!read(text+12,out.data(),out.size())||out.back()!=0)return false;out.pop_back();return out.find('\0')==std::string::npos;
}
template<class Read>bool prefixMatches(const Contract& c,const Header& h,Read read,uintptr_t game){
    const auto n=notesui::field<unsigned>(h.data(),32);const auto pool=notesui::field<uintptr_t>(h.data(),40);
    if(n<c.prefix.size()||n>8192||pool<0x10000)return false;
    for(size_t i=0;i<c.prefix.size();++i){uintptr_t value=0;std::string text;if(!read(pool+i*8,&value,8)||!constantString(value,text,read,game)||text!=c.prefix[i])return false;}return true;
}
template<class Read>bool poolMatches(const Contract& c,const Header& h,Read read,uintptr_t game,bool* unavailable=nullptr){
    const auto n=notesui::field<unsigned>(h.data(),32);const auto pool=notesui::field<uintptr_t>(h.data(),40);
    if(n<c.constants||n>8192||pool<0x10000)return false;
    std::vector<uintptr_t> values(c.constants);if(!read(pool,values.data(),values.size()*8)){if(unavailable)*unavailable=true;return false;}
    uint64_t hash=14695981039346656037ull;
    for(unsigned i=0;i<c.constants;++i){std::string text;if(!constantString(values[i],text,read,game)){if(unavailable)*unavailable=true;return false;}if(i<c.prefix.size()&&text!=c.prefix[i])return false;
        for(unsigned char b:text){hash^=b;hash*=1099511628211ull;}hash^=0;hash*=1099511628211ull;
    }return hash==c.poolHash;
}
// The registration stream pushes the method's dictionary name immediately
// before DefineFunction2. Check the typed instruction and alignment, not a
// numerical rating, a free-floating string or the anonymous function name.
template<class Read>bool declarationMatches(uintptr_t definition,const Contract& c,Read read){
    std::array<unsigned char,16> bytes{};if(definition<16||!read(definition-16,bytes.data(),16))return false;
    for(unsigned at=0;at<14;++at){unsigned count=0,index=0;
        if(bytes[at]==0xa2){count=2;index=bytes[at+1];}
        else if(bytes[at]==0xa3&&at<13){count=3;index=notesui::field<unsigned short>(bytes.data(),at+1);}
        else continue;
        if(at+count>=16||index!=c.methodIndex||bytes[at+count]!=0x8e||((definition-16+at+count+8)&~uintptr_t(7))!=definition)continue;
        bool padding=true;for(unsigned i=at+count+1;i<16;++i)padding&=bytes[i]==0;
        if(padding)return true;
    }return false;
}
struct Group {
    std::string surface;bool recruiting,coldSingle;std::vector<Contract> contracts;std::vector<notesui::Program> programs;std::unique_ptr<notesui::Runtime> owned;notesui::Runtime& runtime;bool prepareOnly=false,interestPolicy=false;
    static std::vector<notesui::Program> annotate(std::vector<notesui::Program> p){for(auto& item:p)semanticPointers(item);return p;}
    Group(std::string name,std::vector<Contract> ids,std::vector<notesui::Program> p,bool recruit=false,bool cold=false):surface(std::move(name)),recruiting(recruit),coldSingle(cold),contracts(std::move(ids)),programs(annotate(std::move(p))),owned(std::make_unique<notesui::Runtime>(programs,true)),runtime(*owned){
        if(cold&&(programs.size()!=1||!programs[0].sourcePointers.empty()||!programs[0].nestedDefinitions.empty()||programs[0].size>2048||programs[0].registers!=programs[0].newRegisters))throw std::runtime_error("Invalid bounded cold UI contract");runtime.prepare();}
    Group(std::string name,std::vector<Contract> ids,notesui::Runtime& existing):surface(std::move(name)),recruiting(false),coldSingle(false),contracts(std::move(ids)),programs(existing.discoveryPrograms()),runtime(existing),prepareOnly(true){}
};
class Service {
    std::vector<std::unique_ptr<Group>> groups;
    using Method=std::pair<size_t,size_t>;
    std::map<notesui::FrameKey,std::vector<Method>> frames;
    using CoarseFrame=std::tuple<unsigned,unsigned short,unsigned short>;
    std::array<std::map<notesui::FrameKey,std::vector<size_t>>,2> invocationFrames;
    std::array<std::set<CoarseFrame>,2> discoveryFrames;
    void indexInvocationGroup(size_t index){
        const auto& group=*groups[index];for(unsigned fixed=0;fixed<2;++fixed){
            for(const auto& key:group.runtime.frameKeys(fixed!=0)){
                auto& owners=invocationFrames[fixed][key];if(std::find(owners.begin(),owners.end(),index)==owners.end())owners.push_back(index);
            }
            for(const auto& p:group.programs)if(!fixed||p.registers==p.newRegisters)
                discoveryFrames[fixed].emplace(p.argc,static_cast<unsigned short>(p.registers),static_cast<unsigned short>(p.flags));
        }
    }
    struct Ticket {uintptr_t definition;Header header;uint64_t ready;bool exact;};
    struct Catalogue {uintptr_t anchor;Header header;uint64_t retry;bool pending;unsigned attempts;};
    std::mutex queueMutex;std::deque<Ticket> queue;
    std::map<uintptr_t,std::pair<Header,uint64_t>> observed;
    std::deque<uintptr_t> observationOrder;
    std::map<uintptr_t,Catalogue> catalogues;
    std::set<std::tuple<uintptr_t,std::string,std::string>> reported;
    std::atomic<unsigned> dropped{0},preempted{0};
    std::atomic_flag openingNotes=ATOMIC_FLAG_INIT;
    std::map<uintptr_t,std::pair<Header,uint64_t>> openingRetry;
    struct OpeningResult {uintptr_t asset;bool ready;uint64_t micros;std::string reason,surface;};
    std::deque<OpeningResult> openingResults;
    uint64_t queueReportAt=0;unsigned lastDropped=0,lastPreempted=0;
    uintptr_t gameImage=reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
    static uint64_t now(){return GetTickCount64();}
    void rememberLocked(uintptr_t definition,const Header& header,uint64_t until){
        auto found=observed.find(definition);if(found!=observed.end()){found->second={header,until};return;}
        if(observed.size()>=8192){observed.erase(observationOrder.front());observationOrder.pop_front();}
        observed.emplace(definition,std::make_pair(header,until));observationOrder.push_back(definition);
    }
    void completed(const Ticket& ticket,uint64_t until=UINT64_MAX){std::lock_guard<std::mutex> lock(queueMutex);rememberLocked(ticket.definition,ticket.header,until);}
    template<class Read>bool dynastyAssetSeed(const Header& h,Read read,uintptr_t game){
        const auto count=notesui::field<unsigned>(h.data(),32);const auto pool=notesui::field<uintptr_t>(h.data(),40);
        if(count<4||count>8192||pool<0x10000)return false;
        std::array<uintptr_t,4> values{};if(!read(pool,values.data(),sizeof(values)))return false;
        std::array<std::string,4> names;for(size_t i=0;i<names.size();++i)if(!constantString(values[i],names[i],read,game))return false;
        // Discovery seeds only. These namespaces do not authorize a patch:
        // every discovered target still needs its full individual contract.
        return names[0]=="_global"&&(names[1]=="logic_cfm"||names[1]=="node_cfm"||(names[1]=="madden"&&names[3]=="ultimatecareer"));
    }
    template<class Read>uintptr_t assetBefore(uintptr_t definition,Read read){
        constexpr uintptr_t page=4096,limit=8*1024*1024;
        const auto end=(definition+page-1)&~(page-1);std::array<unsigned char,page> bytes{};
        for(uintptr_t used=page;used<=limit&&used<=end;used+=page){
            const auto at=end-used;if(read(at,bytes.data(),bytes.size()))for(size_t i=bytes.size()-13;i-- >0;)
                if(at+i<definition&&!memcmp(bytes.data()+i,"Apt Data:1:",11))return at+i;
        }return 0;
    }
    // One-time menu construction boundary, never Draw/focus. The worker is
    // normally ready; this bounded fallback prevents its scheduling from
    // deciding whether the first opening gets a tab. All ten dependencies
    // still validate atomically. No absolute method offsets are used.
    template<class Read>bool prepareOpeningNotes(uintptr_t definition,const Header& h,Read read,Group& group){
        const size_t entries=group.prepareOnly?1:2;size_t index=0;for(;index<entries;++index)if(notesui::headerMatches(group.programs[index],h))break;if(index==entries)return false;if(group.runtime.admitted(definition,h))return true;const auto& entry=group.programs[index];const auto& identity=group.contracts[index];
        if(!notesui::headerMatches(entry,h)||!declarationMatches(definition,identity,read)||!poolMatches(identity,h,read,gameImage))return false;
        {std::lock_guard<std::mutex> lock(queueMutex);const auto prior=openingRetry.find(definition);if(prior!=openingRetry.end()&&prior->second.first==h&&now()<prior->second.second)return false;
            if(openingRetry.size()>=128)openingRetry.erase(openingRetry.begin());openingRetry[definition]={h,now()+1000};}
        if(openingNotes.test_and_set())return false;
        struct Guard {std::atomic_flag& flag;~Guard(){flag.clear();}} guard{openingNotes};
        const auto begin=std::chrono::steady_clock::now();uintptr_t asset=0;bool ready=false;std::string reason;
        try{
            std::vector<unsigned char> entryBody(entry.size);if(!read(definition+48,entryBody.data(),entryBody.size())||notesui::bodyHash(entryBody)!=entry.hash)throw std::runtime_error(identity.method+" complete fingerprint differs; native initialization preserved.");
            asset=assetBefore(definition,read);if(!asset)throw std::runtime_error("Recruiting asset root is unavailable; native initialization preserved.");
            constexpr size_t cap=2*1024*1024,chunk=64*1024;std::vector<unsigned char> bytes;bytes.reserve(cap);std::vector<unsigned char> block(chunk);
            bool end=false;for(size_t at=0;at<cap&&!end;at+=chunk){
                if(read(asset+at,block.data(),block.size()))bytes.insert(bytes.end(),block.begin(),block.end());
                else for(size_t page=0;page<chunk;page+=4096){if(!read(asset+at+page,block.data(),4096)){end=true;break;}bytes.insert(bytes.end(),block.begin(),block.begin()+4096);}
            }
            for(size_t at=14;at+14<bytes.size();++at)if(!memcmp(bytes.data()+at,"Apt Data:1:",11)){bytes.resize(at);break;}
            std::vector<uintptr_t> definitions(group.programs.size(),0);std::map<std::pair<uintptr_t,uint64_t>,bool> pools;
            for(size_t at=16;at+48<=bytes.size();at+=8){
                const auto* p=bytes.data()+at;const auto flags=notesui::field<unsigned short>(p,14);
                if(notesui::field<unsigned>(p,8)>3||notesui::field<unsigned short>(p,12)<2||(flags!=25&&flags!=41&&flags!=42))continue;
                Header candidate{};memcpy(candidate.data(),p,48);
                for(size_t i=0;i<group.programs.size();++i){const auto& program=group.programs[i];const auto& contract=group.contracts[i];
                    if(!notesui::headerMatches(program,candidate)||!declarationMatches(asset+at,contract,read))continue;
                    const auto key=std::make_pair(notesui::field<uintptr_t>(p,40),contract.poolHash);auto known=pools.find(key);
                    if(!(known==pools.end()?(pools[key]=poolMatches(contract,candidate,read,gameImage)):known->second))continue;
                    std::vector<unsigned char> body(program.size);if(!read(asset+at+48,body.data(),body.size())||notesui::sourceBodyHash(program,candidate,body,asset,read)!=program.hash)throw std::runtime_error("Required "+group.surface+" callback "+contract.method+" failed its complete fingerprint; native initialization preserved.");
                    if(definitions[i])throw std::runtime_error("Ambiguous "+group.surface+" declaration "+contract.method+"; native initialization preserved.");definitions[i]=asset+at;
                }
            }
            if(std::any_of(definitions.begin(),definitions.end(),[](uintptr_t p){return !p;}))throw std::runtime_error("Required "+group.surface+" callbacks are unresolved or outside the bounded opening catalogue; native initialization preserved.");
            ready=group.runtime.admit(definitions,read,asset);reason=ready?"All "+std::to_string(definitions.size())+" "+group.surface+" callbacks verified at "+identity.method+" before native construction.":group.surface+" dependencies became unavailable during opening verification; native initialization preserved.";
        }catch(const std::exception& error){reason=error.what();}
        const auto elapsed=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-begin).count();
        {std::lock_guard<std::mutex> lock(queueMutex);if(openingResults.size()<32)openingResults.push_back({asset,ready,static_cast<uint64_t>(elapsed),std::move(reason),group.surface});}
        return ready;
    }
    template<class Report>void diagnostic(uintptr_t asset,const std::string& surface,const std::string& state,const std::string& reason,Report report){
        if(reported.size()<512&&reported.emplace(asset,surface,state).second)report(surface,state,reason,asset);
    }
    template<class Read,class Report>bool discover(uintptr_t asset,Read read,uintptr_t game,Report report){
        // One bounded catalogue pass discovers all reviewed methods in this
        // asset, including dependencies that have not been invoked yet.
        constexpr size_t cap=8*1024*1024;std::vector<unsigned char> bytes;
        for(size_t at=0;at<cap;at+=4096){std::array<unsigned char,4096> page{};if(!read(asset+at,page.data(),page.size()))break;bytes.insert(bytes.end(),page.begin(),page.end());}
        for(size_t at=14;at+14<bytes.size();++at)if(!memcmp(bytes.data()+at,"Apt Data:1:",11)){bytes.resize(at);break;}
        std::vector<std::vector<uintptr_t>> definitions;
        std::vector<std::vector<bool>> ambiguous;
        std::vector<std::string> incompatible(groups.size());
        for(const auto& g:groups){definitions.emplace_back(g->programs.size(),0);ambiguous.emplace_back(g->programs.size(),false);}
        std::map<std::pair<uintptr_t,uint64_t>,bool> pools;
        for(size_t at=16;at+48<=bytes.size();at+=8){const auto key=notesui::frameKey(bytes.data()+at);const auto frame=frames.find(key);if(frame==frames.end())continue;
            Header h{};memcpy(h.data(),bytes.data()+at,48);
            if(!notesui::field<unsigned>(h.data(),32)||notesui::field<unsigned>(h.data(),32)>8192)continue;
            for(const auto& method:frame->second){const auto g=method.first,i=method.second;const auto& c=groups[g]->contracts[i];const auto& program=groups[g]->programs[i];
                if(!declarationMatches(asset+at,c,read))continue;
                const auto pool=std::make_pair(notesui::field<uintptr_t>(h.data(),40),c.poolHash);auto known=pools.find(pool);
                const bool valid=known==pools.end()?(pools[pool]=poolMatches(c,h,read,game)):known->second;if(!valid)continue;
                std::vector<unsigned char> body(program.size);if(!read(asset+at+48,body.data(),body.size()))continue;
                try{if(notesui::sourceBodyHash(program,h,body,asset,read)!=program.hash){incompatible[g]=c.method;continue;}}
                catch(const std::exception&){incompatible[g]=c.method;continue;}
                if(ambiguous[g][i])continue;
                if(definitions[g][i]&&definitions[g][i]!=asset+at){ambiguous[g][i]=true;definitions[g][i]=0;incompatible[g]=c.method+" (ambiguous declarations)";continue;}
                definitions[g][i]=asset+at;
            }
        }
        bool pending=false;
        for(size_t g=0;g<groups.size();++g){auto& group=*groups[g];const auto found=std::count_if(definitions[g].begin(),definitions[g].end(),[](uintptr_t x){return x!=0;});
            if(!found&&incompatible[g].empty())continue;
            if(found!=static_cast<ptrdiff_t>(definitions[g].size())){
                if(!incompatible[g].empty())diagnostic(asset,group.surface,"unsupported","Modified callback "+incompatible[g]+" failed its complete fingerprint; native method preserved.",report);
                else{pending=true;diagnostic(asset,group.surface,"waiting","Required callbacks are not all verified yet; no partial "+group.surface+" dependency group was installed.",report);}continue;
            }
            try{if(!group.runtime.admit(definitions[g],read,asset)){pending=true;continue;}
                std::vector<Header> verified(definitions[g].size());for(size_t i=0;i<definitions[g].size();++i)read(definitions[g][i],verified[i].data(),48);
                {std::lock_guard<std::mutex> lock(queueMutex);for(size_t i=0;i<definitions[g].size();++i)rememberLocked(definitions[g][i],verified[i],UINT64_MAX);}
                diagnostic(asset,group.surface,"ready","Asset catalogue verified class/method identities, resolved constants and complete bodies.",report);
            }catch(const std::exception& error){diagnostic(asset,group.surface,"unsupported",error.what(),report);}
        }
        return pending;
    }
public:
    void registerDependency(std::string surface,std::vector<Contract> contracts,notesui::Runtime& runtime){
        if(contracts.size()!=runtime.methodCount()||contracts.empty())throw std::runtime_error("Incomplete UI dependency contracts");const size_t g=groups.size();groups.emplace_back(std::make_unique<Group>(std::move(surface),std::move(contracts),runtime));
        for(size_t i=0;i<groups.back()->programs.size();++i){const auto& p=groups.back()->programs[i];frames[{p.argc,p.registers,p.flags,p.size}].emplace_back(g,i);}
        indexInvocationGroup(g);
    }
    Service(){
        const auto notes=notesContracts();const auto pnotes=notesui::programs();
        groups.emplace_back(std::make_unique<Group>("Recruit Notes navigation/content",std::vector<Contract>(notes.begin(),notes.begin()+10),std::vector<notesui::Program>(pnotes.begin(),pnotes.begin()+10),true));
        for(size_t i=10;i<notes.size();++i)groups.emplace_back(std::make_unique<Group>("Recruit attribute "+notes[i].method,std::vector<Contract>{notes[i]},std::vector<notesui::Program>{pnotes[i]},true));
        const auto teams=teamContracts();groups.emplace_back(std::make_unique<Group>("Team header",std::vector<Contract>{teams[0]},playercardui::teamModulePrograms()));
        for(const auto& p:aiocurrencyui::group0())if(p.argc==0&&p.registers==5&&p.size==4514)
            groups.emplace_back(std::make_unique<Group>("Team header extended layout",std::vector<Contract>{teams[1]},std::vector<notesui::Program>{p}));
        const auto currency=currencyContracts();const auto programs=aiocurrencyui::group3();
        if(currency.size()!=programs.size())throw std::runtime_error("Currency discovery manifest differs");
        for(size_t i=0;i<currency.size();++i){std::string name="Recruiting currency";for(const auto& part:currency[i].prefix)if(part!="_global"&&part!="Object")name+="."+part;name+="."+currency[i].method;
            groups.emplace_back(std::make_unique<Group>(name,std::vector<Contract>{currency[i]},std::vector<notesui::Program>{programs[i]}));}
        for(auto& group:groups)for(const auto& c:group->contracts)if(c.method=="Draw"&&c.prefix.size()>=5&&c.prefix[1]=="art_control"&&c.prefix[3]=="recruitinghub"&&c.prefix[4]=="PlayerListItem")group->interestPolicy=true;
        groups.emplace_back(std::make_unique<Group>("Weekly Practice matchup team ratings",weeklyMatchupContracts(),weeklymatchupui::programs(),false,true));
        groups.emplace_back(std::make_unique<Group>("Weekly Practice WeekModuleHorizontal team ratings",weekModuleContracts(),weekmoduleui::programs(),false,true));
        const auto circleIds=circleContracts();const auto circlePrograms=teamcircleui::programs();
        for(size_t i=0;i<circleIds.size();++i)groups.emplace_back(std::make_unique<Group>("Team rating circles "+circleIds[i].method,std::vector<Contract>{circleIds[i]},std::vector<notesui::Program>{circlePrograms[i]},false,true));
        for(size_t g=0;g<groups.size();++g)for(size_t i=0;i<groups[g]->programs.size();++i){const auto& p=groups[g]->programs[i];frames[{p.argc,p.registers,p.flags,p.size}].emplace_back(g,i);}
        for(size_t g=0;g<groups.size();++g)indexInvocationGroup(g);
    }
    const auto& surfaces()const{return groups;}
    unsigned droppedCount()const{return dropped.load();}
    unsigned preemptedCount()const{return preempted.load();}
    bool interested(const Header& h,bool fixed)const{
        const unsigned slot=fixed?1:0;if(invocationFrames[slot].find(notesui::frameKey(h.data()))!=invocationFrames[slot].end())return true;
        return notesui::field<unsigned>(h.data(),24)<=32768&&discoveryFrames[slot].count({notesui::field<unsigned>(h.data(),8),notesui::field<unsigned short>(h.data(),12),notesui::field<unsigned short>(h.data(),14)})!=0;
    }
    template<class Read>void invoke(void* object,Read read,bool fixed,bool dynasty,unsigned recruitScale,unsigned rosterScale,unsigned teamScale,bool colors,bool recruiting,bool roster,bool team,bool dollars,unsigned interestMode=0){
        if(!object)return;uintptr_t definition=0;Header h{};
        if(!read(reinterpret_cast<uintptr_t>(object)+0x58,&definition,8)||!definition||!read(definition,h.data(),48))return;
        invokeSnapshot(object,definition,h,read,fixed,dynasty,recruitScale,rosterScale,teamScale,colors,recruiting,roster,team,dollars,interestMode);
    }
    template<class Read>void invokeSnapshot(void* object,uintptr_t definition,const Header& h,Read read,bool fixed,bool dynasty,unsigned recruitScale,unsigned rosterScale,unsigned teamScale,bool colors,bool recruiting,bool roster,bool team,bool dollars,unsigned interestMode=0){
        if(!object||!definition)return;
        bool candidate=false,recognized=false;
        const auto foundFrame=invocationFrames[fixed?1:0].find(notesui::frameKey(h.data()));
        if(foundFrame!=invocationFrames[fixed?1:0].end())for(const auto groupIndex:foundFrame->second){auto& group=groups[groupIndex];
            const bool notes=group->recruiting;
            if(!group->prepareOnly)group->runtime.beforeInvoke(object,read,fixed,dynasty,group->interestPolicy?interestMode:notes?recruitScale:rosterScale,colors,notes?recruiting:roster,team,dollars,teamScale,&recognized);candidate=true;
            if(!recognized&&dynasty&&(group.get()==groups.front().get()||group->prepareOnly)&&prepareOpeningNotes(definition,h,read,*group)&&!group->prepareOnly)
                group->runtime.beforeInvoke(object,read,fixed,dynasty,recruitScale,colors,recruiting,team,dollars,teamScale,&recognized);
            // Small, independent, pointer-free callbacks may bind at their
            // first invocation. This verifies a fixed class dictionary and
            // complete method body; it performs no asset search or scan.
            if(!recognized&&group->coldSingle){
                bool seen=false;{std::unique_lock<std::mutex> lock(queueMutex,std::try_to_lock);if(lock){const auto found=observed.find(definition);seen=found!=observed.end()&&found->second.first==h&&now()<found->second.second;}}
                if(!seen&&declarationMatches(definition,group->contracts[0],read)&&poolMatches(group->contracts[0],h,read,gameImage))try{
                    group->runtime.admit({definition},read,0);
                    group->runtime.beforeInvoke(object,read,fixed,dynasty,rosterScale,colors,roster,team,dollars,teamScale,&recognized);
                }catch(const std::exception&){/* Worker diagnoses an unsupported complete fingerprint. */}
            }
        }
        // Read-only preparation can run while a Dynasty is loading. Actual
        // redirection above still requires the full native Dynasty scope.
        if(recognized)return;const bool exact=candidate;
        if(!candidate)candidate=interested(h,fixed);
        if(!candidate)return;
        std::unique_lock<std::mutex> lock(queueMutex,std::try_to_lock);if(!lock)return;
        const auto time=now();const auto found=observed.find(definition);
        if(found!=observed.end()&&found->second.first==h&&time<found->second.second)return;
        if(queue.size()>=64){const auto weaker=std::find_if(queue.begin(),queue.end(),[](const Ticket& t){return !t.exact;});
            if(exact&&weaker!=queue.end()){rememberLocked(weaker->definition,weaker->header,time+1000);queue.erase(weaker);++preempted;}else{++dropped;rememberLocked(definition,h,time+(exact?200:1000));return;}}
        rememberLocked(definition,h,UINT64_MAX);queue.push_back({definition,h,time,exact});
    }
    template<class Read,class Report>void tick(Read read,uintptr_t game,Report report){
        std::deque<OpeningResult> openings;{std::lock_guard<std::mutex> lock(queueMutex);openings.swap(openingResults);}
        for(const auto& opening:openings)report(opening.surface+" first opening",opening.ready?std::string("ready"):std::string("unavailable"),opening.reason+" Verification took "+std::to_string(opening.micros)+" us.",opening.asset);
        std::deque<Ticket> work;{std::lock_guard<std::mutex> lock(queueMutex);work.swap(queue);}
        std::stable_sort(work.begin(),work.end(),[](const Ticket& a,const Ticket& b){return a.exact&&!b.exact;});unsigned processed=0;
        for(auto ticket:work){
            if(ticket.ready>now()||processed>=8){std::lock_guard<std::mutex> lock(queueMutex);if(queue.size()<64)queue.push_back(ticket);else{++dropped;rememberLocked(ticket.definition,ticket.header,now()+1000);}continue;}++processed;
            Header current{};if(!read(ticket.definition,current.data(),48)||current!=ticket.header)continue;
            const auto count=notesui::field<unsigned>(current.data(),32);if(!count||count>8192){completed(ticket);continue;}
            bool knownClass=false;
            for(const auto& group:groups)for(const auto& c:group->contracts)if(prefixMatches(c,current,read,game)){
                bool unavailable=false;if(poolMatches(c,current,read,game,&unavailable)){knownClass=true;break;}
                diagnostic(ticket.definition,group->surface,unavailable?"waiting":"unsupported",unavailable?"Native constants are not readable yet; attachment deferred.":"Resolved class constants differ; native method preserved.",report);
            }
            if(!knownClass&&!dynastyAssetSeed(current,read,game)){
                // Pools can resolve in place without changing the definition
                // header. Only permanently cache a fully readable class prefix.
                const auto pool=notesui::field<uintptr_t>(current.data(),40);bool resolved=count>=4;std::string text;
                for(unsigned i=0;i<4&&resolved;++i){uintptr_t value=0;resolved=read(pool+i*8,&value,8)&&constantString(value,text,read,game);}
                completed(ticket,resolved?UINT64_MAX:now()+1000);continue;
            }
            const auto asset=assetBefore(ticket.definition,read);if(!asset){completed(ticket,now()+1000);continue;}
            auto cached=catalogues.find(asset);bool scan=cached==catalogues.end();
            if(!scan){Header anchor{};scan=!read(cached->second.anchor,anchor.data(),48)||anchor!=cached->second.header||(cached->second.pending&&now()>=cached->second.retry);}
            if(scan){if(catalogues.size()>=128&&cached==catalogues.end()){diagnostic(asset,"UI asset catalogue","unsupported","Bounded generation limit reached; restart the game.",report);continue;}
                const bool pending=discover(asset,read,game,report);catalogues[asset]={ticket.definition,current,now()+500,pending,0};}
            completed(ticket);
        }
        // Retry a partially resolved dependency group even if its one-time
        // callback has already passed. Never publish a partial Notes bundle.
        unsigned retried=0;for(auto& entry:catalogues){auto& c=entry.second;if(!c.pending||now()<c.retry||retried>=2)continue;++retried;
            Header anchor{};if(!read(c.anchor,anchor.data(),48)||anchor!=c.header){c.pending=false;continue;}
            if(c.attempts++>=20){c.pending=false;diagnostic(entry.first,"UI asset catalogue","unavailable","Required callbacks did not resolve within the bounded loading window; native navigation preserved.",report);continue;}
            c.pending=discover(entry.first,read,game,report);c.retry=now()+500;
        }
        const auto d=dropped.load(),p=preempted.load();if(now()>=queueReportAt&&(d!=lastDropped||p!=lastPreempted)){
            queueReportAt=now()+10000;lastDropped=d;lastPreempted=p;
            report(std::string("UI discovery queue"),std::string("diagnostic"),"Broad candidates dropped: "+std::to_string(d)+"; exact callbacks protected by preemption: "+std::to_string(p),uintptr_t(0));
        }
        for(const auto& group:groups)if(group->coldSingle&&group->runtime.ownerCount())diagnostic(0,group->surface,"attached","Verified first-use callback attached with its unchanged native frame.",report);
    }
};
}
