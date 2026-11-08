#pragma once
// Included in the Social native owner after its common UI helpers.
namespace analyticsmenu {
const cotanalytics::Api* api=nullptr;
uint64_t nextLookup=0;
bool admitted=false;
std::filesystem::path sibling(const std::filesystem::path& own){
    auto name=own.filename().wstring();const std::wstring prefix=L"DynastyEnhancerNative-cot-";
    // Packaged payloads retain stable compatibility filenames. Development
    // builds use timestamped names; both must also match the compiled build ID.
    if(name==L"DynastyEnhancerNative-v103.dll")return own.parent_path()/L"DynastyEnhancerRecruitFog-v1.dll";
    if(name.rfind(prefix,0)==0&&own.extension()==L".dll")return own.parent_path()/(L"DynastyEnhancerRecruitFog-p"+name.substr(prefix.size()));
    return {};
}
const cotanalytics::Api* resolve(){
    if(api)return api;if(GetTickCount64()<nextLookup)return nullptr;nextLookup=GetTickCount64()+1000;
    if(!module)return nullptr;
    wchar_t ownPath[32768]{};if(!GetModuleFileNameW(module,ownPath,32768))return nullptr;
    auto expected=sibling(std::filesystem::path(ownPath));if(expected.empty()||!cotanalytics::buildId)return nullptr;
    auto handle=GetModuleHandleW(expected.c_str());wchar_t actual[32768]{};
    if(!handle||!GetModuleFileNameW(handle,actual,32768)||_wcsicmp(expected.c_str(),actual))return nullptr;
    auto get=reinterpret_cast<cotanalytics::GetApi>(GetProcAddress(handle,"DynastyCotAnalyticsApi"));if(!get)return nullptr;
    const auto* value=get();HMODULE owner=nullptr;
    if(!value||!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(value),&owner)||owner!=handle||!cotanalytics::compatible(*value))return nullptr;
    for(auto address:{reinterpret_cast<uintptr_t>(value->supported),reinterpret_cast<uintptr_t>(value->available),reinterpret_cast<uintptr_t>(value->open),reinterpret_cast<uintptr_t>(value->request),reinterpret_cast<uintptr_t>(value->leave)})
        if(!address||!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(address),&owner)||owner!=handle)return nullptr;
    // The host already verifies each DLL's manifest hash before loading it.
    // Only the exact sibling from this build may service the shared UI.
    HMODULE pinned=nullptr;if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(get),&pinned))return nullptr;
    return api=value;
}
bool append(ContainerProxy& proxy){
    // Recognize the accepted Stats & Records grid before inserting a row.
    // A foreign layout is replayed unchanged; never renumber unknown controls.
    if(proxy.captured.size()<6||proxy.captured.size()>12)return false;
    unsigned index=0;for(const auto& [row,fields]:proxy.captured)if(row!=index++||fields.value("tileCoords",std::string())!=std::to_string(row+1))return false;
    for(const auto& [row,fields]:proxy.captured){forwardContainerFields(proxy.target,row>=5?row+1:row,fields,row>=5?std::to_string(row+2).c_str():nullptr);if(row==4)forwardContainerFields(proxy.target,5,fields,"6");}
    return true;
}
bool containersFor(void* self,void* panel,void* out,const std::string& name){
    if(name!="CM_LEAGUE"||disabled)return false;auto bridge=resolve();admitted=false;if(!bridge||!bridge->supported())return false;
    bridge->leave();bridge->request();ContainerProxy proxy{containerVt,out,{},true};containers(self,panel,&proxy);admitted=append(proxy);
    if(!admitted){for(const auto& [row,fields]:proxy.captured)forwardContainerFields(out,row,fields);note({{"event","analytics_menu_layout_unrecognized"},{"rows",proxy.captured.size()}});}
    return true;
}
bool tilesFor(void* self,void** out,void* panel,const std::string& p,const std::string& c,void*& result){
    if(disabled||p!="CM_LEAGUE"||!admitted)return false;
    unsigned coordinate=0;auto parsed=std::from_chars(c.data(),c.data()+c.size(),coordinate);if(parsed.ec!=std::errc()||parsed.ptr!=c.data()+c.size()||coordinate<6||coordinate>13)return false;
    auto original=std::to_string(coordinate-1);Alias alias{aliasVt,original.c_str()};result=tiles(self,out,panel,&alias);uintptr_t span[2]{};
    if(!out||!*out||!read(reinterpret_cast<uintptr_t>(*out)+0x18,span,16)||span[1]<span[0]||span[1]-span[0]>128)return true;
    for(uintptr_t at=span[0];at<span[1];at+=8){void* tile=nullptr;if(!read(at,&tile,8)||!tile)continue;tileText(tile,"Coordinates",c.c_str());
        if(coordinate==6){tileText(tile,"Id","PLAYER_ANALYTICS");tileText(tile,"Text0","Player Analytics");tileText(tile,"Text1",api&&api->available()?"PFF-style production grades, snaps and season averages":"Preparing current Dynasty grades - please wait");tileText(tile,"NavigationId","PLAYER_ANALYTICS");tileBool(tile,"isInteractive",true);}
    }return true;
}
}
