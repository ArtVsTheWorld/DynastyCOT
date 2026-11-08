#pragma once
#include "CoreAtmosphere.h"
#include "DirectorShake.h"
#include <sstream>

namespace attendance::diagnostic {
enum class Preset {Stop,Roar,Swell,Cheer,Conversion,Defense,Disappointment,Lineup,Critical,Touchdown,Eruption};
inline const char* name(Preset p){switch(p){case Preset::Roar:return "roar";case Preset::Swell:return "swell";case Preset::Cheer:return "cheer";case Preset::Conversion:return "conversion";case Preset::Defense:return "defense";case Preset::Disappointment:return "disappointment";case Preset::Lineup:return "lineup";case Preset::Critical:return "critical";case Preset::Touchdown:return "touchdown";case Preset::Eruption:return "eruption";default:return "stop";}}
inline bool primary(Preset p){return p==Preset::Roar||p==Preset::Lineup||p==Preset::Critical||p==Preset::Touchdown||p==Preset::Eruption;}
inline bool support(Preset p){return p==Preset::Swell||p==Preset::Lineup||p==Preset::Critical||p==Preset::Touchdown||p==Preset::Eruption;}
inline bool cheer(Preset p){return p==Preset::Cheer||p==Preset::Conversion||p==Preset::Defense||p==Preset::Disappointment||p==Preset::Touchdown||p==Preset::Eruption;}
struct Request {unsigned pid=0,seconds=20;uint64_t utc=0,token=0;Preset preset=Preset::Stop;bool sound=true,camera=false;uint64_t deadline=0;};
inline bool parse(const std::string& text,unsigned pid,uint64_t utc,Request& out){
    if(text.size()>256)return false;
    std::istringstream in(text);std::string magic,preset,extra;unsigned sound=0,camera=0;
    Request r;
    if(!(in>>magic>>r.pid>>r.utc>>std::hex>>r.token>>std::dec>>preset>>r.seconds>>sound>>camera)||in>>extra||magic!="DGA-DIAGNOSTIC-1"||r.pid!=pid||!r.token||r.seconds<5||r.seconds>60||sound>1||camera>1)return false;
    if(r.utc>utc?(r.utc-utc>2000):(utc-r.utc>15000))return false;
    bool found=false;for(auto p:{Preset::Stop,Preset::Roar,Preset::Swell,Preset::Cheer,Preset::Conversion,Preset::Defense,Preset::Disappointment,Preset::Lineup,Preset::Critical,Preset::Touchdown,Preset::Eruption})if(preset==name(p)){r.preset=p;found=true;break;}
    if(!found||(r.preset!=Preset::Stop&&!sound&&!camera))return false;r.sound=sound!=0;r.camera=camera!=0;out=r;return true;
}
struct Context {uintptr_t match=0;unsigned epoch=0;uint64_t now=0;bool allowed=false;int phase=0;float fullness=0;const char* unavailable=nullptr;};
struct Preview {
    uintptr_t match=0;unsigned epoch=0;uint64_t token=0,updated=0,until=0;bool active=false,cooling=false,sound=false,camera=false,primary=true,supporting=false;
    float boost=0;sustained::Intent intent;motion::Cue cue;Preset preset=Preset::Stop;
    bool fresh(uintptr_t owner,unsigned generation,uint64_t now)const{return match&&match==owner&&epoch==generation&&now>=updated&&now-updated<=750;}
    bool playing(uintptr_t owner,unsigned generation,uint64_t now)const{return active&&fresh(owner,generation,now)&&now<until;}
};
class Model {
    Request request_;uintptr_t match_=0;unsigned epoch_=0;uint64_t seen_=0,began_=0,until_=0,coolUntil_=0;bool running_=false;std::string status_="idle";
public:
    const std::string& status()const{return status_;}
    void stop(uint64_t now,const char* reason){if(running_)coolUntil_=now+4000;running_=false;status_=reason;}
    Preview update(const Request& incoming,const Context& c,const core::Settings& sound,const motion::Settings& camera){
        if(incoming.token&&incoming.token!=seen_){
            seen_=incoming.token;
            if(incoming.preset==Preset::Stop)stop(c.now,"stopped by user");
            else if(incoming.deadline&&c.now>incoming.deadline){stop(c.now,"not started: request expired; try again while lined up");}
            else if(c.unavailable){stop(c.now,c.unavailable);}
            else if(!c.allowed||!c.match||c.phase!=1||!std::isfinite(c.fullness)||c.fullness<0||c.fullness>1){stop(c.now,"not started: unpause and line up without snapping");}
            else if(!sound.enabled||core::normalLimit(sound)<=0||(incoming.camera&&!sound.cameraShake)){stop(c.now,"not started: requested soundscape/camera control is disabled");}
            else if(running_||c.now<coolUntil_){status_="not started: wait for the current preview/fade to finish";}
            else{request_=incoming;match_=c.match;epoch_=c.epoch;began_=c.now;until_=c.now+1000ull*incoming.seconds;running_=true;status_=std::string("running: ")+name(request_.preset);}
        }
        if(running_&&(!c.allowed||c.match!=match_||c.epoch!=epoch_||c.phase!=1||c.now<began_))stop(c.now,"cancelled: snap, pause, replay or game transition");
        if(running_&&(!sound.enabled||core::normalLimit(sound)<=0||(request_.camera&&!sound.cameraShake)))stop(c.now,"cancelled: settings disabled the requested effects");
        if(running_&&c.now>=until_)stop(c.now,"finished; restoring normal atmosphere");
        Preview out;out.match=match_;out.epoch=epoch_;out.token=request_.token;out.updated=c.now;out.until=until_;out.preset=request_.preset;
        out.active=running_;out.cooling=!running_&&c.now<coolUntil_;out.sound=request_.sound;out.camera=request_.camera;
        if(!running_)return out;
        uint64_t token=request_.token|0x8000000000000000ull;auto& wanted=out.intent;
        wanted.match=match_;wanted.epoch=epoch_;wanted.updated=c.now;wanted.active=true;wanted.reason=sustained::Reason::Diagnostics;wanted.priority=10;
        wanted.diagnosticToken=token;wanted.lineup=token;wanted.size=3;wanted.continuous=true;wanted.cheers=false;
        wanted.attackMs=3000;wanted.releaseMs=4000;wanted.intensity=1000*std::sqrt(c.fullness);
        out.primary=primary(request_.preset);out.supporting=support(request_.preset);
        bool single=request_.preset==Preset::Cheer||request_.preset==Preset::Conversion||request_.preset==Preset::Defense||request_.preset==Preset::Disappointment;
        if(single){out.primary=out.supporting=false;wanted.liveCheer=token;wanted.liveCheerUntil=began_+2500;wanted.liveType=request_.preset==Preset::Defense?59:request_.preset==Preset::Disappointment?95:83;
            if(request_.preset==Preset::Disappointment)wanted.intensity=0;
            if(request_.preset==Preset::Cheer){wanted.liveCheer=0;wanted.resultCheer=token;wanted.resultCheerUntil=began_+2500;wanted.resultType=91;}}
        if(request_.preset==Preset::Touchdown||request_.preset==Preset::Eruption){
            wanted.attackMs=500;wanted.releaseMs=request_.preset==Preset::Eruption?20000:8000;
            wanted.resultCheer=token;wanted.resultCheerUntil=began_+2500;wanted.resultType=91;wanted.deepCheers=true;
            if(request_.preset==Preset::Eruption){wanted.celebration=token;wanted.celebrationUntil=began_+6000;out.boost=volume::boostPercent(core::eruptionLimit(sound));}
        }
        if(request_.preset==Preset::Critical)out.boost=volume::boostPercent(core::highLeverageLimit(sound));
        // Reuse production motion envelopes and configured strengths. No game
        // score, clock, attendance, camera angle or gameplay HFA is modified.
        auto kind=request_.preset==Preset::Eruption?motion::Kind::Eruption:request_.preset==Preset::Touchdown?motion::Kind::Touchdown:request_.preset==Preset::Critical?motion::Kind::Critical:motion::Kind::Ordinary;
        out.cue={match_,epoch_,c.now,began_,until_,kind,motion::configured(camera,kind),false};
        out.cue.oscillationBegan=began_;
        if(kind==motion::Kind::Touchdown||kind==motion::Kind::Eruption){out.cue.until=(std::min)(until_,began_+(kind==motion::Kind::Touchdown?camera.touchdownMs:camera.eruptionMs));out.cue.impact=kind==motion::Kind::Touchdown?camera.touchdownImpact:camera.eruptionImpact;}
        unsigned fade=(std::min)(request_.preset==Preset::Eruption?20000u:request_.preset==Preset::Touchdown?8000u:4000u,request_.seconds*500u);
        uint64_t fading=until_-fade;wanted.releaseMs=static_cast<int>(fade);
        if(c.now>=fading){float weight=1-shape::smooth(static_cast<float>(c.now-fading)/fade);wanted.reaction=true;wanted.continuous=false;wanted.intensity*=weight;out.boost*=weight;}
        if(kind==motion::Kind::Ordinary||kind==motion::Kind::Critical){if(c.now>=fading){out.cue.tail=true;out.cue.began=fading;}}
        return out;
    }
};
}
