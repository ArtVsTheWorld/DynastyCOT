#include "../Source/AttendanceTimeline.h"
#include "../Source/LineupAtmosphere.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
State fixture(int length=720){State s{4,3,24,5,float(length/2),float(length)};s.rank0=65;s.rank1=75;return s;}
int finish(bool night,int home,int away,int length){
    Settings c;departures::Model m;History h;
    State s=fixture(length);s.quarter=1;s.seconds=float(length);s.homeScore=s.awayScore=0;s.night=night;
    h.observe(c,s);m.observe(c,s,h);
    for(int tick=1;tick<=400;++tick){
        float elapsed=4.f*length*tick/400;
        s.quarter=(std::min)(4,1+int(elapsed/length));s.seconds=4*length-elapsed-(4-s.quarter)*length;
        if(tick>=100){s.homeScore=home;s.awayScore=away;}
        h.observe(c,s);m.observe(c,s,h);m.beginWindow();m.settle(s,m.committedLoss,0,c.quarterCap);
    }
    return m.committedLoss;
}
int main(){
    State s=fixture();s.night=true;
    assert(std::fabs(nightRequirement(s,.80f)-.72f)<.000001f);
    assert(nightRequirement(s,.55f)==.50f&&nightRequirement(s,.50f)==.50f);
    assert(nightRequirement(s,1.01f)==1.01f);
    for(int special=0;special<3;++special){auto p=s;if(special==0)p.playoff=true;if(special==1)p.conferenceChampionship=true;if(special==2)p.bowl=1;assert(nightRequirement(p,.80f)==.80f);assert(context::departureEngagement(p,3.5f*720)==1);}
    s.night=false;assert(nightRequirement(s,.80f)==.80f);
    for(uint64_t positions:{6060ull,30464ull,61888ull}){
        energy::Play p;p.game={3,7,7,1,600,720};p.offense=0;p.down=1;p.toFirst=10;p.toGoal=70;
        float day=packed::minimum(positions),night=(std::max)(.50f,day*.9f),between=(day+night)*.5f;
        assert(!lineup::cameraVenue(core::Settings{},p.game,between,true,positions));
        p.game.night=true;assert(lineup::cameraVenue(core::Settings{},p.game,between,true,positions));
        lineup::Model m;m.observe(core::Settings{},p,123,1,1000,true,between,true,nullptr,positions);
        assert(m.playContextActive()&&!m.packedHouse&&std::fabs(m.qualifiedMinimum-night)<.000001f);
        assert(!packed::qualifies(positions,between)); // Night does not relabel a partly filled venue as packed.
        p.game.phase=3;m.observe(core::Settings{},p,123,1,1200,true,between,true,nullptr,positions);
        assert(m.playContextActive());
        assert(!lineup::reactionEligible(p.game,.499f,true,true,m.qualifiedMinimum));
        assert(!lineup::cameraVenue(core::Settings{},p.game,1,true,0));
    }
    for(int length:{60,300,720,1200}){
        auto low=fixture(length);auto high=low;high.homeScore=31;high.awayScore=52; // Same 21-point deficit.
        float t=3.5f*length;
        assert(context::departureEngagement(low,2.f*length)==1);
        float lowDay=context::departureEngagement(low,t),highDay=context::departureEngagement(high,t);
        assert(lowDay>highDay&&lowDay<=1.150001f&&highDay>=.899999f);
        low.night=true;assert(context::departureEngagement(low,t)>lowDay);
        assert(context::departureEngagement(low,t)<=1.380001f);
        auto close=low;close.homeScore=3;close.awayScore=6;
        assert(context::departureEngagement(close,t)<1.10f);
        History story;Settings c;
        auto protectedGame=low;story.comeback=true;assert(departures::rate(c,protectedGame,story,t)==0);
        low.quarter=5;assert(context::departureEngagement(low,t)==1);
        int dayLoss=finish(false,3,24,length),nightLoss=finish(true,3,24,length),shootoutLoss=finish(false,31,52,length);
        assert(nightLoss>=dayLoss&&dayLoss>=shootoutLoss&&nightLoss<=lossCeiling(fixture(length)));
        std::printf("length=%d same deficit: low day/night %d/%d, shootout day %d departures\n",length,dayLoss,nightLoss,shootoutLoss);
    }
    std::puts("PASS: night admission boundaries/50% floor, native packed identity, live reaction inheritance, postseason/unknown-layout isolation, normalized scoring pace, bounded pressure, close/comeback protection and full-game ordering.");
}
