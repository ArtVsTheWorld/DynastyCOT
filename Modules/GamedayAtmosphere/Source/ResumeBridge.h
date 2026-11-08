#pragma once
#include <cstdint>
#include <cmath>
#include <map>
#include <string>
#include "AttendanceRules.h"

namespace attendance::resume {
// Deliberately small, bounded JSON reader. A half-written status file is ignored;
// strings, nesting and duplicate keys are handled rather than searched as text.
struct Json {
    enum Kind {Null,Number,String,Boolean,Object,Array} kind=Null;
    double number=0;std::string text;std::map<std::string,Json> fields;
    const Json& get(const std::string& key)const{static const Json empty;auto i=fields.find(key);return i==fields.end()?empty:i->second;}
    bool is(const char* s)const{return kind==String&&text==s;}
};
class Reader {
    const std::string& s;size_t p=0;unsigned nodes=0;
    void space(){while(p<s.size()&&(s[p]==' '||s[p]=='\r'||s[p]=='\n'||s[p]=='\t'))++p;}
    bool string(std::string& out){
        if(p>=s.size()||s[p++]!='"')return false;
        while(p<s.size()){
            unsigned char c=static_cast<unsigned char>(s[p++]);if(c=='"')return true;if(c<32)return false;
            if(c!='\\'){out+=static_cast<char>(c);continue;}
            if(p>=s.size())return false;char e=s[p++];
            if(e=='"'||e=='\\'||e=='/')out+=e;
            else if(e=='b')out+='\b';else if(e=='f')out+='\f';else if(e=='n')out+='\n';else if(e=='r')out+='\r';else if(e=='t')out+='\t';
            else if(e=='u'){
                for(int i=0;i<4;++i){if(p>=s.size()||!((s[p]>='0'&&s[p]<='9')||(s[p]>='a'&&s[p]<='f')||(s[p]>='A'&&s[p]<='F')))return false;++p;}
                out+='?'; // Unicode text is irrelevant to the ASCII status keys.
            }else return false;
        }return false;
    }
    bool value(Json& out,unsigned depth){
        if(depth>16||++nodes>4096)return false;space();if(p>=s.size())return false;
        char c=s[p];
        if(c=='{'){
            out.kind=Json::Object;++p;space();if(p<s.size()&&s[p]=='}'){++p;return true;}
            for(;;){std::string key;if(!string(key))return false;space();if(p>=s.size()||s[p++]!=':')return false;
                Json child;if(!value(child,depth+1)||!out.fields.emplace(key,std::move(child)).second)return false;
                space();if(p>=s.size())return false;char end=s[p++];if(end=='}')return true;if(end!=',')return false;space();}
        }
        if(c=='['){
            out.kind=Json::Array;++p;space();if(p<s.size()&&s[p]==']'){++p;return true;}
            for(;;){Json child;if(!value(child,depth+1))return false;space();if(p>=s.size())return false;char end=s[p++];if(end==']')return true;if(end!=',')return false;}
        }
        if(c=='"'){out.kind=Json::String;return string(out.text);}
        for(const auto& literal:{std::string("true"),std::string("false"),std::string("null")})if(s.compare(p,literal.size(),literal)==0){
            p+=literal.size();out.kind=literal=="null"?Json::Null:Json::Boolean;out.number=literal=="true"?1:0;return true;
        }
        size_t start=p;if(s[p]=='-')++p;if(p>=s.size())return false;
        if(s[p]=='0')++p;else {if(s[p]<'1'||s[p]>'9')return false;while(p<s.size()&&s[p]>='0'&&s[p]<='9')++p;}
        if(p<s.size()&&s[p]=='.'){++p;size_t begin=p;while(p<s.size()&&s[p]>='0'&&s[p]<='9')++p;if(begin==p)return false;}
        if(p<s.size()&&(s[p]=='e'||s[p]=='E')){++p;if(p<s.size()&&(s[p]=='+'||s[p]=='-'))++p;size_t begin=p;while(p<s.size()&&s[p]>='0'&&s[p]<='9')++p;if(begin==p)return false;}
        try{out.number=std::stod(s.substr(start,p-start));}catch(...){return false;}
        out.kind=Json::Number;return std::isfinite(out.number);
    }
public:
    explicit Reader(const std::string& source):s(source){}
    bool parse(Json& result){if(s.size()>65536||!value(result,0))return false;space();return p==s.size();}
};
inline bool integer(const Json& j,int& result,int low,int high){
    if(j.kind!=Json::Number||j.number<low||j.number>high||std::floor(j.number)!=j.number)return false;
    result=static_cast<int>(j.number);return true;
}
struct Signal {
    bool valid=false,applied=false;int game=0;uint64_t received=0;State live;
    bool matches(const State& s,uint64_t now)const{
        return valid&&now>=received&&now-received<=10000&&s.quarter==live.quarter&&s.homeScore==live.homeScore&&s.awayScore==live.awayScore&&
            std::abs(s.quarterSeconds-live.quarterSeconds)<1&&std::abs(s.seconds-live.seconds)<=20;
    }
};
inline Signal parse(const std::string& text,int pid,uint64_t now){
    Signal out;Json root;if(!Reader(text).parse(root)||!root.get("product").is("midgamesave")||!root.get("source").is("dll"))return out;
    int process=0;if(!integer(root.get("pid"),process,1,2147483647)||process!=pid)return out;
    const auto& game=root.get("game");const auto& live=game.get("live");
    if(game.get("active").kind!=Json::Boolean||game.get("active").number!=1||game.get("ended").kind!=Json::Boolean||game.get("ended").number!=0||!integer(game.get("number"),out.game,0,1000000))return out;
    if(!integer(live.get("quarter"),out.live.quarter,1,6)||!integer(live.get("home_score"),out.live.homeScore,0,255)||!integer(live.get("away_score"),out.live.awayScore,0,255)||
        live.get("clock").kind!=Json::Number||live.get("period").kind!=Json::Number)return out;
    out.live.seconds=static_cast<float>(live.get("clock").number);out.live.quarterSeconds=static_cast<float>(live.get("period").number);
    if(!attendance::valid(out.live))return out;
    const auto& arm=root.get("arm");int applied=0;
    out.applied=arm.get("state").is("applied")&&integer(arm.get("applied_game"),applied,0,1000000)&&applied==out.game;
    out.valid=true;out.received=now;return out;
}
struct Tracker {
    // The first active game after process launch can legitimately be number 0.
    // Keep initialization separate from the native generation value.
    int game=0;bool seen=false,applied=false;
    bool pending(const Signal& s,const State& live,uint64_t now,bool ordinaryGame=false)const{
        // Midgame Save announces an ordinary new game's number after pregame.
        // A verified owned ordinary session adopts that unarmed number, including
        // its first real snap after Super Sim. Explicit applied resumes still reset.
        return s.matches(live,now)&&((seen&&game!=s.game&&!(ordinaryGame&&!s.applied))||(s.applied&&(!applied||game!=s.game)));
    }
    bool observe(const Signal& s,const State& live,uint64_t now,bool ordinaryGame=false){
        if(!s.matches(live,now))return false;
        bool changed=pending(s,live,now,ordinaryGame);
        game=s.game;seen=true;applied=s.applied;return changed;
    }
};
}
