#pragma once
#include "ExcitementRules.h"
namespace attendance::drive {
struct Model {
    float heat=0,highTime=-1,length=0;unsigned chain=0;int offense=-1,quarter=0,home=-1,away=-1;
    uint64_t sequence=0;
    void clear(){heat=0;chain=0;}
    void reset(){*this=Model{};}
    void observe(const energy::Play& p,const energy::Outcome* result=nullptr){
        const auto& s=p.game;if(!valid(s))return;
        float time=elapsedGameTime(s);
        if(highTime>=0&&s.quarter<=4&&time+0.01f<highTime)return;
        if(quarter&&s.quarter<=4&&quarter<=4&&std::fabs(length-s.quarterSeconds)<.01f&&highTime>=0){
            heat*=std::exp2(-(std::max)(0.0f,time-highTime)/(s.quarterSeconds/8));
        }
        bool possession=(s.phase==1||s.phase==5)&&energy::normalDown(p)&&offense>=0&&p.offense!=offense;
        bool score=home>=0&&(s.homeScore!=home||s.awayScore!=away);
        bool halftime=quarter==2&&s.quarter>=3;
        bool changedLength=length>0&&std::fabs(length-s.quarterSeconds)>=.01f;
        if(possession||score||halftime||changedLength||s.phase==7||p.simulated||(quarter<=4&&quarter>0&&overtime(s)))clear();
        if((s.phase==1||s.phase==5)&&energy::normalDown(p))offense=p.offense;
        quarter=s.quarter;length=s.quarterSeconds;home=s.homeScore;away=s.awayScore;highTime=(std::max)(highTime,time);
        if(!result||!result->accepted||!result->completed||!result->sequence||result->sequence==sequence)return;
        sequence=result->sequence;
        if(result->homePoints||result->awayPoints||result->takeaway||p.simulated||s.phase==7){clear();return;}
        if(!energy::normalDown(p)||(!s.neutral&&p.offense!=0)||result->side!=p.offense||possession)return;
        bool first=result->firstDown||result->event==energy::Event::ExplosiveGain||result->event==energy::Event::ThirdDownConversion||result->event==energy::Event::FourthDown;
        float add=first?.18f+.04f*(std::min)(chain,3u):result->yards>=5?.07f:0;
        if(result->event==energy::Event::ThirdDownConversion)add+=.10f;
        if(result->event==energy::Event::FourthDown)add+=.18f;
        if(add>0){if(first)chain=(std::min)(4u,chain+1);heat=(std::min)(1.0f,heat+add);}
        else{heat*=result->yards<0?.55f:.8f;chain=0;}
        if(heat<.005f)heat=0;
    }
};
}
