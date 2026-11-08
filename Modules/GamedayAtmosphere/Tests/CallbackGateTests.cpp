#include "../Source/CallbackGate.h"
#include <thread>
#include <vector>
#include <cassert>
#include <stdexcept>
#include <cstdio>
attendance::CallbackGate gate;
std::atomic<unsigned> inside{0},calls{0};
void body(){assert(inside.fetch_add(1)==0);++calls;assert(gate.run(body));std::this_thread::yield();assert(inside.fetch_sub(1)==1);}
void cppFault(){throw std::runtime_error("test");}
void nativeFault(){RaiseException(EXCEPTION_ACCESS_VIOLATION,0,0,nullptr);}
int main(){
    std::vector<std::thread> threads;for(unsigned n=0;n<8;++n)threads.emplace_back([]{for(unsigned i=0;i<2000;++i)assert(gate.run(body));});
    for(auto& t:threads)t.join();assert(calls>0&&inside==0);
    assert(!gate.run(cppFault));assert(gate.run(body));assert(!gate.run(nativeFault));assert(gate.run(body));
    std::puts("PASS: concurrent presentation/replay update exclusion, safe recursive skips, and reusable gate after both C++ and native exceptions.");
}
