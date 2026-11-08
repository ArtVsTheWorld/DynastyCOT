#include "SeatPatternFixture.h"
#include <cassert>
#include <cstdio>
using namespace attendance::seats;
uint64_t fingerprint(const Pattern& pattern,unsigned count,int loss){
    uint64_t value=1469598103934665603ull;for(unsigned i=0;i<count;++i){value^=pattern.departed(i,loss)?1u:0u;value*=1099511628211ull;}return value;
}
int main(){
    assert(variationBlend(.10)==0&&variationBlend(0)==0&&variationBlend(.50)==1);
    assert(desirability({32,10,0,0})>desirability({32,1,0,0}));
    assert(desirability({32,10,0,0})>desirability({32,10,35,0}));
    assert(desirability({32,10,35,0})>desirability({0,1,65,0}));
    assert(desirability({55,25,0,0})>desirability({0,1,65,0}));
    assert(desirability({32,10,20,0})==desirability({-32,10,-20,0}));
    unsigned gapGroups=0;
    for(unsigned seed=0;seed<128;++seed){
        double a=smallGroupGap({32,10,0,0},seed),b=smallGroupGap({32,10,.2f,0},seed);
        assert(a==b&&a>=0&&a<=.16);gapGroups+=a>0;
    }
    assert(gapGroups>0&&gapGroups<64);
    // Before-change fingerprints captured from 1.95 (064f4e3), using this
    // fixture and native-hole mask. Proves the 90/95% picture stays identical.
    const uint64_t expected[3][3]={
        {14458285009550783363ull,16131538434719675729ull,3631105268084038631ull},
        {14458285009550783363ull,2984148884985834377ull,17168852404632250323ull},
        {14458285009550783363ull,2918203680408403737ull,6498918714603707207ull}};
    auto small=fixture::bowl(16,64,true);unsigned sample=0;
    for(unsigned seed:{0u,31u,1960u}){
        auto p=build(small.positions,small.baseline,small.parts,seed);assert(p.policy==Policy::Position);
        unsigned column=0;for(int loss:{0,5,10})assert(fingerprint(p,static_cast<unsigned>(small.positions.size()),loss)==expected[sample][column++]);
        auto repeat=build(small.positions,small.baseline,small.parts,seed);assert(p.rank==repeat.rank);
        for(int loss=0;loss<=100;++loss){unsigned absent=0;
            for(unsigned i=0;i<small.positions.size();++i){bool hidden=p.departed(i,loss);absent+=hidden;
                if(small.baseline[i]==small.parts)assert(!hidden);
                if(loss)assert(!p.departed(i,loss-1)||hidden);
            }
            assert(absent==static_cast<uint64_t>(p.original)*loss/100);
        }
        ++sample;
    }
    auto ring=fixture::bowl();auto ids=std::vector<unsigned>(ring.positions.size());std::iota(ids.begin(),ids.end(),0u);
    std::sort(ids.begin(),ids.end(),[&](unsigned a,unsigned b){double x=distance(ring.positions[a]),y=distance(ring.positions[b]);return x==y?a<b:x<y;});
    std::vector<double> depth(ids.size());for(size_t k=0;k<ids.size();++k)depth[ids[k]]=(k+.5)/ids.size();
    double frontAverage[2]{},backAverage[2]{},variationAverage[2]{};
    for(unsigned seed:{0u,31u,1950u,1960u,1961u,1962u,1963u,1964u}){
        auto p=build(ring.positions,ring.baseline,ring.parts,seed);
        for(unsigned scenario=0;scenario<2;++scenario){int loss=scenario?70:50;
            unsigned front=0,frontN=0,back=0,backN=0;std::array<unsigned,24> group{},groupN{};
            for(unsigned i=0;i<ids.size();++i){bool present=!p.departed(i,loss);
                if(depth[i]<.35){front+=present;++frontN;unsigned section=(i%288)/12;group[section]+=present;++groupN[section];}
                if(depth[i]>=.75){back+=present;++backN;}
            }
            double mean=0,square=0;for(unsigned k=0;k<24;++k){double v=static_cast<double>(group[k])/groupN[k];mean+=v/24;square+=v*v/24;}
            frontAverage[scenario]+=static_cast<double>(front)/frontN/8;
            backAverage[scenario]+=static_cast<double>(back)/backN/8;
            variationAverage[scenario]+=std::sqrt((std::max)(0.0,square-mean*mean))/8;
        }
    }
    // Broad behavioral bounds avoid pinning the implementation's exact ranks:
    // meaningful lower gaps, variable neighboring areas, general front retention.
    assert(frontAverage[0]>.62&&frontAverage[0]<.80&&frontAverage[0]>backAverage[0]+.25);
    assert(frontAverage[1]>.36&&frontAverage[1]<.60&&frontAverage[1]>backAverage[1]+.20);
    assert(variationAverage[0]>.10&&variationAverage[1]>.10);
    std::printf("PASS: exact legacy 90/95/100-percent seat fingerprints; immutable native holes, exact counts and nested selections; midfield/view-height preferences, short bounded group gaps and eight-seed front retention. Synthetic front occupancy at 50/30: %.1f/%.1f%%.\n",100*frontAverage[0],100*frontAverage[1]);
}
