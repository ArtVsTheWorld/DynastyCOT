#pragma once
#include "EnhancementOptions.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
namespace recruitingfog::measurables {
inline uint64_t hash(const std::string& s){uint64_t h=14695981039346656037ull;for(unsigned char c:s){h^=c;h*=1099511628211ull;}return h;}
inline double noise(const std::string& id,const char* measurement){auto h=hash(id+"|"+measurement);h^=h>>33;h*=0xff51afd7ed558ccdull;h^=h>>33;return double(h%1000001)/500000.-1.;}
inline double interpolate(double rating,double low,double high){return low+(high-low)*(std::clamp)(rating-40.,0.,59.)/59.;}
inline std::string fixed(double value,int places){std::ostringstream s;s<<std::fixed<<std::setprecision(places)<<value;return s.str();}
struct Estimate {std::string label,value;};
inline Estimate estimate(const std::string& attribute,const std::map<std::string,double>& revealed,const std::string& identity,bool fullyScouted=false){
 auto get=[&](const char* key){auto i=revealed.find(key);return i==revealed.end()?NAN:i->second;};
 auto valid=[](double n){return std::isfinite(n)&&n>=0&&n<=100;};
 auto id=std::to_string(enhancements::currentOptionsLeague.load())+"|"+identity;
 if(attribute=="speed"||attribute=="acceleration"){
  const auto s=get("speed"),a=get("acceleration");if(!valid(s)||!valid(a))return {"40-Yard Dash (est.)",fullyScouted?"Unavailable":"?"};
  return {"40-Yard Dash (est.)",fixed((std::clamp)(interpolate(.6*s+.4*a,6.50,4.21)+.08*noise(id,"dash"),4.15,6.60),2)+" s"};
 }
 auto i=revealed.find(attribute);if(i==revealed.end()||!valid(i->second))return {};
 const auto r=i->second;
 if(attribute=="strength"){
  const auto shared=noise(id,"lift"),bench=(std::clamp)(interpolate(r,95,500)+20*shared,80.,520.),squat=(std::clamp)(interpolate(r,135,700)+30*shared+5*noise(id,"squat"),110.,735.);
  return {"Bench / Squat (est.)",std::to_string(int(std::round(bench/5)*5))+" / "+std::to_string(int(std::round(squat/5)*5))+" lb"};
 }
 if(attribute=="change of direction")return {"3-Cone Drill (est.)",fixed((std::clamp)(interpolate(r,9.50,6.40)+.10*noise(id,"cone"),6.30,9.60),2)+" s"};
 if(attribute=="agility")return {"20-Yard Shuttle (est.)",fixed((std::clamp)(interpolate(r,6,3.75)+.07*noise(id,"shuttle"),3.68,6.07),2)+" s"};
 if(attribute=="jumping")return {"Vertical Jump (est.)",fixed((std::clamp)(interpolate(r,12,45)+1.2*noise(id,"vertical"),10.8,46.2),1)+" in"};
 return {};
}
inline bool physical(const std::string& key){return key=="speed"||key=="acceleration"||key=="strength"||key=="agility"||key=="change of direction"||key=="jumping";}
inline std::string key(std::string s){for(auto& c:s)c=char(std::tolower(static_cast<unsigned char>(c)));return s;}
template<class View>std::string identity(View& view,const std::map<std::string,uintptr_t>& f){
 std::string out;for(auto* name:{"FirstName","LastName","Hometown","State"}){std::string v;if(f.count(name))view.string(f.at(name),v);out+=v+"|";}
 double id=0;if(f.count("RecordId")&&view.number(f.at("RecordId"),id)&&id>=0&&id<=UINT32_MAX)out+=std::to_string(uint64_t(id));return out;
}
}
