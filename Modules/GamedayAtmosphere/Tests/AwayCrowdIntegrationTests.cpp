#include "../Source/AwayCrowdAudio.h"
#include "../Source/AttendanceTimeline.h"
#include "../Source/MajorReplay.h"
#include <cassert>
#include <sstream>
#include <cstdio>
using namespace attendance;
int main(){
    for(unsigned away:{500u,5000u,10000u})for(bool homeWinning:{false,true}){
        uint64_t home=20000-away;visitors::Losses loss;uint64_t priorHome=0,priorAway=0;
        for(int percent=0;percent<=85;++percent){
            loss.advance(home,away,percent,homeWinning?42:0,homeWinning?0:42,4);
            assert(loss.home>=priorHome&&loss.away>=priorAway&&loss.home<=home&&loss.away<=away);
            assert(loss.home+loss.away==20000*static_cast<unsigned>(percent)/100);
            priorHome=loss.home;priorAway=loss.away;
            auto before=loss;loss.advance(home,away,percent,homeWinning?0:42,homeWinning?42:0,4);
            assert(before.home==loss.home&&before.away==loss.away); // A lead flip is not a refill.
        }
        if(!homeWinning&&away==10000)assert(away-loss.away>home-loss.home);
    }
    {visitors::Losses loss;loss.advance(800,200,100,0,56,4,150,0);assert(loss.home==650&&loss.away==200);}
    {visitors::Losses loss;loss.advance(800,200,80,0,56,4);assert(loss.home>0);
     loss.restore(800,200,150,0,.95f,.2f);assert(loss.home==650&&loss.away==40);
     loss.restore(800,200,150,0,-1,-1);assert(loss.home==0&&loss.away==0);
     loss.restore(800,200,150,0,NAN,.2f);assert(loss.home==0&&loss.away==0);}
    std::vector<unsigned char> raw(1024*10);
    for(unsigned i=0;i<1024;++i){raw[i*10]=i%5==0?1:0;raw[i*10+6]=i<16?static_cast<unsigned char>(1+i%8):i<116?9:0;}
    raw[116*10+6]=10;
    auto tags=agents::decode(raw,1024);agents::protectStudents(tags,true,false);
    seats::Pattern p;p.policy=seats::Policy::Scattered;std::vector<unsigned short> baseline(1024,0);
    auto g=visitors::groups(p,baseline,1,tags);assert(g.verified&&g.home+g.away==1008&&g.homeProtected==81&&g.awayProtected==0);
    for(unsigned i=0;i<1024;++i){
        if(tags.protectedGroup(i)){assert(!g.selected(i,1,1,100));if(tags.protectedFan(i))assert(g.rank[i]==UINT32_MAX);}
    }
    assert(!tags.protectedFan(20)&&tags.side[20]==1); // Visiting students aren't home cadets.
    agents::protectStudents(tags,false,true);assert(tags.protectedFan(20));
    sustained::Intent sound;sound.active=true;sound.match=123;sound.epoch=1;sound.updated=1000;
    sound.liveCheer=7;sound.liveCheerUntil=2000;sound.liveType=35;
    visitors::Population pop;pop.verified=true;pop.match=123;pop.epoch=1;pop.home=9500;pop.away=500;
    auto mix=visitors::audience(sound,pop,50000,.9f,1000);
    assert(mix.liveCheer==7&&mix.liveType==35&&mix.opponentCheer&&catalog::negative(mix.opponentType)&&mix.opponentSize==1);
    sound.liveType=79;pop.home=500;pop.away=9500;
    mix=visitors::audience(sound,pop,50000,.9f,1000);
    assert(mix.supporterSize==1&&mix.opponentSize==3&&mix.opponentPositive&&!catalog::negative(mix.opponentType));
    assert(mix.opponentType==75); // Native rushing-TD cheer, not a generic extra trigger.
    sound.reason=sustained::Reason::Result;sound.intensity=900;sound.priority=5;
    auto quietHome=visitors::layer(sound,{},pop,50000,.9f);assert(quietHome.intensity<200);
    auto away=sound;away.reason=sustained::Reason::Eruption;away.intensity=1000;away.priority=6;away.size=3;
    auto louder=visitors::layer(mix,away,pop,50000,.9f);assert(louder.intensity>800&&louder.opponentCheer==mix.opponentCheer&&louder.celebration==0);
    // An old weak visitor layer cannot replace a louder home response, even
    // when its semantic priority was higher. Both one-shots remain independent.
    pop.home=9500;pop.away=500;sound.intensity=900;away.intensity=200;
    auto dominant=visitors::layer(sound,away,pop,50000,.9f);assert(dominant.intensity>800&&dominant.reason==sound.reason);
    pop.home=10000;pop.away=0;dominant=visitors::layer(sound,away,pop,50000,.9f);assert(dominant.intensity==900);
    pop.home=0;pop.away=10000;auto visitorOnly=visitors::audience(sound,pop,50000,.9f,1000);
    assert(visitorOnly.liveCheer==0&&visitorOnly.opponentCheer&&visitorOnly.opponentPositive);
    for(int id=0;id<112;++id)if(catalog::usable(id))assert(catalog::usable(visitors::opposite(id)));
    timeline::Archive archive;State s{4,0,42,1,120,720};s.team0=1108;s.team1=1166;
    archive.begin(991,s);timeline::Point point;point.time=100;point.state=s;point.loss=60;point.periodStart=20;
    point.homeLoss=.8f;point.awayLoss=.1f;point.originalAwayShare=.35f;archive.record(point,false);
    std::stringstream text;timeline::write(text,archive);timeline::Archive restored;assert(timeline::read(text,restored));
    auto got=restored.games.front().points.front();assert(got.homeLoss==point.homeLoss&&got.awayLoss==point.awayLoss&&got.originalAwayShare==point.originalAwayShare);
    std::istringstream lines(text.str());std::string magic,header,row;std::getline(lines,magic);std::getline(lines,header);std::getline(lines,row);
    for(int n=0;n<4;++n)row.erase(row.rfind(' '));magic.replace(0,13,"DGA-HISTORY-5");
    std::stringstream old(magic+"\n"+header+"\n"+row+"\n");assert(timeline::read(old,restored));assert(restored.games.front().points.front().homeLoss==-1);
    using namespace majorreplay;
    assert(touchdownScoreProof(7,3,14,3)&&touchdownScoreProof(7,3,7,9));
    assert(!touchdownScoreProof(7,3,10,3)&&!touchdownScoreProof(7,3,0,0)&&!touchdownScoreProof(7,3,14,10));
    Model replay;replay.arm(123,1,1000,1,Kind::Touchdown,Mode::PlayCall);assert(replay.suppressInset(123,1));
    Input input{123,1,1200,4,true,false,true,true,Mode::PlayCall,false};assert(!replay.tick(input));
    input.now=2000;input.phase=5;input.replaying=false;input.playcallOpen=true;assert(!replay.tick(input));
    input.now=2750;assert(replay.tick(input)&&replay.takeNotice()==Notice::FallbackInset);
    std::puts("PASS: monotonic independent supporter exits, visiting-majority takeover, required cadets, no band/identity edits, dual polarity audio, symmetric strengths, v5/v7 history and unconfirmed-fullscreen fallback");
}
