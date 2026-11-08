#pragma once
#include "NativeSkillSignals.h"
#include <map>
namespace attendance::showboating {
struct Sample {uintptr_t carrier=0;int side=-1;uint32_t actor=0;bool valid=false;std::array<uintptr_t,3> tags{};unsigned celebration=0;};
// Shared lightweight carrier identity. Unlike the global offense getter this
// belongs to the held player, including during a defensive return handoff.
inline int carrierSide(uintptr_t image,skills::Read read){
    auto field=[&](uintptr_t p,auto& v){return p>=0x10000&&read(p,&v,sizeof(v));};
    uintptr_t root=0,control=0,vt=0,carrier=0,again=0;int ball=-1,side=-1,slot=-1;uint32_t actor=0;
    if(!field(image+bindings::BallControl,root)||!field(root+0x10,control)||
       !field(control,vt)||vt!=image+0xb47bb10||!field(control+0x168,ball)||
       (ball!=2&&ball!=3&&ball!=4&&ball!=9)||!field(control+0x1a0,carrier)||
       !field(carrier,vt)||vt!=image+bindings::ShowboatPlayerVtable||
       !field(carrier+0x138,side)||!field(carrier+0x13c,slot)||!field(carrier+0x64,actor)||
       (side!=0&&side!=1)||slot<0||slot>255||actor!=(static_cast<uint32_t>(slot)<<16|static_cast<uint32_t>(side)<<8|4u)||
       !field(control+0x1a0,again)||again!=carrier)return -1;
    return side;
}
inline Sample snapshot(uintptr_t image,skills::Read read){
    auto field=[&](uintptr_t p,auto& value){return p>=0x10000&&p<UINTPTR_MAX-sizeof(value)&&read(p,&value,sizeof(value));};
    constexpr unsigned char signature[]{0xb8,0x1f,0,0x40,0,0xc3};unsigned char code[sizeof(signature)]{};
    if(!read(image+bindings::ShowboatPlayerType,code,sizeof(code))||std::memcmp(code,signature,sizeof(code)))return {};
    uintptr_t root=0,control=0,vt=0,again=0;Sample out;int slot=-1,ball=-1;
    if(!field(image+bindings::BallControl,root)||!field(root+0x10,control)||!field(control,vt)||vt!=image+0xb47bb10||
       !field(control+0x168,ball)||(ball!=2&&ball!=3&&ball!=4&&ball!=9)||
       !field(control+0x1a0,out.carrier)||!field(out.carrier,vt)||vt!=image+bindings::ShowboatPlayerVtable||
       !field(out.carrier+0x64,out.actor)||!field(out.carrier+0x138,out.side)||!field(out.carrier+0x13c,slot)||
       (out.side!=0&&out.side!=1)||slot<0||slot>255||out.actor!=(static_cast<uint32_t>(slot)<<16|static_cast<uint32_t>(out.side)<<8|4u)||
       !read(out.carrier+0x1418,out.tags.data(),sizeof(out.tags)))return {};
    constexpr char name[]="PChat_Loco_Celebration";
    for(unsigned n=0;n<3;++n){uintptr_t tag=out.tags[n],value=0,text=0;int kind=-1;char label[sizeof(name)]{};
        if(!tag)continue;
        if(!field(tag,vt))return {};
        if(vt!=image+bindings::ShowboatAudioTagVtable)continue;
        if(!field(tag+0x48,value)||!field(value,vt))return {};
        if(vt!=image+bindings::ShowboatEnumValueVtable)continue;
        if(!field(value+0x38,kind)||!field(value+0x30,text))return {};
        if(kind==2&&text>=0x10000&&read(text,label,sizeof(label))&&!std::memcmp(label,name,sizeof(name)))out.celebration|=1u<<n;
    }
    std::array<uintptr_t,3> stable{};uint32_t actor=0;
    if(!field(control+0x1a0,again)||again!=out.carrier||!field(out.carrier+0x64,actor)||actor!=out.actor||
       !read(out.carrier+0x1418,stable.data(),sizeof(stable))||stable!=out.tags)return {};
    out.valid=true;return out;
}
struct Event {uint64_t id=0;int side=-1;};
class Model {
    struct Carrier {uint32_t actor=0;std::array<uintptr_t,3> tags{};};
    std::map<uintptr_t,Carrier> carriers_;
    uintptr_t match_=0,carrier_=0,observedCarrier_=0;unsigned epoch_=0;uint64_t sequence_=0,last_=0,serial_=0,began_=0,id_=0;
    bool used_=false;
public:
    void reset(){*this=Model{};}
    void suspend(){carriers_.clear();observedCarrier_=0;began_=id_=0;used_=true;}
    Event observe(const energy::Play& snap,const energy::Play& p,uintptr_t match,unsigned epoch,uint64_t sequence,uint64_t now,bool allowed,bool held,const Sample& s){
        if(match!=match_||epoch!=epoch_){reset();match_=match;epoch_=epoch;}
        if(sequence!=sequence_){sequence_=sequence;used_=false;began_=id_=0;}
        if(!match||!sequence||!valid(p.game)||p.simulated||p.resumed||now<last_||(last_&&now-last_>750)){suspend();last_=now;return {};}
        last_=now;
        bool fresh=false;
        if(s.valid){
            auto found=carriers_.find(s.carrier);
            if(held&&observedCarrier_==s.carrier&&found!=carriers_.end()&&found->second.actor==s.actor){
                for(unsigned n=0;n<3;++n)if((s.celebration&(1u<<n))&&s.tags[n]&&
                    std::find(found->second.tags.begin(),found->second.tags.end(),s.tags[n])==found->second.tags.end())fresh=true;
            }
            if(carriers_.size()>=64&&found==carriers_.end())carriers_.clear();
            carriers_[s.carrier]={s.actor,s.tags};
        }
        observedCarrier_=s.valid&&held?s.carrier:0;
        bool ordinary=energy::normalDown(snap)&&!snap.specialTeams;
        bool kickReturn=(snap.selectedType==22||snap.selectedType==23||snap.selectedType==24)&&
            snap.offense>=0&&snap.offense<=1&&s.side==1-snap.offense;
        bool live=allowed&&p.game.phase==3&&held&&s.valid&&p.offense==s.side&&!p.fumble&&
            !energy::clockManagement(snap)&&(ordinary||kickReturn);
        if(!live||s.carrier!=carrier_)began_=id_=0;
        if(!allowed)used_=true;
        if(!fresh||used_)return {};
        used_=true; // Rejected animation events are consumed, never queued for later admission.
        if(!live)return {};
        Event out{++serial_,s.side};carrier_=s.carrier;
        if(s.side==0||p.game.neutral){began_=now;id_=(uint64_t{1}<<62)|out.id;}
        return out;
    }
    live::RunBuildup buildup(uint64_t now)const{
        if(!began_||now<began_||now-began_>=4500)return {};
        return {id_,(std::clamp)(static_cast<float>(now-began_)/2500.f,.0001f,1.f),0};
    }
};
}
