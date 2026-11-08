#pragma once
// Runs both inside the Analytics-only fixture and through the real Fog owner.
namespace analyticscardtests {
using Json=nlohmann::json;
Json output;unsigned allocations=0,decodes=0;bool allocationFails=false,factoryFails=false;
void* __fastcall factory(){return factoryFails?nullptr:reinterpret_cast<void*>(0x12345);}
void* __fastcall object(void* owner,void** out){if(owner!=reinterpret_cast<void*>(0x12345))throw std::runtime_error("Wrong native card allocator");++allocations;output=Json::object();*out=allocationFails?nullptr:&output;return out;}
void __fastcall boolean(void* target,const char* key,bool value){if(target!=&output)throw std::runtime_error("Wrong card target");output[key]=value;}
void __fastcall integer(void* target,const char* key,int value){if(target!=&output)throw std::runtime_error("Wrong card target");output[key]=value;}
void __fastcall text(void* target,const char* key,const char* value){if(target!=&output)throw std::runtime_error("Wrong card target");output[key]=value;}
unsigned __fastcall decode(void* buffer){++decodes;const auto value=*reinterpret_cast<char**>(static_cast<char*>(buffer)+8);return static_cast<unsigned>(std::stoul(value));}
template<class Check,class Invoke>void run(Check check,Invoke invoke){
    using namespace nativeanalytics;
    const auto oldData=data,oldPage=page;const auto oldCards=playerCards;const auto oldRows=rowLookup;
    const auto oldOpen=opened.load(),oldReady=nativeReady.load(),oldDisabled=disabled.load();
    const auto oldReturnTicket=cardReturnPending.load();const auto oldReturnReference=cardReturnReference;const auto oldRestore=cardSelectionRestorePending;
    const auto oldFactory=cardFactory;const auto oldObject=cardObject;const auto oldBool=cardBool;
    const auto oldInt=cardInt;const auto oldText=cardText;const auto oldDecode=decodeReference;
    cardFactory=factory;cardObject=object;cardBool=boolean;cardInt=integer;cardText=text;decodeReference=decode;
    opened=true;nativeReady=true;disabled=false;allocationFails=false;factoryFails=false;
    const unsigned first=(5u<<17),second=(5u<<17)|1;
    Json firstCard={{"firstName","Evan"},{"lastName","Moss"},{"portraitId",301},{"overall",80},{"height","6' 3\""},{"weight",240},{"className","JR"},{"position","TE"},{"teamLogoId",14},{"primaryTeamColor",123},{"secondaryTeamColor",456},{"isInjured",false},{"isEmpty",false}};
    Json secondCard={{"firstName","Jordan"},{"lastName","Wells"},{"portraitId",302},{"overall",71},{"position","CB"},{"teamLogoId",15},{"isInjured",true},{"isEmpty",false}};
    playerCards={{first,firstCard},{second,secondCard}};rowLookup={{first,0},{second,1}};
    std::string keyText=std::to_string(first);AliasArgument arg{aliasVtable,keyText.c_str()};void* target=nullptr;void* result=nullptr;
    check(invoke(&target,&arg,result)&&target==&output&&result==&target,"Known Analytics key did not use native card object ABI");
    check(output==firstCard,"Analytics card fields/types or portrait identity changed");
    output["staleTrait"]="wrong player";keyText=std::to_string(second);arg.text=keyText.c_str();
    check(invoke(&target,&arg,result)&&output==secondCard&&!output.contains("staleTrait"),"National selection retained another player's card fields");
    rowLookup={{second,0},{first,1}};keyText=std::to_string(first);arg.text=keyText.c_str();
    check(invoke(&target,&arg,result)&&output.at("firstName")=="Evan","Sort changed selected card identity");
    rowLookup.erase(first);auto allocationCount=allocations;
    check(!invoke(&target,&arg,result)&&allocations==allocationCount,"A player outside the selected scope reused an Analytics card");
    rowLookup[first]=1;playerCards.erase(first);check(!invoke(&target,&arg,result)&&allocations==allocationCount,"Missing card reused an unrelated player");playerCards[first]=firstCard;
    const auto oldDecodes=decodes;check(!invoke(&target,nullptr,result)&&decodes==oldDecodes,"Unreadable argument was decoded");
    AliasArgument empty{aliasVtable,""};check(!invoke(&target,&empty,result)&&decodes==oldDecodes,"Empty card key was decoded");
    check(!invoke(nullptr,&arg,result),"Null native output was accepted");
    opened=false;check(!invoke(&target,&arg,result),"Closed Analytics page intercepted native player card");opened=true;
    disabled=true;check(!invoke(&target,&arg,result),"Disabled Analytics intercepted native player card");disabled=false;
    nativeReady=false;check(!invoke(&target,&arg,result),"Unverified Analytics contract served a player card");nativeReady=true;
    allocationFails=true;target=nullptr;check(invoke(&target,&arg,result)&&!target&&result==&target,"Native allocation failure fell back to unrelated donor data");allocationFails=false;
    factoryFails=true;target=nullptr;check(invoke(&target,&arg,result)&&!target&&result==&target,"Missing native factory fell back to unrelated donor data");factoryFails=false;
    AliasArgument query{aliasVtable,"COT_RETURN_SELECTION"};const auto queryDecodes=decodes;
    output["stalePlayer"]="must not survive";opened=false;
    check(invoke(&target,&query,result)&&output==Json{{"cotReturnRow",-1}}&&decodes==queryDecodes,"Closed return query fell through or retained donor player data");opened=true;
    disabled=true;check(invoke(&target,&query,result)&&output==Json{{"cotReturnRow",-1}},"Disabled return query reached native key handling");disabled=false;
    nativeReady=false;target=&output;check(invoke(&target,&query,result)&&!target&&result==&target&&decodes==queryDecodes,"Unverified return query used native helpers or player-key fallback");nativeReady=true;
    check(invoke(nullptr,&query,result),"Null-output private return query fell through to native");
    std::string reservedText="COT_CARD:"+std::to_string(first);AliasArgument reservedArg{aliasVtable,reservedText.c_str()};
    auto rejectReserved=[&](){target=&output;result=&output;check(invoke(&target,&reservedArg,result)&&!target&&result==&target&&!cardReturnPending&&!cardReturnReference,"Rejected COT_CARD command reached native fallback or retained a stale ticket");};
    opened=false;rejectReserved();opened=true;
    disabled=true;rejectReserved();disabled=false;
    nativeReady=false;rejectReserved();nativeReady=true;
    check(invoke(nullptr,&reservedArg,result)&&result==nullptr,"Null-output COT_CARD command reached native fallback");
    // This ordinary test process has no live Dynasty; even an otherwise known
    // reference must fail closed through the real Fog-owned callback.
    check(!currentProfile(),"Card fixture unexpectedly acquired a live Dynasty");rejectReserved();
    for(const auto& value:std::vector<std::string>{"COT_CARD:","COT_CARD:unknown","COT_CARD:4294967295"}){reservedArg.text=value.c_str();rejectReserved();}
    data=oldData;page=oldPage;playerCards=oldCards;rowLookup=oldRows;opened=oldOpen;nativeReady=oldReady;disabled=oldDisabled;
    cardReturnPending=oldReturnTicket;cardReturnReference=oldReturnReference;cardSelectionRestorePending=oldRestore;
    cardFactory=oldFactory;cardObject=oldObject;cardBool=oldBool;cardInt=oldInt;cardText=oldText;decodeReference=oldDecode;
}
}
