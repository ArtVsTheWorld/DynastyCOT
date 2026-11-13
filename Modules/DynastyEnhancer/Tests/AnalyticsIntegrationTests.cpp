#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <atomic>
#include <array>
#include <map>
#include <mutex>
#include <optional>
#include <tuple>
#include <iostream>
#include <chrono>
#include <fstream>
#include "../../../Compatibility/PracticeTextSetter.generated.h"
#include "../../../Build/Compatibility/PracticeContractFixtures.h"
#include "../Source/AnalyticsSummary.h"
#include "../Source/AnalyticsBridge.h"
using Json=nlohmann::json;
namespace fixture {
uintptr_t base=0;std::atomic<bool> disabled{false};std::atomic<unsigned> inflight{0};
struct CallGuard{CallGuard(){++inflight;}~CallGuard(){--inflight;}};
std::map<uintptr_t,uint64_t> sessionWords;
bool read(uintptr_t at,void* out,size_t n){auto fake=sessionWords.find(at);if(fake!=sessionWords.end()&&n<=8){memcpy(out,&fake->second,n);return true;}SIZE_T got=0;return at>=0x10000&&ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(at),out,n,&got)&&got==n;}
std::vector<Json> notes;
void note(Json value){notes.push_back(std::move(value));}
bool replace(void*,void*,uintptr_t,uintptr_t){throw std::runtime_error("Tests must not install hooks");}
#include "../Source/AnalyticsRuntime.h"
#include "AnalyticsCardChecks.h"
Json collected;int reserved=0,forwarded=0;std::vector<unsigned> encoded;
template<class T>void __fastcall accept(void*,unsigned row,const char* key,T value){collected[std::to_string(row)][key]=value;}
void __fastcall reserve(void*,int n){reserved=n;}
void __fastcall scalar(void*,const char* value){collected["key"]=value;}
int __fastcall colsCount(void*){return 6;}
int __fastcall colAt(void*,int n){return n+2;}
bool __fastcall sortFallback(void*,int,int column){forwarded=column;return true;}
int __fastcall countFallback(void*,int){++forwarded;return 123;}
void __fastcall encode(unsigned value,void* out,int*){encoded.push_back(value);strcpy_s(*reinterpret_cast<char**>(reinterpret_cast<char*>(out)+8),256,"native-shaped-test-key");}
int checks=0;
void check(bool result,const char* label){++checks;if(!result)throw std::runtime_error(label);}
std::array<uintptr_t,3> sourceColumnArgs{};int sourceColumnCalls=0;
void __fastcall sourceColumnFallback(void* object,void* output,int id){++sourceColumnCalls;sourceColumnArgs={reinterpret_cast<uintptr_t>(object),reinterpret_cast<uintptr_t>(output),static_cast<uintptr_t>(id)};}
void testSourceColumns(void* output){
    using namespace nativeanalytics;const auto previous=sourceColumns;sourceColumns=sourceColumnFallback;void* marker=reinterpret_cast<void*>(0x23456);
    opened=true;disabled=false;source=-1;collected=Json::object();sourceColumnHook(marker,output,0);
    check(source==-1&&collected.size()==8&&collected["5"]["display"]=="GPI GRADE"&&collected["6"]["display"]=="CLASSIFICATION"&&sourceColumnCalls==0,"Pre-BeginUpdate Roster schema did not expose Analytics columns");
    source=0;collected=Json::object();sourceColumnHook(marker,output,0);check(collected.size()==8&&collected["2"]["display"]=="Name"&&sourceColumnCalls==0,"Bound Roster schema did not expose Analytics columns");
    auto native=[&](int id){const int before=sourceColumnCalls;collected=Json::object();sourceColumnHook(marker,output,id);check(sourceColumnCalls==before+1&&collected.empty()&&sourceColumnArgs==std::array<uintptr_t,3>{0x23456,reinterpret_cast<uintptr_t>(output),static_cast<uintptr_t>(id)},"Unowned Roster schema did not forward exact native arguments once");};
    native(1);native(-1);opened=false;native(0);opened=true;disabled=true;native(0);disabled=false;sourceColumns=previous;
}
int sourceCloseCalls=0;std::pair<void*,int> sourceCloseArgs;
void __fastcall sourceCloseFallback(void* object,int id){++sourceCloseCalls;sourceCloseArgs={object,id};}
std::array<uintptr_t,3> sourceLifecycleArgs{};int sourceSyncCalls=0,sourceBeginCalls=0,sourceRowsAtForward=-1;
void observeSourceLifecycle(void* object,int id,int type){
    using namespace nativeanalytics;sourceLifecycleArgs={reinterpret_cast<uintptr_t>(object),static_cast<uintptr_t>(id),static_cast<uintptr_t>(type)};
    if(!disabled&&opened&&id>=0&&id<64&&type==27){check(owns(id),"Roster row source was not bound before the native update could reenter");sourceRowsAtForward=countHook(object,id);}
}
void __fastcall sourceSyncFallback(void* object,int id,int type){++sourceSyncCalls;observeSourceLifecycle(object,id,type);}
void __fastcall sourceBeginFallback(void* object,int id,int type){++sourceBeginCalls;observeSourceLifecycle(object,id,type);}
void testSourceLifecycle(){
    using namespace nativeanalytics;const auto previousSync=syncInit,previousBegin=beginUpdate;syncInit=sourceSyncFallback;beginUpdate=sourceBeginFallback;
    opened=true;disabled=false;source=-1;sourceTypes.fill(0);void* marker=reinterpret_cast<void*>(0x12345);
    beginHook(marker,0,27);check(source==0&&sourceTypes[0]==27&&sourceBeginCalls==1&&sourceRowsAtForward==static_cast<int>(page["rows"].size())&&sourceLifecycleArgs==std::array<uintptr_t,3>{0x12345,0,27},"BeginUpdate-only Roster lifecycle lost ownership, arguments or row data");
    source=-1;syncHook(marker,7,27);check(source==7&&sourceTypes[7]==27&&sourceSyncCalls==1&&sourceRowsAtForward==static_cast<int>(page["rows"].size())&&sourceLifecycleArgs==std::array<uintptr_t,3>{0x12345,7,27},"SyncInit-only Roster lifecycle lost ownership, arguments or row data");
    beginHook(marker,1,28);syncHook(marker,2,29);check(source==7&&sourceTypes[1]==28&&sourceTypes[2]==29,"Filter-choice sources stole row ownership");
    beginHook(marker,3,30);check(source==7,"Unrecognized row type stole Analytics ownership");
    const auto remembered=sourceTypes;beginHook(marker,64,27);syncHook(marker,-1,27);check(source==7&&sourceTypes==remembered,"Invalid source IDs altered binding state");
    opened=false;beginHook(marker,4,27);syncHook(marker,4,27);check(source==7&&sourceTypes==remembered,"Closed Analytics admitted native Roster updates");opened=true;
    disabled=true;beginHook(marker,5,27);syncHook(marker,5,27);check(source==7&&sourceTypes==remembered,"Disabled Analytics admitted native Roster updates");disabled=false;
    const auto priorClose=close;close=sourceCloseFallback;const auto visible=page;const auto visibleRows=rowLookup;const int priorTeam=selectedTeam,priorWeek=selectedWeek;
    closeHook(marker,7);check(sourceCloseCalls==1&&sourceCloseArgs==std::make_pair(marker,7)&&opened&&source==7&&page==visible&&rowLookup==visibleRows&&selectedTeam==priorTeam&&selectedWeek==priorWeek,"Transient native source close lost the active view, rows or exact forwarding");close=priorClose;
    check(sourceBeginCalls==6&&sourceSyncCalls==5,"A source lifecycle call was not forwarded exactly once");syncInit=previousSync;beginUpdate=previousBegin;sourceTypes.fill(0);
}
struct NativeFilterArray{void** vt;std::vector<std::string> keys;int advertisedCount=-2,sizeReads=0,keyReads=0;bool throwRead=false,unterminated=false;};
int __fastcall nativeFilterSize(NativeFilterArray* a){++a->sizeReads;return a->advertisedCount==-2?static_cast<int>(a->keys.size()):a->advertisedCount;}
void __fastcall nativeFilterRead(NativeFilterArray* a,int row,const char* field,char* out,int size){
    ++a->keyReads;check(std::string(field)=="key"&&size==256,"Filter getter ABI differs from native slot6");
    if(a->throwRead)throw std::runtime_error("Synthetic native getter failure");
    if(a->unterminated){memset(out,'x',size);return;}
    check(row>=0&&static_cast<size_t>(row)<a->keys.size(),"Filter getter row is outside native count");strncpy_s(out,size,a->keys[row].c_str(),_TRUNCATE);
}
void* nativeFilterVt[]={nullptr,reinterpret_cast<void*>(nativeFilterSize),nullptr,nullptr,nullptr,nullptr,reinterpret_cast<void*>(nativeFilterRead)};
void* forwardedFilter=nullptr;std::vector<std::string> nativeFilterKeys;int syncFilterCalls=0,asyncFilterCalls=0,refreshRows=-1;bool reenterFilters=false;
bool consumeNativeFilters(void* object,int id,void* input){
    forwardedFilter=input;nativeFilterKeys.clear();auto table=*reinterpret_cast<void***>(input);
    const int n=reinterpret_cast<int(__fastcall*)(void*)>(table[1])(input);
    for(int row=0;row<n;++row){char text[256]{};reinterpret_cast<void(__fastcall*)(void*,int,const char*,char*,int)>(table[6])(input,row,"key",text,256);nativeFilterKeys.emplace_back(text);check(nativeFilterKeys.back().rfind("ANALYTICS_",0)!=0,"Raw custom filter key reached the native provider");}
    if(reenterFilters)refreshRows=nativeanalytics::countHook(object,id);
    return true;
}
bool __fastcall syncFilterFallback(void* object,int id,void* input){++syncFilterCalls;return consumeNativeFilters(object,id,input);}
bool __fastcall asyncFilterFallback(void* object,int id,void* input){++asyncFilterCalls;return consumeNativeFilters(object,id,input);}
bool donorDefaults=true,donorAllPositions=true;
const std::string airForceKey="\x01\x06\x0b\x02\x03",akronKey="\x01\x06\x0b\x02\x05\x03",testTeamKey="\x01\x06\x0b\x02\x12\x03",opponentKey="\x01\x06\x0b\x02\x13\x03",nationalKey="\x01\x06\x0b\x02\x05\x12\x03",rosterPositionKey="\x01\x06\x0b\x02\x03";
std::vector<std::string> decodedFilterKeys;
std::vector<std::pair<std::string,std::string>> nativeTeamChoiceOverride;
unsigned __fastcall decodeNativeFilterKey(void* buffer){
    const std::string text=*reinterpret_cast<char**>(static_cast<char*>(buffer)+8);decodedFilterKeys.push_back(text);
    if(text==airForceKey)return 0;if(text==akronKey)return 1;if(text==testTeamKey)return 83;if(text==opponentKey)return 69;if(text==nationalKey)return 138;if(text=="foreign-team")return 999;if(text=="ou-native83")return 83;if(text=="nc-native69")return 69;if(text=="custom-native7"||text=="conflict-key"||text=="second-team-key")return 7;throw std::runtime_error("Unknown captured native filter key");
}
void __fastcall nativeFilterColumns(void*,void* output,int id,int){
    if(!donorDefaults)return;
    auto choice=[&](unsigned row,const std::string& label,const std::string& encodedKey){nativeanalytics::textField(output,row,"display",label);nativeanalytics::textField(output,row,"key",encodedKey);};
    if(id==8&&!nativeTeamChoiceOverride.empty()){unsigned row=0;for(const auto& entry:nativeTeamChoiceOverride)choice(row++,entry.first,entry.second);}
    else if(id==8){choice(0,"National",nationalKey);choice(1,"Opponent",opponentKey);choice(2,"Air Force",airForceKey);choice(3,"Akron",akronKey);choice(4,"Test University",testTeamKey);choice(5,"Unknown foreign program","foreign-team");choice(6,"Undecodable choice","bad-key");}
    else{choice(0,"QB",akronKey);if(donorAllPositions)choice(1,"Roster",rosterPositionKey);}
}
void testFilters(const Json& dataset){
    using namespace nativeanalytics;
    auto expanded=dataset;expanded["season"]["CurrentWeekType"]="OffSeason";expanded["season"]["CurrentWeek"]=0;
    expanded["teams"].push_back({{"index",0},{"name","Air Force"}});expanded["teams"].push_back({{"index",1},{"name","Akron"}});
    expanded["teams"].push_back({{"index",69},{"name","Oklahoma"},{"nativeNames",Json::array({"OU","Oklahoma"})}});
    expanded["teams"].push_back({{"index",90},{"name","North Carolina State"},{"nativeNames",Json::array({"NC State"})}});
    expanded["teams"].push_back({{"index",110},{"name","East Point"},{"nativeNames",Json::array({"East Point Green Storm","EPUFT"})}});
    expanded["roster"].push_back({{"playerKey","5:2"},{"name","Opponent Player"},{"position","HB"},{"teamIndex",15}});
    for(const auto& [weekNumber,stage,gradeValue]:std::vector<std::tuple<int,std::string,int>>{{0,"RegularSeason",71},{17,"BowlSeason1",89}}){
        auto game=expanded["games"][0];game["SeasonWeek"]=weekNumber;game["SeasonWeekType"]=stage;expanded["games"].push_back(game);
        auto performance=expanded["rows"][0];performance["week"]=weekNumber;performance["weekType"]=stage;performance["gameKey"]="8:"+std::to_string(weekNumber+10);performance["stats"]["DOWNSPLAYED"]=weekNumber?45:20;performance["analytics"]["grade"]=gradeValue;expanded["rows"].push_back(performance);
    }
    useSnapshot(prepareSnapshot(expanded));startOnUserTeam();opened=true;source=-1;sourceTypes.fill(0);sourceTypes[7]=27;
    filter=syncFilterFallback;asyncFilter=asyncFilterFallback;const auto priorDecoder=decodeReference;decodeReference=decodeNativeFilterKey;
    void* outputVt[]={nullptr,reinterpret_cast<void*>(reserve),nullptr,reinterpret_cast<void*>(accept<bool>),reinterpret_cast<void*>(accept<int>),nullptr,reinterpret_cast<void*>(accept<const char*>)};struct FilterOutput{void** vt;} output{outputVt};
    colData=nativeFilterColumns;sourceTypes[8]=28;sourceTypes[9]=29;donorDefaults=true;donorAllPositions=true;decodedFilterKeys.clear();
    colHook(nullptr,&output,8,0);colHook(nullptr,&output,9,0);
    check(nativeTeamKeys.size()==4&&nativeTeamKeys.at(0)==airForceKey&&nativeTeamKeys.at(1)==akronKey&&nativeTeamKeys.at(14)==testTeamKey&&nativeTeamKeys.at(15)==opponentKey&&!nativeTeamKeys.count(analytics::NationalTeam),"Native team keys were mapped by ordinal instead of exact captured team name or retained National");
    check(std::find(decodedFilterKeys.begin(),decodedFilterKeys.end(),nationalKey)==decodedFilterKeys.end()&&!nativeTeamKeys.count(138)&&!nativeTeamKeys.count(999),"National sentinel or foreign program became an ordinary eligible team");
    check(nativeWeekDefault==rosterPositionKey&&nativeWeekDefault!=akronKey,"Analytics week fallback did not choose native all-position Roster label");
    nativeTeamChoiceOverride={{"National",nationalKey},{"OU","ou-native83"},{"NC State","nc-native69"},{"East Point Green Storm","custom-native7"},{"not an exact name","foreign-team"}};
    colHook(nullptr,&output,8,0);
    check(nativeTeamKeys.size()==3&&nativeTeamKeys.at(69)=="ou-native83"&&nativeTeamKeys.at(90)=="nc-native69"&&nativeTeamKeys.at(110)=="custom-native7"&&!nativeTeamKeys.count(83),"Native ordinal83 was confused with TeamIndex69 or custom captured aliases were ignored");
    NativeFilterArray exactTeam{nativeFilterVt,{"ANALYTICS_TEAM:69"}};check(filterHook(nullptr,7,&exactTeam)&&selectedTeam==69&&nativeFilterKeys==std::vector<std::string>{"ou-native83"},"Selecting Oklahoma sent NC State's ordinal69 key to native Roster");
    exactTeam.keys={"ANALYTICS_TEAM:90"};check(asyncFilterHook(nullptr,7,&exactTeam)&&selectedTeam==90&&nativeFilterKeys==std::vector<std::string>{"nc-native69"},"Selecting NC State sent Oklahoma's ordinal83 key to native Roster");
    const auto unchangedTeams=data["teams"];data["teams"][0]["nativeNames"]=Json::array({"Shared"});data["teams"][1]["nativeNames"]=Json::array({"Shared"});
    nativeTeamChoiceOverride={{"Shared","test-key"},{"Unknown program","foreign-team"}};colHook(nullptr,&output,8,0);check(nativeTeamKeys.empty(),"Ambiguous/unknown native labels guessed an eligible team");
    data["teams"]=unchangedTeams;nativeTeamChoiceOverride={{"Test University","conflict-key"},{"Opponent","conflict-key"}};colHook(nullptr,&output,8,0);check(nativeTeamKeys.empty(),"Conflicting native key was assigned to multiple teams");
    nativeTeamChoiceOverride={{"Test University",testTeamKey},{"Test University","second-team-key"}};colHook(nullptr,&output,8,0);check(nativeTeamKeys.empty(),"Multiple native keys for one team silently chose a default");
    nativeTeamChoiceOverride.clear();colHook(nullptr,&output,8,0);useSnapshot(prepareSnapshot(expanded));startOnUserTeam();syncFilterCalls=asyncFilterCalls=0;
    const auto capturedTeams=nativeTeamKeys;sourceTypes[10]=30;colHook(nullptr,&output,10,0);check(nativeTeamKeys==capturedTeams&&nativeWeekDefault==rosterPositionKey,"Foreign column source replaced Roster mappings");
    donorDefaults=false;colHook(nullptr,&output,8,0);colHook(nullptr,&output,9,0);check(nativeTeamKeys.empty()&&nativeWeekDefault.empty(),"Missing donor keys retained stale native mappings");donorDefaults=true;colHook(nullptr,&output,8,0);colHook(nullptr,&output,9,0);
    auto weekKey=[&](int number,const std::string& stage){for(size_t i=0;i<weeks.size();++i)if(weeks[i].week==number&&weeks[i].type==stage)return "ANALYTICS_WEEK:"+std::to_string(i);throw std::runtime_error("Fixture week absent");};
    auto state=[&](){return Json{{"team",selectedTeam},{"name",teamName},{"year",seasonYear},{"week",selectedWeek},{"type",selectedType},{"page",page},{"index",rowLookup}};};
    auto checkRows=[&](){for(size_t i=0;i<page["rows"].size();++i){auto packed=static_cast<unsigned>(std::stoul(recordKey(page["rows"][i]["playerKey"].get<std::string>()),nullptr,16));check(rowForReference(packed)==static_cast<int>(i),"Filter refresh left a stale player-row identity");}check(countHook(nullptr,7)==static_cast<int>(page["rows"].size()),"Filter refresh count differs from visible page");};
    // Init can apply filters before source binding, but only a known type27
    // source belongs to this Analytics lifecycle.
    NativeFilterArray request{nativeFilterVt,{"ANALYTICS_TEAM:14",weekKey(1,"RegularSeason")}};
    check(filterHook(nullptr,7,&request)&&source==-1&&selectedWeek==1,"Initial native filter lifecycle was rejected");
    check(syncFilterCalls==1&&request.sizeReads==1&&request.keyReads==2&&nativeFilterKeys==std::vector<std::string>{testTeamKey,rosterPositionKey},"Sync filter did not translate the selected team/all positions through the native-shaped array");
    source=7;reenterFilters=true;checkRows();
    for(const auto& [teamIndex,number,stage]:std::vector<std::tuple<int,int,std::string>>{{14,0,"RegularSeason"},{0,-1,"Season"},{1,1,"RegularSeason"},{15,17,"BowlSeason1"},{14,-1,"Season"},{14,1,"RegularSeason"}}){
        request.keys={"ANALYTICS_TEAM:"+std::to_string(teamIndex),weekKey(number,stage)};
        const bool asynchronous=number!=0;const int oldSync=syncFilterCalls,oldAsync=asyncFilterCalls;
        check((asynchronous?asyncFilterHook(nullptr,7,&request):filterHook(nullptr,7,&request)),"Valid Analytics filter selection failed");
        check(selectedTeam==teamIndex&&selectedWeek==number&&selectedType==stage,"Filter selection applied the wrong team/week");
        check(nativeFilterKeys==std::vector<std::string>{nativeTeamKeys.at(teamIndex),rosterPositionKey},"Selected native team was replaced by another program or a position-only filter");
        check(syncFilterCalls==oldSync+(asynchronous?0:1)&&asyncFilterCalls==oldAsync+(asynchronous?1:0),"Filter forwarded to the wrong native entry");
        check(refreshRows==static_cast<int>(page["rows"].size()),"Native refresh did not see the completed page transaction");checkRows();
    }
    request.keys={"native-only"};auto before=state();check(filterHook(nullptr,7,&request)&&forwardedFilter==&request&&state()==before,"Native-only filters changed state or array identity");
    reenterFilters=false;check(asyncFilterHook(nullptr,8,&request)&&forwardedFilter==&request&&state()==before,"Unrelated source was intercepted");
    source=-1;sourceTypes[8]=28;check(filterHook(nullptr,8,&request)&&forwardedFilter==&request&&state()==before,"Filter-list source was mistaken for Analytics rows");source=7;
    opened=false;check(filterHook(nullptr,7,&request)&&forwardedFilter==&request,"Closed Analytics intercepted filters");opened=true;
    disabled=true;check(asyncFilterHook(nullptr,7,&request)&&forwardedFilter==&request,"Disabled Analytics intercepted filters");disabled=false;
    auto rejected=[&](bool asynchronous){const auto stable=state();const int calls=syncFilterCalls+asyncFilterCalls;check(!(asynchronous?asyncFilterHook(nullptr,7,&request):filterHook(nullptr,7,&request)),"Invalid custom filter was accepted");check(state()==stable&&syncFilterCalls+asyncFilterCalls==calls,"Rejected filter mutated the view or reached native code");};
    for(const auto& keyText:std::vector<std::string>{"ANALYTICS_WEEK:","ANALYTICS_WEEK:-1","ANALYTICS_WEEK:+1","ANALYTICS_WEEK: 1","ANALYTICS_WEEK:1x","ANALYTICS_WEEK:999999999999999999999","ANALYTICS_WEEK:999","ANALYTICS_TEAM:999","ANALYTICS_TEAM:-2","ANALYTICS_UNKNOWN:1"}){request.keys={keyText};rejected(false);rejected(true);}
    request.keys={"ANALYTICS_TEAM:14","ANALYTICS_WEEK:bad"};rejected(false); // Valid first key cannot partially commit.
    request.keys={"ANALYTICS_TEAM:14","ANALYTICS_TEAM:15"};rejected(true);
    request.keys={weekKey(0,"RegularSeason")};nativeWeekDefault.clear();rejected(false);nativeWeekDefault=rosterPositionKey;
    request.keys={"ANALYTICS_TEAM:-1"};nativeTeamKeys.erase(-1);rejected(true);nativeTeamKeys[-1]=nationalKey;
    request.keys={"ANALYTICS_TEAM:15"};nativeTeamKeys.erase(15);rejected(false);nativeTeamKeys[15]=opponentKey;
    donorAllPositions=false;colHook(nullptr,&output,9,0);check(nativeWeekDefault.empty(),"Missing all-position choice silently chose QB");request.keys={"ANALYTICS_TEAM:15"};rejected(true);request.keys={weekKey(0,"RegularSeason")};rejected(false);donorAllPositions=true;colHook(nullptr,&output,9,0);
    request.keys={weekKey(0,"RegularSeason")};request.advertisedCount=-1;rejected(false);request.advertisedCount=9;rejected(true);request.advertisedCount=-2;
    request.unterminated=true;rejected(false);request.unterminated=false;request.throwRead=true;rejected(true);request.throwRead=false;
    before=state();check(!filterHook(nullptr,7,nullptr)&&state()==before,"Null owned array was dereferenced");check(!asyncFilterHook(nullptr,7,reinterpret_cast<void*>(1))&&state()==before,"Unreadable owned array was dereferenced");
    auto intact=data;data["roster"][0].erase("name");request.keys={weekKey(0,"RegularSeason")};rejected(false);data=std::move(intact);
    // Every rejected attempt above is contained; private diagnostics are bounded.
    size_t errors=0;for(const auto& entry:notes)if(entry.value("event",std::string())=="native_analytics_filter_rejected")++errors;check(errors==16,"Filter rejection logs were not bounded to sixteen samples");
    decodeReference=priorDecoder;clearNativeFilters();useSnapshot(prepareSnapshot(dataset));startOnUserTeam();opened=true;source=7;sourceTypes.fill(0);reenterFilters=false;
}
void testCardReturnSelection(const Json& dataset){
    using namespace nativeanalytics;
    const auto oldFactory=cardFactory;const auto oldObject=cardObject;const auto oldBool=cardBool;const auto oldInt=cardInt;const auto oldText=cardText;const auto oldDecode=decodeReference;
    const auto oldLeague=snapshotLeague.load();const auto oldCareer=snapshotCareer.load(),oldDatabase=snapshotDatabase.load();const auto wasReady=snapshotReady.load(),wasNative=nativeReady.load();
    constexpr uintptr_t service=0x200000000,career=0x200010000,ui=0x200020000,database=0x200030000,owner=0x200040000;
    // Read-only synthetic service graph exercises the actual currentProfile()
    // checks. This neither installs hooks nor writes another process.
    sessionWords={{base+0xfa519b8,service},{service,base+0xbaa8998},{service+0x468,career},{service+0x470,ui},{service+0x460,owner},{service+0x7b1,1},{career,base+0xbce9b08},{ui,base+0xbd7a538},{career+0x20,owner},{career+0x238,database},{database,base+0xbd0d378},{base+0xf55c998,4294967295u}};
    snapshotLeague=4294967295u;snapshotCareer=career;snapshotDatabase=database;snapshotReady=true;nativeReady=true;check(currentProfile(),"Synthetic current-Dynasty binding did not pass the real ownership checks");
    cardFactory=analyticscardtests::factory;cardObject=analyticscardtests::object;cardBool=analyticscardtests::boolean;cardInt=analyticscardtests::integer;cardText=analyticscardtests::text;decodeReference=analyticscardtests::decode;
    analyticscardtests::allocationFails=false;analyticscardtests::factoryFails=false;
    useSnapshot(prepareSnapshot(dataset));startOnUserTeam();opened=true;source=7;clearCardReturn();sortColumn=2;sortDescending=false;hasUserSort=true;rebuildPage();
    const auto unchanged=data;const auto unchangedCards=playerCards;const unsigned first=5u<<17,second=first|1;
    auto viewState=[&](){return Json{{"team",selectedTeam},{"name",teamName},{"year",seasonYear},{"week",selectedWeek},{"type",selectedType},{"column",sortColumn},{"descending",sortDescending},{"userSort",hasUserSort},{"page",page}};};
    void* nativeOut=nullptr;void* nativeResult=nullptr;
    auto query=[&](){AliasArgument arg{aliasVtable,"COT_RETURN_SELECTION"};const auto previousDecodes=analyticscardtests::decodes;check(playerCard(&nativeOut,&arg,nativeResult),"Private return query fell through to native player-key handling");check(analyticscardtests::decodes==previousDecodes,"Private return query was decoded as a player reference");return nativeOut?analyticscardtests::output.at("cotReturnRow").get<int>():-2;};
    auto arm=[&](unsigned reference){clearCardReturn();std::string selected="COT_SELECT:"+std::to_string(reference);AliasArgument selectionArg{aliasVtable,selected.c_str()};check(playerCard(&nativeOut,&selectionArg,nativeResult)&&!nativeOut,"Settled card selection reached native fallback");std::string value="COT_CARD:"+std::to_string(reference);AliasArgument arg{aliasVtable,value.c_str()};check(playerCard(&nativeOut,&arg,nativeResult)&&nativeOut&&analyticscardtests::output.value("cotCardReady",false)&&cardReturnPending&&cardReturnReference==reference,"Validated card action did not allocate a ready response and arm the selected player identity");};
    auto rejectAction=[&](const std::string& value){AliasArgument arg{aliasVtable,value.c_str()};nativeOut=&analyticscardtests::output;nativeResult=nativeOut;check(playerCard(&nativeOut,&arg,nativeResult)&&!nativeOut&&nativeResult==&nativeOut&&!cardReturnPending&&!cardReturnReference&&!cardSelectionRestorePending,"Rejected private card action fell through or retained identity");};
    auto seedNativeFilters=[&](){nativeTeamKeys[14]=testTeamKey;nativeWeekDefault=rosterPositionKey;};
    auto resume=[&](){const auto before=viewState();seedNativeFilters();opened=false;check(resumeCard()&&opened&&!cardReturnPending&&cardSelectionRestorePending,"Card return ticket did not arm a one-shot restore");check(viewState()==before,"Card return changed the filter, ordering or page");check(nativeTeamKeys.empty()&&nativeWeekDefault.empty(),"Card return reused stale native Roster filter mappings");};
    auto leaveView=[&](){AliasArgument arg{aliasVtable,"COT_LEAVE_VIEW"};const auto beforeDecodes=analyticscardtests::decodes;const auto beforeOutput=analyticscardtests::output;nativeOut=&analyticscardtests::output;nativeResult=nativeOut;
        check(playerCard(&nativeOut,&arg,nativeResult)&&!nativeOut&&nativeResult==&nativeOut,"Explicit view exit fell through or returned native player data");
        check(!opened&&source==-1&&std::all_of(sourceTypes.begin(),sourceTypes.end(),[](int type){return type==0;})&&nativeTeamKeys.empty()&&nativeWeekDefault.empty(),"Explicit view exit retained active source/filter state");
        check(analyticscardtests::decodes==beforeDecodes&&analyticscardtests::output==beforeOutput,"Explicit view exit called player helpers or allocated an object");
    };
    arm(second);sourceTypes[7]=27;seedNativeFilters();const auto leavingView=viewState();leaveView();
    check(cardReturnPending&&cardReturnReference==second&&!cardSelectionRestorePending&&viewState()==leavingView,"Explicit view exit lost an armed card ticket or changed saved view");
    resume();check(query()==rowLookup.at(second),"Explicit view exit prevented exact-player card return");
    arm(first);resume();source=7;sourceTypes[7]=27;seedNativeFilters();leaveView();check(!cardReturnPending&&!cardReturnReference&&!cardSelectionRestorePending,"Non-card view exit retained a stale one-shot restore");
    opened=false;disabled=true;nativeReady=false;leaveView();check(!cardReturnPending&&!cardReturnReference,"Disabled/closed reserved exit retained stale identity");
    AliasArgument leaveArg{aliasVtable,"COT_LEAVE_VIEW"};check(playerCard(nullptr,&leaveArg,nativeResult)&&nativeResult==nullptr,"Null-output explicit view exit was forwarded to native");
    disabled=false;nativeReady=true;opened=true;source=7;
    check(query()==-1&&analyticscardtests::output==Json{{"cotReturnRow",-1}},"Absent return ticket did not create a fresh no-selection object");
    arm(second);check(query()==-1&&cardReturnPending&&cardReturnReference==second,"Initial refresh consumed a card ticket before resume");
    // Ordinary side-card fetches must not replace the explicit action identity.
    std::string plain=std::to_string(first);AliasArgument plainArg{aliasVtable,plain.c_str()};check(playerCard(&nativeOut,&plainArg,nativeResult)&&cardReturnReference==second&&!analyticscardtests::output.contains("cotCardReady"),"Side-card refresh replaced the selected return player or acquired a private readiness flag");
    const int selectedRow=rowLookup.at(second);resume();check(!resumeCard()&&cardSelectionRestorePending,"Duplicate resume erased the pending selection restore");check(query()==selectedRow,"Return restored another player's row");check(query()==-1&&!cardReturnReference&&!cardSelectionRestorePending,"Selection restore was not one-shot");
    arm(second);resume();source=7;const auto returningView=viewState();const auto returningRows=rowLookup;
    check(sortHook(nullptr,7,5)&&forwarded==2&&viewState()==returningView&&rowLookup==returningRows,"Automatic return sort changed the desired column, direction, page or player rows");
    check(sortHook(nullptr,7,2)&&forwarded==2&&viewState()==returningView&&rowLookup==returningRows,"Repeated native return sort toggled the saved direction");
    check(query()==returningRows.at(second)&&!cardSelectionRestorePending,"Native return sort changed the selected player's restored identity");
    const bool previousDirection=sortDescending;auto manuallySorted=page;analytics::sortPage(manuallySorted,2,!previousDirection);
    check(sortHook(nullptr,7,2)&&forwarded==2&&sortColumn==2&&hasUserSort&&sortDescending!=previousDirection&&page==manuallySorted,"Manual sorting did not resume after the restore query");
    for(size_t i=0;i<page["rows"].size();++i){auto packed=static_cast<unsigned>(std::stoul(recordKey(page["rows"][i]["playerKey"].get<std::string>()),nullptr,16));check(rowLookup.at(packed)==static_cast<int>(i),"Manual post-return sort retained stale row identity");}
    arm(first);sortDescending=true;rebuildPage();const int movedRow=rowLookup.at(first);resume();check(query()==movedRow,"Return restored the old row number instead of the selected player's current identity");
    arm(first);arm(second);resume();check(query()==rowLookup.at(second),"Latest validated card action did not own the return identity");
    arm(second);rowLookup.erase(second);resume();check(query()==-1,"Out-of-view selected player silently restored row zero");indexRows();
    arm(first);seedNativeFilters();sessionWords[base+0xf55c998]=17;check(!resumeCard()&&!cardReturnReference&&!cardSelectionRestorePending&&nativeTeamKeys.empty()&&nativeWeekDefault.empty(),"Different Dynasty reused a pending card ticket or native filter mapping");check(query()==-1,"Stale Dynasty return query exposed a row");sessionWords[base+0xf55c998]=4294967295u;check(!resumeCard(),"Returning to an old Dynasty revived a consumed ticket");
    arm(first);resume();sessionWords[career+0x238]=database+0x100;check(query()==-1&&!cardReturnReference,"Changed database identity reused a restore");sessionWords[career+0x238]=database;
    arm(first);resume();seedNativeFilters();leave();check(query()==-1&&!cardReturnReference&&!cardReturnPending&&nativeTeamKeys.empty()&&nativeWeekDefault.empty(),"Leaving Analytics retained a player restore or native filter mapping");opened=true;
    arm(first);resume();seedNativeFilters();check(open()&&query()==-1&&!cardReturnReference&&nativeTeamKeys.empty()&&nativeWeekDefault.empty(),"New Analytics entry retained a previous identity or native filter mapping");
    arm(first);seedNativeFilters();snapshotReady=false;check(!open()&&!cardReturnReference&&!cardReturnPending&&nativeTeamKeys.empty()&&nativeWeekDefault.empty(),"Failed Analytics entry retained stale identity or native filter mapping");snapshotReady=true;opened=true;
    arm(first);resume();analyticscardtests::factoryFails=true;check(query()==-2&&nativeResult==&nativeOut&&!cardSelectionRestorePending,"Factory failure leaked a return query or left a stale restore");analyticscardtests::factoryFails=false;check(query()==-1,"Factory recovery replayed a consumed selection");
    arm(first);resume();analyticscardtests::allocationFails=true;check(query()==-2&&nativeResult==&nativeOut&&!cardSelectionRestorePending,"Allocation failure fell through or retained a restore");analyticscardtests::allocationFails=false;check(query()==-1,"Allocation recovery replayed a consumed selection");
    analyticscardtests::factoryFails=true;rejectAction("COT_CARD:"+std::to_string(second));analyticscardtests::factoryFails=false;check(!resumeCard()&&query()==-1,"Factory failure armed a ready card navigation/return ticket");
    analyticscardtests::allocationFails=true;rejectAction("COT_CARD:"+std::to_string(second));check(!analyticscardtests::output.contains("cotCardReady"),"Allocation failure created a ready command response");analyticscardtests::allocationFails=false;check(!resumeCard()&&query()==-1,"Allocation failure armed a card navigation/return ticket");
    for(const auto& value:std::vector<std::string>{"COT_CARD:","COT_CARD:unknown","COT_CARD:999999999999999999999999","COT_CARD:4294967295"})rejectAction(value);
    const std::string validAction="COT_CARD:"+std::to_string(first);
    rowLookup.erase(first);rejectAction(validAction);indexRows();
    const auto savedCard=playerCards.at(first);playerCards.erase(first);rejectAction(validAction);playerCards[first]=savedCard;
    const auto verifiedDecoder=decodeReference;decodeReference=nullptr;rejectAction(validAction);decodeReference=verifiedDecoder;
    opened=false;rejectAction(validAction);opened=true;disabled=true;rejectAction(validAction);disabled=false;nativeReady=false;rejectAction(validAction);nativeReady=true;
    arm(second);sessionWords[base+0xf55c998]=9;rejectAction(validAction);sessionWords[base+0xf55c998]=4294967295u;
    arm(first);resume();AliasArgument queryArg{aliasVtable,"COT_RETURN_SELECTION"};check(playerCard(nullptr,&queryArg,nativeResult)&&!cardSelectionRestorePending,"Null query output fell through or kept a stale restore");check(query()==-1,"Null-output query was not consumed");
    opened=false;check(query()==-1,"Closed Analytics return query was not safely handled");opened=true;
    check(data==unchanged&&playerCards==unchangedCards,"Selection return mutated captured Dynasty/player data");
    clearCardReturn();sessionWords.clear();snapshotLeague=oldLeague;snapshotCareer=oldCareer;snapshotDatabase=oldDatabase;snapshotReady=wasReady;nativeReady=wasNative;
    cardFactory=oldFactory;cardObject=oldObject;cardBool=oldBool;cardInt=oldInt;cardText=oldText;decodeReference=oldDecode;useSnapshot(prepareSnapshot(dataset));startOnUserTeam();opened=true;source=7;
}
int previewMockCalls=0,previewMockRestored=-2;bool previewMockAccepted=true,previewMockThrows=false,previewMockComplete=false;
std::optional<unsigned> previewMockIntermediate;std::vector<std::string> previewMockKeys;
constexpr uintptr_t previewProviderAddress=0x200050000,previewRosterAddress=0x200060000,previewFlowAddress=0x200070000;
std::set<unsigned> previewNativeCache;
void* __fastcall previewLookupMock(void* flow,unsigned reference){check(flow==reinterpret_cast<void*>(previewFlowAddress),"Preview lookup used a fabricated native flow");return previewNativeCache.count(reference)?reinterpret_cast<void*>(0x123456):nullptr;}
bool __fastcall previewAsyncMock(void* object,int id,void* input){
    using namespace nativeanalytics;++previewMockCalls;check(object==reinterpret_cast<void*>(previewProviderAddress)&&id==0,"Preview refresh used a fabricated/foreign receiver or source");
    check(consumeNativeFilters(object,id,input),"Native-shaped preview alias failed");previewMockKeys=nativeFilterKeys;
    check(countHook(object,id)==static_cast<int>(page["rows"].size()),"Preview async call held the view mutex or changed the National page");
    if(previewMockIntermediate){auto value=std::to_string(*previewMockIntermediate);AliasArgument arg{aliasVtable,value.c_str()};ensurePreview(reinterpret_cast<void*>(previewRosterAddress),&arg);}
    if(previewMockThrows)throw std::runtime_error("Synthetic preview async failure");
    if(previewMockComplete){if(preview.requestRef)previewNativeCache.insert(*preview.requestRef);void* out=nullptr;void* result=nullptr;AliasArgument arg{aliasVtable,"COT_RETURN_SELECTION"};check(playerCard(&out,&arg,result)&&out,"Immediate preview refresh did not use a fresh return object");previewMockRestored=analyticscardtests::output.at("cotReturnRow").get<int>();}
    return previewMockAccepted;
}
void testPreviewHydration(const Json& dataset){
    using namespace nativeanalytics;
    const auto oldFactory=cardFactory;const auto oldObject=cardObject;const auto oldBool=cardBool;const auto oldInt=cardInt;const auto oldText=cardText;const auto oldDecode=decodeReference;const auto oldAsync=asyncFilter;const auto oldLookup=lookupPlayer;
    const auto oldLeague=snapshotLeague.load();const auto oldCareer=snapshotCareer.load(),oldDatabase=snapshotDatabase.load();const auto oldSnapshot=snapshotReady.load(),oldReady=nativeReady.load();const auto oldBegin=beginUpdate;
    constexpr uintptr_t service=0x200000000,career=0x200010000,ui=0x200020000,database=0x200030000,owner=0x200040000;
    sessionWords={{base+0xfa519b8,service},{service,base+0xbaa8998},{service+0x468,career},{service+0x470,ui},{service+0x460,owner},{service+0x7b1,1},{career,base+0xbce9b08},{ui,base+0xbd7a538},{career+0x20,owner},{career+0x238,database},{database,base+0xbd0d378},{base+0xf55c998,4294967295u},{previewProviderAddress,base+0xbacd668},{previewRosterAddress,base+0xbab18c0},{previewRosterAddress+0x418,previewFlowAddress},{previewFlowAddress,base+0xbd872e0},{previewFlowAddress+8,base+0xbd873b0},{base+0xbd873b0+8,base+0x381b250},{previewFlowAddress+0xb1,1},{previewFlowAddress+0xb4,1},{previewFlowAddress+0xc4,0},{previewFlowAddress+0x140,0x200080000}};
    snapshotLeague=4294967295u;snapshotCareer=career;snapshotDatabase=database;snapshotReady=true;nativeReady=true;opened=true;disabled=false;
    cardFactory=analyticscardtests::factory;cardObject=analyticscardtests::object;cardBool=analyticscardtests::boolean;cardInt=analyticscardtests::integer;cardText=analyticscardtests::text;decodeReference=analyticscardtests::decode;asyncFilter=previewAsyncMock;lookupPlayer=previewLookupMock;previewNativeCache.clear();beginUpdate=sourceBeginFallback;
    analyticscardtests::factoryFails=false;analyticscardtests::allocationFails=false;clearCardReturn();clearPreview();
    auto expanded=dataset;expanded["teams"].push_back({{"index",16},{"name","Another School"}});
    expanded["roster"].push_back({{"playerKey","5:2"},{"name","Z Opponent"},{"position","CB"},{"teamIndex",15},{"playerInfo",{{"firstName","Z"},{"lastName","Opponent"},{"position","CB"},{"isEmpty",false}}}});expanded["roster"].push_back({{"playerKey","5:3"},{"name","A Another"},{"position","WR"},{"teamIndex",16},{"playerInfo",{{"firstName","A"},{"lastName","Another"},{"position","WR"},{"isEmpty",false}}}});
    useSnapshot(prepareSnapshot(expanded));selectedTeam=analytics::NationalTeam;sortColumn=5;sortDescending=true;hasUserSort=false;rebuildPage();source=0;
    const auto captured=expanded;const auto visiblePage=page;const unsigned first=5u<<17,opponent=first|2,another=first|3;void* output=nullptr;void* result=nullptr;
    auto invoke=[&](unsigned reference){auto text=std::to_string(reference);AliasArgument arg{aliasVtable,text.c_str()};ensurePreview(reinterpret_cast<void*>(previewRosterAddress),&arg);};
    auto choose=[&](unsigned reference){auto text=std::string("COT_SELECT:")+std::to_string(reference);AliasArgument arg{aliasVtable,text.c_str()};const auto before=analyticscardtests::allocations;check(playerCard(&output,&arg,result)&&!output&&analyticscardtests::allocations==before,"Settled selection allocated a DTO or fell through to native decoding");invoke(reference);};
    auto query=[&](){if(preview.pending&&!preview.cancelled&&preview.requestRef)previewNativeCache.insert(*preview.requestRef);AliasArgument arg{aliasVtable,"COT_RETURN_SELECTION"};check(playerCard(&output,&arg,result)&&output,"Preview completion fell through to native player decoding");return analyticscardtests::output.at("cotReturnRow").get<int>();};
    auto seed=[&](){nativeTeamKeys={{14,testTeamKey},{15,opponentKey},{16,"third-native-key"},{analytics::NationalTeam,nationalKey}};nativeWeekDefault=rosterPositionKey;};
    auto bind=[&](){beginHook(reinterpret_cast<void*>(previewProviderAddress),0,27);check(preview.provider==reinterpret_cast<void*>(previewProviderAddress),"Native Begin did not capture the verified real provider");};
    auto reset=[&](){selectedTeam=analytics::NationalTeam;seasonYear=0;selectedWeek=1;selectedType="RegularSeason";sortColumn=5;sortDescending=true;hasUserSort=false;rebuildPage();clearPreview();previewNativeCache.clear();seed();bind();previewMockAccepted=true;previewMockThrows=false;previewMockComplete=false;previewMockIntermediate.reset();};
    seed();capturePreviewSource(reinterpret_cast<void*>(1),0,27);check(!preview.provider,"Unreadable provider was captured");sessionWords[previewProviderAddress]=base+0xbacd670;capturePreviewSource(reinterpret_cast<void*>(previewProviderAddress),0,27);check(!preview.provider,"Foreign provider vtable was captured");sessionWords[previewProviderAddress]=base+0xbacd668;
    capturePreviewSource(reinterpret_cast<void*>(previewProviderAddress),1,27);capturePreviewSource(reinterpret_cast<void*>(previewProviderAddress),0,28);check(!preview.provider,"Non-row/foreign source was admitted for native preview");bind();
    for(const auto& [address,bad]:std::vector<std::pair<uintptr_t,uint64_t>>{{previewRosterAddress,base+0xbab18c8},{previewFlowAddress,base+0xbd872e8},{previewFlowAddress+8,base+0xbd873b8},{base+0xbd873b0+8,base+0x381b258},{previewFlowAddress+0xb1,0},{previewFlowAddress+0xb4,0},{previewFlowAddress+0xc4,1},{previewFlowAddress+0x140,0}}){const auto good=sessionWords.at(address);sessionWords[address]=bad;invoke(opponent);check(previewMockCalls==0&&!preview.pending,"Foreign/unready native Roster context requested preview data");sessionWords[address]=good;}
    invoke(first|999);check(previewMockCalls==0&&!preview.pending,"Unknown player triggered native preview hydration");
    nativeTeamKeys.erase(15);invoke(opponent);check(previewMockCalls==0,"Missing exact native team key guessed a preview team");seed();
    cardReturnReference=first;cardReturnPending=true;invoke(opponent);check(previewMockCalls==0&&cardReturnReference==first,"Preview replaced a full-card ticket");clearCardReturn();
    previewMockIntermediate=first;invoke(opponent);check(previewMockCalls==1&&preview.pending&&preview.requestRef==opponent&&!preview.latestRef&&previewMockKeys==std::vector<std::string>{opponentKey,rosterPositionKey}&&page==visiblePage,"Initial preview hydration used wrong school/positions or let native row0 steal the target");
    AliasArgument premature{aliasVtable,"COT_RETURN_SELECTION"};check(playerCard(&output,&premature,result)&&output&&analyticscardtests::output.at("cotReturnRow")==-1&&preview.pending&&preview.requestRef==opponent&&!preview.loadedTeam,"Premature outer refresh consumed a preview before its player reached native cache");
    sessionWords[previewFlowAddress+0xc4]=1;previewNativeCache.insert(opponent);check(playerCard(&output,&premature,result)&&preview.pending&&analyticscardtests::output.at("cotReturnRow")==-1,"Pending native flow completed a preview merely because an old cache contained the player");sessionWords[previewFlowAddress+0xc4]=0;previewNativeCache.clear();
    invoke(opponent);invoke(first);invoke(another);check(previewMockCalls==1&&!preview.latestRef,"Intermediate getters, including nonzero old rows, stole the pending player");choose(another);invoke(first);check(preview.latestRef==another&&preview.requestRef==opponent,"Queued explicit nonzero selection was lost to intermediate row0");
    check(query()==rowLookup.at(another)&&preview.loadedTeam==15&&!preview.pending,"Preview completion failed to restore the queued exact player");invoke(another);check(previewMockCalls==2&&preview.requestTeam==16&&previewMockKeys[0]=="third-native-key","Queued cross-school selection did not request its actual team after completion");check(query()==rowLookup.at(another),"Second team hydration restored a different player");invoke(another);check(previewMockCalls==2,"Loaded native preview team was requested repeatedly");
    previewMockComplete=true;choose(first);check(previewMockCalls==3&&!preview.pending&&preview.loadedTeam==14&&previewMockRestored==rowLookup.at(first),"Immediate native completion deadlocked or lost selected identity");previewMockComplete=false;
    reset();previewMockAccepted=false;invoke(opponent);const int failedCalls=previewMockCalls;check(!preview.pending&&preview.failedRef==opponent,"Rejected native preview left a request in flight");invoke(opponent);invoke(opponent);check(previewMockCalls==failedCalls,"Rejected preview retried indefinitely on repeated getters");previewMockAccepted=true;choose(another);query();previewMockAccepted=false;choose(opponent);check(previewMockCalls==failedCalls+2&&preview.failedRef==opponent,"New user selection did not allow a bounded retry");
    reset();previewMockThrows=true;invoke(opponent);const int thrownCalls=previewMockCalls;invoke(opponent);check(previewMockCalls==thrownCalls&&!preview.pending&&preview.failedRef==opponent,"Preview exception escaped or caused an automatic retry loop");
    reset();invoke(opponent);std::vector<std::string> selection={"ANALYTICS_TEAM:14"};FilterAlias selected{filterAliasVt,&selection};(void)applyFilters(&selected);check(preview.pending&&preview.cancelled&&!preview.loadedTeam,"Custom unchanged filter failed to invalidate preview cache/restore");check(query()==0&&!preview.pending&&!preview.loadedTeam,"New filter failed to initialize its own first row");choose(first);query();
    reset();invoke(opponent);selection={"ANALYTICS_WEEK:0"};(void)applyFilters(&selected);invoke(first);check(preview.cancelled&&!preview.latestRef,"New-week getter was mistaken for explicit user input");choose(first);check(preview.latestRef==first,"Explicit settled selection was ignored");check(query()==rowLookup.at(first)&&!preview.loadedTeam,"Week refresh restored old view identity or assumed its old cache");
    reset();sortColumn=5;sortDescending=true;hasUserSort=false;rebuildPage();invoke(opponent);const auto priorSort=page;check(sortHook(nullptr,0,5)&&preview.pending&&!preview.cancelled&&page==priorSort&&!hasUserSort,"Automatic same-column preview refresh toggled sort order");check(sortHook(nullptr,0,2)&&preview.cancelled&&sortColumn==2,"Explicit different-column sort retained stale preview restore");check(query()==0,"New sort failed to initialize its own first row");
    reset();invoke(opponent);auto action=std::string("COT_CARD:")+std::to_string(opponent);AliasArgument actionArg{aliasVtable,action.c_str()};check(playerCard(&output,&actionArg,result)&&!output&&!cardReturnPending&&preview.pending,"Full-card action armed against a pending native preview");query();
    check(playerCard(&output,&actionArg,result)&&output&&analyticscardtests::output.value("cotCardReady",false)&&cardReturnPending&&cardReturnReference==opponent,"Completed preview did not expose a ready command and full-card return ticket");clearPreview();check(cardReturnPending&&cardReturnReference==opponent,"Clearing preview state erased an independent full-card ticket");
    check(resumeCard()&&cardSelectionRestorePending&&!preview.provider&&!preview.pending,"Full-card return retained preview provider/cache state");seed();bind();invoke(first);check(!preview.pending,"Intermediate row during full-card return started preview hydration");check(query()==rowLookup.at(opponent),"Full-card return was replaced by a preview identity");previewNativeCache.clear();invoke(opponent);check(preview.pending&&preview.requestRef==opponent,"Restored full-card row failed to rehydrate its actual native team");query();
    reset();invoke(opponent);AliasArgument exit{aliasVtable,"COT_LEAVE_VIEW"};check(playerCard(&output,&exit,result)&&!preview.provider&&!preview.pending,"Explicit view exit retained native preview state");opened=true;source=0;reset();invoke(opponent);leave();check(!preview.provider&&!preview.pending,"View leave retained pending native preview");opened=true;source=0;reset();
    sessionWords[base+0xf55c998]=17;invoke(opponent);check(!preview.provider&&!preview.pending,"Stale Dynasty admitted a native preview query");sessionWords[base+0xf55c998]=4294967295u;
    reset();opened=false;const int beforeClosed=previewMockCalls;invoke(opponent);opened=true;disabled=true;invoke(opponent);disabled=false;check(previewMockCalls==beforeClosed,"Closed/disabled Analytics requested a native preview");
    reset();previewMockThrows=true;for(int i=0;i<24;++i)choose(i%2?first:opponent);size_t failures=0;for(const auto& entry:notes)if(entry.value("event",std::string())=="native_analytics_preview_rejected")++failures;check(failures==16,"Native preview failure diagnostics were not bounded");
    check(data==captured,"Native preview hydration changed captured Dynasty inputs");clearPreview();clearCardReturn();clearNativeFilters();sessionWords.clear();snapshotLeague=oldLeague;snapshotCareer=oldCareer;snapshotDatabase=oldDatabase;snapshotReady=oldSnapshot;nativeReady=oldReady;
    cardFactory=oldFactory;cardObject=oldObject;cardBool=oldBool;cardInt=oldInt;cardText=oldText;decodeReference=oldDecode;asyncFilter=oldAsync;lookupPlayer=oldLookup;beginUpdate=oldBegin;previewMockIntermediate.reset();previewMockComplete=false;previewMockThrows=false;useSnapshot(prepareSnapshot(dataset));startOnUserTeam();opened=true;source=7;
}
void testNationalCap(const Json& dataset){
    using namespace nativeanalytics;auto busy=dataset;busy["roster"]=Json::array();busy["rows"]=Json::array();
    for(int i=0;i<455;++i){auto player=dataset["roster"][0];const auto id="5:"+std::to_string(i);player["playerKey"]=id;player["name"]=(i<50?"A-low-":"Z-high-")+std::to_string(i);busy["roster"].push_back(player);
        if(i<450){auto performance=dataset["rows"][0];performance["playerKey"]=id;performance["name"]=player["name"];performance["analytics"]["grade"]=i<50?40.0+i/100.0:70.0+i/100.0;busy["rows"].push_back(std::move(performance));}
    }
    const auto original=busy;auto national=analytics::teamPage(busy,analytics::NationalTeam,0,1,"RegularSeason");auto complete=analytics::teamPage(busy,14,0,1,"RegularSeason");
    check(national["rows"].size()==400&&complete["rows"].size()==455,"National cap truncated a team roster or failed to bound National");
    check(national["rows"].front()["playerKey"]=="5:449"&&national["rows"].back()["playerKey"]=="5:50","National did not preselect the highest400 GPI grades");
    std::set<std::string> eligible;for(const auto& row:national["rows"]){eligible.insert(row["playerKey"].get<std::string>());check(row["grade"].is_number()&&row["name"].get<std::string>().rfind("Z-high-",0)==0,"Low-grade alphabet-first or ungraded row displaced a National leader");}
    for(int column:{2,3,4,5,6,7})for(bool descending:{false,true}){auto sorted=national;analytics::sortPage(sorted,column,descending);std::set<std::string> actual;for(const auto& row:sorted["rows"])actual.insert(row["playerKey"].get<std::string>());check(actual==eligible,"User sort changed National's preselected400 identities");}
    check(analytics::teamPage(dataset,analytics::NationalTeam,0,1,"RegularSeason")["rows"].size()==2,"Sparse National view was padded or lost eligible rows");
    // Legacy mixed-team helper remains an offline fixture; the native UI is
    // team-only, including stale selections and explicitly forged filters.
    selectedTeam=analytics::NationalTeam;useSnapshot(prepareSnapshot(busy));
    check(selectedTeam==14&&page["rows"].size()==455,"Removed National selection did not fall back to the full user team");
    check(std::none_of(teams.begin(),teams.end(),[](const auto& t){return t.first==analytics::NationalTeam;}),"National remains in Analytics filter choices");
    sortColumn=2;sortDescending=false;hasUserSort=true;rebuildPage();
    check(rowLookup.size()==455,"Team-only name sort truncated the roster");
    opened=true;source=7;nativeTeamKeys={{analytics::NationalTeam,nationalKey},{14,testTeamKey}};nativeWeekDefault=rosterPositionKey;
    std::vector<std::string> selection={"ANALYTICS_TEAM:-1"};FilterAlias request{filterAliasVt,&selection};
    const auto stable=page;bool rejected=false;try{(void)applyFilters(&request);}catch(const std::exception&){rejected=true;}
    check(rejected&&page==stable&&selectedTeam==14,"Stale National key bypassed team-only filter restrictions");
    selection={"ANALYTICS_TEAM:14","ANALYTICS_WEEK:0"};(void)applyFilters(&request);
    check(page["rows"].size()==455&&selectedWeek==analytics::SeasonWeek,"Season team filter inherited National limits");
    check(busy==original,"National selection changed captured inputs");clearNativeFilters();useSnapshot(prepareSnapshot(dataset));startOnUserTeam();opened=true;source=7;
}
Json input(){
 Json raw={{"status",{{"league",4294967295u},{"season",{{"CurrentYear",0},{"CurrentSeasonYear",2026},{"CurrentWeek",2},{"CurrentWeekType","RegularSeason"}}}}},{"records",Json::object()}};
 auto record=[&](const char* key,const char* type,Json v,int capacity=150){raw["records"][key]={{"key",key},{"type",type},{"capacity",capacity},{"v",v}};};
 record("1:0","Franchise",{{"LeagueID",4294967295u},{"FCSTeams","2:0"}},1);record("2:0","Team[]",{{"Member0","3:2"}});
 record("3:0","Team",{{"TeamIndex",14},{"DisplayName","Test University"},{"TEAM_LOGO",14}});record("3:1","Team",{{"TeamIndex",15},{"DisplayName","Opponent"},{"TEAM_LOGO",15}});record("3:2","Team",{{"TeamIndex",254},{"DisplayName","FCS Test"}});
 record("4:0","FranchiseUser",{{"Team","3:0"},{"AdminLevel","Owner"},{"IsRetired",false}},32);
 record("5:0","Player",{{"FirstName","Longfirstname"},{"LastName","Longlastname-Suffix"},{"TeamIndex",14},{"Position","QB"},{"GameStats","6:0"},{"OverallRating",99},{"TraitDevelopment","Elite"},{"Height",73},{"Weight",40},{"SchoolYear","Junior"}});
 record("5:1","Player",{{"FirstName","No"},{"LastName","Snaps"},{"TeamIndex",14},{"Position","WR"},{"GameStats",nullptr}});
 record("6:0","GameStats[]",{{"Member0","7:0"},{"Member1","7:0"},{"Member2",nullptr}});
 Json stats={{"SeasonGame","8:0"},{"OpposingTeam","3:1"},{"DOWNSPLAYED",60},{"GAMERATING",99},{"PASSYARDS",300},{"PASSCOMPLETED",22},{"PASSATTEMPTS",34},{"PASSTDS",3},{"PASSINTS",1},{"PASSSACKED",1},{"RUSHYARDS",15},{"RUSHTDS",0},{"RUSHFUMBLES",0}};
 record("7:0","GameOffensiveStats",stats);
 record("8:0","SeasonGame",{{"IsPractice",false},{"SeasonYear",0},{"SeasonWeek",1},{"SeasonWeekType","RegularSeason"},{"GameStatus","HomeWon"},{"HomeTeam","3:0"},{"AwayTeam","3:1"}});
 return raw;
}
}
int main(int argc,char** argv){using namespace fixture;using namespace fixture::nativeanalytics;
 try{
  std::ifstream calibrationFile(std::filesystem::path(__FILE__).parent_path().parent_path()/"Source/analytics-baselines.json");Json calibration;calibrationFile>>calibration;check(analytics::calibration()==calibration,"Embedded calibration differs from the imported frozen source");
  check(analyticsPrefixRva==0x80764a0,"Analytics prefix must use native tag1 encoder, not tag2");
  auto raw=input(),unchanged=raw;auto result=analytics::live::dataset(raw);check(raw==unchanged,"Adapter changed captured records");
  auto aliasInput=raw;aliasInput["records"]["3:0"]["v"]["LongName"]="Test University";aliasInput["records"]["3:0"]["v"]["TEAM_PREFIX_NAME"]="Test U";aliasInput["records"]["3:0"]["v"]["ShortName"]="TU";
  const auto unchangedAliases=aliasInput;const auto aliasData=analytics::live::dataset(aliasInput);check(aliasData["teams"][0]["nativeNames"]==Json::array({"Test University","Test U","TU"})&&aliasInput==unchangedAliases,"Capture lost reliable native aliases, retained duplicates or changed native records");
  auto hometown=raw;hometown["records"]["5:0"]["v"]["PLYR_HOME_TOWN"]="Madison";hometown["records"]["5:0"]["v"]["PLYR_HOME_STATE"]="Wisconsin";
  check(analytics::live::dataset(hometown)["roster"][0]["playerInfo"]["hometown"]=="Madison, Wisconsin","Card hometown discarded native city/state");
  hometown["records"]["5:0"]["v"]["PLYR_HOME_STATE"]="";check(analytics::live::dataset(hometown)["roster"][0]["playerInfo"]["hometown"]=="Madison","Missing state invented a hometown suffix");
  hometown["records"]["5:0"]["v"]["Position"]="LE";check(analytics::live::dataset(hometown)["roster"][0]["playerInfo"]["position"]=="LEDG","Card position differs from native Analytics display label");
  check(result["leagueId"]==4294967295u&&result["userTeamIndex"]==14,"Unsigned LeagueID/user team mapping changed");
  check(result["rows"].size()==1&&result["issues"].empty(),"Repeated stat references duplicated games");check(result["teams"][2]["isFcs"]==true,"Native FCSTeams references were ignored");
  const auto grade=result["rows"][0]["analytics"];check(grade["grade"].is_number(),"Verified completed QB performance is not graded");check(!result["rows"][0]["stats"].contains("GAMERATING"),"Native GAMERATING entered grading input");
  check(grade.at("modelVersion")==analytics::ModelVersion&&grade.at("cohortSamples")==1,"Native adapter did not use the sparse position model");
  auto summary=analytics::live::socialSummary(raw);check(summary["modelVersion"]==analytics::ModelVersion&&summary["leagueId"]==result["leagueId"]&&summary["weekly"][0]["grade"]==grade["grade"],"Social grade differs from its native weekly menu source");
  check(summary["seasonRows"][0]["grade"]==analytics::teamPage(result,14,0,analytics::SeasonWeek,"Season")["rows"][0]["grade"],"Social season grade differs from the native Season menu");
  auto corroborated=raw;corroborated["records"]["5:0"]["v"]["SeasonStats"]="10:0";
  corroborated["records"]["10:0"]={{"key","10:0"},{"type","SeasonStats[]"},{"capacity",150},{"v",{{"Member0","11:0"}}}};
  corroborated["records"]["11:0"]={{"key","11:0"},{"type","SeasonOffensiveStats"},{"capacity",150},{"v",{{"SEAS_YEAR",0},{"DOWNSPLAYED",60},{"YEARBYYEARTEAMINDEX",14}}}};
  auto verifiedCoverage=analytics::live::socialSummary(corroborated).at("coverage");
  check(verifiedCoverage.at("scheduleComplete")==true&&std::find(verifiedCoverage.at("positions").begin(),verifiedCoverage.at("positions").end(),"QB")!=verifiedCoverage.at("positions").end(),"Reconciled participation cannot enter GPI rankings");
  corroborated["records"]["11:0"]["v"]["DOWNSPLAYED"]=61;auto incompleteCoverage=analytics::live::socialSummary(corroborated).at("coverage");
  check(std::find(incompleteCoverage.at("positions").begin(),incompleteCoverage.at("positions").end(),"QB")==incompleteCoverage.at("positions").end(),"Missing game-cache participation can create a false GPI leader");
  auto changed=raw;changed["records"]["5:0"]["v"]["OverallRating"]=10;changed["records"]["5:0"]["v"]["TraitDevelopment"]="Normal";changed["records"]["7:0"]["v"]["GAMERATING"]=0;
  check(analytics::live::dataset(changed)["rows"][0]["analytics"]==grade,"Hidden/native ratings influence production grade");
  changed=raw;changed["records"]["7:0"]["v"].erase("PASSINTS");check(analytics::live::dataset(changed)["rows"][0]["analytics"]["grade"].is_null(),"Missing stats were fabricated");
  changed=raw;changed["records"]["7:0"]["v"]["DOWNSPLAYED"]=0;check(analytics::live::dataset(changed)["rows"][0]["analytics"]["grade"].is_null(),"Zero snaps generated a grade");
  changed=raw;changed["records"]["8:0"]["v"]["GameStatus"]="Scheduled";check(analytics::live::dataset(changed)["rows"][0]["analytics"]["grade"].is_null(),"Unplayed games generated grades");
  changed=raw;changed["records"]["8:0"]["v"]["IsPractice"]=true;check(analytics::live::dataset(changed)["rows"].empty(),"Practice entered Dynasty grades");
  for(const char* phase:{"PreSeason","Preseason"}){
    changed=raw;changed["records"]["8:0"]["v"]["SeasonWeekType"]=phase;changed["records"]["8:0"]["v"]["IsPractice"]=false;
    auto spring=analytics::live::dataset(changed);check(spring["rows"].empty()&&spring["games"].empty()&&spring["seasonAnalytics"].empty(),"Extension exhibition entered weekly/season grades");
    auto springSummary=analytics::live::socialSummary(changed);check(springSummary["weekly"].empty()&&springSummary["seasonRows"].empty(),"Extension exhibition entered GPI posts");
  }
  changed=raw;changed["records"]["7:0"]["v"]["OpposingTeam"]="3:2";auto foreign=analytics::live::dataset(changed);check(foreign["rows"].empty()&&foreign["issues"].size()==1,"Foreign opponent entered game stats");
  auto unreleased=result;unreleased["season"]["CurrentWeek"]=1;
  auto hidden=prepareSnapshot(unreleased);check(hidden.weeks.size()==1,"Current week appeared in Analytics filters");
  check(analytics::teamPage(unreleased,14,0,1,"RegularSeason")["rows"][0]["grade"].is_null(),"Current-week grade leaked through a direct request");
  check(analytics::teamPage(unreleased,14,0,analytics::SeasonWeek,"Season")["rows"][0]["grade"].is_null(),"Season average leaked current-week grades");
  unreleased["season"]["CurrentWeek"]=0;unreleased["games"][0]["SeasonWeek"]=0;
  check(prepareSnapshot(unreleased).weeks.size()==1,"Week zero appeared before Week one");
  unreleased["season"]["CurrentWeek"]=1;check(prepareSnapshot(unreleased).weeks.back().week==0,"Week zero was not released in Week one");
  unreleased["season"]["CurrentWeekType"]="OffSeason";unreleased["season"]["CurrentWeek"]=0;
  check(analytics::releasedWeek(unreleased,0,20,"PostSeason"),"Offseason reset hid completed postseason");
  check(!analytics::releasedWeek(unreleased,1,0,"RegularSeason"),"Future season was released");
  check(!analytics::releasedWeek(unreleased,0,0,"UnknownStage"),"Unknown stage did not fail closed");
  auto prepared=prepareSnapshot(result);useSnapshot(std::move(prepared));startOnUserTeam();opened=true;source=7;
  check(teams.front().first!=analytics::NationalTeam&&teams.size()==2,"Team choices include removed National or an FCS team");check(selectedTeam==14&&page["rows"].size()==2,"Native table does not start on user roster");
  check(page["rows"][0]["grade"].is_number()&&page["rows"][1]["grade"].is_null(),"Missing participation does not remain N/A");
  check(playerCards.at((5u<<17))["height"]=="6' 1\""&&playerCards.at((5u<<17))["weight"]==200,"Native card dimensions changed");
  analyticscardtests::run(check,[](void** out,void* arg,void*& result){return playerCard(out,arg,result);});
  check(recordKey("5:0")=="000A0000","Native player-reference packing changed");bool rejected=false;try{recordKey("5:131072");}catch(...){rejected=true;}check(rejected,"Out-of-range record accepted");
  void* vt[]={nullptr,reinterpret_cast<void*>(reserve),nullptr,reinterpret_cast<void*>(accept<bool>),reinterpret_cast<void*>(accept<int>),reinterpret_cast<void*>(scalar),reinterpret_cast<void*>(accept<const char*>)};struct Target{void** vt;}target{vt};
  collected=Json::object();columnHook(nullptr,&target,0);check(collected.size()==8&&collected["5"]["display"]=="GPI GRADE"&&collected["6"]["display"]=="CLASSIFICATION","Native columns missing production grades/classification");
  void* cvt[]={nullptr,reinterpret_cast<void*>(colsCount),reinterpret_cast<void*>(colAt)};Target cols{cvt};collected=Json::object();rowHook(nullptr,&target,7,0,&cols);check(reserved==6&&collected["3"]["display"].is_string(),"Native row collector ABI changed");
  encodePrefix=encodeValue=encodeSuffix=encode;collected=Json::object();keyHook(nullptr,&target,7,0);check(encoded.size()==3&&encoded[0]==255&&encoded[1]==(5u<<17)&&encoded[2]==255,"Native serial-key identity changed");
  nativeSort=sortFallback;check(sortHook(nullptr,7,5)&&forwarded==2,"Native sort refresh does not use valid donor column");check(page["rows"].back()["grade"].is_null(),"N/A sorts before real production grades");
  count=countFallback;check(countHook(nullptr,8)==123,"Unrelated table was intercepted");disabled=true;check(countHook(nullptr,7)==123,"Disabled module retained analytics table");disabled=false;
  testSourceColumns(&target);
  testSourceLifecycle();
  testFilters(result);
  testCardReturnSelection(result);
  testNationalCap(result);
  testPreviewHydration(result);
  auto season=analytics::teamPage(result,analytics::NationalTeam,0,analytics::SeasonWeek,"Season");check(season["rows"].size()==2&&season["rows"][0]["grade"].is_number(),"Season/National aggregation changed");
  auto doubleStat=result;auto duplicate=doubleStat["rows"][0];duplicate["statType"]="GameOffensiveKPReturnStats";duplicate["analytics"]["grade"]=nullptr;doubleStat["rows"].push_back(duplicate);check(analytics::teamPage(doubleStat,14,0,1,"RegularSeason")["rows"].size()==2,"Return stats duplicate a player's primary game row");
  doubleStat["rows"][1]["statType"]="GameDefensiveStats";check(analytics::teamPage(doubleStat,14,0,1,"RegularSeason")["rows"].size()==2,"Incompatible position stats entered the weekly grade table");
  auto missingOwner=raw;missingOwner["records"]["4:0"]["v"]["IsRetired"]=true;rejected=false;try{analytics::live::dataset(missingOwner);}catch(...){rejected=true;}check(rejected,"Retired/missing user used a previous Dynasty");
  cardReturnPending=true;check(!resumeCard()&&!cardReturnPending,"Stale Dynasty resumed an Analytics card ticket");
  cardReturnPending=true;leave();check(!cardReturnPending,"Leaving Analytics retained a card return ticket");
  snapshotReady=true;nativeReady=true;check(!available()&&!open(),"Absent native Dynasty admitted a stale snapshot");leave();check(!opened&&source==-1,"Leaving native Analytics retained source ownership");
  for(const auto& binding:bindings){check(!(binding.vt==0xbacd668&&(binding.offset==0x68||binding.offset==0x60))&&!(binding.vt==0xbaa78b8&&binding.offset==0x70)&&binding.vt!=0xbac8c40&&binding.vt!=0xbab18c0,"Analytics competes with a shared COT hook");check(binding.vt!=0xbab13f0,"Analytics still intercepts the unrelated Dynasty information callback");}
  check(std::size(bindings)==14&&bindings[13].vt==0xbacd668&&bindings[13].offset==0x88&&bindings[13].rva==0x7a89420&&bindings[13].hook==reinterpret_cast<void*>(sourceColumnHook),"Roster source-column binding moved or changed its verified native contract");
  std::map<uintptr_t,std::vector<unsigned char>> nativeBytes;constexpr uintptr_t image=0x140000000;
  for(const auto& gate:analyticsGates)nativeBytes[image+gate.rva]={gate.bytes,gate.bytes+32};
  for(const auto& binding:bindings){auto address=image+binding.rva;std::vector<unsigned char> bytes(8);memcpy(bytes.data(),&address,8);nativeBytes[image+binding.vt+binding.offset]=bytes;}
  auto query=[&](uintptr_t at,void* out,size_t n){auto found=nativeBytes.find(at);if(found==nativeBytes.end()||found->second.size()!=n)return false;memcpy(out,found->second.data(),n);return true;};validateContract(image,query);check(true,"Pristine native contract rejected");
  for(auto& [at,bytes]:nativeBytes)for(size_t offset=0;offset<bytes.size();++offset){bytes[offset]^=1;bool refused=false;try{validateContract(image,query);}catch(...){refused=true;}check(refused,"Changed instruction or foreign slot passed Analytics gates");bytes[offset]^=1;}
  rejected=false;try{validateContract(image,[](uintptr_t,void*,size_t){return false;});}catch(...){rejected=true;}check(rejected,"Unreadable native code passed Analytics gates");
  // Exercise the same independently emitted helper fixtures used by the
  // existing COT contract checks. No game process, allocations or writes.
  for(unsigned variant=0;variant<4;++variant){
    const uintptr_t relocated=image+((variant&1)?0x10000000:0),code=0x300000000ull;std::map<uintptr_t,std::vector<unsigned char>> memory;
    for(const auto& gate:analyticsGates)memory[relocated+gate.rva]={gate.bytes,gate.bytes+32};
    for(const auto& binding:bindings){uintptr_t pointer=relocated+binding.rva;auto& bytes=memory[relocated+binding.vt+binding.offset];bytes.resize(8);memcpy(bytes.data(),&pointer,8);}
    auto entry=[&](uintptr_t rva,uintptr_t target,size_t size){auto& bytes=memory[relocated+rva];if(bytes.size()<size)bytes.resize(size);std::fill(bytes.begin(),bytes.begin()+size,static_cast<unsigned char>(0x90));bytes[0]=0xff;bytes[1]=0x25;bytes[2]=bytes[3]=bytes[4]=bytes[5]=0;memcpy(bytes.data()+6,&target,8);};
    entry(cotcompat::practice::site0,code,cotcompat::practice::original0.size());entry(cotcompat::practice::site13,code+13*1024,cotcompat::practice::original13.size());entry(cotcompat::practice::site14,code+14*1024,cotcompat::practice::original14.size());
    memory[code]={fixture0[variant].begin(),fixture0[variant].end()};memory[code+13*1024]={fixture13[variant].begin(),fixture13[variant].end()};memory[code+14*1024]={fixture14[variant].begin(),fixture14[variant].end()};
    auto readFixture=[&](uintptr_t at,void* out,size_t size){auto found=memory.upper_bound(at);if(found==memory.begin())return false;--found;if(at-found->first>found->second.size()||size>found->second.size()-(at-found->first))return false;memcpy(out,found->second.data()+at-found->first,size);return true;};
    const auto original=memory;validateContract(relocated,readFixture);check(true,"Exact known Practice text hook blocked Analytics");check(memory==original,"Analytics normalization changed helper/native memory");
    memory[code+14*1024][20]^=1;bool refused=false;try{validateContract(relocated,readFixture);}catch(...){refused=true;}check(refused,"A changed/foreign Practice body passed Analytics validation");
  }
  check(inflight==0,"UI callbacks leaked ownership drain counts");
  if(argc==2){std::ifstream file(argv[1]);Json captured;file>>captured;const auto before=captured;auto start=std::chrono::steady_clock::now();auto dataset=analytics::live::dataset(captured);auto snapshot=prepareSnapshot(dataset);const auto end=std::chrono::steady_clock::now();check(captured==before,"Real read-only fixture changed");size_t graded=0;for(const auto& row:dataset.at("rows"))if(row.at("analytics").at("grade").is_number())++graded;check(graded>0,"Actual Dynasty fixture has no graded rows");std::cout<<"Actual native adapter: "<<dataset.at("roster").size()<<" roster, "<<dataset.at("rows").size()<<" game records, "<<graded<<" grades, "<<dataset.at("issues").size()<<" rejected linkages; prepare="<<std::chrono::duration<double,std::milli>(end-start).count()<<" ms (offline fixture, not live menu).\n";}
  std::cout<<"PASS: "<<checks<<" analytics integration checks; no game hooks installed.\n";return 0;
 }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
