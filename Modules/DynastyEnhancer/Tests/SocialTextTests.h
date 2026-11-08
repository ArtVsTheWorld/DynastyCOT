#pragma once
#include "../Source/FeedData.h"

namespace socialtexttests {
inline void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
inline void run(){
    const std::vector<std::pair<std::string,std::string>> cases={
        {u8"Ranking movement: 5 → 3","Ranking movement: 5 to 3"},
        {u8"No. 5→No. 3","No. 5 to No. 3"},
        {u8"José Fernández\t →  São José",u8"José Fernández to São José"},
        {u8"A→B→C","A to B to C"},
        {u8"First line\n5→3\nLast line","First line\n5 to 3\nLast line"},
        {u8"José, Łukasz, D’André, café — 31–26; e\u0301",u8"José, Łukasz, D’André, café — 31–26; e\u0301"}
    };
    for(const auto& item:cases){const auto repaired=social::socialDisplayText(item.first);check(repaired==item.second,"Native social text repair differs from the reviewed JS mapping");check(social::socialDisplayText(repaired)==repaired,"Native glyph repair is not idempotent");}
    // Decode creates a display projection. Neither archived input JSON nor
    // publication IDs, handles, numbers or view membership may be rewritten.
    using Json=nlohmann::json;
    const std::string raw=u8"Ranking movement for São José: 5 → 3. José Fernández — 31–26.";
    const Json post={{"id","published"},{"author",u8"José Fernández"},{"handle","@JoseFernandez"},{"body",raw},{"category","polls"},{"voice","official"},{"imageLibrary",79},{"imageValue",598},{"likes",25}};
    const Json edition={{"key","fixture/2026/RegularSeason/2"},{"season",2026},{"phase","RegularSeason"},{"week",2},{"stories",Json::array({post})},{"views",{{"all",Json::array({"published"})}}}};
    const Json archive={{"activeWeekKey","fixture/2026/RegularSeason/2"},{"weeks",Json::array({edition})}};
    const auto originalBytes=archive.dump();const auto display=social::decodeFeed(archive);
    check(archive.dump()==originalBytes,"Display-only repair modified the archived JSON");
    check(archive.at("weeks")[0].at("stories")[0].at("body")==raw,"Published original body changed");
    check(display->posts.size()==1&&display->posts[0].body==social::socialDisplayText(raw),"Archived arrow did not receive a display-only repair");
    check(display->posts[0].author==u8"José Fernández"&&display->posts[0].handle=="@JoseFernandez"&&display->posts[0].likes==25,"Repair changed author identity or numbers");
    check(display->posts[0].bodyHeight==social::postHeight(display->posts[0].body),"Layout was measured before display repair");
}
}
