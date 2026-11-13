#include "../Source/SpringIdentity.h"
#include "../Source/AttendanceTimeline.h"
#include <cassert>
#include <sstream>
#include <iostream>
using namespace attendance;
State springState(int seconds=720){State s{1,0,0,5,static_cast<float>(seconds),static_cast<float>(seconds)};s.team0=1502;s.team1=1123;s.spring=true;s.springSchool=1123;s.springDynasty=77;s.springNumber=4;s.plannedWeek=4;s.plannedYear=0;return s;}
int main(){
    for(int school:{1145,1801,1902}){assert(spring::actualSide(0,250,false,false,1502,school,false,true,1000000)==1);assert(spring::baseEvidence(1000000,1,true,true));}
    assert(spring::actualSide(1,250,false,false,1502,1801,false,true,1000000)<0);
    assert(!spring::baseEvidence(1000000,1,false,true));assert(!spring::baseEvidence(1000000,1,true,false));assert(!spring::baseEvidence(1000000,0,true,true));

    assert(spring::actualSide(0,4,false,false,1502,1123,false,true)==1);
    assert(spring::actualSide(0,4,false,false,1123,1502,true,false)==0);
    for(int type:{1,2,3})assert(spring::actualSide(type,4,false,false,1502,1123,false,true)<0);
    assert(spring::actualSide(0,250,false,false,1502,1123,false,true)<0);
    for(int type:{0,1})for(int stamp:{1100000,1200000})assert(spring::actualSide(type,250,false,false,1502,1123,false,true,stamp)==1);
    for(int stamp:{-1,0,1099999,1100001,1199999,1200001})assert(spring::actualSide(1,250,false,false,1502,1123,false,true,stamp)<0);
    assert(spring::actualSide(2,250,false,false,1502,1123,false,true,1200000)<0);
    assert(spring::actualSide(1,250,false,false,1501,1123,false,true,1200000)<0);
    assert(spring::actualSide(1,250,false,false,1502,1123,false,false,1200000)<0);
    assert(spring::actualSide(1,250,true,false,1502,1123,false,true,1200000)<0);
    assert(spring::actualSide(1,250,false,true,1502,1123,false,true,1200000)<0);
    assert(spring::actualSide(0,4,false,false,1502,1123,true,true)==1);
    assert(spring::actualSide(0,4,true,false,1502,1123,false,true)<0);
    assert(spring::actualSide(0,4,false,false,1111,1123,true,true)<0);
    for(int cap:{1000,25000,100000})for(int native:{0,1,500,15000,99999})assert(spring::turnout(native,cap)==(native>0?(std::min)(native,cap*60/100):native));
    assert(spring::turnout(50000,0)==50000);assert(spring::turnout(90000,100000,true)==90000);
    auto signedId=springState();signedId.springDynasty=-2147483647;assert(spring::verified(signedId));signedId.springDynasty=0;assert(!spring::verified(signedId));
    int cases=0;
    for(int number:{4,250})for(int length:{120,300,720,900})for(unsigned seed=0;seed<100;++seed)for(int lead:{-56,0,56}){
        auto s=springState(length);s.springNumber=number;assert(spring::verified(s));s.variationSeed=seed;s.homeScore=lead>0?lead:0;s.awayScore=lead<0?-lead:0;
        departures::Model d;History h;Settings c;int loss=0;
        for(int q=1;q<=4;++q)for(int tick=0;tick<=60;++tick){s.quarter=q;s.seconds=length*(1-tick/60.f);int old=loss;
            assert(d.observe(c,s,h));d.beginWindow(s,100);loss=d.settle(s,loss,25,c.quarterCap).loss;
            assert(loss>=old);if(q<=2)assert(loss<=4);
        }
        assert(loss>=40&&loss<=55);
        auto rewind=s;rewind.quarter=2;rewind.seconds=120;rewind.homeScore=rewind.awayScore=0;
        if(rewind.seconds<=rewind.quarterSeconds){assert(d.observe(c,rewind,h));d.beginWindow(rewind,100);assert(d.settle(rewind,loss).loss>=loss);}
        ++cases;
    }
    auto s=springState();s.springNumber=250;s.quarter=2;s.seconds=120;assert(spring::departureTarget(s)<=4);
    s.quarter=3;s.seconds=719;assert(spring::departureTarget(s)>4);
    energy::Play p;p.game=s;p.offense=0;energy::Outcome r;r.accepted=r.completed=true;r.sequence=1;r.yards=12;
    assert(spring::reaction(p,r)==spring::Reaction::Cheer);p.offense=1;assert(spring::reaction(p,r)==spring::Reaction::Cheer);
    r.setback=true;r.firstDown=true;assert(spring::reaction(p,r)==spring::Reaction::Cheer);r.firstDown=false;r.setback=false;
    r.yards=-1000;r.awayPoints=7;assert(spring::reaction(p,r,1)==spring::Reaction::Cheer);assert(spring::reaction(p,r,0)==spring::Reaction::None);r.awayPoints=0;assert(spring::reaction(p,r)==spring::Reaction::None);
    r.sack=true;assert(spring::reaction(p,r,1)==spring::Reaction::Disappointment);p.simulated=true;assert(spring::reaction(p,r,1)==spring::Reaction::None);
    timeline::Archive a;s.quarter=3;s.seconds=600;a.begin(123,s,true);timeline::Point pt;pt.state=s;pt.time=1500;pt.loss=pt.periodStart=18;assert(a.record(pt));
    std::stringstream file;timeline::write(file,a);timeline::Archive b;assert(timeline::read(file,b));
    timeline::Point found;assert(b.resume(s,found));s.springDynasty=78;assert(!b.resume(s,found));s.springDynasty=77;s.springNumber=3;assert(!b.resume(s,found));
    std::cout<<"PASS spring role/marker/unsupported guards, no attendance increase, "<<cases<<" score-independent shortened/full-game departure paths, monotonic resets, both-offense reactions and scoped v8 history\n";
}
