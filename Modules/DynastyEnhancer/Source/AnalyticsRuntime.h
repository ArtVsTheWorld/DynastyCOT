#pragma once
// Included inside recruitingfog::runtime, beside its single native hook owner.
#include "AnalyticsNative.h"
namespace nativeanalytics {
std::atomic<bool> captureRequested{false},snapshotReady{false};
std::atomic<uint64_t> requestAfter{0};
std::atomic<uint32_t> snapshotLeague{0};
std::atomic<uintptr_t> snapshotCareer{0},snapshotDatabase{0};
dynasty::live::PackedChangeWatch captureWatch;
bool currentProfile(){
    try{
        auto query=[](uintptr_t at){uintptr_t value=0;if(!read(at,&value,8))throw std::runtime_error("Dynasty identity unavailable");return value;};
        const auto active=dynasty::live::activeSession(base,query);uint32_t league=0;
        return read(base+0xf55c998,&league,4)&&league==snapshotLeague&&active.career==snapshotCareer&&active.database==snapshotDatabase&&dynasty::live::activeSession(base,query)==active;
    }catch(...){return false;}
}
bool supported(){return !disabled&&nativeReady;}
bool available(){return !disabled&&nativeReady&&snapshotReady&&currentProfile();}
void request(){
    if(disabled||!nativeReady||opened)return;
    auto now=GetTickCount64(),after=requestAfter.load();if(now<after||!requestAfter.compare_exchange_strong(after,now+1000))return;
    snapshotReady=false;captureRequested=true;
}
void leave(){opened=false;clearCardReturn();clearNativeFilters();clearPreview();source=-1;sourceTypes.fill(0);}
bool resumeCard(){
    if(!cardReturnPending.exchange(false))return false;
    if(!available()){clearCardReturn();clearNativeFilters();clearPreview();return false;}
    clearPreview();std::lock_guard<std::mutex> lock(viewMutex);
    if(!cardReturnReference){cardSelectionRestorePending=false;return false;}
    cardSelectionRestorePending=true;
    source=-1;sourceTypes.fill(0);nativeTeamKeys.clear();nativeWeekDefault.clear();opened=true;return true;
}
bool open(){
    CallGuard cotCall;clearCardReturn();clearNativeFilters();clearPreview();if(!available())return false;
    try{adoptSnapshot();if(!currentProfile()||data.value("leagueId",Json())!=Json(snapshotLeague.load()))return false;startOnUserTeam();source=-1;sourceTypes.fill(0);opened=true;return true;}
    catch(const std::exception& error){leave();snapshotReady=false;note({{"event","native_analytics_open_deferred"},{"reason",error.what()}});return false;}
}
void tick(){
    if(disabled||!nativeReady||opened||!captureRequested.exchange(false))return;
    static std::string previousError;const auto started=GetTickCount64();
    try{
        if(currentProfile()&&captureWatch.available()&&captureWatch.unchanged(read)){snapshotReady=!captureRequested;return;}
        auto query=[](uintptr_t at){uintptr_t value=0;if(!read(at,&value,8))throw std::runtime_error("Dynasty identity unavailable");return value;};
        const auto binding=dynasty::live::activeSession(base,query);
        dynasty::live::PackedChangeWatch observed;
        auto raw=dynasty::live::Capture(base,GetCurrentProcess(),analytics::live::fields,analytics::live::roots).run(&observed);
        auto dataset=analytics::live::dataset(raw);const auto league=dataset.at("leagueId").get<uint32_t>();auto prepared=prepareSnapshot(std::move(dataset));
        uint32_t activeLeague=0;if(disabled||!read(base+0xf55c998,&activeLeague,4)||activeLeague!=league||!(dynasty::live::activeSession(base,query)==binding)||!observed.unchanged(read))throw std::runtime_error("Dynasty changed during analytics preparation");
        const auto rowCount=prepared.dataset.at("rows").size(),issues=prepared.dataset.at("issues").size();
        {std::lock_guard<std::mutex> lock(pendingMutex);nextSnapshot=std::move(prepared);}
        captureWatch=std::move(observed);snapshotLeague=league;snapshotCareer=binding.career;snapshotDatabase=binding.database;snapshotReady=!captureRequested;
        note({{"event","native_analytics_prepared"},{"league",league},{"rows",rowCount},{"issues",issues},{"milliseconds",GetTickCount64()-started}});previousError.clear();
    }catch(const std::exception& error){
        snapshotReady=false;if(previousError!=error.what()){previousError=error.what();note({{"event","native_analytics_capture_deferred"},{"reason",previousError}});}
        // A later menu request retries. No stale save or other Dynasty fallback.
    }
}
const cotanalytics::Api api={sizeof(cotanalytics::Api),cotanalytics::abi,cotanalytics::buildId,supported,available,open,request,leave,resumeCard};
}
