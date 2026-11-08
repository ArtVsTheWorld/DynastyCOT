#pragma once
#include "AudienceCalibration.h"
#include "AudienceTurnoutEvidence.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

namespace attendance::audience {
struct Season {int wins=0,losses=0,ties=0;};
struct Program {
    int id=-1,row=-1,year=-1,prestige=-1,atmosphere=-1,ap=0;
    bool custom=false,identityKnown=false;
    std::array<Season,3> history{}; // Previous completed seasons, newest first.
};
inline bool valid(const Program& p){return p.id>=0&&p.id<2047&&p.row>=0&&p.row<=1000000&&p.year>=0&&p.year<=100&&p.prestige>=0&&p.prestige<=10&&p.atmosphere>=0&&p.atmosphere<=12&&p.ap>=0&&p.ap<=256;}
inline float recent(const Program& p){
    constexpr float weights[]={.5f,.3f,.2f};float sum=0,total=0;
    for(int i=0;i<3;++i){
        if(p.custom&&i>=p.year)continue; // Never borrow pre-dynasty TeamBuilder results.
        const auto& r=p.history[i];if(r.wins<0||r.wins>30||r.losses<0||r.losses>30||r.ties<0||r.ties>30)continue;
        int games=r.wins+r.losses+r.ties;if(games<1||games>30)continue;
        sum+=weights[i]*(r.wins+.5f*r.ties)/games;total+=weights[i];
    }
    return total>0?sum/total:-1.f;
}
inline float apStrength(int rank){return rank>=1&&rank<=138?1.f-(rank-1)/137.f:0.f;}
inline std::array<float,4> features(const Program& p){return {p.prestige/10.f,(12-p.atmosphere)/12.f,recent(p),p.ap>0?apStrength(p.ap):-1.f};}
inline constexpr std::array<float,4> Weights{.40f,.30f,.20f,.10f};
struct Foundation {double fans=0;std::array<float,4> reference{};bool peers=false;};
inline Foundation peerFoundation(const Program& p){
    struct Peer {double distance,weight;const Calibration* school;};
    std::vector<Peer> peers;auto f=features(p);
    for(const auto& c:CalibrationSchools){
        if(!p.custom&&c.id==p.id)continue;
        double d=(.4*std::abs(f[0]-c.prestige/10.f)+.3*std::abs(f[1]-(12-c.atmosphere)/12.f))/.7;
        peers.push_back({d,1/((d+.05)*(d+.05)),&c});
    }
    std::sort(peers.begin(),peers.end(),[](const Peer& a,const Peer& b){return a.distance<b.distance;});
    double cutoff=peers[24].distance;peers.erase(std::remove_if(peers.begin(),peers.end(),[&](const Peer& a){return a.distance>cutoff+1e-7;}),peers.end());
    std::array<float,4> reference{};std::array<double,4> sums{},weights{};
    for(const auto& x:peers){const auto& c=*x.school;std::array<float,4> cf{c.prestige/10.f,(12-c.atmosphere)/12.f,c.history,apStrength(c.ap)};
        for(int i=0;i<4;++i)if(cf[i]>=0){sums[i]+=x.weight*cf[i];weights[i]+=x.weight;}}
    for(int i=0;i<4;++i)reference[i]=weights[i]>0?static_cast<float>(sums[i]/weights[i]):-1.f;
    std::sort(peers.begin(),peers.end(),[](const Peer& a,const Peer& b){return a.school->referenceFans<b.school->referenceFans;});
    double total=0,walk=0,base=0;for(const auto& x:peers)total+=x.weight;
    for(const auto& x:peers){walk+=x.weight;if(walk>=total*.5){base=x.school->referenceFans;break;}}
    float delta=0,active=0;for(int i=0;i<4;++i)if(f[i]>=0&&reference[i]>=0){delta+=Weights[i]*(f[i]-reference[i]);active+=Weights[i];}
    double factor=(std::clamp)(std::exp(2.0*delta/active),.70,1.35);
    // Freeze the program's starting attributes. Peer history is an expectation
    // reference only, never a fabricated season in the program's history.
    auto initial=f;for(int i=0;i<4;++i)if(initial[i]<0)initial[i]=reference[i];
    return {base*factor,initial,true};
}
inline Foundation foundation(const Program& p){
    if(!valid(p)||!p.identityKnown)return {};
    if(!p.custom)for(const auto& c:CalibrationSchools)if(c.id==p.id){
        auto reference=p;reference.prestige=c.prestige;reference.atmosphere=c.atmosphere;reference.ap=c.ap;reference.history={};
        auto expected=peerFoundation(reference).reference;
        return {c.referenceFans,{c.prestige/10.f,(12-c.atmosphere)/12.f,c.history>=0?c.history:expected[2],apStrength(c.ap)},false};
    }
    return peerFoundation(p);
}
inline double demand(const Foundation& base,const Program& p){
    if(!valid(p)||!p.identityKnown||!std::isfinite(base.fans)||base.fans<=0)return 0;
    auto f=features(p);float delta=0,total=0;
    for(int i=0;i<4;++i)if(f[i]>=0&&base.reference[i]>=0){delta+=Weights[i]*(f[i]-base.reference[i]);total+=Weights[i];}
    if(total<=0)return 0;
    // No annual growth bonus or repeated compounding. Equal current attributes
    // always give the same audience, including after a restart or a rewind.
    return (std::clamp)(base.fans*std::exp(2.0*delta/total),1.0,1000000.0);
}
inline float opponentRetention(const Foundation& base,const Program& p){
    if(p.custom||base.peers)return 0;
    for(const auto& c:CalibrationSchools)if(c.id==p.id){
        for(const auto& e:TurnoutHistory)if(e.id==p.id){
            double x=(std::clamp)((static_cast<double>(e.lowerHistoricalFill)-.85)/.15,0.0,1.0);
            return static_cast<float>(x*x*(3-2*x)*(std::clamp)(demand(base,p)/c.referenceFans,0.0,1.0));
        }
    }
    return 0;
}
struct Entry {int dynasty=0,row=-1,id=-1,born=-1;bool custom=false;Foundation base;};
class Book {
    std::vector<Entry> entries_;
public:
    uint64_t revision=0;
    bool upgraded=false;size_t migrated=0;
    bool resolve(int dynasty,const Program& p,Foundation& out){
        if(!dynasty||!valid(p)||!p.identityKnown)return false;
        for(auto& e:entries_)if(e.dynasty==dynasty&&e.row==p.row&&e.id==p.id&&e.custom==p.custom){
            // A save older than our first observation cannot inherit a future
            // custom-team starting estimate. Rebase to that earlier origin.
            if(e.base.peers&&p.year<e.born){e.base=foundation(p);e.born=p.year;++revision;}
            out=e.base;return out.fans>0;
        }
        if(entries_.size()>=8192)return false;
        out=foundation(p);if(out.fans<=0)return false;
        entries_.push_back({dynasty,p.row,p.id,p.year,p.custom,out});++revision;return true;
    }
    void write(std::ostream& out)const{
        out<<"DGA-AUDIENCE-2 "<<entries_.size()<<'\n'<<std::setprecision(17);
        for(const auto& e:entries_){out<<e.dynasty<<' '<<e.row<<' '<<e.id<<' '<<e.born<<' '<<e.custom<<' '<<e.base.peers<<' '<<e.base.fans;for(float f:e.base.reference)out<<' '<<f;out<<'\n';}
    }
    bool read(std::istream& in){
        Book next;std::string magic;size_t count=0;if(!(in>>magic>>count)||(magic!="DGA-AUDIENCE-1"&&magic!="DGA-AUDIENCE-2")||count>8192)return false;
        for(size_t n=0;n<count;++n){Entry e;int custom=0,peers=0;
            if(!(in>>e.dynasty>>e.row>>e.id>>e.born>>custom>>peers>>e.base.fans)||custom<0||custom>1||peers<0||peers>1)return false;
            e.custom=custom!=0;e.base.peers=peers!=0;
            if(!e.dynasty||e.row<0||e.row>1000000||e.id<0||e.id>=2047||e.born<0||e.born>100||!std::isfinite(e.base.fans)||e.base.fans<=0||e.base.fans>1000000)return false;
            for(auto& f:e.base.reference)if(!(in>>f)||!std::isfinite(f)||f<0||f>1)return false;
            for(const auto& old:next.entries_)if(old.dynasty==e.dynasty&&old.row==e.row&&old.id==e.id&&old.custom==e.custom)return false;
            next.entries_.push_back(e);
        }
        in>>std::ws;if(!in.eof())return false;
        if(magic=="DGA-AUDIENCE-1"){
            for(auto& e:next.entries_)if(!e.custom&&!e.base.peers)for(const auto& c:CalibrationSchools)if(c.id==e.id){
                double updated=e.base.fans*static_cast<double>(c.referenceFans)/c.fans;
                if(!std::isfinite(updated)||updated<=0||updated>1000000)return false;
                e.base.fans=updated;++next.migrated;break;
            }
            next.upgraded=true;++next.revision; // Format 2 records that the correction was applied once.
        }
        *this=std::move(next);return true;
    }
};
}
