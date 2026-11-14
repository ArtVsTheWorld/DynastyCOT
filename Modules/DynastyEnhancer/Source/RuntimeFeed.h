#include "UserRuntimePaths.h"
#pragma once
#include "LiveTables.h"
#include "FeedRefresh.h"
#include "AnalyticsSummary.h"
#include "FeedSchedule.h"
#include "RuntimeWire.h"
namespace dynasty::live {
struct GeneratorJobFiles {
    std::filesystem::path input,output;
    ~GeneratorJobFiles(){std::error_code ec;for(const auto& file:{input,output})if(!file.empty()){std::filesystem::remove(file,ec);std::filesystem::remove(std::filesystem::path(file.wstring()+L".tmp"),ec);}}
};
inline uint64_t fingerprint(std::string_view text){uint64_t value=14695981039346656037ull;for(unsigned char c:text){value^=c;value*=1099511628211ull;}return value;}
inline void validateRuntimeIdentity(const Json& feed,const Json& status){
    const auto& season=status.at("season");
    if(feed.value("sourceKind",std::string())!="runtime"||feed.at("leagueId")!=status.at("league")||feed.at("phase")!=season.at("CurrentWeekType")||feed.at("week")!=season.at("CurrentWeek")||feed.at("season")!=season.at("CurrentSeasonYear"))throw std::runtime_error("Cached/generated edition differs from the active Dynasty");
}
// A worker reads memory only. It never invokes the game's getters or changes its tables.
// A verified full capture plus a matching complete consumed-byte proof after
// settling debounces simulation and midweek edits. Moved/unreadable/changed
// proof falls back to a fresh full capture; identity-only checks never publish.
class FeedWorker {
    std::filesystem::path folder,node,exporter,resumer;Address base=0,lastContext=0;
    uint64_t observed=0,accepted=0,observedAt=0;bool haveObserved=false,haveAccepted=false;FeedSchedule schedule;
    std::optional<Json> pendingRaw;std::string pendingInput;Json pendingMetrics;PackedChangeWatch pendingProbe,pendingIdentity;
    void discardPending(){pendingRaw.reset();pendingInput.clear();pendingMetrics=Json();pendingProbe=PackedChangeWatch{};pendingIdentity=PackedChangeWatch{};}
    std::string lastError,lastStatus;unsigned failures=0;bool identityLimited=false;
    PackedChangeWatch settledProbe,settledIdentity;uint64_t fastPolls=0,identityPolls=0,lastIdentityReport=0,lastProbeReport=0,sameContentRefreshes=0,lastSameReport=0;
public:
    void restart(){discardPending();observedAt=0;schedule.reset();haveObserved=false;haveAccepted=false;lastStatus.clear();lastContext=0;lastError.clear();failures=0;identityLimited=false;settledIdentity=PackedChangeWatch{};settledProbe=PackedChangeWatch{};}
    void configure(const std::filesystem::path& directory,Address image,const std::filesystem::path& code={}){
        folder=directory;base=image;auto c=social::refresh::readJson(folder/L"dynasty-enhancer.json");
        auto resolve=[&](const char* key){return dynasty::userpaths::program(folder,code.empty()?folder:code,key,c.at(key).get<std::string>());};node=resolve("node");exporter=resolve("exporter");
        resumer=exporter.parent_path()/L"resume_runtime.mjs";
        // A diagnostic hot reload may reuse a previously verified context only
        // in this exact process lifetime; Capture rechecks native ownership.
        if(c.contains("diagnosticSessionHint")){
            const auto& hint=c.at("diagnosticSessionHint");FILETIME created{},exited{},kernel{},user{};uint64_t creation=0;
            if(GetProcessTimes(GetCurrentProcess(),&created,&exited,&kernel,&user))memcpy(&creation,&created,sizeof(creation));
            if(hint.value("pid",DWORD(0))==GetCurrentProcessId()&&creation&&hint.value("creation",uint64_t(0))==creation)
                sessionContextHint()={hint.at("career").get<Address>(),hint.at("database").get<Address>(),hint.at("context").get<Address>()};
        }
        if(!std::filesystem::is_regular_file(node)||!std::filesystem::is_regular_file(exporter)||!std::filesystem::is_regular_file(resumer))throw std::runtime_error("Dynasty Enhancer runtime generator is unavailable");
    }
    std::optional<Json> tick(const std::function<bool()>& cancelled,const std::function<void(Json)>& report,const std::function<void()>& invalidate){
        auto now=GetTickCount64();if(!schedule.due(now)||cancelled())return {};schedule.begin(now);
        try{
            bool identityChanged=false;
            if(haveAccepted&&!pendingRaw&&schedule.identityOnly(now)&&settledIdentity.available()){
                const auto began=GetTickCount64();size_t bytes=0,calls=0;bool exceeded=false;
                if(settledIdentity.unchanged([&](uintptr_t address,void* out,size_t size){
                    if(cancelled())return false;
                    if(GetTickCount64()-began>100||bytes+size>1024u*1024){exceeded=true;return false;}
                    SIZE_T got=0;bytes+=size;++calls;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),out,size,&got)&&got==size;
                })){schedule.unchangedIdentity(GetTickCount64());++identityPolls;
                    if(GetTickCount64()-lastIdentityReport>=30000){lastIdentityReport=GetTickCount64();report({{"event","runtime_identity_probe"},{"probeMs",GetTickCount64()-began},{"readBytes",bytes},{"readCalls",calls},{"polls",identityPolls}});}return {};}
                // A costly proof must not turn each quick wakeup into a full
                // capture. Fall back to the existing cadence for this context.
                if(exceeded){identityLimited=true;report({{"event","runtime_identity_probe_limited"},{"readBytes",bytes},{"readCalls",calls}});}
                identityChanged=true;
            }
            if(haveAccepted&&!pendingRaw&&!identityChanged&&settledProbe.available()){
                const auto began=GetTickCount64();size_t bytes=0,calls=0;
                const bool unchanged=settledProbe.unchanged([&](uintptr_t address,void* out,size_t size){
                    if(cancelled()||GetTickCount64()-began>500||bytes+size>PackedChangeWatch::maximumBytes)return false;
                    SIZE_T got=0;++calls;bytes+=size;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),out,size,&got)&&got==size;
                });
                if(unchanged){++fastPolls;lastError.clear();failures=0;schedule.accepted(GetTickCount64(),settledIdentity.available()&&!identityLimited);
                    if(GetTickCount64()-lastProbeReport>=30000){lastProbeReport=GetTickCount64();report({{"event","runtime_unchanged_probe"},{"probeMs",GetTickCount64()-began},{"readBytes",bytes},{"readCalls",calls},{"fastPolls",fastPolls},{"sameContentRefreshes",sameContentRefreshes}});}return {};
                }
            }
            if(cancelled())return {};
            PackedChangeWatch candidateProbe,candidateIdentity;Json raw,metrics;std::string serializedInput;bool reusedCapture=false;
            const auto captureBegan=GetTickCount64();uint64_t proofMs=0;
            if(pendingRaw){
                const auto began=GetTickCount64();size_t bytes=0,calls=0;
                bool same=pendingProbe.available()&&pendingProbe.unchanged([&](uintptr_t address,void* out,size_t size){
                    if(cancelled()||GetTickCount64()-began>500||bytes+size>PackedChangeWatch::maximumBytes)return false;
                    SIZE_T got=0;bytes+=size;++calls;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),out,size,&got)&&got==size;
                });
                proofMs=GetTickCount64()-began;
                if(same){raw=std::move(*pendingRaw);serializedInput=std::move(pendingInput);metrics=std::move(pendingMetrics);candidateProbe=std::move(pendingProbe);candidateIdentity=std::move(pendingIdentity);reusedCapture=true;}
                report({{"event","runtime_settle_verification"},{"matched",same},{"probeMs",proofMs},{"readBytes",bytes},{"readCalls",calls}});
                discardPending();
            }
            if(raw.is_null())raw=Capture(base).run(&candidateProbe,&candidateIdentity);
            const auto captureMs=GetTickCount64()-captureBegan;const auto ctx=raw.at("context").get<Address>();
            auto status=raw.at("status").dump();if(ctx!=lastContext||status!=lastStatus){if(ctx!=lastContext)identityLimited=false;lastStatus=status;lastContext=ctx;haveObserved=false;haveAccepted=false;invalidate();report({{"event","active_dynasty_context"},{"context",ctx}});}
            if(!reusedCapture){
                metrics={{"captureMs",captureMs},{"records",raw.at("records").size()}};
                const auto analyticsBegan=GetTickCount64();
                try{raw["playerAnalytics"]=analytics::live::socialSummary(raw);}
                catch(const std::exception& error){report({{"event","player_analytics_unavailable"},{"reason",error.what()}});}
                metrics["analyticsMs"]=GetTickCount64()-analyticsBegan;
                const auto wireBegan=GetTickCount64();auto wire=generatorWire(raw);metrics["wireMs"]=GetTickCount64()-wireBegan;
                const auto serializeBegan=GetTickCount64();serializedInput=wire.dump();metrics["serializeMs"]=GetTickCount64()-serializeBegan;
            }
            const auto hashBegan=GetTickCount64();
            auto hash=reusedCapture?observed:fingerprint(serializedInput);const auto hashMs=GetTickCount64()-hashBegan;
            auto verifyFacts=[&](){
                const auto began=GetTickCount64();size_t bytes=0,calls=0;bool limited=false;
                bool same=!cancelled()&&Capture(base).status().dump()==status&&candidateProbe.available()&&candidateProbe.unchanged([&](uintptr_t address,void* out,size_t size){
                    if(cancelled())return false;
                    if(GetTickCount64()-began>500||bytes+size>PackedChangeWatch::maximumBytes){limited=true;return false;}
                    SIZE_T got=0;bytes+=size;++calls;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),out,size,&got)&&got==size;
                });
                // Large/slow proofs retain the full-capture path rather than
                // making a supported Dynasty permanently unable to publish.
                if(!cancelled()&&(!candidateProbe.available()||limited)){
                    auto checked=Capture(base).run();try{checked["playerAnalytics"]=analytics::live::socialSummary(checked);}catch(const std::exception&){}
                    same=checked.at("status").dump()==status&&fingerprint(generatorWire(checked).dump())==hash;
                }
                report({{"event","runtime_commit_verification"},{"matched",same},{"probeMs",GetTickCount64()-began},{"readBytes",bytes},{"readCalls",calls}});return same;
            };
            auto accept=[&](Json feed,const char* event)->std::optional<Json>{
                social::refresh::validate(feed,Json{{"save",feed.at("source")},{"leagueId",feed.at("leagueId")},{"userTeamIndex",feed.at("userTeamIndex")}});validateRuntimeIdentity(feed,raw.at("status"));
                if(!verifyFacts()){haveObserved=false;invalidate();return {};}
                social::refresh::writeAtomic(folder/L"runtime-feed.json.tmp",folder/L"runtime-feed.json",feed);
                accepted=hash;haveAccepted=true;lastError.clear();failures=0;settledProbe=std::move(candidateProbe);settledIdentity=std::move(candidateIdentity);schedule.accepted(GetTickCount64(),settledIdentity.available()&&!identityLimited);
                report({{"event",event},{"summary","Weekly Social Media edition verified and ready for the current Dynasty."},{"league",feed.at("leagueId")},{"team",feed.at("userTeamIndex")},{"week",feed.at("week")},{"stories",feed.at("stories").size()},{"saveRequired",false},{"identityProbeEnabled",settledIdentity.available()&&!identityLimited},{"identityProbeBytes",settledIdentity.bytes()},{"identityProbeRanges",settledIdentity.rangeCount()}});return feed;
            };
            if(haveAccepted&&hash==accepted){settledProbe=std::move(candidateProbe);settledIdentity=std::move(candidateIdentity);lastError.clear();failures=0;schedule.accepted(GetTickCount64(),settledIdentity.available()&&!identityLimited);++sameContentRefreshes;
                if(GetTickCount64()-lastSameReport>=30000){lastSameReport=GetTickCount64();report({{"event","runtime_same_content_refresh"},{"captureMs",captureMs},{"captureAndHashMs",GetTickCount64()-captureBegan},{"sameContentRefreshes",sameContentRefreshes}});}return {};}
            if(!haveObserved||hash!=observed){observed=hash;haveObserved=true;observedAt=GetTickCount64();schedule.observed(observedAt);
                report({{"event","runtime_change_observed"},{"records",metrics.at("records")},{"captureMs",captureMs},{"hashMs",hashMs},{"wireMs",metrics.at("wireMs")},{"serializeMs",metrics.at("serializeMs")},{"inputBytes",serializedInput.size()}});
                // The compact immutable input is sufficient after analytics.
                // Release the large native record graph before settling.
                raw.erase("records");raw.erase("playerAnalytics");pendingRaw=std::move(raw);pendingInput=std::move(serializedInput);pendingMetrics=std::move(metrics);
                pendingProbe=std::move(candidateProbe);pendingIdentity=std::move(candidateIdentity);return {};
            }
            if(cancelled())return {};
            const auto suffix=L"-"+std::to_wstring(GetCurrentProcessId())+L".json";
            const auto input=folder/(L"runtime-raw"+suffix),output=folder/(L"runtime-export"+suffix);
            GeneratorJobFiles job{input,output};
            const auto inputBegan=GetTickCount64();social::refresh::writeTextAtomic(std::filesystem::path(input.wstring()+L".tmp"),input,serializedInput);const auto inputWriteMs=GetTickCount64()-inputBegan;
            social::refresh::Settings settings{{},node,exporter,input};
            const auto generationBegan=GetTickCount64();
            auto approve=[&](){auto proposed=social::refresh::readJson(output);social::refresh::validate(proposed,Json{{"save",proposed.at("source")},{"leagueId",proposed.at("leagueId")},{"userTeamIndex",proposed.at("userTeamIndex")}});validateRuntimeIdentity(proposed,raw.at("status"));return verifyFacts();};
            if(social::refresh::exportSave(settings,output,folder/L"runtime-generator.log",cancelled,approve))throw std::runtime_error("Runtime generator failed; see runtime-generator.log");
            const auto generationMs=GetTickCount64()-generationBegan,parseBegan=GetTickCount64();auto feed=social::refresh::readJson(output);const auto outputParseMs=GetTickCount64()-parseBegan;
            report({{"event","runtime_generation_timing"},{"captureMs",captureMs},{"captureReused",reusedCapture},{"proofMs",proofMs},{"hashMs",hashMs},{"originalCaptureMs",metrics.at("captureMs")},{"analyticsMs",metrics.at("analyticsMs")},{"wireMs",metrics.at("wireMs")},{"serializeMs",metrics.at("serializeMs")},{"inputBytes",serializedInput.size()},{"inputWriteMs",inputWriteMs},{"generationMs",generationMs},{"outputParseMs",outputParseMs},{"observedToGeneratedMs",GetTickCount64()-observedAt}});
            return accept(std::move(feed),"runtime_feed_published");
        }catch(const std::exception& e){discardPending();schedule.failed(GetTickCount64());haveObserved=false;if(++failures>=2){haveAccepted=false;invalidate();}if(lastError!=e.what()){lastError=e.what();report({{"event","runtime_waiting"},{"summary","Weekly Social Media edition is waiting: "+lastError},{"reason",lastError}});}return {};}
    }
};
}
