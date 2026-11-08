#include "../Source/AttendanceRules.h"
#include "../Source/CrowdVisibility.h"
#include <vector>
#include <cassert>
#include <iostream>
int main(){
    using namespace attendance;Settings c;State s{1,0,0,1,300,300};
    assert(profileFor(c,s)==Profile::Default);
    s.precipitation=1;assert(profileFor(c,s)==Profile::BadWeather);
    s.precipitation=2;assert(profileFor(c,s)==Profile::BadWeather);
    s.rivalry=true;assert(profileFor(c,s)==Profile::BadWeather);
    s.rivalry=false;s.rank0=1;s.rank1=25;assert(profileFor(c,s)==Profile::Reduced);
    s.rank1=26;assert(profileFor(c,s)==Profile::BadWeather);
    c.profile=Profile::Default;assert(profileFor(c,s)==Profile::Default);c.profile=Profile::Automatic;
    for(int bowl=0;bowl<=127;++bowl)assert(playoffBowl(bowl)==(bowl>=37&&bowl<=53));
    for(int weather:{0,1,2})for(int kind:{0,1,2}){
        s.precipitation=weather;s.playoff=kind>0;s.bowl=kind==2?47:37;
        int floor=(kind==2?80:kind==1?60:25)-(weather?10:0);assert(lossCeiling(s)==100-floor);
    }
    for(int rank:{1,25,26,0,-1})for(int margin:{-35,0,35})for(int weather:{0,1,2})for(bool neutral:{false,true}){
        s=State{4,35,35-margin,5,60,720};s.rank0=rank;s.rank1=127;s.precipitation=weather;s.neutral=neutral;
        bool rankedWinner=!neutral&&rank>=1&&rank<=25&&margin>0;
        assert(lossCeiling(s)==(rankedWinner?65:weather?85:75));
        s.team1=1500;assert(lossCeiling(s)==(rankedWinner?65:weather?85:75));
        s.playoff=true;s.bowl=37;assert(lossCeiling(s)==(weather?50:40));
        s.bowl=47;assert(lossCeiling(s)==(weather?30:20));
    }
    s=State{3,0,35,1,300,300};s.quarter=0;assert(!valid(s));s.quarter=3;s.seconds=NAN;assert(!valid(s));
    s.seconds=302;assert(!valid(s));s.seconds=300;s.homeScore=-1;assert(!valid(s));
    assert(effectiveRank(0,1500)==256&&effectiveRank(-1,1504)==256&&effectiveRank(0,1158)==0);
    assert(effectiveRank(255,1158)==255&&effectiveRank(50,1500)==50);
    s={3,0,35,1,300,300};s.rank0=1;s.rank1=50;float gap=relativeGap(s);
    s.rank1=127;assert(relativeGap(s)>gap);s.rank1=0;s.team1=1500;assert(relativeGap(s)>gap);
    for(int phase:{7,8}){
        s={6,34,33,phase,0,300};assert(elapsedGameTime(s)==1200);
        s.quarterSeconds=0;assert(untimedPeriod(s)&&normalizeUntimedPeriod(s,300));
        normalizeFinalPresentation(s);assert(s.phase==7&&elapsedGameTime(s)==1200);
    }
    s={4,34,33,8,0,300};normalizeFinalPresentation(s);assert(s.phase==8); // Scope only verified EndOfGame.
    // Regression: a partially filled stadium must never grow to a full-capacity target.
    std::vector<bool> starting(10000),current(10000);
    for(unsigned fan=0;fan<starting.size();++fan)current[fan]=starting[fan]=fan>=4000;
    for(int loss:{1,4,8,25,40,52}){
        unsigned visible=0;
        for(unsigned fan=0;fan<starting.size();++fan){
            bool before=current[fan];
            current[fan]=hidePart(false,starting[fan],current[fan],departed(fan,loss),false);
            assert(!before||current[fan]);
            if(!current[fan])++visible;
        }
        assert(visible<=4000); // Never repopulate the 6,000 original empty seats.
        assert(visible<=static_cast<unsigned>(4000*(100-loss)/100+100));
    }
    for(unsigned fan=0;fan<starting.size();++fan){
        current[fan]=hidePart(false,starting[fan],current[fan],false,true);
        assert(current[fan]==starting[fan]); // Restore only the starting crowd.
    }
    assert(hidePart(true,false,false,false,false)); // Preserve native hidden parts too.
    std::cout<<"PASS: profile precedence, updated weather/postseason floors, full rankings/FCS ordering and sparse-crowd regression\n";
}
