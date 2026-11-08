#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>
#include "../Vendor/json.hpp"

namespace analytics {
using Json=nlohmann::json;
inline constexpr const char* ModelVersion="0.1.0-research";
inline double stat(const Json& s,const char* name) {
    if(!s.contains(name)||!s.at(name).is_number())throw std::runtime_error(std::string("Missing stat: ")+name);
    const double v=s.at(name).get<double>();
    if(!std::isfinite(v))throw std::runtime_error("Non-finite stat");
    return v;
}
inline std::string group(const std::string& p) {
    if(p=="QB")return "QB";
    if(p=="HB"||p=="FB")return "RB";
    if(p=="WR")return "WR";
    if(p=="TE")return "TE";
    if(p=="LT"||p=="LG"||p=="C"||p=="RG"||p=="RT")return "OL";
    if(p=="LE"||p=="RE"||p=="DT"||p=="LEDG"||p=="REDG")return "DL";
    if(p=="LOLB"||p=="MLB"||p=="ROLB"||p=="WILL"||p=="MIKE"||p=="SAM")return "LB";
    if(p=="CB")return "CB";
    if(p=="FS"||p=="SS")return "S";
    if(p=="K"||p=="P")return p;
    return "Unsupported";
}
inline bool compatible(const std::string& g,const std::string& type) {
    if(g=="QB"||g=="RB"||g=="WR"||g=="TE")return type=="GameOffensiveStats"||type=="GameOffensiveKPReturnStats";
    if(g=="OL")return type=="GameOLineStats";
    if(g=="DL"||g=="LB"||g=="CB"||g=="S")return type=="GameDefensiveStats"||type=="GameDefensiveKPReturnStats";
    if(g=="K"||g=="P")return type=="GameKickingStats";
    return false;
}
// Authored production weights, not EA values or PFF's proprietary film grading.
inline double points(const Json& s,const std::string& g) {
    auto v=[&](const char* k){return stat(s,k);};
    if(g=="QB")return .10*v("PASSYARDS")+2*v("PASSCOMPLETED")-1.2*v("PASSATTEMPTS")+
        12*v("PASSTDS")-18*v("PASSINTS")-4*v("PASSSACKED")+.18*v("RUSHYARDS")+10*v("RUSHTDS")-12*v("RUSHFUMBLES");
    if(g=="RB")return .6*v("RUSHYARDS")-1.5*v("RUSHATTEMPTS")+12*v("RUSHTDS")+
        2*v("RUSHBROKENTACKLES")+.5*v("RECEIVEYARDS")+2*v("RECEIVECATCHES")+
        12*v("RECEIVETDS")-14*v("RUSHFUMBLES")-4*v("RECEIVEDROPS");
    if(g=="WR"||g=="TE")return .65*v("RECEIVEYARDS")+2*v("RECEIVECATCHES")+
        14*v("RECEIVETDS")-6*v("RECEIVEDROPS")+.5*v("RUSHYARDS")+12*v("RUSHTDS")-14*v("RUSHFUMBLES");
    if(g=="OL")return 5*v("OLINEPANCAKES")-16*v("OLINESACKSALLOWED");
    if(g=="DL"||g=="LB"||g=="CB"||g=="S")return
        (g=="DL"?2.:g=="LB"?2.4:1.8)*v("DEFTACKLES")+.8*v("ASSDEFTACKLES")+
        4*v("DEFTACKLESFORLOSS")+10*v("DLINESACKS")+5*v("DLINEHALFSACK")+
        7*v("DLINEFORCEDFUMBLES")+3*v("DLINEFUMBLERECOVERIES")+
        (g=="CB"||g=="S"?16.:12.)*v("DSECINTS")+
        (g=="CB"||g=="S"?5.:3.)*v("DEFPASSDEFLECTIONS")+
        10*(v("DSECINTTDS")+v("DLINEFUMBLETDS"))+6*v("DLINESAFETIES")+
        4*v("DLINEBLOCKS");
    if(g=="K") {
        double result=0;
        const char* attempts[]={"KICKFGATTEMPTS29ORLESS","KICKFGATTEMPTS30TO39","KICKFGATTEMPTS40TO49","KICKFGATTEMPTS50ORMORE"};
        const char* made[]={"KICKFGMADE29ORLESS","KICKFGMADE30TO39","KICKFGMADE40TO49","KICKFGMADE50ORMORE"};
        const double rewards[]={3,5,7,10},penalties[]={14,10,7,4};
        for(int i=0;i<4;++i){double a=v(attempts[i]),m=v(made[i]);if(m>a)throw std::runtime_error("Made exceeds attempts");result+=rewards[i]*m-penalties[i]*(a-m);}
        return result+v("KICKEPMADE")-8*(v("KICKEPATTEMPTS")-v("KICKEPMADE"));
    }
    if(g=="P")return .2*v("PUNTNETYARDS")-7*v("PUNTATTEMPTS")+3*v("PUNTIN20")-3*v("PUNTTOUCHBACKS")-10*v("PUNTBLOCKED");
    throw std::runtime_error("Unsupported position");
}
inline double noise(const std::string& key,double amplitude) {
    uint64_t h=14695981039346656037ull;
    for(unsigned char c:key){h^=c;h*=1099511628211ull;}
    return (static_cast<double>(h>>11)/9007199254740991.0*2.-1.)*amplitude;
}
inline const char* category(double g) {
    if(g>=90)return "Elite";
    if(g>=80)return "High-quality";
    if(g>=70)return "Good";
    if(g>=60)return "Average or above";
    return "Below average";
}
inline Json grade(const Json& row,const Json& config,const std::string& league) {
    Json out={{"grade",nullptr},{"status","Unavailable"},{"modelVersion",ModelVersion}};
    const auto g=group(row.at("position").get<std::string>());
    out["group"]=g;
    if(!compatible(g,row.at("statType").get<std::string>())){out["reason"]="No matching position model";return out;}
    const auto status=row.at("gameStatus").get<std::string>();
    if(status!="HomeWon"&&status!="AwayWon"&&status!="Tied"){out["reason"]="Game not finalized";return out;}
    try {
        const auto& s=row.at("stats");double snaps=stat(s,"DOWNSPLAYED");out["snaps"]=snaps;
        if(snaps<=0||snaps>1000){out["reason"]="No valid snaps";return out;}
        if(g=="K"&&stat(s,"KICKFGATTEMPTS")+stat(s,"KICKEPATTEMPTS")==0){out["reason"]="No kicking attempts";return out;}
        if(g=="P"&&stat(s,"PUNTATTEMPTS")==0){out["reason"]="No punts";return out;}
        const auto& b=config.at("baselines").at(g);
        double sd=b.at("sd").get<double>(),k=b.at("shrinkSnaps").get<double>();
        double amplitude=config.at("variation").get<double>();
        if(!std::isfinite(sd)||sd<=0||!std::isfinite(k)||k<0||!std::isfinite(amplitude)||amplitude<0||amplitude>5)
            throw std::runtime_error("Invalid model configuration");
        double rate=points(s,g)/snaps;
        double maxZ=config.at("maxStandardScore").get<double>();
        if(!std::isfinite(maxZ)||maxZ<=0||maxZ>10)throw std::runtime_error("Invalid standard score bound");
        double z=std::clamp((rate-b.at("mean").get<double>())/sd,-maxZ,maxZ);
        double reliability=snaps/(snaps+k);
        std::string seed=std::string(ModelVersion)+"|"+league+"|"+row.at("playerKey").get<std::string>()+"|"+
            row.at("year").dump()+"|"+row.at("weekType").get<std::string>()+"|"+row.at("week").dump()+"|"+row.at("gameKey").get<std::string>();
        double variation=noise(seed,amplitude);
        double raw=60+12*(reliability*z-b.at("centerCorrection").get<double>())+variation;
        if(!std::isfinite(raw))throw std::runtime_error("Invalid computed grade");
        double value=std::round(std::clamp(raw,0.,100.)*10)/10;
        out.update({{"grade",value},{"category",category(value)},{"status","Graded"},
          {"lowSample",snaps<(g=="K"||g=="P"?5:20)},{"productionRate",rate},
          {"reliability",reliability},{"variation",variation},{"limitedModel",g=="OL"||g=="CB"||g=="S"}});
    }catch(const std::exception& e){out["reason"]=e.what();}
    return out;
}
}
