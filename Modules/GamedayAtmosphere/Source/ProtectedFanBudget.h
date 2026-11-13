#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace attendance::fanbudget {
inline constexpr uint64_t MaxFans=12800000;
struct Counts {
    uint64_t home=0,away=0,homeProtected=0,awayProtected=0;
    uint64_t total()const{return home+away;}
    bool valid()const{return home<=MaxFans&&away<=MaxFans&&total()<=MaxFans&&homeProtected<=home&&awayProtected<=away;}
};
struct Hidden {uint64_t home=0,away=0;uint64_t total()const{return home+away;}};
inline Hidden permanent(const Counts& c,uint64_t home,uint64_t away){
    return {(std::min)(home,c.home-c.homeProtected),(std::min)(away,c.away-c.awayProtected)};
}
// A percentage is a TOTAL absence budget, including permanent departures.
// Protected spectators occupy that budget; bands never enter these counts.
inline Hidden missing(const Counts& c,Hidden gone,int percent,bool familyFloor=false){
    if(!c.valid()||!c.total())return {};
    gone=permanent(c,gone.home,gone.away);
    uint64_t h=c.home-c.homeProtected,a=c.away-c.awayProtected;
    uint64_t target=(std::max)(gone.total(),(std::min)(h+a,c.total()*static_cast<unsigned>((std::clamp)(percent,0,100))/100));
    if(familyFloor&&c.away){uint64_t keep=(std::min)(c.away-gone.away,(c.total()-target+99)/100);a=(std::min)(a,c.away-keep);target=(std::min)(target,h+a);}
    uint64_t low=(std::max)(gone.home,target>a?target-a:0),high=(std::min)(h,target-gone.away);
    uint64_t home=(std::clamp)(target*c.home/c.total(),low,high);
    return {home,target-home};
}
struct Layout {uint64_t key=0;Counts fans;};
struct Selection {uint64_t key=0;Hidden gone,missing;};
struct Plan {bool valid=false;Counts fans;Hidden gone,missing;std::vector<Selection> layouts;};
inline bool totals(const std::vector<Layout>& layouts,Counts& result){
    if(layouts.empty()||layouts.size()>64)return false;
    Counts c;
    for(size_t i=0;i<layouts.size();++i){
        const auto& x=layouts[i];if(!x.key||!x.fans.valid())return false;
        for(size_t j=0;j<i;++j)if(layouts[j].key==x.key)return false;
        c.home+=x.fans.home;c.away+=x.fans.away;c.homeProtected+=x.fans.homeProtected;c.awayProtected+=x.fans.awayProtected;
        if(!c.valid())return false;
    }
    if(!c.total())return false;result=c;return true;
}
namespace detail {
// Highest-averages allocation is deterministic and house-monotone: increasing
// permanent losses cannot refill a different layout through rounding. Floors
// preserve already departed fans while allocating temporary absences around them.
inline bool divide(const std::vector<Layout>& layouts,bool away,uint64_t target,const std::vector<uint64_t>& floors,std::vector<uint64_t>& out){
    const size_t n=layouts.size();if(floors.size()!=n)return false;
    std::vector<uint64_t> weights(n),caps(n);uint64_t minimum=0,maximum=0;double high=1;
    for(size_t i=0;i<n;++i){const auto& c=layouts[i].fans;
        weights[i]=away?c.away:c.home;caps[i]=weights[i]-(away?c.awayProtected:c.homeProtected);
        if(floors[i]>caps[i])return false;minimum+=floors[i];maximum+=caps[i];high=(std::max)(high,static_cast<double>(weights[i])+1);
    }
    if(target<minimum||target>maximum)return false;
    auto count=[&](size_t i,double threshold){
        double raw=weights[i]/threshold;
        uint64_t value=raw>=static_cast<double>(caps[i])?caps[i]:static_cast<uint64_t>(raw);
        return (std::max)(floors[i],value);
    };
    double low=0;
    for(unsigned step=0;step<64;++step){double mid=(low+high)*.5;uint64_t sum=0;for(size_t i=0;i<n;++i)sum+=count(i,mid);if(sum>target)low=mid;else high=mid;}
    out.resize(n);uint64_t sum=0;for(size_t i=0;i<n;++i){out[i]=count(i,high);sum+=out[i];}
    if(sum>target||target-sum>n*2)return false;
    while(sum<target){size_t best=n;
        for(size_t i=0;i<n;++i)if(out[i]<caps[i]){
            if(best==n||weights[i]*(out[best]+1)>weights[best]*(out[i]+1)||
               (weights[i]*(out[best]+1)==weights[best]*(out[i]+1)&&layouts[i].key<layouts[best].key))best=i;
        }
        if(best==n)return false;++out[best];++sum;
    }
    return true;
}
}
inline Plan build(const std::vector<Layout>& layouts,Hidden gone,int percent,bool familyFloor=false){
    Plan p;if(!totals(layouts,p.fans))return p;
    p.gone=permanent(p.fans,gone.home,gone.away);p.missing=missing(p.fans,p.gone,percent,familyFloor);
    std::vector<uint64_t> zero(layouts.size()),home,away,homeMissing,awayMissing;
    if(!detail::divide(layouts,false,p.gone.home,zero,home)||!detail::divide(layouts,true,p.gone.away,zero,away)||
       !detail::divide(layouts,false,p.missing.home,home,homeMissing)||!detail::divide(layouts,true,p.missing.away,away,awayMissing))return p;
    for(size_t i=0;i<layouts.size();++i)p.layouts.push_back({layouts[i].key,{home[i],away[i]},{homeMissing[i],awayMissing[i]}});
    p.valid=true;return p;
}
}
