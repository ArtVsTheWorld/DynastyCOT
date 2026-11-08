#pragma once
#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cwchar>
#include <cwctype>
namespace attendance::config {
enum class Status {Default,Valid,Invalid,Clamped};
template<class T>struct Value {T value;Status status;};
inline bool endOfValue(const wchar_t* end){while(std::iswspace(*end))++end;return !*end||*end==L';'||*end==L'#';}
inline Value<int> integer(const wchar_t* raw,int fallback,int low,int high){
    if(!raw||!*raw)return {fallback,Status::Default};
    wchar_t* end=nullptr;errno=0;auto n=std::wcstoll(raw,&end,10);
    if(end==raw||errno==ERANGE||!endOfValue(end))return {fallback,Status::Invalid};
    if(n<low)return {low,Status::Clamped};if(n>high)return {high,Status::Clamped};return {static_cast<int>(n),Status::Valid};
}
inline Value<float> real(const wchar_t* raw,float fallback,float low,float high){
    if(!raw||!*raw)return {fallback,Status::Default};
    wchar_t* end=nullptr;errno=0;float n=std::wcstof(raw,&end);
    if(end==raw||errno==ERANGE||!std::isfinite(n)||!endOfValue(end))return {fallback,Status::Invalid};
    if(n<low)return {low,Status::Clamped};if(n>high)return {high,Status::Clamped};return {n,Status::Valid};
}
}
