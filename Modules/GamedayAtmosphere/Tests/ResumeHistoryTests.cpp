#include "../Source/AttendanceTimeline.h"
#include "../Source/ResumeBridge.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace attendance;
State sample(){
    State s{1,0,0,5,495,720};s.team0=1161;s.team1=1206;s.rank0=51;s.rank1=125;s.night=true;
    s.startingAttendance=true;s.plannedKickoff=64;s.plannedFill=50;s.plannedPeak=s.plannedCapacity=75005;
    s.plannedStadium=561;s.plannedWeek=1;s.plannedYear=0;return s;
}
int main(int argc,char** argv){
    // Minimized 1.87 capture: real gameplay ended at 498.098s with 11% still
    // arriving. Loading created exact-clock 495s records with 20%, then 16%.
    std::stringstream captured(R"(DGA-HISTORY-4 3
1 1161 1206 720 0 127 51 125 0 0 0 4 2053304175 0 1 1 64 50 75005 75005 561 1 0 0
0 1 720 0 0 0 36 0 0 0 0 255 0 -1 0 -1
62.3333296 1 687.837 0 0 0 35 0 0 0 0 255 0 -1 0 -1
67.3666627 1 682.8049 0 0 0 35 0 0 0 0 255 0 -1 0 -1
145.0333247 1 498.09845 0 0 0 11 0 0 0 0 255 0 -1 0 -1
2 1161 1206 720 0 127 51 125 0 0 0 2 3442667709 0 1 1 64 50 75005 75005 561 1 0 0
0 1 720 0 0 0 36 0 0 0 0 255 0 -1 0 -1
2.6999998 1 495 0 0 0 20 0 0 0 0 255 0 -1 0 -1
3 1161 1206 720 0 127 51 125 0 0 0 1 3442667709 0 1 1 64 50 75005 75005 561 1 0 0
3.6999998 1 495 0 0 0 16 0 0 0 0 255 0 -1 0 -1
)");
    timeline::Archive archive;assert(timeline::read(captured,archive));
    assert(archive.games[0].resumeReady&&!archive.games[1].resumeReady&&!archive.games[2].resumeReady);
    State s=sample();timeline::Point found;assert(archive.resume(s,found)&&found.temporary==11);
    auto restored=timeline::estimate(Settings{},arrivals::Settings{},s,&found);
    assert(restored.temporary==11&&restored.arrivals.missing==11&&restored.arrivals.startMissing==36);
    auto held=restored.arrivals.observe(arrivals::Settings{},s);assert(!held.changed&&held.missing==11);
    held=restored.arrivals.observe(arrivals::Settings{},s);assert(!held.changed&&held.missing==11);
    s.seconds=480;restored.arrivals.observe(arrivals::Settings{},s);assert(restored.arrivals.missing<=11);
    s.seconds=360;restored.arrivals.observe(arrivals::Settings{},s);assert(restored.arrivals.done&&!restored.arrivals.missing);
    s=sample();s.seconds=490;assert(!archive.resume(s,found)); // Outside the five-second sampling allowance.
    s=sample();s.awayScore=7;assert(!archive.resume(s,found));
    s=sample();s.team1=1130;assert(!archive.resume(s,found));
    s=sample();s.plannedStadium=562;assert(!archive.resume(s,found));
    s=sample();s.plannedWeek=2;assert(!archive.resume(s,found));
    s=sample();s.plannedYear=2026;assert(!archive.resume(s,found));
    s=sample();s.quarterSeconds=900;assert(!archive.resume(s,found));

    timeline::Archive live;s=sample();live.begin(10,s);timeline::Point p;p.state=s;p.time=0;p.temporary=20;
    assert(live.record(p));assert(!live.resume(s,found));
    p.time=6;p.state.seconds=490;assert(live.record(p));assert(!live.resume(s,found)); // Loading/lineup clocks do not confirm a play.
    p.time=5;p.state.phase=3;assert(!live.record(p)&&!live.games.back().resumeReady); // Rewound recording samples cannot confirm history.
    p.time=7;p.state.phase=3;p.state.seconds=489;live.record(p);assert(live.games.back().resumeReady);
    // Pausing within the normal five-second interval records the exact visible
    // crowd and clock once, without creating a stream of duplicate pause points.
    p.time=8;p.state.phase=5;p.state.seconds=488;p.temporary=11;assert(live.record(p,true));
    auto count=live.games.back().points.size();p.time=9;assert(!live.record(p,true)&&live.games.back().points.size()==count);
    assert(live.resume(p.state,found)&&found.temporary==11&&found.state.seconds==488);
    p.time=10;p.temporary=10;assert(live.record(p,true));assert(live.resume(p.state,found)&&found.temporary==10); // Latest equal-clock crowd wins.
    s.seconds=488;live.begin(11,s);p.state=s;p.time=0;p.temporary=20;assert(live.record(p));
    assert(live.resume(s,found)&&found.temporary==10); // Current provisional session cannot select itself.
    live.begin(12,s,true);p.time=0;p.temporary=11;assert(live.record(p));assert(live.resume(s,found)&&found.temporary==11);
    std::stringstream disk;timeline::write(disk,live);timeline::Archive roundtrip;assert(timeline::read(disk,roundtrip));
    assert(roundtrip.games[0].resumeReady&&!roundtrip.games[1].resumeReady&&roundtrip.games[2].resumeReady);
    assert(roundtrip.resume(s,found)&&found.temporary==11);
    auto invalid=disk.str();auto row=invalid.find('\n',invalid.find('\n')+1);auto ready=row;for(int n=0;n<4;++n)ready=invalid.rfind(' ',ready-1);invalid.replace(ready-1,1,"2");
    std::stringstream invalidReady(invalid);assert(!timeline::read(invalidReady,roundtrip));
    if(argc>1){
        std::ifstream actual(argv[1]);timeline::Archive actualArchive;assert(actual&&timeline::read(actual,actualArchive));
        assert(actualArchive.resume(sample(),found)&&found.temporary==11);
        std::cout<<"Captured archive restored "<<(100-found.total())<<"% at source clock "<<found.state.seconds<<"s\n";
    }
    // Follow the whole fresh-process path, not just the archive matcher: game
    // zero must trigger one restoration before the saved arrival plan is used.
    const std::string zeroResume=R"({"product":"midgamesave","source":"dll","pid":29900,"game":{"active":true,"number":0,"ended":false,"live":{"quarter":1,"clock":495,"period":720,"home_score":0,"away_score":0}},"arm":{"state":"applied","applied_game":0}})";
    auto resumedSignal=resume::parse(zeroResume,29900,1000);resume::Tracker bridge;
    assert(bridge.observe(resumedSignal,sample(),1000,true)&&!bridge.observe(resumedSignal,sample(),1001,true));
    assert(archive.resume(sample(),found)&&found.temporary==11);
    auto boot=timeline::estimate(Settings{},arrivals::Settings{},sample(),&found);
    assert(boot.temporary==11&&!boot.arrivals.observe(arrivals::Settings{},sample()).changed);
    if(argc>2){
        assert(argc==4);std::ifstream status(argv[2]);assert(status);
        std::string json((std::istreambuf_iterator<char>(status)),std::istreambuf_iterator<char>());
        auto actualSignal=resume::parse(json,std::stoi(argv[3]),1000);resume::Tracker actualBridge;
        assert(actualSignal.valid&&actualSignal.applied&&actualSignal.game==0&&actualBridge.observe(actualSignal,sample(),1000,true));
        std::cout<<"Captured game-0 Midgame Save signal triggers restoration exactly once\n";
    }
    std::cout<<"PASS: real Q1 reload regression, provisional-source rejection, bounded clock matching, identity guards, pause checkpoint, latest tied clock, readiness persistence and legacy import\n";
}
