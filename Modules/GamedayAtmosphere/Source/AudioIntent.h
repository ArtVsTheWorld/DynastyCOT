#pragma once
#include <cstdint>
namespace attendance::audio {
using Address=uintptr_t;
struct Wanted {Address match=0;unsigned epoch=0;bool active=false;float remaining=1,strength=1,emphasis=1,arrivalFraction=1,reductionStart=.35f;};
}
