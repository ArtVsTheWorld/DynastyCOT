#pragma once
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <string>
#include <string_view>

namespace enhancements {
enum class GradeScale : unsigned {Letters, PlusMinus, STier, Descriptive, Blind};
enum class RecruitInterestDisplay:unsigned {Native,Feedback,RankOnly};
inline std::atomic<unsigned> currentRecruitInterest{0};
struct Options {
    bool social=true,recruiting=true,roster=true;
    GradeScale scale=GradeScale::PlusMinus;
    bool colors=false;
    bool team=true;
    bool dollars=false;
    GradeScale teamScale=GradeScale::PlusMinus;
    RecruitInterestDisplay interest=RecruitInterestDisplay::Native;
    GradeScale recruitingScale=GradeScale::PlusMinus;
    bool measurables=false;
    // Protocol v5 extends v4 in unused bits. Old saved profiles inherit the
    // player scale; both source-built payloads share the resolved bit policy.
    constexpr unsigned packed()const{return unsigned(social)|(unsigned(recruiting)<<1)|(unsigned(roster)<<2)|((unsigned(scale)&3)<<3)|(unsigned(colors)<<5)|(unsigned(team)<<6)|(unsigned(dollars)<<7)|((unsigned(scale)&4)<<6)|(unsigned(teamScale)<<9)|(unsigned(recruitingScale)<<12)|(1u<<15)|(unsigned(measurables)<<16);}
    static Options unpack(unsigned bits){Options v{bool(bits&1),bool(bits&2),bool(bits&4),GradeScale(((bits>>3)&3)|((bits>>6)&4)),bool(bits&32),bool(bits&64),bool(bits&128),GradeScale((bits>>9)&7)};v.recruitingScale=bits&(1u<<15)?GradeScale((bits>>12)&7):v.scale;v.measurables=bool(bits&(1u<<15))&&bool(bits&(1u<<16));return v;}
    static bool validBits(unsigned bits){return bits<131072&&unsigned(unpack(bits).scale)<=4&&unsigned(unpack(bits).teamScale)<=4&&unsigned(unpack(bits).recruitingScale)<=4&&((bits&(1u<<15))||bits<4096);}
};
inline constexpr unsigned defaultOptions=Options{}.packed();
inline std::atomic<unsigned> currentOptions{defaultOptions};
inline std::atomic<uint32_t> currentOptionsLeague{0};
using PresentationGuard=bool(*)();
inline std::atomic<PresentationGuard> presentationGuard{nullptr};
inline Options configuredOptions(){
    const auto bits=currentOptions.load(std::memory_order_relaxed),interest=currentRecruitInterest.load(std::memory_order_relaxed);
    const auto league=currentOptionsLeague.load(std::memory_order_relaxed);
    thread_local unsigned cachedBits=~0u,cachedInterest=~0u;thread_local uint32_t cachedLeague=0;thread_local Options cached;
    if(bits!=cachedBits||interest!=cachedInterest||league!=cachedLeague){cached=Options::unpack(bits);cached.interest=RecruitInterestDisplay(interest);cachedBits=bits;cachedInterest=interest;cachedLeague=league;}
    return cached;
}
inline bool presentationAllowed(){auto guard=presentationGuard.load(std::memory_order_acquire);return !guard||guard();}
// Keep the saved profile independent of its current presentation scope. Leaving
// Dynasty never changes or publishes the user's choices to another profile.
inline Options options(){auto selected=configuredOptions();if(presentationAllowed())return selected;auto native=Options::unpack(0);native.social=selected.social;native.measurables=false;return native;}
inline const char* gradeLabel(int rating,GradeScale scale){
    if(rating<0||rating>100)return "?";
    if(scale==GradeScale::Blind)return u8"–";
    if(scale==GradeScale::PlusMinus){
        if(rating<60)return "F";
        if(rating>=97)return "A+";
        constexpr const char* labels[]={"D-","D","D+","C-","C","C+","B-","B","B+","A-","A","A+"};
        const auto band=(rating-60)/10;const auto digit=rating%10;
        return labels[(band>3?3:band)*3+(digit<3?0:digit<7?1:2)];
    }
    const unsigned band=rating>=90?0:rating>=80?1:rating>=70?2:rating>=60?3:4;
    constexpr const char* letters[]={"A","B","C","D","F"};
    constexpr const char* words[]={"ELITE","GREAT","GOOD","FAIR","POOR"};
    if(scale==GradeScale::Descriptive)return words[band];
    return scale==GradeScale::STier&&rating>=95?"S":letters[band];
}
inline int gradeColor(std::string_view label){
    label=label.substr(0,label.find(','));
    if((label=="ELITE"||label=="Elite"))return 0xBB70EF;if((label=="GREAT"||label=="Great"))return 0x529FF0;if(label=="S")return 0x409EFF;
    if((label=="GOOD"||label=="Good"))return 0x66CF75;if((label=="FAIR"||label=="Fair"))return 0xEDB34F;if((label=="POOR"||label=="Poor"))return 0xEA6666;
    if(label.empty())return 0xF3F0E7;
    switch(label[0]){case 'A':return 0x1FE840;case 'B':return 0xA8D943;case 'C':return 0xFFD34D;case 'D':return 0xF68C2E;case 'F':return 0xF04444;default:return 0xF3F0E7;}
}
inline std::string ratingDisplay(int value){return gradeLabel(value,options().scale);}
inline std::string scopedRatingDisplay(int value,bool fog){return value<0||value>100?"?":fog?ratingDisplay(value):std::to_string(value);}
inline int ratingColor(int value){return gradeColor(gradeLabel(value,options().scale));}
inline bool playerColors(){return options().colors&&options().scale!=GradeScale::Blind;}
}
