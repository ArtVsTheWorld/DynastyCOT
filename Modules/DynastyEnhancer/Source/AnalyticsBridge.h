#pragma once
#include <windows.h>
#include <cstdint>
namespace cotanalytics {
inline constexpr uint32_t abi=1;
#ifndef COT_ANALYTICS_BUILD_ID
#define COT_ANALYTICS_BUILD_ID 0ull
#endif
inline constexpr uint64_t buildId=COT_ANALYTICS_BUILD_ID;
// Same-build, in-process menu coordination. Fog remains sole owner of the
// shared spreadsheet hooks; Social remains sole owner of navigation hooks.
struct Api {uint32_t size,version;uint64_t build;bool(*supported)();bool(*available)();bool(*open)();void(*request)();void(*leave)();};
inline bool compatible(const Api& value){return buildId&&value.size==sizeof(Api)&&value.version==abi&&value.build==buildId;}
using GetApi=const Api*(__cdecl*)();
}
