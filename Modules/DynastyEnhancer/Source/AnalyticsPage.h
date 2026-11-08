#pragma once
#include "AnalyticsModel.h"
#include <map>
#include <set>
#include <vector>
namespace analytics {
inline constexpr int NationalTeam=-1,SeasonWeek=-1;
inline constexpr unsigned NationalRowLimit=400;
// Availability follows captured season/stage progress, never game completion alone.
// Unknown stages fail closed rather than revealing unfinished weekly grades.
inline int stageOrder(const std::string& type){
    if(type=="PreSeason"||type=="Preseason")return 0;
    if(type=="RegularSeason")return 1;
    if(type=="PostSeason"||type=="Postseason"||type=="BowlSeason"||type=="Playoffs")return 2;
    if(type=="BowlSeason1")return 3;
    if(type=="BowlSeason2")return 4;
    if(type=="BowlSeason3")return 5;
    if(type=="NationalChampionship")return 6;
    if(type=="OffSeason"||type=="Offseason")return 7;
    return -1;
}
inline bool releasedWeek(const Json& data,int year,int week,const std::string& type){
    if(!data.contains("season"))return true; // Offline datasets without a clock retain their explicit inputs.
    const auto& now=data.at("season");const int currentYear=now.at("CurrentYear").get<int>();
    if(year!=currentYear)return year<currentYear;
    const int stage=stageOrder(type),currentStage=stageOrder(now.at("CurrentWeekType").get<std::string>());
    if(stage<0||currentStage<0)return false;
    return stage<currentStage||(stage==currentStage&&week<now.at("CurrentWeek").get<int>());
}
inline bool fcsTeam(const Json& team){return team.value("isFcs",team.value("name",std::string()).rfind("FCS ",0)==0);}
inline std::string classification(double grade){
    if(grade>=90)return "Elite";
    if(grade>=85)return "Excellent";
    if(grade>=80)return "Very Good";
    if(grade>=70)return "Good";
    if(grade>=60)return "Average";
    if(grade>=50)return "Below Average";
    return "Poor";
}
inline std::string displayPosition(const std::string& position){
    if(position=="LE")return "LEDG";
    if(position=="RE")return "REDG";
    if(position=="LOLB")return "SAM";
    if(position=="MLB")return "MIKE";
    if(position=="ROLB")return "WILL";
    return position;
}
inline int positionRank(const std::string& position){
    const auto label=displayPosition(position);
    constexpr const char* order[]={"QB","HB","FB","WR","TE","LT","LG","C","RG","RT","LEDG","REDG","DT","SAM","MIKE","WILL","CB","FS","SS","K","P"};
    // Higher football priority sorts first when descending: QB through P.
    for(int i=0;i<21;++i)if(label==order[i])return 21-i;
    return 0;
}
// Visible column indexes are shared with the native spreadsheet. Missing values stay last.
inline void sortPage(Json& page,int column,bool descending){
    auto& rows=page.at("rows");
    std::stable_sort(rows.begin(),rows.end(),[&](const Json& a,const Json& b){
        int comparison=0;
        if(column==3){
            auto ap=a.at("position").get<std::string>(),bp=b.at("position").get<std::string>();
            int x=positionRank(ap),y=positionRank(bp);
            if((x==0)!=(y==0))return x!=0;
            comparison=x<y?-1:x>y?1:x==0?ap.compare(bp):0;
        }else if(column==4||column==5||column==6){
            const char* field=column==4?"snaps":"grade";const auto& av=a.at(field);const auto& bv=b.at(field);
            if(av.is_number()!=bv.is_number())return av.is_number();
            if(av.is_number()){double x=av.get<double>(),y=bv.get<double>();comparison=x<y?-1:x>y?1:0;}
        }else{
            const char* field=column==3?"position":column==7?"teamName":"name";
            comparison=a.value(field,std::string()).compare(b.value(field,std::string()));
        }
        if(comparison)return descending?comparison>0:comparison<0;
        auto an=a.at("name").get<std::string>(),bn=b.at("name").get<std::string>();
        return an==bn?a.at("playerKey").get<std::string>()<b.at("playerKey").get<std::string>():an<bn;
    });
}
// The native UI is team-only. National is retained for bounded offline checks.
// Current native datasets cache grades of accumulated released production.
inline Json teamPage(const Json& data,int teamIndex,int year,int week,const std::string& weekType){
    const bool national=teamIndex==NationalTeam,season=week==SeasonWeek;
    Json result={{"title","Player Analytics"},{"teamIndex",teamIndex},{"year",year},{"week",week},
      {"weekType",weekType},{"rows",Json::array()},{"status",season?"Season":"Bye or no scheduled game"}};
    std::map<int,std::string> names;
    for(const auto& t:data.at("teams"))if(!fcsTeam(t)&&t.at("index")!=255)names[t.at("index").get<int>()]=t.at("name").get<std::string>();
    auto included=[&](int team){return names.count(team)&&(national||team==teamIndex);};
    const bool available=season||releasedWeek(data,year,week,weekType);
    if(!available)result["status"]="Available next week";
    bool scheduled=false,finalized=false;
    if(!season&&available)for(const auto& g:data.at("games")){
        if(g.at("SeasonYear")!=year||g.at("SeasonWeek")!=week||g.at("SeasonWeekType")!=weekType)continue;
        if(!included(g.value("homeTeamIndex",-2))&&!included(g.value("awayTeamIndex",-2)))continue;
        scheduled=true;auto status=g.at("GameStatus").get<std::string>();finalized|=status=="HomeWon"||status=="AwayWon"||status=="Tied";
    }
    if(scheduled)result["status"]=finalized?"Final":"Unplayed";
    std::map<std::string,Json> table;
    for(const auto& p:data.at("roster")){
        int team=p.at("teamIndex").get<int>();if(!included(team))continue;
        table[p.at("playerKey").get<std::string>()]={{"playerKey",p.at("playerKey")},{"name",p.at("name")},
          {"position",displayPosition(p.at("position").get<std::string>())},{"teamIndex",team},{"teamName",names.at(team)},
          {"snaps",nullptr},{"grade",nullptr},{"status","No recorded participation"}};
    }
    struct Average{double sum=0;int count=0,snaps=0;std::set<std::string> games,snapGames;};
    std::map<std::string,Average> averages;
    const bool cachedSeason=season&&data.value("analyticsModelVersion",std::string())==ModelVersion&&data.contains("seasonAnalytics")&&data.value("analyticsClock",Json())==data.value("season",Json());
    if(cachedSeason)for(const auto& row:data.at("seasonAnalytics"))if(row.at("year")==year&&included(row.at("teamIndex").get<int>()))table[row.at("playerKey").get<std::string>()]=row;
    for(const auto& row:data.at("rows")){
        if(cachedSeason)break;
        int team=row.at("teamIndex").get<int>();
        if(!included(team)||row.at("year")!=year)continue;
        if(!releasedWeek(data,row.at("year").get<int>(),row.at("week").get<int>(),row.at("weekType").get<std::string>()))continue;
        if(season){if(row.at("weekType")=="PreSeason"||row.at("weekType")=="Preseason")continue;}
        else if(row.at("week")!=week||row.at("weekType")!=weekType)continue;
        if(!compatible(group(row.at("position").get<std::string>()),row.at("statType").get<std::string>()))continue;
        auto key=row.at("playerKey").get<std::string>();
        Json view={{"playerKey",row.at("playerKey")},{"name",row.at("name")},{"position",displayPosition(row.at("position").get<std::string>())},
          {"teamIndex",team},{"teamName",names.at(team)},{"gameKey",row.at("gameKey")},
          {"snaps",row.at("stats").value("DOWNSPLAYED",0)},{"grade",nullptr},{"status","Not graded"}};
        if(row.contains("analytics"))view.update(row.at("analytics"));
        if(season){
            if(!compatible(group(row.at("position").get<std::string>()),row.at("statType").get<std::string>()))continue;
            auto status=row.at("gameStatus").get<std::string>();if(status!="HomeWon"&&status!="AwayWon"&&status!="Tied")continue;
            auto& average=averages[key];auto game=row.at("gameKey").get<std::string>();
            if(average.snapGames.insert(game).second)average.snaps+=(std::max)(0,view.at("snaps").get<int>());
            if(view.at("grade").is_number()&&average.games.insert(game).second){average.sum+=view.at("grade").get<double>();++average.count;}
            table[key]=std::move(view);continue;
        }
        if(table.count(key)&&table[key].contains("gameKey")){
            if(table[key].at("gameKey")==row.at("gameKey")){
                if(table[key].at("grade").is_number()||!view.at("grade").is_number())continue;
            }else{result["status"]="Multiple recorded games";key+="|"+row.at("gameKey").get<std::string>();}
        }
        table[key]=std::move(view);
    }
    for(auto& [key,row]:table){
        if(season&&averages.count(key)){const auto& a=averages.at(key);row["snaps"]=a.snaps;row["gradedGames"]=a.count;row["grade"]=a.count?Json(std::round(a.sum/a.count*10)/10):Json(nullptr);row["status"]=a.count?"Graded":"Unavailable";}
        row["classification"]=row.at("grade").is_number()?classification(row.at("grade").get<double>()):"N/A";
        result["rows"].push_back(std::move(row));
    }
    // Fix National eligibility by PFF grade before any user column sorting.
    // Every native page path calls this builder; team rosters remain complete.
    sortPage(result,5,true);
    if(national&&result["rows"].size()>NationalRowLimit){auto& rows=result["rows"];rows.erase(rows.begin()+NationalRowLimit,rows.end());}
    return result;
}
}
#include "AnalyticsDistribution.h"
