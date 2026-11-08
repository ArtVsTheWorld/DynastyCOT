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
struct FixturePlan {FixtureKey key;float share=-1;unsigned seed=0;uint64_t history=0;};
inline bool valid(const FixturePlan& p){return p.key.valid()&&std::isfinite(p.share)&&
    ((p.share>=.01f&&p.share<=.70f)||(p.share==-1&&p.seed==0&&p.history!=0));}
// Freeze the first verified plan for this exact fixture. The same selection seed
// regenerates the same outfits before a Midgame Save restores its clock/score.
// Neutral/disabled appearances can keep a history-only ownership record.
struct PlanBook {
    static constexpr size_t Maximum=256;
    std::deque<FixturePlan> entries;uint64_t revision=0;
    bool resolve(const FixtureKey& fixture,const Plan& current,unsigned seed,Plan& out,unsigned& selectedSeed,bool& reused){
        reused=false;if(!fixture.valid()||current.native)return false;
        for(auto& entry:entries)if(entry.key==fixture){
            if(entry.share<0){FixturePlan proposed{fixture,current.share,seed,entry.history};if(!current.valid||!valid(proposed))return false;entry=proposed;++revision;out=current;selectedSeed=seed;return true;}
            out=current;out.valid=true;out.share=entry.share;selectedSeed=entry.seed;reused=true;return true;
        }
        FixturePlan entry{fixture,current.share,seed};if(!current.valid||!valid(entry))return false;
        entries.push_back(entry);while(entries.size()>Maximum)entries.pop_front();++revision;
        out=current;selectedSeed=seed;return true;
    }
    uint64_t source(const FixtureKey& fixture)const{if(fixture.valid())for(const auto& e:entries)if(e.key==fixture)return e.history;return 0;}
    bool bind(const FixtureKey& fixture,uint64_t history){
        if(!fixture.valid()||!history)return false;
        for(auto& e:entries)if(e.key==fixture){if(e.history!=history){e.history=history;++revision;}return true;}
        entries.push_back(FixturePlan{fixture,-1,0,history});while(entries.size()>Maximum)entries.pop_front();++revision;return true;
    }
    void write(std::ostream& f)const{
        f<<"DGA-VISITORS-2 "<<entries.size()<<'\n'<<std::setprecision(17);
        for(const auto& e:entries){const auto& k=e.key;
            f<<k.dynasty<<' '<<k.home<<' '<<k.away<<' '<<k.homeRow<<' '<<k.awayRow<<' '<<k.stadium<<' '<<k.week<<' '<<k.year<<' '<<e.share<<' '<<e.seed<<' '<<e.history<<'\n';}
    }
    static bool read(std::istream& f,PlanBook& target){
        PlanBook next;std::string magic;size_t count=0;
        if(!(f>>magic>>count)||(magic!="DGA-VISITORS-1"&&magic!="DGA-VISITORS-2")||count>Maximum)return false;
        for(size_t i=0;i<count;++i){FixturePlan e;auto& k=e.key;uint64_t seed=0;
            if(!(f>>k.dynasty>>k.home>>k.away>>k.homeRow>>k.awayRow>>k.stadium>>k.week>>k.year>>e.share>>seed)||seed>UINT32_MAX)return false;
            e.seed=static_cast<unsigned>(seed);
            if(magic=="DGA-VISITORS-2"&&!(f>>e.history))return false;
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
