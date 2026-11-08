#pragma once
#include "DefensiveAccents.h"
namespace attendance::defensivemotion {
inline bool routed(live::Event e){using live::Event;return e==Event::Sack||e==Event::TackleForLoss||e==Event::GoalLineStop||e==Event::ThirdGoalStop||e==Event::CriticalStop||e==Event::DownsStop||e==Event::CatchKnockout||e==Event::PassBreakup||e==Event::HitStickStop;}
inline uint64_t cameraId(const live::Outcome& sound,uint64_t soundId,const live::Outcome& defensive){
    if(defensive.id)return defensive.id;
    return sound.id&&!routed(sound.event)&&live::cameraReaction(sound.event)?soundId:0;
}
// Audio retains its own native classifications. Motion gets one decision per
// play from measured loss or a confirmed stop, with optional contact flavor.
class Model {
    uintptr_t match_=0;unsigned epoch_=0;uint64_t sequence_=0,last_=0,until_=0,serial_=0;
    bool used_=false,confirmed_=false;live::Event flavor_=live::Event::None;
    energy::Outcome result_;energy::Play snap_;
public:
    void reset(){*this=Model{};}
    void suspend(){until_=0;confirmed_=false;used_=true;flavor_=live::Event::None;}
    live::Outcome observe(const energy::Play& snap,const energy::Play& p,const energy::Outcome& result,const live::Outcome& evidence,
        uintptr_t match,unsigned epoch,uint64_t sequence,uint64_t now,bool allowed,uint64_t priorMotion=0){
        if(match!=match_||epoch!=epoch_){reset();match_=match;epoch_=epoch;}
        if(sequence!=sequence_){sequence_=sequence;used_=confirmed_=false;until_=0;flavor_=live::Event::None;}
        if(!match||!sequence||!valid(p.game)||p.simulated||now<last_||(last_&&now-last_>750)){suspend();last_=now;return {};}
        last_=now;
        if(used_)return {};
        if(!allowed){suspend();return {};}
        bool post=p.game.phase==4||p.game.phase==5||p.game.phase==1;
        if(p.game.phase!=3&&!post){suspend();return {};}
        // These events already have a verified, once-per-snap official result.
        // A restored game's persistent resume marker must not suppress fresh
        // plays; a raw resume snapshot has no accepted tracked result here.
        if(overtime(snap.game)&&post&&result.accepted&&result.completed&&result.sequence==sequence&&priorMotion!=sequence){
            auto special=live::Event::None;int side=-1;
            int points=result.side==0?result.homePoints:result.awayPoints;
            if(energy::overtimeTry(snap)&&snap.selectedType!=21&&points==2){special=live::Event::OvertimeConversion;side=result.side;}
            else if(energy::overtimeTry(snap)&&snap.selectedType!=21&&result.event==energy::Event::ConversionStop&&!result.homePoints&&!result.awayPoints){special=live::Event::OvertimeConversionStop;side=1-snap.offense;}
            else if(energy::normalDown(snap)&&snap.selectedType==21){
                if(points==3){special=live::Event::OvertimeFieldGoal;side=result.side;}
                else if(!result.homePoints&&!result.awayPoints&&!result.blockedFieldGoal&&!result.takeaway&&p.offense!=snap.offense){special=live::Event::OvertimeMissedFieldGoal;side=1-snap.offense;}
            }
            if(special!=live::Event::None&&side>=0&&side<=1&&(p.game.neutral||side==0)){
                used_=true;return {special,(uint64_t{1}<<63)|++serial_,side,0};
            }
        }
        if(!energy::normalDown(snap)||snap.specialTeams||energy::clockManagement(snap)||(!p.game.neutral&&snap.offense!=1))return {};
        if(p.interception||p.fumble||p.game.homeScore!=snap.game.homeScore||p.game.awayScore!=snap.game.awayScore){suspend();return {};}
        if(priorMotion==sequence||evidence.event==live::Event::ForcedFumble||evidence.event==live::Event::Interception||evidence.event==live::Event::BlockedFieldGoal){used_=true;return {};}
        if(evidence.id&&evidence.side==1-snap.offense){
            if(evidence.event==live::Event::CatchKnockout||evidence.event==live::Event::PassBreakup)flavor_=evidence.event;
            else if((evidence.event==live::Event::HitStick||evidence.event==live::Event::HitStickStop)&&flavor_==live::Event::None)flavor_=live::Event::HitStickStop;
        }
        if(result.accepted&&result.completed&&result.sequence==sequence){
            if(result.homePoints||result.awayPoints||result.takeaway||result.firstDown||result.forcedFumble||result.blockedFieldGoal){suspend();return {};}
            result_=result;snap_=snap;if(!confirmed_)until_=now+5000;confirmed_=true;
        }
        if(!post||!confirmed_||now>until_)return {};
        // No sack predicate participates in this choice. Both a sack and a
        // runner's loss qualify only through the same verified 5+ yard result.
        bool loss=result_.fiveYardLoss&&std::isfinite(result_.yards)&&result_.yards>=-100&&result_.yards<=-4.99f;
        bool stop=defense::stop(snap_,p,result_,sequence);
        bool hitLoss=flavor_==live::Event::HitStickStop&&std::isfinite(result_.yards)&&result_.yards>=-100&&result_.yards<0;
        auto event=live::Event::None;
        if(stop&&snap_.down==4&&result_.goalLineStop)event=live::Event::GoalLineStop;
        else if(hitLoss)event=live::Event::HitStickStop;
        else if(loss)event=live::Event::TackleForLoss;
        else if(stop){
            if(result_.highLeverageStop&&snap_.game.quarter==4)event=live::Event::CriticalStop;
            else if(flavor_!=live::Event::None)event=flavor_;
            else if(result_.turnoverOnDowns)event=live::Event::DownsStop;
            else if(overtime(snap_.game))event=live::Event::OvertimeStop;
        }
        if(event==live::Event::None)return {};
        used_=true;return {event,(uint64_t{1}<<63)|++serial_,1-snap.offense,loss?result_.yards:0};
    }
};
}
