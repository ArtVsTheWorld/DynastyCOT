#include "../Source/DepartureModel.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
struct Run {int loss=0,half=0,late=0;double hazard=0;};
Run rainyFinish(int minutes=12,int interval=1,int settleEvery=1,int wet=1,int margin=38,bool neutral=false,bool post=false,int rank=97,bool protect=false,int strength=100,bool scoreAtEnd=false){
 Settings c;c.formulaStrength=strength;History h;h.comeback=protect;
 departures::Model m;float q=minutes*60.f;State s{3,28,10,5,1,q};s.rank0=rank;s.rank1=116;s.team0=1801;s.team1=1174;s.neutral=neutral;s.playoff=post;
 s.homeScore=s.awayScore+(std::min)(18,margin);assert(m.observe(c,s,h));int loss=m.settle(s,9).loss;Run out;
 for(int tick=0;tick<=720;++tick){
  if(tick%interval&&tick!=252&&tick!=360&&tick!=540&&tick!=713&&tick!=720)continue;
  s.quarter=4;s.seconds=q*(1-tick/720.f);s.precipitation=wet;
  if(scoreAtEnd){s.homeScore=tick<720?10+(std::min)(18,margin):10+margin;s.awayScore=10;}
  else{s.homeScore=tick<252?28:tick<540?38:tick<713?45:51;s.awayScore=tick<540?10:13;int lead=s.homeScore-s.awayScore;s.homeScore=s.awayScore+(std::min)(lead,margin);}
  assert(m.observe(c,s,h));int before=loss;
  if(tick%settleEvery==0||tick==720){m.beginWindow(s,4);loss=m.settle(s,loss,0,c.quarterCap).loss;}
  assert(loss>=before&&loss<=lossCeiling(s));
  if(tick==360)out.half=loss;if(tick==540)out.late=loss;
 }
 out.loss=loss;out.hazard=m.hazard;return out;
}
int main(){
 const auto wet=rainyFinish();const auto dry=rainyFinish(12,1,1,0);
 // Acceptance band for a representative NIU-like Q4: rain begins with an
 // 18-point home lead, two later scores expand it to 32/38. Mapped opening
 // density was 72.3%; the former curve retained about 37% visible fullness.
 assert(wet.loss>=59&&wet.loss<=65&&.723f*(100-wet.loss)>=25&&.723f*(100-wet.loss)<=30);
 assert(dry.loss==34&&rainyFinish(12,1,1,1,21).loss==21); // Prior dry/competitive fixtures.
 assert(wet.half==26&&rainyFinish(12,1,1,2).loss==wet.loss); // First half of Q4 unchanged; snow follows rain.
 int previous=0;
 for(int margin:{8,14,21,24,28,32,35,38,49}){
  auto r=rainyFinish(12,1,1,1,margin);assert(r.loss>=previous);previous=r.loss;
 }
 // Same observations/score changes, different hidden render opportunities
 // and quarter lengths. No extra exits per callback or play-call opening.
 for(int minutes:{1,6,12,20})for(int cadence:{1,5,15})for(int windows:{1,30,120}){
  auto r=rainyFinish(minutes,cadence,windows);assert(std::abs(r.loss-wet.loss)<=1);
 }
 assert(rainyFinish(12,1,1,1,38,false,false,5).loss<=65); // Ranked-home hard floor.
 assert(rainyFinish(12,1,1,1,38,false,true).loss<=50); // Wet postseason ceiling.
 assert(rainyFinish(12,1,1,1,38,false,false,97,true).loss==9); // Comeback protection.
 assert(rainyFinish(12,1,1,1,38,false,false,97,false,0).loss==9); // Formula disabled.
 auto lastScore=rainyFinish(12,1,1,1,38,false,false,97,false,100,true);
 assert(lastScore.loss<30); // A final-snap score cannot backcharge the preceding quarter.
 Settings c;History h;departures::Model m;State s{4,45,10,5,180,720};s.precipitation=1;s.rank0=97;s.rank1=116;
 m.observe(c,s,h);m.settle(s,9);double before=m.hazard;int held=m.committedLoss;
 s.homeScore=51;m.observe(c,s,h);
 for(int i=0;i<500;++i){m.observe(c,s,h);assert(m.settle(s,held).loss==held&&m.hazard==before);}
 s.seconds=150;m.observe(c,s,h);assert(m.plannedLoss>held); // Pressure needs elapsed game time.
 s.seconds=0;m.observe(c,s,h);assert(m.quarterLimit(s,2880,45)==55&&m.quarterLimit(s,2880,12)==15);
 for(int kind=0;kind<5;++kind){
  auto other=s;if(kind==0)other.precipitation=0;if(kind==1)other.neutral=true;
  if(kind==2)other.homeScore=31;if(kind==3)other.quarter=3;if(kind==4)other.playoff=true;
  assert(m.quarterLimit(other,elapsedGameTime(other),45)==(kind==4?20:45));
 }
 // Visible settlement still consumes the same shared hidden-window budget.
 m.beginWindow(1);int capped=m.settle(s,held).loss;assert(capped-held<=1&&m.settle(s,capped).loss==capped);
 std::printf("PASS: rainy home blowout %.1f%% mapped fullness (old comparable path ~37%%); dry %.1f%% unchanged; no score/callback deductions, quarter/cadence scaling, ranked/postseason floors, protection, disabled policy and limited shared cap.\n",.723f*(100-wet.loss),.723f*(100-dry.loss));
}
