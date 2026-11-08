#pragma once
#include <array>
#include <cstdint>
namespace attendance::replaytrace {
// Borrowed addresses are opaque identities, never pointers for later reads.
struct Record {
    uintptr_t node=0;uint64_t at=0;int family=0,phase=0,native=-1,output=-1;
    int armed=0,stage=0,home=0,away=0,startHome=0,startAway=0,down=-1;
    uint64_t result=0,startResult=0;bool allowed=false,replaying=false;
};
class Buffer {
    uintptr_t match_=0;unsigned epoch_=0;uint64_t sequence_=0;
    std::array<Record,128> rows_{};unsigned count_=0,flushed_=0;bool confirmed_=false;
public:
    void reset(){*this=Buffer{};}
    void context(uintptr_t match,unsigned epoch,uint64_t sequence){
        if(match_==match&&epoch_==epoch&&sequence_==sequence)return;
        reset();match_=match;epoch_=epoch;sequence_=sequence;
    }
    void confirm(uintptr_t match,unsigned epoch,uint64_t sequence){context(match,epoch,sequence);confirmed_=true;}
    void add(uintptr_t match,unsigned epoch,uint64_t sequence,Record r){
        if(!match||!sequence)return;context(match,epoch,sequence);
        for(unsigned i=0;i<count_;++i){const auto& p=rows_[i];
            if(p.node==r.node&&p.family==r.family&&p.phase==r.phase&&p.native==r.native&&p.output==r.output&&
               p.armed==r.armed&&p.stage==r.stage&&p.home==r.home&&p.away==r.away&&p.allowed==r.allowed&&p.replaying==r.replaying)return;
        }
        if(count_<rows_.size())rows_[count_++]=r;
    }
    template<class Write>void flush(Write write){
        if(!confirmed_)return;
        while(flushed_<count_){write(sequence_,rows_[flushed_]);++flushed_;}
    }
};
}
