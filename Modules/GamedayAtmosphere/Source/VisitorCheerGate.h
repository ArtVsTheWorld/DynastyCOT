#pragma once
#include "AwayCrowd.h"
#include "SustainedCrowd.h"
namespace attendance::visitorcheers {
constexpr uint64_t FreshMilliseconds=3000;
inline bool eligible(const visitors::Population& p,uintptr_t match,unsigned epoch,uint64_t now){
    // Bands are absent from Population; protected non-band spectators count.
    // The denominator is the current owned audience after all absences.
    if(!p.owns(match,epoch)||(!p.originalHome&&!p.originalAway)||!p.updated||now<p.updated||now-p.updated>FreshMilliseconds)return false;
    if(p.home>12800000||p.away>12800000||p.home+p.away>12800000)return false;
    return p.away>=(p.home+p.away+4)/5; // Exact inclusive 20%, no float rounding.
}
inline bool requested(const sustained::Intent& i){return (i.opponentCheer&&i.opponentPositive)||i.visitorLayer;}
inline sustained::Intent restrain(sustained::Intent i,bool allowed){
    if(allowed||i.spring||i.reason==sustained::Reason::Diagnostics)return i;
    if(i.opponentPositive){i.opponentCheer=0;i.opponentCheerUntil=0;}
    if(i.visitorLayer){
        i.reason=sustained::Reason::None;i.intensity=0;i.continuous=i.reaction=i.supporting=false;
        i.earnedReaction=false;i.reactionHoldUntil=i.reactionUntil=0;
    }
    return i;
}
}
