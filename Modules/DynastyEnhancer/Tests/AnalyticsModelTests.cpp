#include "../Source/AnalyticsCapture.h"
#include <iostream>
#include <numeric>
using analytics::Json;
unsigned checks=0;
void check(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
Json quarterback(int week=0){
    Json stats={{"DOWNSPLAYED",120},{"PASSYARDS",300},{"PASSCOMPLETED",23},{"PASSATTEMPTS",35},{"PASSTDS",3},{"PASSINTS",1},{"PASSSACKED",2},{"RUSHYARDS",15},{"RUSHTDS",0},{"RUSHFUMBLES",0}};
    return {{"playerKey","5:0"},{"name","Longfirstname Longlastname-Suffix"},{"position","QB"},{"teamIndex",1},{"teamName","Long Program Name"},{"year",0},{"week",week},{"weekType","RegularSeason"},{"gameKey","8:"+std::to_string(week)},{"gameStatus","HomeWon"},{"statType","GameOffensiveStats"},{"stats",stats},{"prestigeStars",3.}};
}
double deviation(const std::vector<double>& a){double mean=std::accumulate(a.begin(),a.end(),0.)/a.size(),sum=0;for(double v:a)sum+=(v-mean)*(v-mean);return std::sqrt(sum/a.size());}
void singleGameChecks(const Json& cfg){
    for(const char* position:{"QB","HB","WR","TE","LT","DT","MLB","CB","FS","K","P"})for(int prestige:{1,5})for(int count:{1,30})for(int snaps:{12,80}){
        Json data={{"leagueId",77},{"season",{{"CurrentYear",0},{"CurrentWeek",1},{"CurrentWeekType","RegularSeason"}}},
            {"teams",Json::array({{{"index",1},{"name","Hawaii"}}})},{"roster",Json::array()},{"games",Json::array()},{"rows",Json::array()}};
        for(int i=0;i<count;++i){auto r=quarterback();r["position"]=position;r["prestigeStars"]=prestige;r["playerKey"]="5:"+std::to_string(i);r["teamName"]="Hawaii";
            auto& s=r["stats"];s["DOWNSPLAYED"]=snaps;
            for(const char* field:{"RUSHATTEMPTS","RUSHBROKENTACKLES","RECEIVEYARDS","RECEIVECATCHES","RECEIVETDS","RECEIVEDROPS","OLINESACKSALLOWED",
                "DEFTACKLES","ASSDEFTACKLES","DEFTACKLESFORLOSS","DLINESACKS","DLINEHALFSACK","DLINEFORCEDFUMBLES","DLINEFUMBLERECOVERIES","DSECINTS","DEFPASSDEFLECTIONS","DSECINTTDS","DLINEFUMBLETDS","DLINESAFETIES","DLINEBLOCKS","CTHALLOWED",
                "KICKFGATTEMPTS29ORLESS","KICKFGATTEMPTS30TO39","KICKFGATTEMPTS40TO49","KICKFGATTEMPTS50ORMORE","KICKFGMADE29ORLESS","KICKFGMADE30TO39","KICKFGMADE40TO49","KICKFGMADE50ORMORE","KICKEPATTEMPTS","KICKEPMADE","PUNTIN20","PUNTTOUCHBACKS","PUNTBLOCKED"})s[field]=0;
            s["PASSYARDS"]=120+i*10;s["RECEIVEYARDS"]=i*8;s["OLINESACKSALLOWED"]=i%4;s["DEFTACKLES"]=i%12;
            s["KICKFGATTEMPTS"]=s["KICKFGATTEMPTS30TO39"]=2;s["KICKFGMADE30TO39"]=i%3;
            s["PUNTATTEMPTS"]=3;s["PUNTNETYARDS"]=100+i;
            const auto g=analytics::group(position);r["statType"]=g=="OL"?"GameOLineStats":g=="K"||g=="P"?"GameKickingStats":g=="DL"||g=="LB"||g=="CB"||g=="S"?"GameDefensiveStats":"GameOffensiveStats";
            r["analytics"]=analytics::grade(r,cfg,"77");check(r["analytics"]["grade"].is_number(),"Single-game fixture not graded");data["rows"].push_back(std::move(r));
        }
        const auto nativeStats=data["rows"];analytics::calibrateDataset(data,cfg);
        for(size_t i=0;i<data["rows"].size();++i){
            const auto& weekly=data["rows"][i];const auto key=weekly["playerKey"];
            const auto it=std::find_if(data["seasonAnalytics"].begin(),data["seasonAnalytics"].end(),[&](const Json& r){return r["playerKey"]==key;});
            check(it!=data["seasonAnalytics"].end()&&(*it)["grade"]==weekly["analytics"]["grade"],"One-game season differs from final weekly grade");
            check((*it)["gradedGames"]==1&&(*it)["variation"]==weekly["analytics"]["variation"]&&(*it)["reliability"]==weekly["analytics"]["reliability"],"One-game evaluation metadata drifted");
            check(weekly["stats"]==nativeStats[i]["stats"],"Consistency fix changed tracked stats");
        }
        const auto stable=data;auto repeat=stable;repeat["rows"]=nativeStats;analytics::calibrateDataset(repeat,cfg);check(repeat==stable,"Consistency fix is not deterministic on fresh capture");
        if(count==1){
            const auto& weekly=data["rows"][0]["analytics"];
            auto page=analytics::teamPage(data,1,0,-1,"Season");check(page["rows"][0]["grade"]==weekly["grade"],"Cached Season page differs from Week 0");
            auto legacy=data;legacy.erase("seasonAggregationVersion");legacy["seasonAnalytics"][0]["grade"]=1.;
            page=analytics::teamPage(legacy,1,0,-1,"Season");check(page["rows"][0]["grade"]==weekly["grade"],"Legacy cached season bypassed consistency invalidation");
        }
    }
    Json data={{"leagueId",1},{"season",{{"CurrentYear",0},{"CurrentWeek",2},{"CurrentWeekType","RegularSeason"}}},{"teams",Json::array({{{"index",1},{"name","Hawaii"}}})},{"rows",Json::array()}};
    auto first=quarterback();first["analytics"]=analytics::grade(first,cfg,"1");data["rows"].push_back(first);
    data["rows"].push_back(first);auto future=quarterback(2);future["analytics"]=analytics::grade(future,cfg,"1");data["rows"].push_back(future);
    auto spring=quarterback();spring["weekType"]="PreSeason";spring["gameKey"]="spring";spring["analytics"]=analytics::grade(spring,cfg,"1");data["rows"].push_back(spring);
    analytics::calibrateDataset(data,cfg);check(data["seasonAnalytics"].size()==1&&data["seasonAnalytics"][0]["gradedGames"]==1&&data["seasonAnalytics"][0]["grade"]==data["rows"][0]["analytics"]["grade"],"Duplicate/current/spring rows altered single-game eligibility");
    data["rows"].erase(data["rows"].begin()+1);auto second=quarterback(1);second["stats"]["PASSYARDS"]=100;second["analytics"]=analytics::grade(second,cfg,"1");data["rows"].push_back(second);
    analytics::calibrateDataset(data,cfg);check(data["seasonAnalytics"][0]["gradedGames"]==2&&!data["seasonAnalytics"][0].contains("seasonBasis"),"Multi-game season still uses a single weekly grade");
    auto cumulative=first;cumulative["week"]=-1;cumulative["weekType"]="Season";cumulative["gameKey"]="derived-season";
    for(auto& item:cumulative["stats"].items())item.value()=first["stats"][item.key()].get<double>()+second["stats"][item.key()].get<double>();
    cumulative["analytics"]=analytics::grade(cumulative,cfg,"1");std::vector<Json*> pool{&cumulative};analytics::distribute(pool,"1",true);
    check(data["seasonAnalytics"][0]["grade"]==cumulative["analytics"]["grade"]&&data["seasonAnalytics"][0]["snaps"]==240,"Multi-game accumulated-production model changed");
}
int main(){try{
    const auto cfg=analytics::calibration();auto qb=quarterback();const auto original=qb;
    check(analytics::grade(qb,cfg,"1").at("grade").is_number(),"Recorded QB stats did not grade");
    auto missing=qb;missing["stats"].erase("PASSINTS");check(analytics::grade(missing,cfg,"1").at("grade").is_null(),"Missing input became an invented zero");
    auto season=qb;season["weekType"]="Season";season["stats"]["DOWNSPLAYED"]=1400;check(analytics::grade(season,cfg,"1").at("grade").is_number(),"Season participation inherited the per-game snap limit");
    auto game=season;game["weekType"]="RegularSeason";check(analytics::grade(game,cfg,"1").at("grade").is_null(),"Per-game snap bound was weakened");
    Json ol={{"DOWNSPLAYED",700},{"OLINESACKSALLOWED",2},{"OLINEPANCAKES",0}};const double clean=analytics::points(ol,"OL");ol["OLINEPANCAKES"]=999;check(analytics::points(ol,"OL")==clean,"Sparse pancakes control OL upside");
    ol["OLINESACKSALLOWED"]=9;check(analytics::points(ol,"OL")<clean,"More sacks allowed improved OL production");
    check(analytics::coverageAmplitude({{"prestigeStars",1}})==6&&analytics::coverageAmplitude({{"prestigeStars",5}})==2,"Prestige uncertainty endpoints changed");
    check(analytics::coverageAmplitude({})==4&&analytics::coverageAmplitude({{"prestigeStars",99}})==4,"Unknown prestige was guessed from strength");
    for(const char* group:{"QB","RB","WR","TE","OL","DL","LB","CB","S","K","P"}){
        Json rows=Json::array();for(int i=0;i<240;++i){auto r=qb;r["playerKey"]="5:"+std::to_string(i);r["analytics"]={{"group",group},{"grade",68.},{"snaps",800.},{"opportunities",70.},{"productionRate",double(i)}};rows.push_back(std::move(r));}
        const auto untouched=rows;std::vector<Json*> refs;for(auto& r:rows)refs.push_back(&r);analytics::distribute(refs,"1",true);
        std::vector<double> grades;unsigned good=0,high=0,low=0;for(const auto& r:rows){const double v=r["analytics"]["grade"];grades.push_back(v);good+=v>=70;high+=v>=80;low+=v<50;check(r["stats"]==original["stats"],"Distribution changed native stats");}
        check(deviation(grades)>8&&deviation(grades)<13&&good>70&&good<140&&high>15&&high<55&&low>3,"Position lacks a reasonable two-sided grade distribution");
        auto repeat=untouched;refs.clear();for(auto& r:repeat)refs.push_back(&r);analytics::distribute(refs,"1",true);check(repeat==rows,"Refresh is nondeterministic");
        for(auto& r:repeat){r["name"]="Different Name";r["overall"]=99;r["developmentTrait"]="Elite";r["analytics"]=untouched[0]["analytics"];r["analytics"]["productionRate"]=std::stoi(r["playerKey"].get<std::string>().substr(2));}
        refs.clear();for(auto& r:repeat)refs.push_back(&r);analytics::distribute(refs,"1",true);for(size_t i=0;i<rows.size();++i)check(repeat[i]["analytics"]["grade"]==rows[i]["analytics"]["grade"],"Name/rating/trait changed the grade");
    }
    std::vector<double> lowNoise,highNoise;
    for(int stars:{1,5}){Json rows=Json::array();for(int i=0;i<1000;++i){auto r=qb;r["playerKey"]="5:"+std::to_string(i);r["prestigeStars"]=stars;r["analytics"]={{"group","QB"},{"grade",68.},{"snaps",800.},{"productionRate",1.},{"opportunities",70.}};rows.push_back(std::move(r));}std::vector<Json*> refs;for(auto& r:rows)refs.push_back(&r);analytics::distribute(refs,"1",true);for(const auto& r:rows){check(r["analytics"]["productionPercentile"]==.5,"Tied stats received invented distinctions");(stars==1?lowNoise:highNoise).push_back(r["analytics"]["grade"]);}}
    check(deviation(lowNoise)>2.7*deviation(highNoise),"Small-program uncertainty does not have greater variance");
    check(std::abs(std::accumulate(lowNoise.begin(),lowNoise.end(),0.)/lowNoise.size()-68)<.6,"Coverage noise creates a prestige quality bias");
    Json data={{"leagueId",1},{"season",{{"CurrentYear",0},{"CurrentWeek",2},{"CurrentWeekType","RegularSeason"}}},{"teams",Json::array({{{"index",1},{"name","Test"}}})},{"rows",Json::array()}};
    for(int w:{0,1,2}){auto r=quarterback(w);r["analytics"]=analytics::grade(r,cfg,"1");data["rows"].push_back(std::move(r));}
    const auto source=data;analytics::calibrateDataset(data,cfg);check(data["seasonAnalytics"].size()==1&&data["seasonAnalytics"][0]["snaps"]==240&&data["seasonAnalytics"][0]["gradedGames"]==2,"Current-week stats leaked into season grades");
    for(size_t i=0;i<data["rows"].size();++i)check(data["rows"][i]["stats"]==source["rows"][i]["stats"],"Aggregation mutated source values");
    singleGameChecks(cfg);
    check(qb==original,"Model mutated its input");std::cout<<"PASS: "<<checks<<" model/distribution, tied/sparse, single-game equality, legacy cache, multi-game cumulative production, season availability, prestige variance and factual-input checks; no game access.\n";
}catch(const std::exception& e){std::cerr<<e.what();return 1;}return 0;}
