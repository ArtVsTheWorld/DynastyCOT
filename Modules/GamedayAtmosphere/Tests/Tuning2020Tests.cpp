#include "../Source/StartingAttendance.h"
#include "../Source/ArrivalRules.h"
#include "../Source/AttendanceTimeline.h"
#include "../Source/BackgroundRoar.h"
#include "../Source/MotionSoundPairing.h"
#include "../Source/ReactionPalette.h"
#include <cassert>
#include <sstream>
#include <cstdio>
using namespace attendance;
int main(){
    turnout::Seed seed;seed.home.id=17;seed.away.id=23;seed.home.rank=seed.away.rank=130;
    seed.home.atmosphere=seed.home.tradition=seed.home.brand=seed.home.conference=12;
    seed.home.standing=0;seed.schools=138;seed.stadium=seed.homeStadium=1;
    seed.capacity=50000;seed.ea=15000;seed.regular=true;seed.week=3;seed.year=2026;
    turnout::Launch launch{17,23,1,50000,15000,1200,3,true,false};
    for(int event:{4,6,14,27,40,53,54,58,62,73,81,82,92,95,100,101,116,118,119,121,125,136,139,143})for(int week=0;week<16;++week){
        seed.week=week;launch.rivalryEvent=event;auto r=turnout::calculate(seed,launch);
        auto policy=showcase::event(event);assert(r.applied&&r.showcaseEvent==event&&r.percent==100&&r.attendance==50000);
        assert(r.kickoff>=policy.low&&r.kickoff<=policy.high&&r.fillPercent==30);
        // Completely different team slots still use the same event rule.
        auto other=seed;other.home.id=1900;other.away.id=1901;auto l=launch;l.home=1900;l.away=1901;
        assert(turnout::calculate(other,l).kickoff==r.kickoff);
        other.home.audienceFans=1000;other.away.audienceFans=2000;
        auto absolute=turnout::calculateAudience(other,l);assert(absolute.applied&&absolute.audienceModel&&absolute.attendance==50000&&absolute.kickoff==r.kickoff);
        State state{1,0,0,1,720,720};state.team0=17;state.team1=23;
        state.startingAttendance=true;state.exceptionalOpening=true;state.plannedKickoff=r.kickoff;
        state.plannedPeak=state.plannedCapacity=50000;state.plannedFill=30;state.plannedStadium=1;state.plannedWeek=week;state.plannedYear=2026;
        assert(crowdPlan(state)&&arrivals::openingPercent({},state)==r.kickoff);
        timeline::Archive archive;timeline::Game game;game.id=1;game.context=state;archive.games.push_back(game);
        std::stringstream history;timeline::write(history,archive);timeline::Archive restored;assert(timeline::read(history,restored));
        assert(restored.games[0].context.plannedKickoff==r.kickoff&&restored.games[0].context.exceptionalOpening);
    }
    launch.rivalryEvent=1023;assert(turnout::calculate(seed,launch).showcaseEvent==-1);
    launch.rivalryEvent=0;assert(turnout::calculate(seed,launch).showcaseEvent==-1);
    launch.rivalryEvent=119;launch.rivalry=false;assert(turnout::calculate(seed,launch).showcaseEvent==-1);launch.rivalry=true;
    seed.resumed=true;assert(!turnout::calculate(seed,launch).applied);seed.resumed=false;
    seed.regular=false;assert(!turnout::calculate(seed,launch).applied);seed.regular=true;
    State night{3,42,9,5,360,720};night.night=true;Settings departure;History story;
    assert(departures::nightHomeWinTarget(departure,night,story,1800)==0);
    float last=0;for(int second=1801;second<=2880;++second){
        night.quarter=second<2160?3:4;night.seconds=night.quarter*720.f-second;
        float target=static_cast<float>(departures::nightHomeWinTarget(departure,night,story,static_cast<float>(second)));
        assert(target>=last&&std::isfinite(target));last=target;
    }
    assert(100*std::exp(-last)>=30&&100*std::exp(-last)<=40);
    for(int protection=0;protection<4;++protection){auto s=night;if(protection==0)s.neutral=true;if(protection==1)s.playoff=true;if(protection==2)s.night=false;if(protection==3)s.homeScore=29;
        assert(departures::nightHomeWinTarget(departure,s,story,2880)==0);}
    sustained::Intent quiet;quiet.active=true;quiet.match=1;quiet.epoch=1;quiet.updated=1000;
    State game{2,7,7,3,300,720};
    for(int percent=0;percent<=100;++percent){auto roar=background::merge(quiet,game,61888,percent*.01f);
        assert(!roar.liveCheer&&!roar.resultCheer&&!roar.shake&&!roar.supporting);
        if(percent<=60)assert(roar.reason==sustained::Reason::None);else assert(roar.reason==sustained::Reason::Background&&roar.continuous&&!roar.cheers);}
    auto earned=quiet;earned.reason=sustained::Reason::Result;earned.liveCheer=8;assert(background::merge(earned,game,61888,1).reason==sustained::Reason::Result);
    float priorBackground=background::merge(quiet,game,61888,1).intensity;
    for(int deficit=1;deficit<=50;++deficit){game.awayScore=game.homeScore+deficit;
        float level=background::merge(quiet,game,61888,1).intensity;
        assert(level<=priorBackground&&level>0);priorBackground=level;
        auto reaction=background::merge(earned,game,61888,1);assert(reaction.liveCheer==8&&reaction.intensity==earned.intensity);
    }
    game.neutral=true;assert(background::merge(quiet,game,61888,1).intensity>priorBackground);game.neutral=false;
    assert(background::merge(quiet,game,6060,1).reason==sustained::Reason::None);
    quiet.active=false;assert(!background::merge(quiet,game,61888,1).active);
    motion::Model camera;motion::Settings settings;motion::Input x;x.match=1;x.allowed=x.duringAllowed=true;x.phase=1;x.now=1000;
    x.lineup=x.venueEligible=x.openingKickoff=true;x.lineupId=1;camera.observe(settings,x);x.now=2000;camera.observe(settings,x);
    x.lineup=false;x.phase=3;x.now=2100;x.runBuildup=7;x.runProgress=1;camera.observe(settings,x);
    x.now=2500;auto gain=camera.observe(settings,x);assert(gain.additiveScale>1&&gain.additiveScale<=1.15f);
    auto first=gain.began;float ceiling=motion::strength(gain,x.now);
    x.duringPlay=1;x.supplementalImpact=x.duringImpact=true;x.profile=motion::Profile::Truck;x.now=2600;camera.observe(settings,x);
    x.duringPlay=0;x.now=2800;auto truck=camera.observe(settings,x);assert(truck.additiveScale>1&&truck.additiveScale<=1.15f);
    assert(motion::strength(truck,x.now)<=1.15f*11&&motion::strength(truck,x.now)>ceiling);
    x.supplementalImpact=x.duringImpact=false;x.profile=motion::Profile::Default;
    for(x.now=2900;x.now<3100;x.now+=10){auto c=camera.observe(settings,x);assert(c.additiveScale<=1.15f&&motion::strength(c,x.now)<=1.15f*11);}
    x.phase=4;x.runBuildup=0;x.now=3100;auto tail=camera.observe(settings,x);auto deadline=tail.until;
    x.phase=2;x.now=3200;tail=camera.observe(settings,x);assert(tail.until==deadline&&tail.began>=first);
    pairing::SoundLease lease{1,0,3200,false,3150};assert(pairing::soundWeight(lease,tail,3600,true)==0);
    lease.updated=2000;assert(pairing::soundWeight(lease,tail,3600,true)==0);
    std::array<float,16> projection{};projection[0]=2;projection[5]=3.1715948f*2;projection[11]=1;
    assert(std::abs(motion::projectionScale(projection,motion::Kind::Touchdown)-.5f)<.0001f);
    energy::Play play;play.game=game;play.down=1;
    assert(palette::liveType({live::Event::Truck,1,0,0},play,game,1,1)==11);
    for(auto skill:{live::Event::Hurdle,live::Event::StiffArm,live::Event::Juke,live::Event::Spin})assert(palette::liveType({skill,1,0,0},play,game,1,1)==3);
    std::puts("PASS: event-based showcase attendance independent of team slots, kickoff/history through95%; night-home-blowout target; background priority and empty/small exclusions; bounded independent shake overlap and presentation clock; closeup softening; strongest skill families.");
}
