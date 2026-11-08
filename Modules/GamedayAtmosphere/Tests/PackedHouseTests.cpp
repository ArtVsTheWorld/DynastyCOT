#include "../Source/LineupAtmosphere.h"
#include <cassert>
#include <iostream>
using namespace attendance;
int main(){
    assert(packed::minimum(6060)==.98f&&packed::minimum(61888)==.60f);
    assert(!packed::qualifies(6060,.9799f)&&packed::qualifies(6060,.98f));
    assert(std::fabs(packed::minimum(33974)-.79f)<.000001f); // Midpoint follows the rebalanced 98-to-60 curve.
    assert(std::fabs(packed::strength(6060,.99f)-.5f)<.00001f); // Smooth strength across the new 98-to-100 band.
    assert(!packed::qualifies(0,1)&&!packed::qualifies(511,1)&&!packed::qualifies(13000000,1));
    assert(packed::qualifies(512,.98f)&&packed::qualifies(13000,.95f));
    assert(packed::minimum(120000)==.60f&&!packed::qualifies(120000,.599f));
    assert(!packed::qualifies(50000,NAN)&&!packed::qualifies(50000,1,false));
    float previous=1;
    for(uint64_t n=512;n<200001;n+=17){
        float minimum=packed::minimum(n);assert(minimum>=.60f&&minimum<=.98f&&minimum<=previous);previous=minimum;
        assert(!packed::qualifies(n,minimum-.0001f)&&packed::qualifies(n,minimum));
        assert(packed::strength(n,minimum)==0&&packed::strength(n,1)==1);
        for(float f:{.6f,.7f,.8f,.9f,1.0f})assert(packed::strength(n,f)>=0&&packed::strength(n,f)<=1);
    }
    core::Settings c;energy::Play p;p.game={2,14,14,5,600,720};p.game.rank0=35;p.game.rank1=50;p.down=1;p.offense=0;p.toGoal=75;
    for(uint64_t n:{6060ull,13000ull,30464ull,51424ull,56099ull,61888ull,100000ull}){
        float threshold=packed::minimum(n);
        lineup::Model m;m.observe(c,p,123,1,1000,true,threshold,true,nullptr,n);
        assert(lineup::cameraVenue(c,p.game,threshold,true,n));
        p.game.phase=1;m.observe(c,p,123,1,2000,true,threshold,true,nullptr,n);assert(m.playContextActive()&&m.packedHouse);
        p.game.phase=3;m.observe(c,p,123,1,3000,true,threshold,true,nullptr,n);assert(m.playContextActive());
        p.game.phase=5;m.observe(c,p,123,1,4000,true,threshold-.001f,true,nullptr,n);
        p.game.phase=1;m.observe(c,p,123,1,5000,true,threshold-.001f,true,nullptr,n);assert(!m.playContextActive());
        assert(!lineup::cameraVenue(c,p.game,threshold-.001f,true,n));p.game.phase=5;
    }
    p.game.phase=1;lineup::Model unknown;unknown.observe(c,p,123,1,1000,true,1,true);assert(!unknown.playContextActive());
    p.game.rank0=6;p.game.rank1=11;lineup::Model big;big.observe(c,p,123,1,1000,true,.90f,true,nullptr,6060);assert(big.playContextActive());
    assert(lineup::cameraVenue(c,p.game,.90f,true,6060)); // Big-game exception stays independent of size.
    p.game.homeScore=42;p.game.awayScore=7;lineup::Model rout;rout.observe(c,p,123,1,1000,true,1,true,nullptr,61888);assert(!rout.playContextActive());
    // Large venues admitted below 70% still receive their critical-down layer.
    p.game={2,14,14,1,600,720};p.down=3;p.toFirst=5;
    lineup::Model critical;critical.observe(c,p,123,1,1000,true,.65f,true,nullptr,61888);
    critical.observe(c,p,123,1,1500,true,.65f,true,nullptr,61888);assert(critical.shakeTier==3&&critical.momentWeight()>0);
    p.down=4;assert(lineup::criticalCamera(p,3)&&!lineup::criticalCamera(p,22));
    std::cout<<"PASS: stored-position curve, tiny/oversize/unknown layouts, monotonic thresholds, sound/camera agreement, changing fullness and separate big-game/blowout rules.\n";
}
