#include "../Source/StartingAttendance.h"
#include <cassert>
#include <cstdio>
using namespace attendance::turnout;
int main(){
    for(int type=-2;type<=255;++type)for(bool practice:{false,true})for(bool bowl:{false,true})assert(bowlFixtureProof(type,practice,bowl)==(!practice&&bowl&&type>=1&&type<=5));
    ModeWitness witness;
    assert(!witness.observe(-1,true,1000)); // No remembered Dynasty mode.
    assert(witness.observe(14,true,1100)==1100);auto generation=witness.generation;
    assert(witness.observe(-1,true,1200)==1100); // -1 cannot renew the window.
    assert(witness.observe(-1,true,121100)==1100);assert(!witness.observe(-1,true,121101));
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
    assert(!transitionProof(s,l,134001,witness.generation));assert(!transitionProof(s,l,14000,witness.generation));
    for(int field=0;field<4;++field){auto wrong=l;if(field==0)++wrong.home;if(field==1)++wrong.away;if(field==2)++wrong.stadium;if(field==3)++wrong.ea;assert(!transitionProof(s,wrong,14002,witness.generation));}
    // Regression: real WMU/EMU launch observed mode14 at 26335250, then
    // mode -1 for 23,265 ms before the formula, and 24,093 ms before launch.
    ModeWitness slow;assert(slow.observe(14,true,26335250));
    assert(slow.observe(-1,true,26358515)==26335250);
    auto delayed=s;delayed.dynastyVerifiedAt=slow.verifiedAt;delayed.dynastyGeneration=slow.generation;
    Seeds slowCache;slowCache.remember(delayed,26358515);
    assert(slow.observe(-1,true,26359343));
    assert(slowCache.find(l,26359343,delayed));
    assert(transitionProof(delayed,l,26359343,slow.generation));
    // Loading does not extend the witness or let a different fixture use it.
    assert(slow.observe(-1,true,26455250));assert(!slow.observe(-1,true,26455251));
    assert(!transitionProof(delayed,l,26455251,slow.generation));
    witness.observe(17,true,14003);witness.observe(14,true,14004);assert(!transitionProof(s,l,14005,witness.generation));
    auto restored=s;restored.resumed=true;assert(!transitionProof(restored,l,14005,s.dynastyGeneration)&&capturedLaunchProof(restored,l,14005,s.dynastyGeneration));
    restored=s;restored.regular=false;assert(!transitionProof(restored,l,14005,s.dynastyGeneration)&&capturedLaunchProof(restored,l,14005,s.dynastyGeneration));
    auto springSeed=restored;springSeed.spring=true;springSeed.springSide=1;springSeed.springSchool=s.away.id;springSeed.springNumberKnown=true;springSeed.springNumber=4;springSeed.dynasty=-1972245012;
    assert(transitionProof(springSeed,l,14005,s.dynastyGeneration));
    springSeed.springNumber=250;assert(!transitionProof(springSeed,l,14005,s.dynastyGeneration));springSeed.springNumber=4;
    {auto stamped=springSeed;auto stampedLaunch=l;stamped.home.id=stampedLaunch.home=1502;stamped.springNumber=250;stamped.springType=1;stamped.springStamp=1200000;
     assert(transitionProof(stamped,stampedLaunch,14005,s.dynastyGeneration));stamped.springStamp=1200001;assert(!transitionProof(stamped,stampedLaunch,14005,s.dynastyGeneration));}
    springSeed.dynasty=0;assert(!transitionProof(springSeed,l,14005,s.dynastyGeneration));springSeed.dynasty=-1972245012;
    springSeed.resumed=true;assert(!transitionProof(springSeed,l,14005,s.dynastyGeneration));
    assert(!capturedLaunchProof(restored,l,134001,s.dynastyGeneration));
    std::puts("PASS: verified mode14-to-minus1 bridge, two-minute expiry without renewal, mode changes/unreadable/reversed-clock revocation, fresh matching game identity, generation isolation and resume/postseason rejection.");
}
