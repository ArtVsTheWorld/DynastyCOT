#include "../Source/NativeReactionTrace.h"
#include <cassert>
#include <cstdio>
using namespace attendance::nativevoices;
int main(){
    Trace trace;Snapshot s;assert(!trace.observe(1,2,3,1,1000,s));
    s.count=2;s.voices[0]={91,77};s.voices[1]={83,78};assert(trace.observe(1,2,3,1,1050,s));
    for(uint64_t now=1100;now<10000;now+=50){std::swap(s.voices[0],s.voices[1]);assert(!trace.observe(1,2,3,1,now,s));}
    s.voices[0]={59,79};assert(trace.observe(1,2,3,1,10000,s));
    s.count=0;assert(trace.observe(1,2,3,1,10100,s));assert(!trace.observe(1,2,3,1,10200,s));
    s.count=1;s.voices[0]={91,77};assert(trace.observe(1,2,3,1,11000,s));
    assert(trace.observe(1,2,3,2,11100,s));assert(trace.observe(1,2,4,2,11200,s));assert(trace.observe(1,5,4,2,11300,s));
    assert(trace.observe(1,5,4,2,12051,s));assert(trace.observe(1,5,4,2,1000,s)); // Gap/rewind is a fresh observation, never a fabricated stop.
    s.count=17;assert(!trace.observe(1,5,4,2,1100,s));s.count=0;assert(!trace.observe(0,5,4,2,1100,s));
    Trace bounded;s.count=1;for(uintptr_t n=1;n<=32;++n)assert(bounded.observe(n,2,3,1,1000,s));
    assert(!bounded.observe(33,2,3,1,1100,s));assert(bounded.observe(33,2,3,1,31001,s));
    std::puts("PASS: bounded read-only engine voice snapshots, stable ordering, no frame/age spam, changes, gaps, owner/epoch changes and capacity limits.");
}
