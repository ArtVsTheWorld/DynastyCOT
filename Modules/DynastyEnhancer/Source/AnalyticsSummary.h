#pragma once
#include "AnalyticsCapture.h"
namespace analytics::live {
// Both the native menu and Social use the same compiled model. Only compact
// derived grades cross into the existing Social snapshot; no second JS model.
inline Json socialSummaryFromDataset(const Json& data){
    const auto& season=data.at("season");const int year=season.at("CurrentYear");
    Json out={{"format",1},{"modelVersion",ModelVersion},{"leagueId",data.at("leagueId")},{"year",year},{"season",season.at("CurrentSeasonYear")},{"weekly",Json::array()},{"seasonRows",Json::array()}};
    std::pair<int,int> latest{-1,-1};std::string latestType;
    for(const auto& r:data.at("rows"))if(r.at("year")==year&&releasedWeek(data,year,r.at("week"),r.at("weekType").get<std::string>())&&r.at("analytics").at("grade").is_number()){
        auto key=std::make_pair(stageOrder(r.at("weekType").get<std::string>()),r.at("week").get<int>());if(key>latest){latest=key;latestType=r.at("weekType");}
    }
    auto compact=[](const Json& r,const Json& a){return Json{{"player",r.at("playerKey")},{"team",r.at("teamIndex")},{"position",r.at("position")},{"grade",a.at("grade")},{"snaps",a.at("snaps")},{"opportunities",a.value("opportunities",0.)},{"group",a.at("group")},{"limitedModel",a.value("limitedModel",false)},{"basis","position-relative recorded production"}};};
    std::set<int> teams;for(const auto& t:data.at("teams"))if(!fcsTeam(t))teams.insert(t.at("index").get<int>());
    out["week"]=latest.second;out["weekType"]=latestType;
    for(const auto& r:data.at("rows"))if(r.at("year")==year&&r.at("week")==latest.second&&r.at("weekType")==latestType&&teams.count(r.at("teamIndex").get<int>())){
        const auto& a=r.at("analytics");if(a.at("grade").is_number())out["weekly"].push_back(compact(r,a));
    }
    for(const auto& r:data.at("seasonAnalytics"))if(r.at("year")==year&&r.at("grade").is_number()&&teams.count(r.at("teamIndex").get<int>())){auto v=compact(r,r);v["gradedGames"]=r.at("gradedGames");out["seasonRows"].push_back(std::move(v));}
    return out;
}
inline Json socialSummary(const Json& raw){return socialSummaryFromDataset(dataset(raw));}
}
