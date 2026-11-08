#pragma once
#include <cstdint>
namespace attendance::replaymode {
enum class Mode { Unknown, PlayCall, RealtimeSim };
struct Sample {bool known=false,realtime=false,playcallOpen=false;};
// Capability belongs to this match and presentation session, not the game
// phase. Rendered Slow Sim also visits phases 4/5 and simulation mode zero.
class Tracker {
    uintptr_t match_=0;unsigned epoch_=0;bool seenPlaycall_=false;Mode mode_=Mode::Unknown;
public:
    Mode update(uintptr_t match,unsigned epoch,Sample s){
        if(!match||match!=match_||epoch!=epoch_){match_=match;epoch_=epoch;seenPlaycall_=false;mode_=Mode::Unknown;}
        if(!match||!s.known){seenPlaycall_=false;return mode_=Mode::Unknown;}
        if(s.realtime){seenPlaycall_=false;return mode_=Mode::RealtimeSim;}
        if(s.playcallOpen)seenPlaycall_=true;
        return mode_=seenPlaycall_?Mode::PlayCall:Mode::Unknown;
    }
    Mode mode()const{return mode_;}
};
}
