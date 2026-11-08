#pragma once
#include "LiveTables.h"
#include "AnalyticsPage.h"
#include "AnalyticsCalibration.h"
// A read-only adapter for the active native Dynasty. No manual save selection,
// helper process, ratings-based grade or native data write is involved.
namespace analytics::live {
inline bool roots(const std::string& name){return name=="SeasonInfo"||name=="Franchise"||name=="FranchiseServer_CareerFlow"||name=="FranchiseUser"||name=="Team"||name=="Player"||name=="SeasonGame";}
inline std::set<std::string> fields(const std::string& name){
    if(name=="SeasonInfo")return {"CurrentSeasonYear","CurrentYear","CurrentWeek","CurrentWeekType","CurrentStage","CurrentOffseasonStage"};
    if(name=="Franchise")return {"LeagueID","FCSTeams"};
    if(name=="FranchiseServer_CareerFlow")return {"IsStageActive","IsStageAdvancing"};
    if(name=="FranchiseUser")return {"AdminLevel","IsRetired","Team"};
    if(name=="Team")return {"TeamIndex","TeamPrestige","DisplayName","LongName","TEAM_PREFIX_NAME","ShortName","TEAM_LOGO","TEAM_BACKGROUNDCOLORR","TEAM_BACKGROUNDCOLORG","TEAM_BACKGROUNDCOLORB","TEAM_BACKGROUNDCOLORR2","TEAM_BACKGROUNDCOLORG2","TEAM_BACKGROUNDCOLORB2"};
    if(name=="Player")return {"FirstName","LastName","Position","TeamIndex","PLYR_PORTRAIT","OverallRating","JerseyNum","Height","Weight","SchoolYear","PLYR_HANDEDNESS","PLYR_HOME_STATE","PLYR_HOME_TOWN","InjuryStatus","RedshirtStatus","GameStats","PresentationId"};
    if(name=="SeasonGame")return {"IsPractice","SeasonYear","SeasonWeek","SeasonWeekType","GameStatus","HomeTeam","AwayTeam"};
    if(name.find("[]")!=std::string::npos||name.rfind("Game",0)==0)return {};
    return {"TeamIndex"}; // Do not traverse unrelated recruiting/history graphs.
}
inline Json dataset(const Json& raw){
    const auto& records=raw.at("records");const auto& status=raw.at("status");
    const auto league=status.at("league").get<uint32_t>();
    auto resolve=[&](const Json& ref,const char* type)->const Json*{
        if(!ref.is_string())return nullptr;auto found=records.find(ref.get_ref<const std::string&>());
        return found!=records.end()&&found->at("type")==type?&*found:nullptr;
    };
    auto values=[&](const Json& ref,const char* type)->const Json*{auto record=resolve(ref,type);return record?&record->at("v"):nullptr;};
    std::set<std::string> fcs;
    for(const auto& record:records)if(record.at("type")=="Franchise"&&record.at("v").contains("FCSTeams")){
        const auto& ref=record.at("v").at("FCSTeams");if(ref.is_string()&&records.contains(ref.get<std::string>()))for(const auto& member:records.at(ref.get<std::string>()).at("v"))if(member.is_string())fcs.insert(member.get<std::string>());
    }
    Json users=Json::array();int owner=-2;
    for(const auto& record:records)if(record.at("type")=="FranchiseUser"){
        const auto& v=record.at("v");if(v.value("IsRetired",true))continue;auto team=values(v.value("Team",Json()),"Team");if(!team)continue;
        int index=team->at("TeamIndex").get<int>();bool primary=v.value("AdminLevel",std::string())=="Owner";users.push_back({{"teamIndex",index},{"isOwner",primary}});if(owner==-2||primary)owner=index;
    }
    Json out={{"format",1},{"sourceKind","native-active-dynasty"},{"sourceUnchanged",true},{"leagueId",league},{"season",status.at("season")},{"userTeamIndex",owner},{"userTeams",users},{"teams",Json::array()},{"roster",Json::array()},{"games",Json::array()},{"rows",Json::array()},{"issues",Json::array()}};
    auto color=[](const Json& v,const char* r,const char* g,const char* b){return (v.value(r,0)<<16)|(v.value(g,0)<<8)|v.value(b,0);};
    auto teamName=[](const Json& v){for(const char* key:{"DisplayName","LongName","TEAM_PREFIX_NAME","ShortName"})if(v.contains(key)&&v.at(key).is_string()&&!v.at(key).get_ref<const std::string&>().empty())return v.at(key).get<std::string>();return std::string();};
    for(const auto& record:records)if(record.at("type")=="Team"&&record.at("capacity").get<int>()>1){
        const auto& v=record.at("v");const auto index=v.at("TeamIndex").get<int>();auto name=teamName(v);if(name.empty()||index==255)continue;
        Json nativeNames=Json::array();std::set<std::string> seenNames;
        for(const char* key:{"DisplayName","LongName","TEAM_PREFIX_NAME","ShortName"})if(v.contains(key)&&v.at(key).is_string()){
            auto alias=v.at(key).get<std::string>();if(!alias.empty()&&seenNames.insert(alias).second)nativeNames.push_back(std::move(alias));
        }
        out["teams"].push_back({{"key",record.at("key")},{"index",index},{"name",name},{"nativeNames",std::move(nativeNames)},{"isFcs",fcs.count(record.at("key").get<std::string>())!=0||name.rfind("FCS ",0)==0},{"isUserManaged",index==owner},{"logoId",v.value("TEAM_LOGO",0)},{"primaryColor",color(v,"TEAM_BACKGROUNDCOLORR","TEAM_BACKGROUNDCOLORG","TEAM_BACKGROUNDCOLORB")},{"secondaryColor",color(v,"TEAM_BACKGROUNDCOLORR2","TEAM_BACKGROUNDCOLORG2","TEAM_BACKGROUNDCOLORB2")}});
    }
    auto gameInfo=[&](const Json& record)->Json{
        const auto& v=record.at("v");if(v.value("IsPractice",true))return nullptr;auto home=values(v.value("HomeTeam",Json()),"Team"),away=values(v.value("AwayTeam",Json()),"Team");if(!home||!away)return nullptr;
        Json result=v;result["key"]=record.at("key");result["homeTeamIndex"]=home->at("TeamIndex");result["awayTeamIndex"]=away->at("TeamIndex");return result;
    };
    for(const auto& record:records)if(record.at("type")=="SeasonGame"){auto game=gameInfo(record);if(!game.is_null())out["games"].push_back(std::move(game));}
    std::map<int,double> prestige;for(const auto& record:records)if(record.at("type")=="Team"){const auto& t=record.at("v");if(t.value("TeamPrestige",0)>0&&t.value("TeamPrestige",0)<=10)prestige[t.at("TeamIndex").get<int>()]=t.at("TeamPrestige").get<double>()/2.;}
    const auto config=calibration();
    for(const auto& record:records)if(record.at("type")=="Player"&&record.at("capacity").get<int>()>1){
        const auto& p=record.at("v");const auto key=record.at("key").get<std::string>();auto name=p.value("FirstName",std::string())+" "+p.value("LastName",std::string());auto position=p.value("Position",std::string());int index=p.value("TeamIndex",255);
        if(index!=255){
            Json card={{"firstName",p.value("FirstName",std::string())},{"lastName",p.value("LastName",std::string())},{"position",position},{"portraitId",p.value("PLYR_PORTRAIT",0)},{"overall",p.value("OverallRating",0)},{"jerseyNumber",p.value("JerseyNum",0)},{"hometown",p.value("PLYR_HOME_STATE",std::string())},{"isInjured",p.value("InjuryStatus",std::string("Uninjured"))!="Uninjured"},{"redshirtStatus",p.value("RedshirtStatus",std::string())=="Previous"?1:0},{"isEmpty",false},{"showDealBreaker",false},{"showSeasonStats",false},{"showSkillGroups",false}};
            card["position"]=displayPosition(position);
            if(p.contains("PLYR_HOME_TOWN")&&p.at("PLYR_HOME_TOWN").is_string()&&!p.at("PLYR_HOME_TOWN").get_ref<const std::string&>().empty()){
                const auto state=p.value("PLYR_HOME_STATE",std::string());card["hometown"]=p.at("PLYR_HOME_TOWN").get<std::string>()+(state.empty()?"":", "+state);
            }
            const std::map<std::string,std::string> classes={{"Freshman","FR"},{"Sophomore","SO"},{"Junior","JR"},{"Senior","SR"}};auto year=p.value("SchoolYear",std::string());card["className"]=classes.count(year)?classes.at(year):year;
            if(p.contains("Height")){int height=p.at("Height").get<int>();card["height"]=std::to_string(height/12)+"' "+std::to_string(height%12)+"\"";}
            if(p.contains("Weight"))card["weight"]=p.at("Weight").get<int>()+160;
            if(p.contains("PLYR_HANDEDNESS"))card["handedness"]=p.at("PLYR_HANDEDNESS")=="Left"?"L":"R";
            out["roster"].push_back({{"playerKey",key},{"name",name},{"position",position},{"teamIndex",index},{"playerInfo",std::move(card)}});
        }
        auto stats=values(p.value("GameStats",Json()),"GameStats[]");if(!stats)continue;std::set<std::string> seen;
        for(const auto& ref:*stats){if(!ref.is_string()||!seen.insert(ref.get<std::string>()).second)continue;
            try{
                const auto& stat=records.at(ref.get<std::string>());auto type=stat.at("type").get<std::string>();if(type.rfind("Game",0)!=0||type.find("Stats")==std::string::npos)throw std::runtime_error("Unexpected game-stat record");const auto& s=stat.at("v");
                auto g=resolve(s.value("SeasonGame",Json()),"SeasonGame");if(!g)throw std::runtime_error("Missing game reference");auto game=gameInfo(*g);if(game.is_null())continue;
                auto home=values(g->at("v").at("HomeTeam"),"Team"),away=values(g->at("v").at("AwayTeam"),"Team");const Json* team=nullptr;const auto opponent=s.value("OpposingTeam",Json());
                if(opponent==g->at("v").at("HomeTeam"))team=away;else if(opponent==g->at("v").at("AwayTeam"))team=home;
                if(!team)throw std::runtime_error("Opponent is not part of game");
                // The grading input is deliberately isolated from player ratings,
                // GAMERATING, development traits and presentation metadata.
                Json measures=Json::object();for(const auto& [field,value]:s.items())if(value.is_number()&&field!="GAMERATING")measures[field]=value;
                Json row={{"playerKey",key},{"name",name},{"position",position},{"teamIndex",team->at("TeamIndex")},{"teamName",teamName(*team)},{"gameKey",g->at("key")},{"year",game.at("SeasonYear")},{"week",game.at("SeasonWeek")},{"weekType",game.at("SeasonWeekType")},{"gameStatus",game.at("GameStatus")},{"statType",type},{"statKey",stat.at("key")},{"stats",std::move(measures)}};
                if(prestige.count(team->at("TeamIndex").get<int>()))row["prestigeStars"]=prestige.at(team->at("TeamIndex").get<int>());
                row["analytics"]=grade(row,config,Json(league).dump());out["rows"].push_back(std::move(row));
            }catch(const std::exception& error){out["issues"].push_back({{"playerKey",key},{"error",error.what()}});}
        }
    }
    if(owner<0||owner==255)throw std::runtime_error("Active Dynasty has no eligible user team");calibrateDataset(out,config);return out;
}
}
