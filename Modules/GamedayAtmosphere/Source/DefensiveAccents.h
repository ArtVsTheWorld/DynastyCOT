#pragma once
#include "NativeSkillSignals.h"
#include "DirectorShake.h"

namespace attendance::defense {
struct Sample {uintptr_t object=0;bool valid=false;double deflection=0;int side=-1;};
inline Sample snapshot(uintptr_t image,skills::Read read,double seen=0){
    auto number=[&](uintptr_t p,auto& value){return p>=0x10000&&read(p,&value,sizeof(value));};
    uintptr_t root=0,vt=0,again=0;std::array<unsigned char,45*32> raw{};
    if(!number(image+skills::EventRoot,root)||!number(root,vt)||vt!=image+0xbc90128||!read(root+0x10,raw.data(),raw.size()))return {};
    Sample out{root,true,0,-1};skills::View actors(image,read);
    for(unsigned n=0;n<45;++n){const auto* entry=raw.data()+n*32;int type=-1;double time=0;uint32_t actor=0;
        std::memcpy(&type,entry+24,4);std::memcpy(&time,entry+16,8);std::memcpy(&actor,entry+28,4);
        if(type!=39||!std::isfinite(time)||time<=0||time>=1e9||time<=out.deflection)continue;
        out.deflection=time;out.side=time>seen?actors.actorSide(actor):-1;
        std::array<unsigned char,32> stable{};
        if(!read(root+0x10+n*32,stable.data(),stable.size())||std::memcmp(entry,stable.data(),32))return {};
    }
    return number(image+skills::EventRoot,again)&&again==root?out:Sample{};
}
inline bool stop(const energy::Play& snap,const energy::Play& p,const energy::Outcome& result,uint64_t sequence){
    return energy::normalDown(snap)&&!snap.specialTeams&&(snap.down==3||snap.down==4)&&
        (snap.offense==1||p.game.neutral)&&snap.offense>=0&&snap.offense<=1&&
        result.accepted&&result.completed&&sequence&&result.sequence==sequence&&
        !result.homePoints&&!result.awayPoints&&!result.firstDown&&!result.takeaway&&!p.interception&&!p.fumble&&
        p.game.homeScore==snap.game.homeScore&&p.game.awayScore==snap.game.awayScore&&
        (p.offense!=snap.offense||(snap.down==3&&p.down==4));
}
inline motion::Profile shakeProfile(live::Event e){
    using E=live::Event;using P=motion::Profile;
    switch(e){
    case E::Truck:case E::HitStickStop:return P::Truck;
    case E::StiffArm:case E::Hurdle:return P::Skill;
    case E::LongCatch:case E::CompletedPass:return P::Catch;
    case E::LongRun:case E::RunAfterCatch:case E::KickReturn:case E::PuntReturn:return P::Gain;
    case E::OvertimeStop:case E::OvertimeConversionStop:case E::OvertimeMissedFieldGoal:case E::CatchKnockout:case E::PassBreakup:case E::ThirdGoalStop:return P::Stop;
    case E::Sack:case E::TackleForLoss:return P::Loss;
    case E::OvertimeConversion:case E::OvertimeFieldGoal:case E::FourthConversion:return P::Conversion;
    case E::DownsStop:return P::Downs;
    case E::Interception:case E::FumbleRecovery:return P::Takeaway;
    case E::ForcedFumble:return P::Force;
    case E::GoalLineStop:return P::GoalStop;
    case E::CriticalStop:return P::HighStop;
    default:return P::Default;
    }
}
inline void motion(const live::Outcome& event,motion::Input& out){
    out.profile=shakeProfile(event.event);
    if(event.event==live::Event::ForcedFumble){out.duringImpact=true;out.forcedImpact=true;}
    if(event.event==live::Event::DownsStop||event.event==live::Event::CatchKnockout||event.event==live::Event::PassBreakup){
        out.duringImpact=true;
    }
}
class Model {
    uintptr_t match_=0,object_=0;unsigned epoch_=0;uint64_t last_=0,sequence_=0,began_=0,ended_=0,serial_=0;
    int phase_=-1,side_=-1;double seen_=0,pendingSide_=0;bool baseline_=false,ready_=false,live_=false,tip_=false,hit_=false,used_=false;
public:
    void reset(){*this=Model{};}
    double seenDeflection()const{return seen_;}
    double lookupAfter()const{return pendingSide_?0:seen_;}
    bool deflectionProof()const{return tip_;}
    bool pendingSide()const{return pendingSide_!=0;}
    live::Outcome observe(const energy::Play& p,const energy::Play& snap,const energy::Outcome& result,
        uintptr_t match,unsigned epoch,uint64_t sequence,uint64_t now,bool allowed,const Sample& s,int kind,bool held,
        const live::Outcome& current,uint64_t previousMotion){
        if(match!=match_||epoch!=epoch_){reset();match_=match;epoch_=epoch;}
        if(!allowed||!match||p.simulated||p.resumed||!valid(p.game)||now<last_||(last_&&now-last_>750)){
            ready_=live_=baseline_=false;phase_=-1;last_=now;return {};
        }
        last_=now;
        auto prepare=[&](){
            ready_=energy::normalDown(p)&&!p.specialTeams&&(p.down==3||p.down==4)&&(p.offense==1||p.game.neutral);
            live_=false;pendingSide_=0;side_=p.offense;object_=s.valid?s.object:0;baseline_=s.valid;seen_=s.valid?s.deflection:0;
        };
        if(p.game.phase==1&&!live_)prepare();
        if(p.game.phase==3&&phase_==1&&ready_){live_=true;sequence_=sequence;began_=now;ended_=0;tip_=hit_=used_=false;}
        if(live_&&phase_==3&&p.game.phase!=3)ended_=now;
        bool post=(p.game.phase==4||p.game.phase==5||p.game.phase==1)&&ended_&&now>=ended_&&now-ended_<=5000;
        bool active=live_&&sequence==sequence_&&now>=began_&&now-began_<=45000&&(p.game.phase==3||post);
        if(active&&s.valid){
            if(s.object!=object_){object_=s.object;baseline_=false;tip_=false;pendingSide_=0;}
            bool fresh=baseline_&&std::isfinite(s.deflection)&&s.deflection>seen_;
            if(fresh&&s.side==1-side_){tip_=true;pendingSide_=0;}
            else if(fresh&&s.side<0)pendingSide_=s.deflection;
            if(pendingSide_&&s.deflection==pendingSide_&&s.side>=0){if(s.side==1-side_)tip_=true;pendingSide_=0;}
            seen_=s.deflection;baseline_=true;
        }
        if(active&&current.id&&current.event==live::Event::HitStick&&current.side==1-side_)hit_=true;
        // A knockout or a larger sack/goal-line/turnover impact already owns
        // this play. Consume overlapping proof instead of replaying an accent.
        if(active&&(previousMotion==sequence_||(current.id&&live::cameraReaction(current.event))))used_=true;
        live::Outcome out;
        if(active&&post&&!used_&&stop(snap,p,result,sequence)&&!held){
            bool incomplete=kind==3||kind==4||kind==5;
            auto event=tip_&&incomplete?live::Event::PassBreakup:hit_?live::Event::HitStickStop:live::Event::None;
            if(event!=live::Event::None){used_=true;out={event,++serial_,1-side_,0};}
        }else if(active&&post&&!used_&&hit_&&stop(snap,p,result,sequence)){
            // The tackler/runner can remain attached after a grounded tackle.
            used_=true;out={live::Event::HitStickStop,++serial_,1-side_,0};
        }
        if(p.game.phase==1)prepare();
        else if(p.game.phase!=3&&!post)ready_=live_=false;
        phase_=p.game.phase;return out;
    }
};
}
