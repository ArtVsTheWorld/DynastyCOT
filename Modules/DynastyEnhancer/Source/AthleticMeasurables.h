#pragma once
#include "EnhancementOptions.h"
#include <algorithm>
#include <array>
#include <optional>
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
enum class Event : size_t { Dash, Split, Bench, Squat, Cone, Shuttle, Vertical, Count };
constexpr size_t eventCount=static_cast<size_t>(Event::Count);
enum Input : size_t { Speed, Acceleration, Strength, Direction, Agility, Jumping, InputCount };
inline constexpr std::array<const char*,InputCount> inputNames={
    "speed","acceleration","strength","change of direction","agility","jumping"
};
inline constexpr std::array<const char*,InputCount> nativeFields={
    "SpeedRating","AccelerationRating","StrengthRating","ChangeOfDirectionRating","AgilityRating","JumpingRating"
};
using Inputs=std::array<std::optional<double>,InputCount>;
struct EventSpec {
    const char* name;
    const char* seed;
    Input input;
    double low,high,minimum,maximum,uncertainty;
    int displayScale;
    bool fasterIsBetter;
};
inline constexpr std::array<EventSpec,eventCount> events={{
    {"40-Yard Dash","dash",Speed,6.50,4.21,4.15,6.60,.08,100,true},
    {"10-Yard Split","split",Acceleration,2.48,1.69,1.65,2.52,.04,100,true},
    {"Bench / Squat","lift",Strength,95,500,80,520,20,1,false},
    {"Bench / Squat","squat",Strength,135,700,110,735,35,1,false},
    {"3-Cone Drill","cone",Direction,9.50,6.40,6.30,9.60,.10,100,true},
    {"20-Yard Shuttle","shuttle",Agility,6.00,3.75,3.68,6.07,.07,100,true},
    {"Vertical Jump","vertical",Jumping,12,45,10.8,46.2,1.2,10,false},
}};
inline bool valid(double rating){return std::isfinite(rating)&&rating>=0&&rating<=100;}
inline Inputs revealedInputs(const std::map<std::string,double>& revealed){
    Inputs result;
    for(size_t i=0;i<result.size();++i){
        const auto it=revealed.find(inputNames[i]);
        if(it!=revealed.end()&&valid(it->second))result[i]=it->second;
    }
    return result;
}
inline std::optional<double> ratingFor(Event event,const Inputs& inputs){
    const auto& spec=events[static_cast<size_t>(event)];
    if(event==Event::Dash){
        if(!inputs[Speed]||!inputs[Acceleration])return {};
        return .6*(*inputs[Speed])+.4*(*inputs[Acceleration]);
    }
    return inputs[spec.input];
}
inline double modeled(Event event,double rating,double adjustment,double extra=0){
    const auto& spec=events[static_cast<size_t>(event)];
    return (std::clamp)(interpolate(rating,spec.low,spec.high)+adjustment+extra,spec.minimum,spec.maximum);
}
inline std::string display(Event event,double value){
    const auto& spec=events[static_cast<size_t>(event)];
    if(spec.displayScale==1)return std::to_string(static_cast<int>(std::round(value/5))*5);
    return fixed(value,spec.displayScale==10?1:2);
}
struct Estimate {std::string label,value;};
inline Estimate estimate(Event event,const Inputs& revealed,const std::string& id,bool fullyScouted=false){
    const auto& spec=events[static_cast<size_t>(event)];
    const auto rating=ratingFor(event,revealed);
    if(!rating)return {spec.name,fullyScouted?"Unavailable":"?"};
    const auto value=modeled(event,*rating,spec.uncertainty*noise(id,spec.seed));
    const auto first=display(event,value);
    if(event==Event::Bench){
        // Both lifts retain the original shared strength variation.
        const auto squat=modeled(Event::Squat,*rating,30*noise(id,"lift"),5*noise(id,"squat"));
        return {spec.name,first+" / "+display(Event::Squat,squat)+" lb"};
    }
    return {spec.name,first+(spec.fasterIsBetter?"s":" in")};
}
inline std::string seedIdentity(uint32_t league,const std::string& identity){
    return std::to_string(league)+"|"+identity;
}
inline std::optional<Event> eventFor(const std::string& attribute){
    for(Event event:{Event::Dash,Event::Split,Event::Bench,Event::Cone,Event::Shuttle,Event::Vertical}){
        if(attribute==inputNames[events[static_cast<size_t>(event)].input])return event;
    }
    return {};
}
inline Estimate estimate(const std::string& attribute,const std::map<std::string,double>& revealed,const std::string& identity,bool fullyScouted=false){
    const auto event=eventFor(attribute);
    return event?estimate(*event,revealedInputs(revealed),seedIdentity(enhancements::currentOptionsLeague.load(),identity),fullyScouted):Estimate{};
}
inline bool physical(const std::string& key){return key=="speed"||key=="acceleration"||key=="strength"||key=="agility"||key=="change of direction"||key=="jumping";}
inline std::string key(std::string s){for(auto& c:s)c=char(std::tolower(static_cast<unsigned char>(c)));return s;}
template<class View>std::string identity(View& view,const std::map<std::string,uintptr_t>& f){
 std::string out;for(auto* name:{"FirstName","LastName","Hometown","State"}){std::string v;if(f.count(name))view.string(f.at(name),v);out+=v+"|";}
 double id=0;if(f.count("RecordId")&&view.number(f.at("RecordId"),id)&&id>=0&&id<=UINT32_MAX)out+=std::to_string(uint64_t(id));return out;
}
}
