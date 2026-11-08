#include "../Source/ArrivalRules.h"
#include "../Source/CrowdVisibility.h"
#include "../Source/MatchLifecycle.h"
#include <cassert>
#include <vector>
#include <iostream>
using namespace attendance;
int main(){
    {
        attendance::arrivals::Settings fallback;
        attendance::State holyWar{1,0,0,6,720,720};
        holyWar.team0=1218;holyWar.team1=1116;holyWar.rank0=6;holyWar.rank1=12;
        holyWar.rivalry=true;holyWar.rivalryEvent=27;holyWar.precipitation=2;
        assert(attendance::arrivals::openingPercent(fallback,holyWar)>=70);
        assert(attendance::arrivals::openingPercent(fallback,holyWar)<=80);
        assert(attendance::arrivals::finishFraction(fallback,holyWar)==.30f);
        // Event identity, rather than the participant IDs, selects the policy.
        holyWar.team0=150;holyWar.team1=151;
        assert(attendance::arrivals::openingPercent(fallback,holyWar)>=70);
        holyWar.rivalry=false;assert(attendance::arrivals::openingPercent(fallback,holyWar)<70);
    }
    arrivals::CompletionNotice notice;assert(!notice.settled(0,0,true));notice.targetReached();
    assert(!notice.settled(10,0,true)&&!notice.settled(0,0,false));
    assert(notice.settled(0,0,true)&&!notice.settled(0,0,true));
    notice.targetReached();assert(!notice.settled(12,7,true)&&notice.settled(7,7,true)); // Permanent departures are not arrivals.
    notice.targetReached();notice.reset();assert(!notice.settled(0,0,true));
    arrivals::Settings c;arrivals::Model m;State s{1,0,0,6,720,720};
    const int severe[]={1212,1158,1137,1156,1218,1216,1223};
    const int formerModerate[]={1189,1113,1117,1197,1182,1154,1132};
    for(int team:severe){s.team0=team;assert(arrivals::schoolGroup(s)==arrivals::Group::Severe&&arrivals::openingPercent(c,s)==45&&arrivals::finishFraction(c,s)==.75f);}
    for(int team:formerModerate){s.team0=team;assert(arrivals::schoolGroup(s)==arrivals::Group::Standard&&arrivals::openingPercent(c,s)==55&&arrivals::finishFraction(c,s)==.5f);}
    s.team0=1218;s.neutral=true;assert(arrivals::openingPercent(c,s)==55&&arrivals::finishFraction(c,s)==.5f);s.neutral=false;
    for(int minutes=1;minutes<=20;++minutes)for(int team:{1212,1218,1154,-1}){
        m.reset();s={1,0,0,6,minutes*60.0f,minutes*60.0f};s.team0=team;
        int start=arrivals::lateSchool(s)?45:55;auto r=m.observe(c,s);assert(r.starting&&r.missing==100-start);
        float finish=m.fillSeconds;s.phase=1;s.seconds=s.quarterSeconds-finish/2;
        int halfway=m.observe(c,s).missing;assert(halfway>0&&halfway<100-start);s.seconds=s.quarterSeconds;assert(!m.observe(c,s).changed);
        s.seconds=(std::max)(0.f,s.quarterSeconds-finish-.1f);assert(m.observe(c,s).finished&&m.missing==0);
    }
    for(int team:{-1,1218,1154})for(int wet:{0,1,2})for(int home:{0,1,6,25,50,127,255})for(int away:{0,1,11,25,50,127,255})for(bool rival:{false,true}){
        s={1,0,0,6,720,720};s.team0=team;s.precipitation=wet;s.rank0=home;s.rank1=away;s.rivalry=rival;
        int start=arrivals::openingPercent(c,s);assert(start>=(wet?35:40)&&start<=80);
        float finish=arrivals::finishFraction(c,s);assert(std::fabs(finish-(team==1218?.75f:turnout::big(home,away,rival)?.3f:.5f))<.00001f);
        State rain=s;rain.precipitation=1;assert(arrivals::openingPercent(c,rain)<=start);
    }
    s={1,0,0,6,720,720};s.rivalry=true;assert(arrivals::openingPercent(c,s)==55);s.rank0=34;s.rank1=47;assert(arrivals::openingPercent(c,s)==58);
    s.rank1=51;assert(arrivals::openingPercent(c,s)==55);s.rank0=25;assert(arrivals::openingPercent(c,s)==58);
    for(int bowl:{20,37,47})for(int team:severe){s={1,0,0,6,720,720};s.team0=team;s.playoff=bowl>=37;s.bowl=bowl;int low=bowl==47?80:70;assert(arrivals::openingPercent(c,s)>=low&&arrivals::openingPercent(c,s)<=low+10&&arrivals::finishFraction(c,s)==.2f);}
    s={1,0,0,6,720,720};s.conferenceChampionship=true;assert(arrivals::openingPercent(c,s)>=80);s.precipitation=2;assert(arrivals::openingPercent(c,s)>=80);
    s={1,0,0,6,720,720};s.neutral=true;s.rank0=5;s.rank1=19;assert(arrivals::openingPercent(c,s)==arrivals::openingPercent(c,reversed(s)));
    m.reset();s={1,0,0,6,720,720};auto r=m.observe(c,s);assert(r.missing==45);s.phase=3;s.seconds=600;assert(!m.observe(c,s).changed&&m.missing==45);
    s.phase=5;assert(m.observe(c,s).missing>0&&m.missing<45);s.seconds=360;assert(m.observe(c,s).finished);
    m.reset();s={1,0,0,5,200,720};assert(!m.observe(c,s).changed&&m.done);
    m.reset();s={4,17,17,5,720,720};assert(!m.observe(c,s).changed&&m.done);
    m.reset();s={1,0,0,6,720,720};m.observe(c,s);assert(m.finish().finished&&m.missing==0);
    m.reset();c.enabled=0;assert(!m.observe(c,s).changed);c.enabled=1;
    m.reset();c.startPercent=100;assert(m.observe(c,s).missing==40);c.startPercent=55;
    s={0,0,0,6,0,720};assert(normalizePregame(s)&&s.quarter==1&&s.seconds==720&&valid(s));
    State invalid=s;invalid.quarter=2;assert(!normalizePregame(invalid));invalid=s;invalid.homeScore=7;assert(!normalizePregame(invalid));
    invalid=s;invalid.phase=5;assert(!normalizePregame(invalid));invalid=s;invalid.quarterSeconds=NAN;assert(!normalizePregame(invalid));
    MatchLifecycle lifecycle;assert(!lifecycle.needsReset(1,true));lifecycle.join(1,true);
    for(int i=0;i<100;i++){assert(!lifecycle.needsReset(1,true));lifecycle.join(1,true);assert(lifecycle.keepAtKickoff(1));}
    assert(!lifecycle.keepAtKickoff(2)&&lifecycle.needsReset(2,true));
    lifecycle.live();assert(!lifecycle.keepAtKickoff(1)&&!lifecycle.needsReset(1,false)&&lifecycle.needsReset(1,true));
    lifecycle.reset();lifecycle.join(1,true);lifecycle.leave();assert(lifecycle.needsReset(1,true)&&!lifecycle.keepAtKickoff(1));
    lifecycle.reset();lifecycle.join(2,false);assert(!lifecycle.keepAtKickoff(2));
    State intro{1,0,0,6,720,720};lifecycle.reset();lifecycle.join(10,true);
    assert(lifecycle.carryOpening(11,intro,true)&&lifecycle.keepAtKickoff(11));
    intro.phase=1;assert(lifecycle.carryOpening(12,intro,true)&&lifecycle.keepAtKickoff(12));
    assert(!lifecycle.carryOpening(13,intro,false));
    intro.seconds=710;assert(!lifecycle.carryOpening(13,intro,true));intro.seconds=720;
    lifecycle.live();assert(!lifecycle.carryOpening(13,intro,true));
    lifecycle.reset();lifecycle.join(10,true);lifecycle.leave();assert(!lifecycle.carryOpening(11,intro,true));
    lifecycle.reset();lifecycle.join(10,true);intro.homeScore=7;assert(!lifecycle.carryOpening(11,intro,true));
    // Newly allocated renderer masks can start hidden until the native input arrives.
    assert(!hidePart(false,false,true,false,false,false,false,true));
    assert(hidePart(true,true,true,false,false,false,false,true));
    assert(hidePart(false,false,true,true,false,false,false,true));
    arrivals::Halftime half;
    for(int minutes=1;minutes<=20;++minutes){
        half.reset();float q=static_cast<float>(minutes*60),before=q/4,after=q/4;
        s={2,14,7,5,before+0.1f,q};assert(half.observe(s,5)==0);
        s.seconds=before;assert(half.observe(s,5)==5&&half.active);
        s.seconds=before/2;assert(half.observe(s,5)==16);
        s.seconds=before;assert(half.observe(s,5)==16); // Replay cannot unwind the ramp.
        s.seconds=0;assert(half.observe(s,5)==27);
        s.quarter=3;s.seconds=q;assert(half.observe(s,42)==56);
        s.seconds=q-after/2;assert(half.observe(s,42)==49);
        s.seconds=q-after-0.1f;assert(half.observe(s,42)==0&&half.done);
        s.quarter=2;s.seconds=0;assert(half.observe(s,42)==0);
        half.reset();s={3,0,0,5,q,q};assert(half.observe(s,0)==0); // No invented earlier absences.
    }
    half.reset();s={2,0,49,5,0,720};assert(half.observe(s,90)==90);
    half.reset();s.playoff=true;s.bowl=47;assert(half.observe(s,20)==20);
    half.reset();s.precipitation=1;assert(half.observe(s,20)==30);
    half.reset();s={2,14,7,5,0,720};assert(half.observe(s,5,50)==16);
    assert(half.observe(s,5,0)==0&&!half.active);
    // Simulate a stadium that the GAME only filled to 8,000 of 10,000 seats.
    // Arrivals may reveal only seats this plugin temporarily withheld.
    std::vector<bool> original(10000),current(10000),owned(10000);
    for(unsigned i=0;i<10000;i++)original[i]=current[i]=i>=8000;
    unsigned previous=0;
    for(int missing:{15,14,12,10,8,6,4,2,0}){
        unsigned visible=0;
        for(unsigned i=0;i<10000;i++){
            bool hold=departed(i,missing),wasOwned=owned[i];
            current[i]=hidePart(original[i],original[i],current[i],hold,false,true,wasOwned);
            if(hold&&!original[i])owned[i]=true;else if(!hold)owned[i]=false;
            assert(!original[i]||current[i]);
            if(!current[i])++visible;
        }
        assert(visible>=previous&&visible<=8000);previous=visible;
    }
    assert(previous==8000);
    // Halftime return simulation: temporary 10-point absence, main departure,
    // then only those still-temporary seats refill. Original empties survive.
    auto visit=[&](int permanent,int temporary){
        unsigned visible=0;
        for(unsigned i=0;i<10000;i++){
            bool left=departed(i,permanent),held=departed(i,temporary),wasOwned=owned[i];
            current[i]=hidePart(original[i],original[i],current[i],left||held,false,true,wasOwned);
            if(held&&!left&&!original[i])owned[i]=true;else if(!held)owned[i]=false;
            if(original[i]||left)assert(current[i]);if(!current[i])++visible;
        }
        return visible;
    };
    auto early=visit(5,15),halftime=visit(42,52),returned=visit(42,0);
    assert(early<8000&&halftime<early&&returned>halftime&&returned<early);
    assert(visit(42,0)==returned);previous=returned;
    // Departures then take over permanently: no comeback refill.
    for(int loss:{1,8,30,70}){
        unsigned visible=0;
        for(unsigned i=0;i<10000;i++){
            bool before=current[i];current[i]=hidePart(original[i],original[i],current[i],departed(i,loss),false);
            assert(!before||current[i]);if(!current[i])++visible;
        }
        assert(visible<=previous);previous=visible;
    }
    assert(hidePart(false,false,true,false,false,true,false)); // Unowned hidden seats stay hidden.
    assert(hidePart(true,false,true,false,false,true,true)); // Native hidden state wins.
    assert(hidePart(false,true,true,false,false,true,true)); // Original empty seats stay empty.
    assert(hidePart(false,false,true,true,false,true,true)); // Permanent departures beat owned arrivals.
    std::cout<<"PASS: first-quarter arrivals, pauses/replays, late attaches, between-play updates, original crowd ceiling, owned visibility restoration and permanent departures.\n";
}
