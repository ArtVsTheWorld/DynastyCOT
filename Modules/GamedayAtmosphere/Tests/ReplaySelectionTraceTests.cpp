#include "../Source/ReplaySelectionTrace.h"
#include <cassert>
#include <cstdio>
using namespace attendance::replaytrace;
int main(){
 Buffer b;Record r;r.node=10;r.phase=3;r.family=1;unsigned count=0;
 auto write=[&](uint64_t seq,const Record& row){assert(seq==7&&row.node==10);++count;};
 b.add(123,1,7,r);b.flush(write);assert(count==0); // Retain the decision before score confirmation.
 b.confirm(123,1,7);b.flush(write);assert(count==1);
 for(int i=0;i<10000;++i){r.at=static_cast<uint64_t>(i);b.add(123,1,7,r);}b.flush(write);assert(count==1);
 r.armed=1;b.add(123,1,7,r);b.flush(write);assert(count==2);b.flush(write);assert(count==2);
 for(int i=0;i<1000;++i){r.native=i;b.add(123,1,7,r);}b.flush(write);assert(count==128);
 b.add(123,1,8,r);b.flush(write);assert(count==128); // New play must earn its own confirmation.
 b.confirm(123,2,8);b.flush(write);assert(count==128); // A new epoch has no old records.
 b.add(124,2,8,r);b.flush(write);assert(count==128);
 std::puts("PASS: pre-score replay decisions retained, confirmed-play-only flush, semantic dedupe, bounded 128-row budget, once-only drain and play/match/epoch isolation.");
}
