// Production palette, baseline, visibility and supporter-budget paths run in
// this executable's private objects only. No game handle/injection is used.
#include "../Source/NativeRuntime.cpp"
#include <cassert>
#include <iostream>
namespace {
template<class T>void put(A address,T value){std::memcpy(reinterpret_cast<void*>(address),&value,sizeof(value));}
unsigned __fastcall resolvePart(A,unsigned part){return part;}
struct Scene {
    static constexpr unsigned n=2048;
    std::vector<A> object=std::vector<A>(0xa00/8),renderer=std::vector<A>(0x200/8),component=std::vector<A>(0x80/8);
    std::vector<unsigned short> masks=std::vector<unsigned short>(n),original=std::vector<unsigned short>(n);
    std::vector<unsigned char> raw=std::vector<unsigned char>(n*10);
    std::vector<attendance::seats::Position> coordinates=std::vector<attendance::seats::Position>(n);
    std::array<A,2> tagHolder{},pointHolder{},tagValue{},pointValue{},map{};
    std::array<A,4> entries{};
    unsigned occupied=0;Scene(){
        for(unsigned i=0;i<n;++i){coordinates[i]={28.f+static_cast<float>((i/32)%32),1.f+static_cast<float>(i/32)*.5f,-48.f+static_cast<float>(i%32)*3.f,0};
            original[i]=masks[i]=(i>=16&&i%5<2)?1:0;raw[i*10+6]=i<16?1:0;
            if(i>=16&&!original[i])++occupied;
        }
        tagHolder={n,reinterpret_cast<A>(raw.data())};pointHolder={n,reinterpret_cast<A>(coordinates.data())};
        tagValue={0,reinterpret_cast<A>(tagHolder.data())};pointValue={0,reinterpret_cast<A>(pointHolder.data())};
        entries={image+0xb486033,reinterpret_cast<A>(tagValue.data()),image+attendance::bindings::CrowdPositionKey,reinterpret_cast<A>(pointValue.data())};
        map={reinterpret_cast<A>(entries.data()),reinterpret_cast<A>(entries.data()+4)};
        put(address(),image+attendance::bindings::SceneCrowdVtable);put(address()+0x908,parts());put(address()+0x9f0,render());put(address()+0x9f8,reinterpret_cast<A>(map.data()));
        put(parts(),image+0xb484f98);put(parts()+0x48,reinterpret_cast<A>(map.data()));
        put(render(),image+attendance::bindings::CrowdRendererVtable);put(render()+0x188,reinterpret_cast<A>(masks.data()));put(render()+0x190,reinterpret_cast<A>(masks.data()+n));
    }
    A address()const{return reinterpret_cast<A>(object.data());}A render()const{return reinterpret_cast<A>(renderer.data());}A parts()const{return reinterpret_cast<A>(component.data());}
};
Scene* testSceneCurrent=nullptr;
void __fastcall setPart(A crowd,unsigned part,unsigned agent,bool hidden){assert(testSceneCurrent&&crowd==testSceneCurrent->address());auto flag=static_cast<unsigned short>(1u<<part);auto& mask=testSceneCurrent->masks[agent];mask=static_cast<unsigned short>((mask&~flag)|(hidden?flag:0));}
}
int main(){
    image=0x140000000;A function=image+attendance::bindings::CrowdPartResolve,page=function&~A(0xffff);
    void* owned=VirtualAlloc(reinterpret_cast<void*>(page),65536,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE);assert(owned==reinterpret_cast<void*>(page));
    unsigned char thunk[]={0x48,0xb8,0,0,0,0,0,0,0,0,0xff,0xe0};A target=reinterpret_cast<A>(resolvePart);std::memcpy(thunk+2,&target,8);std::memcpy(reinterpret_cast<void*>(function),thunk,sizeof(thunk));FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(function),sizeof(thunk));
    originalSet=setPart;enabled=true;mode=0;shownTeam0=1502;shownTeam1=1145;shownSpring=true;audioMatch=42;audioEpoch=1;
    // Failing initialization order: native generation has already authored a
    // sparse renderer, then initializes model parts with false before GDA's
    // completed baseline pass. The former first-input policy erased every
    // native empty bit and recorded a full stadium. Exercise fresh and resume
    // (the latter has permanent loss already loaded) without deleting history.
    for(bool resumed:{false,true}){
        Scene initial;testSceneCurrent=&initial;StartingCrowd capture;capture.scene=initial.address();capture.masks=reinterpret_cast<A>(initial.masks.data());
        capture.hidden=initial.original;capture.nativeHidden=capture.hidden;capture.nativeSeen.resize(Scene::n);capture.temporary.resize(Scene::n);
        assert(attendance::seats::retainInitialHidden(capture.hidden,capture.nativeSeen));
        startingCrowds.emplace(initial.render(),std::move(capture));activeLoss=resumed?3:0;
        for(unsigned i=0;i<Scene::n;++i)visibility(initial.address(),0,i,false);
        auto& captured=startingCrowds.at(initial.render());
        // A direct native renderer write at the end of attendance has no
        // corresponding Scene callback. Include it before finalization.
        assert(attendance::seats::finishInitialMasks(captured.hidden,initial.original));captured.baselineComplete=true;
        prepareSeatPatterns();assert(captured.selection.original==initial.occupied);
        for(unsigned i=0;i<Scene::n;++i){visibility(initial.address(),0,i,false);visibility(initial.address(),2,i,false);}
        unsigned empty=0;for(unsigned i=16;i<Scene::n;++i)if(initial.original[i]){assert(initial.masks[i]==5);++empty;}
        assert(empty&&captured.selection.original+empty==Scene::n-16);
        // Re-entering generation / resumed acknowledgement cannot replace the
        // immutable snapshot with current thinned or all-visible masks.
        auto original=captured.hidden;std::vector<unsigned short> saved=original;
        unsigned short restoredParts=captured.parts;std::vector<unsigned short> restoredSeen(Scene::n);
        assert(originalSeatLayouts.restore(initial.coordinates,saved,restoredSeen,restoredParts)&&saved==original);
        startingCrowds.clear();originalSeatLayouts.reset();
    }
    for(unsigned short fill:{static_cast<unsigned short>(0),static_cast<unsigned short>(0xffff)}){std::vector<unsigned short> defaults(Scene::n,fill),seen(Scene::n);assert(!attendance::seats::retainInitialHidden(defaults,seen));for(auto v:seen)assert(v==0);}
    // Reproduce the captured live mask pattern: initialization callbacks had
    // left only inactive-model bits (0x1040), while the native renderer's
    // completed mask contained the real empty position (0x3253).
    {std::vector<unsigned short> callbacks{0,0x1040,0},finished{0,0x3253,0x1040};
     assert(attendance::seats::finishInitialMasks(callbacks,finished));std::vector<unsigned short> seen(3);
     attendance::seats::freezeEmptyPositions(callbacks,seen,0x3253);
     assert(callbacks[0]==0&&callbacks[1]==0xffff&&callbacks[2]==0x1040);}
    activeLoss=0;
    Scene scene;testSceneCurrent=&scene;StartingCrowd b;b.scene=scene.address();b.masks=reinterpret_cast<A>(scene.masks.data());b.hidden=scene.original;b.nativeHidden=b.hidden;b.nativeSeen.assign(Scene::n,1);b.temporary.assign(Scene::n,0);b.parts=1;b.baselineComplete=true;
    startingCrowds.emplace(scene.render(),std::move(b));prepareSeatPatterns();auto& base=startingCrowds.at(scene.render());
    assert(base.selection.original==scene.occupied&&base.supporters.home==scene.occupied&&base.supporters.away==0);
    auto originalDensity=std::atomic_load(&renderedOccupancy);assert(originalDensity->original==scene.occupied);
    // A generated actual-school palette can expose model parts that the
    // practice-squad palette never used. Native generation calls these visible
    // even at authored empty seats. The immutable audience must still win.
    attendance::visitors::Plan unused;auto palette=attendance::visitors::appearance(read,image,scene.parts(),unused,123,false,false,nullptr,1);assert(palette.applied);
    for(unsigned i=0;i<Scene::n;++i)visibility(scene.address(),2,i,false);
    assert(attendance::visitors::restoreAppearance(read,image,palette));
    unsigned resurrected=0;
    for(unsigned i=16;i<Scene::n;++i)if(scene.original[i]&&(scene.masks[i]&5)!=5)++resurrected;
    if(resurrected)std::cerr<<"REGRESSION: "<<resurrected<<" original empty seats became visible after a palette model-part change.\n";
    assert(resurrected==0);
    // Recolor/rebuild cannot change the number of original occupied seats.
    auto generated=std::make_shared<attendance::visitors::GeneratedAppearance>();
    generated->component=scene.parts();generated->map=palette.map;generated->selectors=palette.stream;generated->points=palette.coordinateStream;
    generated->home=1502;generated->away=1145;generated->side=palette.desired;generatedCrowds[0]=generated;
    base.selection={};prepareSeatPatterns();assert(base.selection.original==scene.occupied);
    assert(base.supporters.home==0&&base.supporters.away==scene.occupied);
    // A second generation with yet another part must not depend on reloading
    // the renderer/geometry. Occupied positions accept the new clothing.
    for(unsigned part:{4u,9u})for(unsigned i=0;i<Scene::n;++i){visibility(scene.address(),part,i,false);
        auto flag=static_cast<unsigned short>(1u<<part);assert(bool(scene.masks[i]&flag)==bool(scene.original[i]));}
    activeLoss=25;arrivalMissing=0;prepareVisitorLosses();assert(sharedFanBudgetApplied);
    for(unsigned i=0;i<Scene::n;++i)for(unsigned part:{0u,2u,4u,9u})visibility(scene.address(),part,i,false);
    uint64_t visible=0,expected=scene.occupied-static_cast<uint64_t>(std::llround(scene.occupied*.25));
    for(unsigned i=0;i<Scene::n;++i){
        if(i<16){assert((scene.masks[i]&base.parts)!=base.parts);continue;} // Native band untouched.
        bool occupied=(scene.masks[i]&base.parts)!=base.parts;
        if(scene.original[i])assert(!occupied);else visible+=occupied;
    }
    if(visible!=expected||sharedFanTarget!=expected)std::cerr<<"Visible "<<visible<<" / target "<<sharedFanTarget<<" / expected "<<expected<<" original "<<scene.occupied<<"\n";
    assert(visible==expected&&sharedFanTarget==expected);
    auto finalDensity=std::atomic_load(&renderedOccupancy);assert(finalDensity->original==scene.occupied&&finalDensity->seats==Scene::n-16);
    float visualFullness=static_cast<float>(visible)/static_cast<float>(finalDensity->seats);
    float baselineFullness=attendance::occupancy::fraction(*finalDensity,{},42,1,.75f);
    assert(std::abs(visualFullness-baselineFullness)<=1.f/static_cast<float>(finalDensity->seats));
    attendance::visitors::Population count;count.verified=true;count.match=42;count.epoch=1;count.originalAway=scene.occupied;count.away=visible;
    assert(count.owns(42,1));
    std::shared_ptr<const attendance::visitors::Population> liveCount=std::make_shared<attendance::visitors::Population>(count);std::atomic_store(&visitorPopulation,liveCount);
    // Exercise the production density calculation; its private dummy world is
    // inactive, so no native sound/camera call is made.
    assert(std::abs(audioWanted().remaining-visualFullness)<.000001f);
    count.away=Scene::n;assert(!count.owns(42,1)&&count.share()==0);
    liveCount=std::make_shared<attendance::visitors::Population>(count);std::atomic_store(&visitorPopulation,liveCount);
    assert(std::abs(audioWanted().remaining-baselineFullness)<.000001f);
    // A replacement renderer starts all-visible but reuses the immutable
    // authored layout. Resetting for a genuinely new match permits new turnout.
    std::vector<unsigned short> replacement(Scene::n),seen(Scene::n);unsigned short parts=base.parts;
    assert(originalSeatLayouts.restore(scene.coordinates,replacement,seen,parts));
    for(unsigned i=16;i<Scene::n;++i)if(scene.original[i])assert(replacement[i]==0xffff&&seen[i]==0xffff);
    attendance::seats::Baselines newMatch;newMatch.remember(scene.coordinates,std::vector<unsigned short>(Scene::n),parts);
    assert(newMatch.restore(scene.coordinates,replacement,seen,parts));for(auto mask:replacement)assert(mask==0);
    VirtualFree(owned,0,MEM_RELEASE);
    std::cout<<"PASS: spring palette model-part changes preserve native empty seats, original spectator/audio denominator, exact shared thinning, band protection, repeated generation/stream reload and new-match turnout; invalid overpopulation rejected. Visible "<<visible<<" / "<<finalDensity->seats<<" equals modeled fullness within one seat; private self-process objects only.\n";
}
