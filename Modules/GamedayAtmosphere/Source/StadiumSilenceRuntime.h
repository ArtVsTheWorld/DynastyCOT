#pragma once
#include "ChantMixerRuntime.h"
namespace attendance::silence {
// Empty seats retain only subdued individual detail, never collective crowd
// beds/reactions/chants. Scripted CrowdNIS is excluded from the detail branch.
// A root mute remains the fallback if any required child identity is missing.
class Runtime {
    inline static constexpr std::array<unsigned short,11> groups_{12,13,14,15,16,18,23,74,75,77,32};
    std::array<chants::Runtime,11> buses_;chants::Runtime detail_,fallback_,ambience_,space_;
    bool collective()const{for(unsigned i=0;i<7;++i)if(buses_[i].db()!=-60)return false;return true;}
public:
    void configure(uintptr_t image,chants::Note note=nullptr,chants::Create create=nullptr,chants::Release release=nullptr,chants::Manager manager=nullptr){
        for(auto& bus:buses_)bus.configure(image,note,create,release,manager);detail_.configure(image,note,create,release,manager);fallback_.configure(image,note,create,release,manager);
        ambience_.configure(image,note,create,release,manager,6,nullptr,11);space_.configure(image,note,create,release,manager,3,nullptr,86);
    }
    bool active()const{for(const auto& bus:buses_)if(bus.db()!=0)return true;return detail_.db()!=0||fallback_.db()!=0||ambience_.db()!=0||space_.db()!=0;}
    bool healthy()const{for(const auto& bus:buses_)if(!bus.healthy())return false;return detail_.healthy()&&fallback_.healthy()&&ambience_.healthy()&&space_.healthy();}
    void tick(uintptr_t owner,bool wanted,bool fieldContext=true){
        if(!wanted&&!active())return;
        for(unsigned i=0;i<buses_.size();++i)buses_[i].tick(owner,wanted,-60,false,true,groups_[i]);
        fallback_.tick(owner,wanted&&!collective(),-60,false,true,9);
        detail_.tick(owner,wanted,-3,false,false,17);
        ambience_.tick(owner,wanted&&fieldContext,6,false,false,11);space_.tick(owner,wanted&&fieldContext,3,false,false,86);
    }
    // Preserve the existing reporting/API order: crowd, away band, home band.
    bool muted(unsigned i)const{return i==0?(collective()||fallback_.db()==-60):i==1?buses_[7].db()==-60:i==2?buses_[8].db()==-60:false;}
    bool drumMuted()const{return buses_[9].db()==-60;}
    bool paMuted()const{return buses_[10].db()==-60;}
    float ambienceDb()const{return ambience_.db();}float spaceDb()const{return space_.db();}
    float detailDb()const{return fallback_.db()==-60? -60:detail_.db();}
};
}
