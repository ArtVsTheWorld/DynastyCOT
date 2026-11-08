#pragma once
#include <windows.h>
#include <string>
#include <sstream>
#include <iomanip>
namespace attendance::loader {
inline std::string utf8(const std::wstring& text){
    if(text.empty())return {};int size=WideCharToMultiByte(CP_UTF8,0,text.data(),static_cast<int>(text.size()),nullptr,0,nullptr,nullptr);
    if(size<=0)return "[path could not be converted to UTF-8]";
    std::string out(static_cast<size_t>(size),'\0');WideCharToMultiByte(CP_UTF8,0,text.data(),static_cast<int>(text.size()),out.data(),size,nullptr,nullptr);return out;
}
inline std::string description(DWORD error){
    wchar_t buffer[1024]{};DWORD length=FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,nullptr,error,0,buffer,1024,nullptr);
    if(!length)return "No system description available";
    std::wstring text(buffer,length);for(auto& c:text)if(c==L'\r'||c==L'\n')c=L' ';while(!text.empty()&&text.back()==L' ')text.pop_back();return utf8(text);
}
inline std::string diagnostic(DWORD pid,const char* operation,DWORD error,const std::wstring& detail={}){
    std::ostringstream out;out<<"PID "<<pid<<" | Step: "<<operation;
    if(error)out<<" | Windows error "<<error<<" (0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<error<<std::dec<<"): "<<description(error);
    if(!detail.empty())out<<" | "<<utf8(detail);return out.str();
}
}
