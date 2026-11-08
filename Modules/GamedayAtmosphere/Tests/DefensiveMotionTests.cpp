#include "../Source/DefensiveMotion.h"
#include "../Source/OvertimeAtmosphere.h"
#include "../Source/SoundOnlyReactions.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
struct Case {
    defensivemotion::Model model;energy::Play snap,p;energy::Outcome result;
    uintptr_t match=123;unsigned epoch=1;uint64_t sequence=1,now=1000,prior=0;bool allowed=true;
    Case(int down=1,int side=1,bool neutral=false){
        p.game={2,14,14,1,300,720};p.game.neutral=neutral;p.down=down;p.offense=side;
        p.selectedType=103;p.toFirst=10;p.toGoal=60;p.fieldSpot=-10;p.attackDirection=1;p.resultStamp=10;
        snap=p;tick(1);tick(3);
    }
    live::Outcome tick(int phase,live::Outcome evidence={}){
        now+=100;p.game.phase=phase;
        return model.observe(snap,p,result,evidence,match,epoch,sequence,now,allowed,prior);
    }
    void finish(float yards=0){
        result.accepted=result.completed=true;result.sequence=sequence;result.side=1-snap.offense;result.yards=yards;
        result.fiveYardLoss=yards<=-4.99f;p.down=snap.down+1;
        if(snap.down==4){p.offense=1-snap.offense;p.down=1;result.turnoverOnDowns=true;}
    }
};
int main(){
    // Measured loss, not native sack truth, determines standalone loss motion.
    unsigned cases=0;
    for(int down:{1,2,3,4})for(int side:{0,1})for(bool neutral:{false,true})
    for(int phase:{1,4,5})for(float yards:{-10.f,-5.f,-4.98f,-2.f,0.f})for(bool sack:{false,true}){
        Case c(down,side,neutral);c.finish(yards);c.result.sack=sack;
        auto evidence=live::completedImpact(c.result,c.p.game,true,0,59);
        auto out=c.tick(phase,evidence);bool loss=yards<=-5;bool expected=(side==1||neutral)&&(loss||down==4);
        assert(bool(out.id)==expected);
        if(out.id){assert(out.event==(loss?live::Event::TackleForLoss:live::Event::DownsStop));assert(out.side==1-side);}
        assert(!c.tick(phase,evidence).id);++cases;
    }
    for(float unknown:{NAN,-1000.f,-2.f}){
        Case c;c.finish(unknown);c.result.sack=true;c.p.game.phase=4;
        auto sound=live::completedImpact(c.result,c.p.game,true,0,59);assert(sound.event==live::Event::Sack);
        assert(!c.tick(4,sound).id&&defensivemotion::cameraId(sound,123,{})==0);
        assert(live::cheerType(sound)==59); // Sound stays distinct from motion.
    }
    // End-to-end tracker -> sound classifier -> shared camera router, including
    // independent line-of-scrimmage directions and a short native-confirmed sack.
    for(int direction:{-1,1})for(float yards:{-6.f,-5.f,-4.f})for(bool sack:{false,true}){
        Case c;c.p.attackDirection=direction;c.p.fieldSpot=-10.f*direction;c.snap=c.p;c.snap.game.phase=1;
        energy::Tracker tracker;energy::Settings settings;tracker.observe(settings,c.snap);
        c.p.game.phase=3;tracker.observe(settings,c.p);c.p.game.phase=4;c.p.down=2;
        c.p.game.seconds=295;c.p.resultStamp=11;c.p.fieldSpot+=yards*direction;c.p.toGoal-=yards;c.p.toFirst-=yards;c.p.sack=sack;
        c.result=tracker.observe(settings,c.p);assert(c.result.completed);
        auto sound=live::completedImpact(c.result,c.p.game,true,0,59);auto out=c.tick(4,sound);
        assert(bool(out.id)==(yards<=-5));
        auto id=defensivemotion::cameraId(sound,19,out);assert(id==out.id);
        motion::Model camera;motion::Settings cameraSettings;motion::Input x;
        x.match=123;x.epoch=1;x.now=c.now;x.phase=4;x.allowed=x.duringAllowed=true;x.duringPlay=id;x.duringImpact=out.id!=0;x.profile=motion::Profile::Loss;
        auto cue=camera.observe(cameraSettings,x);assert((cue.kind==motion::Kind::LivePlay)==(yards<=-5));
        if(out.id)assert(cue.impactOnly&&cue.percent==8&&cue.until==c.now+2700);
    }
    // Knockout/contact can arrive before the whistle, but cannot shake until
    // the shared completed stop is verified. Routine third-down incompletions
    // still need their existing impact flavor or high-leverage/goal condition.
    for(auto flavor:{live::Event::CatchKnockout,live::Event::PassBreakup,live::Event::HitStick,live::Event::HitStickStop}){
        for(int down:{1,2,3,4}){
            Case c(down);assert(!c.tick(3,{flavor,1,0,0}).id);assert(!c.tick(4).id);
            c.finish();auto out=c.tick(5);assert(bool(out.id)==(down>=3));
            if(out.id)assert(out.event==(flavor==live::Event::HitStick?live::Event::HitStickStop:flavor));
            assert(!c.tick(1,{flavor,2,0,0}).id);
        }
    }
    {Case c(3);c.finish();assert(!c.tick(4).id);assert(c.tick(5,{live::Event::PassBreakup,1,0,0}).event==live::Event::PassBreakup);}
    {Case c(3);c.finish();for(int n=0;n<52;++n)assert(!c.tick(4).id);assert(!c.tick(5,{live::Event::PassBreakup,1,0,0}).id);} // No lease refresh.
    {Case c(3);c.finish();assert(!c.tick(4,{live::Event::CatchKnockout,1,1,0}).id);} // Wrong defender.
    {Case c(3);c.finish();c.p.down=3;assert(!c.tick(4,{live::Event::CatchKnockout,1,0,0}).id);} // Stop not established.
    for(int kind=0;kind<4;++kind){Case c(kind==0?4:3);c.finish(kind==3?-7.f:0);
        c.result.goalLineStop=kind==0;c.result.thirdGoalStop=kind==1;c.result.highLeverageStop=kind==2;if(kind==2){c.snap.game.quarter=c.p.game.quarter=4;}
        auto out=c.tick(4,{live::Event::PassBreakup,1,0,0});
        assert(out.event==(kind==0?live::Event::GoalLineStop:kind==1?live::Event::PassBreakup:kind==2?live::Event::CriticalStop:live::Event::TackleForLoss));
    }
    for(int down:{3,4}){Case c(down);c.finish(-7);c.result.goalLineStop=down==4;c.result.thirdGoalStop=down==3;
        assert(c.tick(4).event==(down==4?live::Event::GoalLineStop:live::Event::TackleForLoss));} // Preserve goal-line profile over overlapping loss.
    // A current-play landed big hit may shake on a short loss on any down.
    for(int down:{1,2,3,4})for(float yards:{-8.f,-2.f,-.5f,0.f,2.f})for(int side:{0,1})for(bool neutral:{false,true}){
        Case c(down,side,neutral);assert(!c.tick(3,{live::Event::HitStick,1,1-side,0}).id);c.finish(yards);
        auto out=c.tick(4);bool eligible=side==1||neutral;
        assert(bool(out.id)==(eligible&&(yards<0||down>=3)));
        if(out.id)assert(out.event==live::Event::HitStickStop);
        assert(!c.tick(4).id);
    }
    assert(defense::shakeProfile(live::Event::HitStickStop)==motion::Profile::Truck);
    assert(motion::profileStrength(motion::Profile::Truck,0)==7.5f&&motion::profileStrength(motion::Profile::Truck,1)==9);
    // Ownership, accepted penalties, score correction, clock plays, gate loss,
    // replay/pause/gaps and resume cannot manufacture or queue defensive motion.
    for(int bad=0;bad<20;++bad){Case c(3);c.finish(-6);
        if(bad==0)c.allowed=false;if(bad==1)c.p.simulated=true;if(bad==2){c.p.resumed=true;c.result.accepted=false;}
        if(bad==3)c.now+=800;if(bad==4)c.result.accepted=false;if(bad==5)c.result.completed=false;
        if(bad==6)++c.result.sequence;if(bad==7)c.snap.specialTeams=true;if(bad==8)c.snap.selectedType=37;
        if(bad==9)c.snap.selectedType=38;if(bad==10)c.result.firstDown=true;if(bad==11)c.result.takeaway=true;
        if(bad==12)c.p.fumble=true;if(bad==13)c.p.interception=true;if(bad==14)c.result.homePoints=6;
        if(bad==15)++c.p.game.homeScore;if(bad==16)c.result.forcedFumble=true;
        if(bad==17)c.result.blockedFieldGoal=true;if(bad==18)c.prior=c.sequence;if(bad==19)c.snap.down=-1;
        assert(!c.tick(4,{live::Event::CatchKnockout,1,0,0}).id);
    }
    for(auto bigger:{live::Event::ForcedFumble,live::Event::Interception,live::Event::BlockedFieldGoal}){
        Case c;c.finish(-7);assert(!c.tick(4,{bigger,1,0,0}).id);assert(!c.tick(5).id);
    }
    for(int interruption=0;interruption<3;++interruption){Case c(3);c.tick(3,{live::Event::CatchKnockout,1,0,0});
        if(interruption==0)c.model.suspend();if(interruption==1)c.allowed=false;
        assert(!c.tick(interruption==2?2:3).id);c.allowed=true;c.finish();assert(!c.tick(4).id);
        ++c.sequence;c.snap.game.phase=1;c.p=c.snap;c.result={};c.tick(1);c.tick(3);c.finish(-5);assert(c.tick(4).id);
    }
    {Case c;c.finish(-7);auto out=c.tick(4);assert(out.id);c.model.reset();++c.match;++c.epoch;c.result={};c.p=c.snap;c.tick(1);c.tick(3);c.finish(-7);assert(c.tick(4).id);} // New match context.
    for(auto event:{live::Event::Sack,live::Event::TackleForLoss,live::Event::GoalLineStop,live::Event::ThirdGoalStop,
                   live::Event::CriticalStop,live::Event::DownsStop,live::Event::CatchKnockout,live::Event::PassBreakup,live::Event::HitStickStop})
        assert(defensivemotion::cameraId({event,1,0,0},99,{})==0);
    for(auto event:{live::Event::LongRun,live::Event::Interception,live::Event::FumbleRecovery,live::Event::ForcedFumble,live::Event::LongCatch})
        assert(defensivemotion::cameraId({event,1,0,0},99,{})==99);
    for(auto event:{live::Event::Spin,live::Event::Juke,live::Event::BrokenTackle,live::Event::HitStick})
        assert(defensivemotion::cameraId({event,1,0,0},99,{})==0);
    assert(!defensivemotion::cameraId({},99,{}));
    // The reported OT third-down incompletion has a confirmed stop but no
    // fresh knockout/deflection event. It still gets one generic stop impact.
    for(int side:{0,1})for(bool neutral:{false,true}){
        Case c(3,side,neutral);c.snap.game.quarter=c.p.game.quarter=5;c.snap.game.seconds=c.p.game.seconds=0;c.finish();c.result.event=energy::Event::ThirdStop;
        auto out=c.tick(4);assert(bool(out.id)==(side==1||neutral));if(out.id)assert(out.event==live::Event::OvertimeStop);
        assert(!c.tick(5).id);
    }
    {Case c(3);c.snap.game.quarter=c.p.game.quarter=5;c.finish();c.p.resumed=true;assert(c.tick(4).event==live::Event::OvertimeStop);} // A fresh tracked play after resume.
    for(bool neutral:{false,true})for(int side:{0,1})for(bool made:{false,true}){
        Case c(-1,side,neutral);c.snap.game.quarter=c.p.game.quarter=5;c.snap.game.seconds=c.p.game.seconds=0;c.finish();
        c.result.side=made?side:1-side;c.result.event=made?energy::Event::OvertimeConversion:energy::Event::ConversionStop;c.p.offense=1-side;
        if(made){if(side==0){c.result.homePoints=2;c.p.game.homeScore+=2;}else{c.result.awayPoints=2;c.p.game.awayScore+=2;c.result.visitorScore=!neutral;}}
        auto out=c.tick(4);assert(bool(out.id)==(neutral||c.result.side==0));
        if(out.id)assert(out.event==(made?live::Event::OvertimeConversion:live::Event::OvertimeConversionStop));
        assert(!c.tick(5).id);
        core::Settings settings;assert(core::positiveResult(c.result.event));assert(core::resultReaction(c.result)==(made?83:59));
        soundonly::Model sounds;auto sound=sounds.observe(c.result,c.snap,c.p,123,1,true,false,false);assert(sound.sequence);
        assert(sound.type==(!neutral&&c.result.side==1?95:made?83:59));
    }
    for(int side:{0,1})for(bool made:{false,true}){
        Case c(4,side);c.snap.game.quarter=c.p.game.quarter=5;c.snap.selectedType=21;c.snap.specialTeams=true;c.finish();c.result.turnoverOnDowns=false;c.result.side=made?side:1-side;
        if(made){if(side==0){c.result.homePoints=3;c.p.game.homeScore+=3;}else{c.result.awayPoints=3;c.p.game.awayScore+=3;}}
        auto out=c.tick(4);assert(bool(out.id)==(c.result.side==0));if(out.id)assert(out.event==(made?live::Event::OvertimeFieldGoal:live::Event::OvertimeMissedFieldGoal));
    }
    {core::Settings settings;energy::Play p;p.game={5,14,14,1,0,720};p.down=-1;p.offense=0;
        sustained::Intent base;base.match=123;base.epoch=1;base.updated=1000;
        for(int phase:{1,3,4,5}){p.game.phase=phase;auto out=overtimeeffects::fill(base,settings,p,.88f,true,40000,true);assert(out.active&&out.continuous&&out.reason==sustained::Reason::Overtime&&!out.cheers&&!out.shake&&out.layerBoostPercent==0);}
        auto earned=base;earned.active=true;earned.reason=sustained::Reason::Result;earned.priority=5;earned.resultCheer=123;assert(overtimeeffects::fill(earned,settings,p,.88f,true,40000,true).resultCheer==123);
        assert(!overtimeeffects::fill(base,settings,p,.3f,true,40000,true).active);p.game.quarter=4;assert(!overtimeeffects::fill(base,settings,p,.88f,true,40000,true).active);
    }
    {motion::Model normal,extra;motion::Settings settings;motion::Input x;x.match=123;x.epoch=1;x.phase=4;x.now=1000;x.allowed=x.duringAllowed=true;x.duringPlay=1;x.duringImpact=true;x.profile=motion::Profile::Stop;x.venueScale=.9f;
        auto a=normal.observe(settings,x);x.overtime=true;auto b=extra.observe(settings,x);assert(std::fabs(b.percent/a.percent-1.10f)<.0001f);
        x.now+=50;auto again=extra.observe(settings,x);assert(again.percent==b.percent); // Never compound each frame.
    }
    // Real tracker outcomes, including the stationary OT clock and late score
    // proof, feed the new motion/audio routes without fabricated contact flags.
    for(int side:{0,1})for(bool neutral:{false,true})for(bool made:{false,true}){
        energy::Tracker tracker;energy::Settings energySettings;energy::Play p;p.game={5,24,24,1,0,720};p.game.neutral=neutral;
        p.offense=side;p.down=-1;p.selectedType=103;p.resultStamp=100;tracker.observe(energySettings,p);auto snap=p;
        p.game.phase=3;tracker.observe(energySettings,p);p.game.phase=1;p.offense=1-side;++p.resultStamp;
        if(made){if(side==0)p.game.homeScore+=2;else p.game.awayScore+=2;}
        auto result=tracker.observe(energySettings,p);assert(result.accepted&&result.completed&&result.sequence==1);
        defensivemotion::Model camera;auto out=camera.observe(snap,p,result,{},123,1,1,1000,true);
        assert(bool(out.id)==(neutral||(made?side==0:side==1)));assert(!tracker.observe(energySettings,p).completed);
        core::Model sound;core::Settings settings;sound.react(settings,result,1,p.game,true,true,.5f);
        if(out.id)assert(sound.resultCheer==1&&sound.resultType==(made?83:59));else assert(!sound.resultCheer);
    }
    {core::Model sound;core::Settings settings;energy::Play p;p.game={5,26,24,4,0,720};p.down=-1;
        energy::Outcome r;r.accepted=r.completed=true;r.sequence=1;r.side=0;r.homePoints=2;r.event=energy::Event::OvertimeConversion;
        // A just-started winning envelope and four-second ordinary cooldown
        // cannot swallow the next independently verified OT two-point result.
        sound.eruptionAt=.5;sound.eruptionSeconds=60;sound.celebration=1;
        sound.react(settings,r,1,p.game,true,true,.5f);sound.update(settings,p,1.1,1,1);
        auto intent=sound.intent(settings,p,123,1,1100,true);assert(intent.resultCheer==1&&intent.resultType==83);
        assert(!sound.react(settings,r,1.2,p.game,true,true,.5f)&&sound.resultCheer==1);
        r.sequence=2;sound.react(settings,r,2,p.game,true,true,.5f);assert(sound.resultCheer==2);
    }
    std::printf("PASS: %u loss/sack/down/owner/phase combinations; tracker-to-camera integration; one confirmed stop; early/late contact flavor; exclusions, cleanup and audio independence.\n",cases);
}
