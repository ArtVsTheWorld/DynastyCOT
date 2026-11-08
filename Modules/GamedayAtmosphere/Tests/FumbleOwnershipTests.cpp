#include "../Source/LiveFumble.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    for(int owner:{0,1})for(bool neutral:{false,true}){
        live::Fumbles m;energy::Play p;p.game={2,7,7,1,300,720};p.game.neutral=neutral;p.offense=owner;p.down=1;
        live::ForcedEvent e{999,0,true};
        m.observe(p,123,1,1000,true,e,0,2,true,false,{},owner);
        p.game.phase=3;e.type=32;
        // The first live sample may already be loose: the lineup carrier was
        // verified, so a missing held-ball frame must not erase its owner.
        auto hit=m.observe(p,123,1,1100,true,e,0,5,false,true,{},-1);
        assert(bool(hit.id)==(owner==1||neutral));
        assert(m.fumbler()==owner);
        // Native offense label has not flipped yet; verified carrier has.
        auto recovered=m.observe(p,123,1,1200,true,e,0,4,true,false,{},1-owner);
        assert(bool(recovered.id)==(owner==1||neutral));
        if(recovered.id)assert(recovered.event==live::Event::FumbleRecovery&&recovered.side==1-owner);
        assert(m.recoveredBy()==1-owner);
        assert(!m.observe(p,123,1,1300,true,e,0,4,true,false,{},1-owner).id);
    }
    for(int owner:{0,1}){
        live::Fumbles m;energy::Play p;p.game={2,7,7,1,300,720};p.offense=owner;p.down=1;
        live::ForcedEvent e{999,0,true};
        m.observe(p,123,1,1000,true,e,0,2,true,false,{},owner);
        p.game.phase=3;m.observe(p,123,1,1100,true,e,0,2,true,false,{},owner);
        m.observe(p,123,1,1200,true,e,0,5,false,true,{},-1);
        // A changed offense label alone cannot turn an own pickup into a TD-
        // style takeaway response. Unknown carrier identity also proves none.
        p.offense=1-owner;assert(!m.observe(p,123,1,1300,true,e,0,2,true,false,{},-1).id);
        assert(!m.observe(p,123,1,1400,true,e,0,2,true,false,{},owner).id);
        assert(m.recoveredBy()==owner);
    }
    std::puts("PASS: verified fumble carrier ownership, first-sample loose ball, lagging offense label, held state4, own pickups and unknown-owner rejection");
}
