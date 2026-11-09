#pragma once
#include "CurrencyTopbarUiPrograms.h"
#include <array>
#include <stdexcept>

namespace recruitingfog::currencyclassui {
// These are independent native classes. An unlinked/unsupported sibling must
// never prevent a verified active class from formatting its own native label.
inline std::vector<notesui::Program> recruitingPrograms(size_t index){
    static const auto programs=currencytopbarui::recruitingPrograms();
    constexpr std::array<unsigned,3> offsets={0x4d58,0x2c518,0x37668};
    if(programs.size()!=offsets.size()||index>=programs.size()||programs[index].offset!=offsets[index])throw std::runtime_error("Unexpected recruiting currency class contract");
    return {programs[index]};
}
}
