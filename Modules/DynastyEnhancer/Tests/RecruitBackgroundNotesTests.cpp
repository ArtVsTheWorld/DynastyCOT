#include "../Source/ScoutingEvidence.h"
#include <cassert>
#include <iostream>
#include <map>

using namespace recruitingfog;
using namespace recruitingfog::evidence;

int main(){
    NotesProspect p;
    p.identity="league-42|recruit-123";
    p.position="QB";
    p.schoolBackgroundEligible=true;
    for(double partial:{0.,25.,50.,75.,99.,99.99}){
        p.progress=partial;
        assert(!revealBackground(p));
        assert(!obtain(nullptr,p,{},std::nullopt).contains("backgroundNote"));
    }
    p.progress=100;
    assert(revealBackground(p));
    const auto fresh=obtain(nullptr,p,{},std::nullopt);
    assert(fresh.at("backgroundNote").get_ref<const std::string&>()==newBackground(p));
    const auto reloaded=Json::parse(fresh.dump());
    p.backgroundDevelopment="College_Elite";
    p.position="ATH";
    assert(obtain(reloaded,p,{},5)==fresh);
    p.activeTransfer=true;
    assert(!revealBackground(p));
    assert(!obtain(nullptr,p,{},std::nullopt).contains("backgroundNote"));
    p.activeTransfer=false;
    p.schoolBackgroundEligible=false;
    assert(!revealBackground(p));
    assert(!obtain(nullptr,p,{},std::nullopt).contains("backgroundNote"));
    p.schoolBackgroundEligible=true;
    p.progress=101;
    assert(!revealBackground(p));

    p.progress=100;
    const Json legacy={{"version",1},{"claims",Json::object()},{"displayOrder",Json::array()},
        {"athleticBackground","  - First saved line.\r\n- Second saved line."},{"flavor","Third saved line."}};
    assert(storedBackground(legacy)=="First saved line.");
    assert(obtain(legacy,p,{},0)==legacy);
    const Json missing={{"athleticBackground",false},{"flavor","\n- Kept original line.\nAlso saved."}};
    assert(storedBackground(missing)=="Kept original line.");
    std::string bullets="SCOUTING NOTES";
    appendNoteBullets(bullets,"One.\n\n- Two.\r\n Three. ");
    assert(bullets=="SCOUTING NOTES\n- One.\n- Two.\n- Three.");

    const auto same=NotesProspect(p);
    constexpr unsigned samples=10000;
    std::cout<<"Position,distinct,most-frequent-count,prospects\n";
    for(const auto* position:{"QB","HB","FB","WR","TE","LT","LG","C","RG","RT","LEDG","REDG","DT","MIKE","SAM","WILL","LOLB","ROLB","CB","FS","SS","K","P","LS","ATH"}){
        p=same;
        p.position=position;
        p.backgroundDevelopment.clear();
        std::map<std::string_view,unsigned> counts;
        unsigned peak=0;
        for(unsigned i=0;i<samples;++i){
            p.identity="league-42|"+p.position+"|"+std::to_string(i);
            const auto line=newBackground(p);
            assert(!line.empty()&&line.find('\n')==line.npos);
            const auto count=++counts[line];
            peak=(std::max)(peak,count);
        }
        assert(counts.size()==50&&peak<300);
        std::cout<<position<<','<<counts.size()<<','<<peak<<','<<samples<<'\n';
    }
    p.position="QB";
    unsigned rare=0,changedLeague=0;
    for(unsigned i=0;i<30000;++i){
        p.identity="league-42|"+std::to_string(i);
        p.backgroundDevelopment.clear();
        const auto original=newBackground(p);
        p.observations={{"Speed",45,45,true}};
        assert(newBackground(p)==original);
        p.observations={{"Speed",98,98,true}};
        assert(newBackground(p)==original);
        p.backgroundDevelopment="unsupported";
        assert(newBackground(p)==original);
        p.backgroundDevelopment="College_Elite";
        const auto withDevelopment=newBackground(p);
        if(withDevelopment!=original){
            ++rare;
            assert(withDevelopment.find("His coach says")==0&&withDevelopment.find("Elite")==withDevelopment.npos);
        }
        p.backgroundDevelopment.clear();
        p.identity="league-99|"+std::to_string(i);
        changedLeague+=newBackground(p)!=original;
    }
    assert(rare>150&&rare<330&&changedLeague>28500);
    std::cout<<"Trait impressions,"<<rare<<",30000\n";
    std::cout<<"PASS full scouting, storage round trip, legacy preservation, bullets, distribution and Dynasty isolation\n";
}
