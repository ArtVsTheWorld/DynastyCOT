#pragma once
#include "AnalyticsPage.h"
#include <tuple>
namespace analytics {
// Position-relative production estimates. Equal signals keep equal percentiles;
// names, ratings and development traits never break statistical ties.
inline double normalScore(double p){
    p=std::clamp(p,.001,.999);double lo=-3.1,hi=3.1;
    for(unsigned i=0;i<24;++i){double mid=(lo+hi)*.5;if(.5*std::erfc(-mid/std::sqrt(2.))<p)lo=mid;else hi=mid;}
    return (lo+hi)*.5;
}
inline double coverageAmplitude(const Json& row){
    double stars=row.contains("prestigeStars")&&row.at("prestigeStars").is_number()?row.at("prestigeStars").get<double>():3.;
    if(!std::isfinite(stars)||stars<.5||stars>5)stars=3.;
    return 6.-(std::clamp(stars,1.,5.)-1.);
}
inline void distribute(std::vector<Json*>& rows,const std::string& league,bool season){
    std::map<std::string,std::vector<double>> pools;
    auto cohort=[&](const Json& r){return r.at("year").dump()+"|"+(season?"Season":r.at("weekType").get<std::string>()+"|"+r.at("week").dump())+"|"+r.at("analytics").at("group").get<std::string>();};
    for(auto* r:rows){const auto& a=r->at("analytics");const auto g=a.at("group").get<std::string>();
        if(a.at("grade").is_number()&&((g=="K"||g=="P")?a.value("opportunities",0.)>=(season?8:2):a.value("snaps",0.)>=(season?200:20)))pools[cohort(*r)].push_back(a.at("productionRate"));}
    for(auto& entry:pools)std::sort(entry.second.begin(),entry.second.end());
    for(auto* r:rows){auto& a=r->at("analytics");if(!a.at("grade").is_number())continue;
        const auto g=a.at("group").get<std::string>();auto found=pools.find(cohort(*r));
        const auto seed=std::string(ModelVersion)+"|"+league+"|"+r->at("playerKey").get<std::string>()+"|"+r->at("year").dump();
        const double amplitude=coverageAmplitude(*r),variation=season?noise(seed+"|coverage",amplitude):.75*noise(seed+"|coverage",amplitude)+.25*noise(seed+"|"+r->at("weekType").get<std::string>()+"|"+r->at("week").dump(),amplitude);
        const double sample=(g=="K"||g=="P")?a.at("opportunities").get<double>():a.at("snaps").get<double>();
        const double shrink=(g=="K"||g=="P")?(season?4.:1.):(season?40.:12.),reliability=sample/(sample+shrink);
        double base=68.;
        if(found!=pools.end()&&found->second.size()>=24){
            const auto& values=found->second;const double signal=a.at("productionRate");
            const auto lower=std::lower_bound(values.begin(),values.end(),signal),upper=std::upper_bound(values.begin(),values.end(),signal);
            const double percentile=(std::distance(values.begin(),lower)+.5*std::distance(lower,upper))/values.size();
            base+=11.*reliability*normalScore(percentile);a["cohortSamples"]=values.size();a["productionPercentile"]=percentile;a["tiedSignalCount"]=std::distance(lower,upper);
        }else{base=std::clamp(a.at("grade").get<double>(),45.,85.);a["cohortSamples"]=found==pools.end()?0:found->second.size();}
        a["grade"]=std::round(std::clamp(base+variation,0.,100.)*10)/10.;a["category"]=category(a.at("grade"));
        a["variation"]=variation;a["coverageAmplitude"]=amplitude;a["reliability"]=reliability;a["modelVersion"]=ModelVersion;a["basis"]="position-relative recorded production";
    }
}
inline void calibrateDataset(Json& data,const Json& config){
    const std::string league=data.at("leagueId").dump();std::vector<Json*> weekly;
    std::map<std::tuple<int,int,std::string>,Json> aggregates;std::set<std::pair<std::string,std::string>> seen;
    std::set<int> eligibleTeams;for(const auto& t:data.at("teams"))if(!fcsTeam(t)&&t.at("index")!=255)eligibleTeams.insert(t.at("index").get<int>());
    for(auto& r:data.at("rows")){
        if(!eligibleTeams.count(r.at("teamIndex").get<int>()))continue;
        if(!releasedWeek(data,r.at("year"),r.at("week"),r.at("weekType").get<std::string>()))continue;
        auto& a=r.at("analytics");const bool graded=a.at("grade").is_number();
        if(!graded&&a.value("reason",std::string())!="No kicking attempts"&&a.value("reason",std::string())!="No punts")continue;
        if(!seen.emplace(r.at("playerKey").get<std::string>(),r.at("gameKey").get<std::string>()).second){a["grade"]=nullptr;a["status"]="Duplicate recorded game";continue;}
        if(graded)weekly.push_back(&r);auto key=std::make_tuple(r.at("year").get<int>(),r.at("teamIndex").get<int>(),r.at("playerKey").get<std::string>());
        auto found=aggregates.find(key);
        if(found==aggregates.end()){Json sum=r;sum["stats"]=Json::object();sum["week"]=-1;sum["weekType"]="Season";sum["gameKey"]="derived-season";sum["gradedGames"]=0;found=aggregates.emplace(key,std::move(sum)).first;}
        auto& sum=found->second;sum["gradedGames"]=sum.at("gradedGames").get<int>()+(graded?1:0);
        for(const auto& [name,value]:r.at("stats").items())if(value.is_number()&&name!="GAMERATING")sum["stats"][name]=sum["stats"].value(name,0.)+value.get<double>();
    }
    distribute(weekly,league,false);std::vector<Json*> seasons;
    for(auto& entry:aggregates){auto& r=entry.second;r["analytics"]=grade(r,config,league);seasons.push_back(&r);}
    distribute(seasons,league,true);data["seasonAnalytics"]=Json::array();
    for(const auto& entry:aggregates){const auto& r=entry.second;Json v={{"playerKey",r.at("playerKey")},{"name",r.at("name")},{"position",displayPosition(r.at("position"))},{"teamIndex",r.at("teamIndex")},{"teamName",r.at("teamName")},{"year",r.at("year")},{"gradedGames",r.at("gradedGames")}};v.update(r.at("analytics"));v["classification"]=v.at("grade").is_number()?classification(v.at("grade")):"N/A";data["seasonAnalytics"].push_back(std::move(v));}
    data["analyticsModelVersion"]=ModelVersion;data["analyticsClock"]=data.value("season",Json());
}
}
