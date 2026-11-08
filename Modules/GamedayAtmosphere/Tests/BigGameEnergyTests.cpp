#include "../Source/LineupAtmosphere.h"
#include <cassert>
#include <cmath>
#include <cstdio>
using namespace attendance;
struct Snapshot {lineup::Model model;sustained::Intent sound;};
Snapshot sample(State state,float fullness,uint64_t positions,int down=1){
    core::Settings c;energy::Play p;p.game=state;p.offense=0;p.down=down;p.toGoal=70;p.toFirst=5;
    Snapshot r;p.game.phase=5;r.model.observe(c,p,123,1,1000,true,fullness,true,nullptr,positions);
    p.game.phase=1;r.model.observe(c,p,123,1,2000,true,fullness,true,nullptr,positions);
    r.model.observe(c,p,123,1,6000,true,fullness,true,nullptr,positions);
    sustained::Intent base;base.active=true;base.match=123;base.epoch=1;base.updated=6000;
    r.sound=r.model.merge(base,c,p,6000,fullness);return r;
}
State fixture(){State s{2,14,14,1,600,720};s.rank0=6;s.rank1=50;s.variationSeed=73;return s;}
int main(){
    // Preserve every ordinary foundation value, including opening and margins.
    for(float density:{0.f,.1f,.4f,.8f,1.f})for(float close:{.12f,.35f,.75f,1.f})for(float opening:{0.f,.2f,.6f,1.f})
        assert(lineup::ordinaryImportance(density,close,opening)==(std::max)((.20f+.32f*density)*close,.62f*opening*close));
    auto ordinaryState=fixture(),bigState=ordinaryState;bigState.rank1=2;
    auto ordinary=sample(ordinaryState,.9f,56099),big=sample(bigState,.9f,56099);
    assert(ordinary.model.playContextActive()&&big.model.playContextActive());
    assert(std::fabs(ordinary.model.strength-.45982024f)<.00001f); // Previous TAMU 90% ordinary strength remains exact.
    assert(big.model.strength>ordinary.model.strength+.19f&&big.sound.intensity>ordinary.sound.intensity+90);
    assert(ordinary.sound.supporting&&big.sound.supporting&&big.sound.size>=ordinary.sound.size); // Dense ordinary crowds now keep the supporting swell too.
    auto sixty=sample(ordinaryState,.6f,56099);assert(!sixty.model.playContextActive()); // No new penalty or admission change.
    auto bigSixty=sample(bigState,.6f,56099);assert(bigSixty.model.playContextActive());
    assert(big.sound.intensity>bigSixty.sound.intensity);
    assert(balance::crowdBoost(.6f)==0&&balance::crowdBoost(.9f)>14.99f);
    // At the same fullness, a classified big game never loses to ordinary
    // buildup merely because it took a different formula branch.
    for(uint64_t positions:{6060ull,10394ull,30464ull,51424ull,56099ull,61888ull,120000ull})
    for(float fullness:{.6f,.7f,.8f,.9f,.98f,1.f})for(int quarter:{1,2,3,4})for(int margin:{0,7,12,20,24}){
        auto s=fixture();s.quarter=quarter;s.seconds=quarter==1?720.f:60.f;s.awayScore=s.homeScore+margin;
        auto a=sample(s,fullness,positions);s.rank1=2;auto b=sample(s,fullness,positions);
        if(a.model.playContextActive()){
            assert(b.model.playContextActive()&&b.model.strength>=a.model.strength);
            assert(b.sound.intensity+.001f>=a.sound.intensity&&b.sound.size>=a.sound.size);
        }
        assert(b.sound.intensity>=0&&b.sound.intensity<=1000.001f);
        if(margin>=24)assert(!a.model.playContextActive()&&!b.model.playContextActive());
    }
    // Big-game premium grows late, with identical progress at any quarter length.
    for(float seconds:{60.f,180.f,300.f,720.f,1200.f}){
        auto s=bigState;s.quarter=4;s.quarterSeconds=seconds;s.seconds=seconds;
        float early=lineup::bigGamePremium(s,.9f,1);s.seconds=seconds*.125f;
        assert(std::fabs(lineup::bigGamePremium(s,.9f,1)-early-.05f)<.00001f);
        s.seconds=0;assert(std::fabs(lineup::bigGamePremium(s,.9f,1)-early-.10f)<.00001f);
    }
    auto finishState=bigState;finishState.quarter=4;finishState.seconds=60;
    auto finish=sample(finishState,.9f,56099);assert(finish.sound.intensity>big.sound.intensity);
    auto playoffState=bigState;postseason(playoffState,true,false);
    assert(sample(playoffState,.9f,56099).sound.intensity>big.sound.intensity);
    postseason(playoffState,false,true);assert(sample(playoffState,.9f,56099).sound.intensity>big.sound.intensity);
    auto openingState=bigState;openingState.quarter=1;openingState.seconds=720;
    assert(sample(openingState,1,56099).sound.intensity==1000); // Existing opening peak is retained.
    assert(sample(openingState,.70f,30464).model.playContextActive()); // LSU-like 70% opening no longer waits for a 71% size gate.
    for(float length:{60.f,300.f,720.f,1200.f}){
        auto start=openingState;start.quarterSeconds=start.seconds=length;
        assert(lineup::bigRequirement(core::Settings{},start,6060)==.5f);
        float previous=.5f;
        for(int i=0;i<=100;++i){start.seconds=length*(1-i*.01f);float need=lineup::bigRequirement(core::Settings{},start,6060);assert(need>=previous-.000001f&&need<=.90001f);previous=need;}
        assert(previous==.9f);
    }
    assert(!sample(openingState,.499f,30464).model.playContextActive()); // No sparse-crowd reaction bypass.
    auto critical=sample(bigState,.9f,56099,3);assert(critical.model.strength>=.95f);
    auto repeat=sample(bigState,.9f,56099);assert(repeat.sound.intensity==big.sound.intensity); // No accumulating premium.
    core::Settings settings;core::Model gain;energy::Play p;p.game=bigState;
    gain.update(settings,p,6,.9f,.9f);assert(std::fabs(volume::boostPercent(gain.parent)-15)<.001f);
    assert(core::NormalMaximumPercent==25&&core::HighLeverageMaximumPercent==35&&core::EruptionMaximumPercent==50);
    big.model.observe(settings,p,123,1,6100,false,.9f,true,nullptr,56099);
    assert(!big.model.playContextActive()&&!big.model.layeredActive());
    std::puts("PASS: ordinary-game foundation unchanged, big-game energy always at least ordinary, fuller layers, quarter-scaled close finish/postseason emphasis, original opening/critical peaks, blowout/context guards and unchanged volume caps.");
}
