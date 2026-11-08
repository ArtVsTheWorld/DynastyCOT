#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include "ReactionShape.h"
#include "VenueMotion.h"
#include "ShakeProfiles.h"
#include "CameraProjection.h"

namespace attendance::motion {
enum class Kind { None,Ordinary,Critical,Touchdown,Eruption,LivePlay,Safety,BlockedKick,Clincher,Cannon };
inline const char* name(Kind k){switch(k){case Kind::Cannon:return "stadium cannon impact";case Kind::Clincher:return "game-sealing first down";case Kind::BlockedKick:return "blocked-field-goal impact";case Kind::Ordinary:return "ordinary buildup";case Kind::Critical:return "critical buildup";case Kind::Touchdown:return "touchdown";case Kind::Eruption:return "eruption";case Kind::LivePlay:return "live-play reaction";case Kind::Safety:return "safety";default:return "inactive";}}
struct Settings {float ordinary=5,critical=10,touchdown=16,eruption=22.5f,live=11,catchImpact=12;float touchdownImpact=20,eruptionImpact=30;unsigned touchdownMs=11000,eruptionMs=16000;float lineupCheer=6;unsigned reactionFadeMs=5000,impactExtraMs=1000;float hostileOpeningMaximum=10,cannon=18;float awayPercent=0,neutralPercent=75;bool widePlayerCompensation=false;std::array<float,5> cameraPercent{{100,100,100,100,100}};};
inline float percentageScale(float value){return std::isfinite(value)?(std::clamp)(value,0.f,100.f)*.01f:0;}
inline float awayScale(const Settings& s){return percentageScale(s.awayPercent);}
inline float neutralScale(const Settings& s){return percentageScale(s.neutralPercent);}
inline float hostileOpeningStrength(uint64_t positions,float maximum=10){
    if(positions<512||positions>12800000||!std::isfinite(maximum))return 0;
    return (std::clamp)(maximum,0.f,30.f)*venueScale(positions);
}
inline float openingKickoffStrength(float maximum,float importance){
    return maximum*(.6f+.2f*(std::clamp)(importance,0.f,1.f))*.75f;
}
inline float limit(float x){return std::isfinite(x)?(std::clamp)(x,0.0f,100.0f):0;}
inline float configured(const Settings& s,Kind k){switch(k){case Kind::Ordinary:return limit(s.ordinary);case Kind::Critical:return limit(s.critical);case Kind::Touchdown:case Kind::Safety:case Kind::BlockedKick:case Kind::Clincher:return limit(s.touchdown);case Kind::Eruption:return limit(s.eruption);case Kind::LivePlay:return limit(s.live);default:return 0;}}
inline unsigned safetyDuration(const Settings& s){return (std::clamp)(s.touchdownMs,3000u,30000u)*7/10;}
inline float lineupStrength(const Settings& s,bool critical,float choice){
    float maximum=configured(s,critical?Kind::Critical:Kind::Ordinary),minimum=maximum*(critical?.75f:.5f);
    float fraction=std::isfinite(choice)?(std::clamp)(choice,0.0f,1.0f):0;
    return minimum+(maximum-minimum)*fraction;
}
struct Cue {
    uintptr_t match=0;unsigned epoch=0;uint64_t updated=0,began=0,until=0;
    Kind kind=Kind::None;float percent=0;bool tail=false;float impact=0;uint64_t oscillationBegan=0;bool impactOnly=false;
    float transitionFrom=0;uint64_t transitionAt=0;bool carryPlay=false;
    bool runBuildup=false;
    bool catchReaction=false;
    unsigned distanceExtraMs=0;
    float hostileOpening=0;
    bool confirmedRun=false;
    bool cancelled=false;
    float rampFloor=0;
    float reactionScale=1;bool softAccent=false;
    bool forcedImpact=false;
    bool scoreBuildup=false;
    Profile profile=Profile::Default;float leverage=0;float holdRatio=1,peakRatio=1;
    bool smoothRunBuildup=false;float buildupStrength=0;uint64_t buildupUpdated=0;
    float additiveScale=1;bool visitor=false;uint64_t soundAt=0,playSequence=0;
};
inline float smooth(float x){x=(std::clamp)(x,0.0f,1.0f);return x*x*(3-2*x);}
inline float envelopeStrength(const Cue& c,uint64_t now){
    if(c.kind==Kind::None||!c.match||now<c.updated||now-c.updated>750||now<c.began||!std::isfinite(c.percent))return 0;
    float age=static_cast<float>(now-c.began);
    if(c.impactOnly){if(now>=c.until||c.until<=c.began+400)return 0;return limit(c.percent)*(c.softAccent?shape::skillBuildup(age,static_cast<float>(c.until-c.began)):shape::impact(age,static_cast<float>(c.until-c.began),c.forcedImpact?35.f:100.f));}
    if(c.tail){unsigned total=c.until>c.began?static_cast<unsigned>((std::min)(c.until-c.began,uint64_t{16000})):0;return limit(c.percent)*shape::tail(age,static_cast<float>(total),c.confirmedRun?static_cast<float>(shape::tailHold(total)):0.f);}
    if(c.kind==Kind::Ordinary||c.kind==Kind::Critical||c.kind==Kind::LivePlay)return limit(c.percent)*smooth(age/(c.kind==Kind::LivePlay?200.0f:c.kind==Kind::Critical?600.0f:1000.0f));
    if(now>=c.until||c.until<=c.began||limit(c.percent)<=0)return 0;
    float duration=static_cast<float>(c.until-c.began),baseDuration=(std::max)(1.f,duration-c.distanceExtraMs),progress=age/baseDuration;
    bool td=c.kind==Kind::Touchdown||c.kind==Kind::Safety||c.kind==Kind::BlockedKick||c.kind==Kind::Clincher;
    float peakEnd=td?1.0f/9:1.25f/16,settleEnd=td?1.5f/9:2.0f/16,holdEnd=td?5.0f/9:8.0f/16;
    float peak=(std::max)(limit(c.percent),limit(c.impact));
    if(c.scoreBuildup&&progress<peakEnd)return peak; // The shared 300 ms handoff supplies the attack.
    if(progress<peakEnd)return peak*smooth(age/(td?80.0f:60.0f));
    if(progress<settleEnd)return peak+(limit(c.percent)-peak)*smooth((progress-peakEnd)/(settleEnd-peakEnd));
    if(progress<holdEnd)return limit(c.percent);
    return limit(c.percent)*smooth((duration-age)/(duration-baseDuration*holdEnd));
}
inline float strength(const Cue& c,uint64_t now){
    float level=envelopeStrength(c,now);
    unsigned transition=c.forcedImpact?50u:c.profile==Profile::Truck?100u:c.scoreBuildup?300u:200u;
    if(c.transitionAt&&now>=c.transitionAt&&now-c.transitionAt<transition){
        // A new attack must not pull an already moving camera toward zero
        // before rising. Lower targets still ease down normally.
        if((std::max)(c.percent,c.impact)>=c.transitionFrom)level=(std::max)(level,c.transitionFrom);
        return (c.transitionFrom+(level-c.transitionFrom)*smooth(static_cast<float>(now-c.transitionAt)/transition))*c.additiveScale;
    }
    return level*c.additiveScale;
}
struct Input {
    uintptr_t match=0;unsigned epoch=0;uint64_t now=0;int phase=0;
    bool allowed=false,lineup=false,critical=false,eruptionAlive=false;
    bool venueEligible=false,duringAllowed=true,openingKickoff=false;
    uint64_t lineupId=0,touchdown=0,eruption=0,duringPlay=0;
    float lineupScale=1;bool lineupCheer=false,duringImpact=false,duringCatch=false;
    float lineupStrengthScale=1;
    uint64_t runBuildup=0;float runProgress=0;
    uint64_t safety=0;
    unsigned distanceExtraMs=0;
    float hostileOpening=0;uint64_t blockedFieldGoal=0;
    bool cancelLive=false;
    float reactionScale=1;unsigned impactMilliseconds=0;bool supplementalImpact=false;uint64_t clincher=0;float touchdownScale=1;bool softAccent=false;
    bool forcedImpact=false;
    bool neutral=false;
    float venueScale=1;
    Profile profile=Profile::Default;float leverage=0;bool homeCritical=false,gameSavingBlock=false;
    Profile buildupProfile=Profile::Gain;
    bool overtime=false;
    bool smoothRunBuildup=false;
};
inline float liveStrength(const Settings& s,Profile p,float leverage,bool catchImpact=false){
    if(p==Profile::Default)return catchImpact?limit(s.catchImpact):limit(s.live);
    if(catchImpact)return limit(s.catchImpact)*profileStrength(Profile::Catch,leverage)/9.f;
    if(p==Profile::Catch)p=Profile::Gain;
    return limit(s.live)*profileStrength(p,leverage)/11.f;
}
inline unsigned impactDuration(const Settings& s,Profile p,unsigned overrideMs=0){
    if(overrideMs)return (std::clamp)(overrideMs,1000u,10000u);
    unsigned base=p==Profile::Default?(std::clamp)(s.reactionFadeMs,1000u,8000u)+400:profileDuration(p);
    return static_cast<unsigned>((std::clamp)(static_cast<int>(base)+static_cast<int>((std::min)(s.impactExtraMs,5000u))-(p==Profile::Default?0:1000),1000,10000));
}
class Model {
    uintptr_t match_=0;unsigned epoch_=0;uint64_t last_=0,lineup_=0,seenTd_=0,seenEruption_=0,seenLive_=0,ramp_=0,seenSafety_=0,seenBlockedKick_=0,seenClincher_=0;int phase_=0;
    Cue lineupCue_,pulse_,liveCue_,impactOverlay_,skillCue_,output_;
public:
    void reset(){*this=Model{};}
    Cue suspend(uint64_t now){lineup_=ramp_=0;lineupCue_=pulse_=liveCue_=impactOverlay_=skillCue_={};output_={};output_.updated=now;return output_;}
    Cue observe(const Settings& configuredSettings,const Input& x){
        // Scale each crowd target once by occupied seats, and by
        // the configured shared neutral-crowd percentage, before envelopes.
        Settings settings=configuredSettings;float size=std::isfinite(x.venueScale)?(std::clamp)(x.venueScale,0.f,1.25f):0;
        float crowdScale=(x.neutral?neutralScale(settings):1.f)*size*(x.overtime?1.10f:1.f);
        settings.ordinary=limit(settings.ordinary)*crowdScale;settings.critical=limit(settings.critical)*crowdScale;
        settings.touchdown=limit(settings.touchdown)*crowdScale;settings.eruption=limit(settings.eruption)*crowdScale;
        settings.live=limit(settings.live)*crowdScale;settings.catchImpact=limit(settings.catchImpact)*crowdScale;
        settings.touchdownImpact=limit(settings.touchdownImpact)*crowdScale;settings.eruptionImpact=limit(settings.eruptionImpact)*crowdScale;
        settings.lineupCheer=limit(settings.lineupCheer)*crowdScale;
        unsigned reactionFade=(std::clamp)(settings.reactionFadeMs,1000u,8000u);
        if(x.match!=match_||x.epoch!=epoch_){reset();match_=x.match;epoch_=x.epoch;}
        bool td=x.touchdown&&x.touchdown!=seenTd_,eruption=x.eruption&&x.eruption!=seenEruption_,safety=x.safety&&x.safety!=seenSafety_;
        bool blocked=x.blockedFieldGoal&&x.blockedFieldGoal!=seenBlockedKick_;if(x.blockedFieldGoal)seenBlockedKick_=x.blockedFieldGoal;
        bool clincher=x.clincher&&x.clincher!=seenClincher_;if(x.clincher)seenClincher_=x.clincher;
        bool live=x.duringPlay&&x.duringPlay!=seenLive_;if(x.duringPlay)seenLive_=x.duringPlay;
        if(x.touchdown)seenTd_=x.touchdown;if(x.eruption)seenEruption_=x.eruption;if(x.safety)seenSafety_=x.safety;
        if(!x.allowed||!x.match||x.now<last_||(x.neutral&&neutralScale(settings)<=0)){last_=x.now;return suspend(x.now);}last_=x.now;
        bool newSnap=x.phase==3&&phase_!=3;phase_=x.phase;
        Cue previous=output_;previous.additiveScale=1;float previousLevel=strength(previous,x.now);
        auto join=[&](Cue& cue){if(previousLevel>0){cue.oscillationBegan=previous.oscillationBegan?previous.oscillationBegan:previous.began;cue.transitionFrom=previousLevel;cue.transitionAt=x.now;
            bool scoring=cue.kind==Kind::Touchdown||cue.kind==Kind::Eruption;
            bool earned=previous.kind==Kind::LivePlay||previous.kind==Kind::Touchdown||previous.kind==Kind::Eruption||previous.kind==Kind::Safety;
            cue.scoreBuildup=scoring&&earned;
        }};
        if(x.lineup&&x.venueEligible&&x.lineupId&&(x.phase==1||x.phase==5)){
            if(lineup_!=x.lineupId||lineupCue_.tail||lineupCue_.kind==Kind::None){lineupCue_={x.match,x.epoch,x.now,x.now,0,Kind::Ordinary,0,false};lineup_=x.lineupId;lineupCue_.oscillationBegan=x.now;join(lineupCue_);}
            // The opening kick is gentler than the first hostile offensive call:
            // 6-8 versus 8-10 at the default shared opening ceiling of 10.
            float prior=strength(lineupCue_,x.now),target=x.hostileOpening>0?(x.openingKickoff?openingKickoffStrength(limit(x.hostileOpening),importance(x.lineupScale)):limit(x.hostileOpening)*(.8f+.2f*importance(x.lineupScale)))*crowdScale:x.lineupCheer&&!x.critical?limit(settings.lineupCheer)*(2.f/3.f+importance(x.lineupScale)/3.f):lineupStrength(configuredSettings,x.critical,x.lineupScale)*crowdScale;
            if(x.critical&&x.homeCritical)target*=.75f; // Home 5.625-7.5; defense 7.5-10 at stock settings.
            target*=std::isfinite(x.lineupStrengthScale)?(std::clamp)(x.lineupStrengthScale,0.f,1.f):0.f;
            if(std::fabs(target-lineupCue_.percent)>.01f){lineupCue_.transitionFrom=prior;lineupCue_.transitionAt=x.now;}
            lineupCue_.kind=x.critical?Kind::Critical:Kind::Ordinary;
            lineupCue_.percent=target;lineupCue_.hostileOpening=x.hostileOpening;lineupCue_.updated=x.now;
            lineupCue_.carryPlay=x.openingKickoff;
        }else if(lineupCue_.kind!=Kind::None){
            bool carry=lineupCue_.carryPlay&&x.phase==3&&x.now>=lineupCue_.began&&x.now-lineupCue_.began<45000;
            if(!lineupCue_.tail&&!carry){float value=strength(lineupCue_,x.now);lineupCue_.began=x.now;lineupCue_.until=x.now+(lineupCue_.carryPlay?2000:900);lineupCue_.percent=value;lineupCue_.tail=true;lineup_=0;}
            lineupCue_.updated=x.now;if(lineupCue_.tail&&x.now>=lineupCue_.until)lineupCue_={};
        }
        // A presentation cut or play-selection inset is not a new play.
        // Retain the earned impact/tail until its original expiry; a fresh
        // snap, pause/replay suspension or world change cancels the old cue.
        if(newSnap){liveCue_=impactOverlay_=skillCue_={};ramp_=0;}
        bool pulseOwns=(pulse_.kind!=Kind::None&&x.now<pulse_.until&&!newSnap&&(pulse_.kind!=Kind::Eruption||x.eruptionAlive))||
            (x.phase!=3&&(td||eruption||safety||clincher||blocked));
        // Consume lower-priority event IDs without queuing a hidden shake
        // that could reappear after a short celebration has finished.
        if(pulseOwns){liveCue_=impactOverlay_=skillCue_={};ramp_=0;}
        if(x.cancelLive||(!x.duringAllowed&&x.phase!=2))skillCue_={};
        if(x.cancelLive)impactOverlay_={};
        if(x.cancelLive&&liveCue_.kind!=Kind::None&&!liveCue_.cancelled){
            float value=strength(liveCue_,x.now);liveCue_.began=x.now;liveCue_.until=x.now+500;liveCue_.percent=value;
            liveCue_.tail=liveCue_.cancelled=true;liveCue_.impactOnly=liveCue_.confirmedRun=false;ramp_=0;
        }
        if(!x.duringAllowed&&liveCue_.kind!=Kind::None&&!liveCue_.tail&&!liveCue_.impactOnly){
            float value=strength(liveCue_,x.now);liveCue_.began=x.now;liveCue_.until=x.now+reactionFade;liveCue_.percent=value;liveCue_.tail=true;liveCue_.confirmedRun=false;
        }
        bool ramp=!pulseOwns&&!x.cancelLive&&x.runBuildup&&x.duringAllowed&&x.phase==3&&std::isfinite(x.runProgress)&&x.runProgress>0;
        if(ramp&&!liveCue_.runBuildup&&liveCue_.kind!=Kind::None&&x.now<liveCue_.until&&
           liveStrength(settings,x.buildupProfile,x.leverage)<=liveCue_.percent)ramp=false;
        if(ramp&&(liveCue_.kind==Kind::None||ramp_!=x.runBuildup)){
            float inherited=liveCue_.kind==Kind::LivePlay?previousLevel:0;
            liveCue_={x.match,x.epoch,x.now,x.now,x.now+45000,Kind::LivePlay,0,false,0,x.now,false};
            liveCue_.profile=x.buildupProfile;liveCue_.leverage=importance(x.leverage);
            liveCue_.rampFloor=(std::min)(inherited,liveStrength(settings,liveCue_.profile,liveCue_.leverage));
            liveCue_.runBuildup=true;ramp_=x.runBuildup;join(liveCue_);
            liveCue_.smoothRunBuildup=x.smoothRunBuildup;
            liveCue_.buildupStrength=(std::max)(liveCue_.rampFloor,(std::min)(crowdScale,liveStrength(settings,liveCue_.profile,liveCue_.leverage)));
            liveCue_.buildupUpdated=x.now;
        }
        // Crossing the configured gain confirms the cheer without restarting
        // the shake that builds over the preceding five yards.
        float candidate=liveStrength(settings,x.profile,x.leverage,x.duringCatch&&(x.phase!=3||x.duringImpact))*(std::isfinite(x.reactionScale)?(std::clamp)(x.reactionScale,0.f,1.5f):1);
        // Skill success has its own short envelope. It must not be discarded
        // by a weak distance ramp, or replace/restart that ramp. The stronger
        // envelope leads below, with a bounded independent overlap accent.
        if(live&&x.supplementalImpact&&!pulseOwns&&!x.cancelLive&&x.duringAllowed&&(x.phase==3||x.phase==4||x.phase==5||x.phase==1)&&
           (skillCue_.kind==Kind::None||x.now>=skillCue_.until||candidate>skillCue_.percent)){
            skillCue_={x.match,x.epoch,x.now,x.now,x.now+impactDuration(settings,x.profile,x.impactMilliseconds),Kind::LivePlay,candidate,false,0,x.now,true};
            skillCue_.profile=x.profile;skillCue_.leverage=importance(x.leverage);
            skillCue_.reactionScale=std::isfinite(x.reactionScale)?(std::clamp)(x.reactionScale,0.f,1.5f):1;
            skillCue_.softAccent=x.softAccent;join(skillCue_);
        }
        bool continuous=x.phase==3&&!x.duringImpact;
        bool underImpact=continuous&&liveCue_.impactOnly&&liveCue_.kind!=Kind::None;
        bool covered=liveCue_.kind!=Kind::None&&strength(liveCue_,x.now)>0&&(x.supplementalImpact||(!underImpact&&candidate<strength(liveCue_,x.now)));
        if(live&&!x.supplementalImpact&&!pulseOwns&&!covered&&!x.cancelLive&&x.duringAllowed&&!(ramp&&liveCue_.kind==Kind::LivePlay&&!liveCue_.tail&&!x.duringImpact)&&(x.phase==3||x.phase==4||((x.phase==5||x.phase==1)&&x.duringImpact))){
            bool impact=x.phase!=3||x.duringImpact;
            unsigned duration=impactDuration(settings,x.profile,x.impactMilliseconds);
            // A force can still be hitting when its defensive return begins.
            // Retain that original impact above the continuous return, then
            // select the leading envelope as it settles; clocks stay separate.
            if(underImpact&&strength(liveCue_,x.now)>strength(impactOverlay_,x.now))impactOverlay_=liveCue_;
            liveCue_={x.match,x.epoch,x.now,x.now,x.now+(impact?duration:45000),Kind::LivePlay,impact&&x.duringCatch?limit(settings.catchImpact):configured(settings,Kind::LivePlay),false,0,x.now,impact};
            liveCue_.reactionScale=std::isfinite(x.reactionScale)?(std::clamp)(x.reactionScale,0.f,1.5f):1;
            liveCue_.softAccent=x.softAccent;
            liveCue_.forcedImpact=x.forcedImpact;
            liveCue_.profile=x.profile;liveCue_.leverage=importance(x.leverage);ramp_=0;
            liveCue_.catchReaction=x.duringCatch;join(liveCue_);
        }
        if(live&&!x.supplementalImpact&&x.duringAllowed&&liveCue_.kind==Kind::LivePlay){liveCue_.confirmedRun=true;
            if(!covered&&ramp&&!x.duringImpact){liveCue_.profile=x.profile==Profile::Default?Profile::Gain:x.profile;liveCue_.leverage=importance(x.leverage);}
        }
        if(liveCue_.kind!=Kind::None){
            if(!liveCue_.tail&&!liveCue_.impactOnly)liveCue_.distanceExtraMs=(std::max)(liveCue_.distanceExtraMs,(std::min)(x.distanceExtraMs,8000u));
            if(liveCue_.catchReaction&&!liveCue_.impactOnly&&x.phase!=3&&x.now>=liveCue_.began&&x.now-liveCue_.began<=350){
                liveCue_.impactOnly=true;liveCue_.profile=Profile::Catch;liveCue_.percent=liveStrength(settings,liveCue_.profile,liveCue_.leverage,true);liveCue_.until=liveCue_.began+impactDuration(settings,Profile::Catch);join(liveCue_);
            }
            if(liveCue_.runBuildup&&!liveCue_.confirmedRun&&!ramp&&x.phase==3&&!liveCue_.tail){float value=strength(liveCue_,x.now);liveCue_.began=x.now;liveCue_.until=x.now+reactionFade+liveCue_.distanceExtraMs;liveCue_.percent=value;liveCue_.tail=true;}
            if(!liveCue_.impactOnly&&x.phase!=3&&!liveCue_.tail){float value=liveCue_.smoothRunBuildup?strength(liveCue_,x.now):(std::max)(strength(liveCue_,x.now),liveCue_.percent*.7f);liveCue_.began=x.now;liveCue_.until=x.now+reactionFade+liveCue_.distanceExtraMs;liveCue_.percent=value;liveCue_.tail=true;}
            liveCue_.updated=x.now;
            if(!liveCue_.tail){
                float target=liveStrength(settings,liveCue_.profile,liveCue_.leverage,liveCue_.impactOnly&&liveCue_.catchReaction);
                bool building=ramp&&liveCue_.runBuildup&&!liveCue_.impactOnly;
                if(liveCue_.smoothRunBuildup){
                    // Distance supplies the target; a late first sample must
                    // still rise from the reached level, never jump to it.
                    float progress=building?importance(x.runProgress):1.f;
                    float desired=(std::min)(target,(std::max)(liveCue_.rampFloor,crowdScale+(target-crowdScale)*std::pow(progress,1.2f)));
                    auto elapsed=x.now>=liveCue_.buildupUpdated?(std::min)(x.now-liveCue_.buildupUpdated,uint64_t{100}):0;
                    float rise=(std::max)(0.f,target-crowdScale)*static_cast<float>(elapsed)/800.f;
                    liveCue_.buildupStrength=(std::min)(desired,liveCue_.buildupStrength+rise);
                    liveCue_.buildupUpdated=x.now;liveCue_.percent=liveCue_.buildupStrength*liveCue_.reactionScale;
                }else liveCue_.percent=(building?(std::max)(liveCue_.rampFloor,(std::min)(target,crowdScale+(target-crowdScale)*importance((x.runProgress-.0001f)/.9999f))):target)*liveCue_.reactionScale;
            }
            if(x.now>=liveCue_.until)liveCue_={};
        }
        if(pulse_.kind!=Kind::None&&(x.now>=pulse_.until||newSnap||(pulse_.kind==Kind::Eruption&&!x.eruptionAlive)))pulse_={};
        if(eruption&&x.phase!=3){
            bool upgrade=(pulse_.kind==Kind::Touchdown||pulse_.kind==Kind::Safety)&&x.now>=pulse_.began&&x.now-pulse_.began<750;
            auto start=upgrade?pulse_.began:x.now;pulse_={x.match,x.epoch,x.now,start,start+(std::clamp)(settings.eruptionMs,5000u,60000u),Kind::Eruption,configured(settings,Kind::Eruption),false,limit(settings.eruptionImpact)};
            pulse_.distanceExtraMs=(std::min)(x.distanceExtraMs,8000u);pulse_.until+=pulse_.distanceExtraMs;
            join(pulse_);liveCue_=impactOverlay_={};ramp_=0;
        }else if(td&&x.phase!=3&&pulse_.kind!=Kind::Eruption){pulse_={x.match,x.epoch,x.now,x.now,x.now+(std::clamp)(settings.touchdownMs,3000u,30000u),Kind::Touchdown,configured(settings,Kind::Touchdown),false,limit(settings.touchdownImpact)};pulse_.distanceExtraMs=(std::min)(x.distanceExtraMs,8000u);pulse_.until+=pulse_.distanceExtraMs;join(pulse_);liveCue_={};ramp_=0;}
        else if(clincher&&x.phase!=3&&pulse_.kind!=Kind::Eruption&&pulse_.kind!=Kind::Touchdown){pulse_={x.match,x.epoch,x.now,x.now,x.now+safetyDuration(settings),Kind::Clincher,configured(settings,Kind::Clincher),false,limit(settings.touchdownImpact)};pulse_.holdRatio=.75f;pulse_.peakRatio=.8f;join(pulse_);liveCue_={};ramp_=0;}
        else if(blocked&&x.phase!=3&&pulse_.kind!=Kind::Eruption&&pulse_.kind!=Kind::Touchdown){pulse_={x.match,x.epoch,x.now,x.now,x.now+safetyDuration(settings),Kind::BlockedKick,configured(settings,Kind::BlockedKick),false,limit(settings.touchdownImpact)};pulse_.holdRatio=x.gameSavingBlock?1.f:.625f;pulse_.peakRatio=x.gameSavingBlock?1.f:.7f;join(pulse_);liveCue_={};ramp_=0;}
        else if(safety&&x.phase!=3&&pulse_.kind!=Kind::Eruption&&pulse_.kind!=Kind::Touchdown){pulse_={x.match,x.epoch,x.now,x.now,x.now+safetyDuration(settings),Kind::Safety,configured(settings,Kind::Safety),false,limit(settings.touchdownImpact)};pulse_.holdRatio=.75f;pulse_.peakRatio=.8f;join(pulse_);liveCue_={};ramp_=0;}
        if(pulse_.kind!=Kind::None){pulse_.updated=x.now;pulse_.percent=limit(configured(settings,pulse_.kind)*pulse_.holdRatio);pulse_.impact=limit((pulse_.kind==Kind::Eruption?settings.eruptionImpact:settings.touchdownImpact)*pulse_.peakRatio);output_=pulse_;}
        else if(liveCue_.kind!=Kind::None)output_=liveCue_;else{
            if(lineupCue_.kind!=Kind::None&&previous.kind!=Kind::None&&previous.kind!=Kind::Ordinary&&previous.kind!=Kind::Critical){lineupCue_.transitionFrom=previousLevel;lineupCue_.transitionAt=x.now;}
            output_=lineupCue_;
        }
        if(impactOverlay_.kind!=Kind::None){
            if(x.now>=impactOverlay_.until||pulse_.kind!=Kind::None)impactOverlay_={};
            else{impactOverlay_.updated=x.now;if(strength(impactOverlay_,x.now)>strength(output_,x.now))output_=impactOverlay_;}
        }
        if(skillCue_.kind!=Kind::None){
            if(x.now>=skillCue_.until||pulse_.kind!=Kind::None)skillCue_={};
            else{
                skillCue_.updated=x.now;skillCue_.percent=liveStrength(settings,skillCue_.profile,skillCue_.leverage)*skillCue_.reactionScale;
                if(output_.kind==Kind::None||strength(skillCue_,x.now)>strength(output_,x.now))output_=skillCue_;
            }
        }
        // Independent live skill/gain/impact and opening-kickoff envelopes may
        // reinforce each other gently. Stored cues remain unscaled: repeated
        // ticks and live/result reports cannot compound the bonus or its clock.
        if(pulse_.kind==Kind::None){
            const Cue* cues[]={&lineupCue_,&liveCue_,&impactOverlay_,&skillCue_};
            // Keep the admitted reaction's identity on an equal-strength
            // handoff; a fading lineup must not take its place on that frame.
            const Cue* strongest=nullptr;float first=0;
            for(auto cue:cues){float value=strength(*cue,x.now);
                bool selected=cue->kind==output_.kind&&cue->began==output_.began&&cue->profile==output_.profile;
                if(value>first||(value==first&&selected)){first=value;strongest=cue;}}
            if(strongest&&first>0){
                float secondary=0;
                for(auto cue:cues)if(cue!=strongest)secondary+=(std::min)(strength(*cue,x.now),envelopeStrength(*cue,x.now));
                // A new cue borrows the old amplitude to join smoothly. That
                // borrowed level is not independent energy to count twice.
                float attack=(std::clamp)(envelopeStrength(*strongest,x.now)/first,0.f,1.f);
                output_=*strongest;output_.additiveScale=1+.15f*(std::min)(secondary/first,1.f)*attack;
            }
        }
        return output_;
    }
};
inline bool validMatrix(const std::array<float,16>& m){
    for(float v:m)if(!std::isfinite(v)||std::fabs(v)>100000)return false;
    for(int row=0;row<3;++row){float norm=0;for(int col=0;col<3;++col)norm+=m[row*4+col]*m[row*4+col];if(std::fabs(norm-1)>.02f||std::fabs(m[row*4+3])>.01f)return false;}
    for(int a=0;a<3;++a)for(int b=a+1;b<3;++b){float dot=0;for(int i=0;i<3;++i)dot+=m[a*4+i]*m[b*4+i];if(std::fabs(dot)>.02f)return false;}
    return std::fabs(m[15]-1)<.01f;
}
inline void rotate(std::array<float,16>& m,int a,int b,float radians){
    float c=std::cos(radians),s=std::sin(radians);for(int row=0;row<4;++row){float x=m[row*4+a],y=m[row*4+b];m[row*4+a]=c*x+s*y;m[row*4+b]=-s*x+c*y;}
}
inline float projectionScale(const std::array<float,16>& p,Kind kind,bool widePlayerCompensation=false){
    if(kind==Kind::None)return 1;
    auto s=projection::scale(p,widePlayerCompensation);return s.valid?s.pitch:0;
}
inline bool apply(std::array<float,16>& output,const Cue& c,uint64_t now,float pitchScale=1,float yawScale=1,float preference=1){
    float amount=strength(c,now)*.01f*preference;
    if(!std::isfinite(amount)||amount<=0||!std::isfinite(pitchScale)||pitchScale<=0||pitchScale>6||!std::isfinite(yawScale)||yawScale<=0||yawScale>6||!validMatrix(output))return false;
    constexpr float pi=3.14159265358979323846f;
    float t=static_cast<float>((now-(c.oscillationBegan?c.oscillationBegan:c.began))%1000000)*.001f;auto result=output;
    // Quadrature keeps the two main axes moving continuously. Independent
    // 7/9 Hz oscillators periodically nearly cancelled, making a steady
    // touchdown envelope feel like repeated short bursts on close-up views.
    rotate(result,1,2,.65f*(pi/180)*std::sin(2*pi*8*t)*amount*pitchScale);
    rotate(result,0,2,.45f*(pi/180)*std::cos(2*pi*8*t)*amount*yawScale);
    rotate(result,0,1,.12f*(pi/180)*std::sin(2*pi*11.3f*t)*amount);
    if(!validMatrix(result))return false;output=result;return true;
}
}
