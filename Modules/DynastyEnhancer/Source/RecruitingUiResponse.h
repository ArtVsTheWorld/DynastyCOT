#pragma once
#include "RecruitingGradePolicy.h"
#include "AthleticMeasurables.h"
#include <array>
#include <cstdint>
#include <cstring>
#include <map>
#include <set>
#include <vector>

namespace recruitingfog {
// Read is a guarded current-process read. The response is retained by the
// caller's native RefPtr for the entire callback. No database object is used.
template<class Read>class UiResponse {
    Read read;
    size_t textLimit;
public:
    explicit UiResponse(Read r,size_t maximumText=128):read(r),textLimit((std::min)(maximumText,size_t(4096))){}
    template<class T>bool get(uintptr_t at,T& out){return at>=0x10000&&read(at,&out,sizeof(out));}
    unsigned kind(uintptr_t at){unsigned out=0;get(at+0x10,out);return out;}
    bool text(uintptr_t at,std::string& out){
        std::array<unsigned char,16> bytes{};if(!read(at,bytes.data(),bytes.size()))return false;
        if(bytes[15]&128){uintptr_t data=0;unsigned size=0;memcpy(&data,bytes.data(),8);memcpy(&size,bytes.data()+8,4);if(size>textLimit)return false;out.resize(size);return !size||read(data,out.data(),size);}
        if(bytes[15]>15)return false;out.assign(reinterpret_cast<const char*>(bytes.data()),15-bytes[15]);return true;
    }
    bool array(uintptr_t at,std::vector<uintptr_t>& out,size_t maximum){
        uintptr_t begin=0,end=0;if(kind(at)!=4||!get(at+0x18,begin)||!get(at+0x20,end)||begin>end||(end-begin)%8||(end-begin)/8>maximum)return false;
        out.resize((end-begin)/8);return out.empty()||read(begin,out.data(),out.size()*8);
    }
    bool members(uintptr_t at,std::map<std::string,uintptr_t>& out){
        uintptr_t buckets=0;unsigned count=0;if(kind(at)!=5||!get(at+0x20,buckets)||!get(at+0x28,count)||!count||count>512)return false;
        std::vector<uintptr_t> heads(count);if(!read(buckets,heads.data(),heads.size()*8))return false;
        std::set<uintptr_t> visited;
        for(auto node:heads)while(node&&node!=uintptr_t(-1)){
            if(visited.size()>=256||!visited.insert(node).second)return false;
            std::array<uintptr_t,4> words{};if(!read(node,words.data(),sizeof(words)))return false;
            std::string name;if(!text(words[1],name)||name.empty()||!out.emplace(name,words[2]).second)return false;node=words[3];
        }
        return true;
    }
    bool number(uintptr_t at,double& out){return kind(at)==2&&get(at+0x18,out)&&std::isfinite(out);}
    bool string(uintptr_t at,std::string& out){uintptr_t value=0;return kind(at)==3&&get(at+0x20,value)&&text(value,out);}
    bool boolean(uintptr_t at,bool& out){unsigned char value=0;if(kind(at)!=1||!get(at+0x18,value)||value>1)return false;out=value!=0;return true;}
};
struct ConversionStats {unsigned recruits=0,attributes=0,revealed=0,ranges=0,unknown=0,unsupported=0;};

// Setters are the native UI-value factories. They replace only properties in
// ScoutingAttributes, never values in the native Player/Recruit tables.
struct NoSupplement {template<class View>void operator()(View&,const std::map<std::string,uintptr_t>&,std::map<std::string,double>&)const{}};
template<class View,class StringSetter,class BoolSetter,class Supplement=NoSupplement>
ConversionStats convertRecruitingResponse(View& view,uintptr_t root,StringSetter setString,BoolSetter setBool,Supplement supplement={}){
    ConversionStats stats;if(!enhancements::options().recruiting&&!enhancements::options().measurables)return stats;std::vector<uintptr_t> recruits;
    if(!view.array(root,recruits,512)){++stats.unsupported;return stats;}
    for(auto recruit:recruits){
        std::map<std::string,uintptr_t> record;double percent=0;
        if(!view.members(recruit,record)||!record.count("ScoutingAttributes")||!record.count("PercentageScouted")||!view.number(record.at("PercentageScouted"),percent)||percent<0||percent>100){++stats.unsupported;continue;}
        std::vector<uintptr_t> attributes;if(!view.array(record.at("ScoutingAttributes"),attributes,32)){++stats.unsupported;continue;}++stats.recruits;
        std::map<std::string,double> revealed;
        const auto selected=enhancements::options();const auto identity=measurables::identity(view,record);
        struct Attribute {uintptr_t at=0;std::map<std::string,uintptr_t> fields;std::string key;bool slider=false;double low=0,high=0;};
        std::vector<Attribute> rows;rows.reserve(attributes.size());
        // Gather the native reveal state once. In particular the dash must
        // see both inputs before either original value becomes display text.
        for(auto at:attributes){Attribute row;row.at=at;auto& fields=row.fields;
            if(!view.members(at,fields)||!fields.count("HaveSlider")||!fields.count("SliderStart")||!fields.count("SliderEnd")||!view.boolean(fields.at("HaveSlider"),row.slider)||!view.number(fields.at("SliderStart"),row.low)||!view.number(fields.at("SliderEnd"),row.high)){++stats.unsupported;continue;}
            std::string name;if(fields.count("Name"))view.string(fields.at("Name"),name);row.key=measurables::key(name);
            if(!row.slider&&(row.low>0||percent==100)&&row.low>=0&&row.low<=100&&measurables::physical(row.key))revealed[row.key]=row.low;
            rows.push_back(std::move(row));
        }
        if(percent==100&&selected.measurables&&(!revealed.count("speed")||!revealed.count("acceleration"))){std::map<std::string,double> extra;supplement(view,record,extra);for(const auto& value:extra){bool present=false;for(const auto& row:rows)present|=row.key==value.first;if(!present)revealed.insert(value);}}
        for(const auto& row:rows){const auto attribute=row.at;const auto slider=row.slider;const auto low=row.low,high=row.high;const auto& key=row.key;
            // This is the native incognito case. Do not turn its sentinel 0
            // into an F, and retain the original hidden-widget behavior.
            if(!slider&&low==0&&percent<100){++stats.unknown;continue;}
            if(selected.measurables&&measurables::physical(key)&&(!selected.recruiting||selected.recruitingScale!=enhancements::GradeScale::Blind)){
                auto e=measurables::estimate(key,revealed,identity,percent==100);
                const auto label=e.value.empty()?std::string("?"):e.value;
                if(!e.label.empty())setString(attribute,"Name",e.label.c_str());
                setString(attribute,"SliderStart",label.c_str());setString(attribute,"SliderEnd",label.c_str());setBool(attribute,"HaveSlider",false);++stats.attributes;continue;
            }
            if(!selected.recruiting)continue;
            const bool uninformative=slider&&low==0&&high==100&&percent<100;
            auto label=display(uninformative?Visibility::Unknown:slider?Visibility::Range:Visibility::Revealed,low,low,high);
            // The native model copies SliderStart directly into ExtraInfo.Points.
            // Both endpoints become the label and the numeric bar is disabled.
            setString(attribute,"SliderStart",label.c_str());setString(attribute,"SliderEnd",label.c_str());setBool(attribute,"HaveSlider",false);
            ++stats.attributes;if(label=="?")++stats.unknown;else if(slider)++stats.ranges;else ++stats.revealed;
        }
    }
    return stats;
}
}
