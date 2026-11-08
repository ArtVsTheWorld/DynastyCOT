#include "../Source/AttendanceTimeline.h"
#include "../Source/SeatPattern.h"
#include <cassert>
#include <sstream>
#include <iostream>
using namespace attendance;
int halftimeLoss(State s){
    Settings c;History h;departures::Model model;int loss=0;
    int q=static_cast<int>(s.quarterSeconds);
    for(int t=0;t<=2*q;++t){s.quarter=t<q?1:t<2*q?2:3;s.seconds=s.quarter*s.quarterSeconds-t;s.phase=5;
        assert(h.observe(c,s)&&model.observe(c,s,h));loss=model.settle(s,loss).loss;}
    return loss;
}
int main(){
    Settings c;State s{2,0,0,5,0,720};
    struct Band {int margin;float low,high,tempLow,tempHigh;};
    for(auto band:{Band{0,3,5,15,20},{8,3,5,15,20},{17,10,20,15,25},{21,10,20,15,25},
            {24,10,20,15,25},{25,20,30,15,25},{28,20,30,15,25},{34,20,30,15,25},{35,30,40,10,20},{56,30,40,10,20}}){
        for(bool winner:{false,true})for(int wet:{0,1,2})for(unsigned seed:{0u,1u,999u}){
            s.homeScore=winner?band.margin:0;s.awayScore=winner?0:band.margin;s.precipitation=wet;s.variationSeed=seed;
            auto target=context::halftime(s);
            assert(target.permanent>=band.low&&target.permanent<=band.high&&target.temporary>=20&&target.temporary<=30);
        }
    }
    for(int minutes=1;minutes<=20;++minutes)for(int margin:{0,17,21,28,35}){
        s={1,0,margin,5,static_cast<float>(minutes*60),static_cast<float>(minutes*60)};
        auto target=context::halftime(s);int loss=halftimeLoss(s);
        assert(loss>=target.permanent-2&&loss<=target.permanent+(margin>=25?5:2)); // Early home-rout exits may modestly exceed the former halftime-only target. // Actual accumulated exits, not just target labels.
    }
    s={2,7,7,5,0,720};auto defensive=context::halftime(s);s.homeScore=s.awayScore=17;
    assert(context::halftime(s).permanent==defensive.permanent);
    s.rivalry=true;assert(context::halftime(s).permanent==defensive.permanent);s.rivalry=false;s.rank0=6;s.rank1=11;
    assert(context::halftime(s).permanent<=5);
    for(int bowl:{37,47}){s.playoff=true;s.bowl=bowl;s.awayScore=56;assert(context::halftime(s).permanent<=5&&halftimeLoss(s)<=5);}
    // Temporary absence is a share of the people present. It is frozen at
    // halftime and can only return during Q3, even when the score changes.
    s={2,0,21,5,0,720};arrivals::Halftime seats;
    int absentFull=seats.observe(s,10)-10;seats.reset();int absentSparse=seats.observe(s,55)-55;
    assert(absentFull>absentSparse&&absentSparse>0);
    s.quarter=3;s.seconds=660;int first=seats.observe(s,55)-55;s.awayScore=42;s.seconds=650;
    assert(seats.observe(s,55)-55<=first);
    s.seconds=539;assert(seats.observe(s,55)==0&&seats.done);
    // Home celebrations retain some patience even against FCS, with the
    // same hard ranked-home floor. A losing crowd gets no such benefit.
    s={4,35,0,5,360,720};s.team1=1500;s.rank0=6;
    assert(departures::homePatience(s,2520)<1&&departures::homePatience(s,2520)>0.405f&&lossCeiling(s)==65);
    State losing=s;losing.homeScore=0;losing.awayScore=35;assert(departures::homePatience(losing,2520)==1);
    assert(context::separation(State{1,0,0,5,720,720})==0);
    s.rank0=1;s.rank1=51;s.team1=-1;float topGap=context::separation(s);s.rank0=70;s.rank1=120;assert(context::separation(s)<topGap);
    // Possession and remaining timeouts alter hopelessness without trusting
    // missing data. Failed late possession removes comeback protection.
    s={4,14,35,5,45,720};s.homeTimeouts=0;s.offense=1;float hopeless=context::lateOpportunity(s);
    s.offense=0;s.toGoal=10;assert(context::lateOpportunity(s)<1&&hopeless>1);
    s.offense=-1;assert(context::lateOpportunity(s)==1);
    History h;s={3,0,21,5,720,720};h.observe(c,s);s.homeScore=14;s.seconds=650;h.observe(c,s);assert(h.comeback);
    s.quarter=4;s.seconds=45;s.awayScore=28;s.offense=1;s.homeTimeouts=0;h.observe(c,s);assert(!h.comeback);
    // The transition budget is shared across checkpoint, late-score and
    // ordinary settlement. Earned exits are reconciled at the next window;
    // protection halts new decisions without changing earlier earned exits.
    departures::Model model;s={4,0,35,5,600,720};model.observe(c,s,h);model.hazard=0.7;
    model.beginWindow(s,4);auto result=model.settle(s,0);assert(result.loss==4);
    assert(model.settle(s,4).loss==4);model.beginWindow(s,4);assert(model.settle(s,4).loss==4);
    s.seconds=590;model.beginWindow(s,4);int earned=model.plannedLoss;
    assert(earned==45&&model.settle(s,4).loss==earned);
    h.comeback=true;model.observe(c,s,h);model.beginWindow(4);assert(model.settle(s,earned).loss==earned);
    h.reset();model.reset();model.observe(c,s,h);model.hazard=10;model.beginWindow(100);
    assert(model.settle(s,0).loss==45); // Hidden Sim still respects quarter cap.
    // Saved games retain the same variation seed; old history remains readable.
    timeline::Archive archive;s.variationSeed=123456;s.team0=1165;s.team1=1158;archive.begin(9,s);timeline::Point p;p.state=s;p.loss=8;p.time=1;archive.record(p);
    std::stringstream disk;timeline::write(disk,archive);timeline::Archive loaded;assert(timeline::read(disk,loaded));
    assert(loaded.games.front().context.variationSeed==123456);
    p.state={3,14,14,5,600,720};p.state.team0=1165;p.state.team1=1158;p.state.variationSeed=123456;p.loss=8;p.temporary=8;
    State current=p.state;current.variationSeed=999;
    auto restored=timeline::estimate(c,arrivals::Settings{},current,&p);
    assert(restored.halftime.done&&restored.halftime.observe(current,8)==0&&restored.departures.previous.variationSeed==123456);
    p.temporary=12;restored=timeline::estimate(c,arrivals::Settings{},current,&p);
    current.seconds=590;assert(restored.halftime.observe(current,8)<=12);
    std::stringstream legacy("DGA-HISTORY-1 1\n9 1 2 720 0 127 0 0 0 0 0 0\n");assert(timeline::read(legacy,loaded));
    assert(loaded.games.front().context.variationSeed==0);
    std::stringstream badSeed("DGA-HISTORY-2 1\n9 1 2 720 0 127 0 0 0 0 0 0 4294967296\n");assert(!timeline::read(badSeed,loaded));
    // Non-rival marquee eligibility needs both Top25; a ranked rivalry keeps
    // its opening bonus even with a weaker opponent. Disabled bonuses stay off.
    arrivals::Settings arrival;s={1,0,0,6,720,720};s.rank0=25;s.rank1=25;
    int matched=arrivals::openingPercent(arrival,s);s.rank1=127;
    assert(arrivals::openingPercent(arrival,s)<70&&arrivals::openingPercent(arrival,s)<matched);
    s.rivalry=true;int wideRivalry=arrivals::openingPercent(arrival,s);s.rank1=25;
    assert(arrivals::openingPercent(arrival,s)==wideRivalry);s.rivalry=false;s.rank1=127;
    std::swap(s.rank0,s.rank1);assert(arrivals::openingPercent(arrival,s)<70);
    s.rank0=25;int previousOpening=-1;
    for(int rank=25;rank<=255;++rank){s.rank1=rank;int opening=arrivals::openingPercent(arrival,s);
        assert(opening>=30&&(previousOpening<0||opening<=previousOpening));previousOpening=opening;}
    for(int team:{1212,1154,-1})for(int wet:{0,1,2}){s.team0=team;s.team1=1501;s.precipitation=wet;
        assert(arrivals::openingPercent(arrival,s)>=30);}
    s={1,0,0,6,720,720};s.rivalry=true;arrival.bigGameBonus=0;
    assert(arrivals::openingPercent(arrival,s)==55);
    s.rivalry=false;s.playoff=true;s.bowl=47;
    assert(arrivals::openingPercent(arrival,s)==80);
    // The halftime peak is initialized even if no selection occurred during
    // the early seat-break ramp. A restored Q3 return never restarts it.
    arrivals::Halftime halftime;s={2,7,7,5,200,720};halftime.observe(s,8);
    s.quarter=3;s.seconds=720;assert(halftime.observe(s,8)>8);
    int peak=halftime.lastAbsent;s.seconds=710;assert(halftime.observe(s,8)-8<=peak);
    halftime.reset();s={2,0,35,5,0,720};s.playoff=true;s.bowl=47;
    assert(halftime.observe(s,18)==20);s.quarter=3;s.seconds=720;s.precipitation=1;
    assert(halftime.observe(s,18)==20); // Relaxing a floor cannot invent returns to the concourse.
    halftime.reset();s.quarter=2;s.seconds=1;s.precipitation=0;assert(halftime.observe(s,18)==20);
    s.quarter=3;s.seconds=720;s.precipitation=1;assert(halftime.observe(s,18)==20);
    // Winning against a comparable school gets extra patience. A huge
    // mismatch keeps its previous severe-exit behavior.
    s={3,35,0,5,720,720};s.rank0=50;s.rank1=60;
    float comparable=context::halftime(s).permanent,patient=departures::homePatience(s,1440);
    s.rank0=25;s.rank1=127;
    assert(context::halftime(s).permanent>comparable&&departures::homePatience(s,1440)>patient);
    s.team1=1500;assert(context::comparableHomeWin(s)==0);
    // Long drives receive a time-scaled allowance. All visible changes share
    // the same anchored band, and duplicate callbacks cannot renew it.
    for(int minutes=1;minutes<=20;++minutes){
        model.reset();s={2,0,35,5,static_cast<float>(minutes*60),static_cast<float>(minutes*60)};
        model.beginWindow(s,4);model.anchorVisible(8,20);
        assert(model.paceTemporary(8,0)==16&&model.paceTemporary(8,80)==24);
        s.seconds-=s.quarterSeconds/6;model.beginWindow(s,4);model.anchorVisible(8,20);
        assert(model.visibleBudget==16&&model.paceTemporary(8,80)==36);
        int upper=model.visibleHigh;model.beginWindow(s,4);model.anchorVisible(8,36);
        assert(model.visibleHigh==upper);
        model.hazard=2;auto paced=model.settle(s,8);assert(paced.loss<=24);
        assert((std::max)(paced.loss,model.paceTemporary(paced.loss,80))<=36);
    }
    // Even with possession, two scores and no timeouts can end a failed rally.
    h.reset();s={3,0,28,5,720,720};h.observe(c,s);s.homeScore=21;s.seconds=600;h.observe(c,s);assert(h.comeback);
    s.quarter=4;s.awayScore=35;s.seconds=30;s.offense=0;s.homeTimeouts=0;s.toGoal=15;
    h.observe(c,s);assert(!h.comeback);h.observe(c,s);assert(!h.comeback);
    // Exact counts and nested groups survive seed-dependent horizontal timing.
    std::vector<seats::Position> positions;
    for(int side:{-1,1})for(int row=0;row<32;++row)for(int seat=0;seat<24;++seat)
        positions.push_back({side*(28.0f+row*.9f),1+row*.65f,-48+seat*4.0f,0});
    std::vector<unsigned short> original(positions.size());auto a=seats::build(positions,original,3,1),b=seats::build(positions,original,3,2);
    assert(a.policy==seats::Policy::Position&&a.rank!=b.rank&&a.rank==seats::build(positions,original,3,1).rank);
    unsigned kept[8]{},capacity[8]{};
    for(unsigned i=0;i<positions.size();++i){unsigned section=(i/(32*24))*4+(i%24)/6;
        if((i/24)%32<8){++capacity[section];kept[section]+=!a.departed(i,50);}}
    double low=1,high=0;for(unsigned section=0;section<8;++section){double density=static_cast<double>(kept[section])/capacity[section];low=(std::min)(low,density);high=(std::max)(high,density);}
    assert(high-low>0.03); // Horizontal differences also reach the close seating.
    for(int loss:{10,25,50,75,90}){unsigned count=0;for(unsigned i=0;i<positions.size();++i){count+=a.departed(i,loss);assert(!a.departed(i,loss-1)||a.departed(i,loss));}
        assert(count==positions.size()*loss/100);}
    std::cout<<"PASS: halftime bands and realized losses, current-crowd breaks, stable returns, FCS patience, relative rank quality, late possession, visible budgets, hidden caps, history seeds and nested section variation.\n";
}
