#define WIN32_LEAN_AND_MEAN
#include "../Source/AwayAppearance.h"
#include "../Source/AppearanceAdmission.h"
#include <cassert>
#include <cstring>
#include <cstdio>
using namespace attendance;using namespace attendance::visitors;
bool memory(uintptr_t p,void* out,size_t n){SIZE_T done=0;return p>=0x10000&&ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(p),out,n,&done)&&done==n;}
template<class T>void put(uintptr_t p,T value){std::memcpy(reinterpret_cast<void*>(p),&value,sizeof(value));}
template<class T>uintptr_t address(T* p){return reinterpret_cast<uintptr_t>(p);}
int main(){
    const uintptr_t image=0x140000000;std::vector<unsigned char> component(0x80),scene(0x920),raw(10240);
    std::vector<seats::Position> points(1024);
    for(unsigned i=0;i<1024;++i){points[i]={i&1?35.f:-35.f,static_cast<float>(i/64),static_cast<float>(static_cast<int>(i%128)-64),0};
        raw[10*i]=static_cast<unsigned char>(0x80|(i%20==0?1:0));raw[10*i+6]=i<16?static_cast<unsigned char>(1+i%8):i<96?9:0;}
    for(unsigned i=16;i<96;++i)raw[10*i]&=0xfe;raw[100*10+6]=10;
    auto original=raw;uintptr_t tagHolder[]={1024,address(raw.data())},posHolder[]={1024,address(points.data())};
    uintptr_t tagValue[]={0,address(tagHolder)},posValue[]={0,address(posHolder)};
    struct Entry {uintptr_t key,value;};Entry entries[]={{image+0xb486033,address(tagValue)},{image+bindings::CrowdPositionKey,address(posValue)}};
    uintptr_t map[]={address(entries),address(entries+2)};auto c=address(component.data()),s=address(scene.data());
    put(c,image+0xb484f98);put(c+0x48,address(map));put(s+0x908,c);
    Plan plan;plan.valid=true;plan.share=.25f;
    auto prepared=appearance(memory,image,c,plan,123,true,false);assert(prepared.applied&&prepared.band==16&&raw!=original);
    // Native generation consumes the overridden side bits once. Protect tags
    // and uniforms; unrelated native flags can still change during that call.
    for(unsigned i=0;i<1024;++i){assert((raw[10*i]&1)==prepared.desired[i]);assert(raw[10*i+6]==original[10*i+6]);}
    raw[2000]|=0x10;original[2000]|=0x10;
    GeneratedAppearance generated{c,address(map),address(raw.data()),address(points.data()),1153,1193,prepared.desired};
    assert(restoreAppearance(memory,image,prepared)&&raw==original);
    auto tags=agents::decode(raw,1024);agents::protectStudents(tags,true,false);
    assert(generated.apply(memory,image,s,1153,1193,tags)&&tags.side==prepared.desired);
    assert(!generated.apply(memory,image,s,1193,1153,tags));
    // A second fixture begins from EA's flags, not the previous visitor mix.
    plan.share=.05f;auto next=appearance(memory,image,c,plan,321,true,false);assert(next.applied&&next.native.side==prepared.native.side);
    assert(restoreAppearance(memory,image,next)&&raw==original);
    auto native=plan;native.native=true;assert(!appearance(memory,image,c,native,1).applied&&raw==original);
    // Never restore into a different/retired property stream.
    auto changed=appearance(memory,image,c,plan,321,true,false);assert(changed.applied);
    std::vector<unsigned char> replacement=original;tagHolder[1]=address(replacement.data());auto copy=replacement;
    assert(!restoreAppearance(memory,image,changed)&&replacement==copy);
    assert(!generated.apply(memory,image,s,1153,1193,tags));
    tagHolder[1]=address(raw.data());assert(restoreAppearance(memory,image,changed)&&raw==original);
    std::vector<unsigned char> occupied(1024,0);for(unsigned i=200;i<600;++i)occupied[i]=1;
    plan.share=.25f;auto sparse=appearance(memory,image,c,plan,123,true,false,&occupied);
    assert(sparse.applied&&sparse.occupiedVerified&&sparse.spectators==400&&sparse.occupiedVisitors==100);
    assert(restoreAppearance(memory,image,sparse)&&raw==original);
    for(int side:{0,1}){
        Plan ignored;auto one=appearance(memory,image,c,ignored,123,true,false,&occupied,side);assert(one.applied);
        for(unsigned i=0;i<1024;++i){if(agents::regular(one.native.type[i])&&!one.native.protectedFan(i))assert(one.desired[i]==side);else assert(one.desired[i]==one.native.side[i]);}
        assert(restoreAppearance(memory,image,one)&&raw==original);
    }
    // Postseason admission reaches the very same native-generation preparation,
    // preserves native turnout, protected selectors and occupied-only budget.
    for(unsigned i=0;i<1024;++i){unsigned row=i/32,col=i%32;points[i]={35.f+float(row)*.9f,float(row)*.7f,float(col)*.65f-10.4f,0};raw[10*i]=static_cast<unsigned char>(0x80u|unsigned(col>=16));}
    original=raw;turnout::Seed ps;ps.home.id=1153;ps.away.id=1193;ps.stadium=615;ps.capacity=65000;ps.ea=65000;ps.bowlFixture=true;ps.regular=false;
    ps.home.audienceFans=100000;ps.away.audienceFans=75000;ps.home.prestige=ps.away.prestige=8;ps.home.prestigeKnown=ps.away.prestigeKnown=true;
    turnout::Launch pl;pl.home=ps.home.id;pl.away=ps.away.id;pl.stadium=615;pl.capacity=pl.ea=65000;pl.bowl=47;pl.neutral=true;
    turnout::CrowdFixture cf{ps,pl,true};assert(appearanceAdmission(cf,1153,1193,true)==AppearanceStage::Prepared);
    auto pp=attendance::visitors::plan(cf,1153,1193);assert(pp.valid&&!pp.native&&!turnout::turnoutEligible(ps,pl));
    auto post=appearance(memory,image,c,pp,321,true,false,&occupied);assert(post.applied&&!post.nativeFallback&&post.occupiedVerified&&post.spectators==400&&ps.ea==65000&&pl.ea==65000);
    for(unsigned i=0;i<1024;++i)if(post.native.protectedGroup(i))assert(post.desired[i]==post.native.side[i]);
    assert(restoreAppearance(memory,image,post)&&raw==original);
    std::puts("PASS: native selector preparation/restoration, protected/empty-seat behavior, owned postseason fixture admission through native appearance, unchanged EA turnout, generated identities and retired-stream rejection; self-process memory only");
}
