#include "../Source/VisitorPlans.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <numeric>
#include <set>
#include <sstream>

using namespace attendance;
namespace {
constexpr unsigned Columns=20, Rows=17, BlockSize=Columns*Rows, Across=6, Tiers=5;
constexpr unsigned StandSize=Across*Tiers*BlockSize, N=3*StandSize, Seed=739, AnchorBlock=14;
struct Fixture {
    std::vector<seats::Position> positions;
    std::vector<unsigned char> occupied;
    agents::Tags tags;
    unsigned spectators=0;
    Fixture():positions(N),occupied(N,1){
        std::vector<unsigned char> records(N*10);
        for(unsigned i=0;i<N;++i){
            unsigned stand=i/StandSize,block=(i%StandSize)/BlockSize,row=i%BlockSize/Columns,col=i%Columns;
            float outward=80.f+(block/Across)*14.f+row*.7f;
            float along=-46.f+(block%Across)*15.5f+col*.65f;
            float height=2.f+(block/Across)*10.f+row*.55f+(stand==2?62.f:0.f);
            positions[i]={stand==1?-outward:outward,height,along,0};
            records[10*i]=stand==0&&block==AnchorBlock;
            // Leave the 340-seat native anchor intact. Elsewhere include true
            // empties, band members, fixed away/home fans and regular students.
            if(stand==0&&block==AnchorBlock)continue;
            if(i%211==0){records[10*i+6]=10;records[10*i]=i%2;}
            else if(i%149==0)records[10*i+6]=2;
            else if(i%83==0)records[10*i+6]=9;
            if(i%101==0)occupied[i]=0;
        }
        tags=agents::decode(records,N);agents::protectStudents(tags,true,false);
        for(unsigned i=0;i<N;++i)spectators+=occupied[i]&&!tags.protectedBand(i);
    }
};
struct Measures {
    unsigned visitors=0,core=0,protectedAway=0,inner=0,innerVisitors=0,edge=0,edgeVisitors=0;
    std::set<unsigned> blocks;
};
Measures measure(const Fixture& f,const std::vector<unsigned char>& sides,const visitors::seating::Stats& stats){
    Measures m;
    for(unsigned i=0;i<N;++i){
        if(!f.occupied[i]||f.tags.protectedGroup(i))assert(sides[i]==f.tags.side[i]&&!stats.coreMask[i]&&stats.party[i]<0);
        if(!f.occupied[i]||f.tags.protectedBand(i))continue;
        m.visitors+=sides[i];if(f.tags.protectedFan(i))m.protectedAway+=sides[i];
        if(stats.coreMask[i]){assert(i<StandSize);m.blocks.insert(i/BlockSize);m.core+=sides[i];}
        unsigned row=i%BlockSize/Columns,col=i%Columns;bool edge=row==0||col==0||row+1==Rows||col+1==Columns;
        std::array<unsigned,4> adjacent{{col?i-1:i,col+1<Columns?i+1:i,row?i-Columns:i,row+1<Rows?i+Columns:i}};
        for(unsigned j:adjacent){
            if(!stats.coreMask[j])edge=true;
            if(stats.party[i]>=0&&sides[j]&&!f.tags.protectedGroup(j)&&f.occupied[j])assert(stats.party[j]==stats.party[i]);
        }
        if(stats.coreMask[i]){
            if(edge){++m.edge;m.edgeVisitors+=sides[i];}else{++m.inner;m.innerVisitors+=sides[i];}
        }
    }
    std::vector<unsigned> sizes(stats.parties),owners(stats.parties,UINT32_MAX);
    for(unsigned i=0;i<N;++i)if(stats.party[i]>=0){
        unsigned id=static_cast<unsigned>(stats.party[i]);assert(id<sizes.size()&&sides[i]);
        if(!sizes[id])owners[id]=i/BlockSize;
        assert(owners[id]==i/BlockSize);++sizes[id];
    }
    std::array<unsigned,9> counts{};for(auto size:sizes){assert(size>=1&&size<=8);++counts[size];}
    assert(counts==stats.sizes&&m.visitors==m.core+stats.scattered+m.protectedAway);
    // Reconstruct the four physical nearest-neighbor slots from fixture row
    // indices, independently of the allocator's spatial hash and block map.
    // Vacancies can select the next seat diagonally in the neighboring row.
    std::vector<std::vector<unsigned>> withinParty(N);
    for(unsigned i=0;i<N;++i)if(f.occupied[i]&&!f.tags.protectedGroup(i)){
        int row=i%BlockSize/Columns,col=i%Columns;
        std::array<unsigned,4> closest{{N,N,N,N}};std::array<double,4> minimum{{2,2,2,2}};
        for(int dr=-1;dr<=1;++dr)for(int dc=-1;dc<=1;++dc){
            if((dr==0&&dc==0)||row+dr<0||row+dr>=int(Rows)||col+dc<0||col+dc>=int(Columns))continue;
            unsigned j=unsigned(int(i)+dr*int(Columns)+dc);
            if(!f.occupied[j]||f.tags.protectedGroup(j))continue;
            const auto& p=f.positions[i];const auto& q=f.positions[j];
            double x=q.x-p.x,y=q.y-p.y,z=q.z-p.z,d=x*x+y*y+z*z;
            unsigned slot=dr==0?(dc<0?0:1):(dr<0?2:3);
            if(d<minimum[slot]||(d==minimum[slot]&&j<closest[slot])){closest[slot]=j;minimum[slot]=d;}
        }
        for(unsigned j:closest)if(j<N&&sides[i]&&sides[j]&&(stats.party[i]>=0||stats.party[j]>=0)){
            assert(stats.party[i]>=0&&stats.party[i]==stats.party[j]);
            withinParty[i].push_back(j);withinParty[j].push_back(i);
        }
    }
    std::vector<unsigned char> checked(N);
    for(unsigned i=0;i<N;++i)if(stats.party[i]>=0&&!checked[i]){
        std::vector<unsigned> connected{i};checked[i]=1;
        for(size_t k=0;k<connected.size();++k)for(unsigned j:withinParty[connected[k]])if(!checked[j]){checked[j]=1;connected.push_back(j);}
        assert(connected.size()==sizes[stats.party[i]]);
    }
    return m;
}
void phases(const Fixture& f,const std::vector<unsigned char>& sides,const visitors::seating::Stats& stats,bool cluster,bool print){
    auto tags=f.tags;tags.side=sides;
    std::vector<unsigned short> masks(N);for(unsigned i=0;i<N;++i)masks[i]=f.occupied[i]?0:7;
    auto pattern=seats::build(f.positions,masks,7,Seed);
    auto groups=visitors::groups(pattern,masks,7,tags);assert(groups.verified);
    fanbudget::Counts counts{groups.home,groups.away,groups.homeProtected,groups.awayProtected};
    visitors::Losses gone;
    for(auto state:std::vector<std::pair<int,int>>{{0,50},{0,10},{0,0},{0,18},{25,25}}){
        gone.advance(counts.home,counts.away,state.first,21,10,4,counts.homeProtected,counts.awayProtected,false,true);
        auto plan=fanbudget::build({{11,counts}},{gone.home,gone.away},state.second,true);assert(plan.valid);
        unsigned present=0,away=0,core=0;
        for(unsigned i=0;i<N;++i)if(f.occupied[i]&&!tags.protectedBand(i)){
            bool hidden=groups.rank[i]<(sides[i]?plan.missing.away:plan.missing.home);
            if(tags.protectedFan(i))assert(!hidden);
            if(!hidden){++present;away+=sides[i];core+=sides[i]&&stats.coreMask[i];}
        }
        assert(present==counts.total()-plan.missing.total()&&away==counts.away-plan.missing.away);
        if(cluster)assert(core>away/2);
        if(print)std::printf("phase|exit=%d|absent=%d|present=%u|away=%u|main=%u\n",state.first,state.second,present,away,core);
    }
}
}
int main(){
    Fixture f;float share=4751.f/f.spectators;
    std::vector<unsigned char> first;
    for(bool cluster:{false,true}){
#ifdef ORIGINAL_SEATING
        if(cluster)continue;
#endif
        std::vector<double> times;
        for(int run=0;run<8;++run){
            visitors::seating::Stats stats;bool fallback=false;
            auto begin=std::chrono::steady_clock::now();
            auto sides=visitors::seating::assign(f.positions,f.tags,share,Seed,&f.occupied,false,&stats,true,&fallback,true
#ifndef ORIGINAL_SEATING
            ,cluster
#endif
            );
            double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();
            if(run)times.push_back(ms);
            assert(!fallback&&sides.size()==N&&stats.unassigned==0);
            if(run==0){
                first=sides;auto m=measure(f,sides,stats);assert(m.visitors==4751&&stats.core==m.core);
                assert(stats.sizes[2]+stats.sizes[3]+stats.sizes[4]>stats.parties/2);
                if(cluster){assert(m.blocks.size()>1&&m.core>m.visitors*.60&&m.core<m.visitors*.80);
                    assert(m.edge&&m.inner&&double(m.innerVisitors)/m.inner>.9&&double(m.edgeVisitors)/m.edge<.90);}
                else assert(m.blocks.size()==1&&m.core<=340);
                std::printf("distribution|v%u|positions=%u|spectators=%u|budget=%u|main=%u|scattered=%u|fixedAway=%u|blocks=%zu|footprint=%u|inner=%u/%u|edge=%u/%u|parties=%u|sizes=",cluster?5:4,N,f.spectators,m.visitors,m.core,stats.scattered,m.protectedAway,m.blocks.size(),stats.footprint,m.innerVisitors,m.inner,m.edgeVisitors,m.edge,stats.parties);
                for(unsigned k=1;k<=8;++k)std::printf("%s%u",k==1?"":",",stats.sizes[k]);std::puts("");
                phases(f,sides,stats,cluster,true);
            }else assert(sides==first);
        }
        std::sort(times.begin(),times.end());std::printf("preparation|v%u|median_ms=%.3f|min=%.3f|max=%.3f|n=7\n",cluster?5:4,times[3],times.front(),times.back());
    }
#ifndef ORIGINAL_SEATING
    using namespace visitors;
    FixtureKey k{51,1101,1102,3,4,100,9,2026};
    Plan previous;previous.valid=true;previous.layoutVersion=4;previous.share=share;
    Plan fresh=previous;fresh.layoutVersion=CurrentLayoutVersion;fresh.share=share+.01f;
    Plan selected;unsigned seed=0;bool reused=false;PlanBook book;
    assert(book.resolve(k,previous,Seed,selected,seed,reused)&&!reused&&book.bind(k,701));
    assert(book.resolve(k,fresh,200,selected,seed,reused)&&reused&&selected.layoutVersion==4&&seed==Seed&&book.source(k)==701);
    auto old=assign(f.positions,f.tags,selected.share,seed,&f.occupied,selected.layoutVersion);
    assert(old!=first);
    auto other=k;++other.week;assert(book.resolve(other,fresh,100,selected,seed,reused)&&!reused&&book.bind(other,702));
    assert(book.rebuild(k)&&book.source(other)==702);fresh.share=share;
    assert(book.resolve(k,fresh,Seed,selected,seed,reused)&&!reused&&book.bind(k,801));
    std::stringstream disk;book.write(disk);PlanBook load;assert(PlanBook::read(disk,load));
    assert(load.resolve(k,previous,999,selected,seed,reused)&&reused&&selected.layoutVersion==5&&seed==Seed&&load.source(k)==801);
    assert(first==assign(f.positions,f.tags,selected.share,seed,&f.occupied,selected.layoutVersion));
    auto bad=f.tags;bad.type.pop_back();assert(assign(f.positions,bad,share,Seed,&f.occupied,5).empty());
    bad=f.tags;bad.fixed.pop_back();assert(assign(f.positions,bad,share,Seed,&f.occupied,5).empty());
    // A verified but isolated anchor may not select a remote block as core.
    auto isolated=f.positions;for(unsigned i=AnchorBlock*BlockSize;i<(AnchorBlock+1)*BlockSize;++i)isolated[i].y+=95;
    seating::Stats island;bool fallback=false;
    auto native=seating::assign(isolated,f.tags,share,Seed,&f.occupied,false,&island,true,&fallback,true,true);
    assert(native.size()==N&&!fallback&&island.blocks==1&&island.footprint<=BlockSize);
    for(unsigned i=0;i<N;++i)if(island.coreMask[i])assert(i/BlockSize==AnchorBlock);
    // Infeasible requested share preserves native selectors and reports fallback.
    auto fixed=f.tags;for(unsigned i=0;i<N;++i)if(i%10==0){fixed.fixed[i]=1;fixed.side[i]=1;}
    assert(assign(f.positions,fixed,.03f,Seed,&f.occupied,5,false,&fallback)==fixed.side&&fallback);
    for(unsigned i=0;i<N;++i)f.tags.side[i]=(i%StandSize)/BlockSize%Across<3;
    assert(assign(f.positions,f.tags,.45f,Seed,&f.occupied,4,true)==assign(f.positions,f.tags,.45f,Seed,&f.occupied,5,true));
#endif
}
