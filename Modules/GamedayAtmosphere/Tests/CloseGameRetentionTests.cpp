#include "../Source/AttendanceTimeline.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    float earliest=1,latest=0;
    for(int length:{60,300,720,1200})for(unsigned seed=1;seed<=256;++seed){
        State s{2,14,14,5,float(length),float(length)};s.variationSeed=seed;s.rank0=60;s.rank1=70;
        auto t=context::halftimeTiming(s);assert(t.departure>=.2f&&t.departure<=.3f&&t.mainReturn>=.15f&&t.mainReturn<=.25f&&t.lastReturn>=.25f&&t.lastReturn<=.35f&&t.lastReturn>=t.mainReturn);
        assert(t.departure==context::halftimeTiming(s).departure);earliest=(std::min)(earliest,t.lastReturn);latest=(std::max)(latest,t.lastReturn);
        arrivals::Halftime half;int previous=0;
        for(int step=0;step<=100;++step){s.seconds=length*(1-step*.01f);int total=half.observe(s,5);assert(total>=previous);previous=total;}
        int peak=previous;assert(peak>5&&peak<=35);s.quarter=3;
        for(int step=0;step<=40;++step){s.seconds=length*(1-step*.01f);int total=half.observe(s,5);if(total)assert(total<=previous);previous=total;}
        assert(half.done&&!previous);assert(half.observe(s,5)==0);
        // Reclassification changes ownership of empty seats, never adds an
        // extra visible departure or repopulates a permanent absence.
        half.reset();s.quarter=2;s.seconds=0;half.observe(s,5);s.quarter=3;s.seconds=length*.9f;
        int total=half.observe(s,5);int converted=(std::min)(3,total-5);assert(converted>=0);
        half.convert(s,5,5+converted,total,100);assert(std::abs(half.observe(s,5+converted)-total)<=1);
    }
    assert(latest-earliest>.09f);
    State s{2,0,0,5,0,720};
    for(int margin=0;margin<=16;++margin){s.awayScore=margin;float target=context::halftime(s).permanent;
        assert(target>=3&&target<=10);if(margin<=8)assert(target<=5);if(margin==16)assert(target==10);}
    // The steady game clock / scoreboard cannot keep drawing new timings.
    s={3,14,14,5,720,720};s.variationSeed=42;arrivals::Halftime late;assert(!late.observe(s,5));
    s.quarter=2;s.seconds=0;late.reset();int peak=late.observe(s,5);s.quarter=3;s.seconds=660;int next=late.observe(s,5);
    s.seconds=680;assert(late.observe(s,5)==next&&next<=peak);
    // No seed is a supported old-history fallback, with the old timetable.
    s.variationSeed=0;auto timing=context::halftimeTiming(s);assert(timing.departure==.25f&&timing.lastReturn==.25f);
    std::puts("PASS: close-game halftime targets, stable bounded per-game timing, two return waves, monotonic absences, conversion ownership, rewinds and legacy fallback across quarter lengths/seeds.");
}
