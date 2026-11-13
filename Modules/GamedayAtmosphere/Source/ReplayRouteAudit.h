#pragma once
#include <array>
#include <cstdint>
namespace attendance::replayrouteaudit {
struct Record {
    uintptr_t match=0;unsigned epoch=0;uint64_t sequence=0;
    int route=0,phase=0,native=-1,output=-1,yards=-1000;unsigned exclusions=0;
    bool operator==(const Record& other)const{
        return match==other.match&&epoch==other.epoch&&sequence==other.sequence&&
            route==other.route&&phase==other.phase&&native==other.native&&output==other.output&&
            yards==other.yards&&exclusions==other.exclusions;
    }
};
// Fixed memory and at most sixteen distinct records per snap. Repeated native
// evaluations never flood logs or retain game objects for later dereferencing.
class Gate {
    std::array<Record,16> rows_{};unsigned count_=0;
    uintptr_t match_=0;unsigned epoch_=0;uint64_t sequence_=0;
public:
    bool accept(Record record){
        if(!record.match||!record.sequence)return false;
        if(record.match!=match_||record.epoch!=epoch_||record.sequence!=sequence_){
            match_=record.match;epoch_=record.epoch;sequence_=record.sequence;count_=0;
        }
        for(unsigned i=0;i<count_;++i)if(rows_[i]==record)return false;
        if(count_==rows_.size())return false;rows_[count_++]=record;return true;
    }
};
}
