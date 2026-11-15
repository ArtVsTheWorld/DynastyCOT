#pragma once
// COT-owned file I/O only. Never use extended paths as persisted identities,
// fixture keys, ownership checks or user-visible locations.
#include <windows.h>
#ifdef small
#undef small
#endif
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <stdexcept>
namespace aceio {
namespace fs=std::filesystem;
inline fs::path logical_path(const fs::path& p){
 auto s=p.wstring();if(s.rfind(L"\\\\?\\UNC\\",0)==0)return fs::path(L"\\\\"+s.substr(8));
 if(s.rfind(L"\\\\?\\",0)==0){if(s.size()<7||s[5]!=L':'||s[6]!=L'\\')throw std::runtime_error("Unsupported storage namespace");return fs::path(s.substr(4));}
 if(s.rfind(L"\\\\.\\",0)==0)throw std::runtime_error("Device storage paths are unsupported");return p;
}
inline fs::path io_path(const fs::path& p){
 auto clean=logical_path(p);if(clean.native().size()<248)return clean;
 std::vector<wchar_t> b(32768);DWORD n=GetFullPathNameW(clean.c_str(),static_cast<DWORD>(b.size()),b.data(),nullptr);
 if(!n||n>=b.size())throw std::runtime_error("Cannot normalize long storage path");std::wstring s(b.data(),n);
 return fs::path(s.rfind(L"\\\\",0)==0?L"\\\\?\\UNC\\"+s.substr(2):L"\\\\?\\"+s);
}
#define ACE_IO_UNARY(name) template<class... A> inline auto name(const fs::path& p,A&&... args)->decltype(fs::name(p,std::forward<A>(args)...)){return fs::name(io_path(p),std::forward<A>(args)...);}
ACE_IO_UNARY(exists)
ACE_IO_UNARY(file_size)
ACE_IO_UNARY(is_directory)
ACE_IO_UNARY(is_regular_file)
ACE_IO_UNARY(create_directories)
ACE_IO_UNARY(remove)
ACE_IO_UNARY(last_write_time)
#undef ACE_IO_UNARY
#define ACE_IO_BINARY(name) template<class... A> inline auto name(const fs::path& a,const fs::path& b,A&&... args)->decltype(fs::name(a,b,std::forward<A>(args)...)){return fs::name(io_path(a),io_path(b),std::forward<A>(args)...);}
ACE_IO_BINARY(rename)
ACE_IO_BINARY(copy_file)
#undef ACE_IO_BINARY
class ifstream:public std::ifstream {public:ifstream()=default;explicit ifstream(const fs::path& p,std::ios::openmode mode=std::ios::in):std::ifstream(io_path(p),mode){}void open(const fs::path& p,std::ios::openmode mode=std::ios::in){std::ifstream::open(io_path(p),mode);}};
class ofstream:public std::ofstream {public:ofstream()=default;explicit ofstream(const fs::path& p,std::ios::openmode mode=std::ios::out):std::ofstream(io_path(p),mode){}void open(const fs::path& p,std::ios::openmode mode=std::ios::out){std::ofstream::open(io_path(p),mode);}};
class fstream:public std::fstream {public:fstream()=default;explicit fstream(const fs::path& p,std::ios::openmode mode=std::ios::in|std::ios::out):std::fstream(io_path(p),mode){}void open(const fs::path& p,std::ios::openmode mode=std::ios::in|std::ios::out){std::fstream::open(io_path(p),mode);}};
inline HANDLE create_file(LPCWSTR p,DWORD access,DWORD share,LPSECURITY_ATTRIBUTES sa,DWORD creation,DWORD flags,HANDLE templateFile){return CreateFileW(p?io_path(p).c_str():p,access,share,sa,creation,flags,templateFile);}
inline BOOL copy_file_native(LPCWSTR a,LPCWSTR b,BOOL fail){return CopyFileW(io_path(a).c_str(),io_path(b).c_str(),fail);}
inline BOOL move_file(LPCWSTR a,LPCWSTR b,DWORD flags){return MoveFileExW(io_path(a).c_str(),b?io_path(b).c_str():b,flags);}
inline UINT ini_int(LPCWSTR section,LPCWSTR key,INT fallback,LPCWSTR p){return GetPrivateProfileIntW(section,key,fallback,p?io_path(p).c_str():p);}
inline DWORD ini_string(LPCWSTR section,LPCWSTR key,LPCWSTR fallback,LPWSTR out,DWORD size,LPCWSTR p){return GetPrivateProfileStringW(section,key,fallback,out,size,p?io_path(p).c_str():p);}
inline BOOL ini_write(LPCWSTR section,LPCWSTR key,LPCWSTR value,LPCWSTR p){return WritePrivateProfileStringW(section,key,value,p?io_path(p).c_str():p);}
inline DWORD attributes(LPCWSTR p){return GetFileAttributesW(io_path(p).c_str());}
}
