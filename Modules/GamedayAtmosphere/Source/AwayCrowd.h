#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include "SeatHash.h"
#include "SeatPattern.h"
#include "CrowdAgents.h"
#include "StartingAttendance.h"
#include "ProtectedFanBudget.h"

namespace attendance::visitors {
struct Location {double latitude=NAN,longitude=NAN;};
inline bool valid(Location a){return std::isfinite(a.latitude)&&std::isfinite(a.longitude)&&std::abs(a.latitude)<=90&&std::abs(a.longitude)<=180&&(std::abs(a.latitude)+std::abs(a.longitude)>1);}
inline double miles(Location a,Location b){
    if(!valid(a)||!valid(b))return NAN;
    constexpr double rad=3.14159265358979323846/180.;
    double x=std::sin((b.latitude-a.latitude)*rad*.5),y=std::sin((b.longitude-a.longitude)*rad*.5);
    double h=x*x+std::cos(a.latitude*rad)*std::cos(b.latitude*rad)*y*y;
    return 7917.6*std::asin(std::sqrt((std::clamp)(h,0.,1.)));
}
struct Demand {
    double homeFans=0,awayFans=0;
    Location home,away;
    Location homeState,awayState;
    bool sameState=false,rivalry=false,neutral=false;
    double homeStrength=0; // Verified public rank / established record, 0..1.
    bool marquee=false; // Both verified AP top ten, never guessed from brand.
    unsigned seed=0;
};
enum class Travel {Unknown,City,State,Local};
struct Plan {bool valid=false,native=false,travelKnown=false;float share=.05f;double distance=NAN;Travel travel=Travel::Unknown;double travelFactor=1;};
inline Plan influence(Plan p,double percent){
    // Preference for new plans, separate from the underlying travel model.
    // Default is bit-for-bit unchanged; neutral seating remains native.
    if(!p.valid||p.native||!std::isfinite(percent)||percent==100)return p;
    if(!std::isfinite(p.share)||p.share<.01f||p.share>.70f)return p;
    const double strength=(std::clamp)(percent,0.,200.)/100.;
    const double odds=double(p.share)/(1.-p.share)*strength;
    p.share=static_cast<float>((std::clamp)(odds/(1.+odds),.01,.70));return p;
}
inline Plan plan(const Demand& d){
    Plan p;if(d.neutral){p.valid=p.native=true;p.share=.5f;return p;}
    if(!std::isfinite(d.homeFans)||!std::isfinite(d.awayFans)||d.homeFans<=0||d.awayFans<=0)return p;
    p.valid=true;p.distance=miles(d.home,d.away);p.travelKnown=std::isfinite(p.distance);
    if(p.travelKnown){p.travel=Travel::City;p.travelFactor=(.30+1.90*std::exp(-p.distance/600.))*(d.sameState?1.15:1.);}
    else if(d.sameState){p.travel=Travel::Local;p.travelFactor=1.15;} // No fabricated zero-mile trip.
    else if(valid(d.homeState)&&valid(d.awayState)){
        p.travel=Travel::State;p.distance=miles(d.homeState,d.awayState);p.travelKnown=true;
        p.travelFactor=1.+.5*((.30+1.90*std::exp(-p.distance/600.))-1.);
    }
    // Calibrated fan counts, never current stadium capacity or a school-ID list.
    double ratio=(std::clamp)(d.awayFans/d.homeFans,.01,100.);
    // Odds keep ordinary support modest without a discontinuous takeover
    // threshold. Nearby equal-sized fanbases travel better; a national-sized
    // audience can still travel in numbers over a long distance.
    // Absolute audience matters as well as the ratio. Large followings retain
    // a bounded part of their ordinary demand on long trips. Approximate state
    // distance still receives only half the adjustment; unknown stays neutral.
    double national=(std::clamp)((d.awayFans-40000.)/60000.,0.,1.);
    if(p.travel==Travel::City||p.travel==Travel::State){
        double resilience=(.30+.65*national)+(1.90-.65*national)*std::exp(-p.distance/600.);
        p.travelFactor=p.travel==Travel::City?resilience*(d.sameState?1.15:1.):1.+.5*(resilience-1.);
    }
    double travel=p.travelFactor;
    double variety=.9+.2*seats::random({static_cast<int>(d.seed%100000),407});
    double odds=.07*std::pow(ratio,.85)*travel*variety;
    if(d.rivalry)odds*=1.30+.10*seats::random({static_cast<int>(d.seed%100000),409});
    if(d.marquee)odds*=1.15; // Capped 15% demand boost, not percentage points.
    // A strong host keeps more of its own audience. Compress extra proximity
    // and rivalry demand smoothly, even when both fanbases are large. Unknown
    // home quality leaves the ordinary calibrated travel model intact.
    const double resistance=std::isfinite(d.homeStrength)?(std::clamp)(d.homeStrength,0.,1.):0;
    const double ceiling=.70-.54*resistance;
    const double ordinary=odds/(1+odds);
    const double core=(std::min)(ordinary,.055+.015*national);
    const double extra=ordinary-core;
    const double compressed=core+extra/(1+extra/ceiling*resistance*2.);
    odds=compressed/(1-compressed);
    p.share=static_cast<float>((std::clamp)(odds/(1+odds),.01,.70));return p;
}
inline Plan plan(const turnout::CrowdFixture& fixture,int home,int away){
    if(!fixture.owns(home,away))return {};
    const auto& s=fixture.seed;Demand d;
    d.homeFans=s.home.audienceFans;d.awayFans=s.away.audienceFans;
    d.home={s.home.latitude,s.home.longitude};d.away={s.away.latitude,s.away.longitude};
    d.homeState={s.home.stateLatitude,s.home.stateLongitude};d.awayState={s.away.stateLatitude,s.away.stateLongitude};
    d.sameState=turnout::sameState(s);d.rivalry=fixture.launch.rivalry;
    const auto& host=s.home;
    d.marquee=host.apRank>=1&&host.apRank<=10&&s.away.apRank>=1&&s.away.apRank<=10;
    if(host.apRank>=1&&host.apRank<=25)d.homeStrength=.65+.35*(25-host.apRank)/24.;
    const int games=host.wins+host.losses;
    if(host.wins>=0&&host.losses>=0&&games>=4){
        const double form=(std::clamp)((double(host.wins)/games-.5)*2.,0.,1.);
        d.homeStrength=(std::max)(d.homeStrength,form*(std::min)(1.,games/8.));
    }
    d.neutral=fixture.launch.neutral;d.seed=turnout::fixtureHash(s);
    auto out=plan(d);
    if(turnout::fcs(away)&&!d.neutral){out.valid=true;out.share=.01f;}
    return out;
}
// Strong group preference around a deterministic corner/end-zone visiting area,
// with smaller scattered parties. Distance/height balance is handled by the
// per-side departure ranks, so visitors do not all disappear with the top rows.
inline std::vector<unsigned char> assign(const std::vector<seats::Position>& positions,const agents::Tags& tags,float share,unsigned seed,const std::vector<unsigned char>* occupied=nullptr){
    std::vector<unsigned char> side=tags.side;
    if(!tags.verified||positions.size()!=side.size()||!std::isfinite(share)||share<.01f||share>.70f||
       (occupied&&occupied->size()!=side.size()))return {};
    std::vector<unsigned> regular;regular.reserve(side.size());unsigned fixedAway=0,totalFans=0;
    for(unsigned i=0;i<side.size();++i)if(!tags.protectedBand(i)&&(!occupied||(*occupied)[i])){
        ++totalFans;
        if(agents::regular(tags.type[i])&&!tags.protectedFan(i)){regular.push_back(i);side[i]=0;}
        else fixedAway+=side[i];
    }
    unsigned corner=fanHash(seed+401)%4;double sx=corner&1?1.:-1.,sz=corner&2?1.:-1.;
    auto score=[&](unsigned i){const auto& p=positions[i];
        double proximity=std::exp(-std::pow((p.x-sx*36)/38,2)-std::pow((p.z-sz*48)/40,2));
        double group=seats::noise(p.x,p.z,7,static_cast<int>(seed%100000)+413);
        return .7*proximity+.18*group+.12*seats::random({static_cast<int>(i),static_cast<int>(seed%100000),419});
    };
    // Precompute scores; sorting must never repeat geometry/noise work.
    std::vector<double> scores(side.size());for(unsigned i:regular)scores[i]=score(i);
    auto order=[&](unsigned a,unsigned b){return scores[a]==scores[b]?a<b:scores[a]>scores[b];};
    size_t desiredTotal=static_cast<size_t>(std::llround(totalFans*share));
    size_t target=desiredTotal>fixedAway?desiredTotal-fixedAway:0;
    target=(std::min)(target,regular.size());
    // Keep a recognizable visiting section, but let more supporters sit among
    // home fans. The rest travel as small parties rather than isolated dots.
    // The main area's footprint is 25% smaller than the previous 70% block.
    // Leave deterministic home pockets inside it, with the exact remaining
    // visitor budget assigned to small parties outside the core.
    size_t block=target*21/40;
    std::partial_sort(regular.begin(),regular.begin()+block,regular.end(),order);
    size_t coreVisitors=0;
    for(size_t i=0;i<block;++i)if(fanHash(regular[i]^seed^439)%10){side[regular[i]]=1;++coreVisitors;}
    std::sort(regular.begin()+block,regular.end(),[&](unsigned a,unsigned b){
        unsigned x=fanHash(a^seed^421),y=fanHash(b^seed^421);return x==y?a<b:x<y;});
    struct Cell {
        int x,y,z;
        bool operator==(const Cell& b)const{return x==b.x&&y==b.y&&z==b.z;}
    };
    struct CellHash {
        size_t operator()(const Cell& c)const{
            return fanHash(static_cast<unsigned>(c.x))^
                (static_cast<size_t>(fanHash(static_cast<unsigned>(c.y)))<<1)^
                (static_cast<size_t>(fanHash(static_cast<unsigned>(c.z)))<<2);
        }
    };
    auto cell=[](const seats::Position& p){return Cell{static_cast<int>(std::floor(p.x/3)),static_cast<int>(std::floor(p.y)),static_cast<int>(std::floor(p.z/3))};};
    std::unordered_map<Cell,std::vector<unsigned>,CellHash> nearby;
    nearby.reserve(regular.size()-block);
    for(size_t i=block;i<regular.size();++i){const auto& p=positions[regular[i]];
        // Unknown geometry keeps the exact fan budget and deterministic
        // single-seat fallback, without unsafe float-to-integer conversions.
        if(std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z)&&
           std::abs(p.x)<=300&&std::abs(p.z)<=350&&p.y>=-.5f&&p.y<=150)
            nearby[cell(p)].push_back(regular[i]);
    }
    size_t selected=coreVisitors;
    for(size_t k=block;k<regular.size()&&selected<target;++k){
        unsigned leader=regular[k];if(side[leader])continue;
        side[leader]=1;++selected;
        const auto& p=positions[leader];
        if(!std::isfinite(p.x)||!std::isfinite(p.y)||!std::isfinite(p.z)||
           std::abs(p.x)>300||std::abs(p.z)>350||p.y<-.5f||p.y>150)continue;
        size_t buddies=(std::min)(target-selected,static_cast<size_t>(1+fanHash(leader^seed^431)%3));
        if(!buddies)continue;
        // Prefer adjacent seats on the same row. A 3-D grid prevents pairing
        // seats on different decks and avoids an all-pairs stadium scan.
        std::array<unsigned,3> best{};std::array<double,3> distance{};unsigned count=0;
        auto origin=cell(p);
        for(int x=-1;x<=1;++x)for(int y=-1;y<=1;++y)for(int z=-1;z<=1;++z){
            auto found=nearby.find({origin.x+x,origin.y+y,origin.z+z});if(found==nearby.end())continue;
            const auto& bucket=found->second;
            size_t first=fanHash(leader^seed^433)%bucket.size();
            // Also bound malformed/duplicate-coordinate buckets; normal seat
            // cells contain far fewer than this many people.
            for(size_t n=0;n<(std::min)(bucket.size(),size_t(128));++n){
                unsigned candidate=bucket[(first+n)%bucket.size()];
                if(side[candidate])continue;const auto& q=positions[candidate];
                double dx=q.x-p.x,dy=q.y-p.y,dz=q.z-p.z,d=dx*dx+dz*dz;
                if(std::abs(dy)>.45||d>2.75*2.75)continue;
                d+=dy*dy*4;unsigned at=0;
                while(at<count&&(distance[at]<d||(distance[at]==d&&best[at]<candidate)))++at;
                if(at>=buddies)continue;
                unsigned end=(std::min)(count,static_cast<unsigned>(buddies)-1);
                for(unsigned shift=end;shift>at;--shift){best[shift]=best[shift-1];distance[shift]=distance[shift-1];}
                best[at]=candidate;distance[at]=d;count=(std::min)(count+1,static_cast<unsigned>(buddies));
            }
        }
        for(unsigned n=0;n<count;++n){side[best[n]]=1;++selected;}
    }
    return side;
}
struct Population {
    bool verified=false;uint64_t home=0,away=0,originalHome=0,originalAway=0;
    bool sharedBudget=false;
    int appliedLoss=0,appliedTemporary=0;
    uintptr_t match=0;unsigned epoch=0;
    float homeLoss=-1,awayLoss=-1;
    bool owns(uintptr_t m,unsigned e)const{return verified&&match==m&&epoch==e&&home+away>0;}
    float share()const{return verified&&home+away?static_cast<float>(static_cast<double>(away)/(home+away)):0;}
};
struct Losses {
    uint64_t home=0,away=0;
    void restore(uint64_t originalHome,uint64_t originalAway,uint64_t protectedHome,uint64_t protectedAway,float homeFraction,float awayFraction){
        home=away=0;
        if(!std::isfinite(homeFraction)||!std::isfinite(awayFraction)||homeFraction<0||homeFraction>1||awayFraction<0||awayFraction>1||protectedHome>originalHome||protectedAway>originalAway)return;
        home=(std::min)(originalHome-protectedHome,static_cast<uint64_t>(std::llround(originalHome*homeFraction)));
        away=(std::min)(originalAway-protectedAway,static_cast<uint64_t>(std::llround(originalAway*awayFraction)));
    }
    void advance(uint64_t homeOriginal,uint64_t awayOriginal,int percent,int homeScore,int awayScore,int quarter,uint64_t homeProtected=0,uint64_t awayProtected=0,bool neutral=false){
        uint64_t total=homeOriginal+awayOriginal;
        if(!total||home>homeOriginal||away>awayOriginal)return;
        if(homeProtected>homeOriginal||awayProtected>awayOriginal)return;
        uint64_t removable=total-homeProtected-awayProtected;
        uint64_t target=(std::min)(removable,total*static_cast<unsigned>((std::clamp)(percent,0,100))/100);
        if(target<=home+away)return; // A lead change never brings departed fans back.
        if(home+homeProtected>homeOriginal||away+awayProtected>awayOriginal)return;
        uint64_t increment=target-home-away,h=homeOriginal-homeProtected-home,a=awayOriginal-awayProtected-away;
        double leverage=.4+.6*(std::clamp)(quarter,1,4)/4.;
        double pressure=(std::clamp)((awayScore-homeScore)/14.*leverage,-2.5,2.5);
        double hw=std::exp(pressure),aw=std::exp(-pressure),low=0,high=1;
        // A winning visiting section still has traffic departures. Previously
        // exponential score weighting could make its exit rate 148x slower.
        // Ease the imbalance progressively; keep the shared attendance budget,
        // protected groups, neutral crowds and recorded history unchanged.
        if(!neutral&&awayScore>homeScore){
            const double late=(std::clamp)((quarter-1)/3.,0.,1.);
            hw=(std::min)(hw,3.);aw=(std::max)(aw,.20+.35*late);
        }
        auto take=[&](double scale){return h*(std::min)(1.,scale*hw)+a*(std::min)(1.,scale*aw);};
        while(take(high)<increment)high*=2;
        for(unsigned i=0;i<48;++i){double mid=(low+high)*.5;if(take(mid)<increment)low=mid;else high=mid;}
        uint64_t fromHome=(std::min)(h,static_cast<uint64_t>(std::llround(h*(std::min)(1.,high*hw))));
        fromHome=(std::clamp)(fromHome,increment>a?increment-a:0,(std::min)(h,increment));
        home+=fromHome;away+=increment-fromHome;
    }
};
struct Groups {
    bool verified=false;uint64_t home=0,away=0,homeProtected=0,awayProtected=0;
    std::vector<unsigned> rank;std::vector<unsigned char> side;
    bool selected(unsigned agent,float homeLoss,float awayLoss,int arrivals)const{
        if(!verified||agent>=rank.size()||rank[agent]==UINT32_MAX)return false;
        if(!std::isfinite(homeLoss)||!std::isfinite(awayLoss))return false;
        fanbudget::Counts c{home,away,homeProtected,awayProtected};if(!c.valid())return false;
        auto gone=fanbudget::permanent(c,static_cast<uint64_t>(std::llround(home*(std::clamp)(homeLoss,0.f,1.f))),
            static_cast<uint64_t>(std::llround(away*(std::clamp)(awayLoss,0.f,1.f))));
        auto hidden=fanbudget::missing(c,gone,arrivals);
        return rank[agent]<(side[agent]?hidden.away:hidden.home);
    }
};
inline Groups groups(const seats::Pattern& pattern,const std::vector<unsigned short>& baseline,unsigned short parts,const agents::Tags& tags){
    Groups out;if(!tags.verified||tags.type.size()!=baseline.size()||!parts)return out;
    out.rank.assign(baseline.size(),UINT32_MAX);out.side=tags.side;
    std::vector<unsigned> order;
    for(unsigned i=0;i<baseline.size();++i)if(!tags.protectedBand(i)&&(baseline[i]&parts)!=parts)order.push_back(i);
    auto rank=[&](unsigned i){return pattern.policy==seats::Policy::Position&&i<pattern.rank.size()?pattern.rank[i]:fanHash(i);};
    std::sort(order.begin(),order.end(),[&](unsigned a,unsigned b){auto x=rank(a),y=rank(b);return x==y?a<b:x<y;});
    unsigned homeRank=0,awayRank=0;
    for(unsigned i:order){if(out.side[i])++out.away;else ++out.home;
        if(tags.protectedFan(i)){if(out.side[i])++out.awayProtected;else ++out.homeProtected;}
        else out.rank[i]=out.side[i]?awayRank++:homeRank++;
    }
    out.verified=true;return out;
}
// Both sides use the same occupied-seat curve. At an equal supporter count,
// home and visiting reactions receive equal strength, irrespective of labels.
inline float reactionScale(uint64_t supporters,uint64_t total){
    if(!total||!supporters)return 0;
    return (std::clamp)(std::pow(static_cast<float>(static_cast<double>(supporters)/total),.55f),0.f,1.f);
}
}
