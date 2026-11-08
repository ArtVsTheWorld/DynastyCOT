#include "../Source/StartingAttendance.h"
#include <cassert>
#include <cstdio>
using namespace attendance::turnout;
int main(){
    ModeWitness witness;
    assert(!witness.observe(-1,true,1000)); // No remembered Dynasty mode.
    assert(witness.observe(14,true,1100)==1100);auto generation=witness.generation;
    assert(witness.observe(-1,true,1200)==1100); // -1 cannot renew the window.
    assert(witness.observe(-1,true,11100)==1100);assert(!witness.observe(-1,true,11101));
    assert(witness.generation!=generation&&!witness.armed);
    witness.observe(14,true,12000);generation=witness.generation;
    assert(!witness.observe(17,true,12001));witness.observe(14,true,12002);assert(witness.generation!=generation);
    assert(!witness.observe(14,false,12003)); // Unreadable native state.
    witness.observe(14,true,13000);assert(!witness.observe(-1,true,12999)); // Clock reversal.
    witness.observe(14,true,14000);
    Seed s;s.home.id=1161;s.away.id=1206;s.stadium=561;s.ea=66349;s.regular=true;
    s.dynastyVerifiedAt=witness.verifiedAt;s.dynastyGeneration=witness.generation;
    Seeds cache;cache.remember(s,14001);Launch l;l.home=1161;l.away=1206;l.stadium=561;l.ea=66349;
    assert(cache.find(l,14002,s)&&transitionProof(s,l,14002,witness.generation));
    assert(!transitionProof(s,l,24001,witness.generation));assert(!transitionProof(s,l,14000,witness.generation));
    for(int field=0;field<4;++field){auto wrong=l;if(field==0)++wrong.home;if(field==1)++wrong.away;if(field==2)++wrong.stadium;if(field==3)++wrong.ea;assert(!transitionProof(s,wrong,14002,witness.generation));}
    witness.observe(17,true,14003);witness.observe(14,true,14004);assert(!transitionProof(s,l,14005,witness.generation));
    auto restored=s;restored.resumed=true;assert(!transitionProof(restored,l,14005,s.dynastyGeneration)&&capturedLaunchProof(restored,l,14005,s.dynastyGeneration));
    restored=s;restored.regular=false;assert(!transitionProof(restored,l,14005,s.dynastyGeneration)&&capturedLaunchProof(restored,l,14005,s.dynastyGeneration));
    assert(!capturedLaunchProof(restored,l,24001,s.dynastyGeneration));
    std::puts("PASS: verified mode14-to-minus1 bridge, ten-second expiry without renewal, mode changes/unreadable/reversed-clock revocation, fresh matching game identity, generation isolation and resume/postseason rejection.");
}
