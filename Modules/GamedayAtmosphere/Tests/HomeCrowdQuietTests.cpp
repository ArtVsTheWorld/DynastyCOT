#include "../Source/HomeCrowdQuiet.h"
#include "../Source/AttendanceAudio.h"
#include "../Source/AwayCrowdAudio.h"
#include <cassert>
#include <cstdio>
#include <chrono>
using namespace attendance;
unsigned checks=0;
void checked(bool value,const char* expression,int line){++checks;if(!value)std::fprintf(stderr,"Check %u failed at line %d: %s\n",checks,line,expression);assert(value);}
#define check(value) checked((value),#value,__LINE__)
struct Fixture {
    homequiet::Model model;energy::Play p;energy::Outcome result;uint64_t now=1000,seq=1;
    Fixture(){p.game={2,28,7,1,400,720};p.down=1;p.offense=1;p.selectedType=103;p.toGoal=40;tick();}
    homequiet::Envelope tick(unsigned ms=50,bool allowed=true,uintptr_t match=42,unsigned epoch=1,float share=.06f){now+=ms;return model.observe(p,result,match,epoch,seq,now,allowed,share);}
    homequiet::Envelope td(){p.game.awayScore+=6;p.game.seconds-=6;p.game.phase=4;result.accepted=result.completed=true;result.sequence=seq;result.side=1;result.awayPoints=6;return tick();}
    homequiet::Envelope fade(){homequiet::Envelope out;result={};for(int i=0;i<80;++i)out=tick();return out;}
};
int main(){
    State s{2,7,7,1,400,720};float base=background::leadEngagement(s),last=base;
    for(int lead=0;lead<=70;++lead){s.homeScore=7+lead;float g=background::leadEngagement(s);check(g<=last&&g>=.6499f);if(lead<=17)check(g==1);last=g;}
    check(std::abs(last-.65f)<.00001f);s.neutral=true;check(background::leadEngagement(s)==1);
    sustained::Intent bed;bed.active=true;bed.match=42;bed.epoch=1;bed.reason=sustained::Reason::None;
    s.neutral=false;s.homeScore=49;auto reduced=background::merge(bed,s,61888,1);s.homeScore=7;auto tied=background::merge(bed,s,61888,1);
    check(reduced.intensity<tied.intensity&&std::abs(reduced.intensity/tied.intensity-.65f)<.00001f);
    auto earned=bed;earned.reason=sustained::Reason::Result;earned.intensity=900;check(background::merge(earned,s,61888,1).intensity==900);
    Fixture f;auto initial=f.td();check(initial.newTouchdown&&initial.quiet==0);f.result={};
    homequiet::Envelope envelope;
    for(int i=0;i<29;++i){envelope=f.tick();check(envelope.quiet==0&&!envelope.newTouchdown);}
    float prior=0;
    for(int i=0;i<51;++i){envelope=f.tick();check(envelope.quiet>=prior&&envelope.quiet<=1);prior=envelope.quiet;}
    check(envelope.quiet==1&&envelope.postGain()>.29f&&envelope.postGain()<.30f);
    auto post=background::merge(bed,f.p.game,61888,1);float full=post.intensity;
    post=homequiet::restrain(post,envelope,f.now);check(std::abs(post.intensity-full*envelope.postGain())<.001f);
    for(auto reason:{sustained::Reason::Result,sustained::Reason::Eruption,sustained::Reason::Postgame,sustained::Reason::Diagnostics,sustained::Reason::PassFlight}){
        auto reaction=post;reaction.reason=reason;reaction.intensity=900;reaction.liveCheer=11;reaction.resultCheer=12;reaction.extraCheer=13;reaction.opponentCheer=14;reaction.celebration=15;reaction.shake=true;
        auto out=homequiet::restrain(reaction,envelope,f.now);check(out.intensity==900&&out.liveCheer==11&&out.resultCheer==12&&out.extraCheer==13&&out.opponentCheer==14&&out.celebration==15&&out.shake);
    }
    earned.updated=f.now;check(homequiet::restrain(earned,envelope,f.now).intensity==900);
    check(homequiet::restrain(post,envelope,f.now+751).intensity==post.intensity);
    motion::Cue cue;cue.match=42;cue.epoch=1;cue.kind=motion::Kind::Ordinary;
    check(!homequiet::allowHomeCompanion(envelope,cue,f.now));cue.kind=motion::Kind::Critical;check(!homequiet::allowHomeCompanion(envelope,cue,f.now));
    cue.kind=motion::Kind::Touchdown;check(homequiet::allowHomeCompanion(envelope,cue,f.now));cue.kind=motion::Kind::Ordinary;cue.visitor=true;check(homequiet::allowHomeCompanion(envelope,cue,f.now));
    // Pause/replay interruption does not consume the fade or erase the hold.
    Fixture paused;paused.td();for(int i=0;i<40;++i)envelope=paused.tick();float beforePause=envelope.quiet;
    paused.model.suspend();envelope=paused.tick(60000);check(envelope.quiet==beforePause);paused.result={};
    for(int i=0;i<40;++i)envelope=paused.tick();check(envelope.quiet==1);
    // PATs (including two-point tries) and kickoff phases retain the lull.
    for(int type:{21,22,23,24,25}){f.p.selectedType=type;f.p.specialTeams=true;f.p.down=type==23?0:-1;++f.seq;f.p.game.phase=1;check(f.tick().quiet==1);f.p.game.phase=3;check(f.tick().quiet==1);}
    f.p.game.phase=1;f.p.down=1;f.p.selectedType=103;f.p.specialTeams=false;check(f.tick().quiet==0);
    // Missing play type cannot guess a new normal possession.
    Fixture unknown;unknown.td();unknown.fade();++unknown.seq;unknown.p.game.phase=1;unknown.p.selectedType=-1;check(unknown.tick().quiet==1);
    unknown.p.game.phase=3;check(unknown.tick().quiet==0);
    Fixture ready;ready.td();ready.fade();++ready.seq;ready.p.game.phase=1;check(ready.tick().quiet==0);
    // Home scores, corrections, mode changes and new worlds release safely.
    for(int which=0;which<9;++which){Fixture x;x.td();x.fade();x.result={};uintptr_t match=42;unsigned epoch=1;bool allowed=true;
        if(which==0){++x.seq;x.result.accepted=x.result.completed=true;x.result.sequence=x.seq;x.result.homePoints=2;x.p.game.homeScore+=2;}
        if(which==1)x.p.game.awayScore-=6;
        if(which==2)x.p.game.seconds+=60;
        if(which==3)x.p.game.neutral=true;
        if(which==4)x.p.simulated=true;
        if(which==5)allowed=false;
        if(which==6)match=43;
        if(which==7)++epoch;
        if(which==8)x.p.game.phase=7;
        check(x.tick(50,allowed,match,epoch).quiet==0);
    }
    // A field goal, PAT, safety, stale/uncompleted/unaccepted score, sequence
    // mismatch, mixed correction, or repeated score cannot start this window.
    for(int which=0;which<10;++which){Fixture x;auto out=x.td();(void)out;x.model.reset();x.p.game.awayScore=7;x.result={};x.tick();
        x.p.game.awayScore+=6;x.result.accepted=x.result.completed=true;x.result.sequence=x.seq;x.result.side=1;x.result.awayPoints=6;
        if(which<3)x.result.awayPoints=which+1;
        if(which==3)x.result.accepted=false;if(which==4)x.result.completed=false;
        if(which==5)x.result.sequence=99;if(which==6)x.result.side=0;
        if(which==7)x.result.homePoints=6;if(which==8)x.p.simulated=true;
        if(which==9)x.p.game.awayScore=7;
        check(!x.tick().newTouchdown);check(x.fade().quiet==0);
    }
    // Drive the existing native-result tracker: ordinary passing/rushing,
    // defensive and kickoff-return touchdowns all carry verified away points.
    for(int startSide:{0,1})for(int down:{0,1}){
        energy::Tracker tracker;energy::Settings config;homequiet::Model quiet;energy::Play p;
        p.game={2,28,7,1,400,720};p.down=down;p.offense=startSide;p.selectedType=down?103:23;p.specialTeams=!down;
        auto out=tracker.observe(config,p);quiet.observe(p,out,42,1,tracker.serial,1000,true);
        p.game.phase=3;out=tracker.observe(config,p);quiet.observe(p,out,42,1,tracker.serial,1100,true);
        p.game.phase=4;p.game.seconds-=7;p.game.awayScore+=6;out=tracker.observe(config,p);
        auto q=quiet.observe(p,out,42,1,tracker.serial,1200,true);check(q.newTouchdown&&out.awayPoints==6);
        check(!quiet.observe(p,out,42,1,tracker.serial,1250,true).newTouchdown);
    }
    // Relative visitor-size floor, unknown population, no change at neutral.
    float lastFloor=0;
    for(float share:{0.f,.01f,.03f,.06f,.15f,.35f,1.f}){Fixture x;x.td();envelope=x.fade();envelope.visitorShare=share;check(envelope.postGain()>=lastFloor);lastFloor=envelope.postGain();}
    envelope.visitorShare=-1;check(std::abs(envelope.postGain()-.35f)<.00001f);
    auto mix=balance::mix(1,61888);auto quietMix=balance::ambient(mix,envelope.ambientGain());
    check(mix.gain[0]==quietMix.gain[0]&&!quietMix.mute&&quietMix.valid);
    for(unsigned i=0;i<7;++i){bool ambient=i==1||i==2||i==4;check(ambient?quietMix.net(i)<mix.net(i):quietMix.net(i)==mix.net(i));}
    check(balance::ambient(mix,1).key==mix.key&&balance::ambient(mix,NAN).key==mix.key);
    check(balance::ambient(balance::mix(0,61888),.2f).key==balance::mix(0,61888).key);
    // Visitors are added after the home background is restrained and retain
    // their own scaled sustained response rather than being quieted again.
    visitors::Population pop;pop.verified=true;pop.match=42;pop.epoch=1;pop.home=9400;pop.away=600;
    sustained::Intent visitor=post;visitor.reason=sustained::Reason::Result;visitor.active=true;visitor.intensity=1000;
    auto combined=visitors::layer(post,visitor,pop,10000,1);
    check(combined.intensity>=post.intensity&&combined.reason==sustained::Reason::Result);
    // Constant-time model: measure separately from native audio/FPS.
    Fixture bench;bench.td();bench.result={};volatile float sink=0;auto begin=std::chrono::steady_clock::now();
    for(int i=0;i<200000;++i)sink+=bench.tick(1).ambientGain();
    double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();
    check(std::isfinite(sink));std::printf("PASS: %u home-crowd quiet checks; 200000 model updates %.3f ms (offline only).\n",checks,ms);
}
