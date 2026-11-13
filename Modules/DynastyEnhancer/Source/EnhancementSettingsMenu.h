#pragma once
#include "EnhancementOptionsRuntime.h"
#include <algorithm>
#include <map>
#include <variant>
#include <vector>

namespace enhancements::menu {
constexpr uintptr_t tableRva=0xbab9a08,listSlot=0x38,writerSlot=0x50;
constexpr uintptr_t listRva=0x7998c00,writerRva=0x79a5c30;
constexpr int firstKey=0xDE100;
using List=void(__fastcall*)(void*,void*,void*);
using Writer=void(__fastcall*)(void*,void*,void*);
struct Field {unsigned row;std::string key;std::variant<bool,int,float,std::string> value;};
// Settings providers can write a header's label after its other fields, or
// revisit an earlier row. Buffer this one menu build before moving row indices.
// Stored strings own their bytes; native callback temporaries need not survive.
struct Collector {void** table;void* target;unsigned next=0;unsigned reserved=0;std::vector<Field> fields;};
template<class T>inline void put(void* target,unsigned row,const char* key,T value,unsigned slot){
    reinterpret_cast<void(__fastcall*)(void*,unsigned,const char*,T)>((*reinterpret_cast<void***>(target))[slot])(target,row,key,value);
}
template<class T>inline void forward(Collector* sink,unsigned row,const char* key,T value){sink->next=(std::max)(sink->next,row+1);sink->fields.push_back({row,key,value});}
inline void __fastcall reserve(Collector* sink,unsigned count){sink->reserved=(std::max)(sink->reserved,count);}
inline void __fastcall boolean(Collector* s,unsigned r,const char* k,bool v){forward(s,r,k,v);}
inline void __fastcall integer(Collector* s,unsigned r,const char* k,int v){forward(s,r,k,v);}
inline void __fastcall real(Collector* s,unsigned r,const char* k,float v){forward(s,r,k,v);}
inline void __fastcall text(Collector* s,unsigned r,const char* k,const char* v){forward(s,r,k,std::string(v?v:""));}
inline void* collectorTable[]={nullptr,reinterpret_cast<void*>(reserve),nullptr,reinterpret_cast<void*>(boolean),reinterpret_cast<void*>(integer),reinterpret_cast<void*>(real),reinterpret_cast<void*>(text)};
inline void append(void* out,unsigned row,Options selected,bool interactive){
    put(out,row,"label","Beyond the Field",6);put(out,row,"key",firstKey-1,4);
    put(out,row,"linkage","madden.ultimatecareer.component.ListCategoryHeader",6);++row;
    constexpr const char* names[]={"Social Media Feed","Recruiting Fog","Roster Fog","Team Fog","Measurables Display","Interest Display","Rating Colors","Dollar Values"};
    constexpr const char* descriptions[]={
        "Follow weekly Dynasty news, player stories and fan reactions.",
        "Choose how recruit ratings look. Off keeps ratings normal; Measurables and Interest Display still work. Blind hides ratings.",
        "Choose how your players' ratings look in Dynasty. Off shows normal ratings; Blind hides them.",
        "Choose how team Overall, Offense and Defense ratings look. Off shows normal numbers; Blind hides them.",
        "Show estimated workout results instead of physical ratings, even with Recruiting Fog off. Scouting reveals them; Blind hides measurements.",
        "Keep normal interest bars, use broad standings, or show ranks only. Recruiting Blind hides interest information.",
        "Add colors to ratings and grades. Off keeps the game's normal colors.",
        "Show shorter money amounts, such as $40K or $2M. This only changes labels, not your budget or offers."};
    auto domain=[](bool enabled,GradeScale scale){return !enabled?0:scale==GradeScale::PlusMinus?1:scale==GradeScale::Letters?2:int(scale)+1;};
    const int values[]={selected.social?1:0,domain(selected.recruiting,selected.recruitingScale),domain(selected.roster,selected.scale),domain(selected.team,selected.teamScale),selected.measurables?1:0,int(selected.interest),selected.colors?1:0,selected.dollars?1:0};
    constexpr unsigned keys[]={0,1,2,5,9,8,4,6};
    for(unsigned i=0;i<8;++i,++row){
        put(out,row,"linkage","madden.component.SettingsToggleHorizontal",6);put(out,row,"key",firstKey+int(keys[i]),4);
        put(out,row,"label",names[i],6);put(out,row,"description",descriptions[i],6);
        const bool scaleRow=i>=1&&i<=3,interestRow=i==5,measurementRow=i==4;
        const int count=scaleRow?6:interestRow?3:2;put(out,row,"numItems",count,4);put(out,row,"isDefault",values[i],4);put(out,row,"isInteractive",interactive&&(!(interestRow||measurementRow)||selected.recruiting),3);
        constexpr const char* scales[]={"Off","A+-","A","S","Descriptive","Blind"};
        for(int j=0;j<count;++j){const auto display="Display"+std::to_string(j),value="Value"+std::to_string(j);
            put(out,row,display.c_str(),scaleRow?scales[j]:interestRow?(j==0?"Default":j==1?"Recruit Feedback":"Rank Only"):measurementRow?(j?"Athletic Measurables":"Default"):j?"On":"Off",6);put(out,row,value.c_str(),j,4);}
    }
}
inline unsigned insertionRow(const Collector& sink){
    struct Header {std::string label,linkage;int key=-1;};
    std::map<unsigned,Header> headers;
    for(const auto& field:sink.fields){
        if(const auto* value=std::get_if<std::string>(&field.value)){
            if(field.key=="label")headers[field.row].label=*value;
            else if(field.key=="linkage")headers[field.row].linkage=*value;
        }else if(field.key=="key")if(const auto* number=std::get_if<int>(&field.value))headers[field.row].key=*number;
    }
    bool commissioner=false;
    for(const auto& [row,header]:headers){
        if(header.linkage!="madden.ultimatecareer.component.ListCategoryHeader")continue;
        // The native Commissioner header has key 0. Its text is an additional
        // anchor for providers that preserve the title but remap their keys.
        if(commissioner)return row;
        if(header.key==0||header.label=="Commissioner Settings"||header.label=="COMMISSIONER SETTINGS")commissioner=true;
    }
    return sink.next; // Never displace an unrecognized provider's section.
}
inline unsigned emit(const Collector& sink,Options selected,bool interactive){
    const unsigned at=insertionRow(sink),extra=sink.next?9:0;
    if(sink.reserved)reinterpret_cast<void(__fastcall*)(void*,unsigned)>((*reinterpret_cast<void***>(sink.target))[1])(sink.target,(std::max)(sink.reserved,sink.next)+extra);
    for(const auto& field:sink.fields){
        const auto row=field.row+(field.row>=at?extra:0);
        if(const auto* value=std::get_if<bool>(&field.value))put(sink.target,row,field.key.c_str(),*value,3);
        else if(const auto* number=std::get_if<int>(&field.value))put(sink.target,row,field.key.c_str(),*number,4);
        else if(const auto* realNumber=std::get_if<float>(&field.value))put(sink.target,row,field.key.c_str(),*realNumber,5);
        else put(sink.target,row,field.key.c_str(),std::get<std::string>(field.value).c_str(),6);
    }
    if(extra)append(sink.target,at,selected,interactive);
    return at;
}
// The native writer's bounds-checked switch ignores our private key range.
// Original rows are forwarded unchanged, including other mods' settings.
template<class Change>inline unsigned consume(void* source,Change change){
    if(!source)return 0;auto** table=*reinterpret_cast<void***>(source);
    const auto count=reinterpret_cast<int(__fastcall*)(void*)>(table[1])(source);if(count<0||count>1024)return 0;
    unsigned changed=0;
    for(int row=0;row<count;++row){const auto get=reinterpret_cast<int(__fastcall*)(void*,int,const char*)>(table[3]);
        const int key=get(source,row,"key");if(key<firstKey||key>=firstKey+10)continue;
        const int value=get(source,row,"value");if(change(unsigned(key-firstKey),value))++changed;
    }return changed;
}
}
