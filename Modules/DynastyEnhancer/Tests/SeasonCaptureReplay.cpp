// Offline fixture only. No game handles, calls, injection or table writes.
#include "AnalyticsCapture.h"
#include <fstream>
#include <iostream>
#include <chrono>
int main(int argc,char** argv){try{
    if(argc!=3)throw std::runtime_error("input.json output.json required");
    std::ifstream in(argv[1],std::ios::binary);analytics::Json raw;in>>raw;
    analytics::Json report={{"runsMs",analytics::Json::array()}},reference;
    for(int i=0;i<3;++i){const auto began=std::chrono::steady_clock::now();auto data=analytics::live::dataset(raw);
        report["runsMs"].push_back(std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count());
        if(i&&data!=reference)throw std::runtime_error("Analytics dataset changed on refresh");
        reference=std::move(data);
    }
    // Exact dataset serialization is compared between baseline and candidate.
    const auto text=reference.dump();uint64_t hash=14695981039346656037ull;for(unsigned char c:text){hash^=c;hash*=1099511628211ull;}
    report["datasetHash"]=std::to_string(hash);report["datasetBytes"]=text.size();report["rows"]=reference.at("rows").size();report["seasonRows"]=reference.at("seasonAnalytics").size();report["issues"]=reference.at("issues").size();
    std::ofstream out(argv[2]);out<<report.dump(2);std::cout<<report.dump()<<"\n";
}catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}return 0;}
