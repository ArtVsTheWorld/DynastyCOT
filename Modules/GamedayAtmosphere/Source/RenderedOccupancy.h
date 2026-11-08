#pragma once
#include <map>
#include "StadiumOccupancy.h"

namespace attendance::occupancy {
struct Rendered {
    uintptr_t match=0;unsigned epoch=0,groups=0;
    uint64_t seats=0,original=0;
    bool valid()const{return match&&groups>0&&groups<=64&&seats>=512&&seats<=12800000&&original<=seats;}
    bool owns(uintptr_t owner,unsigned generation)const{return valid()&&match==owner&&epoch==generation;}
    float peak()const{return valid()?static_cast<float>(static_cast<double>(original)/seats):1;}
};
// Build only from verified whole seating layouts and their original masks.
// Neither camera culling, current replay masks nor tiny sideline crowds enter
// this denominator. The same geometry shared by two renderers counts once.
class RenderedBuilder {
    struct Group {uint64_t seats,original;};
    std::map<uintptr_t,Group> groups_;bool bad_=false;
public:
    void add(uintptr_t geometry,uint64_t seats,uint64_t original){
        if(!geometry||seats<512||seats>200000||original>seats){bad_=true;return;}
        auto found=groups_.find(geometry);
        if(found!=groups_.end()){
            if(found->second.seats!=seats||found->second.original!=original)bad_=true;
            return;
        }
        if(groups_.size()>=64){bad_=true;return;}
        groups_.emplace(geometry,Group{seats,original});
    }
    Rendered finish(uintptr_t owner,unsigned epoch,bool pending,const Rendered& previous={})const{
        Rendered candidate{owner,epoch,static_cast<unsigned>(groups_.size()),0,0};
        for(const auto& item:groups_){candidate.seats+=item.second.seats;candidate.original+=item.second.original;}
        bool known=previous.owns(owner,epoch);
        // Keep a verified whole layout while a streamed replacement is being
        // prepared. A disappearing draw must not shrink the audio denominator.
        if(bad_||pending||!candidate.valid()||(known&&candidate.seats<previous.seats))return known?previous:Rendered{};
        // Attendance is an original-match property, not a new renderer's default
        // all-visible mask. Equal geometry capacity cannot repopulate its native
        // empty seats. A verified new game changes the epoch and starts afresh.
        if(known&&candidate.seats==previous.seats)return previous;
        return candidate;
    }
};
inline float fraction(const Rendered& rendered,const Snapshot& nominal,uintptr_t owner,unsigned epoch,float remaining){
    if(!std::isfinite(remaining))return 0;
    float retained=(std::clamp)(remaining,0.0f,1.0f);
    return rendered.owns(owner,epoch)?retained*rendered.peak():fraction(nominal,owner,epoch,retained);
}
}
