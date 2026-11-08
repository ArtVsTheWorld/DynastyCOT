#pragma once
#include "LiveReactions.h"
#include "GameBindings.h"
#include <array>
#include <cstring>

namespace attendance::skills {
using Address=uintptr_t;
using Read=bool(*)(Address,void*,size_t);
inline constexpr Address TypeBuckets=0xf8cce08,TypeCount=0xf8cce10,EventRoot=0xfad50e0,ActorRoot=0xfad4e58;
struct Record {int type=-1,side=-1;double time=0;uint32_t actor=0;};
struct Sample {bool timesValid=false,eventsValid=false;std::array<double,6> times{};std::array<Record,45> events{};int hurdleCount=-1;};
// Read-only counterparts of the inspected component and actor getters. Every
// chain is bounded; no engine function is called and no game pointer is kept.
class View {
    Address image_;Read read_;
    template<class T>bool field(Address p,T& out)const{return p>=0x10000&&p<UINTPTR_MAX-sizeof(T)&&read_(p,&out,sizeof(T));}
    template<class T>T at(Address p)const{T out{};return field(p,out)?out:T{};}
    Address component()const{
        unsigned count=at<unsigned>(image_+TypeCount);Address buckets=at<Address>(image_+TypeBuckets);
        if(!count||count>100000||!buckets)return 0;
        constexpr unsigned hash=0x3784eae8;Address node=at<Address>(buckets+(hash%count)*8),type=0;
        for(unsigned n=0;node&&n<32;++n){if(at<unsigned>(node)==hash){type=at<Address>(node+8);break;}node=at<Address>(node+16);}
        Address storage=at<Address>(type+0xc8);count=at<unsigned>(storage+0x38);buckets=at<Address>(storage+0x30);
        if(!count||count>100000||!buckets||!at<Address>(storage+0x20))return 0;
        constexpr uint64_t key=0x4f00000000000000ull;node=at<Address>(buckets+(key%count)*8);
        for(unsigned n=0;node&&n<32;++n){if(at<uint64_t>(node)==key){Address value=at<Address>(node+8);return at<uint64_t>(node)==key?value:0;}node=at<Address>(node+24);}
        return 0;
    }
    int side(uint32_t key)const{
        if((key&255)!=4||((key>>8)&255)>1||(key>>16)>255)return -1;
        Address root=at<Address>(image_+ActorRoot),buckets=at<Address>(root+0x170),begin=at<Address>(root+0x28),end=at<Address>(root+0x30);
        unsigned count=at<unsigned>(root+0x178);
        if(!count||count>100000||!buckets||!begin||end<begin||end-begin>4096*8||(end-begin)%8)return -1;
        Address node=at<Address>(buckets+(key%count)*8);
        for(unsigned n=0;node&&n<32;++n){
            if(at<uint32_t>(node)==key){
                int index=at<int>(node+4);if(index<0||static_cast<uint64_t>(index)>=(end-begin)/8)return -1;
                Address actor=at<Address>(begin+static_cast<Address>(index)*8),data=at<Address>(actor+0x18);
                int team=-1,slot=-1,player=-1;
                if(!data||at<unsigned char>(data+0x44)!=1||!field(data+0x304,team)||!field(data+0x308,slot)||!field(data+0x310,player))return -1;
                return team==static_cast<int>((key>>8)&255)&&slot==static_cast<int>(key>>16)&&player>=0&&player<0xfffff&&
                    at<Address>(actor+0x18)==data&&at<uint32_t>(node)==key?team:-1;
            }
            node=at<Address>(node+8);
        }
        return -1;
    }
public:
    View(Address image,Read read):image_(image),read_(read){}
    int actorSide(uint32_t key)const{return side(key);}
    Sample snapshot(const std::array<double,4>& seen={},bool counts=true)const{
        Sample out;Address value=component();
        if(value&&read_(value+0x1640,out.times.data(),sizeof(out.times))){
            out.timesValid=value==component();for(double t:out.times)out.timesValid=out.timesValid&&std::isfinite(t)&&t>=0&&t<1e9;
        }
        Address counters=counts?at<Address>(image_+bindings::SkillCounters):0;int hurdles=-1;
        if(counters&&at<Address>(counters)==image_+bindings::SkillCountersVtable&&field(counters+0x860,hurdles)&&hurdles>=0&&hurdles<=32&&at<Address>(image_+bindings::SkillCounters)==counters)out.hurdleCount=hurdles;
        Address root=at<Address>(image_+EventRoot);std::array<unsigned char,45*32> raw{};
        if(at<Address>(root)!=image_+0xbc90128||!read_(root+0x10,raw.data(),raw.size())||at<Address>(image_+EventRoot)!=root)return out;
        out.eventsValid=true;
        for(unsigned n=0;n<45;++n){const auto* b=raw.data()+n*32;int type=-1;double time=0;uint32_t actor=0;
            std::memcpy(&type,b+24,4);std::memcpy(&time,b+16,8);std::memcpy(&actor,b+28,4);
            if((type!=31&&type!=35&&type!=52&&type!=54)||!std::isfinite(time)||time<=0||time>=1e9)continue;
            unsigned index=type==31?0:type==35?1:type==52?2:3;
            out.events[n]={type,time>seen[index]?side(actor):-1,time,actor};
        }
        return out;
    }
};
class Model {
    Address match_=0;unsigned epoch_=0;uint64_t last_=0,serial_=0,lastAccent_=0;int phase_=-1;
    bool ready_=false,live_=false,haveTimes_=false,haveEvents_=false;unsigned accents_=0;uint64_t snap_=0;
    uint64_t ended_=0;int snapOffense_=-1,hurdles_=-1;bool hurdleSent_=false;
    std::array<double,6> times_{};std::array<double,4> events_{};
    static int eventIndex(int type){return type==31?0:type==35?1:type==52?2:type==54?3:-1;}
    void baseline(const Sample& s){
        if(s.timesValid){times_=s.times;haveTimes_=true;}
        if(s.eventsValid){for(const auto& e:s.events){int n=eventIndex(e.type);if(n>=0)events_[n]=(std::max)(events_[n],e.time);}haveEvents_=true;}
    }
public:
    void reset(){*this=Model{};}
    const std::array<double,4>& eventTimes()const{return events_;}
    live::Outcome observe(const energy::Play& p,Address match,unsigned epoch,uint64_t now,bool allowed,bool held,const Sample& s){
        if(match!=match_||epoch!=epoch_){reset();match_=match;epoch_=epoch;}
        if(!allowed||!match||p.simulated||!valid(p.game)||now<last_||(last_&&now-last_>750)){ready_=live_=false;phase_=-1;haveTimes_=false;hurdles_=-1;ended_=0;last_=now;return {};}
        last_=now;
        if(p.game.phase==1){ready_=energy::normalDown(p)||p.down==0;live_=false;baseline(s);}
        if(p.game.phase==3&&phase_==1&&ready_){live_=true;accents_=0;snap_=now;lastAccent_=ended_=0;snapOffense_=p.offense;hurdleSent_=false;}
        bool admitted=live_&&p.game.phase==3&&now>=snap_&&now-snap_<45000;
        if(live_&&phase_==3&&(p.game.phase==4||p.game.phase==5))ended_=now;
        bool settling=live_&&ended_&&now>=ended_&&now-ended_<=600&&(p.game.phase==4||p.game.phase==5);
        live::Outcome selected;
        // Success times are independent of the attempt counts, which settle
        // later and can include moves that never defeated a defender.
        if(s.timesValid){
            constexpr live::Event types[]{live::Event::Truck,live::Event::None,live::Event::StiffArm,live::Event::Juke,live::Event::Spin,live::Event::Hurdle};
            for(unsigned n=0;n<6;++n){bool fresh=haveTimes_&&s.times[n]>times_[n]&&s.times[n]>0;
                if(admitted&&held&&fresh&&types[n]!=live::Event::None&&(types[n]!=live::Event::Hurdle||!hurdleSent_)&&(p.offense==0||p.game.neutral)&&p.offense>=0&&p.offense<=1)selected={types[n],1,p.offense,0};
            }
            times_=s.times;haveTimes_=true;
        }
        if(s.eventsValid){for(const auto& e:s.events){int n=eventIndex(e.type);if(n<0)continue;
            bool fresh=haveEvents_&&e.time>events_[n];events_[n]=(std::max)(events_[n],e.time);
            if(!admitted||!fresh||e.side<0||e.side>1||(!p.game.neutral&&e.side!=0))continue;
            bool hit=e.type==35||e.type==52||e.type==54;
            if(hit||!selected.id)selected={hit?live::Event::HitStick:live::Event::BrokenTackle,1,e.side,0};
        }haveEvents_=true;}
        // The credited hurdle counter settles about 0.27 seconds after the
        // whistle in captured plays; the fake-out timestamp can remain zero.
        // Require a fresh increase during this snap, never a positive old count.
        bool credited=s.hurdleCount>=0&&hurdles_>=0&&s.hurdleCount>hurdles_;
        hurdles_=s.hurdleCount;
        if(!selected.id&&!hurdleSent_&&credited&&(admitted||settling)&&snapOffense_>=0&&snapOffense_<=1&&(snapOffense_==0||p.game.neutral)&&p.offense==snapOffense_&&!p.interception&&!p.fumble)selected={live::Event::Hurdle,1,snapOffense_,0};
        if(selected.event==live::Event::Hurdle)hurdleSent_=true;
        if(p.game.phase!=1&&p.game.phase!=3&&!settling)ready_=live_=false;phase_=p.game.phase;
        if(!selected.id||accents_>=3||(lastAccent_&&now-lastAccent_<900))return {};
        ++accents_;lastAccent_=now;selected.id=++serial_;return selected;
    }
};
}
