#pragma once
#include <atomic>
#include <chrono>
#include <cstdint>
namespace attendance::timing {
struct Counts {
    std::atomic<uint64_t> calls{0},maximumUs{0},over4ms{0},over16ms{0};
    void record(uint64_t us){
        ++calls;if(us>=4000)++over4ms;if(us>=16000)++over16ms;
        auto previous=maximumUs.load();while(previous<us&&!maximumUs.compare_exchange_weak(previous,us)){}
    }
};
class Scope {
    Counts& counts_;std::chrono::steady_clock::time_point start_=std::chrono::steady_clock::now();
public:
    explicit Scope(Counts& counts):counts_(counts){}
    ~Scope(){counts_.record(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-start_).count()));}
};
// Measure only the adapter's work around a forwarded native call. Nested
// adapters get their own measurement; the outer one pauses while forwarding.
template<class Clock=std::chrono::steady_clock>class ExclusiveScope {
    Counts& counts_;std::atomic<uint64_t>* interval_;
    typename Clock::time_point start_=Clock::now();
    typename Clock::duration elapsed_{};bool running_=true;
public:
    explicit ExclusiveScope(Counts& counts,std::atomic<uint64_t>* interval=nullptr):counts_(counts),interval_(interval){}
    void pause(){if(running_){elapsed_+=Clock::now()-start_;running_=false;}}
    void resume(){if(!running_){start_=Clock::now();running_=true;}}
    ~ExclusiveScope(){pause();auto ns=static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed_).count());counts_.record(ns/1000);if(interval_)interval_->fetch_add(ns);}
};
}
