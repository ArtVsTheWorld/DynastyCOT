#include "../Vendor/json.hpp"
#include "../Source/FeedData.h"
#include <iostream>
#include <stdexcept>
using Json=nlohmann::json;
void check(bool yes,const char* message){if(!yes)throw std::runtime_error(message);}
int main(){try{
 Json post{{"id","legacy-analytics"},{"author","PFF"},{"handle","@PFF_College"},{"body","A recorded 91.3 grade across 40 snaps."},{"stamp","Analytics"},{"imageLibrary",88},{"imageValue",125},{"category","analytics"}};
 Json week{{"key","w1"},{"week",1},{"season",2027},{"stories",Json::array({post})},{"views",{{"for-you",Json::array({"legacy-analytics"})}}}};
 Json stored{{"activeWeekKey","w1"},{"scopeOptions",Json::array({{{"key","for-you"},{"label","For You"}}})},{"weeks",Json::array({week})}};
 const auto before=stored;auto feed=social::decodeFeed(stored);check(stored==before,"Archived JSON changed");
 const auto& projected=feed->catalog->weeks[0].stories.at("legacy-analytics");
 check(projected.author=="GPI"&&projected.handle=="@GPIndex","Old account header did not rebrand");
 check(projected.imageLibrary==88&&projected.imageValue==116,"Requested native G icon missing");
 check(projected.displayAuthor=="GPI (@GPIndex)"&&projected.body==post["body"],"Projection changed published wording or malformed header");
 social::FeedPost fresh{"GPI","@GPIndex","New publication","Analytics",88,116};
 check(fresh.displayAuthor==projected.displayAuthor,"Current and historical account presentations differ");
 for(auto wrong:{std::pair<std::string,std::string>{"PFF","@Unrelated"},{"Another Account","@PFF_College"}}){
  social::FeedPost other{wrong.first,wrong.second,"Body","Analytics",34,1096};
  check(other.author==wrong.first&&other.handle==wrong.second&&other.imageValue==1096,"Unrelated identity was overwritten");
 }
 std::cout<<"PASS: GPI native account/archived-header projection, exact logo, immutable history and unrelated account isolation.\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}return 0;}
