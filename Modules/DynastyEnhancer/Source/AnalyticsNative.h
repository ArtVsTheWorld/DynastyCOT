#pragma once
// Included inside the existing COT Fog owner after its common helpers.
#include "AnalyticsGates.h"
#include <limits>
namespace nativeanalytics {
std::atomic<bool> nativeReady{false};

struct AliasArgument{void** vt;const char* text;};
void __fastcall aliasRead(AliasArgument* a,char* out,unsigned n){if(n)strncpy_s(out,n,a->text,_TRUNCATE);}
void* aliasVtable[]={nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,reinterpret_cast<void*>(aliasRead)};
std::string argument(void* p){
    uintptr_t vt=0,method=0;if(!read(reinterpret_cast<uintptr_t>(p),&vt,8))return {};
    if(vt==reinterpret_cast<uintptr_t>(aliasVtable))return static_cast<AliasArgument*>(p)->text;
    if(vt!=base+0xb575928||!read(vt+0x30,&method,8)||method!=base+0x3f07c50)return {};
    char text[128]{};reinterpret_cast<void(__fastcall*)(void*,char*,unsigned)>(method)(p,text,128);text[127]=0;return text;
}


std::atomic<bool> opened{false},intent{false},cardReturnPending{false};
bool currentProfile();
bool resumeCard();std::atomic<int> source{-1};
Json data,page;
std::map<unsigned,Json> playerCards;
int seasonYear=0,selectedTeam=-2,selectedWeek=1,sortColumn=5;
bool sortDescending=true,hasUserSort=false;
std::string selectedType="RegularSeason",teamName;
std::mutex viewMutex;
std::optional<unsigned> cardReturnReference;
bool cardSelectionRestorePending=false;
struct PreviewState{
    void* provider=nullptr;void* roster=nullptr;
    std::optional<int> loadedTeam,requestTeam;
    std::optional<unsigned> requestRef,latestRef,lastRef,failedRef,desiredRef;
    bool pending=false,cancelled=false;
    uint64_t requestId=0;
};
PreviewState preview;uint64_t previewSequence=0;
void clearPreview(){std::lock_guard<std::mutex> lock(viewMutex);preview=PreviewState{};++previewSequence;}
void invalidatePreviewLocked(){
    preview.loadedTeam.reset();preview.failedRef.reset();preview.lastRef.reset();preview.latestRef.reset();preview.desiredRef.reset();
    if(preview.pending)preview.cancelled=true;
}
void capturePreviewSource(void* object,int id,int type)noexcept{
    if(disabled||!opened||id!=0||type!=27)return;
    uintptr_t vt=0;if(!read(reinterpret_cast<uintptr_t>(object),&vt,8)||vt!=base+0xbacd668)return;
    std::lock_guard<std::mutex> lock(viewMutex);preview.provider=object;
}
void ensurePreview(void* rosterObject,void* argument)noexcept;
bool previewReady(void* rosterObject,std::optional<unsigned> reference=std::nullopt)noexcept;
void clearCardReturn(){std::lock_guard<std::mutex> lock(viewMutex);cardReturnPending=false;cardReturnReference.reset();cardSelectionRestorePending=false;}
std::array<int,64> sourceTypes{};
using SyncInit=void(__fastcall*)(void*,int,int);
using ColData=void(__fastcall*)(void*,void*,int,int);
using Filter=bool(__fastcall*)(void*,int,void*);
SyncInit syncInit=nullptr,beginUpdate=nullptr;ColData colData=nullptr;Filter filter=nullptr,asyncFilter=nullptr;
std::map<int,std::string> nativeTeamKeys;
std::string nativeWeekDefault;
void clearNativeFilters(){std::lock_guard<std::mutex> lock(viewMutex);nativeTeamKeys.clear();nativeWeekDefault.clear();}
std::vector<std::pair<int,std::string>> teams;
struct WeekChoice{int year,week;std::string type;};
std::vector<WeekChoice> weeks;
struct PreparedSnapshot{Json dataset;std::map<unsigned,Json> cards;std::vector<std::pair<int,std::string>> teams;std::vector<WeekChoice> weeks;};
std::mutex pendingMutex;
std::optional<PreparedSnapshot> nextSnapshot;

using Columns=void(__fastcall*)(void*,void*,int);
using Rows=void(__fastcall*)(void*,void*,int,int,void*);
using Count=int(__fastcall*)(void*,int);
using Sort=bool(__fastcall*)(void*,int,int);
using SortInfo=void(__fastcall*)(void*,void*,int);
using RowIndex=int(__fastcall*)(void*,int,void*);
Sort nativeSort=nullptr;SortInfo nativeSortInfo=nullptr;RowIndex nativeRowIndex=nullptr;
using Key=void(__fastcall*)(void*,void*,int,int);
using Init=void(__fastcall*)(void*,int,int,void*);
using Close=void(__fastcall*)(void*,int);
using Info=void(__fastcall*)(void*,void*);
using NavInfo=void(__fastcall*)(void*,void*,void*);
using NavRequest=void(__fastcall*)(void*,unsigned,const char*,void*);
using DisabledRow=bool(__fastcall*)(void*,int);
DisabledRow disabledRow=nullptr;
using DecodeReference=unsigned(__fastcall*)(void*);
using LookupPlayer=void*(__fastcall*)(void*,unsigned);
LookupPlayer lookupPlayer=nullptr;
using CardFactory=void*(__fastcall*)();
using CardObject=void*(__fastcall*)(void*,void**);
using CardBool=void(__fastcall*)(void*,const char*,bool);
using CardInt=void(__fastcall*)(void*,const char*,int);
using CardText=void(__fastcall*)(void*,const char*,const char*);
DecodeReference decodeReference=nullptr;CardFactory cardFactory=nullptr;CardObject cardObject=nullptr;
CardBool cardBool=nullptr;CardInt cardInt=nullptr;CardText cardText=nullptr;
std::atomic<unsigned> identitySamples{0},cardSamples{0},cardMissSamples{0};
using Encode=void(__fastcall*)(unsigned,void*,int*);
Encode encodePrefix=nullptr,encodeValue=nullptr,encodeSuffix=nullptr;
Columns columns=nullptr,sourceColumns=nullptr;Rows rows=nullptr;Count count=nullptr;Key key=nullptr;Init init=nullptr;Close close=nullptr;Info info=nullptr;NavInfo navInfo=nullptr;NavRequest navRequest=nullptr;
std::string recordKey(const std::string& value){
    auto colon=value.find(':');if(colon==std::string::npos)throw std::runtime_error("Invalid exported record key");
    unsigned table=std::stoul(value.substr(0,colon)),row=std::stoul(value.substr(colon+1));
    if(table>=32768||row>=131072)throw std::runtime_error("Record reference exceeds FTC bit fields");
    char result[9]{};sprintf_s(result,"%08X",(table<<17)|row);return result;
}
std::map<unsigned,int> rowLookup;
void indexRows(){rowLookup.clear();for(size_t i=0;i<page.at("rows").size();++i)rowLookup.emplace(static_cast<unsigned>(std::stoul(recordKey(page.at("rows").at(i).at("playerKey").get<std::string>()),nullptr,16)),static_cast<int>(i));}
int userTeam(const Json& dataset);
PreparedSnapshot prepareSnapshot(Json dataset){
    PreparedSnapshot result;result.dataset=std::move(dataset);const auto& input=result.dataset;
    std::map<int,Json> teamInfo;
    for(const auto& t:input.at("teams")){int index=t.at("index").get<int>();teamInfo[index]=t;if(!analytics::fcsTeam(t)&&index!=255)result.teams.emplace_back(index,t.at("name").get<std::string>());}
    std::sort(result.teams.begin(),result.teams.end(),[](const auto& a,const auto& z){return a.second<z.second;});
    for(const auto& p:input.at("roster"))if(p.contains("playerInfo")){
        Json card=p.at("playerInfo");auto t=teamInfo.find(p.at("teamIndex").get<int>());
        if(t!=teamInfo.end()){card["teamLogoId"]=t->second.value("logoId",0);card["primaryTeamColor"]=t->second.value("primaryColor",0);card["secondaryTeamColor"]=t->second.value("secondaryColor",0);}
        result.cards[std::stoul(recordKey(p.at("playerKey").get<std::string>()),nullptr,16)]=std::move(card);
    }
    std::set<std::tuple<int,int,std::string>> choices;
    for(const auto& g:input.at("games"))if(analytics::releasedWeek(input,g.at("SeasonYear").get<int>(),g.at("SeasonWeek").get<int>(),g.at("SeasonWeekType").get<std::string>()))choices.emplace(g.at("SeasonYear").get<int>(),g.at("SeasonWeek").get<int>(),g.at("SeasonWeekType").get<std::string>());
    const auto& season=input.at("season");
    result.weeks.push_back({season.at("CurrentYear").get<int>(),analytics::SeasonWeek,"Season"});
    for(const auto& [year,week,type]:choices)if(year==season.at("CurrentYear").get<int>())result.weeks.push_back({year,week,type});
    std::sort(result.weeks.begin()+1,result.weeks.end(),[](const auto& a,const auto& b){return std::make_tuple(a.year,analytics::stageOrder(a.type),a.week)<std::make_tuple(b.year,analytics::stageOrder(b.type),b.week);});
    int owner=userTeam(input);
    if(std::none_of(result.teams.begin(),result.teams.end(),[&](const auto& t){return t.first==owner&&owner!=analytics::NationalTeam;}))throw std::runtime_error("Save has no eligible user team");
    if(result.teams.empty()||result.weeks.empty())throw std::runtime_error("Refresh has no team/week choices");
    return result;
}
int userTeam(const Json& dataset){
    if(dataset.contains("userTeamIndex")&&dataset.at("userTeamIndex").is_number_integer())return dataset.at("userTeamIndex").get<int>();
    for(const auto& t:dataset.at("teams"))if(t.value("isUserManaged",false)&&!analytics::fcsTeam(t))return t.at("index").get<int>();
    throw std::runtime_error("Save has no user team; export updated team metadata");
}
void rebuildPage(){page=analytics::teamPage(data,selectedTeam,seasonYear,selectedWeek,selectedType);analytics::sortPage(page,sortColumn,sortDescending);indexRows();}
void startOnUserTeam(){
    std::lock_guard<std::mutex> lock(viewMutex);int wanted=userTeam(data);
    auto team=std::find_if(teams.begin(),teams.end(),[&](const auto& t){return t.first==wanted;});
    if(team==teams.end())throw std::runtime_error("User team is not an eligible analytics team");
    selectedTeam=team->first;teamName=team->second;sortColumn=5;sortDescending=true;hasUserSort=false;rebuildPage();
}
void useSnapshot(PreparedSnapshot snapshot){
    std::lock_guard<std::mutex> lock(viewMutex);
    auto team=std::find_if(snapshot.teams.begin(),snapshot.teams.end(),[](const auto& t){return t.first==selectedTeam;});
    if(team==snapshot.teams.end()){int wanted=userTeam(snapshot.dataset);team=std::find_if(snapshot.teams.begin(),snapshot.teams.end(),[&](const auto& t){return t.first==wanted;});}
    if(team==snapshot.teams.end())throw std::runtime_error("User team is not an eligible analytics team");
    int nextTeam=team->first;std::string nextName=team->second;
    const auto& latest=snapshot.weeks.back();int nextYear=latest.year,nextWeek=latest.week;auto nextType=latest.type;
    auto nextPage=analytics::teamPage(snapshot.dataset,nextTeam,nextYear,nextWeek,nextType);analytics::sortPage(nextPage,sortColumn,sortDescending);
    data=std::move(snapshot.dataset);playerCards=std::move(snapshot.cards);teams=std::move(snapshot.teams);weeks=std::move(snapshot.weeks);page=std::move(nextPage);
    selectedTeam=nextTeam;teamName=std::move(nextName);seasonYear=nextYear;selectedWeek=nextWeek;selectedType=std::move(nextType);indexRows();
}
void adoptSnapshot(){
    std::optional<PreparedSnapshot> ready;
    {std::lock_guard<std::mutex> lock(pendingMutex);ready.swap(nextSnapshot);}
    if(!ready)return;
    try{useSnapshot(std::move(*ready));note({{"event","save_refresh_applied"},{"source",data.at("sourceKind")},{"week",selectedWeek},{"team",selectedTeam}});}
    catch(const std::exception& e){note({{"event","analytics_snapshot_apply_failed"},{"reason",e.what()}});throw;}
}
template<class T>void set(void* out,unsigned row,const char* name,T value,size_t slot){reinterpret_cast<void(__fastcall*)(void*,unsigned,const char*,T)>((*reinterpret_cast<void***>(out))[slot])(out,row,name,value);}
void textField(void* out,unsigned row,const char* name,const std::string& v){set(out,row,name,v.c_str(),6);}
void objectText(void* out,const char* name,const char* value){reinterpret_cast<void(__fastcall*)(void*,const char*,const char*)>((*reinterpret_cast<void***>(out))[4])(out,name,value);}
bool owns(int id){return !disabled&&opened&&source==id;}
void __fastcall columnHook(void* object,void* out,int id){CallGuard cotCall;
    if(disabled||!opened||id!=0){columns(object,out,id);return;}
    const char* names[]={"FilterKey","Stat","Name","POS","SNAPS","PFF GRADE","CLASSIFICATION","TEAM"};
    const int widths[]={0,0,300,100,140,170,250,220};
    for(unsigned i=0;i<8;++i){textField(out,i,"display",names[i]);textField(out,i,"hintText",i==5?"Position-based estimate; completed games only":names[i]);set(out,i,"isHidden",i<2,3);set(out,i,"isStatic",false,3);set(out,i,"pixelWidth",widths[i],4);set(out,i,"formatType",0,4);}
    note({{"event","native_analytics_columns"},{"count",8}});
}
void __fastcall sourceColumnHook(void* object,void* out,int id){CallGuard cotCall;
    // Roster requests its schema before BeginUpdate can bind the row source.
    // Its verified primary source is zero; other sources retain native schema.
    if(!disabled&&opened&&id==0){columnHook(object,out,id);return;}
    sourceColumns(object,out,id);
}
std::string cell(const Json& player,int col){
    if(col==0){int rowTeam=player.value("teamIndex",selectedTeam);auto t=std::find_if(data.at("teams").begin(),data.at("teams").end(),[&](const Json& t){return t.at("index")==rowTeam;});return t==data.at("teams").end()?"":recordKey(t->at("key").get<std::string>());}if(col==1)return "0";
    if(col==2)return player.at("name").get<std::string>();
    if(col==3)return player.at("position").get<std::string>();
    if(col==4)return player.at("snaps").is_number()?std::to_string(player.at("snaps").get<int>()):"--";
    if(col==5){if(!player.at("grade").is_number())return "N/A";char b[24]{};sprintf_s(b,"%.1f",player.at("grade").get<double>());return b;}
    if(col==6)return player.at("grade").is_number()?analytics::classification(player.at("grade").get<double>()):"N/A";
    if(col==7)return player.value("teamName",std::string());return "";
}
bool __fastcall sortHook(void* object,int id,int column){CallGuard cotCall;
    if(!owns(id))return nativeSort(object,id,column);
    if(column<2||column>7)return false;
    {std::lock_guard<std::mutex> lock(viewMutex);
        // The first native refresh replays sort setup on card return. It is
        // not user input; keep the desired page/order until restore is read.
        if(!cardSelectionRestorePending&&!(preview.pending&&column==sortColumn)){
            invalidatePreviewLocked();sortDescending=hasUserSort&&column==sortColumn?!sortDescending:true;hasUserSort=true;sortColumn=column;analytics::sortPage(page,sortColumn,sortDescending);indexRows();}
    }
    // Preserve native refresh notifications using the donor's valid Name column.
    bool forwarded=nativeSort(object,id,2);
    note({{"event","native_analytics_sort"},{"column",column},{"descending",sortDescending},{"nativeResult",forwarded}});return true;
}
void __fastcall sortInfoHook(void* object,void* out,int id){CallGuard cotCall;
    if(!owns(id)){nativeSortInfo(object,out,id);return;}
    std::lock_guard<std::mutex> lock(viewMutex);
    reinterpret_cast<void(__fastcall*)(void*,int)>((*reinterpret_cast<void***>(out))[1])(out,1);
    // Native arrow values: 0 points down (descending), 1 points up (ascending).
    set(out,0,"key",sortColumn,4);set(out,0,"order",sortDescending?0:1,4);
}
int rowForReference(unsigned packed){auto found=rowLookup.find(packed);return found==rowLookup.end()?0:found->second;}
int __fastcall rowIndexHook(void* object,int id,void* arg){CallGuard cotCall;
    if(!owns(id))return nativeRowIndex(object,id,arg);
    auto text=argument(arg);struct Buffer{int capacity;char* value;bool allocated;}buffer{static_cast<int>(text.size()+1),text.data(),false};
    unsigned decoded=decodeReference(&buffer);
    std::lock_guard<std::mutex> lock(viewMutex);return rowForReference(decoded);
}
void __fastcall rowHook(void* object,void* out,int id,int row,void* cols){CallGuard cotCall;
    if(!owns(id)){rows(object,out,id,row,cols);return;}
    std::lock_guard<std::mutex> lock(viewMutex);
    if(row<0||static_cast<size_t>(row)>=page.at("rows").size())return;
    auto vt=*reinterpret_cast<void***>(cols);int n=reinterpret_cast<int(__fastcall*)(void*)>(vt[1])(cols);
    if(n<0||n>64)return;
    reinterpret_cast<void(__fastcall*)(void*,int)>((*reinterpret_cast<void***>(out))[1])(out,n);
    for(int i=0;i<n;++i){int col=reinterpret_cast<int(__fastcall*)(void*,int)>(vt[2])(cols,i);textField(out,static_cast<unsigned>(i),"display",cell(page.at("rows").at(row),col));}
}
int __fastcall countHook(void* object,int id){CallGuard cotCall;if(!owns(id))return count(object,id);std::lock_guard<std::mutex> lock(viewMutex);return static_cast<int>(page.at("rows").size());}
void __fastcall keyHook(void* object,void* out,int id,int row){CallGuard cotCall;
    if(!owns(id)){key(object,out,id,row);return;}
    std::lock_guard<std::mutex> lock(viewMutex);
    if(row<0||static_cast<size_t>(row)>=page.at("rows").size())return;
    auto packed=std::stoul(recordKey(page.at("rows").at(row).at("playerKey").get<std::string>()),nullptr,16);
    char buffer[256]{};struct NativeBuffer{int capacity;char* bytes;bool allocated;} destination{256,buffer,false};int written=0;
    encodePrefix(255,&destination,&written);
    encodeValue(packed,&destination,&written);
    encodeSuffix(255,&destination,&written);
    std::string k(buffer);
    if(identitySamples.fetch_add(1)<100)note({{"event","native_analytics_row_key"},{"row",row},{"reference",packed},{"name",page.at("rows").at(row).at("name")},{"encoded",std::to_string(k.size())}});
    reinterpret_cast<void(__fastcall*)(void*,const char*)>((*reinterpret_cast<void***>(out))[5])(out,k.c_str());
}
void __fastcall initHook(void* object,int id,int type,void* initialFilters){CallGuard cotCall;capturePreviewSource(object,id,type);if(!disabled&&opened&&id>=0&&id<64&&type==27)source=id;init(object,id,type,initialFilters);}
void __fastcall closeHook(void* object,int id){CallGuard cotCall;
    // Native Roster closes/rebuilds data sources while its view remains open.
    // Only the explicit UI shutdown command owns Analytics view lifetime.
    close(object,id);
}
bool __fastcall disabledRowHook(void* object,int row){CallGuard cotCall;if(disabled||!opened)return disabledRow(object,row);std::lock_guard<std::mutex> lock(viewMutex);return row<0||static_cast<size_t>(row)>=page.at("rows").size();}
// Private commands use the existing Fog-owned Roster GetEntryInfo callback.
// Ordinary player requests retain the full native Roster getter and Fog path.
// Command responses and isolated fixtures receive fresh, exact-player objects.
bool playerCard(void** out,void* arg,void*& result){
    auto text=argument(arg);
    if(text.rfind("COT_SELECT:",0)==0){
        // Only an explicit native row event supplies user selection. Getter
        // callbacks during a refresh are not evidence that the user moved.
        result=out;if(out)*out=nullptr;
        if(disabled||!nativeReady||!opened||!decodeReference||!currentProfile())return true;
        try{
            text.erase(0,11);if(text.empty())return true;
            struct Buffer{int capacity;char* value;bool allocated;}buffer{static_cast<int>(text.size()+1),text.data(),false};
            const unsigned reference=decodeReference(&buffer);
            std::lock_guard<std::mutex> lock(viewMutex);
            if(!currentProfile()||cardSelectionRestorePending||!rowLookup.count(reference))return true;
            // A new user row event in the still-open Analytics view also
            // retires a command ticket whose native action did not navigate.
            cardReturnPending=false;cardReturnReference.reset();
            preview.desiredRef=reference;if(preview.pending)preview.latestRef=reference;
        }catch(...){}
        return true;
    }
    if(text=="COT_LEAVE_VIEW"){
        // Reserved even after shutdown/disable: never send it to a native
        // player-key decoder or allocate a preview object during teardown.
        opened=false;source=-1;sourceTypes.fill(0);clearNativeFilters();clearPreview();
        {std::lock_guard<std::mutex> lock(viewMutex);
            if(!cardReturnPending){cardReturnReference.reset();cardSelectionRestorePending=false;}
        }
        result=out;if(out)*out=nullptr;return true;
    }
    if(text=="COT_RETURN_SELECTION"){
        // This private query is never a native player key, including when the
        // page/session is gone. Return a fresh object instead of donor data.
        int restoredRow=-1;const bool eligible=!disabled&&nativeReady&&opened&&currentProfile();
        {
            std::lock_guard<std::mutex> lock(viewMutex);
            if(!eligible){cardReturnPending=false;cardReturnReference.reset();cardSelectionRestorePending=false;preview=PreviewState{};++previewSequence;}
            else if(cardSelectionRestorePending){
                if(cardReturnReference){auto found=rowLookup.find(*cardReturnReference);if(found!=rowLookup.end()){restoredRow=found->second;preview.desiredRef=*cardReturnReference;}}
                cardSelectionRestorePending=false;cardReturnReference.reset();
            }else if(preview.pending&&previewReady(preview.roster,preview.cancelled?std::optional<unsigned>{}:preview.requestRef)){
                auto reference=preview.latestRef?preview.latestRef:preview.cancelled?std::optional<unsigned>{}:preview.requestRef;
                // A new team/week/sort view intentionally starts at its own
                // first row. This never substitutes row0 for a missing Back
                // ticket or an old view's player reference.
                if(preview.cancelled&&!reference)for(const auto& [packed,row]:rowLookup)if(row==0){reference=packed;break;}
                if(!preview.cancelled)preview.loadedTeam=preview.requestTeam;
                if(reference){auto found=rowLookup.find(*reference);if(found!=rowLookup.end()){restoredRow=found->second;preview.desiredRef=*reference;}}
                preview.pending=false;preview.cancelled=false;preview.requestRef.reset();preview.requestTeam.reset();preview.latestRef.reset();
            }
        }
        result=out;if(!out)return true;*out=nullptr;
        if(!nativeReady||!cardFactory||!cardObject||!cardInt)return true;
        try{auto* factory=cardFactory();if(!factory)return true;result=cardObject(factory,out);if(*out)cardInt(*out,"cotReturnRow",restoredRow);}
        catch(...){*out=nullptr;result=out;}
        return true;
    }
    const bool cardAction=text.rfind("COT_CARD:",0)==0;
    auto reject=[&](){if(!cardAction)return false;result=out;if(out)*out=nullptr;return true;};
    // Reserved commands must never become native Stats decoder inputs, even
    // when the caller races page/session teardown or supplies a bad key.
    if(cardAction)clearCardReturn();
    if(disabled||!nativeReady||!opened||!out)return reject();
    if(cardAction){if(!currentProfile())return reject();text.erase(0,9);}
    if(text.empty()){
        if(cardMissSamples.fetch_add(1)<8)note({{"event","native_analytics_card_deferred"},{"reason","unsupported or empty native player key"}});
        return reject();
    }
    auto buildCard=[&](){
    if(cardAction&&(!decodeReference||!cardFactory||!cardObject||!cardBool||!cardInt||!cardText))return reject();
    struct Buffer{int capacity;char* value;bool allocated;}buffer{static_cast<int>(text.size()+1),text.data(),false};
    const unsigned decoded=decodeReference(&buffer);
    std::lock_guard<std::mutex> lock(viewMutex);auto card=playerCards.find(decoded);
    if(!rowLookup.count(decoded)||card==playerCards.end()){
        if(cardMissSamples.fetch_add(1)<8)note({{"event","native_analytics_card_deferred"},{"reason","key is not in this Analytics view"},{"reference",decoded}});
        return reject();
    }
    if(cardAction&&(preview.pending||(preview.desiredRef&&preview.desiredRef!=decoded)))return reject();
    auto* factory=cardFactory();if(!factory){*out=nullptr;result=out;return true;}
    result=cardObject(factory,out);
    if(!*out)return true; // Preserve native allocation-failure semantics.
    for(const auto& [name,value]:card->second.items()){
        if(value.is_boolean())cardBool(*out,name.c_str(),value.get<bool>());
        else if(value.is_number_integer())cardInt(*out,name.c_str(),value.get<int>());
        else if(value.is_string())cardText(*out,name.c_str(),value.get_ref<const std::string&>().c_str());
    }
    if(cardSamples.fetch_add(1)<16)note({{"event","native_analytics_portrait"},{"decoded",decoded},{"name",card->second.value("firstName",std::string())+" "+card->second.value("lastName",std::string())}});
    if(cardAction){
        if(!currentProfile())return reject();
        cardReturnReference=decoded;cardSelectionRestorePending=false;cardReturnPending=true;
        cardBool(*out,"cotCardReady",true);
    }
    return true;
    };
    if(!cardAction)return buildCard();
    try{return buildCard();}catch(...){clearCardReturn();return reject();}
}
void __fastcall syncHook(void* object,int id,int type){CallGuard cotCall;capturePreviewSource(object,id,type);if(!disabled&&opened&&id>=0&&id<64){sourceTypes.at(id)=type;if(type==27)source=id;note({{"event","native_analytics_source"},{"id",id},{"type",type}});}syncInit(object,id,type);}
void __fastcall beginHook(void* object,int id,int type){CallGuard cotCall;capturePreviewSource(object,id,type);if(!disabled&&opened&&id>=0&&id<64){sourceTypes.at(id)=type;if(type==27)source=id;note({{"event","native_analytics_begin"},{"id",id},{"type",type}});}beginUpdate(object,id,type);}
struct FilterCapture{void** vt;void* target;Json fields=Json::array();std::map<unsigned,std::string> keys,labels;bool discard=false;};
void __fastcall filterReserve(FilterCapture* p,int n){if(!p->discard)reinterpret_cast<void(__fastcall*)(void*,int)>((*reinterpret_cast<void***>(p->target))[1])(p->target,n);}
template<class T>void filterPut(FilterCapture* p,unsigned row,const char* k,T value,size_t slot){if(!p->discard)reinterpret_cast<void(__fastcall*)(void*,unsigned,const char*,T)>((*reinterpret_cast<void***>(p->target))[slot])(p->target,row,k,value);}
void __fastcall filterBool(FilterCapture* p,unsigned row,const char* k,bool v){filterPut(p,row,k,v,3);}
void __fastcall filterInt(FilterCapture* p,unsigned row,const char* k,int v){filterPut(p,row,k,v,4);}
void __fastcall filterFloat(FilterCapture* p,unsigned row,const char* k,float v){filterPut(p,row,k,v,5);}
void __fastcall filterString(FilterCapture* p,unsigned row,const char* k,const char* v){filterPut(p,row,k,v,6);if(strcmp(k,"key")==0)p->keys[row]=v;if(strcmp(k,"display")==0)p->labels[row]=v;}
void* filterVt[]={nullptr,reinterpret_cast<void*>(filterReserve),nullptr,reinterpret_cast<void*>(filterBool),reinterpret_cast<void*>(filterInt),reinterpret_cast<void*>(filterFloat),reinterpret_cast<void*>(filterString)};
void __fastcall colHook(void* object,void* out,int id,int col){CallGuard cotCall;
    int type=id>=0&&id<64?sourceTypes.at(id):0;
    if(disabled||!opened||(type!=28&&type!=29)){colData(object,out,id,col);return;}
    FilterCapture proxy{filterVt,out};proxy.discard=true;colData(object,&proxy,id,col);
    std::lock_guard<std::mutex> lock(viewMutex);
    if(type==28){
        nativeTeamKeys.clear();
        // Filter keys encode menu ordinals, not TeamIndex. Match only exact,
        // unique captured native names; never infer an identity from the ordinal.
        std::map<std::string,std::set<int>> labels;
        for(const auto& team:data.at("teams")){
            const int index=team.at("index").get<int>();
            if(std::none_of(teams.begin(),teams.end(),[&](const auto& entry){return entry.first==index&&index!=analytics::NationalTeam;}))continue;
            auto addName=[&](const Json& name){if(name.is_string()&&!name.get_ref<const std::string&>().empty())labels[name.get<std::string>()].insert(index);};
            addName(team.at("name"));
            if(team.contains("nativeNames")&&team.at("nativeNames").is_array())for(const auto& name:team.at("nativeNames"))addName(name);
        }
        std::map<int,std::set<std::string>> candidates;
        std::map<std::string,std::set<int>> keyOwners;
        for(const auto& [row,value]:proxy.keys){
            if(value.empty()||value.rfind("ANALYTICS_",0)==0)continue;
            auto label=proxy.labels.find(row);if(label==proxy.labels.end())continue;
            // National Analytics was removed after live responsiveness testing.
            // Only exact, eligible team identities may reach the native query.
            auto matches=labels.find(label->second);
            if(label->second=="National"||matches==labels.end()||matches->second.size()!=1||!decodeReference)continue;
            try{struct Buffer{int capacity;const char* value;bool allocated;}buffer{static_cast<int>(value.size()+1),value.c_str(),false};(void)decodeReference(&buffer);}
            catch(...){continue;}
            const int index=*matches->second.begin();
            candidates[index].insert(value);keyOwners[value].insert(index);
        }
        for(const auto& [index,keys]:candidates)if(keys.size()==1&&keyOwners.at(*keys.begin()).size()==1)nativeTeamKeys.emplace(index,*keys.begin());
        reinterpret_cast<void(__fastcall*)(void*,int)>((*reinterpret_cast<void***>(out))[1])(out,static_cast<int>(teams.size()));
        for(size_t i=0;i<teams.size();++i){textField(out,static_cast<unsigned>(i),"display",teams[i].second);textField(out,static_cast<unsigned>(i),"key","ANALYTICS_TEAM:"+std::to_string(teams[i].first));set(out,static_cast<unsigned>(i),"isDefault",teams[i].first==selectedTeam,3);}
    }else{
        nativeWeekDefault.clear();
        // Analytics owns the week view; the native Roster cache must include
        // every position, independent of that selected week or Season view.
        for(const auto& [row,label]:proxy.labels)if(label=="Roster"&&proxy.keys.count(row)&&proxy.keys.at(row).rfind("ANALYTICS_",0)!=0)nativeWeekDefault=proxy.keys.at(row);
        reinterpret_cast<void(__fastcall*)(void*,int)>((*reinterpret_cast<void***>(out))[1])(out,static_cast<int>(weeks.size()));
        for(size_t i=0;i<weeks.size();++i){auto label=weeks[i].week==analytics::SeasonWeek?std::string("Season"):(weeks[i].type=="RegularSeason"?std::string():weeks[i].type+" ")+"Week "+std::to_string(weeks[i].week);textField(out,static_cast<unsigned>(i),"display",label);textField(out,static_cast<unsigned>(i),"key","ANALYTICS_WEEK:"+std::to_string(i));set(out,static_cast<unsigned>(i),"isDefault",weeks[i].year==seasonYear&&weeks[i].week==selectedWeek&&weeks[i].type==selectedType,3);}
    }
    note({{"event","native_analytics_filters"},{"type",type},{"choices",type==28?teams.size():weeks.size()}});
}
struct FilterAlias{void** vt;std::vector<std::string>* keys;};
int __fastcall filterSize(FilterAlias* p){return static_cast<int>(p->keys->size());}
void __fastcall filterRead(FilterAlias* p,int row,const char*,char* out,unsigned n){if(!out||!n)return;out[0]=0;if(row>=0&&static_cast<size_t>(row)<p->keys->size())strncpy_s(out,n,(*p->keys)[row].c_str(),_TRUNCATE);}
void* filterAliasVt[]={nullptr,reinterpret_cast<void*>(filterSize),nullptr,nullptr,nullptr,nullptr,reinterpret_cast<void*>(filterRead)};
std::vector<std::string> filterKeys(void* f){
    uintptr_t vt=0,sizeMethod=0,readMethod=0;
    if(!read(reinterpret_cast<uintptr_t>(f),&vt,8)||!read(vt+8,&sizeMethod,8)||!read(vt+0x30,&readMethod,8)||!sizeMethod||!readMethod)throw std::runtime_error("Unreadable Analytics filter array");
    const int n=reinterpret_cast<int(__fastcall*)(void*)>(sizeMethod)(f);
    if(n<0||n>8)throw std::runtime_error("Unsupported Analytics filter count");
    std::vector<std::string> keys;keys.reserve(static_cast<size_t>(n));
    for(int i=0;i<n;++i){
        char keyText[256]{};reinterpret_cast<void(__fastcall*)(void*,int,const char*,char*,int)>(readMethod)(f,i,"key",keyText,256);
        if(!memchr(keyText,0,sizeof(keyText)))throw std::runtime_error("Unterminated Analytics filter key");
        keys.emplace_back(keyText);
    }
    return keys;
}
bool filterNumber(std::string_view text,int& value,bool national=false){
    if(national&&text=="-1"){value=-1;return true;}
    if(text.empty())return false;
    int result=0;
    for(char ch:text){if(ch<'0'||ch>'9'||result>((std::numeric_limits<int>::max)()-(ch-'0'))/10)return false;result=result*10+(ch-'0');}
    value=result;return true;
}
struct FilterRequest{std::vector<std::string> keys;bool custom=false;};
FilterRequest applyFilters(void* f){
    FilterRequest request{filterKeys(f)};std::lock_guard<std::mutex> lock(viewMutex);
    int nextTeam=selectedTeam,nextYear=seasonYear,nextWeek=selectedWeek;std::string nextName=teamName,nextType=selectedType;
    bool teamSeen=false,weekSeen=false;
    for(auto& k:request.keys){
        if(k.rfind("ANALYTICS_TEAM:",0)==0){
            int id=0;if(teamSeen||!filterNumber(std::string_view(k).substr(15),id,true))throw std::runtime_error("Invalid Analytics team selection");teamSeen=true;
            auto team=std::find_if(teams.begin(),teams.end(),[&](const auto& t){return t.first==id;});
            auto nativeTeam=nativeTeamKeys.find(id);
            if(id==analytics::NationalTeam||team==teams.end()||nativeTeam==nativeTeamKeys.end())throw std::runtime_error("Unavailable Analytics team selection");
            nextTeam=id;nextName=team->second;k=nativeTeam->second;request.custom=true;
        }else if(k.rfind("ANALYTICS_WEEK:",0)==0){
            int choice=0;if(weekSeen||!filterNumber(std::string_view(k).substr(15),choice))throw std::runtime_error("Invalid Analytics week selection");weekSeen=true;
            if(static_cast<size_t>(choice)>=weeks.size()||nativeWeekDefault.empty()||nativeWeekDefault.rfind("ANALYTICS_",0)==0)throw std::runtime_error("Unavailable Analytics week selection");
            const auto& w=weeks[choice];nextYear=w.year;nextWeek=w.week;nextType=w.type;k=nativeWeekDefault;request.custom=true;
        }else if(k.rfind("ANALYTICS_",0)==0)throw std::runtime_error("Unknown Analytics filter key");
    }
    if(request.custom&&nativeWeekDefault.empty())throw std::runtime_error("Native all-position Roster filter is unavailable");
    if(nextTeam!=selectedTeam||nextYear!=seasonYear||nextWeek!=selectedWeek||nextType!=selectedType){
        // Build all throwing state before changing the visible selection. Native
        // refresh callbacks can reenter only after this complete page is ready.
        auto nextPage=analytics::teamPage(data,nextTeam,nextYear,nextWeek,nextType);analytics::sortPage(nextPage,sortColumn,sortDescending);
        std::map<unsigned,int> nextLookup;
        for(size_t i=0;i<nextPage.at("rows").size();++i)nextLookup.emplace(static_cast<unsigned>(std::stoul(recordKey(nextPage.at("rows").at(i).at("playerKey").get<std::string>()),nullptr,16)),static_cast<int>(i));
        page.swap(nextPage);rowLookup.swap(nextLookup);teamName.swap(nextName);selectedType.swap(nextType);selectedTeam=nextTeam;seasonYear=nextYear;selectedWeek=nextWeek;
        try{note({{"event","native_analytics_selection"},{"team",selectedTeam},{"week",selectedWeek},{"rows",page.at("rows").size()}});}catch(...){}
    }
    if(request.custom)invalidatePreviewLocked();
    return request;
}
bool ownsFilters(int id){return !disabled&&opened&&id>=0&&id<static_cast<int>(sourceTypes.size())&&(source==id||(source==-1&&sourceTypes[id]==27));}
std::atomic<unsigned> filterFailureSamples{0};
void filterFailure(int id,const char* reason)noexcept{try{if(filterFailureSamples.fetch_add(1)<16)note({{"event","native_analytics_filter_rejected"},{"source",id},{"reason",reason}});}catch(...) {}}
bool dispatchFilters(Filter native,void* object,int id,void* f){
    if(!ownsFilters(id))return native(object,id,f);
    try{
        auto request=applyFilters(f);if(!request.custom)return native(object,id,f);
        // Verified native 0x7ab0230 consumes slot1/slot6 synchronously; neither
        // sync nor async entry retains this alias or its backing strings.
        FilterAlias alias{filterAliasVt,&request.keys};return native(object,id,&alias);
    }catch(const std::exception& error){filterFailure(id,error.what());return false;}
    catch(...){filterFailure(id,"Unknown Analytics filter failure");return false;}
}
bool __fastcall filterHook(void* object,int id,void* f){CallGuard cotCall;return dispatchFilters(filter,object,id,f);}
bool __fastcall asyncFilterHook(void* object,int id,void* f){CallGuard cotCall;return dispatchFilters(asyncFilter,object,id,f);}
std::atomic<unsigned> previewFailureSamples{0};
void previewFailure(const char* reason)noexcept{try{if(previewFailureSamples.fetch_add(1)<16)note({{"event","native_analytics_preview_rejected"},{"reason",reason}});}catch(...) {}}
bool previewReady(void* rosterObject,std::optional<unsigned> reference)noexcept{
    if(!lookupPlayer||!currentProfile())return false;
    try{
        uintptr_t vt=0,flow=0,flowVt=0,contextVt=0,getContext=0,metadata=0;unsigned char ready=0;int loaded=0,pendingNative=0;
        const bool valid=read(reinterpret_cast<uintptr_t>(rosterObject),&vt,8)&&vt==base+0xbab18c0&&
            read(reinterpret_cast<uintptr_t>(rosterObject)+0x418,&flow,8)&&flow&&read(flow,&flowVt,8)&&flowVt==base+0xbd872e0&&
            read(flow+8,&contextVt,8)&&contextVt==base+0xbd873b0&&read(contextVt+8,&getContext,8)&&getContext==base+0x381b250&&
            read(flow+0xb1,&ready,1)&&ready&&read(flow+0xb4,&loaded,4)&&loaded>0&&read(flow+0xc4,&pendingNative,4)&&pendingNative<=0&&
            read(flow+0x140,&metadata,8)&&metadata;
        const bool present=valid&&(!reference||lookupPlayer(reinterpret_cast<void*>(flow),*reference)!=nullptr);
        return currentProfile()&&present;
    }catch(...){return false;}
}
void ensurePreview(void* rosterObject,void* arg)noexcept{
    CallGuard cotCall;
    if(disabled||!nativeReady||!opened||!decodeReference||!asyncFilter)return;
    if(!currentProfile()){clearPreview();return;}
    uint64_t requestId=0;unsigned decoded=0;void* provider=nullptr;std::vector<std::string> keys;
    try{
        auto text=argument(arg);if(text.empty()||text.rfind("COT_",0)==0)return;
        struct Buffer{int capacity;char* value;bool allocated;}buffer{static_cast<int>(text.size()+1),text.data(),false};decoded=decodeReference(&buffer);
        {
            std::lock_guard<std::mutex> lock(viewMutex);
            if(cardReturnPending||cardSelectionRestorePending)return;
            auto found=rowLookup.find(decoded);if(found==rowLookup.end()||found->second<0||static_cast<size_t>(found->second)>=page.at("rows").size())return;
            const auto& row=page.at("rows").at(found->second);
            if(static_cast<unsigned>(std::stoul(recordKey(row.at("playerKey").get<std::string>()),nullptr,16))!=decoded)return;
            if(preview.pending||(preview.desiredRef&&preview.desiredRef!=decoded))return;
            if(!previewReady(rosterObject))return;
            preview.roster=rosterObject;
            const int team=row.at("teamIndex").get<int>();
            auto nativeTeam=nativeTeamKeys.find(team);
            if(team==analytics::NationalTeam||nativeTeam==nativeTeamKeys.end()||nativeWeekDefault.empty()||nativeWeekDefault.rfind("ANALYTICS_",0)==0)return;
            if(!preview.desiredRef)preview.desiredRef=decoded;
            if(preview.lastRef!=decoded){preview.lastRef=decoded;preview.failedRef.reset();}
            if(preview.failedRef==decoded||previewReady(rosterObject,decoded))return;
            uintptr_t vt=0;provider=preview.provider;
            if(!provider||!read(reinterpret_cast<uintptr_t>(provider),&vt,8)||vt!=base+0xbacd668)return;
            keys={nativeTeam->second,nativeWeekDefault};
            preview.pending=true;preview.cancelled=false;preview.requestRef=decoded;preview.latestRef.reset();preview.requestTeam=team;preview.loadedTeam.reset();preview.requestId=requestId=++previewSequence;
        }
        // Never call native refresh while holding the view mutex. The native
        // completion notification may synchronously reenter rows/getters/query.
        FilterAlias alias{filterAliasVt,&keys};const bool accepted=asyncFilter(provider,0,&alias);
        if(accepted)return;
        {std::lock_guard<std::mutex> lock(viewMutex);if(preview.pending&&preview.requestId==requestId){if(!preview.cancelled)preview.failedRef=decoded;preview.pending=false;preview.cancelled=false;preview.requestRef.reset();preview.requestTeam.reset();preview.latestRef.reset();}}
        previewFailure("Native roster preview request was rejected");
    }catch(const std::exception& error){
        {std::lock_guard<std::mutex> lock(viewMutex);if(requestId&&preview.pending&&preview.requestId==requestId){if(!preview.cancelled)preview.failedRef=decoded;preview.pending=false;preview.cancelled=false;preview.requestRef.reset();preview.requestTeam.reset();preview.latestRef.reset();}}
        previewFailure(error.what());
    }catch(...){
        {std::lock_guard<std::mutex> lock(viewMutex);if(requestId&&preview.pending&&preview.requestId==requestId){if(!preview.cancelled)preview.failedRef=decoded;preview.pending=false;preview.cancelled=false;preview.requestRef.reset();preview.requestTeam.reset();preview.latestRef.reset();}}
        previewFailure("Unknown native preview failure");
    }
}
void __fastcall infoHook(void* object,void* out){CallGuard cotCall;info(object,out);if(!disabled&&opened){std::lock_guard<std::mutex> lock(viewMutex);objectText(out,"Title","Player Analytics");}}
struct Binding{uintptr_t vt,offset,rva;void* hook;void** slot=nullptr;};
Binding bindings[]={
 {0xbaa78b8,0x38,0x78edb60,reinterpret_cast<void*>(infoHook)},
 {0xbaa78b8,0x98,0x78f0a50,reinterpret_cast<void*>(disabledRowHook)},
 {0xbacd668,0xb0,0x7a89b50,reinterpret_cast<void*>(countHook)},
 {0xbacd668,0x78,0x7a89180,reinterpret_cast<void*>(keyHook)},
 {0xbacd668,0x80,0x7a892d0,reinterpret_cast<void*>(rowIndexHook)},
 {0xbacd668,0xa0,0x7a89880,reinterpret_cast<void*>(sortHook)},
 {0xbacd668,0xa8,0x7a899d0,reinterpret_cast<void*>(sortInfoHook)},
 {0xbacd668,0x58,0x7a88bd0,reinterpret_cast<void*>(initHook)},
 {0xbacd668,0x50,0x7a88b70,reinterpret_cast<void*>(syncHook)},
 {0xbacd668,0x38,0x7a88a60,reinterpret_cast<void*>(beginHook)},
 {0xbacd668,0x70,0x7a88e90,reinterpret_cast<void*>(colHook)},
 {0xbacd668,0x90,0x7a896a0,reinterpret_cast<void*>(filterHook)},
 {0xbacd668,0x98,0x7a89770,reinterpret_cast<void*>(asyncFilterHook)},
 {0xbacd668,0x88,0x7a89420,reinterpret_cast<void*>(sourceColumnHook)},};
bool stop(){
    nativeReady=false;bool ok=true;
    for(auto& x:bindings)if(x.slot){if(replace(x.hook,reinterpret_cast<void*>(base+x.rva),x.vt,x.offset))x.slot=nullptr;else ok=false;}
    return ok;
}
template<class Read>void validateContract(uintptr_t b,Read query){
    for(const auto& gate:analyticsGates){
        unsigned char bytes[32]{};if(!query(b+gate.rva,bytes,32))throw std::runtime_error("Unreadable native analytics helper");
        if(memcmp(bytes,gate.bytes,32))::cotcompat::practice::normalize(b,gate.rva,bytes,gate.bytes,32,query);
        if(memcmp(bytes,gate.bytes,32))throw std::runtime_error("Unsupported native analytics helper");
    }
    for(const auto& binding:bindings){uintptr_t value=0;if(!query(b+binding.vt+binding.offset,&value,8)||value!=b+binding.rva)throw std::runtime_error("Native analytics slot is already owned or unsupported");}
}
void install(uintptr_t b){
    validateContract(b,read);
    columns=reinterpret_cast<Columns>(b+0x78ef740);sourceColumns=reinterpret_cast<Columns>(b+0x7a89420);info=reinterpret_cast<Info>(b+0x78edb60);rows=reinterpret_cast<Rows>(b+0x7a88cd0);count=reinterpret_cast<Count>(b+0x7a89b50);key=reinterpret_cast<Key>(b+0x7a89180);init=reinterpret_cast<Init>(b+0x7a88bd0);close=reinterpret_cast<Close>(b+0x7a88c90);
    syncInit=reinterpret_cast<SyncInit>(b+0x7a88b70);beginUpdate=reinterpret_cast<SyncInit>(b+0x7a88a60);colData=reinterpret_cast<ColData>(b+0x7a88e90);filter=reinterpret_cast<Filter>(b+0x7a896a0);asyncFilter=reinterpret_cast<Filter>(b+0x7a89770);
    nativeSort=reinterpret_cast<Sort>(b+0x7a89880);nativeSortInfo=reinterpret_cast<SortInfo>(b+0x7a899d0);nativeRowIndex=reinterpret_cast<RowIndex>(b+0x7a892d0);
    encodePrefix=reinterpret_cast<Encode>(b+analyticsPrefixRva);encodeValue=reinterpret_cast<Encode>(b+0x8076180);encodeSuffix=reinterpret_cast<Encode>(b+0x8079e70);
    disabledRow=reinterpret_cast<DisabledRow>(b+0x78f0a50);decodeReference=reinterpret_cast<DecodeReference>(b+0x376f6d0);
    lookupPlayer=reinterpret_cast<LookupPlayer>(b+0x822c030);
    cardFactory=reinterpret_cast<CardFactory>(b+0x3f0a1d0);cardObject=reinterpret_cast<CardObject>(b+0x8aec980);
    cardBool=reinterpret_cast<CardBool>(b+0x2b07670);cardInt=reinterpret_cast<CardInt>(b+0x2b075c0);cardText=reinterpret_cast<CardText>(b+0x25a3e60);
    try{for(auto& x:bindings){if(!replace(reinterpret_cast<void*>(b+x.rva),x.hook,x.vt,x.offset))throw std::runtime_error("Native analytics ownership changed");x.slot=reinterpret_cast<void**>(b+x.vt+x.offset);}}
    catch(...){stop();throw;}
    nativeReady=true;note({{"event","native_analytics_active"},{"sharedHookOwners","existing COT dispatchers"},{"capture","active native Dynasty; on demand"}});
}
}
