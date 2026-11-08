#pragma once
#include <cstdint>
namespace attendance::turnout {
// A mode witness may bridge only the observed -1 loading sentinel, for ten
// seconds after a real offline Dynasty mode14 sample. -1 never renews it.
struct ModeWitness {
    static constexpr uint64_t Lifetime=10000;
    uint64_t verifiedAt=0,generation=1;bool armed=false;
    void reset(){if(armed)++generation;verifiedAt=0;armed=false;}
    uint64_t observe(int mode,bool safe,uint64_t now){
        if(!safe||(mode!=14&&mode!=-1)){reset();return 0;}
        if(mode==14){verifiedAt=now;armed=true;}
        if(!armed||!verifiedAt||now<verifiedAt||now-verifiedAt>Lifetime){reset();return 0;}
        return verifiedAt;
    }
};
}
