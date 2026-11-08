#pragma once
#include "LiveReactions.h"
#include "GameBindings.h"
namespace attendance::knockout {
struct Sample {uintptr_t object=0;bool valid=false,knockout=false;};
template<class Read>Sample snapshot(uintptr_t image,Read read){
    auto number=[&](uintptr_t p,auto& value){return p>=0x10000&&read(p,&value,sizeof(value));};
    uintptr_t object=0,vt=0,again=0;unsigned char flag=0;
    if(!number(image+bindings::CatchSummary,object)||!number(object,vt)||vt!=image+bindings::CatchSummaryVtable||
       !number(object+0x175,flag)||flag>1||!number(image+bindings::CatchSummary,again)||again!=object)return {};
    return {object,true,flag!=0};
}
class Model {
    uintptr_t match_=0,object_=0;unsigned epoch_=0;uint64_t last_=0,began_=0,ended_=0,serial_=0;
    int phase_=-1,side_=-1;bool ready_=false,live_=false,cleared_=false,proof_=false,used_=false;
public:
    void reset(){*this=Model{};}
    bool proof()const{return proof_;}
    bool cleared()const{return cleared_;}
    live::Outcome observe(const energy::Play& p,uintptr_t match,unsigned epoch,uint64_t now,bool allowed,const Sample& s,int kind,bool held){
        if(match!=match_||epoch!=epoch_){reset();match_=match;epoch_=epoch;}
        if(!allowed||!match||p.simulated||!valid(p.game)||now<last_||(last_&&now-last_>750)){ready_=live_=false;phase_=-1;last_=now;return {};}
        last_=now;
        if(p.game.phase==1){ready_=energy::normalDown(p)&&(p.down==3||p.down==4)&&(p.offense==1||p.game.neutral)&&!p.specialTeams;side_=p.offense;live_=false;object_=s.valid?s.object:0;}
        if(p.game.phase==3&&phase_==1&&ready_){live_=true;began_=now;ended_=0;cleared_=proof_=used_=false;}
        if(live_&&phase_==3&&(p.game.phase==4||p.game.phase==5))ended_=now;
        bool post=(p.game.phase==4||p.game.phase==5)&&ended_&&now>=ended_&&now-ended_<=5000;
        bool active=live_&&now>=began_&&now-began_<=45000&&(p.game.phase==3||post);
        if(active&&s.valid&&object_&&s.object==object_){
            if(!s.knockout)cleared_=true;
            if(cleared_&&s.knockout)proof_=true;
        }
        live::Outcome out;
        // A fresh native knockout plus its dropped-pass result distinguishes
        // contact at the catch from a normal incompletion or receiver drop.
        // Contact breakups can publish kind 5 (observed with a fresh native
        // knockout in Texas), while dropped catches can publish kind 4.
        if(active&&proof_&&!used_&&(kind==4||kind==5)&&!held&&!p.interception&&!p.fumble){used_=true;out={live::Event::CatchKnockout,++serial_,1-side_,0};}
        if(p.game.phase!=1&&p.game.phase!=3&&!post)ready_=live_=false;
        phase_=p.game.phase;return out;
    }
};
}
