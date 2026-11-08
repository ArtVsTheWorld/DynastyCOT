#pragma once
// Included inside the existing COT Fog owner after its common helpers.
#include "AnalyticsGates.h"
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


std::atomic<bool> opened{false},intent{false};std::atomic<int> source{-1};
Json data,page;
std::map<unsigned,Json> playerCards;
int seasonYear=0,selectedTeam=-2,selectedWeek=1,sortColumn=5;
bool sortDescending=true,hasUserSort=false;
std::string selectedType="RegularSeason",teamName;
std::mutex viewMutex;
std::array<int,64> sourceTypes{};
using SyncInit=void(__fastcall*)(void*,int,int);
using ColData=void(__fastcall*)(void*,void*,int,int);
using Filter=bool(__fastcall*)(void*,int,void*);
SyncInit syncInit=nullptr,beginUpdate=nullptr;ColData colData=nullptr;Filter filter=nullptr,asyncFilter=nullptr;
std::string nativeTeamDefault,nativeWeekDefault;
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
using PlayerInfo=void*(__fastcall*)(void*,void**,void*);
using DisabledRow=bool(__fastcall*)(void*,int);
PlayerInfo playerInfo=nullptr;DisabledRow disabledRow=nullptr;
std::atomic<unsigned> identitySamples{0};
using Encode=void(__fastcall*)(unsigned,void*,int*);
Encode encodePrefix=nullptr,encodeValue=nullptr,encodeSuffix=nullptr;
Columns columns=nullptr;Rows rows=nullptr;Count count=nullptr;Key key=nullptr;Init init=nullptr;Close close=nullptr;Info info=nullptr;NavInfo navInfo=nullptr;NavRequest navRequest=nullptr;
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
    result.teams.insert(result.teams.begin(),{analytics::NationalTeam,"National"});
    for(const auto& p:input.at("roster"))if(p.contains("playerInfo")){
        Json card=p.at("playerInfo");auto t=teamInfo.find(p.at("teamIndex").get<int>());
        if(t!=teamInfo.end()){card["teamLogoId"]=t->second.value("logoId",0);card["primaryTeamColor"]=t->second.value("primaryColor",0);card["secondaryTeamColor"]=t->second.value("secondaryColor",0);}
        result.cards[std::stoul(recordKey(p.at("playerKey").get<std::string>()),nullptr,16)]=std::move(card);
    }
    std::set<std::tuple<int,int,std::string>> choices;
    for(const auto& g:input.at("games"))choices.emplace(g.at("SeasonYear").get<int>(),g.at("SeasonWeek").get<int>(),g.at("SeasonWeekType").get<std::string>());
    const auto& season=input.at("season");choices.emplace(season.at("CurrentYear").get<int>(),season.at("CurrentWeek").get<int>(),season.at("CurrentWeekType").get<std::string>());
    result.weeks.push_back({season.at("CurrentYear").get<int>(),analytics::SeasonWeek,"Season"});
    for(const auto& [year,week,type]:choices)result.weeks.push_back({year,week,type});
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
    const auto& season=snapshot.dataset.at("season");int nextYear=season.at("CurrentYear").get<int>(),nextWeek=season.at("CurrentWeek").get<int>();auto nextType=season.at("CurrentWeekType").get<std::string>();
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
    for(unsigned i=0;i<8;++i){textField(out,i,"display",names[i]);textField(out,i,"hintText",i==5?"Production grade; Season averages graded games":names[i]);set(out,i,"isHidden",i<2,3);set(out,i,"isStatic",false,3);set(out,i,"pixelWidth",widths[i],4);set(out,i,"formatType",0,4);}
    note({{"event","native_analytics_columns"},{"count",8}});
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
    {std::lock_guard<std::mutex> lock(viewMutex);sortDescending=hasUserSort&&column==sortColumn?!sortDescending:true;hasUserSort=true;sortColumn=column;analytics::sortPage(page,sortColumn,sortDescending);indexRows();}
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
    unsigned decoded=reinterpret_cast<unsigned(__fastcall*)(void*)>(reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr))+0x376f6d0)(&buffer);
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
void __fastcall initHook(void* object,int id,int type,void* initialFilters){CallGuard cotCall;if(opened&&type==27)source=id;init(object,id,type,initialFilters);}
void __fastcall closeHook(void* object,int id){CallGuard cotCall;close(object,id);if(source==id){source=-1;opened=false;}}
bool __fastcall disabledRowHook(void* object,int row){CallGuard cotCall;if(disabled||!opened)return disabledRow(object,row);std::lock_guard<std::mutex> lock(viewMutex);return row<0||static_cast<size_t>(row)>=page.at("rows").size();}
void* __fastcall playerInfoHook(void* object,void** out,void* arg){CallGuard cotCall;
    void* result=playerInfo(object,out,arg);
    if(!disabled&&opened&&out&&*out){auto text=argument(arg);struct Buffer{int capacity;char* value;bool allocated;}buffer{static_cast<int>(text.size()+1),text.data(),false};auto b=reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));unsigned decoded=reinterpret_cast<unsigned(__fastcall*)(void*)>(b+0x376f6d0)(&buffer);
        std::lock_guard<std::mutex> lock(viewMutex);auto card=playerCards.find(decoded);if(card!=playerCards.end()){
            for(const auto& [name,value]:card->second.items()){
                if(value.is_boolean())reinterpret_cast<void(__fastcall*)(void*,const char*,bool)>(b+0x2b07670)(*out,name.c_str(),value.get<bool>());
                else if(value.is_number_integer())reinterpret_cast<void(__fastcall*)(void*,const char*,int)>(b+0x2b075c0)(*out,name.c_str(),value.get<int>());
                else if(value.is_string())reinterpret_cast<void(__fastcall*)(void*,const char*,const char*)>(b+0x25a3e60)(*out,name.c_str(),value.get_ref<const std::string&>().c_str());
            }
            note({{"event","native_analytics_portrait"},{"decoded",decoded},{"name",card->second.value("firstName",std::string())+" "+card->second.value("lastName",std::string())}});
        }
    }
    return result;
}
void __fastcall syncHook(void* object,int id,int type){CallGuard cotCall;if(opened&&id>=0&&id<64){sourceTypes.at(id)=type;note({{"event","native_analytics_source"},{"id",id},{"type",type}});}syncInit(object,id,type);}
void __fastcall beginHook(void* object,int id,int type){CallGuard cotCall;if(opened&&id>=0&&id<64){sourceTypes.at(id)=type;note({{"event","native_analytics_begin"},{"id",id},{"type",type}});}beginUpdate(object,id,type);}
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
        for(const auto& [row,label]:proxy.labels)if(label=="Buffalo"&&proxy.keys.count(row))nativeTeamDefault=proxy.keys.at(row);
        if(nativeTeamDefault.empty()&&!proxy.keys.empty())nativeTeamDefault=proxy.keys.begin()->second;
        reinterpret_cast<void(__fastcall*)(void*,int)>((*reinterpret_cast<void***>(out))[1])(out,static_cast<int>(teams.size()));
        for(size_t i=0;i<teams.size();++i){textField(out,static_cast<unsigned>(i),"display",teams[i].second);textField(out,static_cast<unsigned>(i),"key","ANALYTICS_TEAM:"+std::to_string(teams[i].first));set(out,static_cast<unsigned>(i),"isDefault",teams[i].first==selectedTeam,3);}
    }else{
        if(!proxy.keys.empty())nativeWeekDefault=proxy.keys.begin()->second;
        reinterpret_cast<void(__fastcall*)(void*,int)>((*reinterpret_cast<void***>(out))[1])(out,static_cast<int>(weeks.size()));
        for(size_t i=0;i<weeks.size();++i){auto label=weeks[i].week==analytics::SeasonWeek?std::string("Season"):(weeks[i].type=="RegularSeason"?std::string():weeks[i].type+" ")+"Week "+std::to_string(weeks[i].week);textField(out,static_cast<unsigned>(i),"display",label);textField(out,static_cast<unsigned>(i),"key","ANALYTICS_WEEK:"+std::to_string(i));set(out,static_cast<unsigned>(i),"isDefault",weeks[i].year==seasonYear&&weeks[i].week==selectedWeek&&weeks[i].type==selectedType,3);}
    }
    note({{"event","native_analytics_filters"},{"type",type},{"choices",type==28?teams.size():weeks.size()}});
}
struct FilterAlias{void** vt;std::vector<std::string>* keys;};
int __fastcall filterSize(FilterAlias* p){return static_cast<int>(p->keys->size());}
void __fastcall filterRead(FilterAlias* p,int row,const char*,char* out,unsigned n){if(n)strncpy_s(out,n,p->keys->at(row).c_str(),_TRUNCATE);}
void* filterAliasVt[]={nullptr,reinterpret_cast<void*>(filterSize),nullptr,nullptr,nullptr,nullptr,reinterpret_cast<void*>(filterRead)};
std::vector<std::string> filterKeys(void* f){std::vector<std::string> keys;auto vt=*reinterpret_cast<void***>(f);int n=reinterpret_cast<int(__fastcall*)(void*)>(vt[1])(f);for(int i=0;i<n&&i<8;++i){char keyText[256]{};reinterpret_cast<void(__fastcall*)(void*,int,const char*,char*,int)>(vt[6])(f,i,"key",keyText,256);keys.emplace_back(keyText);}return keys;}
std::vector<std::string> applyFilters(void* f){
    auto keys=filterKeys(f);bool changed=false;std::lock_guard<std::mutex> lock(viewMutex);
    for(auto& k:keys){
        if(k.rfind("ANALYTICS_TEAM:",0)==0){int id=std::stoi(k.substr(15));auto team=std::find_if(teams.begin(),teams.end(),[&](const auto& t){return t.first==id;});if(team!=teams.end()){selectedTeam=id;teamName=team->second;changed=true;}k=nativeTeamDefault;}
        else if(k.rfind("ANALYTICS_WEEK:",0)==0){auto choice=static_cast<size_t>(std::stoul(k.substr(15)));if(choice<weeks.size()){const auto& w=weeks[choice];seasonYear=w.year;selectedWeek=w.week;selectedType=w.type;changed=true;}k=nativeWeekDefault;}
    }
    if(changed){rebuildPage();note({{"event","native_analytics_selection"},{"team",selectedTeam},{"week",selectedWeek},{"rows",page.at("rows").size()}});}
    return keys;
}
bool __fastcall filterHook(void* object,int id,void* f){CallGuard cotCall;if(disabled||!opened)return filter(object,id,f);auto keys=applyFilters(f);FilterAlias alias{filterAliasVt,&keys};return filter(object,id,&alias);}
bool __fastcall asyncFilterHook(void* object,int id,void* f){CallGuard cotCall;if(disabled||!opened)return asyncFilter(object,id,f);auto keys=applyFilters(f);FilterAlias alias{filterAliasVt,&keys};return asyncFilter(object,id,&alias);}
void __fastcall infoHook(void* object,void* out){CallGuard cotCall;info(object,out);if(!disabled&&opened){std::lock_guard<std::mutex> lock(viewMutex);objectText(out,"Title","Player Analytics");}}
struct Binding{uintptr_t vt,offset,rva;void* hook;void** slot=nullptr;};
Binding bindings[]={
 {0xbaa78b8,0x38,0x78edb60,reinterpret_cast<void*>(infoHook)},
 {0xbaa78b8,0x98,0x78f0a50,reinterpret_cast<void*>(disabledRowHook)},
 {0xbab13f0,0x50,0x79411c0,reinterpret_cast<void*>(playerInfoHook)},
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
 {0xbacd668,0x98,0x7a89770,reinterpret_cast<void*>(asyncFilterHook)},};
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
    columns=reinterpret_cast<Columns>(b+0x78ef740);info=reinterpret_cast<Info>(b+0x78edb60);rows=reinterpret_cast<Rows>(b+0x7a88cd0);count=reinterpret_cast<Count>(b+0x7a89b50);key=reinterpret_cast<Key>(b+0x7a89180);init=reinterpret_cast<Init>(b+0x7a88bd0);close=reinterpret_cast<Close>(b+0x7a88c90);
    syncInit=reinterpret_cast<SyncInit>(b+0x7a88b70);beginUpdate=reinterpret_cast<SyncInit>(b+0x7a88a60);colData=reinterpret_cast<ColData>(b+0x7a88e90);filter=reinterpret_cast<Filter>(b+0x7a896a0);asyncFilter=reinterpret_cast<Filter>(b+0x7a89770);
    nativeSort=reinterpret_cast<Sort>(b+0x7a89880);nativeSortInfo=reinterpret_cast<SortInfo>(b+0x7a899d0);nativeRowIndex=reinterpret_cast<RowIndex>(b+0x7a892d0);
    encodePrefix=reinterpret_cast<Encode>(b+0x8076300);encodeValue=reinterpret_cast<Encode>(b+0x8076180);encodeSuffix=reinterpret_cast<Encode>(b+0x8079e70);
    playerInfo=reinterpret_cast<PlayerInfo>(b+0x79411c0);disabledRow=reinterpret_cast<DisabledRow>(b+0x78f0a50);
    try{for(auto& x:bindings){if(!replace(reinterpret_cast<void*>(b+x.rva),x.hook,x.vt,x.offset))throw std::runtime_error("Native analytics ownership changed");x.slot=reinterpret_cast<void**>(b+x.vt+x.offset);}}
    catch(...){stop();throw;}
    nativeReady=true;note({{"event","native_analytics_active"},{"sharedHookOwners","existing COT dispatchers"},{"capture","active native Dynasty; on demand"}});
}
}
