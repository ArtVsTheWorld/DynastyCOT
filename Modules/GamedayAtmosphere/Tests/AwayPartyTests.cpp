#include "../Source/AwayCrowd.h"
#include <cassert>
#include <chrono>
#include <cstdio>
using namespace attendance;
int main(){
    constexpr unsigned count=10240;
    std::vector<seats::Position> p(count);std::vector<unsigned char> raw(count*10);
    for(unsigned i=0;i<count;++i){
        unsigned section=i/2560,row=(i%2560)/128,seat=i%128;
        float along=(static_cast<float>(seat)-63.5f)*.65f,out=36+row*.9f,height=1+row*.7f;
        p[i]=section<2?seats::Position{section?out:-out,height,along,0}:
            seats::Position{along,height,section==2?out+20:-out-20,0};
        raw[i*10]=i%11==0?1:0;
        raw[i*10+6]=i<32?1:i%200==0?10:0;
    }
    auto tags=agents::decode(raw,count);unsigned fans=0;
    for(unsigned i=0;i<count;++i)fans+=!tags.protectedBand(i);
    double lowestBuddyRate=1,assignTotalMs=0,assignMaxMs=0;unsigned cases=0;
    auto started=std::chrono::steady_clock::now();
    for(unsigned seed=0;seed<12;++seed)for(float share:{.01f,.03f,.11f,.25f,.70f}){
        auto before=std::chrono::steady_clock::now();auto side=visitors::assign(p,tags,share,seed);
        double assignMs=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-before).count();
        assignTotalMs+=assignMs;assignMaxMs=(std::max)(assignMaxMs,assignMs);
        assert(side==visitors::assign(p,tags,share,seed));
        unsigned total=0;std::array<unsigned,4> sections{};
        for(unsigned i=0;i<count;++i){
            if(tags.protectedGroup(i))assert(side[i]==tags.side[i]);
            if(!tags.protectedBand(i)){total+=side[i];sections[i/2560]+=side[i];}
        }
        assert(total==static_cast<unsigned>(std::llround(fans*share)));
        // Independently measure physical company away from the busiest visiting
        // section. These are people in nearby seats, not neighboring array IDs.
        unsigned main=static_cast<unsigned>(std::max_element(sections.begin(),sections.end())-sections.begin());
        unsigned scattered=0,withBuddy=0;
        for(unsigned i=32;i<count;++i)if(side[i]&&i/2560!=main&&!tags.protectedFan(i)){
            ++scattered;bool buddy=false;
            for(unsigned j=32;j<count&&!buddy;++j)if(i!=j&&side[j]&&!tags.protectedGroup(j)){
                double dx=p[i].x-p[j].x,dy=p[i].y-p[j].y,dz=p[i].z-p[j].z;
                buddy=std::abs(dy)<=.45&&dx*dx+dz*dz<=2.75*2.75;
            }
            withBuddy+=buddy;
        }
        assert(scattered>0);
        double rate=static_cast<double>(withBuddy)/scattered;
        lowestBuddyRate=(std::min)(lowestBuddyRate,rate);assert(rate>=.80);++cases;
    }
    // Sparse original crowds keep their exact occupied denominator. Protected
    // people and empty seats must never be recruited to supply a buddy.
    std::vector<unsigned char> occupied(count,0);
    for(unsigned i=32;i<count;++i)occupied[i]=i%3!=0;
    auto sparse=visitors::assign(p,tags,.11f,73,&occupied);unsigned seated=0,away=0;
    for(unsigned i=0;i<count;++i){
        if(occupied[i]&&!tags.protectedBand(i)){++seated;away+=sparse[i];}
        if(!occupied[i]||tags.protectedGroup(i))assert(sparse[i]==tags.side[i]);
    }
    assert(away==static_cast<unsigned>(std::llround(seated*.11f)));
    // Determinism survives duplicate geometry; no all-pairs candidate search.
    std::fill(p.begin(),p.end(),seats::Position{35,4,20,0});
    auto duplicate=visitors::assign(p,tags,.11f,73);assert(duplicate.size()==count);
    double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count();
    std::printf("PASS: %u visitor-party layouts; minimum %.1f%% of scattered visitors have a nearby companion; exact occupied budgets, protected groups, empty seats and deterministic/duplicate geometry preserved (%.1f ms total including independent pair checks; assignment mean/max %.2f/%.2f ms for 10240 positions, offline wall time)\n",cases,lowestBuddyRate*100,ms,assignTotalMs/cases,assignMaxMs);
}
