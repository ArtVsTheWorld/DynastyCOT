#include "../Source/CannonSoundRuntime.h"
#include <cassert>
#include <cstdio>
#include <unordered_map>
using namespace attendance;
std::unordered_map<uintptr_t,unsigned char> memory;
template<class T>void put(uintptr_t p,T value){for(size_t n=0;n<sizeof(T);++n)memory[p+n]=reinterpret_cast<unsigned char*>(&value)[n];}
bool read(uintptr_t p,void* out,size_t n){for(size_t i=0;i<n;++i){if(!memory.count(p+i))return false;static_cast<unsigned char*>(out)[i]=memory[p+i];}return true;}
int main(){
    constexpr uintptr_t image=0x140000000,entity=0x100000,event=0x200000,data=0x300000,patch=0x400000,name=0x500000,row=0x600000;
    for(size_t i=0;i<=std::strlen(cannon::Patch);++i)put(name+i,cannon::Patch[i]);
    // Golden inputs from the authored graph, independent of detector constants:
    // 0x01ade220 feeds ADSR Trigger; 0x82b90c1c feeds ADSR Release.
    auto fill=[&](){put(entity,image+bindings::LegacySoundVtable);put(event+8,0x01ade220u);put(entity+0xe8,static_cast<unsigned char>(1));
        put(entity+0x38,data);put(data+0x60,patch|4);put(data+0x68,1.f);put(patch+0x18,name);put(entity+0x98,uintptr_t{0x700000});
        put(entity+0x58,row);put(entity+0x60,row+0x28);put(row+0x20,cannon::EffectParameter);put(row+0x24,3);};
    bool started=true,integerValid=true;int value=10;
    auto integer=[&](uintptr_t p,int& out){assert(p==row);out=value;return integerValid;};
    auto playing=[&](uintptr_t p){assert(p==0x700000);return started;};
    // Independent authored selector range: the stadium switch lists all nine
    // Cannon_01..Cannon_09 at slots 10..18, without any team identifier.
    for(value=-1;value<=110;++value){fill();assert(cannon::started(image,entity,event,read,integer,playing)==(value>=10&&value<=18?value:-1));}
    value=10;
    for(int failure=0;failure<13;++failure){fill();started=integerValid=true;
        if(failure==0)put(entity,uintptr_t{0});if(failure==1)put(event+8,0x82b90c1cu); // Release is not a blast, even with a playing graph.
        if(failure==2)put(entity+0xe8,static_cast<unsigned char>(0));if(failure==3)put(entity+0x98,uintptr_t{0});
        if(failure==4)started=false;if(failure==5)put(entity+0x60,row-1);if(failure==6)put(entity+0x60,row+64*0x28+1);
        if(failure==7)put(row+0x20,0u);if(failure==8)put(row+0x24,0);if(failure==9)integerValid=false;
        if(failure==10)put(patch+0x18,uintptr_t{0});if(failure==11)put(entity+0x60,row+1);
        if(failure==12)put(data+0x68,0.f);
        assert(cannon::started(image,entity,event,read,integer,playing)==-1);
    }
    fill();started=integerValid=true;
    assert(cannon::started(image,entity,event,read,integer,playing)==10);
    put(event+8,0x82b90c1cu);assert(cannon::started(image,entity,event,read,integer,playing)==-1);
    put(event+8,0x01ade220u);assert(cannon::started(image,entity,event,read,integer,playing)==10);
    cannon::Blast blast{123,1,1000,10};motion::Cue quiet;motion::Settings settings;
    assert(settings.cannon==18&&settings.live==11);
    for(int selector=10;selector<=18;++selector)for(float liveStrength:{0.f,10.f,30.f}){
        settings.live=liveStrength;blast.effect=selector;
        auto impact=cannon::merge(quiet,blast,1100,settings.cannon,true);
        assert(impact.kind==motion::Kind::Cannon&&impact.impactOnly&&impact.percent==18&&impact.until==1900);
        for(uint64_t now=1000;now<1900;now+=25)assert(motion::strength(cannon::merge(quiet,blast,now,settings.cannon,true),now)<=18.001f);
    }
    assert(cannon::merge(quiet,blast,1900,15,true).kind==motion::Kind::None);
    assert(cannon::merge(quiet,blast,999,15,true).kind==motion::Kind::None);
    assert(cannon::merge(quiet,blast,1100,15,false).kind==motion::Kind::None);
    assert(cannon::merge(quiet,blast,1100,0,true).kind==motion::Kind::None);
    assert(cannon::merge(quiet,blast,1100,NAN,true).kind==motion::Kind::None);
    assert(cannon::merge(quiet,blast,1100,7.5f,true).percent==7.5f);
    motion::Cue touchdown{123,1,1100,500,11500,motion::Kind::Touchdown,15,false,20};
    auto winner=cannon::merge(touchdown,blast,1100,15,true);assert(winner.kind==motion::Kind::Touchdown&&winner.began==500&&winner.until==11500);
    assert(cannon::merge(touchdown,blast,1100,0,true).kind==motion::Kind::Touchdown);
    std::puts("PASS: authored-graph start/release/start; independent all-nine cannon selector coverage; separate 18% default unaffected by live strength; zero/custom strength; sound/patch/parameter validation; no horn/firework triggers; bounded 900ms overlay and unchanged stronger touchdown.");
}
