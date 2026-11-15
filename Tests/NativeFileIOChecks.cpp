#include "../Core/NativeFileIO.h"
#include "../Modules/DynastyEnhancer/Source/UserRuntimePaths.h"
#include "../Modules/GamedayAtmosphere/Source/RuntimePaths.h"
#include <iostream>
#include <sstream>
int wmain(int argc,wchar_t** argv){try{
 if(argc!=2)return 2;const std::filesystem::path root=argv[1];if(!aceio::exists(root/L".cot-fixture"))return 3;int checks=0;
 auto check=[&](bool ok,const char* why){if(!ok)throw std::runtime_error(why);++checks;};
 for(const auto desired:{235u,248u,259u,260u,262u,324u,430u}){
  auto folder=root;while(folder.native().size()<desired){size_t n=desired-folder.native().size()-1;if(n>70)n=70;folder/=std::wstring(n,L'x');}
  aceio::create_directories(folder);auto original=folder/L"history.json",temporary=folder/L"new.tmp",copy=folder/L"copy.bak",ini=folder/L"settings.ini";
  {aceio::ofstream f(original,std::ios::binary);f<<"original published wording";check(f.good(),"write history");}
  check(aceio::exists(original)&&aceio::file_size(original)==26,"read size");aceio::ifstream read(original,std::ios::binary);std::string text;std::getline(read,text);check(text=="original published wording","read history");read.close();
  check(aceio::copy_file(original,copy),"copy");check(aceio::is_regular_file(copy)&&aceio::is_directory(folder),"type checks");
  {aceio::ofstream f(temporary);f<<"replacement";}
  check(aceio::move_file(temporary.c_str(),original.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0,"atomic commit");
  aceio::ifstream before(copy);std::getline(before,text);check(text=="original published wording","backup retained");before.close();
  check(aceio::ini_write(L"Settings",L"Value",L"42",ini.c_str())!=0,"INI write");check(aceio::ini_int(L"Settings",L"Value",0,ini.c_str())==42,"INI read");
  wchar_t value[20]{};aceio::ini_string(L"Settings",L"Value",L"",value,20,ini.c_str());check(std::wstring(value)==L"42","INI string");
  check(aceio::logical_path(aceio::io_path(original))==original,"logical identity");std::error_code error;check(!aceio::exists(folder/L"missing",error)&&!error,"missing file");
  check(aceio::last_write_time(original,error)!=std::filesystem::file_time_type{}&&!error,"file time");
  aceio::rename(copy,temporary,error);check(!error&&aceio::exists(temporary),"rename");check(aceio::remove(temporary,error)&&!error,"cleanup");
  const auto data=folder/L"Ace"/L"DynastyCOT"/L"BeyondTheField";const auto code=data/L"Runtime-1.0.26";
  check(dynasty::userpaths::dataFromCode(aceio::io_path(code),data)==data,"BTF prefix-neutral ownership");
  attendance::runtimepaths::Layout layout;auto gda=folder/L"Ace"/L"DynastyCOT"/L"GamedayAtmosphere"/L"Runtime-2.1.40"/L"GamedayAtmosphereNative-2.1.40.dll";
  check(attendance::runtimepaths::resolve(aceio::io_path(gda),layout)&&layout.runtime==gda.parent_path(),"GDA logical ownership");
 }
 bool rejected=false;try{aceio::io_path(L"\\\\?\\GLOBALROOT\\Device\\bad");}catch(...){rejected=true;}check(rejected,"unsupported namespace");
 std::cout<<"PASS "<<checks<<" native file/INI/history/atomic/ownership checks; no game calls\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
