#ifndef COT_FEED_HEADER
#define COT_FEED_HEADER "../Source/FeedData.h"
#endif
#include COT_FEED_HEADER
#include <chrono>
#include <iostream>
// Offline comparison only. These timings do not measure native widgets/FPS.
int main(int argc,char** argv){
    if(argc!=2)return 2;
    try{
        std::ifstream file(argv[1]);nlohmann::json data;file>>data;
        using Clock=std::chrono::steady_clock;auto begin=Clock::now();auto feed=social::decodeFeed(data);auto decoded=Clock::now();
        nlohmann::json report={{"decodeMs",std::chrono::duration<double,std::milli>(decoded-begin).count()},{"editions",nlohmann::json::array()}};
        for(size_t week=0;week<feed->catalog->weeks.size();++week){
            size_t displayed=0,eligible=0;auto start=Clock::now();
            for(int n=0;n<50;++n){auto selected=social::selectFeed(feed->catalog,week,1);displayed=selected->posts.size();eligible=selected->total;}
            report["editions"].push_back({{"label",feed->catalog->weeks[week].label},{"cards",displayed},{"eligible",eligible},{"selectMeanMs",std::chrono::duration<double,std::milli>(Clock::now()-start).count()/50}});
        }
        std::cout<<report.dump(2)<<'\n';return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
