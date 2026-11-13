#pragma once
#include "SpringGame.h"
#include "SustainedCrowd.h"
namespace attendance::spring {
// Own the short request between the result callback and native sound updates.
// Fresh input refreshes ownership, never the original request deadline.
struct Reactions {
    uintptr_t match=0;unsigned epoch=0;uint64_t handled=0,id=0,until=0,sequence=0,lastTime=0;
    int type=91,offense=-1;
    void reset(){*this=Reactions{};}
    void suspend(){id=until=0;}
    void interrupt(){suspend();handled=sequence;}
    struct Update {sustained::Intent intent;bool selected=false,expired=false;};
    Update observe(const energy::Play& p,const energy::Outcome& r,uintptr_t owner,unsigned generation,uint64_t serial,uint64_t now,bool enabled,int snapOffense,bool clockPlay=false){
        if(match!=owner||epoch!=generation){reset();match=owner;epoch=generation;}
        Update out;auto& w=out.intent;w.match=owner;w.epoch=generation;w.updated=now;w.spring=true;
        w.populationSize=w.size=2;w.playSequence=serial;
        if(now<lastTime){suspend();handled=serial;}lastTime=now;
        if(sequence!=serial){suspend();sequence=serial;}
        const bool phase=p.game.phase==1||p.game.phase==3||p.game.phase==4||p.game.phase==5;
        w.active=enabled&&owner&&generation&&serial&&verified(p.game)&&!p.simulated&&!clockPlay&&phase;
        if(!w.active){suspend();handled=serial;return out;}
        if(id&&now>=until){suspend();out.expired=true;}
        auto cue=reaction(p,r,snapOffense);
        if(r.sequence==serial&&cue!=Reaction::None&&handled!=serial){
            // Native catalog: 22=GEN_YEAH, 23=GEN_AWW, 25=GEN_BOO.
            // Medium requests survive the downstream selector; genuine sparse
            // population can still reduce them, and empty mode stays silent.
            handled=id=serial;until=now+600;type=cue==Reaction::Cheer?90:102;offense=snapOffense;
            out.selected=true;
        }
        if(id){w.resultCheer=id;w.resultCheerUntil=until;w.resultType=type;w.offense=offense;}
        return out;
    }
};
}
