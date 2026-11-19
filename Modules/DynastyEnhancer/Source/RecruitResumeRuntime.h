#pragma once
#include "../../../Core/NativeFileIO.h"
#include "LiveTables.h"
#include "RecruitResumeCaptureFields.h"
#include "RecruitResumeSource.h"
#include <atomic>
#include <array>
#include <chrono>
#include <mutex>
#include "ScoutingEvidence.h"
namespace recruitingfog::resume {
struct Published {dynasty::live::SessionBinding binding;NativeClass data;uint64_t capturedAt=0;dynasty::live::PackedChangeWatch annualIdentity;bool transfers=false,backgroundReady=true;evidence::Context staff;Json reports=Json::object();};

inline measurables::Inputs notesMeasurementInputs(const NotesProspect& prospect,const NativeEntry* verifiedEntry){
    using namespace measurables;
    Inputs inputs;
    std::array<bool,InputCount> concealed{};
    for(const auto& observation:prospect.observations){
        const auto name=notesKey(observation.name);
        const auto input=std::find(inputNames.begin(),inputNames.end(),name);
        if(input==inputNames.end())continue;
        const auto index=static_cast<size_t>(input-inputNames.begin());
        if(!observation.exact)concealed[index]=true;
        else if(observation.low>=1&&valid(observation.low))inputs[index]=observation.low;
    }
    // Full scouting unlocks measurements from verified typed ratings. This is
    // Notes-only: do not add archetype omissions to the Scouting DTO or report.
    if(prospect.progress==100&&verifiedEntry){
        for(size_t index=0;index<inputs.size();++index){
            if(inputs[index]||concealed[index])continue;
            const auto rating=verifiedEntry->prospect.ratings.find(nativeFields[index]);
            if(rating!=verifiedEntry->prospect.ratings.end()&&rating->second>=1&&valid(rating->second)&&std::floor(rating->second)==rating->second)
                inputs[index]=rating->second;
        }
    }
    return inputs;
}

inline void appendMeasurements(std::string& text,const NotesProspect& prospect,uint32_t league,const NativeEntry* verifiedEntry=nullptr){
    using namespace measurables;
    const auto inputs=notesMeasurementInputs(prospect,verifiedEntry);
    const auto identity=seedIdentity(league,prospect.measurementIdentity);
    bool started=false;
    for(Event event:{Event::Dash,Event::Split,Event::Bench,Event::Cone,Event::Shuttle,Event::Vertical}){
        if(!ratingFor(event,inputs))continue;
        if(!started){
            if(!text.empty())text+="\n\n";
            text+="ATHLETIC MEASURABLES";
            started=true;
        }
        const auto result=estimate(event,inputs,identity);
        text+="\n"+result.label+": "+result.value;
    }
}
class Cache {
    friend struct CacheTestAccess;
    std::shared_ptr<const Published> published;std::atomic<bool> requested{true};uint64_t nextAttempt=0;
    struct ReportView {std::array<std::string,3> scouting;std::string background;};
    struct ReportEdition {std::string league;int season=0;evidence::Context staff;Json reports=Json::object();std::map<std::string,ReportView> views;};
    static void prepareView(ReportEdition& edition,const std::string& identity){
        const auto& value=edition.reports.at(identity);
        edition.views[identity]={{evidence::render(value,25,false),evidence::render(value,50,false),evidence::render(value,75,false)},evidence::storedBackground(value)};
    }
    std::shared_ptr<const ReportEdition> reportEdition;
    std::mutex reportMutex;std::map<std::string,NotesProspect> pendingReports;
    struct Wait {uint64_t first=0,last=0;};std::map<std::string,Wait> waitingReports;
    std::atomic<bool> backgroundFailed{false},reportFailed{false};std::atomic<uint64_t> waitingSince{0};uint64_t nextIdentityCheck=0;
    std::string lastFailure;unsigned failures=0;dynasty::live::SessionBinding attemptedBinding{};
    template<class Read>Json reportTick(const std::shared_ptr<const Published>& prior,const std::filesystem::path& folder,uintptr_t image,Read read){
        std::map<std::string,NotesProspect> pending;{std::lock_guard<std::mutex> lock(reportMutex);pending.swap(pendingReports);}
        if(pending.empty()||!prior)return nullptr;
        auto oldReports=std::atomic_load(&reportEdition);auto next=std::make_shared<ReportEdition>();next->league=prior->data.league;next->season=prior->data.season;if(oldReports&&oldReports->league==next->league&&oldReports->season==next->season){next->reports=oldReports->reports;next->views=oldReports->views;}bool changed=false;
        // Small staff/pipeline capture only when new evidence was requested.
        // Existing reports are immutable; idle menus never poll talent tables.
        Json raw;bool pipelineUnavailable=false;
        try{raw=dynasty::live::Capture(image,GetCurrentProcess(),dynasty::live::resumePipelineFields,[](const std::string& n){return n=="Franchise"||n=="SeasonInfo"||n=="FranchiseServer_CareerFlow"||n=="FranchiseUser";}).run();}
        catch(...){
            // Missing optional relationships are neutral, but only AFTER a
            // separate settled identity capture. A changing save is never
            // permission to publish guessed or another Dynasty's evidence.
            pipelineUnavailable=true;raw=dynasty::live::Capture(image,GetCurrentProcess(),dynasty::live::resumeIdentityFields,[](const std::string& n){return n=="Franchise"||n=="SeasonInfo"||n=="FranchiseServer_CareerFlow";}).run();
        }
        auto q=[&](uintptr_t a){uintptr_t v=0;if(!read(a,&v,8))throw std::runtime_error("Report ownership unavailable");return v;};
        if((!raw.is_null()&&(std::to_string(static_cast<uint32_t>(raw.at("status").at("league").get<int64_t>()))!=prior->data.league||raw.at("status").at("season").at("CurrentSeasonYear")!=prior->data.season))||!(dynasty::live::activeSession(image,q)==prior->binding))throw std::runtime_error("Dynasty changed during report capture");
        next->staff=evidence::context(raw);next->staff.staffData=evidence::StaffData::NotCapturedForPipelineModel;if(pipelineUnavailable){next->staff.pipelinesKnown=false;next->staff.pipelines.clear();}
        for(const auto& [identity,p]:pending){const NativeEntry* entry=nullptr;for(const auto& [id,e]:prior->data.entries){(void)id;if(e.identity==identity){entry=&e;break;}}if(!entry&&identity.rfind("ui|"+prior->data.league+"|",0)!=0)continue;
            const auto pipeline=evidence::pipelineFor(next->staff,entry?entry->homePipeline:std::string());const auto value=evidence::obtain(next->reports.value(identity,Json()),p,next->staff,pipeline);
            if(!next->reports.contains(identity)||next->reports.at(identity)!=value){next->reports[identity]=value;prepareView(*next,identity);changed=true;}
        }
        // A completed request releases its wait slot even when the saved report
        // already contains this evidence or the prospect is no longer eligible.
        auto finishRequests=[&](){
            std::lock_guard<std::mutex> lock(reportMutex);
            for(const auto& item:pending)if(!pendingReports.count(item.first))waitingReports.erase(item.first);
        };
        if(!changed){finishRequests();return nullptr;}
        const auto dir=folder/L"ScoutingReports"/next->league;aceio::create_directories(dir);const auto path=dir/(std::to_string(next->season)+".json"),temp=dir/(std::to_string(next->season)+".tmp");
        const Json saved={{"format",1},{"league",next->league},{"season",next->season},{"reports",next->reports}};const auto bytes=saved.dump();if(bytes.size()>64u*1024*1024)throw std::runtime_error("Scouting report journal exceeds bounds");
        {aceio::ofstream out(temp,std::ios::binary|std::ios::trunc);out<<bytes;out.flush();if(!out)throw std::runtime_error("Cannot write scouting reports");}
        if(!aceio::move_file(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("Cannot commit scouting reports");
        std::atomic_store(&reportEdition,std::shared_ptr<const ReportEdition>(next));finishRequests();return {{"event","scouting_reports_ready"},{"obtained",pending.size()},{"staffData",evidence::staffDataName(next->staff.staffData)},{"calibration","pipeline-v2"},{"pipelinesKnown",next->staff.pipelinesKnown},{"pipelines",next->staff.pipelines.size()}};
    }
    bool queue(const NotesProspect& p){std::lock_guard<std::mutex> lock(reportMutex);const auto now=GetTickCount64();if(!waitingReports.count(p.identity)&&waitingReports.size()>=64){
        for(auto i=waitingReports.begin();i!=waitingReports.end();){if(now-i->second.last>=8000&&!pendingReports.count(i->first))i=waitingReports.erase(i);else ++i;}
        if(waitingReports.size()>=64)return false;
    }auto& wait=waitingReports[p.identity];if(!wait.first||now-wait.last>2000)wait.first=now;wait.last=now;if(pendingReports.size()<64||pendingReports.count(p.identity))pendingReports[p.identity]=p;return now-wait.first<8000;}
    std::string report(const std::shared_ptr<const Published>& cache,const NotesProspect& p,bool& loading,const NativeEntry* entry=nullptr){
        auto reports=std::atomic_load(&reportEdition);
        if(reports&&(!cache||reports->league!=cache->data.league||reports->season!=cache->data.season))reports.reset();
        const auto saved=reports?reports->reports.find(p.identity):Json::const_iterator{};
        const Json empty;
        const Json& reportValue=!reports||saved==reports->reports.end()?empty:*saved;
        const auto view=reports?reports->views.find(p.identity):std::map<std::string,ReportView>::const_iterator{};
        const auto* prepared=reports&&view!=reports->views.end()?&view->second:nullptr;
        const auto showBackground=evidence::revealBackground(p);
        const auto hasBackground=prepared&&!prepared->background.empty();
        const bool obtain=!reportValue.is_object()||!reportValue.contains("displayOrder")||
            (showBackground&&!hasBackground)||evidence::hasNewEvidence(reportValue,p);
        if(obtain)loading=queue(p)&&!reportFailed.load();
        std::string text=prepared&&p.progress>=25?prepared->scouting[p.progress>=75?2:p.progress>=50?1:0]:evidence::render(Json(),p.progress,false);
        if(obtain&&p.progress>=25&&!reportValue.is_object())
            text="SCOUTING NOTES\n- "+std::string(!loading?"Observations unavailable or delayed. Reopen Notes to retry.":"Preparing scouting observations...");
        if(obtain&&reportFailed.load())text+="\n- New observations are temporarily unavailable. Reopen Notes to retry.";
        if(showBackground){
            text+="\n\nBACKGROUND";
            evidence::appendNoteBullets(text,hasBackground?std::string_view(prepared->background):evidence::newBackground(p));
        }
        const auto selected=enhancements::options();
        if(selected.measurables&&(!selected.recruiting||selected.recruitingScale!=enhancements::GradeScale::Blind)){
            const auto league=enhancements::currentOptionsLeague.load();
            const auto* verifiedEntry=cache&&cache->data.league==std::to_string(league)?entry:nullptr;
            appendMeasurements(text,p,league,verifiedEntry);
        }
        return text;
    }
public:
    template<class View>void completeMeasurements(View& view,const std::map<std::string,uintptr_t>& fields,std::map<std::string,double>& revealed){
        auto cache=std::atomic_load(&published);if(!cache||cache->data.league!=std::to_string(enhancements::currentOptionsLeague.load()))return;
        double id=0;std::string first,last;if(!fields.count("RecordId")||!fields.count("FirstName")||!fields.count("LastName")||!view.number(fields.at("RecordId"),id)||id<0||id>UINT32_MAX||std::floor(id)!=id||!view.string(fields.at("FirstName"),first)||!view.string(fields.at("LastName"),last))return;
        auto found=cache->data.entries.find(static_cast<uint32_t>(id));if(found==cache->data.entries.end()||found->second.first!=first||found->second.last!=last)return;
        complete(found->second,revealed);
    }
    static void complete(const NativeEntry& entry,std::map<std::string,double>& revealed){
        for(const auto& pair:{std::make_pair("speed","SpeedRating"),std::make_pair("acceleration","AccelerationRating")}){const auto value=entry.prospect.ratings.find(pair.second);if(!revealed.count(pair.first)&&value!=entry.prospect.ratings.end()&&std::isfinite(value->second)&&value->second>=1&&value->second<=100)revealed[pair.first]=value->second;}
    }
    void request(){requested=true;uint64_t zero=0;waitingSince.compare_exchange_strong(zero,GetTickCount64());}
    template<class Read>std::shared_ptr<const Published> current(uintptr_t image,Read read){
        auto value=std::atomic_load(&published);if(!value){request();return {};}
        try{auto q=[&](uintptr_t address){uintptr_t v=0;if(!read(address,&v,8))throw std::runtime_error("Dynasty ownership unavailable");return v;};
            if(dynasty::live::activeSession(image,q)==value->binding){
                // Refresh actual college records only on a UI data request;
                // an idle hub never starts a periodic class/stat scan.
                if(value->transfers&&GetTickCount64()-value->capturedAt>=15000)requested=true;
                return value;
            }
        }catch(...){}
        requested=true;return {};
    }
    template<class Read>Json tick(uintptr_t image,const std::filesystem::path& folder,Read read){
        auto q=[&](uintptr_t address){uintptr_t v=0;if(!read(address,&v,8))throw std::runtime_error("Dynasty ownership unavailable");return v;};
        dynasty::live::SessionBinding binding;
        try{binding=dynasty::live::activeSession(image,q);}catch(...){std::atomic_store(&published,std::shared_ptr<const Published>{});requested=true;nextAttempt=GetTickCount64()+1000;return nullptr;}
        if(!(binding==attemptedBinding)){lastFailure.clear();failures=0;attemptedBinding=binding;nextAttempt=0;backgroundFailed=false;reportFailed=false;waitingSince=GetTickCount64();{std::lock_guard<std::mutex> lock(reportMutex);pendingReports.clear();waitingReports.clear();}}
        if(GetTickCount64()<nextAttempt)return nullptr;
        auto existing=std::atomic_load(&published);
        if(existing&&existing->binding==binding&&GetTickCount64()>=nextIdentityCheck){nextIdentityCheck=GetTickCount64()+500;if(existing->annualIdentity.available()&&!existing->annualIdentity.unchanged(read)){std::atomic_store(&published,std::shared_ptr<const Published>{});std::atomic_store(&reportEdition,std::shared_ptr<const ReportEdition>{});{std::lock_guard<std::mutex> lock(reportMutex);pendingReports.clear();waitingReports.clear();}existing.reset();requested=true;backgroundFailed=false;waitingSince=GetTickCount64();}}
        if(existing&&existing->binding==binding){try{if(auto reports=reportTick(existing,folder,image,read);!reports.is_null()){reportFailed=false;return reports;}}catch(const std::exception& error){reportFailed=true;return {{"event","scouting_reports_unavailable"},{"error",error.what()},{"retry","reopen Notes"}};}if(!requested.exchange(false))return nullptr;}
        requested=false;nextAttempt=GetTickCount64()+2000;
        const auto began=GetTickCount64();
        try{
        // Explicit selectors are independent of include order with Analytics.
        auto roots=[](const std::string& name){return name=="SeasonInfo"||name=="Franchise"||name=="FranchiseServer_CareerFlow"||name=="Recruit"||name=="Team"||name=="FranchiseUser";};
        const auto captureBegan=std::chrono::steady_clock::now();
        dynasty::live::Capture capture(image,GetCurrentProcess(),dynasty::live::resumeBackgroundFields,roots);auto raw=capture.run();
        const auto captureEnded=std::chrono::steady_clock::now();
        auto data=parseNativeClass(raw);
        if(!(dynasty::live::activeSession(image,q)==binding))throw std::runtime_error("Dynasty changed while preparing resume cache");
        persistClass(data,folder);
        auto value=std::make_shared<Published>();value->binding=binding;
        for(const auto& item:capture.identityProof(true))for(const auto& check:item.at("checks")){const auto size=check.at("size").get<unsigned>();auto expected=check.at("expected").get<uint64_t>(),mask=check.at("mask").get<uint64_t>();if(!size||size>8)throw std::runtime_error("Invalid Notes identity proof");std::vector<unsigned char> bytes(size),masks(size);memcpy(bytes.data(),&expected,size);memcpy(masks.data(),&mask,size);value->annualIdentity.recordMasked(check.at("address").get<uintptr_t>(),bytes,masks);}
        value->data=std::move(data);value->capturedAt=GetTickCount64();value->staff=evidence::context(raw);if(existing&&existing->data.league==value->data.league&&existing->data.season==value->data.season){auto reports=std::atomic_load(&reportEdition);if(reports&&reports->league==value->data.league&&reports->season==value->data.season)value->reports=reports->reports;}else {
            const auto path=folder/L"ScoutingReports"/value->data.league/(std::to_string(value->data.season)+".json");if(aceio::exists(path)){if(aceio::file_size(path)>64u*1024*1024)throw std::runtime_error("Scouting reports exceed read bounds");aceio::ifstream input(path);Json saved;input>>saved;if(saved.at("format")!=1||saved.at("league")!=value->data.league||saved.at("season")!=value->data.season||!saved.at("reports").is_object()||saved.at("reports").size()>16000)throw std::runtime_error("Scouting report identity differs");value->reports=saved.at("reports");}}
        // Carry immutable evidence for the same persistent recruit into the
        // next season. A reused native row with a different identity cannot
        // inherit it. Write the additive carry-forward so quiet years persist.
        const auto reportDir=folder/L"ScoutingReports"/value->data.league;
        const auto previousPath=reportDir/(std::to_string(value->data.season-1)+".json");bool carried=false;
        if(aceio::exists(previousPath)){
            if(aceio::file_size(previousPath)>64u*1024*1024)throw std::runtime_error("Prior scouting reports exceed bounds");
            aceio::ifstream input(previousPath);Json previous;input>>previous;
            if(previous.at("format")!=1||previous.at("league")!=value->data.league||previous.at("season")!=value->data.season-1||!previous.at("reports").is_object())throw std::runtime_error("Prior scouting report identity differs");
            for(const auto& [id,entry]:value->data.entries){(void)id;if(!value->reports.contains(entry.identity)&&previous.at("reports").contains(entry.identity)){value->reports[entry.identity]=previous.at("reports").at(entry.identity);carried=true;}}
        }
        if(carried){aceio::create_directories(reportDir);const auto path=reportDir/(std::to_string(value->data.season)+".json"),temp=reportDir/(std::to_string(value->data.season)+".tmp");const Json journal={{"format",1},{"league",value->data.league},{"season",value->data.season},{"reports",value->reports}};
            const auto bytes=journal.dump();if(bytes.size()>64u*1024*1024)throw std::runtime_error("Carried scouting reports exceed bounds");{aceio::ofstream out(temp,std::ios::binary|std::ios::trunc);out<<bytes;out.flush();if(!out)throw std::runtime_error("Cannot carry scouting reports");}if(!aceio::move_file(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("Cannot commit carried reports");}
        for(const auto& [id,e]:value->data.entries){(void)id;if(!e.highSchool&&!e.juniorCollege)value->transfers=true;}size_t hs=0;for(const auto& [id,e]:value->data.entries){(void)id;if(e.highSchool)++hs;}
        auto reports=std::make_shared<ReportEdition>();reports->league=value->data.league;reports->season=value->data.season;reports->staff=value->staff;reports->reports=std::move(value->reports);for(const auto& entry:reports->reports.items())prepareView(*reports,entry.key());std::atomic_store(&reportEdition,std::shared_ptr<const ReportEdition>(reports));
        std::atomic_store(&published,std::shared_ptr<const Published>(value));requested=false;
        lastFailure.clear();failures=0;backgroundFailed=false;waitingSince=0;
        return {{"event","recruit_resume_cache_ready"},{"highSchoolProspects",hs},{"unavailableProspects",value->data.unavailable.size()},{"records",raw.at("records").size()},{"ms",GetTickCount64()-began},{"league",value->data.league},{"season",value->data.season},
            {"captureUs",std::chrono::duration_cast<std::chrono::microseconds>(captureEnded-captureBegan).count()}};
        }catch(const std::exception& error){
            backgroundFailed=true;
            // A failed background graph must not block revealed scouting evidence.
            // A separate minimal capture establishes the SAME live Dynasty/year;
            // it neither guesses a background identity nor bypasses remapping.
            if(!existing||!(existing->binding==binding))try{
                auto fields=[](const std::string& name)->std::set<std::string>{if(name=="Franchise"||name=="SeasonInfo"||name=="FranchiseServer_CareerFlow")return dynasty::live::resumeCaptureFields(name);return {"__no_notes_fields__"};};
                auto status=dynasty::live::Capture(image,GetCurrentProcess(),fields,[](const std::string& name){return name=="Franchise"||name=="SeasonInfo"||name=="FranchiseServer_CareerFlow";}).run();
                if(!(dynasty::live::activeSession(image,q)==binding))throw std::runtime_error("Dynasty changed during Notes fallback");
                auto fallback=std::make_shared<Published>();fallback->binding=binding;fallback->backgroundReady=false;fallback->capturedAt=GetTickCount64();fallback->data.league=std::to_string(static_cast<uint32_t>(status.at("status").at("league").get<int64_t>()));fallback->data.season=status.at("status").at("season").at("CurrentSeasonYear");
                if(fallback->data.season<1800||fallback->data.season>3000||fallback->data.league!=std::to_string(enhancements::currentOptionsLeague.load()))throw std::runtime_error("Notes fallback identity differs");
                auto reports=std::make_shared<ReportEdition>();reports->league=fallback->data.league;reports->season=fallback->data.season;
                const auto path=folder/L"ScoutingReports"/reports->league/(std::to_string(reports->season)+".json");
                if(aceio::exists(path)){if(aceio::file_size(path)>64u*1024*1024)throw std::runtime_error("Report journal exceeds bounds");aceio::ifstream input(path);Json saved;input>>saved;if(saved.at("format")!=1||saved.at("league")!=reports->league||saved.at("season")!=reports->season||!saved.at("reports").is_object())throw std::runtime_error("Report journal identity differs");reports->reports=saved.at("reports");for(const auto& entry:reports->reports.items())prepareView(*reports,entry.key());}
                std::atomic_store(&reportEdition,std::shared_ptr<const ReportEdition>(reports));std::atomic_store(&published,std::shared_ptr<const Published>(fallback));
            }catch(...){/* No validated identity: show unavailable, never another save's report. */}
            const bool repeated=lastFailure==error.what();lastFailure=error.what();failures=repeated?(std::min)(failures+1,5u):1u;
            nextAttempt=GetTickCount64()+(std::min)(60000ull,2000ull<<failures);requested=true;
            // Log the first error; identical failures back off without log spam.
            if(!repeated)throw;return nullptr;
        }
    }
    std::string compose(const std::shared_ptr<const Published>& cache,NotesProspect& publicNotes,double rawId,const std::string& first,const std::string& last,bool& loading){
        loading=false;const NativeEntry* found=nullptr;
        if(cache&&rawId>=0&&rawId<=4294967295.0&&std::floor(rawId)==rawId){auto it=cache->data.entries.find(static_cast<uint32_t>(rawId));if(it!=cache->data.entries.end()&&it->second.first==first&&it->second.last==last)found=&it->second;}
        if(!found){
            if(cache&&rawId>=0&&rawId<=4294967295.0&&std::floor(rawId)==rawId){const auto missing=cache->data.unavailable.find(static_cast<uint32_t>(rawId));
                if(missing!=cache->data.unavailable.end()&&missing->second==std::make_pair(first,last)){auto notes=publicNotes;notes.activeTransfer=cache->data.unavailableTransfers.count(static_cast<uint32_t>(rawId))!=0;notes.schoolBackgroundEligible=cache->data.unavailableSchoolRecruits.count(static_cast<uint32_t>(rawId))!=0;notes.identity="ui|"+cache->data.league+"|"+publicNotes.identity;return "RECRUIT BACKGROUND\nBackground information unavailable.\n\n"+report(cache,notes,loading);}}
            request();const bool pending=!backgroundFailed.load()&&GetTickCount64()-waitingSince.load()<8000;loading=pending;
            std::string scouting=evidence::render(Json(),publicNotes.progress,false);
            if(cache){auto notes=publicNotes;notes.identity="ui|"+cache->data.league+"|"+publicNotes.identity;scouting=report(cache,notes,loading);}
            return std::string("RECRUIT BACKGROUND\n")+(pending?"Preparing background report...":"Background information is temporarily unavailable. Reopen Notes to retry.")+"\n\n"+scouting;
        }
        auto notes=publicNotes;
        notes.schoolBackgroundEligible=found->highSchool||found->juniorCollege;
        notes.activeTransfer=!notes.schoolBackgroundEligible;
        if(notes.schoolBackgroundEligible)notes.backgroundDevelopment=found->prospect.development;
        notes.identity=found->identity;const auto priorReports=std::atomic_load(&reportEdition);const auto uiIdentity="ui|"+cache->data.league+"|"+publicNotes.identity;if(priorReports&&priorReports->league==cache->data.league&&priorReports->season==cache->data.season&&priorReports->reports.contains(uiIdentity))notes.identity=uiIdentity;
        if(found->juniorCollege&&!found->classYear.empty())publicNotes.classification=found->saved.at("competitionClass").get<std::string>()+" "+found->classYear;
        else if(const auto classification=nativeTransferClass(*found);!classification.empty())publicNotes.classification=classification;
        if(evidence::noteRole(notes.position)==evidence::NR::Unknown&&evidence::noteRole(found->prospect.position)!=evidence::NR::Unknown)notes.position=found->prospect.position;
        if(publicNotes.progress==100){
            std::map<std::string,double> values;complete(*found,values);
            if(notes.position=="LS"&&found->prospect.position=="LS"){
                const auto rating=found->prospect.ratings.find("LongSnapRating");
                if(rating!=found->prospect.ratings.end()&&std::isfinite(rating->second)&&rating->second>=1&&rating->second<=100)values.emplace("long snap",rating->second);
            }
            for(const auto& pair:values){bool present=false;for(const auto& o:notes.observations){const auto key=notesKey(o.name);present|=key==pair.first||(pair.first=="long snap"&&key=="long snapping");}if(!present)notes.observations.push_back({pair.first,pair.second,pair.second,true});}
        }
        if(found->highSchool||found->juniorCollege)return renderResume(found->saved,publicNotes.progress>=50&&(!enhancements::options().recruiting||enhancements::options().recruitingScale!=enhancements::GradeScale::Blind))+"\n\n"+report(cache,notes,loading,found);
        loading=GetTickCount64()-cache->capturedAt>=15000;
        return renderTransfer(found->actual,found->saved.at("gpa"),cache->data.season,cache->data.relativeYear)+"\n"+report(cache,notes,loading);
    }
};
}
