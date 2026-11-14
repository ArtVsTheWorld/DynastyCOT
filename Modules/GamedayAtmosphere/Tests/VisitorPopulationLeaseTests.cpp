// Three disposable native-shaped renderers. No game process or hooks.
#include "../Source/NativeRuntime.cpp"
#include <cassert>
#include <iostream>
namespace {
template<class T> A address(T* p){return reinterpret_cast<A>(p);}
struct RendererFixture {
    static constexpr unsigned seats=640;
    std::array<A,0xa00/8> scene{};
    std::array<A,0x200/8> renderer{};
    std::array<unsigned short,seats> masks{};
    std::array<attendance::seats::Position,seats> positions{};
    std::array<A,2> holder{},value{},property{},map{};
    void bind(){
        holder={seats,address(positions.data())};value={0,address(holder.data())};
        property={image+attendance::bindings::CrowdPositionKey,address(value.data())};
        map={address(property.data()),address(property.data()+2)};
        scene[0]=image+attendance::bindings::SceneCrowdVtable;scene[0x9f0/8]=address(renderer.data());scene[0x9f8/8]=address(map.data());
        renderer[0]=image+attendance::bindings::CrowdRendererVtable;renderer[0x188/8]=address(masks.data());renderer[0x190/8]=address(masks.data()+seats);
        StartingCrowd crowd;crowd.baselineComplete=true;crowd.parts=1;crowd.scene=address(scene.data());crowd.masks=address(masks.data());crowd.geometry=address(positions.data());
        crowd.hidden.resize(seats);crowd.nativeHidden.resize(seats);crowd.supporters.verified=true;crowd.supporters.side.resize(seats);crowd.supporters.rank.resize(seats);
        crowd.selection.policy=attendance::seats::Policy::Position;crowd.supporters.home=512;crowd.supporters.away=128;
        startingCrowds.emplace(address(renderer.data()),std::move(crowd));
    }
};
}
int main(){
    image=0x140000000;audioMatch=451;audioEpoch=19;activeLoss=0;arrivalMissing=0;seatPatternPending=false;visibilityGeneration=3;entityGeneration=3;crowdRevision=10;
    std::array<RendererFixture,3> fixtures;for(auto& f:fixtures)f.bind();
    auto seed=[]{
        auto p=std::make_shared<attendance::visitors::Population>();p->match=451;p->epoch=19;p->verified=true;p->revision=10;p->updated=GetTickCount64()-3500;
        p->originalHome=p->home=1536;p->originalAway=p->away=384;
        visibilityRetry=false;std::shared_ptr<const attendance::visitors::Population> old=p;std::atomic_store(&visitorPopulation,old);
        return old;
    };
    auto old=seed();assert(!attendance::visitorcheers::eligible(*old,451,19,GetTickCount64()));
    const auto passes=forcedCrowdPasses.load();refreshVisitorPopulationProof();
    auto renewed=std::atomic_load(&visitorPopulation);assert(renewed->updated>old->updated&&renewed->home==1536&&renewed->away==384);
    assert(attendance::visitorcheers::eligible(*renewed,451,19,GetTickCount64()));
    refreshVisitorPopulationProof();assert(std::atomic_load(&visitorPopulation)==renewed);
    assert(visitorCountsEligible(*renewed,451,19,GetTickCount64()));
    visibilityRetry=true;assert(!visitorCountsEligible(*renewed,451,19,GetTickCount64()));visibilityRetry=false;
    arrivalMissing=4;assert(!visitorCountsEligible(*renewed,451,19,GetTickCount64()));arrivalMissing=0;
    auto reject=[](){refreshVisitorPopulationProof();assert(!std::atomic_load(&visitorPopulation)->verified&&visibilityRetry);};
    seed();fixtures[2].renderer[0x190/8]-=2;reject();fixtures[2].renderer[0x190/8]+=2;
    seed();fixtures[2].holder[1]+=16;reject();fixtures[2].holder[1]-=16;
    seed();auto& bad=startingCrowds.at(address(fixtures[0].renderer.data()));bad.supporters.side.pop_back();reject();bad.supporters.side.push_back(0);
    seed();bad.selection.policy=attendance::seats::Policy::Pending;reject();bad.selection.policy=attendance::seats::Policy::Position;
    seed();activeLoss=1;reject();activeLoss=0;seed();arrivalMissing=1;reject();arrivalMissing=0;
    seed();++crowdRevision;reject();--crowdRevision;seed();audioEpoch=20;reject();audioEpoch=19;
    seed();++entityGeneration;reject();--entityGeneration;assert(forcedCrowdPasses.load()==passes);
    // A late audio callback from an earlier play must never release an already
    // consumed request from this play, even after the bounded index wraps.
    auto cue=[](uint64_t seq){
        attendance::sustained::Intent i;i.match=451;i.epoch=19;i.playSequence=seq;i.opponentPositive=true;i.opponentCheer=seq+1;
        return i;
    };
    auto publishCue=[&](uint64_t seq){std::shared_ptr<const attendance::sustained::Intent> next=std::make_shared<attendance::sustained::Intent>(cue(seq));std::atomic_store(&sustainedIntent,next);};
    auto allowed=[&](attendance::sustained::Intent i){return visitorSoundPolicy(i,GetTickCount64()).opponentCheer!=0;};
    seed();refreshVisitorPopulationProof();
    publishCue(200);arrivalMissing=1;assert(!allowed(cue(200)));
    publishCue(201);assert(!allowed(cue(201)));arrivalMissing=0;
    assert(!allowed(cue(200))&&!allowed(cue(201)));
    publishCue(202);assert(allowed(cue(202)));
    publishCue(216);arrivalMissing=1;assert(!allowed(cue(216)));
    assert(!allowed(cue(200)));arrivalMissing=0;assert(!allowed(cue(216)));
    auto other=cue(216);other.match=999;assert(!allowed(other));
    other=cue(216);other.epoch=20;assert(!allowed(other));
    publishCue(217);assert(allowed(cue(217)));
    std::cout<<"PASS: three renderer settled 20% lease renewed, same-cadence reuse, malformed stream/masks/side/pending/absence/revision/context rejected, no forced native pass\n";
}
