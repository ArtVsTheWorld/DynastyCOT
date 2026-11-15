#pragma once
#include "RecruitNotesUiPrograms.h"
#include <array>
#include <bitset>
#include <type_traits>
#include <atomic>
#include <mutex>
#include <fstream>
#include <set>
#include <stdexcept>
#include <string>
#include <map>
#include <memory>
#include <functional>

namespace recruitingfog::notesui {
inline uint64_t bodyHash(const std::vector<unsigned char>& bytes){uint64_t hash=14695981039346656037ull;for(auto b:bytes){hash^=b;hash*=1099511628211ull;}return hash;}
template<class T>T field(const unsigned char* bytes,size_t at){T value{};memcpy(&value,bytes+at,sizeof(value));return value;}
using FrameKey=std::array<unsigned,4>;
inline FrameKey frameKey(const unsigned char* h){return {field<unsigned>(h,8),field<unsigned short>(h,12),field<unsigned short>(h,14),field<unsigned>(h,24)};}
using GuardedRead=std::function<bool(uintptr_t,void*,size_t)>;
inline uint64_t sourceBodyHash(const Program& p,const std::array<unsigned char,48>& h,const std::vector<unsigned char>& original,uintptr_t asset,const GuardedRead& read={}){
    if(p.nestedDefinitions.empty()&&p.sourcePointers.empty())return bodyHash(original);
    auto normalized=original;
    for(const auto& pointer:p.sourcePointers){
        if(pointer.offset>original.size()||original.size()-pointer.offset<8)throw std::runtime_error("Source UI pointer exceeds its method");
        const auto value=field<uintptr_t>(original.data(),pointer.offset);
        if(pointer.byText){
            if(!read||pointer.text.size()>1024)throw std::runtime_error("Semantic UI literal requires a guarded read");
            const auto address=asset&&value<8*1024*1024?asset+value:value;std::vector<char> text(pointer.text.size()+1);
            if(!read(address,text.data(),text.size())||text.back()!=0||memcmp(text.data(),pointer.text.data(),pointer.text.size()))throw std::runtime_error("Source UI literal semantics changed");
        }else if(value!=pointer.value&&(!asset||value!=asset+pointer.value))throw std::runtime_error("Source UI pointer relocation is unsupported");
        memcpy(normalized.data()+pointer.offset,&pointer.value,8);
    }
    for(const auto& nested:p.nestedDefinitions){
        if(nested.offset>original.size()||original.size()-nested.offset<48)throw std::runtime_error("Nested player UI definition exceeds its method");
        const auto at=nested.offset;const auto name=field<uintptr_t>(original.data(),at),params=field<uintptr_t>(original.data(),at+16);
        const auto poolCount=field<unsigned>(original.data(),at+32);const auto pool=field<uintptr_t>(original.data(),at+40);
        if(nested.semantic){
            auto text=[&](uintptr_t pointer,const std::string& expected){std::vector<char> bytes(expected.size()+1);return expected.size()<=1024&&read&&read(pointer,bytes.data(),bytes.size())&&!bytes.back()&&!memcmp(bytes.data(),expected.data(),expected.size());};
            if(!text(name,nested.nameText)||field<unsigned>(original.data(),at+8)!=nested.arguments.size())throw std::runtime_error("Nested UI declaration changed");
            for(size_t i=0;i<nested.arguments.size();++i){std::array<unsigned char,16> argument{};if(!read(params+i*16,argument.data(),argument.size())||field<unsigned>(argument.data(),0)!=nested.arguments[i].first||!text(field<uintptr_t>(argument.data(),8),nested.arguments[i].second))throw std::runtime_error("Nested UI argument changed");}
        }else if((name!=nested.name&&(!asset||name!=asset+nested.name))||(params!=nested.params&&(!asset||params!=asset+nested.params)))throw std::runtime_error("Nested player UI pointer relocation is unsupported");
        const unsigned sourceCount=nested.emptyPool?0:0x98765432u;const uintptr_t sourcePool=nested.emptyPool?0:0x12345678u;
        if(!((poolCount==sourceCount&&pool==sourcePool)||(poolCount==field<unsigned>(h.data(),32)&&pool==field<uintptr_t>(h.data(),40))))throw std::runtime_error("Nested player UI pool differs from its enclosing class");
        memcpy(normalized.data()+at,&nested.name,8);memcpy(normalized.data()+at+16,&nested.params,8);
        memcpy(normalized.data()+at+32,&sourceCount,4);memcpy(normalized.data()+at+40,&sourcePool,8);
    }
    return bodyHash(normalized);
}
inline bool headerMatches(const Program& p,const std::array<unsigned char,48>& h){return field<unsigned>(h.data(),8)==p.argc&&field<unsigned short>(h.data(),12)==p.registers&&field<unsigned short>(h.data(),14)==p.flags&&field<unsigned>(h.data(),24)==p.size&&field<unsigned>(h.data(),32)>0&&field<unsigned>(h.data(),32)<16384&&field<uintptr_t>(h.data(),40)>=0x10000;}
inline std::vector<unsigned char> makeDefinition(const Program& p,const std::array<unsigned char,48>& h,const std::vector<unsigned char>& original,uintptr_t asset=0,const GuardedRead& read={}){
    if(!headerMatches(p,h)||original.size()!=p.size||sourceBodyHash(p,h,original,asset,read)!=p.hash||p.newRegisters>64||p.newRegisters<p.registers||p.code.empty()||p.code.size()>32768||p.copyBytes>original.size()||p.copyAt>p.code.size()||p.copyBytes>p.code.size()-p.copyAt)throw std::runtime_error("Native UI method unsupported at offset "+std::to_string(p.offset)+" (body hash "+std::to_string(bodyHash(original))+")");
    for(const auto* offsets:{&p.scaleOffsets,&p.teamScaleOffsets,&p.colorOffsets,&p.fogOffsets,&p.teamOffsets,&p.dollarOffsets})for(const auto at:*offsets){
        const bool copied=p.copyBytes&&at>=p.copyAt&&at-p.copyAt<p.copyBytes;
        const bool priorCopied=p.copyBytes&&at>p.copyAt&&at-1-p.copyAt<p.copyBytes;
        if(!at||at>=p.code.size()||copied||priorCopied||p.code[at-1]!=0xb5)throw std::runtime_error("Invalid UI display policy literal");
    }
    std::vector<unsigned char> result(h.begin(),h.end());result.insert(result.end(),p.code.begin(),p.code.end());
    if(p.copyBytes)memcpy(result.data()+48+p.copyAt,original.data(),p.copyBytes);
    const auto registers=static_cast<unsigned short>(p.newRegisters);const auto size=static_cast<unsigned>(p.code.size());memcpy(result.data()+12,&registers,2);memcpy(result.data()+24,&size,4);return result;
}
struct OwnedDefinition {uintptr_t original=0,owned=0;std::map<unsigned,uintptr_t> variants{};};
inline int redirectDefinition(PVOID volatile* location,const std::vector<OwnedDefinition>& definitions){
    const auto current=reinterpret_cast<uintptr_t>(*location);
    for(size_t i=0;i<definitions.size();++i){const auto& d=definitions[i];if(current!=d.original)continue;
        return InterlockedCompareExchangePointer(location,reinterpret_cast<void*>(d.owned),reinterpret_cast<void*>(d.original))==reinterpret_cast<void*>(d.original)?static_cast<int>(i):-1;
    }
    return -1;
}
class Runtime {
    struct Bank {uintptr_t asset=0;std::vector<OwnedDefinition> definitions;std::vector<std::array<unsigned char,48>> headers;std::vector<std::vector<unsigned char>> nativeBodies;};
    using Validator=std::function<bool(uintptr_t,const std::array<unsigned char,48>&,const GuardedRead&)>;
    std::vector<Validator> identities;
    struct Attempt {std::array<unsigned char,48> header;uint64_t retry;};
    std::map<uintptr_t,Attempt> attempts;std::mutex discoveryMutex;
    std::vector<std::string> discoveryMessages;
    std::vector<Program> code;std::vector<Bank> banks;std::mutex bankMutex;bool discoveredOnly=false;
    std::atomic<unsigned> attached{0};
    std::atomic<size_t> retainedBytes{0};
    std::atomic<size_t> retainedSourceBytes{0};
    std::atomic<unsigned> retainedDefinitions{0},retainedBanks{0};
    // A known immutable definition already selected for these options needs
    // no mutex, bank walk or allocation on subsequent Draw/focus callbacks.
    static constexpr unsigned variantModes=1024,cacheBuckets=32;
    using CacheRow=std::array<std::atomic<uintptr_t>,cacheBuckets>;
    std::unique_ptr<CacheRow[]> satisfied;
    std::array<unsigned short,variantModes> cacheModes{};
    unsigned cacheModeCount=1;
public:
    explicit Runtime(std::vector<Program> selected=programs(),bool symbolic=false):code(std::move(selected)),discoveredOnly(symbolic){
        unsigned mask=0;for(const auto& p:code){if(!p.scaleOffsets.empty())mask|=7;if(!p.teamScaleOffsets.empty())mask|=56;if(!p.colorOffsets.empty())mask|=64;if(!p.fogOffsets.empty())mask|=128;if(!p.teamOffsets.empty())mask|=256;if(!p.dollarOffsets.empty())mask|=512;}
        unsigned bits=0;for(unsigned n=0;n<10;++n)if(mask&(1u<<n))++bits;cacheModeCount=1u<<bits;
        for(unsigned mode=0;mode<variantModes;++mode){unsigned index=0,bit=0;for(unsigned n=0;n<10;++n)if(mask&(1u<<n)){if(mode&(1u<<n))index|=1u<<bit;++bit;}cacheModes[mode]=static_cast<unsigned short>(index);}
        satisfied=std::make_unique<CacheRow[]>(cacheModeCount);for(unsigned mode=0;mode<cacheModeCount;++mode)for(auto& entry:satisfied[mode])entry.store(0,std::memory_order_relaxed);
    }
    bool prepared=false;
    const std::vector<Program>& discoveryPrograms()const{return code;}
    bool admitted(uintptr_t definition,const std::array<unsigned char,48>& header){std::lock_guard<std::mutex> lock(bankMutex);for(const auto& bank:banks)for(size_t i=0;i<bank.definitions.size();++i)if(bank.definitions[i].original==definition&&bank.headers[i]==header)return true;return false;}
    size_t displayCacheBytes()const{return sizeof(cacheModes)+size_t(cacheModeCount)*sizeof(CacheRow);}
    void prepare(){prepared=true;}
    void configureDiscovery(std::vector<Program> annotated,std::vector<Validator> validators){
        if(prepared||annotated.size()!=code.size()||validators.size()!=code.size())throw std::runtime_error("UI discovery must be configured before startup");
        for(size_t i=0;i<code.size();++i)if(code[i].hash!=annotated[i].hash||code[i].code!=annotated[i].code)throw std::runtime_error("UI discovery cannot change display programs");
        code=std::move(annotated);identities=std::move(validators);discoveredOnly=true;
    }
    std::vector<std::string> takeDiscoveryMessages(){std::lock_guard<std::mutex> lock(discoveryMutex);auto result=std::move(discoveryMessages);discoveryMessages.clear();return result;}
    template<class Read>bool admitMethod(size_t index,uintptr_t definition,Read read,uintptr_t asset=0){
        if(index>=code.size())return false;std::array<unsigned char,48> header{};std::vector<unsigned char> body(code[index].size);
        if(!read(definition,header.data(),48)||!read(definition+48,body.data(),body.size()))return false;
        makeDefinition(code[index],header,body,asset,read);Bank bank;bank.asset=asset;bank.headers.resize(code.size());bank.nativeBodies.resize(code.size());bank.definitions.resize(code.size());
        bank.headers[index]=header;bank.nativeBodies[index]=std::move(body);bank.definitions[index].original=definition;
        std::lock_guard<std::mutex> lock(bankMutex);for(const auto& previous:banks)if(previous.definitions[index].original==definition&&previous.headers[index]==header)return true;
        if(banks.size()>=128)throw std::runtime_error("UI discovery generation limit reached");retainedSourceBytes+=48+bank.nativeBodies[index].size();++retainedBanks;banks.push_back(std::move(bank));return true;
    }
    template<class Read>bool discoverInvocation(uintptr_t definition,const std::array<unsigned char,48>& header,Read read){
        if(identities.empty())return false;std::unique_lock<std::mutex> guard(discoveryMutex,std::try_to_lock);if(!guard)return false;
        const auto prior=attempts.find(definition);if(prior!=attempts.end()&&prior->second.header==header&&GetTickCount64()<prior->second.retry)return false;
        if(attempts.size()>=2048)attempts.erase(attempts.begin());attempts[definition]={header,GetTickCount64()+1000};
        for(size_t i=0;i<code.size();++i)if(headerMatches(code[i],header)&&identities[i]&&identities[i](definition,header,read)){
            try{if(!admitMethod(i,definition,read))return false;if(discoveryMessages.size()<64)discoveryMessages.push_back("Method "+std::to_string(i)+" verified by class, declaration, resolved constants and complete body.");attempts[definition].retry=UINT64_MAX;return true;}
            catch(const std::exception& error){attempts[definition].retry=UINT64_MAX;if(discoveryMessages.size()<64)discoveryMessages.push_back("Method "+std::to_string(i)+" unsupported; native method preserved: "+error.what());return false;}
        }return false;
    }
    // Discovery supplies verified live definitions, never retained receivers.
    // Dependency groups are admitted atomically before any receiver can change.
    template<class Read>bool admit(const std::vector<uintptr_t>& definitions,Read read,uintptr_t asset){
        if(definitions.size()!=code.size())return false;
        Bank bank;bank.asset=asset;
        for(size_t i=0;i<code.size();++i){std::array<unsigned char,48> h{};std::vector<unsigned char> body(code[i].size);
            if(!read(definitions[i],h.data(),48)||!read(definitions[i]+48,body.data(),body.size()))return false;
            makeDefinition(code[i],h,body,asset,read);
            bank.headers.push_back(h);bank.nativeBodies.push_back(std::move(body));bank.definitions.push_back({definitions[i],0,{}});
        }
        std::lock_guard<std::mutex> lock(bankMutex);
        for(const auto& prior:banks)if(prior.headers==bank.headers&&prior.definitions.front().original==definitions.front())return true;
        if(banks.size()>=128)throw std::runtime_error("UI discovery reached its bounded generation limit; restart the game");
        size_t bytes=bank.headers.size()*48;for(const auto& body:bank.nativeBodies)bytes+=body.size();
        banks.push_back(std::move(bank));retainedSourceBytes+=bytes;++retainedBanks;return true;
    }
    std::vector<FrameKey> frameKeys(bool fixedFrame=false)const{
        std::vector<FrameKey> keys;for(const auto& p:code)if(!fixedFrame||p.newRegisters==p.registers){
            keys.push_back({p.argc,p.registers,p.flags,p.size});
            keys.push_back({p.argc,p.newRegisters,p.flags,static_cast<unsigned>(p.code.size())});
        }return keys;
    }
    bool acceptsFrame(const unsigned char* h,bool fixedFrame=false)const{
        if(!prepared)return false;
        for(const auto& p:code){
            const bool native=field<unsigned>(h,24)==p.size&&field<unsigned short>(h,12)==p.registers;
            const bool owned=field<unsigned>(h,24)==p.code.size()&&field<unsigned short>(h,12)==p.newRegisters;
            if((!fixedFrame||p.newRegisters==p.registers)&&(native||owned)&&field<unsigned>(h,8)==p.argc&&field<unsigned short>(h,14)==p.flags)return true;
        }
        return false;
    }
    // Rare navigation restoration: recognize only definitions this bank owns.
    // Shape matches alone never authorize a screen-state change.
    template<class Read>bool ownsInvocation(void* object,unsigned index,Read read){
        uintptr_t definition=0;if(index>=code.size()||!read(reinterpret_cast<uintptr_t>(object)+0x58,&definition,8))return false;
        std::lock_guard<std::mutex> guard(bankMutex);
        for(const auto& bank:banks)for(const auto& variant:bank.definitions.at(index).variants)if(variant.second==definition)return true;
        return false;
    }
    template<class Read>int beforeInvoke(void* object,Read read,bool fixedFrame=false,bool enabled=true,unsigned scale=0,bool colors=true,bool fog=true,bool team=true,bool dollars=false,unsigned teamScale=UINT32_MAX,bool* recognized=nullptr){
        if(!prepared)return -1;
        // The native invocation guarantees a live ScriptFunction2 receiver and
        // definition. Most calls leave after these small in-process comparisons;
        // no process scan, allocation or ReadProcessMemory is used on that path.
        auto location=reinterpret_cast<PVOID volatile*>(static_cast<unsigned char*>(object)+0x58);
        const auto definition=reinterpret_cast<uintptr_t>(*location);
        std::array<unsigned char,48> current{};memcpy(current.data(),reinterpret_cast<const void*>(definition),current.size());
        if(teamScale==UINT32_MAX)teamScale=scale;
        if(scale>4||teamScale>4)throw std::runtime_error("Unsupported grade scale");
        if(!acceptsFrame(current.data(),fixedFrame))return -1;
        const auto selectedVariant=scale+(teamScale<<3)+(colors?0u:64u)+(fog?0u:128u)+(team?0u:256u)+(dollars?512u:0u);const unsigned cacheMode=cacheModes[selectedVariant];
        const auto cachedIndex=(definition>>12)&(cacheBuckets-1);
        if(enabled&&satisfied[cacheMode][cachedIndex].load(std::memory_order_relaxed)==definition){if(recognized)*recognized=true;return -1;}
        std::unique_lock<std::mutex> lock(bankMutex);
        const auto redirect=[&](Bank& bank)->int{
            for(size_t i=0;i<bank.definitions.size();++i){
                if(fixedFrame&&code[i].newRegisters!=code[i].registers)continue;
                auto& owned=bank.definitions[i];bool ours=definition==owned.original;
                for(const auto& variant:owned.variants)ours=ours||definition==variant.second;
                if(!ours)continue;
                auto targetVariant=selectedVariant;
                if(code[i].scaleOffsets.empty())targetVariant&=~7u;
                if(code[i].teamScaleOffsets.empty())targetVariant&=~56u;
                if(code[i].colorOffsets.empty())targetVariant&=~64u;
                if(code[i].fogOffsets.empty())targetVariant&=~128u;
                if(code[i].teamOffsets.empty())targetVariant&=~256u;
                if(code[i].dollarOffsets.empty())targetVariant&=~512u;
                if(enabled&&!owned.variants[targetVariant]){
                    auto bytes=makeDefinition(code[i],bank.headers[i],bank.nativeBodies[i],bank.asset,read);
                    for(const auto at:code[i].scaleOffsets)bytes[48+at]=static_cast<unsigned char>(scale);
                    for(const auto at:code[i].teamScaleOffsets)bytes[48+at]=static_cast<unsigned char>(teamScale);
                    for(const auto at:code[i].colorOffsets)bytes[48+at]=colors?1:0;
                    for(const auto at:code[i].fogOffsets)bytes[48+at]=fog?1:0;
                    for(const auto at:code[i].teamOffsets)bytes[48+at]=team?1:0;
                    for(const auto at:code[i].dollarOffsets)bytes[48+at]=dollars?1:0;
                    const auto allocationBytes=(bytes.size()+4095)&~size_t(4095);
                    auto memory=VirtualAlloc(nullptr,allocationBytes,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);if(!memory)throw std::runtime_error("Cannot retain a player grade definition");
                    memcpy(memory,bytes.data(),bytes.size());owned.variants[targetVariant]=reinterpret_cast<uintptr_t>(memory);
                    if(targetVariant==0)owned.owned=owned.variants[0];
                    retainedBytes.fetch_add(allocationBytes,std::memory_order_relaxed);retainedDefinitions.fetch_add(1,std::memory_order_relaxed);
                }
                const auto desired=enabled?owned.variants[targetVariant]:owned.original;
                if(desired==definition){if(enabled)satisfied[cacheMode][cachedIndex]=desired;return -1;}
                if(InterlockedCompareExchangePointer(location,reinterpret_cast<void*>(desired),reinterpret_cast<void*>(definition))==reinterpret_cast<void*>(definition)){if(enabled)satisfied[cacheMode][(desired>>12)&(cacheBuckets-1)]=desired;++attached;return static_cast<int>(i);}
                return -1;
            }return -1;
        };
        for(auto& bank:banks){
            for(size_t i=0;i<bank.definitions.size();++i){const auto& owned=bank.definitions[i];
                bool ours=definition==owned.original&&bank.headers[i]==current;for(const auto& variant:owned.variants)ours=ours||definition==variant.second;
                if(ours){if(recognized)*recognized=true;return redirect(bank);}
            }
        }
        lock.unlock();
        if(enabled&&discoverInvocation(definition,current,read))return beforeInvoke(object,read,fixedFrame,enabled,scale,colors,fog,team,dollars,teamScale,recognized);
        if(!enabled||discoveredOnly)return -1;
        lock.lock();
        size_t index=0;for(;index<code.size();++index)if((!fixedFrame||code[index].newRegisters==code[index].registers)&&headerMatches(code[index],current))break;
        if(index==code.size()||definition<code[index].offset+0x10000)return -1;
        const auto asset=definition-code[index].offset;
        char magic[14]{};if(!read(asset,magic,14)||memcmp(magic,"Apt Data:1:",11))return -1;
        if(banks.size()>=512)throw std::runtime_error("Notes reached its bounded asset-generation limit");
        Bank bank;bank.asset=asset;
        std::set<std::pair<uintptr_t,unsigned>> checkedPools;
        for(const auto& p:code){
            const auto original=asset+p.offset;std::array<unsigned char,48> h{};std::vector<unsigned char> body(p.size);
            if(!read(original,h.data(),48)||!read(original+48,body.data(),body.size()))throw std::runtime_error("Notes source definition is no longer readable");
            makeDefinition(p,h,body,asset);
            const auto poolAddress=field<uintptr_t>(h.data(),40);const auto poolCount=field<unsigned>(h.data(),32);
            if(checkedPools.emplace(poolAddress,poolCount).second){std::vector<unsigned char> pool(size_t(poolCount)*8);
                if(!read(poolAddress,pool.data(),pool.size()))throw std::runtime_error("Notes native constant pool is no longer readable");}
            bank.headers.push_back(h);bank.nativeBodies.push_back(std::move(body));
        }
        // Gate the entire set before allocating or changing the first receiver.
        for(size_t i=0;i<code.size();++i){
            const auto nativeByte=[&](unsigned at){return code[i].copyBytes&&at>=code[i].copyAt&&at-code[i].copyAt<code[i].copyBytes;};
            for(const auto at:code[i].scaleOffsets)if(at==0||at>=code[i].code.size()||nativeByte(at)||nativeByte(at-1)||code[i].code[at-1]!=0xB5||code[i].code[at]!=0)throw std::runtime_error("Invalid grade scale literal metadata");
            for(const auto at:code[i].teamScaleOffsets)if(at==0||at>=code[i].code.size()||nativeByte(at)||nativeByte(at-1)||code[i].code[at-1]!=0xB5||code[i].code[at]!=0)throw std::runtime_error("Invalid team grade scale literal metadata");
            for(const auto at:code[i].colorOffsets)if(at==0||at>=code[i].code.size()||nativeByte(at)||nativeByte(at-1)||code[i].code[at-1]!=0xB5||code[i].code[at]!=1)throw std::runtime_error("Invalid rating color literal metadata");
            for(const auto* offsets:{&code[i].fogOffsets,&code[i].teamOffsets})for(const auto at:*offsets)if(at==0||at>=code[i].code.size()||nativeByte(at)||nativeByte(at-1)||code[i].code[at-1]!=0xB5||code[i].code[at]!=1)throw std::runtime_error("Invalid rating fog literal metadata");
            for(const auto at:code[i].dollarOffsets)if(at==0||at>=code[i].code.size()||nativeByte(at)||nativeByte(at-1)||code[i].code[at-1]!=0xB5||code[i].code[at]!=0)throw std::runtime_error("Invalid dollar literal metadata");
            OwnedDefinition retained;retained.original=asset+code[i].offset;bank.definitions.push_back(retained);
        }
        // Script owners are released on leaving Recruiting. Never retain their
        // addresses: attach only to the live receiver at native invocation setup,
        // before flags, argument registers, body length or body are consumed.
        size_t sourceBytes=bank.headers.size()*48;for(const auto& body:bank.nativeBodies)sourceBytes+=body.size();
        banks.push_back(std::move(bank));retainedSourceBytes.fetch_add(sourceBytes,std::memory_order_relaxed);retainedBanks.fetch_add(1,std::memory_order_relaxed);return redirect(banks.back());
    }
    unsigned ownerCount()const{return attached.load();}
    size_t methodCount()const{return code.size();}
    size_t retainedByteCount()const{return retainedBytes.load(std::memory_order_relaxed)+retainedSourceBytes.load(std::memory_order_relaxed);}
    size_t retainedSourceByteCount()const{return retainedSourceBytes.load(std::memory_order_relaxed);}
    unsigned retainedDefinitionCount()const{return retainedDefinitions.load(std::memory_order_relaxed);}
    unsigned retainedBankCount()const{return retainedBanks.load(std::memory_order_relaxed);}
};
// Dispatch by immutable frame shape before visiting any adapter bank. This is
// only a candidate index: beforeInvoke still verifies ownership/body hashes.
template<size_t N>class FrameDispatch {
public:
    using Mask=std::conditional_t<(N<=64),uint64_t,std::bitset<N>>;
private:
    std::array<std::map<FrameKey,Mask>,2> matches;
public:
    explicit FrameDispatch(const std::array<Runtime*,N>& banks){static_assert(N<=128);for(unsigned fixed=0;fixed<2;++fixed)
        for(size_t i=0;i<N;++i)for(const auto& key:banks[i]->frameKeys(fixed!=0)){
            if constexpr(N<=64)matches[fixed][key]|=uint64_t{1}<<i;
            else matches[fixed][key].set(i);
        }
    }
    Mask candidates(const unsigned char* header,bool fixed=false)const{
        const auto& index=matches[fixed?1:0];const auto found=index.find(frameKey(header));return found==index.end()?Mask{}:found->second;
    }
};
}
