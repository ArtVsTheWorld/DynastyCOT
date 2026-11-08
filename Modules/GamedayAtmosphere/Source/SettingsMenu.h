#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

namespace attendance::settingsmenu {
inline constexpr int Category=0x444741,FirstKey=0x44470000;
enum class Kind {Toggle,Slider,Choice,Profile,AttendanceProfile};
inline constexpr const char* AttendanceProfileLabels[]={"Fuller Crowds","Default"};
inline constexpr const wchar_t* ProfileValues[]={L"Auto",L"Default",L"Reduced",L"BadWeather"};
inline constexpr const char* ProfileLabels[]={"Automatic","Normal","Patient Fans","Bad Weather"};
struct Option {const char* group;const wchar_t* section;const wchar_t* key;const char* label;const char* help;double factory,low,high;Kind kind;};
inline constexpr Option Options[]={
#include "SettingsMenuOptions.inc"
};
inline constexpr size_t Count=sizeof(Options)/sizeof(*Options);
inline const Option* find(int key){return key>=FirstKey&&static_cast<size_t>(key-FirstKey)<Count?&Options[key-FirstKey]:nullptr;}
inline bool owned(int key){return key>=FirstKey&&key<FirstKey+static_cast<int>(Count);}
inline double bounded(const Option& o,double value){return std::isfinite(value)?(std::clamp)(value,o.low,o.high):o.factory;}
struct Choices {
    std::vector<double> values;
    Choices(const Option& o,double current){
        current=bounded(o,current);
        double step=o.kind==Kind::Choice&&o.high<=8&&o.low>=0?.5:1;
        for(double n=o.low;n<=o.high+.00001&&values.size()<2000;n+=step)values.push_back(n);
        values.push_back(o.factory);values.push_back(current);
        std::sort(values.begin(),values.end());
        values.erase(std::unique(values.begin(),values.end(),[](double a,double b){return std::abs(a-b)<.00001;}),values.end());
    }
    int index(double value)const{for(size_t i=0;i<values.size();++i)if(std::abs(values[i]-value)<.00001)return static_cast<int>(i);return -1;}
};
// Implemented by both the native field writer and the test fixture.
template<class Writer>void list(Writer& out){
    int row=0;
    // Append new IDs without shifting existing controls; display each group once.
    for(size_t g=0;g<Count;++g){
        bool seen=false;for(size_t prior=0;prior<g;++prior)if(!std::strcmp(Options[prior].group,Options[g].group)){seen=true;break;}
        if(seen)continue;
        out.integer(row,"type",0);out.text(row,"display",Options[g].group);out.integer(row,"key",-1);
        out.boolean(row,"isEnabled",false);out.boolean(row,"isInteractive",false);out.boolean(row,"isSelectable",false);out.boolean(row,"isFocusable",false);++row;
        for(size_t i=0;i<Count;++i){const auto& o=Options[i];if(std::strcmp(o.group,Options[g].group))continue;
        out.integer(row,"type",o.kind==Kind::Slider?2:1);out.integer(row,"key",FirstKey+static_cast<int>(i));
        out.text(row,"display",o.label);out.text(row,"description",o.help);
        out.boolean(row,"isEnabled",true);out.boolean(row,"isInteractive",true);out.boolean(row,"isSelectable",true);out.boolean(row,"isFocusable",true);++row;
        }
    }
}
template<class Writer>void toggle(Writer& out,const Option& o,const Choices& choices,double current){
    for(size_t i=0;i<choices.values.size();++i){
        char label[48]{};double value=choices.values[i];
        if(o.kind==Kind::AttendanceProfile)std::snprintf(label,sizeof(label),"%s",AttendanceProfileLabels[static_cast<int>(value)]);
        else if(o.kind==Kind::Profile)std::snprintf(label,sizeof(label),"%s",ProfileLabels[static_cast<int>(value)]);
        else if(o.kind==Kind::Toggle)std::snprintf(label,sizeof(label),"%s",value==0?"Off":"On");
        else std::snprintf(label,sizeof(label),"%.4g",value);
        int row=static_cast<int>(i);out.text(row,"display",label);out.integer(row,"key",row);
        out.boolean(row,"isDefault",std::abs(current-value)<.00001);out.boolean(row,"isResetOption",std::abs(o.factory-value)<.00001);
        out.boolean(row,"isEnabled",true);
    }
}
template<class Writer>void slider(Writer& out,const Option& o,double current){
    out.integer("min",static_cast<int>(o.low));out.integer("max",static_cast<int>(o.high));out.integer("step",1);
    out.integer("defaultValue",static_cast<int>(std::lround(bounded(o,current))));out.integer("resetValue",static_cast<int>(o.factory));
    out.integer("key",FirstKey+static_cast<int>(&o-Options));
}
// The native switch handles every existing button before this fallback. Extend
// only its unknown-button handler: our ID passes through; other IDs still return
// its original zero fallback. No existing case, branch target or function size changes.
inline std::array<unsigned char,32> categoryFallback(){
    std::array<unsigned char,32> b{};
    b[0]=0xb9;b[1]=2;b[2]=0xb7;
    for(unsigned i=0;i<4;++i)b[3+i]=static_cast<unsigned char>(Category>>(8*i));
    b[7]=0x66;b[8]=0xb8; // strict equality; branch-if-false, aligned on blob offset 0x28938.
    b[12]=0x10; // branch end 0x2893c -> original return at 0x2894c.
    b[16]=0xb9;b[17]=2;b[18]=0x3e;return b;
}
}
