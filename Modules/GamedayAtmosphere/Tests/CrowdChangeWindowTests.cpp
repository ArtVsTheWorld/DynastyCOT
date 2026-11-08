#include "../Source/CrowdChangeWindow.h"
#include "../Source/ArrivalRules.h"
#include "../Source/DepartureModel.h"
#include "../Source/SuperSimState.h"
#include <cassert>
#include <cstdio>
#include <cstring>
using namespace attendance;
int main(){
    CrowdChangeWindow window;arrivals::Model arrivals;arrivals::Settings arriving;
    arriving.startPercent=60;arriving.finishPercent=50; // Supported upper ordinary setting; exercise a long arrival window.
    State s{1,0,0,6,900,900};
    auto entry=[&](){window.observe(123,s);return window.take(123,s);};
    assert(entry());assert(arrivals.observe(arriving,s).missing==40);
    for(int i=0;i<20;++i)assert(!entry()); // Same intro never opens a second window.
    s.phase=1;
    for(int seconds=900;seconds>=700;seconds-=5){s.seconds=static_cast<float>(seconds);assert(!entry());assert(arrivals.missing==40);}
    s.phase=3;s.seconds=680;assert(!entry());s.phase=4;s.seconds=670;assert(!entry());
    s.phase=5;s.seconds=650;assert(entry());assert(arrivals.observe(arriving,s).missing==11);
    for(int seconds=645;seconds>=550;seconds-=5){s.seconds=static_cast<float>(seconds);assert(!entry());assert(arrivals.missing==11);}
    s.phase=1;s.seconds=540;assert(!entry());s.phase=3;s.seconds=530;assert(!entry());
    s.phase=5;s.seconds=500;window.observe(123,s); // A throttled decision can consume the pending entry later in selection.
    assert(window.take(123,s));assert(!window.take(123,s));assert(arrivals.observe(arriving,s).missing==2);
    s.phase=3;s.seconds=480;window.observe(123,s);s.phase=5;s.seconds=470;window.observe(123,s);
    s.phase=1;s.seconds=460;window.observe(123,s);assert(!window.take(123,s)); // Missed brief selection must not settle at the line.
    s.phase=5;s.seconds=440;assert(entry());assert(arrivals.observe(arriving,s).finished);
    s.seconds=600;assert(!entry()); // Replay rewind cannot open a new visual window.
    window.reset();departures::Model model;Settings exits;History history;
    s={4,0,35,5,900,900};int loss=50;assert(entry());model.observe(exits,s,history);
    s.phase=1;
    for(int seconds=895;seconds>=600;seconds-=5){s.seconds=static_cast<float>(seconds);model.observe(exits,s,history);assert(!entry());assert(loss==50);}
    s.phase=3;s.seconds=590;assert(!entry());s.phase=4;s.seconds=580;assert(!entry());
    s.phase=5;s.seconds=570;assert(entry());model.observe(exits,s,history);loss=model.settle(s,loss).loss;assert(loss>50);int first=loss;
    s.seconds=500;model.observe(exits,s,history);assert(!entry());assert(loss==first); // Repeated selection frames do not apply accumulated exits.
    s.phase=1;s.seconds=490;assert(!entry());s.phase=3;s.seconds=480;assert(!entry());
    s.phase=5;s.seconds=450;assert(entry());model.observe(exits,s,history);loss=model.settle(s,loss).loss;assert(loss>=first);
    // New Q4 catch-up accrues pressure without changing visible attendance
    // until a fresh selection entry, even if the clock crosses the warning.
    window.reset();model.reset();s={4,14,14,5,720,720};loss=6;
    assert(entry());model.observe(exits,s,history);model.settle(s,loss);
    s.phase=1;s.seconds=600;s.awayScore=49;model.observe(exits,s,history);assert(!entry());
    for(int seconds=595;seconds>=110;seconds-=5){
        s.seconds=static_cast<float>(seconds);model.observe(exits,s,history);assert(!entry());assert(loss==6);
    }
    assert(model.catchupQueued);s.phase=3;assert(!entry());
    s.phase=5;assert(entry());model.observe(exits,s,history);
    auto catchup=model.settle(s,loss);loss=catchup.loss;
    // The 35-point late home loss can use eight extra quarter points, but
    // none of them became visible during the held lineup/live-view sequence.
    assert(catchup.catchup&&loss>51&&loss<=59);
    s.seconds=100;model.observe(exits,s,history);assert(!entry());assert(loss==catchup.loss);
    s.quarter=5;s.seconds=s.quarterSeconds;assert(entry()); // A real new quarter is a break even if the phase stays PlayCall.
    window.observe(456,State{1,0,0,6,900,900});assert(window.take(456,State{1,0,0,6,900,900}));
    window.reset();s={2,0,21,0,700,720};
    window.observe(789,s);assert(!window.take(789,s)); // None alone is not a Sim window.
    window.observe(789,s,true);assert(window.take(789,s,true));assert(!window.take(789,s,true));
    s.seconds=650;window.observe(789,s,true);assert(window.take(789,s,true)); // Hidden progress can settle without PlayCall.
    s.seconds=660;window.observe(789,s,true);assert(!window.take(789,s,true)); // Rewind remains blocked.
    s.seconds=640;window.observe(789,s,true);s.phase=1;window.observe(789,s,false);assert(!window.take(789,s,true));
    s.phase=3;s.seconds=600;window.observe(789,s,true);assert(!window.take(789,s,true)); // Visible Slow Sim does not open snap windows.
    s.phase=5;s.seconds=590;window.observe(789,s,true);assert(window.take(789,s,true));
    s.seconds=550;window.observe(789,s,true);assert(window.take(789,s,true)); // Normal/Fast holds PlayCall between simulated plays too.
    window.observe(789,s,true);assert(!window.take(789,s,true)); // Identical Sim frames never renew an opportunity.
    s.seconds=560;window.observe(789,s,true);assert(!window.take(789,s,true));
    s.seconds=540;s.phase=1;window.observe(789,s,true);assert(!window.take(789,s,true)); // Controller mode never licenses pre-snap changes.
    struct Sim {uintptr_t vt;int current,next;} controller{0x140000000+bindings::SuperSimVtable,1,1};
    constexpr uintptr_t image=0x140000000;uintptr_t address=reinterpret_cast<uintptr_t>(&controller);
    auto read=[&](uintptr_t p,void* out,size_t n){
        if(p==image+bindings::SuperSimController&&n==sizeof(address)){std::memcpy(out,&address,n);return true;}
        if(p>=address&&p+n<=address+sizeof(controller)){std::memcpy(out,reinterpret_cast<void*>(p),n);return true;}
        return false;
    };
    assert(simulation::offscreen(read,image,0));assert(!simulation::offscreen(read,image,1));
    assert(simulation::offscreen(read,image,5));
    for(int phase:{2,3,4,6,7,8})assert(!simulation::offscreen(read,image,phase));
    controller.next=0;assert(!simulation::offscreen(read,image,0));controller.next=1;controller.current=0;assert(!simulation::offscreen(read,image,0));
    assert(!simulation::offscreen(read,image,5));
    controller.current=1;controller.vt=0;assert(!simulation::offscreen(read,image,0));
    std::puts("PASS: arrivals/exits stay fixed at the line, during snaps and repeated selection frames; only selection entries, initial pregame and real breaks settle changes; throttling and replay guards.");
}
