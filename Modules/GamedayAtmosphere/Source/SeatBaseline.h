#pragma once
#include <map>
#include <cstring>
#include "SeatPattern.h"
namespace attendance::seats {
// Per-match copies of authored position order and original masks. A streamed
// renderer is not a new attendance target. Reset only for a verified new game.
class Baselines {
    struct Entry {std::vector<Position> positions;std::vector<unsigned short> masks;unsigned short parts=0;};
    std::map<uint64_t,Entry> entries_;
    static uint64_t key(const std::vector<Position>& positions){
        uint64_t h=1469598103934665603ull;
        for(const auto& p:positions){const auto* bytes=reinterpret_cast<const unsigned char*>(&p);for(unsigned i=0;i<12;++i){h^=bytes[i];h*=1099511628211ull;}}
        return h^static_cast<uint64_t>(positions.size());
    }
    static bool same(const std::vector<Position>& a,const std::vector<Position>& b){
        if(a.size()!=b.size())return false;
        for(size_t i=0;i<a.size();++i)if(std::memcmp(&a[i],&b[i],12))return false;
        return true;
    }
public:
    static uint64_t fingerprint(const std::vector<Position>& positions){return key(positions);}
    void reset(){entries_.clear();}
    void remember(const std::vector<Position>& positions,const std::vector<unsigned short>& masks,unsigned short parts){
        if(!parts||positions.size()!=masks.size()||positions.size()<512||positions.size()>200000||entries_.size()>=64)return;
        entries_.emplace(key(positions),Entry{positions,masks,parts}); // Original snapshot is immutable.
    }
    bool restore(const std::vector<Position>& positions,std::vector<unsigned short>& masks,std::vector<unsigned short>& seen,unsigned short& parts)const{
        auto found=entries_.find(key(positions));if(found==entries_.end()||!same(found->second.positions,positions))return false;
        const auto& old=found->second;masks=old.masks;seen.resize(masks.size());parts|=old.parts;
        for(size_t i=0;i<masks.size();++i){
            bool empty=(old.masks[i]&old.parts)==old.parts;
            if(empty){masks[i]=0xffff;seen[i]=0xffff;} // New model parts cannot populate an originally empty position.
            else seen[i]|=old.parts;
        }
        return true;
    }
};
}
