#include "../Source/LineupAtmosphere.h"
#include "../Source/StadiumOccupancy.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
energy::Play play(){energy::Play p;p.game={2,14,14,1,600,720};p.down=1;p.offense=0;p.toGoal=80;p.toFirst=10;return p;}
energy::Outcome first(uint64_t sequence,int side=0){energy::Outcome r;r.accepted=r.completed=r.firstDown=true;r.sequence=sequence;r.side=side;r.yards=12;return r;}
sustained::Intent layer(lineup::Model& model,const core::Settings& settings,energy::Play& p,uint64_t now,float fullness,bool known=true,const energy::Outcome* result=nullptr,uint64_t positions=26101){
    model.observe(settings,p,123,1,now,true,fullness,known,result,positions);sustained::Intent base;base.active=true;base.match=123;base.epoch=1;base.updated=now;return model.merge(base,settings,p,now,fullness);
}
int main(){
    core::Settings c;auto p=play();lineup::Model packed;
    float packedMinimum=packed::minimum(26101);
    auto quiet=layer(packed,c,p,1000,packedMinimum-.0001f);assert(!quiet.lineup);p.game.phase=5;layer(packed,c,p,2000,packedMinimum);p.game.phase=1;layer(packed,c,p,3000,packedMinimum);auto mild=layer(packed,c,p,7000,packedMinimum);assert(mild.lineup&&packed.packedHouse&&mild.size<=2&&!mild.deepCheers);
    lineup::Model full;layer(full,c,p,1000,1);auto fullSound=layer(full,c,p,6000,1);assert(fullSound.intensity>mild.intensity&&fullSound.size<=3&&!fullSound.deepCheers);
    lineup::Model unknown;assert(!layer(unknown,c,p,6000,1,false).lineup);c.packedHouse=false;lineup::Model disabled;assert(!layer(disabled,c,p,6000,1).lineup);c.packedHouse=true;
    // Critical down and goal-line opportunities do not require a big-game tag.
    for(int down:{3,4})for(int side:{0,1})for(float f:{.699f,.70f,.85f}){
        auto q=play();q.down=down;q.offense=side;q.game.phase=5;lineup::Model m;auto prime=layer(m,c,q,1000,f,true,nullptr,37962);q.game.phase=1;layer(m,c,q,2000,f,true,nullptr,37962);auto loud=layer(m,c,q,6000,f,true,nullptr,37962);
        assert(bool(loud.lineup)==(f>=.70f));if(f>=.70f){assert(prime.cameraPrime&&prime.shakeTier==3&&loud.size==3&&!loud.deepCheers&&loud.shakeTier==3&&m.momentWeight()==1);core::Model gain;gain.update(c,q,6,f,f,m.momentWeight());assert(volume::boostPercent(gain.parent)<=35.01f);}
    }
    float previous=0;
    for(float goal:{20.f,15.f,10.f,5.f,1.f}){
        auto q=play();q.down=2;q.toGoal=goal;lineup::Model m;layer(m,c,q,1000,.8f);auto w=layer(m,c,q,6000,.8f);assert(w.lineup&&w.intensity>previous);previous=w.intensity;
        assert((w.shakeTier==2)==(goal<=5));assert((m.momentWeight()>0)==(goal<=10));q.offense=1;assert(lineup::goalLine(q)==0);q.game.neutral=true;assert(lineup::goalLine(q)>0);
    }
    p=play();p.game.awayScore=23;p.down=3;lineup::Model blowout;assert(!layer(blowout,c,p,1000,.8f).lineup);
    // Drive first downs grow excitement without lifting ordinary gain ceilings.
    p=play();lineup::Model driveLayers;layer(driveLayers,c,p,1000,.95f);auto baseline=layer(driveLayers,c,p,6000,.95f);float initial=baseline.intensity;
    for(uint64_t n=1;n<=4;++n){auto r=first(n);p.game.phase=5;layer(driveLayers,c,p,6000+n*5000,.95f,true,&r);p.game.phase=1;layer(driveLayers,c,p,7000+n*5000,.95f);auto w=layer(driveLayers,c,p,11000+n*5000,.95f);assert(w.intensity>initial);initial=w.intensity;}
    assert(driveLayers.drive.heat>.8f&&driveLayers.drive.heat<=1&&driveLayers.drive.chain==4);
    core::Model gain;gain.update(c,p,40,.95f,.95f,driveLayers.momentWeight());assert(std::fabs(volume::boostPercent(gain.parent)-20)<.01f); // Drive heat changes layers, not the mixer tier.
    float heat=driveLayers.drive.heat;p.game.seconds-=90;layer(driveLayers,c,p,40000,.95f);assert(std::fabs(driveLayers.drive.heat-heat*.5f)<.00001f);
    heat=driveLayers.drive.heat;auto failed=first(5);failed.firstDown=false;failed.yards=-3;layer(driveLayers,c,p,41000,.95f,true,&failed);assert(driveLayers.drive.heat<heat&&driveLayers.drive.chain==0);
    p.offense=1;layer(driveLayers,c,p,42000,.95f);assert(driveLayers.drive.heat==0);
    p.offense=0;layer(driveLayers,c,p,43000,.95f);auto r=first(6);p.resumed=true;layer(driveLayers,c,p,44000,.95f,true,&r);assert(driveLayers.drive.heat>0); // A resumed match can build new, observed momentum.
    p.game.homeScore+=3;layer(driveLayers,c,p,45000,.95f);assert(driveLayers.drive.heat==0);p.resumed=false;
    r=first(7);layer(driveLayers,c,p,46000,.95f,true,&r);assert(driveLayers.drive.heat>0);p.simulated=true;layer(driveLayers,c,p,47000,.95f);assert(driveLayers.drive.heat==0);p.simulated=false;
    for(int minutes=1;minutes<=20;++minutes){
        drive::Model m;p=play();p.game.quarterSeconds=static_cast<float>(minutes*60);p.game.seconds=p.game.quarterSeconds;r=first(1);m.observe(p,&r);heat=m.heat;p.game.seconds-=p.game.quarterSeconds/8;m.observe(p);assert(std::fabs(m.heat-heat*.5f)<.00001f);
        r=first(2);m.observe(p,&r);heat=m.heat;p.game.seconds+=10;m.observe(p,&r);assert(m.heat==heat); // Rewinds and duplicate facts do not build a drive.
    }
    // Read first downs and yardage from the actual verified-play tracker,
    // including a short first down that does not emit a legacy big-play event.
    energy::Tracker tracker;energy::Settings ec;p=play();p.down=2;p.toFirst=3;p.resultStamp=1;tracker.observe(ec,p);p.game.phase=3;tracker.observe(ec,p);p.game.phase=1;p.down=1;p.toGoal=76;p.game.seconds-=5;p.resultStamp=2;
    auto shortFirst=tracker.observe(ec,p);assert(shortFirst.completed&&shortFirst.firstDown&&shortFirst.yards==4&&shortFirst.event==energy::Event::None);
    drive::Model tracked;tracked.observe(p,&shortFirst);assert(tracked.heat>0);float once=tracked.heat;tracked.observe(p,&shortFirst);assert(tracked.heat==once);
    // A result can first become verifiable at the next lineup. Preserve the
    // previous layer context before observe resets the new attack to zero.
    tracker.reset();lineup::Model pipeline;core::Model accents;p=play();p.down=2;p.toFirst=3;p.resultStamp=1;
    auto observe=[&](uint64_t now){
        auto out=tracker.observe(ec,p);bool had=pipeline.match==123&&pipeline.epoch==1&&pipeline.layeredActive();
        pipeline.observe(c,p,123,1,now,true,.95f,true,&out,26101);
        accents.react(c,out,now/1000.0,p.game,had||pipeline.layeredActive());return out;
    };
    observe(1000);observe(6000);p.game.phase=3;observe(7000);p.game.phase=1;p.down=1;p.toGoal=76;p.game.seconds-=5;p.resultStamp=2;
    auto verified=observe(9000);assert(verified.firstDown&&accents.resultCheer==1&&accents.resultType==83);
    assert(pipeline.weight==0&&pipeline.drive.heat>0);observe(9050);assert(accents.resultCheer==1);
    assert(!accents.intent(c,p,123,1,12000,true).resultCheer);
    std::puts("PASS: verified size-based packed-house gate, lighter normal buildups, independent critical/goal-line atmosphere, native maximum shake, drive growth/stalls/possession/score/sim/resume, all-quarter-length decay and unchanged 25/35/50 volume ceilings.");
}
