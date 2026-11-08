#include "../Source/DirectorShake.h"
#include <cassert>
#include <iostream>
#include <limits>
using namespace attendance::motion;
int main(){
    // Steady post-TD envelope must not repeatedly lose both main rotation
    // axes together. Check every millisecond across multiple former beat periods.
    {Cue steady{123,1,1000,1000,10000,Kind::LivePlay,10};
        std::array<float,16> identity{};identity[0]=identity[5]=identity[10]=identity[15]=1;
        for(uint64_t now=2000;now<5000;++now){steady.updated=now;auto rotated=identity;assert(apply(rotated,steady,now));
            float pitch=rotated[6],yaw=rotated[2];assert(std::hypot(pitch,yaw)>.0007f);}}
    // Neutral attenuation is applied exactly once before transitions: fixed
    // floors, random lineup choices, live distance ramps and score handoffs
    // all retain the same shape/timing at 75% of the normal amplitude.
    for(float choice:{0.f,.5f,1.f})for(int scenario=0;scenario<10;++scenario){
        Model home,split;Settings config;Input a;a.match=321;a.epoch=1;a.allowed=a.venueEligible=a.lineup=true;
        a.phase=1;a.lineupId=1;a.lineupScale=choice;a.critical=scenario==1;a.hostileOpening=scenario==2?13.6f:0;
        for(uint64_t now=1000;now<=14500;now+=50){
            a.now=now;
            if(now>=3000){a.phase=3;a.lineup=false;}
            if(now>=3400&&now<4200){a.runBuildup=1;a.runProgress=(now-3400)/800.f;}else a.runBuildup=0;
            if(now==4000){a.duringPlay=1;a.duringCatch=scenario==3;a.duringImpact=scenario==4||scenario==5;a.softAccent=scenario==4;a.forcedImpact=scenario==5;}
            if(now>=4400)a.phase=4;
            if(now==4600){if(scenario==6)a.touchdown=1;if(scenario==7){a.eruption=1;a.eruptionAlive=true;}if(scenario==8)a.safety=1;if(scenario==9)a.clincher=1;}
            auto b=a;b.neutral=true;auto normal=home.observe(config,a),neutral=split.observe(config,b);
            assert(normal.kind==neutral.kind&&normal.began==neutral.began&&normal.until==neutral.until);
            assert(std::fabs(strength(normal,now)*.75f-strength(neutral,now))<.002f);
        }
    }
    Settings s;assert(s.ordinary==5&&s.critical==10&&s.touchdownMs==11000&&s.lineupCheer==6);
    assert(lineupStrength(s,false,0)==2.5f&&lineupStrength(s,false,1)==5);
    assert(lineupStrength(s,true,0)==7.5f&&lineupStrength(s,true,1)==10);
    {auto custom=s;custom.ordinary=custom.critical=0;assert(lineupStrength(custom,false,1)==0&&lineupStrength(custom,true,0)==0);
        custom.ordinary=1;custom.critical=3;assert(lineupStrength(custom,false,0)==.5f&&lineupStrength(custom,true,0)==2.25f);}
    s.ordinary=5;s.critical=10;s.touchdown=15;s.live=10;s.catchImpact=8;s.touchdownMs=9000; // Explicit legacy profile exercises the existing envelope math.
    Input x;x.match=123;x.epoch=1;x.now=1000;x.phase=1;x.allowed=x.lineup=true;x.lineupId=1;
    Model m;auto c=m.observe(s,x);assert(c.kind==Kind::None); // A critical down alone cannot qualify.
    x.critical=true;x.now=1100;c=m.observe(s,x);assert(c.kind==Kind::None);
    x.venueEligible=true;x.critical=false;x.now=2000;c=m.observe(s,x);assert(strength(c,2000)==0);
    x.now=3000;c=m.observe(s,x);assert(strength(c,3000)==5);
    x.critical=true;c=m.observe(s,x);assert(strength(c,3000)==5); // Ease up rather than jumping to the new target.
    x.now=3200;c=m.observe(s,x);assert(strength(c,3200)==10);
    x.lineup=false;x.phase=3;c=m.observe(s,x);assert(c.tail&&strength(c,3200)==10);
    x.now=3650;c=m.observe(s,x);assert(std::fabs(strength(c,3650)-5)<.001);
    x.now=4100;c=m.observe(s,x);assert(c.kind==Kind::None);
    x.phase=4;x.now=4100;x.touchdown=1;c=m.observe(s,x);assert(c.kind==Kind::Touchdown&&c.until==13100);
    x.now=4300;c=m.observe(s,x);assert(strength(c,4300)==20);
    x.now=5100;c=m.observe(s,x);assert(std::fabs(strength(c,5100)-20)<.001);
    x.now=5600;c=m.observe(s,x);assert(strength(c,5600)==15);
    x.now=9000;c=m.observe(s,x);assert(strength(c,9000)==15);
    x.now=11100;c=m.observe(s,x);assert(std::fabs(strength(c,11100)-7.5)<.001);
    x.now=13100;c=m.observe(s,x);assert(c.kind==Kind::None);
    x.now=13200;x.touchdown=2;c=m.observe(s,x);
    x.now=13500;x.eruption=1;x.eruptionAlive=true;c=m.observe(s,x);assert(c.kind==Kind::Eruption&&c.began==13200&&strength(c,13500)==20);
    x.now=15200;c=m.observe(s,x);assert(strength(c,15200)==22.5f);
    x.now=21200;c=m.observe(s,x);assert(strength(c,21200)==22.5f);
    x.now=25200;c=m.observe(s,x);assert(std::fabs(strength(c,25200)-11.25f)<.001);
    assert(strength(c,26000)==0); // Stale producer cancels even a long eruption.
    x.now=29200;c=m.observe(s,x);assert(c.kind==Kind::None); // Same IDs cannot retrigger.
    x.now=30000;x.touchdown=3;c=m.observe(s,x);assert(c.kind==Kind::Touchdown);
    x.now=30200;x.phase=1;x.lineup=true;x.lineupId=2;x.critical=false;c=m.observe(s,x);
    assert(c.kind==Kind::Touchdown&&strength(c,30200)>0); // A camera cut into lineup retains the earned celebration.
    x.now=31200;c=m.observe(s,x);assert(c.kind==Kind::Touchdown&&strength(c,31200)>=15&&strength(c,31200)<=20);
    x.allowed=false;x.now=31300;c=m.observe(s,x);assert(c.kind==Kind::None); // Disabling motion cancels immediately.
    x.allowed=false;x.now=31400;x.touchdown=4;c=m.observe(s,x);assert(c.kind==Kind::None);
    x.allowed=true;x.phase=4;x.lineup=false;x.now=31500;c=m.observe(s,x);assert(c.kind==Kind::None); // No replay of blocked event.
    x.epoch=2;x.now=32000;x.touchdown=0;x.eruption=0;x.lineup=true;x.lineupId=1;x.phase=1;
    c=m.observe(s,x);assert(c.epoch==2&&c.began==32000);
    s.ordinary=0;x.now=33000;c=m.observe(s,x);assert(strength(c,33000)==0);
    x.now=31000;c=m.observe(s,x);assert(c.kind==Kind::None); // Time rollback cancels.
    s.ordinary=999;s.critical=-1;s.touchdown=std::numeric_limits<float>::quiet_NaN();
    assert(configured(s,Kind::Ordinary)==100&&configured(s,Kind::Critical)==0&&configured(s,Kind::Touchdown)==0);
    // During-play motion has priority over the lineup tail and settles after the whistle.
    Settings liveSettings;liveSettings.live=10;liveSettings.touchdown=15;liveSettings.catchImpact=8;liveSettings.reactionFadeMs=2000;liveSettings.impactExtraMs=0;liveSettings.ordinary=5;Model liveModel;Input v;v.match=999;v.epoch=1;v.allowed=true;v.venueEligible=true;v.phase=3;v.now=50000;v.duringPlay=1;
    auto liveCue=liveModel.observe(liveSettings,v);assert(liveCue.kind==Kind::LivePlay);
    v.now=50200;liveCue=liveModel.observe(liveSettings,v);assert(strength(liveCue,v.now)==10);
    auto anchor=liveCue.oscillationBegan;v.now=50900;liveModel.observe(liveSettings,v);v.phase=4;v.now=51000;liveCue=liveModel.observe(liveSettings,v);assert(liveCue.tail&&strength(liveCue,v.now)==10&&liveCue.oscillationBegan==anchor);
    v.now=52000;liveCue=liveModel.observe(liveSettings,v);assert(strength(liveCue,v.now)>7.4f&&strength(liveCue,v.now)<7.5f); // Brief hold precedes the smooth fade.
    v.now=53000;liveCue=liveModel.observe(liveSettings,v);assert(liveCue.kind==Kind::None);
    v.phase=3;v.now=54000;v.duringPlay=2;liveModel.observe(liveSettings,v);v.phase=4;v.now=54200;v.touchdown=1;liveCue=liveModel.observe(liveSettings,v);assert(liveCue.kind==Kind::Touchdown);
    v.allowed=false;v.now=54300;assert(liveModel.observe(liveSettings,v).kind==Kind::None);
    Model immediateModel;Input hit;hit.match=555;hit.epoch=1;hit.allowed=hit.duringAllowed=true;hit.phase=4;hit.now=60000;hit.duringPlay=1;
    auto impact=immediateModel.observe(liveSettings,hit);assert(impact.kind==Kind::LivePlay&&impact.impactOnly);
    hit.now=60200;impact=immediateModel.observe(liveSettings,hit);assert(strength(impact,hit.now)>9);
    hit.now=61400;impact=immediateModel.observe(liveSettings,hit);assert(strength(impact,hit.now)>0&&strength(impact,hit.now)<6);
    hit.now=62400;assert(immediateModel.observe(liveSettings,hit).kind==Kind::None);
    // Loss of competitive context fades an existing live cue rather than cutting it.
    hit.phase=3;hit.now=63000;hit.duringPlay=2;immediateModel.observe(liveSettings,hit);hit.now=63300;immediateModel.observe(liveSettings,hit);
    hit.duringAllowed=false;hit.now=63400;impact=immediateModel.observe(liveSettings,hit);assert(impact.tail&&strength(impact,hit.now)>0);
    hit.now=64400;impact=immediateModel.observe(liveSettings,hit);assert(strength(impact,hit.now)>0&&strength(impact,hit.now)<6);
    // Upgrades preserve both current amplitude and the oscillation clock.
    Model joined;Input j;j.match=777;j.epoch=1;j.allowed=true;j.phase=3;j.now=70000;j.duringPlay=1;
    joined.observe(liveSettings,j);j.now=70200;auto moving=joined.observe(liveSettings,j);assert(strength(moving,j.now)==10);
    j.phase=4;j.now=70400;j.touchdown=1;auto scoredCue=joined.observe(liveSettings,j);assert(scoredCue.kind==Kind::Touchdown&&strength(scoredCue,j.now)==10&&scoredCue.oscillationBegan==moving.oscillationBegan);
    j.now=70600;scoredCue=joined.observe(liveSettings,j);float joinedTd=10+10*smooth(200.f/300);assert(std::fabs(strength(scoredCue,j.now)-joinedTd)<.001f);
    j.eruption=1;j.eruptionAlive=true;auto eruptedCue=joined.observe(liveSettings,j);assert(eruptedCue.kind==Kind::Eruption&&std::fabs(strength(eruptedCue,j.now)-joinedTd)<.001f&&eruptedCue.oscillationBegan==moving.oscillationBegan);
    j.now=70800;eruptedCue=joined.observe(liveSettings,j);assert(std::fabs(strength(eruptedCue,j.now)-(joinedTd+(30.f-joinedTd)*smooth(200.f/300)))<.001f);
    j.now=70900;eruptedCue=joined.observe(liveSettings,j);assert(strength(eruptedCue,j.now)==30.f);
    Model kick;Input k;k.match=888;k.epoch=1;k.allowed=k.lineup=k.venueEligible=k.openingKickoff=true;k.lineupId=1;k.phase=1;k.now=80000;
    kick.observe(liveSettings,k);k.now=81000;assert(strength(kick.observe(liveSettings,k),k.now)==5);
    k.lineup=false;k.phase=3;k.now=82000;auto heldKick=kick.observe(liveSettings,k);assert(!heldKick.tail&&strength(heldKick,k.now)==5);
    k.now=82500;kick.observe(liveSettings,k);k.phase=4;k.now=83000;heldKick=kick.observe(liveSettings,k);assert(heldKick.tail&&heldKick.until==85000&&strength(heldKick,k.now)==5);
    k.now=84000;assert(std::fabs(strength(kick.observe(liveSettings,k),k.now)-2.5f)<.001f);
    k.now=85000;assert(kick.observe(liveSettings,k).kind==Kind::None);
    std::array<float,16> projection{};projection[0]=2;projection[5]=3.1715948f;projection[11]=1;
    assert(projectionScale(projection,Kind::Eruption)==1);projection[5]=12;
    assert(projectionScale(projection,Kind::Eruption)<.43f&&projectionScale(projection,Kind::Touchdown)<.43f);
    for(auto kind:{Kind::Ordinary,Kind::Critical,Kind::LivePlay,Kind::Touchdown,Kind::Eruption,Kind::Safety,Kind::BlockedKick})assert(projectionScale(projection,kind)<.43f);
    projection[5]=500;assert(projectionScale(projection,Kind::Eruption)<.038f);
    // Read-only default RTG capture: 88.5-degree vertical FOV. Its screen
    // displacement per small angular movement should match the reference view.
    projection[0]=.5773501992f;projection[5]=1.0264003277f;
    for(auto kind:{Kind::Ordinary,Kind::Critical,Kind::LivePlay,Kind::Touchdown,Kind::Eruption,Kind::Safety,Kind::BlockedKick,Kind::Clincher}){
        assert(projectionScale(projection,kind)>3.08f&&projectionScale(projection,kind)<3.10f);
        const auto scale=projectionScale(projection,kind,true);
        assert(scale>3.08f&&scale<3.10f);
        assert(std::fabs(projection[5]*scale-3.1715948f)<.00001f);
        projection[5]=.11f;assert(projectionScale(projection,kind)==4);assert(projectionScale(projection,kind,true)==6);
        projection[5]=1.0264003277f;
    }
    assert(projectionScale(projection,Kind::None)==1);
    projection[5]=NAN;assert(projectionScale(projection,Kind::Eruption)==0);
    std::array<float,16> source{1,0,0,0,0,1,0,0,0,0,1,0,12,-30,90,1};
    c={123,1,1000,0,5000,Kind::Eruption,30,false};auto output=source;
    assert(apply(output,c,1000)&&output!=source&&validMatrix(output));
    // A pure camera-space rotation preserves camera position and zoom.
    for(int col=0;col<3;++col){float before=0,after=0;for(int row=0;row<3;++row){before-=source[12+row]*source[col*4+row];after-=output[12+row]*output[col*4+row];}assert(std::fabs(before-after)<.001);}
    assert(output[3]==0&&output[7]==0&&output[11]==0&&output[15]==1);
    auto invalid=source;invalid[0]=2;auto saved=invalid;assert(!apply(invalid,c,1000)&&invalid==saved);
    invalid=source;invalid[1]=std::numeric_limits<float>::infinity();assert(!validMatrix(invalid));
    c.percent=0;output=source;assert(!apply(output,c,1000)&&output==source);
    std::cout<<"PASS: configurable motion, venue gating, impact-hold-fade profiles, earned post-TD carry until expiry/new snap, event priority/deduplication, pause/new-world cleanup and camera-position-preserving matrix rotation.\n";
}
