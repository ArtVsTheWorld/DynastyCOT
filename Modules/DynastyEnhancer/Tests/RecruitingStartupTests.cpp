#include "../Source/RecruitingStartup.h"
#include <cassert>
#include <iostream>

int main() {
    using namespace recruitingfog::startup;
    unsigned checks=0;
    for (unsigned delay=0;delay<100;++delay) {
        unsigned probes=0,pauses=0,reports=0;
        const auto ready=awaitVerified([]{return false;},[&]{
            return ++probes<=delay?Result{State::Waiting,"Code initializing"}:Result{State::Ready,{}};
        },[&](const auto&){++reports;},[&]{++pauses;});
        assert(ready&&probes==delay+2&&pauses==delay+1&&reports==(delay?1u:0u));++checks;
    }
    {
        unsigned probes=0,pauses=0,reports=0;
        bool rejected=false;
        try { awaitVerified([]{return false;},[&]{++probes;return Result{State::Waiting,"Unknown bytes"};},
                            [&](const auto&){++reports;},[&]{++pauses;},4); }
        catch(const std::runtime_error& e){rejected=std::string(e.what()).find("Timed out")!=std::string::npos;}
        assert(rejected&&probes==4&&pauses==3&&reports==1);++checks;
    }
    {
        unsigned probes=0,pauses=0;bool rejected=false;
        try { awaitVerified([]{return false;},[&]{++probes;return Result{State::Rejected,"Foreign owner"};},
                            [](const auto&){assert(false);},[&]{++pauses;}); }
        catch(const std::runtime_error& e){rejected=std::string(e.what())=="Foreign owner";}
        assert(rejected&&probes==1&&pauses==0);++checks;
    }
    {
        bool cancelled=false;unsigned probes=0;
        assert(!awaitVerified([&]{return cancelled;},[&]{++probes;return Result{State::Ready,{}};},
                              [](const auto&){},[&]{cancelled=true;}));
        assert(probes==1);++checks;
        probes=0;
        assert(!awaitVerified([&]{return cancelled;},[&]{++probes;return Result{State::Ready,{}};},
                              [](const auto&){},[]{}));
        assert(probes==0);++checks;
    }
    {
        unsigned probes=0;
        assert(awaitVerified([]{return false;},[&]{++probes;return Result{probes==2?State::Waiting:State::Ready,"Transient"};},
                             [](const auto&){},[]{},5));
        assert(probes==4);++checks;
    }
    {
        bool cancelled=false;
        assert(!awaitVerified([&]{return cancelled;},[&]{cancelled=true;return Result{State::Ready,{}};},
                              [](const auto&){},[]{},1,1));++checks;
    }
    std::cout<<"PASS: "<<checks<<" recruiting startup cases; delayed/unstable code, bounded timeout, foreign ownership and cancellation. No game access.\n";
}
