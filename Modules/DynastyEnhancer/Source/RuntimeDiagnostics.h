#pragma once
#include <filesystem>
#include <fstream>
#include <memory>
#include <chrono>
#include <ctime>
#include <string>
#include <deque>
#include "../Vendor/json.hpp"

namespace dynasty::diagnostics {
// Called only by the native worker after swapping out the bounded callback queue.
class RollingText {
    std::filesystem::path path;
    std::ofstream file;
    uintmax_t used=0;
    static constexpr uintmax_t limit=2u*1024*1024;
public:
    explicit RollingText(std::filesystem::path name):path(std::move(name)){
        std::error_code ec;
        if(std::filesystem::exists(path,ec))used=std::filesystem::file_size(path,ec);
        if(ec)used=limit;
    }
    void line(const std::string& text){
        if(used+text.size()+1>limit){
            file.close();file.clear();
            auto backup=path;backup+=L".1";
            std::error_code ec;std::filesystem::remove(backup,ec);
            if(ec)return;
            std::filesystem::rename(path,backup,ec);if(ec)return;
            used=0;
        }
        if(!file.is_open())file.open(path,std::ios::app|std::ios::binary);
        if(file){file<<text<<'\n';used+=text.size()+1;}
    }
};
inline void write(const std::filesystem::path& folder,const std::wstring& component,const std::deque<nlohmann::json>& batch) noexcept{
    if(batch.empty())return;
    try{
        RollingText detail(folder/(component+L".jsonl"));
        std::unique_ptr<RollingText> summary;
        for(const auto& record:batch){
            auto text=record.dump(-1,' ',false,nlohmann::json::error_handler_t::replace);
            if(text.size()>32768)text=nlohmann::json{{"event",record.value("event",std::string("diagnostic"))},{"originalBytes",text.size()},{"detail","Oversized diagnostic omitted"}}.dump();
            detail.line(text);
            const auto message=record.find("summary");
            if(message!=record.end()&&message->is_string()){
                if(!summary)summary=std::make_unique<RollingText>(folder/(component+L"-status.log"));
                const auto now=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::tm utc{};gmtime_s(&utc,&now);char stamp[24]{};std::strftime(stamp,sizeof(stamp),"%Y-%m-%d %H:%M:%S UTC",&utc);
                summary->line(std::string(stamp)+" | "+message->get<std::string>().substr(0,2048));
            }
        }
    }catch(...){/* A diagnostic write must not change feature activation. */}
}
}
