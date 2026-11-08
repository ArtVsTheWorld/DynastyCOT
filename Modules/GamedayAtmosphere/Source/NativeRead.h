#pragma once
#include <cstdint>
#include <type_traits>
namespace attendance {
// A failed read may have modified part of its destination. Never publish that
// partial value as a pointer, flag, clock or native result.
template<class T,class Read>T nativeValue(uintptr_t address,Read read){
    static_assert(std::is_trivially_copyable_v<T>);
    T value{};return read(address,&value,sizeof(value))?value:T{};
}
}
