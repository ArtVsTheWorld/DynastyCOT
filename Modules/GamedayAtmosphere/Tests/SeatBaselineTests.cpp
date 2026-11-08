#include "../Source/SeatBaseline.h"
#include <cassert>
#include <cstdio>
using namespace attendance::seats;
int main(){
    Baselines cache;std::vector<Position> xyz(7468);std::vector<unsigned short> masks(xyz.size()),seen(xyz.size());
    for(unsigned i=0;i<xyz.size();++i){xyz[i]={float(i%100),float(i/100),float(i%17),0};masks[i]=i<2262?0:7;}
    cache.remember(xyz,masks,7);
    std::vector<unsigned short> fresh(xyz.size());unsigned short parts=0;
    assert(cache.restore(xyz,fresh,seen,parts)&&parts==7);
    for(unsigned i=0;i<xyz.size();++i){assert(fresh[i]==(i<2262?0:0xffff));assert(seen[i]==(i<2262?7:0xffff));}
    std::fill(fresh.begin(),fresh.end(),static_cast<unsigned short>(0));cache.remember(xyz,fresh,7); // Renderer defaults cannot overwrite the snapshot.
    for(auto& p:xyz)p.packed=17; // The fourth value is not geometry.
    assert(cache.restore(xyz,fresh,seen,parts)&&fresh[2262]==0xffff);
    xyz[0].x+=1;assert(!cache.restore(xyz,fresh,seen,parts));xyz[0].x-=1;
    cache.reset();assert(!cache.restore(xyz,fresh,seen,parts));
    cache.remember(xyz,std::vector<unsigned short>(xyz.size()),7);
    assert(cache.restore(xyz,fresh,seen,parts)&&fresh[2262]==0); // A new match can legitimately fill every position.
    std::puts("PASS: immutable native empty positions, exact geometry, replacement masks, packed metadata and new-match reset.");
}
