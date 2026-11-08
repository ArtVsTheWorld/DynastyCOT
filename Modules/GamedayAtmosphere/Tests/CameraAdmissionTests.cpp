#include "../Source/LineupAtmosphere.h"
#include "../Source/DirectorShake.h"
#include <cassert>
#include <iostream>
using namespace attendance;
energy::Play fixture(){energy::Play p;p.game={2,14,14,1,600,720};p.offense=0;p.down=1;p.toGoal=75;p.toFirst=10;return p;}
int main(){
    core::Settings c;
    for(uint64_t n:{6060ull,26101ull,56099ull,61888ull,120000ull})for(int side:{0,1}){
        float standard=packed::standardMinimum(n),special=packed::specialMinimum(n),full=packed::minimum(n);
        auto p=fixture();p.offense=side;
        auto check=[&](float f,int type=3,bool known=true){return lineup::cameraAdmission(c,p,f,known,n,type);};
        assert(!check(standard).allowed); // Ordinary unranked calls still need packed house.
        p.down=3;
        for(float yards:{0.f,7.f,8.f,9.f,10.f}){p.toFirst=yards;auto cue=check(standard);assert(cue.allowed&&cue.critical);assert(!check(standard-.0001f).allowed);}
        p.toFirst=10.1f;assert(!check(standard).allowed);auto longThird=check(full);assert(longThird.allowed&&!longThird.critical);
        for(float invalid:{-1.f,NAN,INFINITY}){p.toFirst=invalid;assert(!check(standard).allowed);}
        p.toFirst=10;
        for(int type:{21,22,105,106,154,155}){assert(!check(standard,type).allowed);assert(check(full,type).allowed&&!check(full,type).critical);}
        p.down=4;assert(check(standard).allowed&&check(standard).critical);
        for(int type:{-1,0,21,22,23,24,25,37,38,105,106,154,155})assert(!check(standard,type).allowed);
        p.down=3;p.game.awayScore=23;assert(!check(standard).allowed);assert(check(full).allowed&&!check(full).critical);p.game.awayScore=14;
        p.down=1;p.game.quarter=5;p.game.seconds=0;
        auto ot=check(special);assert(ot.allowed&&!ot.critical&&!lineup::cameraVenue(c,p.game,special,true,n));
        assert(!check(special-.0001f).allowed);
        p.down=3;p.toFirst=10;assert(check(special).critical);p.toFirst=11;assert(check(special).allowed&&!check(special).critical);
        p.down=4;assert(check(special,21).allowed&&!check(special,21).critical); // OT field goal remains gentle.
        p.down=1;p.game.quarter=4;p.game.seconds=90;p.toGoal=30;p.offense=0;
        auto late=check(special);assert(late.allowed&&!late.critical);assert(!check(special-.0001f).allowed);
        p.offense=1;assert(check(special).allowed);p.game.neutral=true;assert(check(special).allowed);p.game.neutral=false;p.offense=0;
        p.down=3;p.toFirst=10;assert(check(special).critical);p.toFirst=11;assert(check(special).allowed&&!check(special).critical);
        p.down=1;p.game.seconds=181;assert(!check(special).allowed);
        p=fixture();p.game.rank0=6;p.game.rank1=11;assert(check(standard).allowed&&!check(standard).critical);
        p.game.phase=3;assert(!check(1).allowed);p.game.phase=7;assert(!check(1).allowed);p.game.phase=1;p.simulated=true;assert(!check(1).allowed);p.simulated=false;
        assert(!check(NAN).allowed&&!check(1.1f).allowed&&!check(1,3,false).allowed&&!check(1,-1).allowed);
        c.cameraShake=false;assert(!check(1).allowed);c.cameraShake=true;c.lineups=false;assert(!check(1).allowed);c.lineups=true;
        assert(!lineup::cameraAdmission(c,p,1,true,0,3).allowed);
    }
    // Exercise the admission decision through the actual motion envelope.
    auto p=fixture();p.game.quarter=5;p.game.seconds=0;motion::Settings settings;
    for(bool critical:{false,true}){
        p.down=critical?3:1;p.toFirst=10;auto admit=lineup::cameraAdmission(c,p,.50f,true,61888,3);assert(admit.allowed&&admit.critical==critical);
        motion::Model model;motion::Input x;x.match=123;x.epoch=1;x.phase=1;x.now=1000;x.allowed=x.lineup=true;x.lineupId=1;x.lineupScale=.5f;x.venueEligible=admit.allowed;x.critical=admit.critical;x.homeCritical=admit.critical&&!p.game.neutral&&p.offense==0;x.lineupStrengthScale=x.homeCritical?1.f:admit.strengthScale;
        model.observe(settings,x);x.now=2000;auto cue=model.observe(settings,x);float level=(critical?(x.homeCritical?6.5625f:8.75f):3.75f);
        assert(std::fabs(motion::strength(cue,x.now)-level)<.001f);
        x.phase=3;x.lineup=false;x.now=2100;assert(model.observe(settings,x).tail);x.now=3000;assert(model.observe(settings,x).kind==motion::Kind::None);
    }
    // Actual big games receive an opening allowance; ordinary small venues do not.
    p=fixture();p.game.quarter=1;p.game.seconds=720;p.game.rank0=6;p.game.rank1=11;
    assert(lineup::cameraAdmission(c,p,.50f,true,6060,3).allowed);
    assert(lineup::cameraAdmission(c,p,.90f,true,6060,3).allowed);
    // Home-offense critical downs soften; home-defense/neutral critical downs
    // keep their strength. Punts soften independently of side and custom ceiling.
    for(bool neutral:{false,true})for(int side:{0,1})for(int type:{3,21,22})for(int down:{1,3,4})for(float ceiling:{0.f,4.f,10.f,30.f}){
        p=fixture();p.offense=side;p.game.neutral=neutral;p.down=down;
        auto admit=lineup::cameraAdmission(c,p,1,true,61888,type);assert(admit.allowed);
        float scale=type==22?.5f:admit.critical&&!neutral&&side==0?.85f:1.f;
        assert(admit.strengthScale==scale);
        auto custom=settings;custom.ordinary=custom.critical=ceiling;
        motion::Model model;motion::Input x;x.match=456;x.epoch=1;x.now=1000;x.phase=1;x.allowed=x.lineup=x.venueEligible=true;
        x.lineupId=1;x.lineupScale=.5f;x.critical=admit.critical;x.homeCritical=admit.critical&&!p.game.neutral&&p.offense==0;x.lineupStrengthScale=x.homeCritical?1.f:admit.strengthScale;
        model.observe(custom,x);x.now=2000;auto cue=model.observe(custom,x);
        float expected=motion::lineupStrength(custom,admit.critical,.5f)*x.lineupStrengthScale*(x.homeCritical?.75f:1.f);
        assert(std::fabs(motion::strength(cue,x.now)-expected)<.001f);
        x.phase=3;x.lineup=false;x.now=2100;cue=model.observe(custom,x);
        assert(std::fabs(motion::strength(cue,x.now)-expected)<.001f);
        x.now=2550;cue=model.observe(custom,x);assert(std::fabs(motion::strength(cue,x.now)-expected*.5f)<.001f);
        // A return/long run and touchdown use their own settings after buildup.
        x.now=3100;x.duringPlay=1;model.observe(custom,x);x.now=3400;cue=model.observe(custom,x);
        assert(cue.kind==motion::Kind::LivePlay&&std::fabs(motion::strength(cue,x.now)-custom.live)<.001f);
        x.phase=4;x.now=3500;x.touchdown=1;model.observe(custom,x);x.now=3800;cue=model.observe(custom,x);
        assert(cue.kind==motion::Kind::Touchdown&&std::fabs(motion::strength(cue,x.now)-(std::max)(custom.touchdown,custom.touchdownImpact))<.001f);
        x.allowed=false;x.now=3900;assert(model.observe(custom,x).kind==motion::Kind::None);
    }
    // Sound-only venues above the independent native HFA cutoff still cannot
    // produce a DGA impact, even with fresh scoring/live event IDs. Losing
    // layered admission also cancels an already-active impact immediately.
    p=fixture();p.game.quarter=3;p.game.phase=1;
    constexpr uint64_t smallVenue=19375;
    lineup::Model layers;layers.observe(c,p,789,1,1000,true,.60f,true,nullptr,smallVenue);
    assert(!layers.playContextActive());
    bool eligible=lineup::reactionEligible(p.game,.60f,true,layers.playContextActive(),layers.qualifiedMinimum);
    assert(!eligible&&!lineup::cameraAdmission(c,p,.60f,true,smallVenue,3).allowed);
    motion::Model gated;motion::Input gatedInput;gatedInput.match=789;gatedInput.epoch=1;gatedInput.now=1000;gatedInput.phase=4;
    gatedInput.allowed=gatedInput.duringAllowed=eligible;gatedInput.touchdown=gatedInput.duringPlay=gatedInput.eruption=1;gatedInput.eruptionAlive=true;
    assert(gated.observe(settings,gatedInput).kind==motion::Kind::None);
    gatedInput.now=1100;gatedInput.allowed=gatedInput.duringAllowed=true;
    assert(gated.observe(settings,gatedInput).kind==motion::Kind::None); // No delayed blocked event.
    gatedInput.now=1200;gatedInput.eruption=0;gatedInput.touchdown=2;
    assert(gated.observe(settings,gatedInput).kind==motion::Kind::Touchdown);
    gatedInput.now=1300;gatedInput.allowed=false;
    assert(gated.observe(settings,gatedInput).kind==motion::Kind::None);
    std::cout<<"PASS: independent camera admission, sparse event rejection/cancellation, stable buildup envelopes, special teams, side/neutral scope and strict size/phase/sim gates.\n";
}
