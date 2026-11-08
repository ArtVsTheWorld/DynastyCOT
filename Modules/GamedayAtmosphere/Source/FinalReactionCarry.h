#pragma once
#include "DirectorShake.h"
#include "SustainedCrowd.h"
namespace attendance::finalcarry {
struct Output {sustained::Intent sound;motion::Cue camera;bool carrying=false;};
class Model {
    uintptr_t match_=0;unsigned epoch_=0;uint64_t sequence_=0,last_=0,soundEnd_=0,fadeAt_=0,cameraEnd_=0,cameraFade_=0;
    bool ended_=false; sustained::Intent sound_;motion::Cue camera_;
    void clear(){sound_={};camera_={};soundEnd_=fadeAt_=cameraEnd_=cameraFade_=0;}
public:
    void reset(){*this=Model{};}
    void suspend(){clear();}
    Output observe(const State& s,uintptr_t match,unsigned epoch,uint64_t sequence,uint64_t now,
                   bool allowed,bool motionAllowed,const sustained::Intent& sound,const motion::Cue& camera,unsigned fade=5000){
        Output out{sound,camera,false};
        if(match!=match_||epoch!=epoch_){reset();match_=match;epoch_=epoch;}
        if(!allowed||!match||!valid(s)||now<last_){clear();last_=now;return out;}
        if(s.phase!=7){
            if(ended_){clear();last_=now;return out;}
            if(sequence!=sequence_){clear();sequence_=sequence;}
            bool reaction=sound.earnedReaction&&sustained::fresh(sound,match,epoch,now)&&sound.reason==sustained::Reason::Result&&!sound.celebration&&std::isfinite(sound.intensity)&&sound.intensity>0&&sequence;
            if(reaction)sound_=sound;else sound_={};
            bool earned=camera.kind==motion::Kind::LivePlay||camera.kind==motion::Kind::Touchdown||camera.kind==motion::Kind::Safety||camera.kind==motion::Kind::BlockedKick||camera.kind==motion::Kind::Clincher;
            if(earned&&motionAllowed&&sequence&&camera.match==match&&camera.epoch==epoch&&(!camera.runBuildup||camera.confirmedRun)&&motion::strength(camera,now)>0)camera_=camera;else camera_={};
            last_=now;return out;
        }
        if(!ended_){
            ended_=true;
            if(!sequence||sequence!=sequence_||!last_||now-last_>750)clear();
            unsigned normal=(std::clamp)(fade,1000u,8000u);
            unsigned cap=sound_.scoringReaction?11000:normal;
            soundEnd_=sound_.active?(sound_.reactionUntil?(std::min)(sound_.reactionUntil,now+cap):now+cap):0;
            fadeAt_=sound_.reactionHoldUntil?(std::clamp)(sound_.reactionHoldUntil,now,soundEnd_>now?soundEnd_:now):now;
            // Reserve a release at the end of a capped hold.
            if(soundEnd_>now)fadeAt_=(std::min)(fadeAt_,soundEnd_-(std::min)(uint64_t{2000},soundEnd_-now));
            if(camera_.kind==motion::Kind::LivePlay&&!camera_.tail&&!camera_.impactOnly){
                float level=motion::strength(camera_,now);camera_.began=now;camera_.until=now+normal;
                camera_.percent=level;camera_.tail=true;camera_.transitionAt=0;
            }
            if(camera_.kind!=motion::Kind::None){
                cameraEnd_=(std::min)(camera_.until,now+(camera_.kind==motion::Kind::LivePlay?normal:11000u));
                // A long custom duration must not become an eruption-length
                // postgame hold. Keep its original envelope/oscillation and
                // apply only a final release if this bounded cap is earlier.
                if(cameraEnd_<camera_.until)cameraFade_=cameraEnd_-(std::min)(uint64_t{2000},cameraEnd_>now?cameraEnd_-now:0);
            }
        }
        last_=now;
        if(!motionAllowed)camera_={};
        if(sound.reason==sustained::Reason::Eruption||camera.kind==motion::Kind::Eruption){clear();return out;}
        if(sound_.active&&now<soundEnd_){
            auto held=sound_;held.updated=now;held.match=match;held.epoch=epoch;
            float weight=now<=fadeAt_?1.f:1-motion::smooth(static_cast<float>(now-fadeAt_)/(std::max)(1.f,static_cast<float>(soundEnd_-fadeAt_)));
            held.intensity*=weight;held.active=held.intensity>0;held.continuous=now<fadeAt_;held.reaction=now>=fadeAt_;
            held.liveCheer=held.resultCheer=held.lineupCheer=held.celebration=0;
            held.cheers=held.deepCheers=held.staggered=held.overlapMilestone=false;
            held.shake=false;held.supporting=held.supporting&&!held.reaction;
            out.sound=held;out.carrying=held.active;
        }
        if(motionAllowed&&camera_.kind!=motion::Kind::None&&now<cameraEnd_){
            auto held=camera_;held.updated=now;
            float fadeWeight=cameraFade_&&now>cameraFade_?1-motion::smooth(static_cast<float>(now-cameraFade_)/static_cast<float>(cameraEnd_-cameraFade_)):1;
            held.percent*=fadeWeight;held.impact*=fadeWeight;held.transitionFrom*=fadeWeight;
            if(motion::strength(held,now)>0){out.camera=held;out.carrying=true;}
        }
        return out;
    }
};
}
