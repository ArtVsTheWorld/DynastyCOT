#pragma once
#include <filesystem>
#include <string>
#include <cwctype>
namespace attendance::runtimepaths {
struct Layout {std::filesystem::path runtime,data,plugins;};
inline bool equal(std::wstring a,std::wstring b){
    if(a.size()!=b.size())return false;
    for(size_t i=0;i<a.size();++i)if(std::towlower(a[i])!=std::towlower(b[i]))return false;
    return true;
}
inline bool resolve(const std::filesystem::path& module,Layout& out){
    auto runtime=module.lexically_normal().parent_path();auto owner=runtime.parent_path();
    if(runtime.filename().wstring().find(L"Runtime-")!=0)return false;
    Layout next;next.runtime=runtime;
    if(equal(owner.filename().wstring(),L"DGA-Runtime")){
        next.plugins=owner.parent_path()/L"Plugins";next.data=next.plugins/L"DynamicGamedayAtmosphere";
    }else if(equal(owner.filename().wstring(),L"DynamicGamedayAtmosphere")&&equal(owner.parent_path().filename().wstring(),L"Plugins")){
        next.data=owner;next.plugins=owner.parent_path(); // Earlier on-disk layout.
    }else return false;
    out=next;return true;
}
}
