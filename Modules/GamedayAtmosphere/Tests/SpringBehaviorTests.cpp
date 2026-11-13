#include "../Source/SpringReactions.h"
#include "../Source/AttendanceTimeline.h"
#include <cassert>
#include <iostream>
#include <cstring>
using namespace attendance;
State fixture(){State s{2,7,7,5,30,180};s.team0=1502;s.team1=1145;s.spring=true;s.springSchool=1145;s.springDynasty=-1972245012;s.springNumber=250;s.variationSeed=3147028881u;return s;}
int main(){
    energy::Play p;p.game=fixture();p.offense=1;energy::Outcome r;r.accepted=r.completed=true;r.sequence=77;r.yards=12;
    spring::Reactions lease;auto first=lease.observe(p,r,123,1,77,1000,true,1);assert(first.selected&&first.intent.resultCheer==77&&first.intent.resultCheerUntil==1600);
    r={};for(uint64_t now=1016;now<1600;now+=16){auto next=lease.observe(p,r,123,1,77,now,true,1);assert(!next.selected&&next.intent.resultCheer==77&&next.intent.resultCheerUntil==1600);}
    auto expired=lease.observe(p,r,123,1,77,1600,true,1);assert(expired.expired&&!expired.intent.resultCheer);
    r.accepted=r.completed=true;r.sequence=77;r.yards=12;assert(!lease.observe(p,r,123,1,77,1700,true,1).intent.resultCheer);
    r.sequence=78;r.yards=-5;assert(lease.observe(p,r,123,1,78,1800,true,1).intent.resultType==102);lease.interrupt();
    assert(!lease.observe(p,r,123,1,78,1810,true,0).intent.resultCheer);
    r.sequence=79;assert(!lease.observe(p,r,123,1,79,1900,false,0).intent.resultCheer);
    assert(!lease.observe(p,r,123,1,79,1910,true,0).intent.resultCheer);
    r.sequence=80;p.simulated=true;assert(!lease.observe(p,r,123,1,80,2000,true,0).intent.resultCheer);p.simulated=false;
    r.sequence=81;assert(lease.observe(p,r,456,2,81,2100,true,1).selected);
    assert(!lease.observe(p,r,456,2,82,2110,true,0).intent.resultCheer); // old result, new snap
    unsigned cases=0;
    for(unsigned seed=1;seed<=100;++seed)for(int length:{120,180,720}){
        int baseLoss=-1,baseTemporary=-1;
        for(int score:{-49,0,49}){
            auto s=fixture();s.variationSeed=seed;s.quarterSeconds=static_cast<float>(length);s.seconds=0;s.homeScore=score>0?score:0;s.awayScore=score<0?-score:0;
            Settings c;History history;departures::Model model;arrivals::Halftime seats;
            assert(history.observe(c,s)&&model.observe(c,s,history));model.beginWindow(s,100);int loss=model.settle(s,0,0,45).loss;assert(loss==4);
            s.quarter=3;s.seconds=static_cast<float>(length);assert(history.observe(c,s)&&model.observe(c,s,history));model.beginWindow(s,100);loss=model.settle(s,loss,0,45).loss;
            assert(loss==static_cast<int>(4+spring::halftimeWave(s))&&loss>=14&&loss<=19);
            // The same halftime cannot add the wave twice, even after a drill reset.
            for(int n=0;n<3;++n){s.quarter=2;s.seconds=static_cast<float>(length/2);s.homeScore=s.awayScore=0;assert(history.observe(c,s)&&model.observe(c,s,history));model.beginWindow(s,100);assert(model.settle(s,loss,0,45).loss==loss);s.quarter=3;s.seconds=static_cast<float>(length);assert(history.observe(c,s)&&model.observe(c,s,history));model.beginWindow(s,100);assert(model.settle(s,loss,0,45).loss==loss);}
            s.homeScore=score>0?score:0;s.awayScore=score<0?-score:0;s.quarter=2;s.seconds=0;int total=seats.observe(s,4,100,true);assert(total>4&&total<=34);
            s.quarter=3;s.seconds=static_cast<float>(length);total=seats.observe(s,loss,100,true);assert(total>=loss);
            if(baseLoss<0){baseLoss=loss;baseTemporary=total;}else assert(baseLoss==loss&&baseTemporary==total);
            for(int tick=1;tick<=30;++tick){s.seconds=length*(1-tick/30.f);int absent=seats.observe(s,loss,100,true);assert(absent==0||absent>=loss);}
            assert(seats.done&&!seats.active&&seats.lastAbsent==0);
            s.quarter=2;s.seconds=0;assert(seats.observe(s,loss,100,true)==0); // no repeated temporary wave
            s.quarter=4;s.seconds=0;assert(history.observe(c,s)&&model.observe(c,s,history));model.beginWindow(s,100);loss=model.settle(s,loss,0,45).loss;assert(loss>=40&&loss<=55);
            assert(std::strstr(departures::reason(c,s,history),"spring exhibition"));++cases;
        }
    }
    auto s=fixture();s.quarter=3;s.seconds=175;timeline::Point saved;saved.state=s;saved.loss=18;saved.temporary=31;saved.periodStart=18;
    auto restored=timeline::estimate(Settings{},arrivals::Settings{},s,&saved);assert(restored.loss==18&&restored.temporary==31&&restored.halftime.lastAbsent==13);
    assert(restored.halftime.observe(s,18)>=18);s.seconds=90;assert(restored.halftime.observe(s,18)==0);
    std::cout<<"PASS spring request retention/expiry/pause/ownership/settings/simulation guards, "<<cases<<" score-independent halftime waves, no repeat after drill resets, temporary returns, unchanged final-retention bounds and saved temporary/permanent separation; isolated only\n";
}
