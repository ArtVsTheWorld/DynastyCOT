#pragma once
#include <filesystem>
#include <string>
#include <cwctype>
#include <fstream>
#include <sstream>
#include <cstdint>
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
    if(equal(owner.filename().wstring(),L"GamedayAtmosphere")&&equal(owner.parent_path().filename().wstring(),L"DynastyCOT")){
        next.data=owner; // Shared per-user data/cache; MMC origin is PID-bound.
    }else if(equal(owner.filename().wstring(),L"DGA-Runtime")){
        next.plugins=owner.parent_path()/L"Plugins";next.data=next.plugins/L"DynamicGamedayAtmosphere";
    }else if(equal(owner.filename().wstring(),L"DynamicGamedayAtmosphere")&&equal(owner.parent_path().filename().wstring(),L"Plugins")){
        next.data=owner;next.plugins=owner.parent_path(); // Earlier on-disk layout.
    }else return false;
    out=next;return true;
}
// The origin is needed only for the existing read-only MidgameSave bridge.
// Never infer an MMC installation from a per-user cache path.
inline bool originText(const std::string& text,unsigned pid,uint64_t creation,std::filesystem::path& plugins){
    if(text.size()>4096||!pid||!creation)return false;
    std::istringstream in(text);std::string magic,pidText,stamp,path,extra;
    if(!std::getline(in,magic)||!std::getline(in,pidText)||!std::getline(in,stamp)||!std::getline(in,path)||std::getline(in,extra)||magic!="DGA-ORIGIN-1")return false;
    try{size_t n=0;if(std::stoull(pidText,&n)!=pid||n!=pidText.size())return false;
        if(std::stoull(stamp,&n)!=creation||n!=stamp.size())return false;
        auto candidate=std::filesystem::u8path(path).lexically_normal();
        if(!candidate.is_absolute()||!equal(candidate.filename().wstring(),L"Plugins"))return false;
        plugins=candidate;return true;
    }catch(...){return false;}
}
inline bool loadOrigin(Layout& layout,unsigned pid,uint64_t creation){
    auto file=layout.runtime/(L"origin-"+std::to_wstring(pid)+L".txt");std::error_code error;
    auto size=std::filesystem::file_size(file,error);if(error||size>4096)return false;
    std::ifstream input(file,std::ios::binary);if(!input)return false;
    std::string text(static_cast<size_t>(size),'\0');input.read(text.data(),static_cast<std::streamsize>(text.size()));
    return input&&originText(text,pid,creation,layout.plugins);
}
}
