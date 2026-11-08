#pragma once
#include <cstdint>
#include <filesystem>

namespace attendance {
// Called by the single settings worker. Keep live editing without reparsing an
// unchanged INI every second. Periodic refresh also catches preserved timestamps.
class SettingsPoll {
    bool seen=false,hadFile=false;
    std::filesystem::file_time_type stamp{};
    uintmax_t bytes=0;
    uint64_t lastRead=0;
public:
    bool due(const std::filesystem::path& path,uint64_t now){
        std::error_code ec;
        auto nextStamp=std::filesystem::last_write_time(path,ec);
        bool available=!ec;
        uintmax_t nextBytes=0;
        if(available){nextBytes=std::filesystem::file_size(path,ec);available=!ec;}
        bool changed=!seen||available!=hadFile||(available&&(stamp!=nextStamp||bytes!=nextBytes));
        bool refresh=changed||now<lastRead||now-lastRead>=10000;
        seen=true;hadFile=available;stamp=nextStamp;bytes=nextBytes;
        if(refresh)lastRead=now;
        return refresh;
    }
};
}
