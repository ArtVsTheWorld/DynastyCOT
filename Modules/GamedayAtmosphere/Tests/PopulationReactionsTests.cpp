#include "../Source/CrowdPopulation.h"
#include "../Source/DefensiveMotion.h"
#include "../Source/ReactionPalette.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    assert(std::abs(motion::occupiedScale(6060,1)-.80f)<.00001f);
    assert(std::abs(motion::occupiedScale(61888,1)-1.25f)<.00001f);
    assert(motion::occupiedScale(60600,.1f)==motion::occupiedScale(6060,1));
    assert(motion::occupiedScale(61888,.85f)>motion::occupiedScale(6060,.99f));
    for(uint64_t seats:{6060ull,30000ull,61888ull}){float lastMotion=0,lastAudio=0;
        for(int i=0;i<=100;++i){float fullness=i*.01f;
            float motion=motion::occupiedScale(seats,fullness),audio=population::presence(seats,fullness);
            assert(motion>=lastMotion&&audio>=lastAudio&&motion<=1.25f&&audio<=1);lastMotion=motion;lastAudio=audio;
        }
    }
    assert(motion::occupiedScale(0,1)==0&&motion::occupiedScale(61888,NAN)==0&&motion::occupiedScale(61888,-1)==0);
    sustained::Intent base;base.active=true;base.reason=sustained::Reason::Result;base.intensity=1000;
    auto small=population::layers(base,10000,1);base.intensity=1000*std::sqrt(.5f);
    auto largeHalf=population::layers(base,20000,.5f);assert(std::abs(small.intensity-largeHalf.intensity)<.001f);
    auto larger=population::layers(base,60000,.5f);assert(larger.intensity>largeHalf.intensity);
    assert(larger.intensity<=1000&&larger.liveCheer==base.liveCheer&&larger.resultCheer==base.resultCheer);
    assert(small.populationSize==1&&largeHalf.populationSize==1&&larger.populationSize==2);
    assert(population::size(61888,1)==3);
    assert(population::reactionType(83,1)==81&&population::reactionType(59,2)==58&&population::reactionType(103,1)==101);
    assert(population::negative(101)&&population::negative(95)&&!population::negative(83));
    assert(population::reactionType(107,1)==105); // Auditioned applause/cheer family.
    assert(population::reactionType(23,3)==22&&population::reactionType(15,3)==0);
    assert(population::layers(base,61888,0).intensity==0&&population::layers(base,0,1).intensity==0);
    base.reason=sustained::Reason::Diagnostics;assert(population::layers(base,6060,1).intensity==base.intensity);
    base.reason=sustained::Reason::None;assert(population::layers(base,6060,1).intensity==base.intensity);
    // Added-layer population scaling cannot create a cheer, extend a lease or
    // admit camera motion. Native reaction peaks never enter this helper.
    base.reason=sustained::Reason::Result;base.celebration=3;base.celebrationUntil=8000;base.liveCheer=10;
    auto scaled=population::layers(base,6060,.9f);assert(scaled.celebration==3&&scaled.celebrationUntil==8000&&scaled.liveCheer==10);
    for(int down:{3,4})for(int quarter:{1,2,3,4}){
        energy::Play snap,p;snap.game={quarter,14,14,1,100,720};snap.offense=1;snap.down=down;snap.selectedType=103;snap.toGoal=4;
        p=snap;defensivemotion::Model router;router.observe(snap,p,{}, {},123,1,1,1000,true);
        p.game.phase=3;router.observe(snap,p,{}, {},123,1,1,1100,true);
        energy::Outcome result;result.accepted=result.completed=true;result.sequence=1;result.side=0;
        result.thirdGoalStop=down==3;result.goalLineStop=down==4;
        p.game.phase=4;p.down=4;if(down==4){p.offense=0;p.down=1;result.turnoverOnDowns=true;}
        auto out=router.observe(snap,p,result,{},123,1,1,1200,true);
        assert(bool(out.id)==(down==4));if(out.id)assert(out.event==live::Event::GoalLineStop);
        router.reset();p=snap;router.observe(snap,p,{}, {},123,1,1,1000,true);
        p.game.phase=3;router.observe(snap,p,{}, {},123,1,1,1100,true);
        result.goalLineStop=result.thirdGoalStop=result.turnoverOnDowns=false;result.highLeverageStop=true;
        p.game.phase=4;p.down=4;if(down==4){p.offense=0;p.down=1;}
        out=router.observe(snap,p,result,{},123,1,1,1200,true);assert(bool(out.id)==(quarter==4));
    }
    for(bool eruption:{false,true}){sustained::Intent strong;strong.active=true;strong.reason=eruption?sustained::Reason::Eruption:sustained::Reason::PassFlight;strong.intensity=600;if(eruption)strong.celebration=1;else strong.passFlight=1;
      auto selected=population::layers(strong,6060,.6f);assert(selected.size==3&&selected.populationSize==3&&selected.intensity<=1000);}
    for(int code=1;code<=static_cast<int>(live::Event::OvertimeMissedFieldGoal);++code){
      energy::Play snap;snap.down=3;State s;auto type=palette::liveType({static_cast<live::Event>(code),1,1,30},snap,s,1,1,true);
      assert(catalog::usable(type)&&catalog::negative(type));
    }
    std::puts("PASS: occupied-seat curve and equal-count density invariance; continuous bounded added-layer population targets; untouched IDs/native routes; fourth-down-only goal-line and Q4-only high-leverage motion.");
}
