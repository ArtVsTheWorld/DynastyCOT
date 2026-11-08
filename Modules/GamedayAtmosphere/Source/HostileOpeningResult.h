#pragma once
#include "SustainedCrowd.h"
namespace attendance::openingresult {
// Each fresh away snap in the qualified opening series can own one response.
// First downs, punts, possession changes and scores end the series elsewhere;
// this model never turns a later drive into a new opening.
struct Model {
    uintptr_t match=0;unsigned epoch=0;bool armed=false,used=false,closed=false;
    uint64_t sequence=0,snapAt=0,until=0,cheer=0;
    int awayAtSnap=0;
    void reset(){*this=Model{};}
    void suspend(){until=0;}
    bool observe(const energy::Play& p,const energy::Play& snap,const energy::Outcome& result,
                 uintptr_t owner,unsigned generation,uint64_t serial,uint64_t now,bool qualifiedHostile,bool allowed,int resultKind,bool sawPass,bool seriesOpen=true){
        if(owner!=match||generation!=epoch){reset();match=owner;epoch=generation;}
        if(!owner||p.resumed||p.simulated||p.game.neutral||p.game.quarter<1||p.game.quarter>2){armed=false;closed=true;until=0;return false;}
        if(!seriesOpen){if(armed||sequence)closed=true;armed=false;until=0;return false;}
        if(closed||!allowed){until=0;return false;}
        if(qualifiedHostile&&p.offense==1&&energy::normalDown(p)&&!p.specialTeams&&(p.game.phase==1||p.game.phase==5))armed=true;
        if(armed&&serial&&serial!=sequence&&p.game.phase==3&&energy::normalDown(snap)&&snap.offense==1&&!snap.specialTeams){
            sequence=serial;snapAt=now;awayAtSnap=snap.game.awayScore;used=false;until=0;
        }
        if(!sequence||used)return false;
        if(serial!=sequence||now<snapAt||now-snapAt>45000){used=true;until=0;return false;}
        if(!result.accepted||!result.completed||result.sequence!=sequence)return false;
        used=true;
        // Stronger score/takeaway routes keep ownership. A short first-down
        // conversion is not a successful home defensive stop.
        if(result.homePoints||result.awayPoints||result.visitorScore||p.game.awayScore>awayAtSnap||result.takeaway||result.safety||result.firstDown){closed=true;armed=false;until=0;return false;}
        bool incomplete=sawPass&&(resultKind==4||resultKind==5);
        bool shortStop=std::isfinite(result.yards)&&result.yards>=-100&&result.yards<=2.01f;
        if(!incomplete&&!shortStop)return false;
        cheer=(uint64_t{1}<<59)|sequence;until=now+2500;return true;
    }
    sustained::Intent merge(sustained::Intent out,const energy::Play& p,uint64_t now)const{
        if(!out.active||out.match!=match||out.epoch!=epoch||!until||now>=until||p.simulated||p.resumed||p.game.neutral||
           (p.game.phase!=1&&p.game.phase!=4&&p.game.phase!=5)||out.celebration||out.scoringReaction)return out;
        // Promote the official-result slot when already present; otherwise use
        // one dedicated ID. Existing live accents and native audio keep playing.
        if(!out.resultCheer){out.resultCheer=cheer;out.resultCheerUntil=until;}
        out.resultType=91;out.cheers=true;out.hostileOpeningResult=true;return out;
    }
};
}
