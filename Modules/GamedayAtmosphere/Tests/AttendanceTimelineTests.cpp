#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include "../Source/AttendanceTimeline.h"
#include "../Source/CrowdVisibility.h"
#include "../Source/CoreAtmosphere.h"
using namespace attendance;
State game(int q,float seconds,int home=0,int away=0,float length=720){
    State s;s.quarter=q;s.seconds=seconds;s.quarterSeconds=length;s.homeScore=home;s.awayScore=away;s.phase=5;s.team0=1178;s.team1=1183;s.rank0=1;s.rank1=9;return s;
}
int main(){
    // Replay ownership must survive normal viewing but must never carry a
    // renderer restoration into the hub, a new match or uninitialized context.
    assert(timeline::ownsReplayCrowd(true,0x1000,0x1000,true));
    assert(!timeline::ownsReplayCrowd(false,0x1000,0x1000,true));
    assert(!timeline::ownsReplayCrowd(true,0x2000,0x1000,true));
    assert(!timeline::ownsReplayCrowd(true,0x1000,0x1000,false));
    assert(!timeline::ownsReplayCrowd(true,0,0,true));
    assert(!timeline::ownsReplayCrowd(true,0x1000,0x1000,true,false)); // Reused object, different session.
    auto earlier=game(1,700),later=game(4,50,21,28);
    assert(timeline::recordingRestarted(true,10,100,earlier,later));
    assert(!timeline::recordingRestarted(false,10,100,earlier,later));
    assert(!timeline::recordingRestarted(true,10,-1,earlier,later));
    assert(!timeline::recordingRestarted(true,100,100,earlier,later)); // A playback seek does not rewind recording time.
    assert(!timeline::recordingRestarted(true,10,100,later,earlier));
    double selected=-1;
    assert(!timeline::recordingTime(true,false,0,700,selected)&&selected==-1);
    assert(timeline::recordingTime(true,true,500,700,selected)&&selected==500);
    assert(timeline::recordingTime(false,false,0,700,selected)&&selected==700);
    assert(!timeline::recordingTime(true,true,NAN,700,selected));
    assert(!timeline::recordingTime(true,true,-1,700,selected));
    assert(!timeline::recordingTime(true,true,86401,700,selected));
    for(bool active:{false,true})for(bool paused:{false,true})for(bool menu:{false,true}){
        auto policy=timeline::replayPolicy(active,paused,menu);
        assert(policy.historical==active);assert(policy.suspendAtmosphere==(active&&(paused||menu)));
    }
    timeline::Archive a;auto context=game(1,720);a.begin(1,context,true);
    timeline::Point p;p.state=context;p.time=0;p.temporary=25;assert(a.record(p));
    p.time=100;p.temporary=0;assert(a.record(p));
    p.state=game(3,720,14,35);p.time=1000;p.loss=25;p.periodStart=10;assert(a.record(p));
    p.state=game(4,100,21,42);p.time=2000;p.loss=45;p.periodStart=30;assert(a.record(p));
    timeline::Point found;assert(a.replay(50,found)&&found.total()==25);assert(a.replay(101,found)&&found.total()==0);
    // A missing native manager must skip a snapshot, rather than placing a
    // game-clock sample ahead of the next real recording-time sample.
    timeline::Archive clocks;clocks.begin(2,context);timeline::Point tick;tick.state=context;
    for(auto sample:{std::pair<bool,double>{true,500},{false,0},{true,550}}){
        if(timeline::recordingTime(true,sample.first,sample.second,700,tick.time)){tick.loss=sample.second==500?10:20;assert(clocks.record(tick));}
    }
    assert(clocks.games.back().points.size()==2&&clocks.replay(525,found)&&found.loss==10);
    assert(clocks.replay(551,found)&&found.loss==20);
    assert(a.replay(1500,found)&&found.loss==25);assert(a.replay(2100,found)&&found.loss==45);
    assert(a.replay(10,found)&&found.loss==0&&found.temporary==25); // Seek back without changing live loss.
    assert(a.games.back().points.back().loss==45);
    // An automatic halftime/postgame montage jumps among the same absolute
    // recording times as the viewer. Keep live attendance and its winning roar.
    core::Model celebration;core::Settings audio;energy::Play final;
    final.game=game(4,0,24,21);final.game.phase=4;
    energy::Outcome win;win.accepted=true;win.event=energy::Event::LastSecondLead;win.sequence=7;
    assert(celebration.react(audio,win,0,final.game));celebration.update(audio,final,0,.65f,.65f);
    final.game.quarter=6;final.game.phase=7;celebration.update(audio,final,1,.65f,.65f);
    assert(celebration.postgameHeld);int liveLoss=45;
    for(auto sample:{std::pair<double,int>{50,25},{1500,25},{101,0},{2100,45},{10,25}}){
        auto policy=timeline::replayPolicy(true,false,false);assert(policy.historical&&!policy.suspendAtmosphere);
        assert(a.replay(sample.first,found)&&found.total()==sample.second);
        celebration.update(audio,final,80,.65f,.65f);
    assert(celebration.postgameHeld&&celebration.celebration==1&&std::fabs(volume::boostPercent(celebration.parent)-50*std::sqrt(.65f))<.001f);
        assert(liveLoss==45&&a.games.back().points.back().loss==liveLoss);
    }
    assert(!timeline::replayPolicy(false,false,false).historical);
    assert(celebration.finishPostgame()); // The real menu, not broadcast clips, ends it.
    assert(a.resume(game(4,100,21,42),found)&&found.loss==45);
    assert(!a.resume(game(4,110,21,42),found));assert(!a.resume(game(4,100,21,41),found));
    auto other=game(4,100,21,42);other.team1=1503;assert(!a.resume(other,found));
    other=game(4,100,21,42,300);assert(!a.resume(other,found));
    std::stringstream disk;timeline::write(disk,a);timeline::Archive loaded;assert(timeline::read(disk,loaded));
    assert(loaded.resume(game(4,100,21,42),found)&&found.loss==45&&found.periodStart==30);
    assert(!loaded.replay(100,found)); // Disk history cannot impersonate the current native replay recording.
    auto corrupted=a;corrupted.games.front().points.front().time=-0.5;
    std::stringstream negative;timeline::write(negative,corrupted);assert(!timeline::read(negative,loaded)&&loaded.games.size()==1);
    for(const char* bad:{"bad","DGA-HISTORY-1 9999","DGA-HISTORY-1 1\n1 1178 1183 720 0 127 999999","DGA-HISTORY-1 0 trailing"}){
        std::stringstream f(bad);assert(!timeline::read(f,loaded));assert(loaded.games.size()==1);
    }
    // Conference classification survives disk saves and cannot match a CFP
    // or ordinary game with otherwise identical teams, score and clock.
    auto ccArchive=a;for(auto& g:ccArchive.games){postseason(g.context,false,true);for(auto& point:g.points)postseason(point.state,false,true);}
    std::stringstream ccDisk;timeline::write(ccDisk,ccArchive);timeline::Archive restoredConference;assert(timeline::read(ccDisk,restoredConference));
    auto ccGame=game(4,100,21,42);postseason(ccGame,false,true);assert(restoredConference.resume(ccGame,found));ccGame.conferenceChampionship=false;assert(!restoredConference.resume(ccGame,found));
    // Previous release archives are still readable; they had no conference flag.
    std::stringstream oldWrite;timeline::write(oldWrite,a);std::string magic,row;std::getline(oldWrite,magic);std::getline(oldWrite,row);
    magic.replace(magic.find("DGA-HISTORY-8"),13,"DGA-HISTORY-2");for(int n=0;n<4;++n)row.erase(row.rfind(' '));for(int field=0;field<4;++field)row.erase(row.rfind(' '));
    std::string legacyPoints,line;while(std::getline(oldWrite,line)){if(line.empty())continue;for(int extra=0;extra<4;++extra)line.erase(line.rfind(' '));legacyPoints+=line+"\n";}
    std::stringstream legacy(magic+"\n"+row+"\n"+legacyPoints);timeline::Archive legacyArchive;assert(timeline::read(legacy,legacyArchive)&&!legacyArchive.games.front().context.conferenceChampionship);
    // Native and originally empty seats are never filled by historical overrides.
    for(unsigned agent=0;agent<10000;++agent){assert(historicalPart(true,false,agent,0));assert(historicalPart(false,true,agent,0));assert(!historicalPart(false,false,agent,0));assert(historicalPart(false,false,agent,100));}
    Settings c;arrivals::Settings arrivals;
    auto close=game(4,47.53f,31,33,300);auto seed=timeline::estimate(c,arrivals,close);
    assert(seed.loss<15&&seed.loss>0);assert(seed.arrivals.done&&seed.temporary==0);assert(!seed.history.comeback);assert(seed.history.warningDone);
    int previous=seed.loss;
    close.seconds=40;seed.history.observe(c,close);assert(seed.departures.observe(c,close,seed.history));assert(seed.departures.settle(close,previous).loss>=previous);
    // Restore a saved history's cap budget and comeback protection, rather than inventing it.
    p.state=game(4,100,21,42);p.loss=45;p.periodStart=30;p.temporary=0;p.history.seen=true;p.history.comeback=true;p.history.maxDeficit=35;
    seed=timeline::estimate(c,arrivals,p.state,&p);assert(seed.loss==45&&seed.temporary==0&&seed.departures.periodStartLoss==30&&seed.history.comeback);
    assert(seed.departures.settle(p.state,45,25,45).loss<=75);
    // Quarter lengths scale the estimate and arrival curve consistently.
    for(int minutes=1;minutes<=20;++minutes){
        float length=minutes*60.0f;
        auto start=game(1,length*0.9f,7,0,length);auto fill=timeline::estimate(c,arrivals,start);
        assert(fill.arrivals.active&&fill.arrivals.missing>0&&fill.arrivals.missing<fill.arrivals.startMissing);
        auto blowout=game(4,length*0.1f,0,42,length);auto dry=timeline::estimate(c,arrivals,blowout);
        assert(dry.loss>=20&&dry.loss<=75);assert(dry.departures.committedLoss==dry.loss);
        blowout.playoff=true;auto playoff=timeline::estimate(c,arrivals,blowout);assert(playoff.loss<=40);
        blowout.bowl=47;auto title=timeline::estimate(c,arrivals,blowout);assert(title.loss<=20);
    }
    // Bounded history retains opening samples and remains monotonic after compaction.
    timeline::Archive bounded;bounded.begin(2,context);p={};p.state=context;
    for(int i=0;i<20000;++i){p.time=i*3.0;p.loss=(i/500)%76;bounded.record(p);}
    assert(bounded.games.back().points.size()<=timeline::Archive::MaxPoints&&bounded.games.back().points.front().time==0);
    for(uint64_t i=3;i<30;++i)bounded.begin(i,context);assert(bounded.games.size()==timeline::Archive::MaxGames);
    std::cout<<"PASS: replay ownership, recording-clock isolation, historical seeks, native seats, resume estimates, saved caps, quarter scaling and bounded history validation\n";
}
