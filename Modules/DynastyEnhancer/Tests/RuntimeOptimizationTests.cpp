#include "../Source/RuntimeWire.h"
#include "../Source/FeedRefresh.h"
#include <tlhelp32.h>
#include <psapi.h>
#include <fstream>
#include <iostream>
#include <cassert>
using namespace dynasty::live;
uint64_t micros(){LARGE_INTEGER now{},frequency{};QueryPerformanceCounter(&now);QueryPerformanceFrequency(&frequency);return static_cast<uint64_t>(now.QuadPart*1000000/frequency.QuadPart);}
Json load(const std::filesystem::path& file){std::ifstream in(file);Json value;in>>value;return value;}
void save(const std::filesystem::path& file,const Json& value){std::ofstream out(file,std::ios::binary);out<<value.dump();if(!out)throw std::runtime_error("Private evidence write failed");}
int wmain(int argc,wchar_t** argv){
 try{
  if(argc==4&&std::wstring(argv[1])==L"--wire"){
   auto raw=load(argv[2]);const auto began=micros();auto wire=generatorWire(raw);auto encoded=micros();auto text=wire.dump();auto ended=micros();
   std::ofstream out(std::filesystem::path(argv[3]),std::ios::binary);out<<text;out.close();
   std::cout<<Json{{"wireMs",(encoded-began)/1000.},{"serializeMs",(ended-encoded)/1000.},{"bytes",text.size()},{"rows",wire.at("rows").size()}}.dump()<<'\n';return 0;
  }
  if(argc==4&&std::wstring(argv[1])==L"--live"){
   DWORD pid=std::stoul(argv[2]);HANDLE process=OpenProcess(PROCESS_VM_READ|PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);if(!process)throw std::runtime_error("Read-only process unavailable");
   wchar_t exe[32768]{};DWORD size=32768;if(!QueryFullProcessImageNameW(process,0,exe,&size)||_wcsicmp(std::filesystem::path(exe).filename().c_str(),L"CollegeFB27.exe"))throw std::runtime_error("Expected game process");
   HANDLE modules=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pid);MODULEENTRY32W entry{};entry.dwSize=sizeof(entry);Address base=0;if(modules!=INVALID_HANDLE_VALUE){if(Module32FirstW(modules,&entry))base=reinterpret_cast<Address>(entry.modBaseAddr);CloseHandle(modules);}if(!base)throw std::runtime_error("No native image");
   Json measurements=Json::array();std::filesystem::path folder(argv[3]);
   for(int i=0;i<3;++i){
    PackedChangeWatch proof,identity;const auto began=micros();auto raw=Capture(base,process).run(&proof,&identity);auto captured=micros();auto oldText=raw.at("records").dump();auto hashed=micros();
    Sleep(750);const auto verify=micros();size_t readBytes=0,readCalls=0;bool same=proof.unchanged([&](uintptr_t address,void* out,size_t bytes){SIZE_T got=0;readBytes+=bytes;++readCalls;return ReadProcessMemory(process,reinterpret_cast<void*>(address),out,bytes,&got)&&got==bytes;});auto verified=micros();
    auto wire=generatorWire(raw);auto compacted=micros();auto wireText=wire.dump();auto serialized=micros();
    measurements.push_back({{"captureMs",(captured-began)/1000.},{"recordsDumpMs",(hashed-captured)/1000.},{"settleMs",(verify-hashed)/1000.},{"proofMs",(verified-verify)/1000.},{"wireMs",(compacted-verified)/1000.},{"wireSerializeMs",(serialized-compacted)/1000.},{"proofMatched",same},{"proofBytes",readBytes},{"proofCalls",readCalls},{"rawBytes",oldText.size()},{"wireBytes",wireText.size()},{"records",raw.at("records").size()},{"status",raw.at("status")}});
    if(i==0){save(folder/L"live-capture.json",raw);std::ofstream out(folder/L"live-wire.json",std::ios::binary);out<<wireText;}
   }
   CloseHandle(process);save(folder/L"native-measurements.json",measurements);std::cout<<measurements.dump()<<'\n';return 0;
  }
  if(argc==5&&std::wstring(argv[1])==L"--gate"){
   std::filesystem::path node(argv[2]),script(argv[3]),folder(argv[4]);std::filesystem::create_directories(folder);
   social::refresh::Settings config{{},node,script,folder/L"input.json"};save(config.config,{{"fixture",true}});
   bool called=false;auto code=social::refresh::exportSave(config,folder/L"output.json",folder/L"child.log",[]{return false;},[&]{called=true;return true;});assert(called&&code==0&&std::filesystem::exists(folder/L"committed.json"));
   std::filesystem::path denied=folder/L"Denied";std::filesystem::create_directories(denied);config.config=denied/L"input.json";save(config.config,{{"fixture",true}});bool refused=false;
   try{social::refresh::exportSave(config,denied/L"output.json",denied/L"child.log",[]{return false;},[]{return false;});}catch(const std::exception&){refused=true;}
   assert(refused&&!std::filesystem::exists(denied/L"committed.json"));
   std::cout<<"PASS native commit-gate approve/deny, no denied history write.\n";return 0;
  }
  Json raw={{"format",1},{"sourceKind","runtime"},{"status",{{"league",7}}},{"records",Json::object()}};
  raw["records"]["p1"]={{"key","p1"},{"type","Player"},{"capacity",100},{"v",{{"A",0},{"B",nullptr},{"C","p2"}}}};
  raw["records"]["p2"]={{"key","p2"},{"type","Player"},{"capacity",100},{"v",{{"A",1},{"B",false},{"C","p1"}}}};
  auto wire=generatorWire(raw);assert(wire.at("format")==2&&wire.at("types").size()==1&&wire.at("rows").size()==2&&wire.at("status")==raw.at("status"));
  PackedChangeWatch proof;std::vector<unsigned char> bytes(4096,7);proof.record(0x100000,bytes);auto read=[&](uintptr_t at,void* out,size_t n){memcpy(out,bytes.data()+at-0x100000,n);return true;};
  assert(proof.unchanged(read));bytes[100]=8;assert(!proof.unchanged(read));bytes[100]=7;assert(!proof.unchanged([](uintptr_t,void*,size_t){return false;}));
  assert(proof.unchanged(read));std::cout<<"PASS native lossless wire dictionary and complete consumed-byte settlement proof.\n";
 }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
