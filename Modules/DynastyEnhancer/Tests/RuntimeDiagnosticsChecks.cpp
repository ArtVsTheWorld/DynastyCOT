#include "../Source/RuntimeDiagnostics.h"
#include <cassert>
#include <iostream>
int main(){
    namespace fs=std::filesystem;using namespace dynasty::diagnostics;
    const auto dir=fs::current_path()/("log-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));fs::create_directory(dir);
    const auto log=dir/"test.jsonl",brief=dir/"test-status.log";{std::ofstream full(log);full<<std::string(2u*1024*1024,'x');}
    write(dir,L"test",{{{"event","active"},{"summary","Verified feature"}},{{"event","oversized"},{"detail",std::string(35000,'x')}}});
    assert(fs::file_size(log)<32768&&fs::file_size(dir/"test.jsonl.1")==2u*1024*1024&&!fs::exists(dir/"test.jsonl.2"));
    std::ifstream in(log);nlohmann::json a,b;in>>a>>b;assert(a.at("summary")=="Verified feature"&&b.at("originalBytes")>32768);
    std::ifstream s(brief);std::string line;std::getline(s,line);assert(line.find("UTC | Verified feature")!=std::string::npos);
    write(dir/"missing",L"test",{{{"event","unwritable"}}});assert(!fs::exists(dir/"missing"));
    std::cout<<"PASS BTF status/detail separation, long-record bound, two-file rotation and unavailable storage\n";
}
