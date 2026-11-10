#pragma once
#include <stdexcept>
#include <string>

namespace recruitingfog::startup {
enum class State { Waiting, Ready, Rejected };
struct Result { State state; std::string reason; };

// Read-only readiness gate. Never authorizes an unknown fingerprint or owner.
// A short stable window prevents transient initialization from consuming the
// runtime's only activation attempt. The caller installs hooks only on success.
template<class Cancel, class Probe, class Report, class Pause>
bool awaitVerified(Cancel cancelled, Probe probe, Report report, Pause pause,
                   unsigned attempts=240, unsigned stableReads=2) {
    if (!attempts || !stableReads) throw std::invalid_argument("Invalid startup readiness budget");
    unsigned stable=0;
    std::string previous;
    for (unsigned i=0; i<attempts; ++i) {
        if (cancelled()) return false;
        const auto result=probe();
        if (result.state==State::Rejected) throw std::runtime_error(result.reason);
        if (result.state==State::Ready) {
            if (++stable>=stableReads) return !cancelled();
        } else {
            stable=0;
            if (result.reason!=previous) { previous=result.reason; report(previous); }
        }
        if (i+1<attempts) pause();
    }
    throw std::runtime_error("Timed out waiting for verified Dynasty settings code: "+previous);
}
}
