#include "../Source/LiveReactions.h"
#include "../Source/LineupAtmosphere.h"
#include <cassert>
#include <iostream>
using namespace attendance;
int main(){
    live::Settings settings;settings.minimumGainYards=25;live::Model model;energy::Play p;
    p.game={1,0,0,1,700,720};p.down=1;p.offense=0;p.fieldSpot=-17;p.attackDirection=1;
    uint64_t now=1000;uintptr_t match=123,record=456;bool allowed=true;
    auto tick=[&](int phase,int kind,int ball,int side,float caught){p.game.phase=phase;p.offense=side;now+=100;return model.observe(settings,p,match,1,now,allowed,record,kind,ball,caught);};
    auto begin=[&](int side){tick(1,1,1,side,0);tick(3,0,2,side,0);tick(3,3,4,side,0);};
    settings.minimumCatchYards=30;
    begin(0);auto longCatch=tick(3,1,4,0,13);assert(longCatch.event==live::Event::LongCatch&&longCatch.yards==30&&longCatch.id==1);
    assert(!tick(3,1,4,0,30).id);assert(!tick(4,1,1,0,30).id);
    begin(0);assert(!tick(3,1,4,0,12).id);assert(!tick(3,1,4,0,20).id); // Catch location is frozen once.
    begin(1);auto pick=tick(3,2,4,1,0);assert(pick.event==live::Event::Interception&&pick.id>longCatch.id);assert(!tick(3,2,4,0,0).id); // No wait for the possession label.
    p.game.neutral=true;begin(1);assert(tick(3,1,4,1,13).event==live::Event::LongCatch);
    begin(0);assert(tick(3,2,4,1,0).event==live::Event::Interception);
    p.game.neutral=false;begin(1);assert(!tick(3,1,4,1,13).id);
    p.fieldSpot=20;p.attackDirection=-1;begin(0);assert(tick(3,1,4,0,-10).yards==30);
    allowed=false;begin(0);assert(!tick(3,1,4,0,-10).id);allowed=true;assert(!tick(3,1,4,0,-10).id);
    begin(0);record=789;assert(!tick(3,1,4,0,-10).id);record=456;
    begin(0);assert(!tick(3,5,5,1,0).id); // No new fumble behavior.
    begin(0);assert(!tick(3,1,4,0,std::numeric_limits<float>::quiet_NaN()).id);
    p.simulated=true;begin(0);assert(!tick(3,2,4,1,0).id);p.simulated=false;
    settings.enabled=false;begin(0);assert(!tick(3,1,4,0,-10).id);settings.enabled=true;
    settings.minimumCatchYards=40;begin(0);assert(!tick(3,1,4,0,-19).id);begin(0);assert(tick(3,1,4,0,-20).yards==40);settings.minimumCatchYards=30;
    // A 30-yard catch that is tackled between 20 Hz samples still celebrates.
    begin(0);auto immediate=tick(4,1,4,0,-10);assert(immediate.event==live::Event::LongCatch);assert(!tick(4,1,4,0,-10).id);
    begin(0);now+=300;assert(!tick(4,1,4,0,-10).id); // No stale postplay reconstruction.
    p.fieldSpot=-20;p.attackDirection=1;
    auto runner=[&](int kind,int ball,float spot,bool held=true){now+=500;return model.observe(settings,p,match,1,now,allowed,record,kind,ball,-18,spot,held);};
    begin(0);assert(!tick(3,1,4,0,-18).id);assert(!runner(1,4,4.9f).id);
    assert(runner(1,4,5).event==live::Event::RunAfterCatch);assert(!runner(1,4,20).id);
    // A rushing play needs no pass/catch, but must have a held live ball.
    tick(1,0,1,0,0);tick(3,0,9,0,0);assert(!runner(0,9,5,false).id);
    assert(runner(3,2,10).event==live::Event::LongRun);assert(!runner(0,2,10).id);assert(!runner(0,9,5).id); // Held state 2 is a verified live runner; the same gain cannot repeat.
    begin(0);assert(!runner(3,4,30).id); // A long airborne pass is not a long run.
    begin(0);assert(!tick(3,1,4,0,-18).id);allowed=false;assert(!runner(1,4,5).id);allowed=true;
    p.fieldSpot=20;p.attackDirection=-1;begin(0);assert(!tick(3,1,4,0,18).id);assert(!runner(1,4,0).id);assert(runner(1,4,-10).event==live::Event::RunAfterCatch);
    // Third-down range and fourth-down go-for-it classification do not depend on a shake tier.
    p.down=3;p.toFirst=10;assert(lineup::criticalCamera(p,3));p.toFirst=10.1f;assert(!lineup::criticalCamera(p,3));
    p.toFirst=-1;assert(!lineup::criticalCamera(p,3));p.down=4;assert(lineup::criticalCamera(p,3));
    for(int special:{-1,21,22,23,24,25,37,38,105,106,154,155})assert(!lineup::criticalCamera(p,special));
    p.down=-1;assert(!lineup::shakePlay(p,21,false)); // PAT excluded despite sharing the field-goal type.
    p.down=4;for(int type:{21,22,105,106,154,155})assert(lineup::shakePlay(p,type,false));
    p.down=0;assert(!lineup::shakePlay(p,23,false)&&lineup::shakePlay(p,23,true));
    assert(!lineup::shakePlay(p,24,false)&&lineup::shakePlay(p,24,true)); // Receiving formation shares the verified opening exception.
    for(int type:{25,-1})assert(!lineup::shakePlay(p,type,true));
    p.down=1;assert(lineup::shakePlay(p,3,false));
    // Real runtime inputs: field LOS and world Z may be mirrored. Short
    // plays must not look like 70-yard gains before the runner has moved.
    for(int axis:{-1,1})for(int direction:{-1,1}){
        live::RunnerCoordinates coordinates;live::Model detector;energy::Play play;
        play.game={1,0,0,1,700,720};play.game.neutral=true;play.offense=0;play.down=1;
        play.fieldSpot=direction==1?-35.0f:35.0f;play.attackDirection=direction;
        uint64_t time=10000;
        auto sample=[&](int phase,float gain){
            play.game.phase=phase;time+=600;
            float raw=(play.fieldSpot+gain*direction)*axis;
            float field=coordinates.observe(play,123,1,time,true,raw);
            return detector.observe(settings,play,123,1,time,true,456,0,9,0,field,true);
        };
        assert(!sample(1,0).id);assert(!sample(3,0).id);
        assert(!sample(3,-4).id);assert(!sample(3,7).id);assert(!sample(3,24.9f).id);
        auto genuine=sample(3,25);assert(genuine.event==live::Event::LongRun&&genuine.yards==25);
        assert(!sample(3,30).id);
    }
    live::RunnerCoordinates coordinates;energy::Play position;
    position.game={1,0,0,1,700,720};position.offense=0;position.down=1;position.fieldSpot=-35;
    assert(coordinates.observe(position,123,1,1000,true,35)==-35);
    position.game.phase=3;assert(coordinates.observe(position,123,1,1100,true,28)==-28);
    position.game.phase=4;coordinates.observe(position,123,1,1200,true,28);
    position.fieldSpot=0;position.game.phase=1;
    assert(coordinates.observe(position,123,1,1300,true,0)==0); // Verified same-quarter midfield axis.
    position.game.phase=3;assert(coordinates.observe(position,123,1,1400,true,-25)==25);
    position.offense=1;assert(std::isnan(coordinates.observe(position,123,1,1500,true,-25)));
    position.game.phase=1;position.offense=0;position.game.quarter=2;
    assert(std::isnan(coordinates.observe(position,123,1,1600,true,0))); // No cross-quarter assumption.
    position.fieldSpot=-35;assert(coordinates.observe(position,123,1,1700,true,-35)==-35);
    position.game.phase=3;assert(std::isnan(coordinates.observe(position,123,1,2600,true,-10))); // Stale feed.
    position.game.phase=1;position.fieldSpot=0;
    assert(std::isnan(coordinates.observe(position,456,2,2700,true,0))); // No cross-game axis.
    position.fieldSpot=-35;assert(std::isnan(coordinates.observe(position,456,2,2800,true,0))); // Not at LOS.
    assert(std::isnan(coordinates.observe(position,456,2,2900,false,35)));
    assert(std::isnan(coordinates.observe(position,456,2,3000,true,NAN)));
    // New default profile: exact 20-yard crossings and critical fourth downs
    // celebrate at the line to gain, without waiting for a completed result.
    for(int direction:{-1,1})for(int side:{0,1}){
        live::Settings defaults;assert(defaults.minimumCatchYards==20&&defaults.minimumGainYards==15);defaults.minimumGainYards=20; // Preserve explicit advanced threshold coverage below.
        defaults.minimumGainYards=20;live::Model detector;energy::Play play;play.game={4,20,20,1,120,720};play.game.neutral=true;
        play.offense=side;play.down=4;play.toFirst=2;play.fieldSpot=direction==1?-20.0f:20.0f;play.attackDirection=direction;
        uint64_t time=10000;bool go=true;
        auto read=[&](int phase,int kind,int ball,float gain,bool held){play.game.phase=phase;time+=16;float spot=play.fieldSpot+gain*direction;return detector.observe(defaults,play,123,1,time,true,456,kind,ball,spot,spot,held,go);};
        read(1,0,1,0,true);read(3,0,9,0,true);assert(!read(3,0,9,1.99f,true).id);
        auto conversion=read(3,0,9,2,true);assert(conversion.event==live::Event::FourthConversion&&conversion.yards==2);
        assert(!read(3,0,9,30,true).id); // One reaction, not a second long-run cheer.
        read(1,0,1,0,true);read(3,0,2,0,true);read(3,3,1,0,false);
        assert(read(4,1,4,2,true).event==live::Event::FourthConversion); // Immediate tackle at catch.
        go=false;read(1,0,1,0,true);read(3,0,9,0,true);assert(!read(3,0,9,2,true).id);
        for(float gain:{4.f,6.f,8.f,10.f,12.f,14.f,16.f,18.f})assert(!read(3,0,9,gain,true).id);assert(!read(3,0,9,19.99f,true).id);assert(read(3,0,9,20,true).event==live::Event::LongRun);
        play.down=1;read(1,0,1,0,true);read(3,0,2,0,true);read(3,3,1,0,false);
        auto caught=read(4,1,4,20,true);assert(caught.event==live::Event::LongCatch&&caught.yards==20);
        read(1,0,1,0,true);read(3,0,2,0,true);read(3,3,1,0,false);assert(!read(3,1,4,2,true).id);
        for(float gain:{4.f,6.f,8.f,10.f,12.f,14.f,16.f,18.f})assert(!read(3,1,4,gain,true).id);assert(!read(3,1,4,19.99f,true).id);assert(read(3,1,4,20,true).event==live::Event::RunAfterCatch);
    }
    std::cout<<"PASS: 20-yard defaults and exact crossings, short fourth-down conversions at catch/live line, one reaction per play, mirrored coordinates, interceptions, immediate tackles and critical-down/special-team exclusions.\n";
}
