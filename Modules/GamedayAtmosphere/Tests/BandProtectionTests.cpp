#include "../Source/CrowdAgents.h"
#include "../Source/SeatPattern.h"
#include "../Source/DirectorShake.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    std::vector<unsigned char> raw(1024*10);
    for(unsigned i=0;i<1024;++i)raw[i*10+6]=static_cast<unsigned char>(i%12|0xe0);
    auto tags=agents::decode(raw,1024);assert(tags.verified&&tags.band>0);
    for(unsigned i=0;i<1024;++i){assert(tags.protectedBand(i)==(i%12>=1&&i%12<=8));assert(tags.type[i]==i%12);}
    assert(!tags.protectedBand(1024));
    // Native military seats stay protected without school metadata. Student
    // seats need verified military-school ownership; standing alone proves no
    // requirement to stay. Check both home and visiting academy supporters.
    for(unsigned i=0;i<1024;++i)raw[i*10]=static_cast<unsigned char>((i/12)%2);
    auto ordinary=agents::decode(raw,1024);
    for(unsigned i=0;i<1024;++i)assert(ordinary.protectedFan(i)==(i%12==10));
    auto home=ordinary,away=ordinary,both=ordinary;
    agents::protectStudents(home,true,false);agents::protectStudents(away,false,true);agents::protectStudents(both,true,true);
    for(unsigned i=0;i<1024;++i){
        assert(home.protectedFan(i)==(i%12==10||(i%12==9&&!home.side[i])));
        assert(away.protectedFan(i)==(i%12==10||(i%12==9&&away.side[i])));
        assert(both.protectedFan(i)==(i%12==10||i%12==9));
        if(i%12==11)assert(!both.protectedGroup(i));
    }
    raw[6]=31;assert(!agents::decode(raw,1024).verified);
    raw.resize(100);assert(!agents::decode(raw,1024).verified);
    assert(!agents::decode(std::vector<unsigned char>(100*10),100).verified);
    // Protection remains independent from the geometry fallback's hash order.
    seats::Pattern fallback;fallback.policy=seats::Policy::Scattered;
    for(int loss:{0,25,50,75,100})for(unsigned i=0;i<1024;++i){
        bool removed=!tags.protectedBand(i)&&fallback.departed(i,loss);
        if(tags.protectedBand(i))assert(!removed);
    }
    // Opening kick only: admitted defensive opening strength is unchanged.
    for(int i=0;i<=100;++i){float t=i*.01f;
        float previous=10*(.6f+.2f*t),current=motion::openingKickoffStrength(10,t);
        assert(std::abs(current-previous*.75f)<1e-5f);
        assert(current>=4.49f&&current<=6.01f);
    }
    std::puts("PASS: all 12 native crowd labels, musician/military protection, home/away academy-student ownership, ordinary-student and standing-only exclusions, unknown-layout fallback and gentler kickoff-only envelope");
}
