#pragma once
#include <deque>
#include <iomanip>
#include <istream>
#include <ostream>
#include <string>
#include <tuple>
#include "AwayCrowd.h"

namespace attendance::visitors {
struct FixtureKey {
    int dynasty=0,home=-1,away=-1,homeRow=-1,awayRow=-1,stadium=-1,week=-1,year=-1;
    auto fields()const{return std::tie(dynasty,home,away,homeRow,awayRow,stadium,week,year);}
    bool operator==(const FixtureKey& other)const{return fields()==other.fields();}
    bool valid()const{return dynasty!=0&&home>=0&&home<2047&&away>=0&&away<2047&&home!=away&&
        homeRow>=0&&homeRow<=1000000&&awayRow>=0&&awayRow<=1000000&&homeRow!=awayRow&&
        stadium>=0&&stadium<1023&&week>=0&&week<=31&&year>=0&&year<=4095;}
};
inline FixtureKey key(const turnout::Seed& s){return {s.dynasty,s.home.id,s.away.id,s.home.recordRow,s.away.recordRow,s.stadium,s.week,s.year};}
struct FixturePlan {FixtureKey key;float share=-1;unsigned seed=0;uint64_t history=0;unsigned layoutVersion=1;int attendance=0,capacity=0,kickoff=0,fill=0;bool exceptional=false;};
inline bool valid(const FixturePlan& p){return p.key.valid()&&std::isfinite(p.share)&&
    ((p.share>=.01f&&p.share<=.70f)||(p.share==-1&&p.seed==0&&p.history!=0))&&(p.layoutVersion>=1&&p.layoutVersion<=4)&&
    (p.attendance==0&&p.capacity==0||(p.attendance>0&&p.capacity>=1000&&p.capacity<=200000&&p.attendance<=p.capacity&&p.kickoff>=0&&p.kickoff<=100&&p.fill>=0&&p.fill<=100));}
// Freeze the first verified plan for this exact fixture. The same selection seed
// regenerates the same outfits before a Midgame Save restores its clock/score.
// Neutral/disabled appearances can keep a history-only ownership record.
struct PlanBook {
    static constexpr size_t Maximum=256;
    std::deque<FixturePlan> entries;uint64_t revision=0;
    bool resolve(const FixtureKey& fixture,const Plan& current,unsigned seed,Plan& out,unsigned& selectedSeed,bool& reused){
        reused=false;if(!fixture.valid()||current.native)return false;
        for(auto& entry:entries)if(entry.key==fixture){
            if(entry.share<0&&entry.layoutVersion==1){out=current;out.valid=out.native=true;out.layoutVersion=1;selectedSeed=0;reused=true;return true;}
            if(entry.share<0){FixturePlan proposed{fixture,current.share,seed,entry.history};if(!current.valid||!valid(proposed))return false;proposed.layoutVersion=current.layoutVersion;entry=proposed;++revision;out=current;selectedSeed=seed;return true;}
            out=current;out.valid=true;out.share=entry.share;out.layoutVersion=entry.layoutVersion;selectedSeed=entry.seed;reused=true;return true;
        }
        FixturePlan entry{fixture,current.share,seed};if(!current.valid||!valid(entry))return false;
        entry.layoutVersion=current.layoutVersion;entries.push_back(entry);while(entries.size()>Maximum)entries.pop_front();++revision;
        out=current;selectedSeed=seed;return true;
    }
    const FixturePlan* find(const FixtureKey& fixture)const{if(fixture.valid())for(const auto& e:entries)if(e.key==fixture)return &e;return nullptr;}
    bool rebuild(const FixtureKey& fixture){
        if(!fixture.valid())return false;
        auto at=std::find_if(entries.begin(),entries.end(),[&](const FixturePlan& p){return p.key==fixture;});
        if(at==entries.end())return false;entries.erase(at);++revision;return true;
    }
    void attendance(const FixtureKey& fixture,const turnout::Result& r,int capacity){for(auto& e:entries)if(e.key==fixture&&e.attendance==0&&r.applied){e.attendance=r.attendance;e.capacity=capacity;e.kickoff=r.kickoff;e.fill=r.fillPercent;e.exceptional=r.exceptional;++revision;return;}}
    void importMissing(const PlanBook& legacy){for(const auto& e:legacy.entries){bool found=false;for(const auto& current:entries)if(current.key==e.key){found=true;break;}if(!found&&entries.size()<Maximum){entries.push_back(e);++revision;}}}
    uint64_t source(const FixtureKey& fixture)const{if(fixture.valid())for(const auto& e:entries)if(e.key==fixture)return e.history;return 0;}
    bool bind(const FixtureKey& fixture,uint64_t history,unsigned layoutVersion=1){
        if(!fixture.valid()||!history)return false;
        for(auto& e:entries)if(e.key==fixture){if(e.history!=history){e.history=history;++revision;}return true;}
        FixturePlan entry{fixture,-1,0,history};entry.layoutVersion=layoutVersion;entries.push_back(entry);while(entries.size()>Maximum)entries.pop_front();++revision;return true;
    }
    void write(std::ostream& f)const{
        f<<"DGA-VISITORS-3 "<<entries.size()<<'\n'<<std::setprecision(17);
        for(const auto& e:entries){const auto& k=e.key;
            f<<k.dynasty<<' '<<k.home<<' '<<k.away<<' '<<k.homeRow<<' '<<k.awayRow<<' '<<k.stadium<<' '<<k.week<<' '<<k.year<<' '<<e.share<<' '<<e.seed<<' '<<e.history<<' '<<e.layoutVersion<<' '<<e.attendance<<' '<<e.capacity<<' '<<e.kickoff<<' '<<e.fill<<' '<<e.exceptional<<'\n';}
    }
    static bool read(std::istream& f,PlanBook& target){
        PlanBook next;std::string magic;size_t count=0;
        if(!(f>>magic>>count)||(magic!="DGA-VISITORS-1"&&magic!="DGA-VISITORS-2"&&magic!="DGA-VISITORS-3")||count>Maximum)return false;
        for(size_t i=0;i<count;++i){FixturePlan e;auto& k=e.key;uint64_t seed=0;
            if(!(f>>k.dynasty>>k.home>>k.away>>k.homeRow>>k.awayRow>>k.stadium>>k.week>>k.year>>e.share>>seed)||seed>UINT32_MAX)return false;
            e.seed=static_cast<unsigned>(seed);
            if(magic!="DGA-VISITORS-1"&&!(f>>e.history))return false;
            if(magic=="DGA-VISITORS-3"){int exceptional=0;if(!(f>>e.layoutVersion>>e.attendance>>e.capacity>>e.kickoff>>e.fill>>exceptional)||exceptional<0||exceptional>1)return false;e.exceptional=exceptional!=0;}
            if(!valid(e))return false;
            for(const auto& prior:next.entries)if(prior.key==k)return false;
            next.entries.push_back(e);
        }
        f>>std::ws;if(!f.eof())return false;target=std::move(next);return true;
    }
};
inline bool linkedHistoryReady(const PlanBook& book,const std::vector<uint64_t>& live,const std::vector<uint64_t>& saved){
    for(const auto& e:book.entries)if(e.history&&std::find(live.begin(),live.end(),e.history)!=live.end()&&std::find(saved.begin(),saved.end(),e.history)==saved.end())return false;
    return true;
}
}
