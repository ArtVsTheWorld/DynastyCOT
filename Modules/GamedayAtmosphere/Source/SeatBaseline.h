#pragma once
#include <map>
#include <cstring>
#include "SeatPattern.h"
namespace attendance::seats {
// A generated renderer can already contain native empty positions before its
// first SceneCrowd::SetVisibility callback. Those callbacks also initialize
// newly generated model parts with false. Do not let that initialization erase
// an existing native hidden bit. Uniform allocation defaults are not evidence
// of a completed audience and still need the native first-input path.
inline bool retainInitialHidden(const std::vector<unsigned short>& masks,std::vector<unsigned short>& seen){
    if(masks.size()<512||seen.size()!=masks.size())return false;
    bool visible=false,hidden=false;
    for(auto mask:masks){visible=visible||mask==0;hidden=hidden||(mask!=0&&mask!=0xffff);}
    if(!visible||!hidden)return false;
    for(size_t i=0;i<masks.size();++i)if(masks[i]!=0xffff)seen[i]|=masks[i];
    return true;
}
// Scene callbacks are not the renderer's sole writer. Native generation and
// attendance can also finish masks directly. At the FIRST completed native
// attendance pass, retain those completed masks as well as native hidden bits
// observed during initialization. Later passes must never recapture departed
// or temporarily absent spectators as a new original audience.
inline bool finishInitialMasks(std::vector<unsigned short>& masks,const std::vector<unsigned short>& completed){
    if(masks.size()!=completed.size()||masks.empty())return false;
    for(size_t i=0;i<masks.size();++i)masks[i]|=completed[i];
    return true;
}
// Once the completed native baseline identifies an empty position, protect it
// across every model part. Recoloring can introduce new body/clothing parts;
// their first callback must not turn an authored empty seat into a spectator.
inline void freezeEmptyPositions(std::vector<unsigned short>& masks,std::vector<unsigned short>& seen,unsigned short parts){
    if(!parts||seen.size()!=masks.size())return;
    for(size_t i=0;i<masks.size();++i)if((masks[i]&parts)==parts){masks[i]=0xffff;seen[i]=0xffff;}
}
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
    void remember(const std::vector<Position>& positions,const std::vector<unsigned short>& masks,unsigned short parts,uint64_t layoutKey=0){
        if(!parts||positions.size()!=masks.size()||positions.size()<512||positions.size()>200000||entries_.size()>=64)return;
        if(!layoutKey)layoutKey=key(positions);
        if(entries_.find(layoutKey)==entries_.end())entries_.emplace(layoutKey,Entry{positions,masks,parts});
    }
    bool restore(const std::vector<Position>& positions,std::vector<unsigned short>& masks,std::vector<unsigned short>& seen,unsigned short& parts,uint64_t layoutKey=0)const{
        auto found=entries_.find(layoutKey?layoutKey:key(positions));if(found==entries_.end()||!same(found->second.positions,positions))return false;
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
