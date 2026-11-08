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
// Leader claims need stronger coverage than an individual spotlight. Reconcile
// native season participation against all finalized game caches (including a
// completed but still unreleased current week), then rank only released grades.
inline Json rankingCoverage(const Json& data,const Json& raw,const Json& summary){
    const int year=data.at("season").at("CurrentYear");const auto& records=raw.at("records");
    std::set<int> fbs,played;std::set<std::string> positions,bad;
    for(const auto& t:data.at("teams"))if(!fcsTeam(t)&&t.at("index")!=255)fbs.insert(t.at("index").get<int>());
    bool scheduleComplete=true;
    for(const auto& g:data.at("games"))if(g.at("SeasonYear")==year&&releasedWeek(data,year,g.at("SeasonWeek").get<int>(),g.at("SeasonWeekType").get<std::string>())&&std::make_pair(stageOrder(g.at("SeasonWeekType").get<std::string>()),g.at("SeasonWeek").get<int>())>std::make_pair(stageOrder(summary.at("weekType").get<std::string>()),summary.at("week").get<int>()))scheduleComplete=false;
    for(const auto& g:data.at("games"))if(g.at("SeasonYear")==year&&g.at("SeasonWeek")==summary.at("week")&&g.at("SeasonWeekType")==summary.at("weekType")){
        const auto status=g.at("GameStatus").get<std::string>();scheduleComplete&=status=="HomeWon"||status=="AwayWon"||status=="Tied";
        for(const char* side:{"homeTeamIndex","awayTeamIndex"})if(fbs.count(g.at(side).get<int>()))played.insert(g.at(side).get<int>());
    }
    std::map<std::string,double> snaps;std::set<std::pair<std::string,std::string>> seen;
    for(const auto& r:data.at("rows")){
        if(r.at("year")!=year||!compatible(group(r.at("position").get<std::string>()),r.at("statType").get<std::string>()))continue;
        const auto status=r.at("gameStatus").get<std::string>();if(status!="HomeWon"&&status!="AwayWon"&&status!="Tied")continue;
        const auto position=displayPosition(r.at("position").get<std::string>()),player=r.at("playerKey").get<std::string>();
        if(!seen.emplace(player,r.at("gameKey").get<std::string>()).second)continue;
        const auto& stats=r.at("stats");
        if(!stats.contains("DOWNSPLAYED")||!stats.at("DOWNSPLAYED").is_number()){bad.insert(position);continue;}
        const double downs=stats.at("DOWNSPLAYED");snaps[player]+=downs;
        const auto& a=r.at("analytics");const auto reason=a.value("reason",std::string());
        if(downs>0&&a.at("grade").is_null()&&reason!="No kicking attempts"&&reason!="No punts")bad.insert(position);
    }
    for(const auto& p:data.at("roster")){
        if(!fbs.count(p.at("teamIndex").get<int>()))continue;
        const auto position=displayPosition(p.at("position").get<std::string>());if(group(position)=="Unsupported")continue;positions.insert(position);
        const auto key=p.at("playerKey").get<std::string>();bool verified=false;
        const auto player=records.find(key);
        if(player!=records.end()){
            const auto& native=player->at("v");const auto ref=native.value("SeasonStats",Json());
            if(ref.is_string()&&records.contains(ref.get<std::string>()))for(const auto& member:records.at(ref.get<std::string>()).at("v")){
                if(!member.is_string()||!records.contains(member.get<std::string>()))continue;
                const auto& stat=records.at(member.get<std::string>());const auto type=stat.at("type").get<std::string>();const auto& value=stat.at("v");
                if(type.rfind("Season",0)!=0||!compatible(group(position),"Game"+type.substr(6))||value.value("SEAS_YEAR",-1)!=year)continue;
                verified=value.contains("DOWNSPLAYED")&&value.at("DOWNSPLAYED").is_number()&&value.at("DOWNSPLAYED").get<double>()==snaps[key]&&value.value("YEARBYYEARTEAMINDEX",p.at("teamIndex").get<int>())==p.at("teamIndex").get<int>();
                if(!verified)bad.insert(position);
            }
        }
        if(!verified&&snaps[key]>0)bad.insert(position);
    }
    Json complete=Json::array();for(const auto& p:positions)if(!bad.count(p))complete.push_back(p);
    return {{"format",1},{"source","native season participation and finalized game caches"},{"fbsTeams",fbs},{"weeklyTeams",played},{"trackedPositions",positions},{"positions",complete},{"scheduleComplete",scheduleComplete&&!played.empty()&&data.at("issues").empty()}};
}
inline Json socialSummary(const Json& raw){const auto data=dataset(raw);auto out=socialSummaryFromDataset(data);out["coverage"]=rankingCoverage(data,raw,out);return out;}
}
