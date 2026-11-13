#pragma once
#include <windows.h>
#include <shlobj.h>
#ifdef small
#undef small
#endif
#include <filesystem>
#include <string>
#include <cwctype>
#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"ole32.lib")
namespace dynasty::userpaths {
inline bool equal(std::wstring a,std::wstring b){if(a.size()!=b.size())return false;for(size_t i=0;i<a.size();++i)if(std::towlower(a[i])!=std::towlower(b[i]))return false;return true;}
inline std::filesystem::path dataFromCode(const std::filesystem::path& code,const std::filesystem::path& expected){
    if(code.filename().wstring().find(L"Runtime-")!=0||!equal(code.parent_path().lexically_normal().wstring(),expected.lexically_normal().wstring()))return {};
    return expected;
}
inline std::filesystem::path data(const std::filesystem::path& code){
    PWSTR local=nullptr;std::filesystem::path expected;
    if(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData,0,nullptr,&local))&&local){expected=std::filesystem::path(local)/L"Ace"/L"DynastyCOT"/L"BeyondTheField";CoTaskMemFree(local);}
    return expected.empty()?expected:dataFromCode(code,expected);
}
inline std::filesystem::path program(const std::filesystem::path& folder,const std::filesystem::path& code,const char* key,const std::string& configured){
    if(std::string(key)=="node"&&configured=="../Shared/node.exe")return code/L"Shared"/L"node.exe";
    if(std::string(key)=="exporter"&&configured=="Tools/export_runtime.mjs")return code/L"Tools"/L"export_runtime.mjs";
    auto p=std::filesystem::u8path(configured);return std::filesystem::absolute(p.is_absolute()?p:folder/p);
}
}
