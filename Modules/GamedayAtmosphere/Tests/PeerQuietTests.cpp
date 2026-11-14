#include "../Source/AudienceModel.h"
#include "../Source/HomeCrowdQuiet.h"
#include <cassert>
#include <sstream>
#include <iostream>
int main(){using namespace attendance;
 for(const auto& c:audience::CalibrationSchools){audience::Program p;p.id=c.id;p.row=1;p.year=0;p.identityKnown=true;p.prestige=c.prestige;p.atmosphere=c.atmosphere;p.ap=c.ap;assert(audience::foundation(p).fans==c.referenceFans);}
 {audience::Program p;p.id=1801;p.row=1;p.year=0;p.identityKnown=p.custom=true;p.prestige=5;p.atmosphere=3;
  std::stringstream legacy("DGA-AUDIENCE-2 1\n42 1 1801 0 1 1 47236 .5 .75 .5 .4\n");audience::Book b;assert(b.read(legacy));audience::Foundation f;assert(b.resolve(42,p,f)&&f.fans==47236&&f.policy==1);std::stringstream v3;b.write(v3);audience::Book restored;assert(restored.read(v3)&&restored.resolve(42,p,f)&&f.fans==47236&&f.policy==1);
  p.prestige=-1;assert(audience::foundation(p).fans==0);
 }
 for(int prestige:{0,5,10}){audience::Program p;p.id=1801;p.row=1;p.year=0;p.custom=p.identityKnown=true;p.prestige=prestige;p.atmosphere=0;
  auto a=audience::foundation(p);p.atmosphere=12;p.ap=1;auto b=audience::foundation(p);assert(a.fans==b.fans&&a.policy==2);
  assert(audience::demand(a,p)==a.fans);audience::Book book;audience::Foundation saved;assert(book.resolve(42,p,saved));std::stringstream stream;book.write(stream);audience::Book reload;assert(reload.read(stream));assert(reload.resolve(42,p,saved)&&saved.fans==b.fans);
  std::cout<<"prestige "<<prestige/2.<<" stars: "<<a.fans<<" fans\n";
 }
 energy::Play p;p.game={2,10,0,1,400,720};p.down=1;p.offense=1;p.selectedType=103;auto snap=p;homequiet::Model q;energy::Outcome r;
 q.observe(p,r,42,1,1,1000,true,.08f,&snap);
 // Score precedes the completed result. Old poll-to-poll delta was zero.
 p.game.awayScore=6;p.game.phase=3;q.observe(p,r,42,1,1,1100,true,.08f,&snap);
 p.game.phase=4;r.accepted=r.completed=true;r.sequence=1;r.side=1;r.awayPoints=6;
 auto out=q.observe(p,r,42,1,1,1200,true,.08f,&snap);assert(out.newTouchdown&&out.corroboration==1);
 assert(!q.observe(p,r,42,1,1,1250,true,.08f,&snap).newTouchdown);
 // Bounded pending result, matching update, expiry, corrections, new fixture.
 for(int mode=0;mode<4;++mode){homequiet::Model model;auto v=snap;v.game.phase=4;model.observe(v,{},42,1,1,1000,true,.08f,&snap);
  auto pending=model.observe(v,r,42,1,1,1100,true,.08f,&snap);assert(pending.corroboration==2);
  v.game.awayScore=mode==2?3:6;auto settled=model.observe(v,{},mode==3?43:42,1,1,mode==1?3000:1200,true,.08f,&snap);
  assert(settled.newTouchdown==(mode==0));assert(!model.observe(v,r,42,1,1,4000,true,.08f,&snap).newTouchdown);
 }
 std::cout<<"PASS prestige-only peers, format3 reload, score-before-result, delayed corroboration, expiry, corrections, fixture ownership and dedup\n";
}
