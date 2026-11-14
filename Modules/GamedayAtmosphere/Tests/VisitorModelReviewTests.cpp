#include "../Source/VisitorPlans.h"
#include "../Source/AttendanceTimeline.h"
#include "LegacyVisitorFixture.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <sstream>
using namespace attendance;
static turnout::CrowdFixture fixture(int hp=7,int ap=9,int hw=3,int hl=7,int aw=6,int al=4,double distance=1924){
 turnout::CrowdFixture f;auto& s=f.seed;auto& l=f.launch;f.verified=true;
 s.dynasty=42;s.week=11;s.year=1;s.stadium=s.homeStadium=100;s.capacity=s.ea=75000;s.schools=136;s.regular=true;s.serial=1;s.dynastyGeneration=1;s.dynastyVerifiedAt=1;
 s.home.id=1101;s.away.id=1102;s.home.recordRow=1;s.away.recordRow=2;
 for(auto* t:{&s.home,&s.away}){t->rank=60;t->prestigeKnown=true;t->toughest=50;t->homeGames=4;t->standing=.65f;t->atmosphere=t->tradition=t->brand=t->conference=5;}
 s.home.prestige=hp;s.away.prestige=ap;s.home.wins=hw;s.home.losses=hl;s.away.wins=aw;s.away.losses=al;
 s.home.audienceFans=68000;s.away.audienceFans=75000;
 s.home.latitude=30;s.home.longitude=-80;s.away.latitude=float(30+distance/69.0934);s.away.longitude=-80;
 l.home=s.home.id;l.away=s.away.id;l.stadium=s.stadium;l.capacity=s.capacity;l.ea=s.ea;l.minutes=900;l.precipitation=0;l.senior=l.seniorKnown=true;return f;
}
int main(){
 assert(visitormodel::season(10,-1,-1)==1&&visitormodel::season(10,1000000000,1000000000)==1);
 assert(visitormodel::national(6)==0&&visitormodel::national(7)==.12&&visitormodel::national(8)==.4&&visitormodel::national(9)==.7&&visitormodel::national(10)==1);
 for(int p=7;p<10;++p)for(int k=0;k<99;++k)assert(visitormodel::national(p+k/100.)<=visitormodel::national(p+(k+1)/100.));
 for(double x:{150.,400.,800.,1500.})assert(std::abs(visitormodel::travel(x-.001)-visitormodel::travel(x+.001))<.001);
 auto reference=fixture();
 // Use shipped program foundations as explicit proxies, not invented current Dynasty observations.
 reference.seed.home.id=reference.launch.home=1161;reference.seed.away.id=reference.launch.away=1183;
 reference.seed.home.audienceFans=69446.5;reference.seed.away.audienceFans=56592.;
 auto rp=visitors::plan(reference,1161,1183);auto rr=visitors::attendancePlan(reference.seed,reference.launch,false,100);
 std::printf("REFERENCE Oregon/MSU: visitors %.3f%%, capacity %.3f%%, home %.0f regional %.0f local %.0f national %.0f travel %.3f\n",100*rp.share,rr.percent,rp.components.home,rp.components.regional,rp.components.local,rp.components.national,rp.travelFactor);
 assert(rp.valid&&rr.applied&&rp.share>=.12f&&rp.share<=.15f&&rr.percent>=60&&rr.percent<=75);
 struct Example{const char* name;turnout::CrowdFixture f;};
 auto farSmall=fixture(4,3,5,5,5,5,2300);farSmall.seed.home.audienceFans=30000;farSmall.seed.away.audienceFans=18000;
 auto nearby=fixture(7,7,6,4,6,4,80);nearby.launch.rivalry=true;
 auto strong=fixture(10,10,8,2,9,1,1200);strong.seed.home.audienceFans=100000;strong.seed.away.audienceFans=100000;
 auto weak=fixture(3,10,1,9,9,1,1200);weak.seed.home.audienceFans=24000;weak.seed.away.audienceFans=100000;
 auto poor=fixture(10,10,2,8,2,8,1400);auto both=fixture(5,5,2,8,2,8,900);
 auto fcs=fixture(5,0,5,5,0,0,100);fcs.seed.away.id=fcs.launch.away=1500;fcs.seed.away.audienceFans=0;
 auto neutral=strong;neutral.launch.neutral=true;neutral.seed.stadium=neutral.launch.stadium=101;
 std::vector<Example> cases{{"Oregon at MSU",reference},{"Small/far",farSmall},{"Nearby rivals",nearby},{"Two strong",strong},{"Strong visitor/weak host",weak},{"High prestige/poor seasons",poor},{"Both poor",both},{"FCS regional proxy",fcs},{"Neutral two strong",neutral}};
 for(auto& e:cases){
  auto before=visitors::fixturePlan(e.f,e.f.launch.home,e.f.launch.away);auto after=visitors::plan(e.f,e.f.launch.home,e.f.launch.away);auto a=visitors::attendancePlan(e.f.seed,e.f.launch,false,100);auto b=turnout::calculateAudienceLegacy(e.f.seed,e.f.launch,false);
  std::printf("TABLE|%s|%.2f|%.2f|%.2f|%.2f|%.0f|%.0f|%.0f|%.0f|%.2f\n",e.name,100*before.share,100*after.share,b.percent,a.percent,after.components.home,after.components.regional,after.components.local,after.components.national,after.components.takeover);
  assert(after.valid&&after.share>=(e.f.launch.neutral?.30f:.03f)&&after.share<=(e.f.launch.neutral?.70f:.35f));
 }
 unsigned common=0,total=0;auto start=std::chrono::steady_clock::now();
 for(int hp:{3,5,7,8,9,10})for(int ap:{3,5,7,8,9,10})for(double dist:{75.,250.,600.,1100.,1924.,2800.})for(unsigned seed=0;seed<20;++seed){
  auto f=fixture(hp,ap,5,5,6,4,dist);f.seed.home.audienceFans=10000+7000*hp;f.seed.away.audienceFans=10000+7000*ap;f.seed.year=int(seed);
  auto p=visitors::plan(f,f.launch.home,f.launch.away);assert(p.valid&&p.share>=.03f&&p.share<=.35f);assert(p.share<=p.ceiling+1e-6);common+=p.share>=.05f&&p.share<=.15f;++total;
  assert(p.share==visitors::plan(f,f.launch.home,f.launch.away).share);
  auto low=visitors::influence(p,0),high=visitors::influence(p,200);assert(low.share>=.03f&&low.share<=p.share&&high.share>=p.share&&high.share<=p.ceiling+1e-6);
 }
 assert(common*100>=total*70);
 auto millis=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();std::printf("CALIBRATION|%u/%u in 5-15%%; %.3f ms total (offline generation)\n",common,total,millis);
 auto known=visitors::demand(reference),unknown=known;unknown.home=unknown.away={};auto missing=visitors::plan(unknown);assert(missing.valid&&missing.travel==visitors::Travel::Unknown&&missing.travelFactor==1);
 unknown.sameState=true;auto local=visitors::plan(unknown);assert(!local.travelKnown&&local.travelFactor>1&&local.travelFactor<1.2);
 unknown.sameState=false;unknown.homeState={42,-84};unknown.awayState={44,-123};auto state=visitors::plan(unknown);assert(state.travel==visitors::Travel::State&&state.travelFactor>rp.travelFactor);
 auto same=known;same.sameState=true;assert(visitors::plan(same).share==visitors::plan(known).share);
 auto marquee=known;marquee.marquee=true;assert(visitors::plan(marquee).share==visitors::plan(known).share);
 auto east=visitors::demand(fcs);east.home={40,-75};east.away={};auto localFcs=visitors::plan(east);assert(localFcs.travel==visitors::Travel::Region&&localFcs.distance==75&&localFcs.travelFactor>1);
 auto west=east;west.fcsRegion=4;auto farFcs=visitors::plan(west);assert(farFcs.travel==visitors::Travel::Region&&farFcs.distance>1500&&farFcs.travelFactor<1&&localFcs.share>farFcs.share);
 auto southeast=east;southeast.home={30,-84};southeast.fcsRegion=3;assert(visitors::plan(southeast).distance==75);
 auto midwest=east;midwest.home={42,-89};midwest.fcsRegion=1;assert(visitors::plan(midwest).distance==75);
 auto northwest=east;northwest.home={46,-122};northwest.fcsRegion=2;assert(visitors::plan(northwest).distance==75);
 auto unknownFcs=east;unknownFcs.home={};assert(visitors::plan(unknownFcs).travelFactor==1);
 auto unverified=reference;unverified.seed.away.prestigeKnown=false;assert(!visitors::plan(unverified,unverified.launch.home,unverified.launch.away).valid);
 unverified=reference;unverified.seed.ea=unverified.launch.ea=0;assert(!visitors::plan(unverified,unverified.launch.home,unverified.launch.away).valid);
 for(int rank:{7,8,9,10}){auto n=known;n.awayPrestige=rank;std::printf("NATIONAL|%.1f stars %.0f modeled national supporters\n",rank/2.,visitors::plan(n).components.national);}
 auto balancedNeutral=fixture(5,8,3,7,7,3,1200);balancedNeutral.launch.neutral=true;balancedNeutral.seed.home.audienceFans=25000;balancedNeutral.seed.away.audienceFans=45000;auto labelA=visitors::attendancePlan(balancedNeutral.seed,balancedNeutral.launch,false,100);std::swap(balancedNeutral.seed.home,balancedNeutral.seed.away);std::swap(balancedNeutral.launch.home,balancedNeutral.launch.away);auto labelB=visitors::attendancePlan(balancedNeutral.seed,balancedNeutral.launch,false,100);assert(labelA.attendance==labelB.attendance);
 auto poorNational=visitors::demand(poor);auto successfulNational=poorNational;successfulNational.awayWins=8;successfulNational.awayLosses=2;assert(visitors::plan(poorNational).components.national>visitors::plan(successfulNational).components.national*.8);
 // National support is independent of campus distance, and neutral labels are symmetric.
 auto nearDemand=known;nearDemand.away={30.1,-80};assert(visitors::plan(nearDemand).components.national==visitors::plan(known).components.national);
 auto neutralDemand=visitors::demand(neutral);auto neutralPlan=visitors::plan(neutralDemand);std::swap(neutralDemand.homeFans,neutralDemand.awayFans);std::swap(neutralDemand.homePrestige,neutralDemand.awayPrestige);std::swap(neutralDemand.homeWins,neutralDemand.awayWins);std::swap(neutralDemand.homeLosses,neutralDemand.awayLosses);assert(std::abs(visitors::plan(neutralDemand).share+neutralPlan.share-1)<1e-6);
 auto skew=visitors::demand(neutral);skew.homeFans=100000;skew.awayFans=15000;skew.homePrestige=10;skew.awayPrestige=4;assert(visitors::plan(skew).share==.3f);std::swap(skew.homeFans,skew.awayFans);std::swap(skew.homePrestige,skew.awayPrestige);assert(visitors::plan(skew).share==.7f);
 auto notStruggling=visitors::demand(weak);notStruggling.homeWins=8;notStruggling.homeLosses=2;notStruggling.homeTickets=notStruggling.homeFans;assert(visitors::plan(notStruggling).ceiling==.18);
 auto noNational=visitors::demand(weak);noNational.awayPrestige=6;assert(visitors::plan(noNational).ceiling==.18);
 auto losingVisitor=visitors::demand(weak);losingVisitor.awayWins=2;losingVisitor.awayLosses=8;assert(visitors::plan(losingVisitor).ceiling==.18);
 // Old VISITORS-2 shares and geometry revision are preserved exactly.
 std::stringstream legacy("DGA-VISITORS-2 1\n42 1101 1102 1 2 100 11 1 0.021 99 7\n");visitors::PlanBook book;assert(visitors::PlanBook::read(legacy,book));auto key=visitors::key(fixture().seed);visitors::Plan selected;unsigned seed=0;bool reused=false;
 assert(book.resolve(key,rp,500,selected,seed,reused)&&reused&&selected.layoutVersion==1&&seed==99&&selected.share==.021f&&book.revision==0);
 auto k=key;++k.week;assert(book.resolve(k,rp,111,selected,seed,reused)&&!reused&&selected.layoutVersion==attendance::visitors::CurrentLayoutVersion);book.attendance(k,rr,75000);std::stringstream disk;book.write(disk);visitors::PlanBook loaded;assert(visitors::PlanBook::read(disk,loaded));assert(loaded.find(k)->attendance==rr.attendance&&loaded.find(k)->layoutVersion==attendance::visitors::CurrentLayoutVersion);
 std::stringstream historyOnly("DGA-VISITORS-2 1\n42 1101 1102 1 2 100 11 1 -1 0 7\n");visitors::PlanBook historical;assert(visitors::PlanBook::read(historyOnly,historical));assert(historical.resolve(key,rp,500,selected,seed,reused)&&reused&&selected.native&&historical.revision==0);
 auto priorSize=loaded.entries.size();loaded.importMissing(book);assert(loaded.entries.size()==priorSize&&loaded.revision==0);auto newKey=key;++newKey.year;assert(historical.bind(newKey,99));loaded.importMissing(historical);assert(loaded.find(newKey)&&loaded.find(newKey)->layoutVersion==1);
 // No resurrecting departures, floor applies only to new plans, not legacy.
 visitors::Losses losses;uint64_t h=0,a=0;for(int percent=0;percent<=99;++percent){losses.advance(97000,3000,percent,0,70,4,0,0,false,true);assert(losses.home>=h&&losses.away>=a);h=losses.home;a=losses.away;uint64_t left=100000-h-a;assert(!left||double(3000-a)/left>=.01-1e-9);}
 auto before=losses;losses.advance(97000,3000,10,70,0,4,0,0,false,true);assert(losses.home==before.home&&losses.away==before.away);
 // Arrival/temporary absences also retain a visitor contingent without restoring permanent exits.
 for(int gone=0;gone<99;++gone){visitors::Losses g;g.advance(97000,3000,gone,0,70,4,0,0,false,true);fanbudget::Counts counts{97000,3000,0,0};for(int absent=gone;absent<100;++absent){auto m=fanbudget::missing(counts,{g.home,g.away},absent,true);auto left=counts.total()-m.total();assert(!left||double(counts.away-m.away)/left>=.01-1e-9);}}
 for(auto pair:std::vector<std::pair<const char*,turnout::CrowdFixture>>{{"Reference - low slider",reference},{"Reference - high slider",reference}}){double strength=pair.first[12]=='l'?0:200;auto p=visitors::influence(visitors::plan(pair.second,pair.second.launch.home,pair.second.launch.away),strength);std::printf("SLIDER|%s|%.2f\n",pair.first,100*p.share);}
 // Capacity caps are stable per fixture; exceptions are exact and independent.
 auto capFixture=strong;capFixture.seed.home.id=capFixture.launch.home=1212;capFixture.seed.home.rank=capFixture.seed.away.rank=60;capFixture.launch.senior=false;
 for(int year=0;year<40;++year){capFixture.seed.year=year;auto c=visitors::attendancePlan(capFixture.seed,capFixture.launch,true,100);assert(c.capacityLimit>=65&&c.capacityLimit<=75&&c.percent<=c.capacityLimit+1e-5);assert(c.percent==visitors::attendancePlan(capFixture.seed,capFixture.launch,true,100).percent);}
 for(int exception=0;exception<3;++exception){auto f=capFixture;if(exception==0)f.seed.home.homeGames=0;if(exception==1)f.launch.rivalry=true;if(exception==2)f.seed.home.rank=f.seed.away.rank=5;auto c=visitors::attendancePlan(f.seed,f.launch,false,100);assert(c.capacityLimit==100);}
 auto severeSenior=capFixture;severeSenior.launch.senior=true;severeSenior.launch.minutes=1200;assert(visitors::attendancePlan(severeSenior.seed,severeSenior.launch,false,100).capacityLimit<=75);
 auto fcsCap=strong;fcsCap.seed.away.id=fcsCap.launch.away=1500;fcsCap.seed.away.audienceFans=0;fcsCap.launch.senior=false;auto c=visitors::attendancePlan(fcsCap.seed,fcsCap.launch,true,100);assert(c.capacityLimit>=75&&c.capacityLimit<=85);
 auto fcsRival=fcsCap;fcsRival.launch.rivalry=true;assert(visitors::attendancePlan(fcsRival.seed,fcsRival.launch,true,100).capacityLimit<=85);
 for(int exception=0;exception<3;++exception){auto f=fcsCap;if(exception==0)f.seed.home.homeGames=0;if(exception==1)f.launch.senior=true;if(exception==2)f.launch.minutes=1200;assert(visitors::attendancePlan(f.seed,f.launch,false,100).capacityLimit==100);}
 auto gap=strong;gap.seed.away.prestige=4;gap.launch.senior=false;assert(visitors::attendancePlan(gap.seed,gap.launch,true,100).capacityLimit<=85);gap.launch.rivalry=true;assert(visitors::attendancePlan(gap.seed,gap.launch,false,100).capacityLimit==100);gap.launch.rivalry=false;gap.seed.away.prestige=5;assert(visitors::attendancePlan(gap.seed,gap.launch,false,100).capacityLimit==100);
 // Native geometry fixtures: four stands, aisles every 16 seats, protected bands.
 constexpr unsigned N=12288;std::vector<seats::Position> pos(N);std::vector<unsigned char> raw(N*10);
 for(unsigned i=0;i<N;++i){unsigned stand=i/3072,row=(i%3072)/64,seat=i%64;float along=(float(seat)-31.5f)*.65f+float(seat/16)*1.8f,out=36+row*.9f,height=1+row*.7f;pos[i]=stand<2?seats::Position{stand?out:-out,height,along,0}:seats::Position{along,height,stand==2?out+20:-out-20,0};raw[i*10]=i%11==0;raw[i*10+6]=i<32?1:i<96?9:i<112?10:0;if(i>=32&&i<96)raw[i*10]=0;if(i>=96&&i<112)raw[i*10]=1;}
 auto tags=agents::decode(raw,N);agents::protectStudents(tags,true,false);for(bool neutralSite:{false,true})for(float share:neutralSite?std::vector<float>{.30f,.5f,.70f}:std::vector<float>{.03f,.12f,.35f}){
  auto oldAt=std::chrono::steady_clock::now();auto oldSide=visitors::assignLegacy(pos,tags,share,73);double oldMs=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-oldAt).count();assert(!oldSide.empty());
  visitors::seating::Stats stats;auto at=std::chrono::steady_clock::now();auto side=visitors::seating::assign(pos,tags,share,73,nullptr,neutralSite,&stats);double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-at).count();
  std::printf("SEATING|neutral %d|share %.2f|valid %d|unassigned %u|core %u|parties %u|1..8",neutralSite,share,!side.empty(),stats.unassigned,stats.core,stats.parties);for(unsigned size=1;size<=8;++size)std::printf(" %u",stats.sizes[size]);std::printf("|before %.3f after %.3f ms\n",oldMs,ms);
  assert(!side.empty());
  for(unsigned i=32;i<N;++i)if(stats.party[i]>=0){unsigned row=(i%3072)/64,seat=i%64;for(unsigned j:{seat<63&&seat%16!=15?i+1:i,row<47?i+64:i})if(j!=i&&stats.party[j]>=0)assert(stats.party[i]==stats.party[j]);}
  unsigned weighted=stats.sizes[2]+stats.sizes[3]+stats.sizes[4];assert(weighted*2>stats.parties);
  unsigned away=0;for(unsigned i=0;i<N;++i){if(tags.protectedGroup(i))assert(side[i]==tags.side[i]);if(!tags.protectedBand(i))away+=side[i];}
  assert(away==unsigned(std::llround((N-32)*share)));assert(side==visitors::assign(pos,tags,share,73,nullptr,2,neutralSite));
 }
 auto shuffledPos=pos;auto shuffledRaw=raw;std::reverse(shuffledPos.begin(),shuffledPos.end());for(unsigned i=0;i<N;++i)for(unsigned j=0;j<10;++j)shuffledRaw[10*i+j]=raw[10*(N-i-1)+j];auto shuffled=agents::decode(shuffledRaw,N);agents::protectStudents(shuffled,true,false);assert(!visitors::assign(shuffledPos,shuffled,.12f,73,nullptr,2).empty());
 std::vector<unsigned char> occupied(N,0);for(unsigned i=32;i<N;++i)occupied[i]=i%3!=0;auto sparse=visitors::assign(pos,tags,.12f,73,&occupied,2);assert(!sparse.empty());unsigned occupiedCount=0,occupiedAway=0;for(unsigned i=0;i<N;++i){if(!occupied[i]||tags.protectedGroup(i))assert(sparse[i]==tags.side[i]);if(occupied[i]){++occupiedCount;occupiedAway+=sparse[i];}}assert(occupiedAway==unsigned(std::llround(occupiedCount*.12f)));
 std::puts("PASS visitor model review: components, ranges, geography, old/new history, floor, seating, deterministic generation; no game access.");
}
