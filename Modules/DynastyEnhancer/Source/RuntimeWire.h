#pragma once
#include "LiveTables.h"
#include <unordered_map>
namespace dynasty::live {
// Lossless generator transport. Schema/field names are shared, not repeated
// for each record. Capture, native analytics and proof masks stay unchanged.
inline Json generatorWire(const Json& raw){
    if(raw.at("format")!=1||raw.at("sourceKind")!="runtime"||!raw.at("records").is_object())throw std::runtime_error("Unsupported generator transport source");
    Json out=Json::object();for(const auto& [key,value]:raw.items())if(key!="records")out[key]=value;
    out["format"]=2;out["types"]=Json::array();out["rows"]=Json::array();
    std::unordered_map<std::string,size_t> types;
    for(const auto& [key,row]:raw.at("records").items()){
        const auto name=row.at("type").get<std::string>();const auto& values=row.at("v");
        if(!values.is_object()||row.at("key")!=key)throw std::runtime_error("Invalid generator transport row");
        std::string signature=name;signature+='\0';
        for(const auto& [field,value]:values.items()){(void)value;signature+=field;signature+='\0';}
        // Field shape is part of the dictionary identity: optional fields and
        // variable-length native arrays preserve exact presence and order.
        auto [found,fresh]=types.emplace(signature,types.size());
        if(fresh){Json fields=Json::array();for(const auto& [field,value]:values.items()){(void)value;fields.push_back(field);}out["types"].push_back(Json::array({name,std::move(fields)}));}
        Json cells=Json::array();for(const auto& value:values)cells.push_back(value);
        out["rows"].push_back(Json::array({key,found->second,row.value("capacity",Json()),std::move(cells)}));
    }
    return out;
}
}
