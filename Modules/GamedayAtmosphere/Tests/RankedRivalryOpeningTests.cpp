#include "../Source/OpeningKickoffAtmosphere.h"
#include "../Source/ArrivalRules.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    core::Settings settings;
    for(bool neutral:{false,true})for(bool rankedHome:{false,true}){
        energy::Play p;p.game={1,0,0,1,720,720};p.game.team0=1153;p.game.team1=1193;p.game.rivalry=true;p.game.neutral=neutral;
        p.game.rank0=rankedHome?12:70;p.game.rank1=rankedHome?70:12;p.offense=1;p.down=0;
        assert(lineup::bigGame(p.game)&&bigGameMatchup(p.game));
        assert(lineup::cameraVenue(settings,p.game,.50f,true,20000));assert(!lineup::cameraVenue(settings,p.game,.499f,true,20000));
        auto admitted=lineup::cameraAdmission(settings,p,.50f,true,20000,23,true);assert(admitted.allowed);
        kickoff::Model opening;assert(opening.observe(p,123,1,1000,true,true,23,.5f));
        sustained::Intent sound;sound.active=true;sound.match=123;sound.epoch=1;auto out=opening.merge(sound,1100);assert(out.cheers&&out.supporting&&out.size==3);
        // Shared marquee identity does not invent an exceptional kickoff share.
        arrivals::Settings arrivals;auto unranked=p.game;unranked.rank0=unranked.rank1=70;
        assert(arrivals::baselinePercent(arrivals,p.game)==arrivals::baselinePercent(arrivals,unranked));
        p.game.rivalry=false;assert(!lineup::bigGame(p.game));assert(!lineup::cameraVenue(settings,p.game,.5f,true,20000));
    }
    State s;s.rank0=s.rank1=70;s.rivalry=true;assert(!lineup::bigGame(s));s.rank0=0;s.rank1=25;assert(lineup::bigGame(s));s.rank1=26;assert(!lineup::bigGame(s));
    s.rank0=34;s.rank1=47;assert(lineup::bigGame(s)&&bigGameMatchup(s));s.rank0=50;s.rank1=50;assert(lineup::bigGame(s));s.rank1=51;assert(!lineup::bigGame(s));s.rank0=135;s.rank1=137;assert(!lineup::bigGame(s));
    s.rank0=5;s.rank1=25;s.rivalry=false;assert(lineup::bigGame(s));s.rank0=s.rank1=70;s.playoff=true;assert(lineup::bigGame(s));
    std::puts("PASS: shared ranked-rivalry identity, opening sound/camera at measured50%, unknown/unranked guards and neutral classification.");
}
