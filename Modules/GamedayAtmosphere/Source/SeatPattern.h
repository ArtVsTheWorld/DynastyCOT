#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numeric>
#include <unordered_map>
#include <vector>
#include "SeatHash.h"
#include "CrowdVisibility.h"
namespace attendance::seats {
struct Position {float x=0,y=0,z=0,packed=0;};
static_assert(sizeof(Position)==16);
enum class Policy {Pending,Position,Scattered};
struct Pattern {
    Policy policy=Policy::Pending;unsigned original=0;std::vector<unsigned> rank;
    bool departed(unsigned agent,int loss)const{
        loss=(std::clamp)(loss,0,100);
        if(policy!=Policy::Position)return attendance::departed(agent,loss);
        return agent<rank.size()&&rank[agent]<static_cast<uint64_t>(original)*static_cast<unsigned>(loss)/100;
    }
};
inline double noise(double x,double z,double scale=28,int seed=0){
    double gx=x/scale,gz=z/scale;int ix=static_cast<int>(std::floor(gx)),iz=static_cast<int>(std::floor(gz));
    double tx=gx-ix,tz=gz-iz;tx=tx*tx*(3-2*tx);tz=tz*tz*(3-2*tz);
    auto value=[&](int a,int b){return seed?random({a,b,seed}):random({a,b});};
    double a=value(ix,iz),b=value(ix+1,iz),c=value(ix,iz+1),d=value(ix+1,iz+1);
    return (a+(b-a)*tx)*(1-tz)+(c+(d-c)*tx)*tz;
}
// A preparation-local cache of grid-corner hashes. Nearby seats interpolate
// the same corners; reuse those exact values without changing the seeded field.
class NoiseCache {
    double scale_;int seed_;std::unordered_map<uint64_t,double> corners_;
    double value(int a,int b){
        uint64_t key=(uint64_t(static_cast<uint32_t>(a))<<32)|static_cast<uint32_t>(b);
        auto found=corners_.find(key);if(found!=corners_.end())return found->second;
        double v=seed_?random({a,b,seed_}):random({a,b});corners_.emplace(key,v);return v;
    }
public:
    NoiseCache(double scale,int seed):scale_(scale),seed_(seed){}
    double sample(double x,double z){
        double gx=x/scale_,gz=z/scale_;int ix=static_cast<int>(std::floor(gx)),iz=static_cast<int>(std::floor(gz));
        double tx=gx-ix,tz=gz-iz;tx=tx*tx*(3-2*tx);tz=tz*tz*(3-2*tz);
        double a=value(ix,iz),b=value(ix+1,iz),c=value(ix,iz+1),d=value(ix+1,iz+1);
        return (a+(b-a)*tx)*(1-tz)+(c+(d-c)*tx)*tz;
    }
};
inline double interpolate(double p,const std::array<double,4>& values){
    if(p<=.125)return values[0];if(p>=.875)return values[3];
    int k=(std::min)(2,static_cast<int>((p-.125)/.25));double t=(p-(.125+k*.25))/.25;t=t*t*(3-2*t);
    return values[k]+(values[k+1]-values[k])*t;
}
inline double smooth(double x){x=(std::clamp)(x,0.0,1.0);return x*x*(3-2*x);}
inline double variationBlend(double missing){return smooth((missing-.10)/.40);}
// Bounded viewing-location preference, not native ticket/section metadata.
// Keep the old distance/height curve and favor midfield sidelines within it.
inline double desirability(const Position& p){
    double midfield=1-smooth((std::abs(static_cast<double>(p.z))-9)/38);
    double sideline=smooth((std::abs(static_cast<double>(p.x))-18)/12);
    double usefulHeight=smooth((p.y-.75)/4)*(1-smooth((p.y-16)/20));
    double endzone=smooth((std::abs(static_cast<double>(p.z))-38)/24);
    return .26*midfield*sideline*(.8+.2*usefulHeight)-.12*endzone;
}
inline double smallGroupGap(const Position& p,unsigned seed){
    bool side=std::abs(p.x)/24.384>=std::abs(p.z)/54.864;
    double tangent=side?p.z:p.x;
    int sector=side?(p.x>=0?0:1):(p.z>=0?2:3);
    double draw=random({static_cast<int>(std::floor(tangent/3)),static_cast<int>(std::floor(p.y/.9)),sector,static_cast<int>(seed%100000),197});
    // Occasional short, roughly row-aligned groups, rather than broad blobs.
    // Desirable midfield seating gets a smaller gap allowance, not immunity.
    double central=(1-smooth((std::abs(static_cast<double>(p.z))-9)/38))*smooth((std::abs(static_cast<double>(p.x))-18)/12);
    return .16*(1-.6*central)*(std::max)(0.0,(draw-.78)/.22);
}
inline int32_t rounded(double x){
    double f=std::floor(x),t=x-f;return static_cast<int32_t>(f+(t>.5||(t==.5&&static_cast<int64_t>(f)%2!=0)?1:0));
}
inline double distance(const Position& p){
    double x=(std::max)(std::abs(static_cast<double>(p.x))-24.384,0.0),z=(std::max)(std::abs(static_cast<double>(p.z))-54.864,0.0);
    return std::sqrt(x*x+z*z+static_cast<double>(p.y)*p.y);
}
// The builder validates and totals weights while producing them. Reusing that
// exact sum avoids a second full pass at every one-percent departure level.
inline bool normalizeWithTotal(const std::vector<double>& weights,size_t target,double total,std::vector<double>& probabilities){
    if(weights.empty()||!target||target>=weights.size())return false;
    if(!std::isfinite(total)||total<=0)return false;
    double scale=static_cast<double>(target)/total;size_t previous=0;
    for(unsigned iteration=0;iteration<128;++iteration){
        size_t capped=0;double uncapped=0;
        for(double w:weights){if(scale*w>1)++capped;else uncapped+=w;}
        if(capped==previous){probabilities.resize(weights.size());for(size_t i=0;i<weights.size();++i)probabilities[i]=(std::min)(1.0,scale*weights[i]);return true;}
        if(capped<previous||capped>=target||uncapped<=0)return false;
        scale=static_cast<double>(target-capped)/uncapped;previous=capped;
    }
    return false;
}
inline bool normalize(const std::vector<double>& weights,size_t target,std::vector<double>& probabilities){
    double total=0;for(double w:weights){if(!std::isfinite(w)||w<=0)return false;total+=w;}
    return normalizeWithTotal(weights,target,total,probabilities);
}
// Reject unfamiliar geometry rather than infer a field transform from it.
// Multiple renderers may cover only part of the bowl, so symmetry is not assumed.
inline bool plausible(const std::vector<Position>& positions,const std::vector<unsigned>& ids){
    if(positions.size()<512||positions.size()>200000||ids.size()<256)return false;
    unsigned outside=0;double lowest=1e9,highest=-1e9;
    for(unsigned i:ids){if(i>=positions.size())return false;const auto& p=positions[i];
        if(!std::isfinite(p.x)||!std::isfinite(p.y)||!std::isfinite(p.z)||std::abs(p.x)>300||std::abs(p.z)>350||p.y<-.5f||p.y>150)return false;
        double d=distance(p);lowest=(std::min)(lowest,d);highest=(std::max)(highest,d);
        if(std::abs(p.x)>24.384||std::abs(p.z)>54.864)++outside;
    }
    return outside>=ids.size()*9/10&&highest-lowest>5&&highest<250;
}
inline Pattern build(const std::vector<Position>& positions,const std::vector<unsigned short>& baseline,unsigned short parts,unsigned seed=0){
    Pattern result;result.policy=Policy::Scattered;
    if(!parts||positions.size()!=baseline.size())return result;
    std::vector<unsigned> ids;for(unsigned i=0;i<baseline.size();++i)if((baseline[i]&parts)!=parts)ids.push_back(i);
    if(!plausible(positions,ids))return result;
    const size_t n=ids.size();auto byDepth=ids;
    std::vector<double> distances(positions.size());for(unsigned i:ids)distances[i]=distance(positions[i]);
    std::sort(byDepth.begin(),byDepth.end(),[&](unsigned a,unsigned b){double da=distances[a],db=distances[b];return da==db?a<b:da<db;});
    std::vector<double> percentile(positions.size()),logWeight(positions.size()),area(positions.size()),choice(positions.size()),quality(positions.size());
    for(size_t k=0;k<n;++k)percentile[byDepth[k]]=(static_cast<double>(k)+.5)/static_cast<double>(n);
    const int sectionSeed=static_cast<int>(seed%100000);
    NoiseCache coarse(28,sectionSeed+11),middle(9,sectionSeed+27),broad(55,sectionSeed+59);
    for(unsigned i:ids){
        const auto& p=positions[i];double x=p.x,y=p.y,z=p.z,depth=percentile[i];
        // One continuous curve avoids density shoulders at seating-zone edges.
        logWeight[i]=std::log(1.45-.9*depth*depth*(3-2*depth));
        int fx=static_cast<int>(std::floor(x/1.8)),fy=static_cast<int>(std::floor(y)),fz=static_cast<int>(std::floor(z/1.8));
        int32_t rx=rounded(x*100),ry=rounded(y*100),rz=rounded(z*100);
        choice[i]=random({rx,ry,rz,117})<.32?random({rx,ry,rz,71}):random({fx,fy,fz});
        // Uneven arrival/exit order across the entire bowl, with soft-edged
        // seating areas, families and scattered stragglers still overlapping.
        area[i]=(.45*(2*coarse.sample(x,z)-1)+.35*(2*middle.sample(x+97,z-131)-1)+
            .20*(2*broad.sample(x-33,z+51)-1))*(.25+.75*std::pow(depth,1.2));
        quality[i]=desirability(p)-smallGroupGap(p,seed)*(1-smooth((depth-.4)/.3));
    }
    std::vector<unsigned> remaining=ids,order;order.reserve(n);
    std::vector<double> horizontal(n),weights(n),probabilities,probability(positions.size()),score(positions.size());
    // These geometry-only values do not change over the 100 loss levels.
    std::vector<unsigned char> zones(n),slots(n);std::vector<double> blends(n);std::array<size_t,4> counts{};
    for(size_t k=0;k<n;++k){double p=percentile[ids[k]];zones[k]=static_cast<unsigned char>((std::min)(3u,static_cast<unsigned>(p*4)));++counts[zones[k]];
        if(p<=.125){slots[k]=4;continue;}if(p>=.875){slots[k]=5;continue;}
        int slot=(std::min)(2,static_cast<int>((p-.125)/.25));double t=(p-(.125+slot*.25))/.25;
        slots[k]=static_cast<unsigned char>(slot);blends[k]=t*t*(3-2*t);
    }
    for(size_t count:counts)if(!count)return result;
    for(unsigned loss=1;loss<=100;++loss){
        size_t target=n-static_cast<uint64_t>(n)*loss/100;
        if(!target){
            std::sort(remaining.begin(),remaining.end(),[&](unsigned a,unsigned b){return score[a]==score[b]?a<b:score[a]<score[b];});
            order.insert(order.end(),remaining.begin(),remaining.end());break;
        }
        double missing=static_cast<double>(loss)/100,blend=variationBlend(missing);std::array<double,4> sums{};
        for(size_t k=0;k<n;++k){unsigned i=ids[k];horizontal[k]=std::exp(4.4*missing*area[i]);sums[zones[k]]+=horizontal[k];}
        for(unsigned k=0;k<4;++k)sums[k]/=static_cast<double>(counts[k]);
        double total=0;
        for(size_t k=0;k<n;++k){unsigned i=ids[k],slot=slots[k];double divisor=slot==4?sums[0]:slot==5?sums[3]:sums[slot]+(sums[slot+1]-sums[slot])*blends[k];
            weights[k]=std::exp(2.3*missing*(logWeight[i]+blend*quality[i]))*horizontal[k]/divisor;
            if(!std::isfinite(weights[k])||weights[k]<=0)return result;total+=weights[k];
        }
        if(!normalizeWithTotal(weights,target,total,probabilities))return result;
        for(size_t k=0;k<n;++k)probability[ids[k]]=probabilities[k];
        for(unsigned i:remaining)score[i]=choice[i]/(std::max)(1e-15,probability[i]);
        auto compare=[&](unsigned a,unsigned b){return score[a]==score[b]?a<b:score[a]<score[b];};
        auto cut=remaining.begin()+static_cast<ptrdiff_t>(target);
        if(cut!=remaining.end())std::nth_element(remaining.begin(),cut,remaining.end(),compare);
        std::sort(cut,remaining.end(),compare);
        for(size_t k=remaining.size();k>target;--k)order.push_back(remaining[k-1]);
        remaining.resize(target);
    }
    if(order.size()!=n)return result;
    result.rank.assign(positions.size(),(std::numeric_limits<unsigned>::max)());
    for(unsigned k=0;k<order.size();++k)result.rank[order[k]]=k;
    result.original=static_cast<unsigned>(n);result.policy=Policy::Position;return result;
}
}
