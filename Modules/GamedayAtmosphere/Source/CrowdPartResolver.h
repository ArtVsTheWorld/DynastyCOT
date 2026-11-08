#pragma once
#include <windows.h>
#include <cstdint>
#include <array>

namespace attendance::visibility {
using PartResolver=unsigned(__fastcall*)(uintptr_t,unsigned);
// The part table can retire before its renderer during scene changes. A failed
// optional inspection must not bypass native handling or fault the whole mod.
inline bool resolvePart(PartResolver resolver,uintptr_t parts,unsigned part,unsigned& bit)noexcept{
    bit=16;
    if(!resolver||parts<0x10000)return false;
    __try{bit=resolver(parts,part);return bit<16;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
// A native attendance traversal repeatedly resolves the same model parts for
// thousands of fans. Cache only successful resolutions inside that one pass.
// Natural callbacks and the next native pass always resolve afresh.
class PassParts {
    struct Entry {PartResolver resolver;uintptr_t parts;unsigned part,bit;};
    std::array<Entry,32> entries_{};unsigned count_=0;bool active_=false;
public:
    void begin(){count_=0;active_=true;}
    void end(){count_=0;active_=false;}
    bool resolve(PartResolver resolver,uintptr_t parts,unsigned part,unsigned& bit){
        if(active_)for(unsigned i=0;i<count_;++i)if(entries_[i].resolver==resolver&&entries_[i].parts==parts&&entries_[i].part==part){bit=entries_[i].bit;return true;}
        if(!resolvePart(resolver,parts,part,bit))return false;
        if(active_&&count_<entries_.size())entries_[count_++]={resolver,parts,part,bit};
        return true;
    }
};
}
