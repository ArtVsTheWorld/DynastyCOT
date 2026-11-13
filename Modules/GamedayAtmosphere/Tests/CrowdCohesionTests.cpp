#include "../Source/AttendanceTimeline.h"
#include "../Source/OpeningWelcome.h"
#include "../Source/PresentationAtmosphere.h"
#include <cassert>
#include <sstream>
#include <cstdio>
using namespace attendance;
State sample(){State s{1,0,0,5,720,720};s.team0=1153;s.team1=1193;s.rank0=50;s.rank1=60;s.startingAttendance=true;s.plannedKickoff=60;s.plannedFill=50;s.plannedPeak=60000;s.plannedCapacity=60000;s.plannedStadium=553;s.plannedWeek=8;s.plannedYear=2026;return s;}
int main(){
    using turnout::openingPercent;
    assert(openingPercent(0,1153,100,110,false,false,false,false,0,false)==50);
    assert(openingPercent(1,1153,50,60,false,true,false,false,0,false)==65);
    assert(openingPercent(1,1218,1,2,false,false,true,true,0,false,55,45,3,100)<70);
    assert(openingPercent(1,1153,1,2,false,true,true,true,0,false,55,45,3,100)==80);
    assert(openingPercent(1,1153,1,2,false,true,true,true,0,false,55,45,3,94)<=70);
    assert(openingPercent(1,1153,1,2,false,true,true,true,0,false,55,45,0,100)<=70);
    for(float demand:{0.f,.5f,.8f,1.f})for(int rain:{0,1,2,3})for(int home:{1153,1218}){
        int n=openingPercent(demand,home,1,2,false,true,true,true,rain,false,55,45,3,100);assert(n>=(rain?35:40)&&n<=80);
    }
    State s=sample();s.plannedKickoff=80;s.exceptionalOpening=true;s.night=true;
    State refreshed=s;refreshed.startingAttendance=false;refreshed.night=false;refreshed.plannedKickoff=-1;
    inheritCrowdPlan(s,refreshed);assert(crowdPlan(refreshed)&&refreshed.plannedKickoff==80&&refreshed.night);
    refreshed.plannedKickoff=60;inheritCrowdPlan(s,refreshed);assert(refreshed.plannedKickoff==60);
    refreshed.startingAttendance=false;refreshed.plannedStadium=554;inheritCrowdPlan(s,refreshed);assert(!crowdPlan(refreshed));
    timeline::Archive original;original.begin(1,s,true);timeline::Point p;p.state=s;p.state.seconds=600;p.time=120;p.temporary=11;assert(original.record(p));
    std::stringstream data;timeline::write(data,original);assert(data.str().find("DGA-HISTORY-8")==0);
    timeline::Archive loaded;assert(timeline::read(data,loaded));State live=p.state;live.startingAttendance=false;live.plannedKickoff=live.plannedFill=-1;timeline::Point saved;assert(loaded.resume(live,saved));assert(crowdPlan(saved.state)&&saved.state.plannedKickoff==80&&saved.state.exceptionalOpening&&saved.state.night);
    auto resumed=timeline::estimate(Settings{},arrivals::Settings{},live,&saved);assert(resumed.arrivals.startMissing==20&&resumed.arrivals.missing==11);auto next=live;next.seconds=590;resumed.arrivals.observe(arrivals::Settings{},next);assert(resumed.arrivals.missing<=11);
    live.plannedStadium=554;assert(!loaded.resume(live,saved));auto invalid=s;invalid.plannedKickoff=96;assert(!crowdPlan(invalid));invalid=s;invalid.plannedPeak=60001;assert(!crowdPlan(invalid));
    State rivalry{2,14,14,5,90,720};rivalry.rank0=34;rivalry.rank1=47;rivalry.rivalry=true;
    auto target=context::halftime(rivalry);assert(target.permanent<=4);arrivals::Halftime half;half.observe(rivalry,0);assert(half.lastAbsent<=4);
    auto blowout=rivalry;blowout.awayScore=49;assert(context::halftime(blowout).permanent>target.permanent);half.reset();half.observe(blowout,0);assert(half.lastAbsent>4);
    core::Settings c;energy::Play play;play.game=sample();play.game.phase=6;openingwelcome::Model welcome;
    auto sound=welcome.observe(c,play,123,1,1000,true,true,.20f);assert(sound.active&&sound.intensity>0&&!sound.shake&&sound.layerBoostPercent==0&&sound.supportingOnly);
    auto base=sound;base.reason=sustained::Reason::None;base.intensity=0;assert(openingwelcome::Model::merge(base,sound).reason==sustained::Reason::Opening);base.reason=sustained::Reason::Result;assert(openingwelcome::Model::merge(base,sound).reason==sustained::Reason::Result);
    play.resumed=true;assert(!welcome.observe(c,play,123,1,1100,true,true,.20f).active);play.resumed=false;play.simulated=true;assert(!welcome.observe(c,play,123,1,1200,true,true,.20f).active);play.simulated=false;
    welcome.reset();play.game.phase=1;play.game.quarter=2;assert(!welcome.observe(c,play,123,1,1000,true,true,.50f).active);
    play.game=sample();play.game.phase=1;play.down=1;play.offense=0;play.toFirst=10;
    assert(lineup::openingMinimum(play.game,12999)>.55f);assert(!lineup::cameraAdmission(c,play,.55f,true,12999,3,false,true).allowed);
    assert(lineup::openingMinimum(play.game,13000)==.55f);assert(lineup::cameraAdmission(c,play,.55f,true,13000,3,false,true).allowed);
    play.game.rivalry=true;assert(lineup::openingMinimum(play.game,20000)==.55f);assert(!lineup::cameraAdmission(c,play,.49f,true,20000,3,false,true).allowed);assert(lineup::cameraAdmission(c,play,.55f,true,20000,3,false,true).allowed);
    play.offense=1;lineup::Model light;light.observe(c,play,123,1,1000,true,.55f,true,nullptr,20000);
    assert(light.openingDriveQualified&&light.playContextActive()&&!light.hostileOpening&&light.strength<=.45f&&light.qualifiedMinimum==.55f);
    play.game.plannedPeak=50000;assert(lineup::openingMinimum(play.game,20000)>.55f);
    std::puts("PASS: exceptional80% gate, severe/ordinary/weather bounds, archived peak/arrival plan and Q1 continuation, venue guard, close-rival halftime patience, sparse opening sound without motion and high-demand opening admission.");
}
