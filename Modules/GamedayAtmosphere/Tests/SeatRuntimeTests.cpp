// Run the actual production visibility/apply/cache functions in this test
// executable, with native scene callbacks replaced by private test objects.
// No running game, injection or external process handle is involved.
#include "../Source/NativeRuntime.cpp"
#include <cassert>
#include <iostream>
#include <sstream>
namespace {
template<class T>void testPut(A address,T value){std::memcpy(reinterpret_cast<void*>(address),&value,sizeof(value));}
struct TestScene {
    std::vector<A> object,renderer,properties,entry,value,holder;
    std::vector<unsigned short> masks,native,original;
    std::vector<attendance::seats::Position> xyz;
    TestScene(unsigned n,bool mapped=true):object(0xa00/8),renderer(0x200/8),properties(2),entry(2),value(2),holder(2),masks(n),native(n),original(n),xyz(n){
        for(unsigned i=0;i<n;++i){float row=static_cast<float>((i/24)%32),seat=static_cast<float>(i%24);xyz[i]={28+row*.9f,1+row*.65f,-48+seat*4,0};
            masks[i]=native[i]=original[i]=static_cast<unsigned short>(i%7==0?7:(i&1)?4:0);
        }
        testPut(address(),image+attendance::bindings::SceneCrowdVtable);testPut(address()+0x9f0,render());testPut(address()+0x908,A(0x10000));
        testPut(render(),image+attendance::bindings::CrowdRendererVtable);buffers();
        if(mapped){testPut(address()+0x9f8,reinterpret_cast<A>(properties.data()));properties[0]=reinterpret_cast<A>(entry.data());properties[1]=properties[0]+16;
            entry[0]=image+attendance::bindings::CrowdPositionKey;entry[1]=reinterpret_cast<A>(value.data());value[1]=reinterpret_cast<A>(holder.data());holder[0]=n;holder[1]=reinterpret_cast<A>(xyz.data());}
    }
    A address()const{return reinterpret_cast<A>(object.data());}A render()const{return reinterpret_cast<A>(renderer.data());}
    void buffers(){testPut(render()+0x188,reinterpret_cast<A>(masks.data()));testPut(render()+0x190,reinterpret_cast<A>(masks.data()+masks.size()));}
};
std::vector<TestScene*> testScenes;
bool nativePresentationActive=false;
unsigned __fastcall testPart(A,unsigned part){return part;}
void __fastcall testSet(A scene,unsigned part,unsigned agent,bool hidden){
    for(auto* s:testScenes)if(s->address()==scene){auto flag=static_cast<unsigned short>(1u<<part);s->masks[agent]=static_cast<unsigned short>((s->masks[agent]&~flag)|(hidden?flag:0));return;}
    assert(false);
}
void __fastcall testUpdate(A){
    if(applying)assert(!nativePresentationActive);
    for(auto* s:testScenes)for(unsigned i=0;i<s->masks.size();++i)for(unsigned part=0;part<3;++part){
        // One model part only exists on even-numbered agents. Baseline readiness
        // must not require that part's callback for every agent.
        if(part==2&&(i&1))continue;
        visibility(s->address(),part,i,(s->native[i]&(1u<<part))!=0);
    }
}
void __fastcall testPresentation(A,float){
    nativePresentationActive=true;
    for(A controller:entities)attendanceUpdate(controller);
    nativePresentationActive=false;
}
void checkScene(const TestScene& s,int loss){
    const auto& base=startingCrowds.at(s.render());assert(base.selection.policy!=attendance::seats::Policy::Pending);
    for(unsigned i=0;i<s.masks.size();++i){bool selected=base.selection.departed(i,loss);
        for(unsigned part=0;part<3;++part){if(part==2&&(i&1))continue;auto flag=static_cast<unsigned short>(1u<<part);
            bool expected=(s.native[i]&flag)||(s.original[i]&flag)||selected;
            if(((s.masks[i]&flag)!=0)!=expected)std::cerr<<"Visibility mismatch: loss="<<loss<<" agent="<<i<<" part="<<part<<" active="<<activeLoss<<" arriving="<<arrivalMissing<<" Q="<<shownQuarter<<" phase="<<shownPhase<<" clock="<<shownSeconds<<" expectedHidden="<<expected<<"\n";
            assert(((s.masks[i]&flag)!=0)==expected);
        }
    }
}
std::vector<void*> worldPages;
void testPage(A address){
    A page=address&~A(0xffff);for(auto p:worldPages)if(reinterpret_cast<A>(p)==page)return;
    auto p=VirtualAlloc(reinterpret_cast<void*>(page),65536,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE);assert(p==reinterpret_cast<void*>(page));worldPages.push_back(p);
}
template<class T>void testGlobal(A rva,T value){testPage(image+rva);testPut(image+rva,value);}
template<class F>void testFunction(A rva,F function){
    testPage(image+rva);unsigned char thunk[]={0x48,0xb8,0,0,0,0,0,0,0,0,0xff,0xe0};A target=reinterpret_cast<A>(function);
    std::memcpy(thunk+2,&target,8);std::memcpy(reinterpret_cast<void*>(image+rva),thunk,sizeof(thunk));FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(image+rva),sizeof(thunk));
}
struct TestVariant {A vt,type,data;};
bool __fastcall testOption(A,unsigned key,TestVariant* value,A,A){
    static unsigned short bowl=127,rivalry=1023;
    value->data=reinterpret_cast<A>(key==0x9e2b2900?&bowl:&rivalry);return true;
}
bool __fastcall testNeutral(){return false;}
int __fastcall testRank(A,int team){return team>=1500?255:26;}
void __fastcall testPeriod(A manager,int next){testPut(manager+8,next);}
// Build the same verified read-only world layout in THIS executable, allowing
// decision() and the native attendance wrapper to be exercised before a snap.
struct TestWorld {
    std::vector<A> config,states,wrapper,match,manager,clock,weather,service;
    unsigned char paused=0;
    TestWorld():config(0x100/8),states(0xb0/8),wrapper(4),match(0x200/8),manager(0x40/8),clock(0x40/8),weather(0x400/8),service(2){
        testGlobal(0xd9eb808,14);testGlobal(0xf3fddbc,3);testGlobal(0xf63f9f0,reinterpret_cast<A>(config.data()));
        testGlobal(attendance::bindings::GameStates,reinterpret_cast<A>(states.data()));states[0]=image+attendance::bindings::GameStatesVtable;phase(6);
        testGlobal(attendance::bindings::MatchWrapper,reinterpret_cast<A>(wrapper.data()));wrapper[2]=reinterpret_cast<A>(match.data());match[0]=image+attendance::bindings::MatchVtable;
        testGlobal(attendance::bindings::ClockManager,reinterpret_cast<A>(manager.data()));manager[0]=image+0xb47eec8;testPut(reinterpret_cast<A>(manager.data())+8,0);manager[5]=reinterpret_cast<A>(clock.data());
        testPut(reinterpret_cast<A>(clock.data())+0x28,900.0f);testPut(reinterpret_cast<A>(clock.data())+0x2c,900.0f);
        // Start with comparable teams so the later severe/FCS game has a different
        // opening target instead of both hitting the new regular-season floor.
        testGlobal(0xd9fba20,1119);testGlobal(0xd9fba24,1120);testGlobal(attendance::bindings::PauseFlag,reinterpret_cast<A>(&paused));pauseBinding=true;
        testGlobal(0xf506000,reinterpret_cast<A>(weather.data()));weather[0]=image+0xb32a290;testPut(reinterpret_cast<A>(weather.data())+0xe8,70);
        testGlobal(0xe2e53e8,reinterpret_cast<A>(service.data()));service[0]=image+0xb29ee30;testPage(service[0]);testPut(service[0]+0x68,image+0x9099440);
        testGlobal(0xfc90fb8,A(0));testFunction(0x9099440,testOption);testFunction(attendance::bindings::NeutralGetter,testNeutral);testFunction(0xa0b3000,testRank);
    }
    void phase(int value){testPut(reinterpret_cast<A>(states.data())+0xa0,value);}
};
struct TestExpressions {
    std::vector<unsigned char> bytes=std::vector<unsigned char>(65536);size_t next=0;
    A allocate(size_t n){assert(next+n<=bytes.size());A address=reinterpret_cast<A>(bytes.data()+next);next+=n;return address;}
    A fn(const char* id){A node=allocate(0x50);testPut(node,image+attendance::bindings::MuseFunctionVtable);std::memcpy(reinterpret_cast<void*>(node+0x48),id,std::strlen(id)+1);return node;}
    A compare(int op,const char* id,int value){A node=allocate(0x30),constant=allocate(0x18);testPut(node,image+attendance::bindings::MuseComparisonVtable);testPut(node+8,fn(id));testPut(node+0x18,constant);testPut(node+0x28,op);testPut(constant,image+attendance::bindings::MuseConstantVtable);testPut(constant+8,value);return node;}
    A logic(std::initializer_list<A> children,int op=0){A node=allocate(0x50),entries=allocate(children.size()*16);testPut(node,image+attendance::bindings::MuseLogicVtable);testPut(node+8,entries);testPut(node+0x10,entries+children.size()*16);testPut(node+0x48,op);size_t index=0;for(A child:children)testPut(entries+16*index++,child);return node;}
};
unsigned testLogicCalls=0,testComparisonCalls=0,testEventCalls=0;
A testLogicRoot=0,testLogicChild=0;
bool __fastcall testLogic(A node,A context){
    ++testLogicCalls;
    if(node==testLogicRoot)assert(museLogicEvaluate(testLogicChild,context));
    if(node==testLogicChild){testPut(node+0x38,static_cast<unsigned char>(0));testPut(node+0x40,4);}
    return true;
}
A __fastcall testComparison(A,A output){++testComparisonCalls;return output;}
void __fastcall testReplayEvent(A){++testEventCalls;}
void testReplaySelectors(TestWorld& world){
    unsigned char copied[256]{};const unsigned char source[]={1,2,3,4};
    assert(readReplayObject(reinterpret_cast<A>(source),copied,4)&&!std::memcmp(copied,source,4));
    assert(!readReplayObject(0,copied,4)&&!readReplayObject(UINTPTR_MAX,copied,4));
    assert(!readReplayObject(reinterpret_cast<A>(source),copied,257)&&!readReplayObject(reinterpret_cast<A>(source),nullptr,4));
    SYSTEM_INFO sys{};GetSystemInfo(&sys);size_t pageSize=sys.dwPageSize;
    auto pages=static_cast<unsigned char*>(VirtualAlloc(nullptr,pageSize*2,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));assert(pages);
    DWORD old=0;assert(VirtualProtect(pages+pageSize,pageSize,PAGE_NOACCESS,&old));
    assert(!readReplayObject(reinterpret_cast<A>(pages+pageSize),copied,4));
    assert(!readReplayObject(reinterpret_cast<A>(pages+pageSize-2),copied,4));
    VirtualFree(pages,0,MEM_RELEASE);
    originalMuseLogic=testLogic;originalMuseComparison=testComparison;originalReplayEventNode=testReplayEvent;
    replayPresentationInstalled=true;extraReplays=false;
    auto selectorCalls=replaySelectorTimings.calls.load();
    assert(museLogicEvaluate(UINTPTR_MAX,123)&&museComparisonValue(UINTPTR_MAX,456)==456);
    replayEventNode(UINTPTR_MAX);
    assert(testLogicCalls==1&&testComparisonCalls==1&&testEventCalls==1&&replaySelectorTimings.calls==selectorCalls);
    // Private native objects, not a game: eligibility is valid but no play-data
    // bindings are available. An unrelated selector must not sample that data.
    std::vector<A> replay(0x1040/8);replay[0]=image+attendance::bindings::ReplayVtable;
    testGlobal(attendance::bindings::ReplayManager,reinterpret_cast<A>(replay.data()));replayBinding=true;world.phase(4);
    extraReplays=true;bigPlayReplays=true;majorReplayBindings=true;majorReplayFailed=false;playBindingsAvailable=false;
    auto refreshes=replayMajorSelectorSamples.load();TestExpressions expressions;
    A unrelated=expressions.logic({expressions.fn("999")});
    assert(replayPresentationAllowed());
    for(int i=0;i<200;++i)assert(museLogicEvaluate(unrelated,123));
    assert(replayMajorSelectorSamples==refreshes&&testLogicCalls==201&&!replaySelection.depth);
    A touchdown=expressions.logic({expressions.fn("27")});assert(museLogicEvaluate(touchdown,123));
    assert(replayMajorSelectorSamples==refreshes+1&&!replaySelection.depth);
    // No graph identity is retained: changing the same root's child is seen
    // immediately, and an invalid pointer simply fails shape recognition.
    A entries=at<A>(touchdown+8);testPut(entries,expressions.fn("999"));assert(museLogicEvaluate(touchdown,123));
    assert(replayMajorSelectorSamples==refreshes+1&&museLogicEvaluate(UINTPTR_MAX,123));
    world.paused=1;assert(!replayPresentationAllowed());world.paused=0;
    testPut(reinterpret_cast<A>(replay.data())+0x38,static_cast<unsigned char>(1));assert(!replayPresentationAllowed());
    testPut(reinterpret_cast<A>(replay.data())+0x38,static_cast<unsigned char>(0));
    world.phase(3);assert(!replayPresentationAllowed());world.phase(4);
    testPut(reinterpret_cast<A>(world.config.data())+0x84,static_cast<unsigned char>(1));assert(!replayPresentationAllowed());
    testPut(reinterpret_cast<A>(world.config.data())+0x84,static_cast<unsigned char>(0));
    // The real wrapper must see the native child's freshly produced Variant,
    // preserve nesting, and discard its selection before another evaluation.
    A gate=expressions.logic({expressions.fn("257"),expressions.fn("36"),expressions.fn("38"),
        expressions.logic({expressions.compare(0,"128",2),expressions.compare(5,"22",5)}),
        expressions.logic({expressions.compare(0,"128",3),expressions.compare(5,"22",7)})},1);
    A routine=expressions.logic({gate,expressions.fn("1")});
    testLogicRoot=routine;testLogicChild=gate;replayMode=attendance::replaymode::Mode::PlayCall;
    // This mock deliberately has no play/snap or native play-call bindings.
    // Retain the native false gate; positive admission is covered by ReplayEntryGateTests.
    assert(museLogicEvaluate(routine,123)&&at<unsigned char>(gate+0x38)==0&&!replaySelection.depth&&!replaySelection.selection.insetGate);
    extraReplays=false;assert(museLogicEvaluate(routine,123)&&at<unsigned char>(gate+0x38)==0);
    extraReplays=true;testLogicRoot=testLogicChild=0;replayMode=attendance::replaymode::Mode::Unknown;
    A unrelatedEvent=expressions.allocate(0x60);testPut(unrelatedEvent,image+attendance::bindings::ReplayEventNodeVtable);testPut(unrelatedEvent+0x50,11);
    auto beforeUnrelated=replayMajorSelectorSamples.load();
    replayEventNode(unrelatedEvent);assert(testEventCalls==2&&replayMajorSelectorSamples==beforeUnrelated);
    replayPresentationInstalled=false;majorReplayBindings=false;extraReplays=true;replayBinding=false;world.phase(6);
    testGlobal(attendance::bindings::ReplayManager,A(0));
    originalMuseLogic=nullptr;originalMuseComparison=nullptr;originalReplayEventNode=nullptr;
}
unsigned testIntensityUpdates=0;
void __fastcall testIntensityUpdate(A){++testIntensityUpdates;}
const std::array<float,16> callbackMatrix{1,0,0,0,0,1,0,0,0,0,1,0,4,12,20,1};
void __fastcall testCameraCopy(A,float* output){std::memcpy(output,callbackMatrix.data(),sizeof(callbackMatrix));}
void testAudioCameraCallbacks(TestWorld& world){
    A savedMatch=audioMatch.load();unsigned savedEpoch=audioEpoch.load();bool savedAudio=audioSetting,savedCamera=cameraAvailable;
    auto savedDensity=std::atomic_load(&renderedOccupancy);auto savedCue=std::atomic_load(&cameraCue);const bool savedAudible=audioAudible;auto savedCore=std::atomic_load(&coreSettings);
    int savedLoss=activeLoss,savedMissing=arrivalMissing;auto savedIntensity=originalIntensityUpdate;auto savedView=originalCameraView;
    audioMatch=world.wrapper[2];audioEpoch=99;audioSetting=true;activeLoss=10;arrivalMissing=20;
    std::shared_ptr<const attendance::occupancy::Rendered> density=std::make_shared<attendance::occupancy::Rendered>(attendance::occupancy::Rendered{audioMatch,99,1,1000,600});std::atomic_store(&renderedOccupancy,density);
    auto wanted=audioWanted();assert(wanted.active&&std::fabs(wanted.remaining-.54f)<.0001f&&std::fabs(wanted.arrivalFraction-.48f)<.0001f);
    auto samples=audioContextSnapshots.load(),worldSamples=audioWorldSnapshots.load();originalIntensityUpdate=testIntensityUpdate;
    intensityUpdate(0);assert(testIntensityUpdates==1&&audioContextSnapshots==samples+1&&audioWorldSnapshots==worldSamples+1);
    audioSetting=false;worldSamples=audioWorldSnapshots;wanted=audioWanted();assert(!wanted.active&&audioWorldSnapshots==worldSamples&&wanted.match==audioMatch&&wanted.epoch==99);
    assert(std::fabs(wanted.remaining-.54f)<.0001f&&std::fabs(wanted.arrivalFraction-.48f)<.0001f);
    audioSetting=true;audioEpoch=100;wanted=audioWanted();assert(!wanted.active&&wanted.remaining==.75f&&wanted.arrivalFraction==.75f);audioEpoch=99;
    audioMatch=123;assert(!audioWanted().active);audioMatch=world.wrapper[2];
    std::vector<A> replay(0x80/8),cameraSettingsData(0x500/8),owner(2),cameraData(0x300/8),views(4*0x68/8),view(0x800/8);
    replay[0]=image+attendance::bindings::ReplayVtable;testGlobal(attendance::bindings::ReplayManager,reinterpret_cast<A>(replay.data()));
    cameraSettingsData[0]=image+attendance::bindings::CameraSettingsVtable;
    testPut(reinterpret_cast<A>(cameraSettingsData.data())+0x3e6,static_cast<unsigned short>(1));testPut(reinterpret_cast<A>(cameraSettingsData.data())+0x41a,static_cast<unsigned short>(1));testGlobal(attendance::bindings::CameraSettings,reinterpret_cast<A>(cameraSettingsData.data()));
    owner[0]=reinterpret_cast<A>(cameraData.data());testPut(owner[0]+0x1f0,reinterpret_cast<A>(views.data()));views[0x68/8]=reinterpret_cast<A>(view.data());testGlobal(attendance::bindings::CameraOwner,reinterpret_cast<A>(owner.data()));view[0]=image+attendance::bindings::CameraVtable;
    std::array<float,16> projection{};projection[0]=2;projection[5]=3.1715948f;projection[11]=1;std::memcpy(reinterpret_cast<void*>(reinterpret_cast<A>(view.data())+0x2b4),projection.data(),sizeof(projection));
    auto originalView=view;auto now=GetTickCount64();attendance::motion::Cue cue;cue.match=audioMatch;cue.epoch=99;cue.began=now-100;cue.updated=now;cue.until=now+5000;cue.kind=attendance::motion::Kind::LivePlay;cue.percent=5;
    std::shared_ptr<const attendance::motion::Cue> published=std::make_shared<attendance::motion::Cue>(cue);std::atomic_store(&cameraCue,published);
    auto audibleCore=std::make_shared<attendance::core::Settings>(*savedCore);audibleCore->enabled=true;audibleCore->cameraShake=true;std::shared_ptr<const attendance::core::Settings> readyCore=audibleCore;std::atomic_store(&coreSettings,readyCore);audioAudible=false;
    cameraAvailable=true;originalCameraView=testCameraCopy;presentationCamera.configure(image);world.phase(3);auto before=presentationCamera.changed();std::array<float,16> output{};
    // User mute still wins. When sound is enabled, native voice-start latency
    // no longer suppresses a valid camera envelope. Audio follows the cue.
    cameraView(reinterpret_cast<A>(view.data()),output.data());assert(output==callbackMatrix&&presentationCamera.changed()==before);
    audioAudible=true;
    cameraView(reinterpret_cast<A>(view.data()),output.data());assert(output!=callbackMatrix&&presentationCamera.changed()==before+1&&view==originalView);
    world.paused=1;cameraView(reinterpret_cast<A>(view.data()),output.data());assert(output==callbackMatrix&&presentationCamera.changed()==before+1);world.paused=0;
    testPut(reinterpret_cast<A>(cameraSettingsData.data())+0x3e6,static_cast<unsigned short>(0));cameraView(reinterpret_cast<A>(view.data()),output.data());assert(output==callbackMatrix&&presentationCamera.changed()==before+1);
    world.phase(6);cameraAvailable=savedCamera;audioSetting=savedAudio;audioMatch=savedMatch;audioEpoch=savedEpoch;activeLoss=savedLoss;arrivalMissing=savedMissing;
    originalIntensityUpdate=savedIntensity;originalCameraView=savedView;std::atomic_store(&renderedOccupancy,savedDensity);std::atomic_store(&cameraCue,savedCue);audioAudible=savedAudible;std::atomic_store(&coreSettings,savedCore);
    testGlobal(attendance::bindings::ReplayManager,A(0));testGlobal(attendance::bindings::CameraSettings,A(0));testGlobal(attendance::bindings::CameraOwner,A(0));
}
void testEmptyMode(TestWorld& world){
    auto savedEntities=entities;auto savedScenes=testScenes;TestScene stadium(1536);
    std::vector<A> controller(8),link(2);controller[0]=image+0xb485368;controller[7]=reinterpret_cast<A>(link.data());link[1]=image+0xf50ab30;
    entities={reinterpret_cast<A>(controller.data())};testScenes={&stadium};world.phase(1);
    testGlobal(0xf3fddbc,1);attendance::State noDynasty;A match=0;assert(!gameState(noDynasty,match));
    auto revision=attendanceArchive.revision;activeLoss=17;arrivalMissing=34;emptyMode2020=true;emptyModePending=true;
    assert(guardedEmptyService()&&emptyCrowdActive&&appliedVisibilityTarget==100&&shownVisibleFans==0);
    for(auto mask:stadium.masks)assert((mask&3)==3);
    assert(activeLoss==17&&arrivalMissing==34&&attendanceArchive.revision==revision);
    // Replacement renderer masks and replay refreshes stay empty without
    // changing the underlying attendance timeline or original empty seats.
    replayLoss=30;assert(refreshReplayVisibility());for(auto mask:stadium.masks)assert((mask&3)==3);replayLoss=-1;
    auto oldMasks=std::move(stadium.masks);stadium.masks=stadium.native;stadium.buffers();testUpdate(0);assert(guardedEmptyService());
    for(auto mask:stadium.masks)assert((mask&3)==3);
    emptyMode2020=false;emptyModePending=true;assert(guardedEmptyService()&&!emptyCrowdActive);assert(stadium.masks==stadium.native);
    assert(activeLoss==17&&arrivalMissing==34&&attendanceArchive.revision==revision);
    // Mode is also available with normal attendance disabled. Its own mask
    // override is reversible and must not become a permanent100% departure.
    auto disabled=std::make_shared<attendance::Settings>();disabled->enabled=false;auto savedSettings=std::atomic_load(&settings);
    std::shared_ptr<const attendance::Settings> off=disabled;std::atomic_store(&settings,off);
    emptyMode2020=true;emptyModePending=true;assert(guardedEmptyService()&&emptyCrowdActive);assert(!audioWanted().active&&audioWanted().remaining==0);
    emptyMode2020=false;emptyModePending=true;assert(guardedEmptyService()&&!emptyCrowdActive&&stadium.masks==stadium.native);
    std::atomic_store(&settings,savedSettings);testGlobal(0xf3fddbc,3);
    // A normal owned Dynasty must restore its CURRENT arrival/departure target,
    // including seats which were temporarily hidden only by 2020 Mode.
    contextReady=true;lastMatch=world.wrapper[2];
    shownTeam0=at<int>(image+0xd9fba20);shownTeam1=at<int>(image+0xd9fba24);matchLifecycle.join(lastMatch,false);
    exactVisibility=true;assert(guardedApply(true,true));exactVisibility=false;auto normalMasks=stadium.masks;
    emptyMode2020=true;emptyModePending=true;assert(guardedEmptyService()&&shownVisibleFans==0);
    auto passes=forcedCrowdPasses.load();for(int i=0;i<50;++i)assert(guardedEmptyService());assert(forcedCrowdPasses==passes);
    emptyMode2020=false;emptyModePending=true;assert(guardedEmptyService()&&stadium.masks==normalMasks&&!exactVisibility);
    // MMC's existing restore/stop protocol also releases the independent mode.
    emptyMode2020=true;emptyModePending=true;assert(guardedEmptyService()&&shownVisibleFans==0);
    auto priorRequest=request.load();assert(DynamicGamedayAtmosphereStop(nullptr)==0&&!emptyMode2020&&emptyModePending&&request==3);
    assert(guardedEmptyService()&&stadium.masks==normalMasks);request=priorRequest;
    world.phase(6);resetMatch();entities=savedEntities;testScenes=savedScenes;emptyModePending=false;
}
}
int main(){
    image=0x10000000;enabled=true;originalSet=testSet;originalAttendanceUpdate=testUpdate;audioMatch=123;audioEpoch=7;
    logsFolder=std::filesystem::temp_directory_path()/L"Ace-SeatRuntimeTests";std::filesystem::create_directories(logsFolder);
    writeActivationStatus("INACTIVE","Private fixture ownership refusal");
    for(auto name:{"status.txt","technical-status.txt"}){std::ifstream f(logsFolder/name);std::string text((std::istreambuf_iterator<char>(f)),{});assert(text.find("Activation: INACTIVE")!=std::string::npos&&text.find("Private fixture ownership refusal")!=std::string::npos);}
    // Reproduce the reported incompatible visibility entry in this private
    // test executable. The required check must exit BEFORE creating hooks,
    // touching any game code, or enabling the audio/presentation subsystems.
    testPage(image);testPage(image+0x64b3930);
    const unsigned char incompatible[]={0x48,0x8b,0xce,0x44,0x89,0x7d,0xd0,0xc5,0xfa,0x7f,0x45,0xe0,0x89,0x5d,0xd4};
    std::memcpy(reinterpret_cast<void*>(image+0x64b3930),incompatible,sizeof(incompatible));
    assert(!install()&&!signatures&&patches.empty()&&!audioHookInstalled&&!replayPresentationInstalled);
    assert(originalSet==testSet&&originalAttendanceUpdate==testUpdate);
    assert(!std::memcmp(reinterpret_cast<void*>(image+0x64b3930),incompatible,sizeof(incompatible)));
    // A resolver shim in this executable's own memory supplies the verified
    // native ABI expected by the production part resolver.
    A resolver=image+0x6508340,page=resolver&~A(0xffff);void* allocation=VirtualAlloc(reinterpret_cast<void*>(page),65536,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE);assert(allocation==reinterpret_cast<void*>(page));
    unsigned char thunk[]={0x48,0xb8,0,0,0,0,0,0,0,0,0xff,0xe0};A target=reinterpret_cast<A>(&testPart);std::memcpy(thunk+2,&target,8);std::memcpy(reinterpret_cast<void*>(resolver),thunk,sizeof(thunk));FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(resolver),sizeof(thunk));
    std::vector<A> controller(8),link(2);controller[0]=image+0xb485368;controller[7]=reinterpret_cast<A>(link.data());link[1]=image+0xf50ab30;entities={reinterpret_cast<A>(controller.data())};
    TestScene bowl(1536),deck(1536),sideline(60);for(auto& p:deck.xyz)p.y+=15;testScenes={&bowl,&deck,&sideline};
    assert(guardedApply(false,false,true));assert(shownSeatPatterns==2&&shownSeatFallbacks==1&&!seatPatternPending);
    assert(startingCrowds.at(bowl.render()).selection.policy==attendance::seats::Policy::Position);
    auto density=std::atomic_load(&renderedOccupancy);
    assert(density->owns(123,7)&&density->groups==2&&density->seats==3072);
    assert(density->original==startingCrowds.at(bowl.render()).selection.original+startingCrowds.at(deck.render()).selection.original);
    float peakDensity=density->peak();
    // Reproduce a full/live-equivalent replay after every play. A replay
    // timestamp change alone must not force a second full stadium traversal.
    assert(guardedApply(false,true));visibilityGeneration=entityGeneration.load();
    auto passes=forcedCrowdPasses.load();auto skips=unchangedReplaySkips.load();
    for(int play=0;play<20;++play){replayLoss=0;assert(refreshReplayVisibility());replayLoss=-1;assert(refreshReplayVisibility());}
    assert(forcedCrowdPasses==passes&&unchangedReplaySkips==skips+40);
    replayLoss=60;assert(refreshReplayVisibility());assert(forcedCrowdPasses>passes);passes=forcedCrowdPasses;
    assert(refreshReplayVisibility()&&forcedCrowdPasses==passes);for(auto* scene:testScenes)checkScene(*scene,60);
    replayLoss=-1;exactVisibility=true;assert(refreshReplayVisibility());exactVisibility=false;assert(forcedCrowdPasses>passes);for(auto* scene:testScenes)checkScene(*scene,0);passes=forcedCrowdPasses;
    // New controllers, retired masks, pending layouts and explicit restore
    // mode cannot be hidden by the equal-target optimization.
    ++entityGeneration;assert(refreshReplayVisibility()&&forcedCrowdPasses>passes);passes=forcedCrowdPasses;
    visibilityRetry=true;assert(refreshReplayVisibility()&&forcedCrowdPasses>passes);passes=forcedCrowdPasses;
    mode=2;assert(refreshReplayVisibility()&&forcedCrowdPasses>passes);mode=0;assert(refreshReplayVisibility());
    appliedVisibilityTarget=-1;passes=forcedCrowdPasses;assert(refreshReplayVisibility()&&forcedCrowdPasses>passes);
    // Equal nonzero live/history targets also skip. A genuinely different
    // target and a renderer that has not delivered native callbacks retry.
    arrivalMissing=30;assert(refreshReplayVisibility());passes=forcedCrowdPasses;
    replayLoss=30;assert(refreshReplayVisibility()&&forcedCrowdPasses==passes);
    replayLoss=-1;exactVisibility=true;assert(refreshReplayVisibility()&&forcedCrowdPasses==passes);exactVisibility=false;
    auto scenes=testScenes;testScenes.clear();visibilityRetry=true;assert(refreshReplayVisibility()&&visibilityRetry);passes=forcedCrowdPasses;
    assert(refreshReplayVisibility()&&forcedCrowdPasses>passes&&visibilityRetry);testScenes=scenes;assert(refreshReplayVisibility()&&!visibilityRetry);
    arrivalMissing=0;exactVisibility=true;assert(refreshReplayVisibility());exactVisibility=false;for(auto* scene:testScenes)checkScene(*scene,0);
    // A new mask buffer at 100% live attendance must invalidate the cache
    // through the natural native callback, without an attendance change.
    auto previousFullMasks=std::move(bowl.masks);bowl.masks=bowl.native;bowl.buffers();
    testUpdate(0);assert(visibilityRetry&&seatPatternPending);passes=forcedCrowdPasses;
    replayLoss=0;assert(refreshReplayVisibility()&&forcedCrowdPasses>passes&&!visibilityRetry);replayLoss=-1;
    for(auto* scene:testScenes)checkScene(*scene,0);
    auto savedMasks=bowl.masks;std::fill(bowl.masks.begin(),bowl.masks.end(),static_cast<unsigned short>(0xffff));
    {std::lock_guard<std::mutex> lock(crowdMutex);publishRenderedOccupancyLocked();}
    assert(std::atomic_load(&renderedOccupancy)->peak()==peakDensity); // Camera/current masks never redefine original seating.
    std::copy(savedMasks.begin(),savedMasks.end(),bowl.masks.begin());
    // Natural sideline visibility may be observed once even though this
    // attendance controller never visits that small draw in a baseline pass.
    auto& idleSmall=startingCrowds.at(sideline.render());idleSmall.baselineComplete=false;idleSmall.selection={};
    prepareSeatPatterns();assert(!seatPatternPending&&idleSmall.selection.policy==attendance::seats::Policy::Pending);
    idleSmall.baselineComplete=true;prepareSeatPatterns();assert(idleSmall.selection.policy==attendance::seats::Policy::Scattered);
    auto order=startingCrowds.at(bowl.render()).selection.rank;
    auto settle=[&](int permanent,int temporary){activeLoss=permanent;arrivalMissing=temporary;assert(guardedApply(true,true));for(auto* s:testScenes)checkScene(*s,(std::max)(permanent,temporary));
        auto mapped=std::atomic_load(&renderedOccupancy);if(!audioMatch){assert(!mapped->valid());return;}
        assert(mapped->owns(123,7)&&mapped->peak()==peakDensity);
        float f=attendance::occupancy::fraction(*mapped,{},123,7,(100-(std::max)(permanent,temporary))*.01f);
        assert(std::fabs(f-peakDensity*(100-(std::max)(permanent,temporary))*.01f)<.00001f);
    };
    settle(0,55);settle(0,25);settle(0,0);settle(35,45);settle(35,35);settle(35,0);
    assert(startingCrowds.at(bowl.render()).selection.rank==order);
    for(int earlier:{5,70,20,0,55,10}){replayLoss=earlier;assert(guardedApply(true,true));for(auto* s:testScenes)checkScene(*s,earlier);
        assert(std::atomic_load(&renderedOccupancy)->peak()==peakDensity&&activeLoss==35&&arrivalMissing==0);
    }
    replayLoss=-1;exactVisibility=true;assert(guardedApply(true,true));for(auto* s:testScenes)checkScene(*s,35);exactVisibility=false;
    assert(shownExpectedFans==shownVisibleFans&&shownOtherExtra==0&&shownExtraVisible==0);
    TestScene late(1536);testScenes.push_back(&late);assert(guardedApply(true,true));assert(seatPatternPending);assert(startingCrowds.at(late.render()).selection.policy==attendance::seats::Policy::Pending);
    assert(guardedApply(true,true));assert(!seatPatternPending&&shownSeatPatterns==3);for(auto* s:testScenes)checkScene(*s,35);
    // New visibility buffer at the same renderer must recapture native seats,
    // never reinterpret old plugin holes as permanent original empties.
    auto old=std::move(bowl.masks);bowl.masks=bowl.native;bowl.buffers();assert(guardedApply(true,true));assert(seatPatternPending);assert(guardedApply(true,true));assert(!seatPatternPending);
    assert(startingCrowds.at(bowl.render()).selection.rank==order);checkScene(bowl,35);
    // A replacement renderer may supply all-visible masks and inputs. The
    // identical authored geometry retains the match's original empty positions.
    auto originalInputs=bowl.native;bowl.native.assign(bowl.native.size(),0);
    auto replacedMasks=std::move(bowl.masks);bowl.masks=bowl.native;bowl.buffers();
    assert(guardedApply(true,true));assert(guardedApply(true,true));
    assert(startingCrowds.at(bowl.render()).selection.rank==order);
    assert(std::atomic_load(&renderedOccupancy)->peak()==peakDensity);checkScene(bowl,35);
    bowl.native=originalInputs;
    // Retired crowd cannot keep other renderers perpetually pending.
    testScenes.pop_back();testPut(late.address(),A(0));assert(guardedApply(true,true));assert(startingCrowds.find(late.render())==startingCrowds.end());
    // Scenario cleanup must win over a stale exact/historical replay request.
    mode=2;replayLoss=70;exactVisibility=true;assert(guardedApply(false,true));for(auto* s:testScenes)checkScene(*s,0);
    resetMatch();assert(startingCrowds.empty()&&!seatPatternPending&&shownSeatPatterns==0&&!std::atomic_load(&renderedOccupancy)->valid());
    // A genuine new game's native renderer seeds absent model-part bits too.
    bowl.masks=bowl.native;bowl.buffers();
    mode=0;assert(guardedApply(false,false,true));assert(startingCrowds.at(bowl.render()).selection.rank==order);settle(60,0);
    mode=2;assert(guardedApply(false,true));
    // Late renderer: the intro's single arrival window was consumed while no
    // native crowd existed. Repeated PREgame decisions must finish setup now,
    // without a quarter transition, play selection or a first snap.
    resetMatch();mode=0;entities.clear();testScenes.clear();TestWorld world;originalPeriod=testPeriod;lastDecision=0;
    testReplaySelectors(world);
    testAudioCameraCallbacks(world);
    testEmptyMode(world);
    assert(guardedDecision());int opening=arrivalMissing.load();assert(opening>0&&visibilityRetry&&arrivalModel.active);
    assert(!crowdChangeWindow.pending); // No new ordinary crowd-change window.
    TestScene intro(1536);testScenes={&intro};testUpdate(0);assert(seatPatternPending);originalPresentationUpdate=testPresentation;
    entities={reinterpret_cast<A>(controller.data())};lastDecision=0;assert(guardedDecision());
    assert(!seatPatternPending&&shownSeatPatterns==1);checkScene(intro,opening);assert(arrivalMissing==opening);
    // A native attendance callback may replace masks later in the same intro.
    auto oldIntro=std::move(intro.masks);intro.masks=intro.native;intro.buffers();lastDecision=GetTickCount64();
    presentationUpdate(reinterpret_cast<A>(world.states.data()),.016f);
    assert(!seatPatternPending);checkScene(intro,opening);
    assert(arrivalMissing==opening); // Same intro cannot advance arrivals.
    auto openingOrder=startingCrowds.at(intro.render()).selection.rank;
    // The engine can replace the match object when handing pregame to kickoff.
    // Same teams + no snap + full Q1 clock must preserve the prepared picture.
    auto kickoffMatch=world.match;world.wrapper[2]=reinterpret_cast<A>(kickoffMatch.data());
    periodChanged(reinterpret_cast<A>(world.manager.data()),1);
    assert(lastMatch==world.wrapper[2]&&arrivalMissing==opening&&arrivalModel.active&&std::atomic_load(&renderedOccupancy)->owns(lastMatch,audioEpoch.load()));
    assert(startingCrowds.at(intro.render()).selection.rank==openingOrder);checkScene(intro,opening);
    world.phase(1);lastDecision=0;assert(guardedDecision());assert(arrivalMissing==opening);
    auto activeMatch=kickoffMatch;world.wrapper[2]=reinterpret_cast<A>(activeMatch.data());lastDecision=0;
    assert(guardedDecision());assert(lastMatch==world.wrapper[2]&&arrivalMissing==opening);checkScene(intro,opening);
    // Midgame Save publishes its ordinary new-game number only at preplay,
    // after this plugin has prepared the intro. That unarmed acknowledgement
    // must not reset arrivals or refill the renderer before the opening snap.
    resumeTracker={};
    auto publishOpening=[&](int number){
        auto next=std::make_shared<attendance::resume::Signal>();next->valid=true;next->game=number;next->received=GetTickCount64();
        next->live={1,0,0,1,900,900};std::shared_ptr<const attendance::resume::Signal> value=next;std::atomic_store(&resumeSignal,value);
    };
    publishOpening(10);lastDecision=0;assert(guardedDecision());auto preparedCaptures=captureSequence;
    publishOpening(11);assert(ordinaryOpeningAcknowledgement(lastMatch,{1,0,0,1,900,900}));
    lastDecision=0;assert(guardedDecision());
    assert(resumeTracker.game==11&&arrivalMissing==opening&&matchLifecycle.prepared&&captureSequence==preparedCaptures);
    assert(startingCrowds.at(intro.render()).selection.rank==openingOrder);checkScene(intro,opening);
    std::atomic_store(&resumeSignal,std::make_shared<const attendance::resume::Signal>());
    // Quit with presentation suspended: the stage may remain 14 while the
    // secondary mode leaves 3. Then reuse the SAME match/render buffers for a
    // new intro with a different school and a completed old arrival curve.
    matchLifecycle.live();arrivalModel.finish();currentLoss=highestTarget=35;activeLoss=35;arrivalMissing=0;assert(guardedApply(true,true));
    testGlobal(0xf3fddbc,2);observeWorldExit();assert(matchLifecycle.leftWorld);testGlobal(0xf3fddbc,3);
    testGlobal(0xd9fba20,1212);testGlobal(0xd9fba24,1501);world.phase(6);testPut(reinterpret_cast<A>(world.manager.data())+8,0);lastDecision=0;
    assert(guardedDecision());int secondOpening=arrivalMissing.load();assert(secondOpening>opening&&arrivalModel.active&&!matchLifecycle.leftWorld);
    assert(activeLoss==0&&!seatPatternPending&&shownSeatPatterns==1);checkScene(intro,secondOpening);
    auto captures=captureSequence;for(int i=0;i<10;++i){lastDecision=0;assert(guardedDecision());assert(arrivalMissing==secondOpening);}
    assert(captureSequence==captures); // A ready intro does not rebuild or recapture per frame.
    // The immediate renderer retry is pregame-only. At the line, ordinary
    // changes still wait until the next selection/break.
    world.phase(1);testPut(reinterpret_cast<A>(world.manager.data())+8,1);
    auto secondBuffer=std::move(intro.masks);intro.masks=intro.native;intro.buffers();attendanceUpdate(reinterpret_cast<A>(controller.data()));
    assert(seatPatternPending);world.phase(5);lastDecision=0;assert(guardedDecision());assert(!seatPatternPending);checkScene(intro,secondOpening);
    // Exercise the actual decision/render path in offscreen Super Sim.
    std::vector<A> simController(4);simController[0]=image+attendance::bindings::SuperSimVtable;
    testGlobal(attendance::bindings::SuperSimController,reinterpret_cast<A>(simController.data()));superSimBinding=true;
    std::vector<A> replayManager(0x1040/8);replayManager[0]=image+attendance::bindings::ReplayVtable;
    testGlobal(attendance::bindings::ReplayManager,reinterpret_cast<A>(replayManager.data()));replayBinding=true;
    testPut(reinterpret_cast<A>(replayManager.data())+0x1030,100.0);
    std::vector<A> replayEntity(4);replayEntity[0]=image+attendance::bindings::ReplayEntityVtable;
    originalReplayUpdate=[](A){};
    world.phase(0);testPut(reinterpret_cast<A>(world.clock.data())+0x28,100.0f);lastDecision=0;
    replayUpdate(reinterpret_cast<A>(replayEntity.data()));assert(arrivalMissing==secondOpening); // None alone is rejected.
    testPut(reinterpret_cast<A>(simController.data())+8,1);testPut(reinterpret_cast<A>(simController.data())+12,1);lastDecision=0;
    replayUpdate(reinterpret_cast<A>(replayEntity.data()));assert(shownSuperSim&&arrivalMissing<secondOpening);checkScene(intro,arrivalMissing);
    auto observedTime=gameHistory.elapsed;
    world.paused=1;testPut(reinterpret_cast<A>(replayManager.data())+0x38,static_cast<unsigned char>(1));
    testPut(reinterpret_cast<A>(world.clock.data())+0x28,90.0f);lastDecision=0;
    replayUpdate(reinterpret_cast<A>(replayEntity.data()));assert(gameHistory.elapsed==observedTime);
    world.paused=0;testPut(reinterpret_cast<A>(replayManager.data())+0x38,static_cast<unsigned char>(0));
    replayUpdate(reinterpret_cast<A>(replayEntity.data()));
    // The live Normal/Fast trace held PlayCall for its ENTIRE play-by-play
    // sequence. Main presentation is suspended, so only replayUpdate runs.
    world.phase(5);testPut(reinterpret_cast<A>(world.manager.data())+8,2);
    testPut(world.wrapper[2]+0x1d8,35);testPut(world.wrapper[2]+0x1dc,0);
    auto simulateClock=[&](float clock){
        testPut(reinterpret_cast<A>(world.clock.data())+0x28,clock);lastDecision=0;
        replayUpdate(reinterpret_cast<A>(replayEntity.data()));assert(shownSuperSim);
        checkScene(intro,(std::max)(activeLoss.load(),arrivalMissing.load()));
    };
    simulateClock(900);assert(arrivalMissing==0&&arrivalModel.done);int beforeSim=activeLoss;
    attendance::State departureTiming;departureTiming.variationSeed=attendanceSeed;
    float beforeSeatBreak=900*attendance::context::halftimeTiming(departureTiming).departure+1;
    for(float clock:{600.0f,300.0f,beforeSeatBreak})simulateClock(clock);
    assert(shownHalftime==0&&activeLoss>beforeSim); // Permanent exits happen during the quarter.
    simulateClock(100);assert(shownQuarter==2&&halftimeModel.active&&shownHalftime>0);
    simulateClock(60);int earlyAbsent=shownHalftime;
    simulateClock(0);assert(shownHalftime>=earlyAbsent);int peakAbsent=shownHalftime;
    testPut(reinterpret_cast<A>(world.manager.data())+8,3);simulateClock(900);
    assert(shownHalftime>0&&shownHalftime<=peakAbsent);
    simulateClock(850);assert(shownHalftime>0&&shownHalftime<=peakAbsent);
    simulateClock(800);assert(shownHalftime>0&&shownHalftime<peakAbsent);
    // Midgame Save's next game number can arrive at the first manually played
    // snap AFTER a half of Super Sim. This is not an armed scenario load.
    auto lateNumber=std::make_shared<attendance::resume::Signal>();lateNumber->valid=true;lateNumber->game=12;lateNumber->received=GetTickCount64();
    lateNumber->live={3,35,0,1,800,900};
    std::shared_ptr<const attendance::resume::Signal> lateSignal=lateNumber;std::atomic_store(&resumeSignal,lateSignal);
    auto simEpoch=audioEpoch.load();auto simCaptures=captureSequence;auto simPeak=std::atomic_load(&renderedOccupancy)->peak();
    auto simArchive=attendanceArchive.active;lastDecision=0;assert(guardedDecision());
    assert(resumeTracker.game==12&&audioEpoch==simEpoch&&captureSequence==simCaptures&&attendanceArchive.active==simArchive);
    assert(std::atomic_load(&renderedOccupancy)->peak()==simPeak&&shownHalftime>0);
    std::atomic_store(&resumeSignal,std::make_shared<const attendance::resume::Signal>());
    attendance::State returnTiming;returnTiming.variationSeed=attendanceSeed;
    float returnDeadline=900*(1-attendance::context::halftimeTiming(returnTiming).lastReturn)-1;
    simulateClock(returnDeadline);assert(shownQuarter==3&&shownHalftime==0&&arrivalMissing==0&&halftimeModel.done);
    testPut(reinterpret_cast<A>(world.manager.data())+8,4);simulateClock(900);int q4SimStart=activeLoss;
    testPut(world.wrapper[2]+0x1d8,70);
    for(float clock:{800.0f,600.0f,300.0f,200.0f})simulateClock(clock);
    assert(shownTeam1>=1500&&activeLoss>q4SimStart&&activeLoss-q4SimStart<=45&&activeLoss<=75);
    auto lastSimTime=gameHistory.elapsed;int simLoss=activeLoss;
    world.paused=1;testPut(reinterpret_cast<A>(world.clock.data())+0x28,195.0f);lastDecision=0;
    replayUpdate(reinterpret_cast<A>(replayEntity.data()));assert(gameHistory.elapsed==lastSimTime&&activeLoss==simLoss);
    world.paused=0;
    auto historyBefore=attendanceArchive.revision;assert(simulationRecording==100);
    assert(guardedRecordAttendance());assert(attendanceArchive.revision==historyBefore);
    int afterSim=activeLoss;testPut(reinterpret_cast<A>(simController.data())+8,0);testPut(reinterpret_cast<A>(simController.data())+12,0);
    world.phase(1);testPut(reinterpret_cast<A>(world.clock.data())+0x28,190.0f);lastDecision=0;assert(guardedDecision());
    assert(!shownSuperSim&&activeLoss==afterSim); // No hidden pending window at the visible line.
    lastDecision=17;replayUpdate(reinterpret_cast<A>(replayEntity.data()));assert(lastDecision==17&&activeLoss==afterSim);
    assert(guardedRecordAttendance());assert(attendanceArchive.revision==historyBefore); // Frozen filming time preserves the earlier clip.
    world.phase(3);testPut(reinterpret_cast<A>(world.clock.data())+0x28,180.0f);lastDecision=0;assert(guardedDecision());assert(activeLoss==afterSim);
    world.phase(5);lastDecision=0;assert(guardedDecision());assert(activeLoss>=afterSim);checkScene(intro,(std::max)(activeLoss.load(),arrivalMissing.load()));
    lastDecision=23;replayUpdate(reinterpret_cast<A>(replayEntity.data()));assert(lastDecision==23); // Ordinary PlayCall is not offscreen Sim.
    testPut(reinterpret_cast<A>(replayManager.data())+0x1030,100.1);
    assert(guardedRecordAttendance());assert(attendanceArchive.revision>historyBefore&&simulationRecording<0);
    // Final presentation streams a new seated crowd after live play ends.
    // It must inherit the final picture without a new arrival/departure step.
    world.phase(7);testPut(reinterpret_cast<A>(world.manager.data())+8,6);testPut(reinterpret_cast<A>(world.clock.data())+0x28,0.0f);
    TestScene finalScene(1536);testScenes={&finalScene};int finalLoss=activeLoss,finalMissing=arrivalMissing;auto finalHazard=departureModel.hazard;
    attendanceUpdate(reinterpret_cast<A>(controller.data()));assert(seatPatternPending&&postgameRendererRetry);
    lastDecision=GetTickCount64();presentationUpdate(reinterpret_cast<A>(world.states.data()),.016f);
    assert(!seatPatternPending&&!visibilityRetry);checkScene(finalScene,(std::max)(finalLoss,finalMissing));
    assert(activeLoss==finalLoss&&arrivalMissing==finalMissing&&departureModel.hazard==finalHazard);
    auto finalCaptures=captureSequence;for(int i=0;i<5;++i)presentationUpdate(reinterpret_cast<A>(world.states.data()),.016f);
    assert(captureSequence==finalCaptures); // A ready postgame does not recapture every frame.
    world.phase(8);TestScene finalTransition(1536);testScenes={&finalTransition};
    attendanceUpdate(reinterpret_cast<A>(controller.data()));assert(postgameRendererRetry);
    lastDecision=GetTickCount64();presentationUpdate(reinterpret_cast<A>(world.states.data()),.016f);
    assert(!seatPatternPending&&!visibilityRetry);checkScene(finalTransition,(std::max)(finalLoss,finalMissing));
    assert(activeLoss==finalLoss&&departureModel.hazard==finalHazard); // EndOfGame + QuarterEnd uses the same final-only repair.
    mode=2;assert(guardedApply(false,true));
    // A late blowout queues the new catch-up in the actual production
    // decision path, but seats stay fixed until play selection is entered.
    resetMatch();mode=0;TestScene q4Scene(1536);testScenes={&q4Scene};
    testGlobal(0xd9fba20,1165);testGlobal(0xd9fba24,1158);
    world.phase(6);testPut(reinterpret_cast<A>(world.manager.data())+8,0);
    testPut(world.wrapper[2]+0x1d8,0);testPut(world.wrapper[2]+0x1dc,0);
    testPut(reinterpret_cast<A>(world.clock.data())+0x28,900.0f);
    lastDecision=0;assert(guardedDecision());lastDecision=0;assert(guardedDecision());
    world.phase(5);testPut(reinterpret_cast<A>(world.manager.data())+8,4);
    testPut(world.wrapper[2]+0x1d8,14);testPut(world.wrapper[2]+0x1dc,14);
    lastDecision=0;assert(guardedDecision());int q4Opening=activeLoss,q4Visible=(std::max)(q4Opening,arrivalMissing.load());
    // This synthetic jump goes straight from pregame into Q4; any paced
    // arrival return still belongs to the visible target until another break.
    assert(!seatPatternPending);checkScene(q4Scene,q4Visible);
    world.phase(1);testPut(world.wrapper[2]+0x1dc,49);
    testPut(reinterpret_cast<A>(world.clock.data())+0x28,800.0f);lastDecision=0;assert(guardedDecision());
    for(float clock:{700.0f,600.0f,500.0f}){
        testPut(reinterpret_cast<A>(world.clock.data())+0x28,clock);lastDecision=0;assert(guardedDecision());
        assert(activeLoss==q4Opening);checkScene(q4Scene,q4Visible);
    }
    assert(departureModel.catchupQueued);world.phase(5);lastDecision=0;assert(guardedDecision());
    int bounded=(std::min)(departureModel.plannedLoss,departureModel.periodStartLoss+settings->quarterCap);
    assert(activeLoss>q4Opening&&activeLoss==bounded&&activeLoss-q4Opening<=settings->quarterCap);
    int caughtUp=activeLoss;checkScene(q4Scene,activeLoss);
    assert(!departureModel.catchupQueued); // The accepted pressure was settled once.
    lastDecision=0;assert(guardedDecision());assert(activeLoss==caughtUp);
    guardedQuarterBreak(3,4);assert(activeLoss==caughtUp); // A duplicate same-time checkpoint cannot grant another visible batch.
    // Cross the two-minute mark while lined up. The checkpoint must wait
    // for selection, settle through the ordinary verified renderer path and
    // remain once-only despite repeated callbacks during the break.
    world.phase(1);testPut(reinterpret_cast<A>(world.clock.data())+0x28,121.f);
    lastDecision=0;assert(guardedDecision());
    testPut(reinterpret_cast<A>(world.clock.data())+0x28,119.f);
    lastDecision=0;assert(guardedDecision());assert(activeLoss==caughtUp);checkScene(q4Scene,caughtUp);
    world.phase(5);lastDecision=0;assert(guardedDecision());
    // The long held lineup can earn the new late-traffic allowance. It is
    // reconciled only at this hidden break, sharing the once-only warning budget.
    int warningLoss=activeLoss;assert(warningLoss>=caughtUp&&warningLoss-caughtUp<=(std::max)(settings->transitionCap,settings->lateTrafficQuarterExtra));
    assert(warningLoss<=departureModel.periodStartLoss+settings->quarterCap+settings->lateTrafficQuarterExtra);checkScene(q4Scene,warningLoss);
    auto warningHazard=departureModel.hazard;
    for(int repeat=0;repeat<5;++repeat){lastDecision=0;assert(guardedDecision());assert(activeLoss==warningLoss&&departureModel.hazard==warningHazard);checkScene(q4Scene,warningLoss);}
    mode=2;assert(guardedApply(false,true));
    // The production decision and recorder must carry the new conference flag,
    // rather than rebuilding a generic playoff snapshot for saved replays.
    resetMatch();mode=0;conferenceBinding=true;
    // This tiny synthetic venue needs 90% for its conference-game atmosphere.
    // Make its real captured baseline full rather than the usual 6/7 fixture.
    TestScene conferenceScene(1536);for(unsigned i=0;i<1536;++i)conferenceScene.masks[i]=conferenceScene.native[i]=conferenceScene.original[i]=static_cast<unsigned short>((i&1)?4:0);
    testScenes={&conferenceScene};
    std::vector<A> visualManager(0x100/8),visualReference(2),visual(8);
    testGlobal(attendance::bindings::MatchVisualManager,reinterpret_cast<A>(visualManager.data()));
    visualManager[4]=reinterpret_cast<A>(visualReference.data());visualReference[1]=reinterpret_cast<A>(visual.data());
    visual[0]=image+attendance::bindings::MatchVisualVtable;testPut(reinterpret_cast<A>(visual.data())+0x27,static_cast<unsigned char>(1));
    world.phase(6);testPut(reinterpret_cast<A>(world.manager.data())+8,0);
    testPut(world.wrapper[2]+0x1d8,0);testPut(world.wrapper[2]+0x1dc,0);
    testPut(reinterpret_cast<A>(world.clock.data())+0x28,900.0f);
    lastDecision=0;assert(guardedDecision());assert(shownConference&&shownConferenceKnown&&shownPlayoff);
    assert(!attendanceArchive.games.empty()&&attendanceArchive.games.back().context.conferenceChampionship);
    world.phase(1);testPut(reinterpret_cast<A>(world.manager.data())+8,1);
    testPut(reinterpret_cast<A>(replayManager.data())+0x1030,100.2);
    assert(guardedRecordAttendance());assert(!attendanceArchive.games.back().points.empty());
    assert(attendanceArchive.games.back().points.back().state.conferenceChampionship);
    // A live scoreboard/clock correction is rejected by event detection, but
    // cannot briefly mute an otherwise valid lineup. Pause still wins.
    activeLoss=0;arrivalMissing=0; // This audio-continuity fixture needs a sufficiently full current crowd.
    playBindingsAvailable=false;atmosphereMatch=0;world.phase(5);lastAtmosphereTick=0;atmosphereTick();
    world.phase(1);lastAtmosphereTick=0;atmosphereTick();lineupAtmosphere.began=GetTickCount64()-4000;
    playTracker.seen=true;playTracker.highHome=1;auto oldCelebration=coreAtmosphere.celebration;
    lastAtmosphereTick=0;atmosphereTick();auto audibleIntent=std::atomic_load(&sustainedIntent);
    assert(audibleIntent->active&&audibleIntent->reason==attendance::sustained::Reason::Lineup&&audibleIntent->intensity>0);
    assert(coreAtmosphere.celebration==oldCelebration); // No score/eruption is invented from the rejected sample.
    arrivalMissing=81;world.phase(5);lastAtmosphereTick=0;atmosphereTick();world.phase(1);lastAtmosphereTick=0;atmosphereTick();
    auto smallCrowd=std::atomic_load(&sustainedIntent);assert(smallCrowd->active&&smallCrowd->intensity==0&&smallCrowd->reason==attendance::sustained::Reason::None);
    assert(std::atomic_load(&cameraCue)->kind==attendance::motion::Kind::None);arrivalMissing=0;
    world.paused=1;lastAtmosphereTick=0;atmosphereTick();assert(!std::atomic_load(&sustainedIntent)->active&&!lineupAtmosphere.carrying);world.paused=0;
    // Actual production audio context: paused intro, historical automatic clip,
    // normalized final cutscene and final-menu cleanup. No native sound is sent.
    auto oldRendered=std::atomic_load(&renderedOccupancy);
    std::shared_ptr<const attendance::occupancy::Rendered> fullSeats=std::make_shared<attendance::occupancy::Rendered>(attendance::occupancy::Rendered{audioMatch.load(),audioEpoch.load(),1,56099,56099});
    std::atomic_store(&renderedOccupancy,fullSeats);activeLoss=arrivalMissing=0;audioAudible=true;lastAudibleUpdate=GetTickCount64();
    coreAtmosphere.reset();presentationAtmosphere.reset();playTracker.reset();
    world.phase(6);testPut(reinterpret_cast<A>(world.manager.data())+8,0);testPut(world.wrapper[2]+0x1d8,0);testPut(world.wrapper[2]+0x1dc,0);
    testPut(reinterpret_cast<A>(world.clock.data())+0x28,900.f);world.paused=1;lastAtmosphereTick=0;atmosphereTick();
    auto introAudio=wantedSwell();assert(introAudio.active&&introAudio.reason==attendance::sustained::Reason::Opening&&!introAudio.shake&&!introAudio.cheers);
    assert(playTracker.serial==0); // Intro did not invent a play or score.
    assert(playTracker.openingSeen&&playTracker.openingReliable&&!playTracker.seen); // Fresh intro proof survives until the first gameplay phase.
    activeLoss=80;auto liveWanted=audioWanted();assert(std::fabs(audibleFullness(liveWanted)-.2f)<.00001f);
    auto introVolume=audioBusMix();assert(introVolume.valid&&introVolume.key==attendance::balance::mix(.2f,56099).key);
    {std::shared_ptr<const attendance::occupancy::Rendered> small=std::make_shared<attendance::occupancy::Rendered>(attendance::occupancy::Rendered{audioMatch.load(),audioEpoch.load(),1,12000,12000});
        std::atomic_store(&renderedOccupancy,small);assert(audioBusMix().valid);
        small=std::make_shared<attendance::occupancy::Rendered>(attendance::occupancy::Rendered{audioMatch.load(),audioEpoch.load(),1,12001,12001});std::atomic_store(&renderedOccupancy,small);assert(audioBusMix().valid);
        std::atomic_store(&renderedOccupancy,fullSeats);}
    replayLoss=10;assert(std::fabs(audibleFullness(liveWanted)-.9f)<.00001f); // Automatic clip follows recorded crowd.
    auto clipVolume=audioBusMix();assert(clipVolume.valid&&clipVolume.key==attendance::balance::mix(.9f,56099).key);
    replaySuspendsAtmosphere=true;assert(std::fabs(audibleFullness(liveWanted)-.2f)<.00001f);
    assert(!audioBusMix().valid); // Interactive replay never inherits a category-volume override.
    replaySuspendsAtmosphere=false;replayLoss=-1;activeLoss=0;
    world.phase(1);assert(!audioBusMix().valid); // Ordinary gameplay pause restores native volume.
    // Production audio consumer: only a fresh same-world envelope can lower
    // continuous beds; all reaction buses, disabled strength and pause survive.
    world.paused=0;lastAudibleUpdate=GetTickCount64();
    auto stockMix=audioBusMix();assert(stockMix.valid);
    auto hush=std::make_shared<attendance::homequiet::Envelope>();hush->match=audioMatch.load();hush->epoch=audioEpoch.load();hush->updated=GetTickCount64();hush->quiet=1;hush->visitorShare=.06f;
    std::shared_ptr<const attendance::homequiet::Envelope> hushed=hush;std::atomic_store(&homeQuietEnvelope,hushed);
    auto bedMix=audioBusMix();assert(bedMix.valid&&bedMix.net(1)<stockMix.net(1));
    for(unsigned child:{0u,3u,5u,6u})assert(bedMix.net(child)==stockMix.net(child));
    ++hush->epoch;assert(audioBusMix().key==stockMix.key);--hush->epoch;
    hush->updated=GetTickCount64()-1000;assert(audioBusMix().key==stockMix.key);hush->updated=GetTickCount64();
    busMixStrength=0;assert(!audioBusMix().needed());busMixStrength=100;
    replaySuspendsAtmosphere=true;assert(!audioBusMix().valid);replaySuspendsAtmosphere=false;
    world.paused=1;assert(!audioBusMix().valid);
    std::shared_ptr<const attendance::homequiet::Envelope> noHush=std::make_shared<attendance::homequiet::Envelope>();std::atomic_store(&homeQuietEnvelope,noHush);
    world.phase(8);testPut(reinterpret_cast<A>(world.manager.data())+8,6);testPut(reinterpret_cast<A>(world.clock.data())+0x28,0.f);
    testPut(world.wrapper[2]+0x1d8,21);testPut(world.wrapper[2]+0x1dc,14);world.paused=0;lastAtmosphereTick=0;atmosphereTick();
    assert(audioPresentationPhase()==7&&at<int>(reinterpret_cast<A>(world.states.data())+0xa0)==8);
    auto outroAudio=wantedSwell();assert(outroAudio.active&&outroAudio.reason==attendance::sustained::Reason::Postgame&&!outroAudio.cheers&&!outroAudio.shake);
    coreAtmosphere.eruptionAt=GetTickCount64()/1000.0-1;coreAtmosphere.eruptionSeconds=45;coreAtmosphere.eruptionMinimum=.6f;coreAtmosphere.postgameHeld=true;
    replayLoss=80;lastAtmosphereTick=0;atmosphereTick();assert(coreAtmosphere.erupting&&coreAtmosphere.postgameHeld); // An early sparse clip cannot cancel the earned live celebration.
    replayLoss=-1;
    world.paused=1;lastAtmosphereTick=0;atmosphereTick();assert(!wantedSwell().active&&coreAtmosphere.postgameMenu);
    std::atomic_store(&renderedOccupancy,oldRendered);world.paused=0;
    mode=2;assert(guardedApply(false,true));
    // Exercise the actual report with the private world, including separate
    // live/settings sections and attendance audio when added layers are off.
    debugLog=false;audioHookInstalled=true;audioSetting=true;
    auto reportSettings=std::make_shared<attendance::core::Settings>(*std::atomic_load(&coreSettings));reportSettings->enabled=false;
    std::shared_ptr<const attendance::core::Settings> reportCore=reportSettings;std::atomic_store(&coreSettings,reportCore);
    std::ostringstream report;writeStatus(report,GetTickCount64());auto text=report.str();
    size_t previous=0;
    for(const char* title:{"GAME & MATCHUP","VISIBLE ATTENDANCE","CROWD AUDIO - CURRENT LEVELS","ATMOSPHERE & CAMERA - LIVE EFFECTS","REPLAYS","DYNAMIC GAMEDAY ATTENDANCE - SETTINGS","DYNAMIC GAMEDAY SOUNDSCAPE - SETTINGS","DYNAMIC GAMEDAY BROADCAST - CAMERA","SETTINGS - LOGGING"}){
        auto next=text.find(std::string("\n\n=== ")+title+" ===\n");assert(next!=std::string::npos&&next>previous);previous=next;
    }
    assert(text.find("DEBUG - TECHNICAL COUNTERS")==std::string::npos);
    auto audioLine=text.substr(text.find("Crowd audio:"),text.find('\n',text.find("Crowd audio:"))-text.find("Crowd audio:"));
    assert(audioLine.find("enabled")!=std::string::npos); // Layer switch does not disable the common attendance cut.
    assert(text.find("Crowd measured:")!=std::string::npos&&text.find("Chants gain:")!=std::string::npos);
    {std::ofstream f(logsFolder/"status-preview.txt");f<<text;}
    debugLog=true;std::ostringstream technical;writeStatus(technical,GetTickCount64());assert(technical.str().find("DEBUG - TECHNICAL COUNTERS")!=std::string::npos);debugLog=false;
    // Execute the actual native INI reader against mixed legacy/current settings.
    dataFolder=logsFolder/L"SettingsCase";std::filesystem::create_directories(dataFolder);
    {std::ofstream ini(dataFolder/L"DynamicGamedayAtmosphere.ini");ini<<"[Attendance]\nEnabled=1\n[Atmosphere]\nMaximumBoostPercent=25\nCriticalShakeStrengthPercent=8\n[Dynamic Gameday Attendance]\nEnabled=0\nArrivalsEnabled=0\nStartPercent=42\n[Dynamic Gameday Soundscape]\nEnabled=0\nLayeredAtmosphereEnabled=0\nMaximumBoostPercent=17.5\nHighLeverageBoostPercent=22\nEruptionBoostPercent=35\nChantMaximumBoostPercent=11\nLongCompletionMinimumYards=21\nDuringPlayReactionsEnabled=0\n[Dynamic Gameday Broadcast]\nCameraShake=0\nDuringPlayShakeStrengthPercent=12\nExtraReplaysEnabled=0\n";}
    readSettings();assert(!std::atomic_load(&settings)->enabled&&!arrivalEnabled&&arrivalStart==50&&!audioSetting&&!extraReplays&&busMixStrength==100);
    auto nativeConfig=std::atomic_load(&coreSettings);auto liveConfig=std::atomic_load(&liveReactionSettings);auto motionConfig=std::atomic_load(&cameraSettings);
    assert(!nativeConfig->enabled&&!nativeConfig->cameraShake&&busMaximumBoost==17.5f&&nativeConfig->maximumPercent==17.5f&&nativeConfig->highLeveragePercent==22&&nativeConfig->eruptionPercent==35&&nativeConfig->chantMaximumPercent==11);
    assert(!liveConfig->enabled&&liveConfig->minimumCatchYards==21&&motionConfig->live==12&&motionConfig->critical==8);
    // Default (non-debug) reports must retain a useful incompatible-code reason.
    debugLog=false;assert(!requiredBytes(0x3c,{0xab,0xcd}));
    log("reaction-audio","Test audit: native cheer dispatched.");log("play-summary","Test audit: play result observed.");log("late-turnover","Test audit: turnover exits queued.");log("away-motion","Test audit: visitor camera request.");
    while(queuedLogs.size())drainLogs();
    {std::ifstream f(logsFolder/"DynamicGamedayAtmosphere.log");std::string logText((std::istreambuf_iterator<char>(f)),{});assert(logText.find("Required game-code check failed at RVA 0x3c")!=std::string::npos&&logText.find("not an EXE hash check")!=std::string::npos);}
    {std::ifstream f(logsFolder/"technical.log");std::string logText((std::istreambuf_iterator<char>(f)),{});assert(logText.find("Test audit: native cheer dispatched.")!=std::string::npos&&logText.find("Test audit: play result observed.")!=std::string::npos&&logText.find("Test audit: turnover exits queued.")!=std::string::npos&&logText.find("Test audit: visitor camera request.")!=std::string::npos);}
    {std::ifstream f(logsFolder/"DynamicGamedayAtmosphere.log");std::string logText((std::istreambuf_iterator<char>(f)),{});assert(logText.find("Test audit: play result observed.")!=std::string::npos&&logText.find("Test audit: turnover exits queued.")!=std::string::npos&&logText.find("Test audit: visitor camera request.")==std::string::npos);}
    void* unused=reinterpret_cast<void*>(1);auto patchCount=patches.size();
    assert(!makePatch(0x64b3930,reinterpret_cast<void*>(testUpdate),&unused,14)); // Incomplete guarded prologue is rejected before allocation.
    assert(!makePatch(0x64b3930,reinterpret_cast<void*>(testUpdate),&unused,13));
    assert(!makePatch(0x64b3930,reinterpret_cast<void*>(testUpdate),&unused,33));
    assert(!makePatch(0x64b3930,nullptr,&unused,15)&&patches.size()==patchCount&&unused==reinterpret_cast<void*>(1));
    // Production snapshot refresh must preserve a restored Q1 crowd plan,
    // while different matches/venues and explicit fresh plans stay isolated.
    attendance::State plan{1,0,0,5,600,720};plan.team0=1153;plan.team1=1193;
    plan.startingAttendance=plan.night=true;plan.plannedKickoff=80;plan.plannedFill=30;
    plan.plannedPeak=plan.plannedCapacity=60000;plan.plannedStadium=553;plan.plannedWeek=8;plan.plannedYear=2026;
    audioMatch=123;publishSessionPlan(123,plan);auto refresh=plan;refresh.startingAttendance=false;refresh.plannedKickoff=-1;refresh.night=false;
    showContext(attendance::Settings{},refresh);auto working=refresh;working.plannedStadium=-1;applySessionPlan(working,123);
    assert(attendance::crowdPlan(working)&&working.plannedKickoff==80&&working.night);
    working=refresh;applySessionPlan(working,456);assert(!working.startingAttendance);
    working=refresh;working.plannedStadium=554;applySessionPlan(working,123);assert(!working.startingAttendance&&working.plannedStadium==554);
    for(auto p:worldPages)VirtualFree(p,0,MEM_RELEASE);VirtualFree(allocation,0,MEM_RELEASE);
    std::cout<<"PASS: production capture/visibility, cached layouts, arrivals/replays, late intro renderers, kickoff/Midgame Save opening preservation, quit/reload, phase0/5 Super Sim Q2 departures, early halftime breaks/Q3 returns, Q4 home/FCS exits, pause isolation, live-play gating, conference classification, paused-intro/final presentation audio and historical clip fullness.\n";
}
