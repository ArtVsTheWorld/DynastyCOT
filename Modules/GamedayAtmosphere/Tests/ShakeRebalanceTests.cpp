#include "../Source/DefensiveAccents.h"
#include "../Source/CannonImpact.h"
#include <cassert>
#include <cstdio>
using namespace attendance::motion;
namespace defense=attendance::defense;namespace live=attendance::live;
int main(){
    Settings s;assert(s.ordinary==5&&s.critical==10&&s.live==11&&s.catchImpact==12&&s.touchdown==16&&s.hostileOpeningMaximum==10);
    const float counts[]{3030,6060,15000,30000,50000,61888};
    const float factors[]{.7756f,.8f,.8721f,.9930f,1.1542f,1.25f};
    for(int i=0;i<6;++i)assert(std::fabs(occupiedScale(static_cast<uint64_t>(counts[i]),1)-factors[i])<.001f);
    assert(occupiedScale(6060,.5f)<occupiedScale(6060,.99f)&&occupiedScale(6060,.99f)<occupiedScale(51424,.85f));
    assert(occupiedScale(6060,0)==0&&occupiedScale(0,1)==0&&occupiedScale(6060,NAN)==0);
    // Independent visitor and neutral sliders scale the same home baseline
    // once, including the endpoints, without changing each other's venue.
    assert(s.awayPercent==0&&s.neutralPercent==75);
    for(float percent:{0.f,1.f,25.f,50.f,75.f,100.f})for(float neutralPercent:{0.f,50.f,75.f,100.f})for(bool neutral:{false,true})for(float population:{.8f,1.1f,1.25f})for(auto profile:{Profile::Gain,Profile::Truck,Profile::Catch,Profile::Takeaway,Profile::Loss}){
        Model home,away;Input x;x.match=1;x.epoch=1;x.allowed=x.duringAllowed=true;x.phase=4;x.now=1000;x.duringPlay=1;x.duringImpact=true;
        x.profile=profile;x.venueScale=population;x.duringCatch=profile==Profile::Catch;
        auto normal=home.observe(s,x);auto selected=s;selected.awayPercent=percent;selected.neutralPercent=neutralPercent;x.neutral=neutral;
        if(!neutral){x.venueScale*=awayScale(selected);x.allowed=awayScale(selected)>0;}
        auto visitor=away.observe(selected,x);
        float expected=neutral?neutralPercent:percent;assert(std::fabs(visitor.percent-normal.percent*expected*.01f)<.0001f);
        if(expected==0)assert(visitor.kind==Kind::None);
    }
    // Turning neutral movement down to zero cancels an already-running score;
    // the independently controlled physical cannon remains available.
    {Model model;Input x;x.match=1;x.epoch=1;x.now=1000;x.phase=4;x.allowed=x.neutral=true;x.touchdown=1;
     auto config=s;auto cue=model.observe(config,x);assert(cue.kind==Kind::Touchdown);
     x.now=1200;config.neutralPercent=0;cue=model.observe(config,x);assert(cue.kind==Kind::None);
     attendance::cannon::Blast blast{1,1,1100,10};assert(attendance::cannon::merge(cue,blast,1200,18,true).kind==Kind::Cannon);}
    for(auto p:{Profile::Truck,Profile::Gain,Profile::Skill,Profile::Catch,Profile::Stop,Profile::Loss,Profile::Conversion,Profile::Downs,Profile::Takeaway,Profile::Force,Profile::GoalStop,Profile::HighStop}){
        for(bool neutral:{false,true})for(float leverage:{0.f,.5f,1.f})for(float population:{.8f,1.1f}){
            Model m;Input x;x.match=1;x.epoch=1;x.allowed=true;x.phase=4;x.now=1000;x.duringPlay=1;x.duringImpact=true;
            x.profile=p;x.leverage=leverage;x.venueScale=population;x.neutral=neutral;x.duringCatch=p==Profile::Catch;
            x.forcedImpact=p==Profile::Force;x.softAccent=p==Profile::Skill;
            auto a=m.observe(s,x);float expected=profileStrength(p,leverage)*(p==Profile::Catch?s.catchImpact/9.f:1.f)*population*(neutral?.75f:1.f);
            assert(std::fabs(a.percent-expected)<.001f&&a.until==1000+profileDuration(p));
            // Actual production receives no outcome on later ticks. Profile,
            // leverage and sharp force attack must survive that one-shot input.
            x.now=1200;x.duringPlay=0;x.profile=Profile::Default;x.leverage=0;x.forcedImpact=x.softAccent=x.duringCatch=false;
            auto b=m.observe(s,x);assert(b.profile==p&&b.leverage==leverage&&b.forcedImpact==a.forcedImpact&&b.softAccent==a.softAccent);
            assert(std::fabs(b.percent-expected)<.001f&&b.began==a.began&&b.until==a.until);
            for(x.now=1400;x.now<=a.until;x.now+=100)m.observe(s,x);
            x.now=a.until;assert(m.observe(s,x).kind==Kind::None);
        }
    }
    for(bool home:{false,true})for(float choice:{0.f,.5f,1.f}){
        Model m;Input x;x.match=1;x.allowed=x.lineup=x.venueEligible=true;x.lineupId=1;x.phase=1;x.now=1000;x.lineupScale=choice;
        auto a=m.observe(s,x);assert(a.percent>=2.5f&&a.percent<=5);
        x.critical=true;x.homeCritical=home;x.now=2000;a=m.observe(s,x);
        assert(a.percent>=(home?5.625f:7.5f)&&a.percent<=(home?7.5f:10.f));
        x.critical=x.homeCritical=false;x.lineupStrengthScale=.5f;x.now=2100;a=m.observe(s,x);assert(a.percent>=1.25f&&a.percent<=2.5f);
        x.lineupStrengthScale=1;x.hostileOpening=s.hostileOpeningMaximum;x.now=2200;a=m.observe(s,x);assert(a.percent>=8&&a.percent<=10);
        x.lineup=false;x.phase=3;x.now=2300;a=m.observe(s,x);assert(a.tail&&a.until==3200); // Snap still fades.
    }
    for(bool eruption:{false,true}){
        Model m;Input x;x.match=1;x.allowed=true;x.phase=3;x.now=1000;x.runBuildup=1;x.runProgress=1;x.leverage=.5f;
        m.observe(s,x);x.now=1300;auto gain=m.observe(s,x);assert(gain.percent==11);
        x.phase=4;x.now=1400;x.runBuildup=0;x.eruptionAlive=eruption;if(eruption)x.eruption=1;else x.touchdown=1;
        auto score=m.observe(s,x);float previous=strength(score,x.now);assert(previous==11&&score.oscillationBegan==gain.oscillationBegan);
        for(x.now=1410;x.now<=1700;x.now+=10){score=m.observe(s,x);float level=strength(score,x.now);assert(level>=previous&&level<=(eruption?30.f:20.f));previous=level;}
        assert(std::fabs(previous-(eruption?30.f:20.f))<.001f);
    }
    // Lower live impacts are consumed without shortening an earned takeaway.
    {Model m;Input x;x.match=1;x.allowed=true;x.phase=3;x.now=1000;x.duringPlay=1;x.profile=Profile::Takeaway;
        m.observe(s,x);x.now=1300;auto before=m.observe(s,x);x.now=1400;x.duringPlay=2;x.duringImpact=true;x.profile=Profile::Stop;
        auto after=m.observe(s,x);assert(after.began==before.began&&after.until==before.until&&after.profile==Profile::Takeaway);
        x.duringPlay=3;x.profile=Profile::Force;x.forcedImpact=true;x.now=1500;after=m.observe(s,x);assert(after.profile==Profile::Force);
        x.now=1550;assert(strength(m.observe(s,x),x.now)==13);
    }
    // Custom zero settings remain real opt-outs even under profile selection.
    {Model m;Input x;x.match=1;x.allowed=true;x.phase=3;x.now=1000;x.duringPlay=1;
        defense::motion({live::Event::ForcedFumble,1,0,0},x);m.observe(s,x);x.now=1200;assert(strength(m.observe(s,x),x.now)==13);
        x.now=1300;x.duringPlay=2;x.duringImpact=x.forcedImpact=false;x.profile=Profile::Takeaway;
        auto carried=m.observe(s,x);assert(strength(carried,x.now)>=13&&strength(carried,x.now)<=13*1.15f); // Force peak stays above the return.
        x.duringPlay=0;for(x.now=1400;x.now<=4500;x.now+=100)carried=m.observe(s,x);
        assert(carried.profile==Profile::Takeaway&&!carried.impactOnly&&strength(carried,4500)==12);
        x.now=4600;x.phase=4;assert(m.observe(s,x).tail);x.allowed=false;x.now=4700;assert(m.observe(s,x).kind==Kind::None);
    }
    {auto off=s;off.live=off.catchImpact=0;for(auto p:{Profile::Truck,Profile::Skill,Profile::Force,Profile::Gain,Profile::Catch})assert(liveStrength(off,p,1,p==Profile::Catch)==0);}
    {auto custom=s;custom.live=22;custom.catchImpact=18;custom.impactExtraMs=2000;
        assert(liveStrength(custom,Profile::Force,0)==26&&liveStrength(custom,Profile::Catch,.5f,true)==18);
        assert(impactDuration(custom,Profile::Catch)==3700&&impactDuration(custom,Profile::Stop)==3300);
        custom.impactExtraMs=0;assert(impactDuration(custom,Profile::Force)==2000);
    }
    for(int family=0;family<4;++family)for(bool saving:{false,true}){
        Model m;Input x;x.match=1;x.allowed=true;x.phase=4;x.now=1000;x.gameSavingBlock=saving;
        if(family==0)x.touchdown=1;if(family==1)x.safety=1;if(family==2)x.clincher=1;if(family==3)x.blockedFieldGoal=1;
        auto cue=m.observe(s,x);x.now=1300;cue=m.observe(s,x);
        assert(cue.percent==(family==0?16.f:family==3?(saving?16.f:10.f):12.f));
        assert(cue.impact==(family==0?20.f:family==3?(saving?20.f:14.f):16.f));
    }
    assert(defense::shakeProfile(live::Event::FourthConversion)==Profile::Conversion);
    assert(defense::shakeProfile(live::Event::PassBreakup)==Profile::Stop);
    for(auto event:{live::Event::Juke,live::Event::Spin,live::Event::BrokenTackle,live::Event::HitStick})assert(!live::cameraReaction(event));
    assert(profileStrength(Profile::Skill,1)<profileStrength(Profile::Stop,0)&&profileStrength(Profile::Stop,1)<=profileStrength(Profile::Conversion,0));
    assert(profileStrength(Profile::Downs,1)<=profileStrength(Profile::Takeaway,0)&&profileStrength(Profile::Force,1)<s.touchdownImpact);
    std::puts("PASS: occupied-seat reference curve; all profile ranges, neutral applied once; latched one-shot events; short expiry; softer preplay; monotone gain-to-score peak; lower-event retention; zero controls and existing trigger exclusions.");
}
