#include "../Source/DefensiveAccents.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
 for(int down:{3,4})for(int side:{0,1})for(bool neutral:{false,true})for(bool kick:{false,true}){
  energy::Tracker tracker;energy::Settings rules;energy::Play p;p.game={4,35,23,1,200,720};p.game.neutral=neutral;
  p.down=down;p.offense=side;p.toFirst=17;p.toGoal=70;p.resultStamp=10;p.selectedType=kick?22:103;p.specialTeams=kick;
  tracker.observe(rules,p);p.game.phase=3;tracker.observe(rules,p);p.resultStamp=11;p.passReleased=true;tracker.observe(rules,p);
  p.game.phase=4;p.game.seconds=195;p.offense=1-side;p.down=1;auto result=tracker.observe(rules,p);
  bool admitted=down==4&&!kick&&(side==1||neutral);assert(result.turnoverOnDowns==admitted);
  auto event=live::completedImpact(result,p.game,true,0,59);assert((event.event==live::Event::DownsStop)==admitted);
  if(admitted){assert(live::cheerType(event)==59);motion::Input x;x.match=1;x.epoch=1;x.allowed=true;x.phase=4;x.now=1000;x.duringPlay=1;x.neutral=neutral;
   defense::motion(event,x);motion::Model camera;motion::Settings settings;auto cue=camera.observe(settings,x);
   assert(cue.impactOnly&&cue.until==3700&&cue.percent==(neutral?7.5f:10.f));}
 }
 std::puts("PASS: confirmed fourth-down incompletion/possession flip gets a small paired impact, both neutral sides at75%, no third-down/punt/away-home-crowd false ownership.");
}
