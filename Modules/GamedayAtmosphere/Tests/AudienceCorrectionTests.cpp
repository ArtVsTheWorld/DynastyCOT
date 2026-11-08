#include "../Source/StartingAttendance.h"
#include <cassert>
#include <sstream>
#include <set>
#include <limits>
#include <iostream>
using namespace attendance;
int main(){
    std::set<int> ids;
    for(const auto& evidence:audience::TurnoutHistory){
        assert(ids.insert(evidence.id).second&&evidence.lowerHistoricalFill>0&&std::isfinite(evidence.lowerHistoricalFill));
        bool found=false;for(const auto& c:audience::CalibrationSchools)if(c.id==evidence.id)found=true;assert(found);
    }
    assert(ids.size()==audience::CalibrationSchools.size());
    for(const auto& c:audience::CalibrationSchools){
        audience::Program p;p.id=c.id;p.row=c.id;p.year=3;p.prestige=c.prestige;p.atmosphere=c.atmosphere;p.ap=c.ap;p.identityKnown=true;
        auto base=audience::foundation(p);assert(!base.peers&&base.fans==c.referenceFans);
        std::stringstream legacy;legacy<<"DGA-AUDIENCE-1 1\n1 "<<c.id<<' '<<c.id<<" 0 0 0 "<<std::setprecision(17)<<c.fans<<" .4 .3 .2 .1\n";
        audience::Book book;assert(book.read(legacy)&&book.upgraded&&book.migrated==1&&book.revision==1);
        audience::Foundation migrated;assert(book.resolve(1,p,migrated));assert(std::abs(migrated.fans-c.referenceFans)<.000001);
        assert((migrated.reference==std::array<float,4>{.4f,.3f,.2f,.1f}));
        double fans=audience::demand(migrated,p);
        std::stringstream saved;book.write(saved);assert(saved.str().find("DGA-AUDIENCE-2 ")==0);
        audience::Book again;assert(again.read(saved)&&!again.upgraded&&again.migrated==0&&again.revision==0);
        audience::Foundation repeat;assert(again.resolve(1,p,repeat)&&repeat.fans==migrated.fans&&audience::demand(repeat,p)==fans);
    }
    // Custom replacements, unmapped programs and prior peer estimates cannot
    // acquire a known school's attendance/retention through its presentation ID.
    for(int kind=0;kind<3;++kind){
        int id=kind==2?1400:1168;bool custom=kind==0,peers=kind==1;
        std::stringstream legacy;legacy<<"DGA-AUDIENCE-1 1\n1 28 "<<id<<" 0 "<<custom<<' '<<peers<<" 23047 .3 .4 .5 .6\n";
        audience::Book book;assert(book.read(legacy)&&book.migrated==0);
        audience::Program p;p.id=id;p.row=28;p.year=0;p.custom=custom;p.identityKnown=true;p.prestige=3;p.atmosphere=8;p.ap=100;
        audience::Foundation result;assert(book.resolve(1,p,result)&&result.fans==23047);
        assert(audience::opponentRetention(result,p)==0);
    }
    // Reject a damaged upgrade transaction rather than altering a live book.
    audience::Book intact;audience::Program p;p.id=1168;p.row=28;p.year=0;p.identityKnown=true;p.prestige=8;p.atmosphere=4;p.ap=56;
    audience::Foundation base;assert(intact.resolve(1,p,base));
    for(const char* text:{"DGA-AUDIENCE-3 0","DGA-AUDIENCE-1 1\n1 28 1168 0 0 0 73353 .4 .3 .2 .1 trailing","DGA-AUDIENCE-2 1\n1 28 1168 0 0 0 nan .4 .3 .2 .1"}){
        std::stringstream bad(text);assert(!intact.read(bad));audience::Foundation after;assert(intact.resolve(1,p,after)&&after.fans==base.fans);
    }
    assert(audience::opponentRetention(base,p)>.99f);
    auto bad=p;bad.year=3;bad.prestige=2;bad.atmosphere=11;bad.ap=138;bad.history={audience::Season{1,11,0},audience::Season{2,10,0},audience::Season{2,10,0}};
    assert(audience::opponentRetention(base,bad)<audience::opponentRetention(base,p));
    bad=p;bad.custom=true;assert(audience::opponentRetention(base,bad)==0);
    p.id=1101;p.prestige=0;p.atmosphere=11;p.ap=132;auto akron=audience::foundation(p);assert(audience::opponentRetention(akron,p)==0);
    turnout::Seed s;s.home={1168,56,31,8,2,4,1,4,0,3,0,.7f};s.home.apRank=56;s.home.audienceFans=86860.5;
    s.away={1101,138,138,0,0,0,1,12,12,12,8,.01f};s.away.apRank=138;s.away.audienceFans=10000;
    s.stadium=s.homeStadium=568;s.capacity=85458;s.ea=50000;s.schools=138;s.week=3;s.year=3;s.regular=true;
    turnout::Launch l{1168,1101,568,85458,50000,720,0,false,false};
    auto ordinary=turnout::calculateAudience(s,l);s.home.opponentRetention=1;auto retained=turnout::calculateAudience(s,l);
    assert(ordinary.appeal==-6&&retained.appeal==-1.5f&&retained.momentum==ordinary.momentum&&retained.opponentRelief==4.5f);
    s.away.id=l.away=1500;assert(turnout::calculateAudience(s,l).appeal==-2.5f);
    s.away.id=l.away=1101;s.away.rank=5;assert(turnout::calculateAudience(s,l).opponentRelief==0);
    s.away.rank=138;l.rivalry=true;auto rivalry=turnout::calculateAudience(s,l);assert(rivalry.appeal==4.5f); // Rivalry bonus itself is not reduced.
    l.rivalry=false;l.neutral=true;s.away.opponentRetention=.5f;auto neutral=turnout::calculateAudience(s,l);
    auto flipped=s;std::swap(flipped.home,flipped.away);auto fl=l;std::swap(fl.home,fl.away);
    assert(turnout::calculateAudience(flipped,fl).percent==neutral.percent);
    l.neutral=false;s.home.opponentRetention=std::numeric_limits<float>::quiet_NaN();assert(turnout::calculateAudience(s,l).appeal==-6);
    std::cout<<"PASS: 134 evidence entries, raw observed anchors, one-time legacy migration/restart, references and custom/peer/unmapped preservation, corruption transaction, dynamic retention, home/neutral opponent ownership, positive appeal/rivalry/form preservation.\n";
}
