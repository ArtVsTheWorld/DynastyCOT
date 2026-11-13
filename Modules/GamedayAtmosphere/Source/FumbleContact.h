#pragma once
#include "NativeSkillSignals.h"
namespace attendance::fumblecontact {
// The native per-play cache records the contacted carrier (8) and opposing
// contact player (9). These also occur on ordinary tackles: they are never
// sufficient without a subsequent held->loose->confirmed-fumble sequence.
struct Sample {uintptr_t object=0;bool valid=false;double took=0,made=0;int carrier=-1,defender=-1;};
struct Proof {uint64_t id=0;int owner=-1;};
inline Sample snapshot(uintptr_t image,skills::Read read,double seen=0){
    auto number=[&](uintptr_t p,auto& v){return p>=0x10000&&read(p,&v,sizeof(v));};
    uintptr_t root=0,vt=0,again=0;std::array<unsigned char,45*32> raw{};
    if(!number(image+skills::EventRoot,root)||!number(root,vt)||vt!=image+0xbc90128||!read(root+16,raw.data(),raw.size()))return {};
    Sample out{root,true};skills::View actors(image,read);
    for(unsigned n=0;n<45;++n){const auto* row=raw.data()+n*32;int type=-1;double time=0;uint32_t actor=0;
        std::memcpy(&type,row+24,4);std::memcpy(&time,row+16,8);std::memcpy(&actor,row+28,4);
        if((type!=8&&type!=9)||!std::isfinite(time)||time<=0||time>=1e9)continue;
        double& latest=type==8?out.took:out.made;int& side=type==8?out.carrier:out.defender;
        if(time<=latest)continue;latest=time;side=time>seen?actors.actorSide(actor):-1;
        std::array<unsigned char,32> stable{};
        if(!read(root+16+n*32,stable.data(),stable.size())||std::memcmp(row,stable.data(),32))return {};
    }
    return number(image+skills::EventRoot,again)&&again==root?out:Sample{};
}
class Model {
    uintptr_t match_=0,object_=0;unsigned epoch_=0;int phase_=-1,owner_=-1,contactOwner_=-1,home_=0,away_=0;
    uint64_t last_=0,began_=0,grounded_=0,loose_=0,contact_=0,ended_=0,serial_=0;
    double seen_=0;bool ready_=false,live_=false,baseline_=false,used_=false;
public:
    void reset(){*this=Model{};}
    double seen()const{return seen_;}
    Proof observe(const energy::Play& p,uintptr_t match,unsigned epoch,uint64_t now,bool allowed,
        const Sample& s,int ball,bool held,bool fumble){
        if(match!=match_||epoch!=epoch_){reset();match_=match;epoch_=epoch;}
        if(!allowed||!match||!valid(p.game)||p.simulated||p.resumed||now<last_||(last_&&now-last_>750)){
            ready_=live_=baseline_=false;phase_=-1;last_=now;return {};
        }
        last_=now;
        if(p.game.phase==1){
            ready_=energy::normalDown(p)||p.down==0;live_=false;object_=s.valid?s.object:0;
            baseline_=s.valid;seen_=s.valid?(std::max)(s.took,s.made):0;
        }
        if(p.game.phase==3&&phase_==1&&ready_){live_=true;began_=now;grounded_=loose_=contact_=ended_=0;owner_=contactOwner_=-1;home_=p.game.homeScore;away_=p.game.awayScore;used_=false;}
        if(live_&&phase_==3&&(p.game.phase==4||p.game.phase==5))ended_=now;
        bool post=(p.game.phase==4||p.game.phase==5)&&ended_&&now>=ended_&&now-ended_<=1500;
        bool active=live_&&p.game.homeScore==home_&&p.game.awayScore==away_&&now>=began_&&now-began_<=45000&&((p.game.phase==3&&!ended_)||post);
        if(active){
            if(p.game.phase==3&&!loose_&&held&&!fumble&&(ball==2||ball==3||ball==9)&&p.offense>=0&&p.offense<=1){if(!contact_||now>contact_+500)loose_=0;owner_=p.offense;grounded_=now;}
            if(!held&&(ball==0||fumble)&&grounded_&&!loose_)loose_=now;
            if(s.valid){
                if(s.object!=object_){object_=s.object;baseline_=false;contact_=0;}
                double pair=(std::min)(s.took,s.made);
                if(baseline_&&pair>seen_&&std::isfinite(pair)&&std::fabs(s.took-s.made)<=.075&&
                   grounded_&&now>=grounded_&&now-grounded_<=350&&s.carrier==owner_&&s.defender==1-owner_){
                    contact_=now;contactOwner_=owner_;if(loose_&&loose_+250<contact_)loose_=0;
                }
                // A split publication must not consume the first half of a
                // new pair before its matching contact arrives next frame.
                if(!baseline_)seen_=(std::max)(s.took,s.made);
                else if(std::fabs(s.took-s.made)<=.075)seen_=(std::min)(s.took,s.made);
                baseline_=true;
            }
        }
        Proof out;
        bool nearby=contact_&&loose_&&loose_+250>=contact_&&loose_<=contact_+500;
        if(active&&!used_&&fumble&&nearby&&now>=contact_&&now-contact_<=1500){used_=true;out={++serial_,contactOwner_};}
        if(p.game.phase!=1&&p.game.phase!=3&&!post)ready_=live_=false;
        phase_=p.game.phase;return out;
    }
};
}
