#include "../Source/VisitorPlans.h"
#include "../Source/AttendanceTimeline.h"
#include <cassert>
#include <sstream>
#include <cstdio>
using namespace attendance::visitors;
int main(){
    FixtureKey fixture{-1649480587,1153,1193,7,32,553,8,0};assert(fixture.valid());
    PlanBook book;Plan current;current.valid=true;current.share=.07f;current.travelKnown=true;current.distance=900;
    Plan selected;unsigned seed=0;bool reused=false;
    assert(book.resolve(fixture,current,12345,selected,seed,reused)&&!reused&&seed==12345&&selected.share==.07f&&book.revision==1);
    // A fresh process has no audience model inputs yet. It can still restore
    // the exact fixture's palette plan and deterministic position assignments.
    std::stringstream disk;book.write(disk);PlanBook loaded;assert(PlanBook::read(disk,loaded));
    assert(loaded.resolve(fixture,{},999,selected,seed,reused)&&reused&&seed==12345&&selected.valid&&selected.share==.07f);
    assert(loaded.revision==0); // Reusing a saved fixture causes no disk rewrite.
    auto takeoverFixture=fixture;++takeoverFixture.week;auto takeover=current;takeover.share=.70f;
    PlanBook takeoverBook;assert(takeoverBook.resolve(takeoverFixture,takeover,99,selected,seed,reused));
    std::stringstream takeoverDisk;takeoverBook.write(takeoverDisk);PlanBook takeoverLoad;
    assert(PlanBook::read(takeoverDisk,takeoverLoad)&&takeoverLoad.entries.front().share==.70f);
    assert(loaded.resolve(fixture,{},0,selected,seed,reused)&&selected.share==.07f);
    for(int i=0;i<8;++i){auto other=fixture;
        if(i==0)other.dynasty=998;if(i==1)++other.home;if(i==2)++other.away;if(i==3)++other.homeRow;
        if(i==4)++other.awayRow;if(i==5)++other.stadium;if(i==6)++other.week;if(i==7)++other.year;
        assert(other.valid()&&!loaded.resolve(other,{},1,selected,seed,reused));
    }
    auto bad=fixture;bad.dynasty=0;assert(!book.resolve(bad,current,1,selected,seed,reused));
    bad=fixture;bad.homeRow=-1;assert(!book.resolve(bad,current,1,selected,seed,reused));
    auto neutral=current;neutral.native=true;assert(!loaded.resolve(fixture,neutral,1,selected,seed,reused));
    // Saved plans reproduce the same side bits while native band/cadet tags
    // remain unchanged, even with a different current fanbase estimate.
    std::vector<attendance::seats::Position> positions(1024);std::vector<unsigned char> raw(10240);
    for(unsigned i=0;i<1024;++i){positions[i]={35.f,static_cast<float>(i/32),static_cast<float>(i%128),0};raw[10*i+6]=i<16?static_cast<unsigned char>(1+i%8):i<96?9:0;}
    auto tags=attendance::agents::decode(raw,1024);attendance::agents::protectStudents(tags,true,false);
    auto before=assign(positions,tags,current.share,12345),after=assign(positions,tags,selected.share,seed);assert(before==after);
    for(unsigned i=0;i<1024;++i)if(tags.protectedGroup(i))assert(after[i]==tags.side[i]);
    PlanBook bounded;for(int i=0;i<300;++i){auto f=fixture;f.year=i;assert(bounded.resolve(f,current,static_cast<unsigned>(i),selected,seed,reused));}
    assert(bounded.entries.size()==PlanBook::Maximum&&!bounded.resolve(fixture,{},1,selected,seed,reused));
    // Neutral crowds and disabled outfit changes still need save ownership.
    PlanBook ownership;assert(ownership.bind(fixture,100)&&ownership.source(fixture)==100);
    assert(!ownership.resolve(fixture,neutral,1,selected,seed,reused));
    assert(!linkedHistoryReady(ownership,{100},{}));assert(linkedHistoryReady(ownership,{100},{100}));assert(linkedHistoryReady(ownership,{},{}));
    attendance::State state{2,7,0,5,120,720};state.team0=1153;state.team1=1193;
    attendance::timeline::Archive history;attendance::timeline::Point point;point.state=state;point.loss=20;point.time=10;
    history.begin(100,state,true);assert(history.record(point));
    point.loss=60;history.begin(200,state,true);assert(history.record(point));
    attendance::timeline::Point restored;
    assert(history.resume(state,restored,ownership.source(fixture))&&restored.loss==20);
    auto refreshed=state;refreshed.rank0=18;refreshed.precipitation=1;
    assert(history.resume(refreshed,restored,100)&&restored.loss==20);assert(!history.resume(refreshed,restored));
    auto otherDynasty=fixture;++otherDynasty.dynasty;assert(ownership.source(otherDynasty)==0);
    assert(!history.resume(state,restored,999));
    std::stringstream ownedDisk;ownership.write(ownedDisk);PlanBook ownedLoad;assert(PlanBook::read(ownedDisk,ownedLoad)&&ownedLoad.source(fixture)==100&&ownedLoad.entries.front().share==-1);
    assert(ownedLoad.resolve(fixture,current,12345,selected,seed,reused)&&!reused&&ownedLoad.source(fixture)==100);
    auto unchangedRevision=ownedLoad.revision;assert(ownedLoad.bind(fixture,100)&&ownedLoad.revision==unchangedRevision);
    assert(ownedLoad.bind(fixture,200)&&ownedLoad.revision==unchangedRevision+1);
    std::stringstream legacy("DGA-VISITORS-1 1\n-1649480587 1153 1193 7 32 553 8 0 .07 12345\n");PlanBook legacyBook;
    assert(PlanBook::read(legacy,legacyBook)&&legacyBook.entries.size()==1&&legacyBook.source(fixture)==0);
    for(const char* text:{"bad","DGA-VISITORS-1 257","DGA-VISITORS-1 0 trailing","DGA-VISITORS-3 0",
        "DGA-VISITORS-1 1\n0 1153 1193 7 32 553 8 0 .07 1",
        "DGA-VISITORS-1 1\n1 1153 1193 7 32 553 8 0 .701 1",
        "DGA-VISITORS-1 1\n1 1153 1193 7 32 553 8 0 .07 4294967296",
        "DGA-VISITORS-1 2\n1 1153 1193 7 32 553 8 0 .07 1\n1 1153 1193 7 32 553 8 0 .08 2"}){
        std::stringstream input(text);assert(!PlanBook::read(input,loaded)&&loaded.entries.size()==1);
    }
    std::puts("PASS: exact dynasty/program-row/year/week/stadium ownership, signed native identity, saved seed/outfit repeatability, native neutral and protected-group isolation, bounded persistence, no read-induced writes and corrupt/duplicate/truncated rejection");
}
