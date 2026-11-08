#include <cassert>
#include <fstream>
#include <iostream>
#include "../Source/ResumeBridge.h"
using namespace attendance::resume;
int main(){
    const std::string text=R"({"product":"midgamesave","source":"dll","pid":9716,"game":{"active":true,"number":3,"ended":false,"live":{"quarter":4,"clock":47.53,"period":300,"home_score":31,"away_score":33}},"arm":{"state":"applied","applied_game":3,"situation":{"period":900}},"ignored":[null,true,{"text":"a\\b\"c\u1234"}]})";
    auto s=parse(text,9716,1000);assert(s.valid&&s.game==3&&s.applied&&s.live.quarterSeconds==300);
    assert(!parse(text,1,1000).valid);assert(!parse(text.substr(0,text.size()-1),9716,1000).valid);
    auto live=s.live;live.phase=1;assert(s.matches(live,2000));assert(!s.matches(live,11001));
    Tracker tracker;assert(tracker.pending(s,live,1000)&&tracker.pending(s,live,1000));
    assert(!tracker.game&&!tracker.applied); // Replay ownership checks must not consume the scenario signal.
    assert(tracker.observe(s,live,1000));assert(!tracker.pending(s,live,1001)&&!tracker.observe(s,live,1001));
    auto stale=s;stale.applied=false;stale.game=4;stale.live.homeScore=0;assert(!tracker.observe(stale,live,1002));
    stale.live=live;assert(tracker.observe(stale,live,1003));assert(!tracker.observe(stale,live,1004));
    stale.applied=true;assert(tracker.observe(stale,live,1005));assert(!tracker.observe(stale,live,1006));
    // An unarmed ordinary-game number arrives after a crowd was prepared.
    // Adopt it without clearing that opening, but keep actual resumes and
    // established midgame generation changes authoritative.
    Tracker opening;auto ordinary=s;ordinary.applied=false;ordinary.game=1;
    ordinary.live={1,0,0,1,720,720};live=ordinary.live;
    assert(!opening.observe(ordinary,live,1000,true));ordinary.game=2;
    assert(!opening.pending(ordinary,live,1001,true)&&opening.pending(ordinary,live,1001));
    assert(!opening.observe(ordinary,live,1001,true)&&opening.game==2);
    ordinary.applied=true;assert(opening.pending(ordinary,live,1002,true));
    assert(opening.observe(ordinary,live,1002,true)&&!opening.pending(ordinary,live,1003,true));
    ordinary.applied=false;ordinary.game=3;assert(opening.observe(ordinary,live,1004));
    ordinary.game=4;ordinary.live=live={3,14,14,1,595.22f,900};
    assert(!opening.pending(ordinary,live,1005,true));
    assert(!opening.observe(ordinary,live,1005,true)&&opening.game==4); // First real snap after simming a half.
    ordinary.applied=true;assert(opening.observe(ordinary,live,1006,true)); // Explicit scenario still wins.
    // A previous game's applied marker must not count as this game's resume.
    auto foreign=text;foreign.replace(foreign.find("\"applied_game\":3"),16,"\"applied_game\":2");
    auto wrong=parse(foreign,9716,1000);assert(wrong.valid&&!wrong.applied);
    // Captured fresh-process resume: active game 0, applied_game 0. Zero is a
    // real generation and must not be confused with an uninitialized tracker.
    auto zeroText=text;zeroText.replace(zeroText.find("\"number\":3"),10,"\"number\":0");
    zeroText.replace(zeroText.find("\"applied_game\":3"),16,"\"applied_game\":0");
    auto zero=parse(zeroText,9716,1000);assert(zero.valid&&zero.applied&&zero.game==0);
    Tracker first;assert(!first.seen&&first.pending(zero,zero.live,1000,true));
    assert(first.observe(zero,zero.live,1000,true)&&first.seen&&first.game==0&&first.applied);
    assert(!first.observe(zero,zero.live,1001,true)); // Exactly one restoration.
    auto next=zero;next.game=1;next.applied=false;
    assert(first.pending(next,next.live,1002)&&first.observe(next,next.live,1002));
    Tracker ordinaryZero;zero.applied=false;
    assert(!ordinaryZero.observe(zero,zero.live,1000,true)&&ordinaryZero.seen);
    assert(!ordinaryZero.observe(next,next.live,1001,true)&&ordinaryZero.game==1); // Ordinary adoption remains intact.
    auto foreignZero=zeroText;foreignZero.replace(foreignZero.find("\"applied_game\":0"),16,"\"applied_game\":1");
    auto notApplied=parse(foreignZero,9716,1000);assert(notApplied.valid&&!notApplied.applied);
    auto negative=zeroText;negative.replace(negative.find("\"number\":0"),10,"\"number\":-1");assert(!parse(negative,9716,1000).valid);
    auto missing=zeroText;missing.replace(missing.find("\"number\":0"),10,"\"number\":null");assert(!parse(missing,9716,1000).valid);
    Json json;
    for(const char* invalid:{"{\"a\":1,\"a\":2}","{\"a\":01}","{\"a\":1.}","{\"a\":true,}","{\"a\":\"\\x\"}","{\"a\":1e9999}","[] trailing"})assert(!Reader(std::string(invalid)).parse(json));
    assert(!Reader(std::string(65537,' ')).parse(json));
    std::cout<<"PASS: bounded JSON parsing, PID/freshness/score validation, scenario generations and one-shot resume detection\n";
}
