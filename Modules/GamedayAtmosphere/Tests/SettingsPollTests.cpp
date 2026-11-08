#include "../Source/SettingsPoll.h"
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
int main(){
    auto path=std::filesystem::temp_directory_path()/("DgaSettingsPoll-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+".ini");
    attendance::SettingsPoll poll;
    assert(poll.due(path,100)); // First read, even with no file.
    assert(!poll.due(path,1100));
    {std::ofstream f(path);f<<"Enabled=1\n";}
    assert(poll.due(path,2100)); // Creation is noticed immediately.
    auto stamp=std::filesystem::last_write_time(path);
    assert(!poll.due(path,3100));
    {std::ofstream f(path);f<<"Enabled=0\n";}
    std::filesystem::last_write_time(path,stamp); // Same-size, preserved-stamp edit.
    assert(!poll.due(path,4100));
    assert(poll.due(path,12100)); // Fallback refresh guarantees it is eventually read.
    assert(!poll.due(path,13100));
    {std::ofstream f(path,std::ios::app);f<<"Other=1\n";}
    std::filesystem::last_write_time(path,stamp);
    assert(poll.due(path,14100)); // Size changes work even if stamp is preserved.
    std::filesystem::last_write_time(path,stamp+std::chrono::seconds(2));
    assert(poll.due(path,15100));
    std::filesystem::remove(path);
    assert(poll.due(path,16100)); // Removal uses the existing defaults-reading path.
    assert(!poll.due(path,17100));
    assert(poll.due(path,100)); // A clock reset cannot stall refresh indefinitely.
    std::cout<<"PASS: settings creation, edits, unchanged polling, deletion, preserved timestamps and clock reset.\n";
}
