#pragma once
#include <string>
#include <string_view>

namespace social {
// Display-only equivalent of prose_cleanup.mjs. The native News font lacks
// U+2192. Leave every other UTF-8 byte intact, including accented names.
inline std::string socialDisplayText(std::string_view text){
    constexpr std::string_view arrow="\xe2\x86\x92";
    if(text.find(arrow)==std::string_view::npos)return std::string(text);
    std::string result;result.reserve(text.size());
    for(size_t i=0;i<text.size();){
        if(text.substr(i,arrow.size())==arrow){
            while(!result.empty()&&(result.back()==' '||result.back()=='\t'))result.pop_back();
            result+=" to ";i+=arrow.size();
            while(i<text.size()&&(text[i]==' '||text[i]=='\t'))++i;
        }else result.push_back(text[i++]);
    }
    return result;
}
}
