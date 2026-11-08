#pragma once
#include <atomic>
#include <windows.h>
namespace attendance {
// Presentation and replay callbacks can both request an atmosphere update.
// Never run their pre-lock snapshots/diagnostics concurrently or recursively.
class CallbackGate {
    std::atomic<bool> busy_{false};
    static bool invoke(void(*callback)()){try{callback();return true;}catch(...){return false;}}
public:
    bool run(void(*callback)()){
        if(busy_.exchange(true,std::memory_order_acquire))return true;
        bool okay=false;
        __try{okay=invoke(callback);}__except(EXCEPTION_EXECUTE_HANDLER){okay=false;}
        busy_.store(false,std::memory_order_release);return okay;
    }
};
}
