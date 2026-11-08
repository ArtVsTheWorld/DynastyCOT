#include "../Source/NativeCameraAdmission.h"
#include "../Source/StartingAttendance.h"
#include "../Source/ContextReactions.h"
#include "../Source/DepartureModel.h"
#include "../Source/HalftimeExitConversion.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    Settings attendance;core::Settings sound;
    for(int home:{0,1,25,26,50,51,135})for(int away:{0,1,25,26,50,51,137})for(bool rivalry:{false,true}){
        State s{3,14,14,1,600,720};s.rank0=home;s.rank1=away;s.rivalry=rivalry;
        bool expected=(top25(home)&&top25(away))||(rivalry&&(top25(home)||top25(away)||(home>=1&&home<=50&&away>=1&&away<=50)));
        assert(marqueeMatchup(s)==expected&&bigGameMatchup(s)==expected&&lineup::bigGame(s)==expected&&turnout::big(home,away,rivalry)==expected);
        assert((profileFor(attendance,s)==Profile::Reduced)==expected);
        assert(contextreaction::premium(s,1,61888)==expected);
        if(expected)assert(context::importance(s)>=.85f);
        energy::Play p;p.game=s;p.down=1;p.offense=0;p.toFirst=10;p.toGoal=70;
        // Match classification alone cannot override a sparse crowd.
        assert(!lineup::cameraAdmission(sound,p,.49f,true,61888,3).allowed);
        p.game.playoff=true;assert(lineup::bigGame(p.game));
    }
    // Poll opinion affects only its 5% component. Prestige now contributes 25%.
    turnout::Team t;t.atmosphere=t.tradition=t.brand=t.conference=6;t.prestige=5;t.apRank=1;
    assert(std::fabs(turnout::support(t,138)-.525f)<.00001f);
    auto other=t;other.rank=138;other.toughest=138;assert(turnout::support(t,138)==turnout::support(other,138));
    other=t;other.apRank=138;assert(std::fabs(turnout::support(t,138)-turnout::support(other,138)-.05f)<.00001f);
    other=t;other.prestige=10;assert(std::fabs(turnout::support(other,138)-turnout::support(t,138)-.125f)<.00001f);
    other=t;other.apRank=0;auto unknown=other;unknown.apRank=255;assert(turnout::support(other,138)==turnout::support(unknown,138));
    State postseason{3,14,14,1,600,720};postseason.playoff=true;postseason.bowl=37;postseason.rank0=50;postseason.rank1=60;postseason.precipitation=1;
    assert(bigGameMatchup(postseason)&&profileFor(attendance,postseason)==Profile::BadWeather);
    postseason.rank0=1;postseason.rank1=25;assert(profileFor(attendance,postseason)==Profile::Reduced);
    // Native HFA gets the same decision before an actual DGA effect exists.
    energy::Play p;p.game={2,14,14,1,600,720};p.down=1;p.offense=0;p.toFirst=10;p.toGoal=70;
    auto decide=[&](float f= .60f,bool earned=false){return nativecamera::decide(sound,p,123,2,1000,f,true,19375,3,false,false,earned);};
    auto a=decide();assert(a.known&&!a.allowed&&nativecamera::suppress(a,123,2,1000,.60f,50));
    p.game.rank0=6;p.game.rank1=11;a=decide(1);assert(a.allowed&&!nativecamera::suppress(a,123,2,1000,1,50));
    // An admitted native context yields to actual DGA output, not merely an
    // eligible/prepared cue. Ownership, freshness and explicit opt-out remain.
    assert(nativecamera::suppress(a,123,2,1000,1,50,true));
    assert(!nativecamera::suppress(a,123,2,1000,1,50,false));
    assert(!nativecamera::suppress(a,123,2,1000,1,0,true));
    assert(!nativecamera::suppress(a,456,2,1000,1,50,true));
    assert(!nativecamera::suppress(a,123,3,1000,1,50,true));
    assert(!nativecamera::suppress(a,123,2,1751,1,50,true));
    assert(!nativecamera::suppress(a,123,2,999,1,50,true));
    p.game.rank0=p.game.rank1=60;p.down=3;a=decide(packed::standardMinimum(19375));assert(a.allowed);
    p.down=1;p.game.phase=3;a=decide(.95f,true);assert(a.allowed);
    a=decide(.95f,false);assert(!a.allowed); // No invented live context.
    p.game.phase=4;a=decide(.95f,true);assert(a.allowed);
    sound.cameraShake=0;a=decide(.95f,true);assert(!a.allowed);sound.cameraShake=1;
    p.simulated=true;assert(!decide(.95f,true).allowed);p.simulated=false;
    p.game.phase=1;p.game.rank0=6;p.game.rank1=11;p.game.night=true;
    float night=lineup::bigRequirement(sound,p.game,19375);a=decide(night);assert(a.allowed);
    p.game.night=false;assert(!decide(night).allowed);
    assert(!nativecamera::suppress(a,123,2,1000,night,0)); // Explicit legacy opt-out.
    assert(nativecamera::suppress(a,123,2,1000,.60f,80)); // Custom stricter floor.
    a.allowed=false;
    assert(!nativecamera::suppress(a,456,2,1000,.60f,50));
    assert(!nativecamera::suppress(a,123,3,1000,.60f,50));
    assert(!nativecamera::suppress(a,123,2,1751,.60f,50));
    assert(!nativecamera::suppress(a,123,2,999,.60f,50));
    a.known=false;assert(!nativecamera::suppress(a,123,2,1000,.60f,50));
    assert(nativecamera::suppress(a,123,2,1000,.49f,50));
    std::puts("PASS: shared marquee boundaries and density gates, prestige/AP component isolation, native camera crowd/context admission, night/critical/earned routes, opt-out/floor and stale-world safeguards.");
}
