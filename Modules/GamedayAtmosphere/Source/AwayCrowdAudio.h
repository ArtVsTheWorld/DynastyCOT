#pragma once
#include "AwayCrowd.h"
#include "CrowdPopulation.h"
namespace attendance::visitors {
inline int opposite(int type){
    if(!catalog::usable(type))return 0;int f=type/4,tier=type%4;
    if(f<22){if(f==2)return catalog::type(24,tier);return catalog::type(f^1,tier);}
    return catalog::type(f==22?23:f==23||f==25?22:f==26?23:f,tier);
}
// Independent one-shot for the other supporters. Never consume or cancel the
// existing home response, skill slot, native voices or live camera event.
inline sustained::Intent audience(sustained::Intent out,const Population& pop,uint64_t positions,float fullness,uint64_t now){
    if(!out.active||!pop.owns(out.match,out.epoch)||!positions||pop.home+pop.away==0)return out;
    // Share of a stadium is not the visiting section's density. Use actual
    // modeled supporter counts and overall stadium fullness as a conservative
    // sparsity signal; do not invent alumni locations or section capacity.
    out.supporterSize=population::sizeForCount(static_cast<double>(pop.home),fullness);
    out.opponentSize=population::sizeForCount(static_cast<double>(pop.away),fullness);
    uint64_t id=0,until=0;int type=0,source=0;
    if(out.liveCheer&&now<out.liveCheerUntil){id=out.liveCheer;until=out.liveCheerUntil;type=out.liveType;source=1;}
    else if(out.resultCheer&&now<out.resultCheerUntil){id=out.resultCheer;until=out.resultCheerUntil;type=out.resultType;source=2;}
    else if(out.celebration&&now<out.celebrationUntil){id=out.celebration;until=out.celebrationUntil;type=out.celebrationType;source=3;}
    if(id&&pop.away&&catalog::usable(type)){
        out.opponentCheer=catalog::hash(id,static_cast<unsigned>(source)+811,out.epoch);
        if(!out.opponentCheer)out.opponentCheer=1;
        out.opponentCheerUntil=until;out.opponentType=opposite(type);
        out.opponentPositive=catalog::negative(type);
    }
    // No home supporters means no added home-side voice. Derive the visiting
    // response first; this does not touch EA's independently playing audio.
    if(!pop.home){out.liveCheer=out.resultCheer=out.celebration=out.extraCheer=0;}
    return out;
}
inline sustained::Intent layer(sustained::Intent home,const sustained::Intent& visitor,const Population& pop,uint64_t,float fullness){
    if(!pop.owns(home.match,home.epoch))return home;
    bool homeReaction=home.reason==sustained::Reason::Result||home.reason==sustained::Reason::Eruption||home.reason==sustained::Reason::Lineup;
    if(homeReaction)home.intensity*=reactionScale(pop.home,pop.home+pop.away);
    if(!visitor.active||!pop.away)return home;
    bool reaction=visitor.reason==sustained::Reason::Result||visitor.reason==sustained::Reason::Eruption;
    if(!reaction)return home;
    float visitorLevel=visitor.intensity*reactionScale(pop.away,pop.home+pop.away);
    if(visitorLevel<=home.intensity)return home;
    // Transfer only the sustained layer. The separate home and visiting
    // one-shots retain their IDs and never dispatch the same score twice.
    home.reason=visitor.reason;home.priority=visitor.priority;home.intensity=visitorLevel;
    home.continuous=visitor.continuous;home.reaction=visitor.reaction;home.supporting=visitor.supporting;
    home.attackMs=visitor.attackMs;home.releaseMs=visitor.releaseMs;
    home.earnedReaction=visitor.earnedReaction;home.reactionHoldUntil=visitor.reactionHoldUntil;home.reactionUntil=visitor.reactionUntil;
    home.size=(std::min)(visitor.size,population::sizeForCount(static_cast<double>(pop.away),fullness));
    return home;
}
}
