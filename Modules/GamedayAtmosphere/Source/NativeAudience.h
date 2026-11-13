#pragma once
#include "NativeStartingAttendance.h"
#include <string>
namespace attendance::audiencenative {
using namespace turnout::native;
struct CaptureTrace {
    const char* failure=nullptr;
    const char* side="dynasty";
    int year=-1,row=-1,custom=-1,historyCount=-1;
    int id=-1,prestige=-1,atmosphere=-1,ap=-1,seasonSlot=-1,wins=-1,losses=-1,ties=-1;
    bool fail(const char* reason){failure=reason;return false;}
    std::string describe()const{
        return std::string(failure?failure:"native audience inputs captured")+"; scope "+side+
            "; dynasty year "+std::to_string(year)+"; program row "+std::to_string(row)+
            "; TeamBuilder "+std::to_string(custom)+"; history slots "+std::to_string(historyCount)+
            "; program ID/grades/AP "+std::to_string(id)+" / "+std::to_string(prestige)+" / "+std::to_string(atmosphere)+" / "+std::to_string(ap)+
            "; completed-season slot/W-L-T "+std::to_string(seasonSlot)+" / "+std::to_string(wins)+"-"+std::to_string(losses)+"-"+std::to_string(ties)+
            " (-1 means not read)";
    }
};
// Enum labels/ordinals verified in the post-title-update mapped image. Unknown
// layouts disable only the new turnout model, not ordinary native gameplay.
inline bool bindingsReady(A image){
    Reader r{image};A name=0;int ordinal=-1;
    return r.value(image+0xbdd30b8,name)&&sustained::named(name,"TeamSeasonStats")&&r.value(image+0xbdd30b0,ordinal)&&ordinal==0x184&&
        r.value(image+0xbdc5208,name)&&sustained::named(name,"LeagueData")&&r.value(image+0xbdc5200,ordinal)&&ordinal==0&&
        r.value(image+0xbdc5218,name)&&sustained::named(name,"LeagueHistoryManager")&&r.value(image+0xbdc5210,ordinal)&&ordinal==1&&
        r.value(image+0xbde1278,name)&&sustained::named(name,"Dynasty")&&r.value(image+0xbde1270,ordinal)&&ordinal==0&&
        r.value(image+0xbded4e8,name)&&sustained::named(name,"League")&&r.value(image+0xbded4e0,ordinal)&&ordinal==0x1b&&
        r.value(image+0xbdd20f8,name)&&sustained::named(name,"IsTeamBuilder")&&r.value(image+0xbdd20f0,ordinal)&&ordinal==0x88&&
        r.value(image+0xbded4f8,name)&&sustained::named(name,"LeagueID")&&r.value(image+0xbded4f0,ordinal)&&ordinal==0x1c&&
        r.value(image+0xbdeeca8,name)&&sustained::named(name,"WINS")&&r.value(image+0xbdeeca0,ordinal)&&ordinal==57&&
        r.value(image+0xbdeea78,name)&&sustained::named(name,"LOSSES")&&r.value(image+0xbdeea70,ordinal)&&ordinal==22&&
        r.value(image+0xbdeec68,name)&&sustained::named(name,"TIES")&&r.value(image+0xbdeec60,ordinal)&&ordinal==53;
}
inline bool sameRecord(const Reader& r,A left,A right){
    A a=0,b=0;int ar=-1,br=-1;
    return r.value(left+0x10,a)&&a&&r.value(right+0x10,b)&&b&&
        r.value(left+0x18,ar)&&ar>=0&&ar<=1000000&&r.value(right+0x18,br)&&br>=0&&br<=1000000&&a==b&&ar==br;
}
inline bool activeFranchise(const Reader& r,A manager,A& out,CaptureTrace& trace){
    // StatManager's database view does not expose the Franchise table by name.
    // Follow the active manager's own relationships instead of querying FIRST.
    A vt=0;if(!r.value(manager,vt)||vt!=r.image+StatManagerVtable)return trace.fail("launch statistics manager identity unavailable");
    Record history(r),league(r),ownerLeague(r);
    if(!r.related(manager,1,history.p)||!history.p)return trace.fail("statistics manager's league-history relationship unavailable");
    if(!r.related(history.p,0,out)||!out)return trace.fail("league-history manager's active dynasty relationship unavailable");
    if(!r.related(manager,0,league.p)||!league.p)return trace.fail("statistics manager's active league unavailable");
    if(!r.related(out,0x1b,ownerLeague.p)||!ownerLeague.p)return trace.fail("dynasty's owning league unavailable");
    if(!sameRecord(r,league.p,ownerLeague.p))return trace.fail("dynasty and launch statistics manager refer to different leagues");
    return true;
}
inline bool identity(A image,A manager,int& dynasty){
    dynasty=0;Reader r{image};Record franchise(r);CaptureTrace trace;
    int observed=0;if(!activeFranchise(r,manager,franchise.p,trace)||!r.integer(franchise.p,0x1c,observed)||!observed)return false;
    dynasty=observed;return true;
}
inline bool program(const Reader& r,A record,turnout::Team& team,int year,const Api& api,CaptureTrace& trace){
    auto& p=team.program;p={};int custom=-1;
    trace.year=year;trace.row=-1;trace.custom=-1;trace.historyCount=-1;
    trace.id=team.id;trace.prestige=team.prestige;trace.atmosphere=team.atmosphere;trace.ap=team.apRank;
    trace.seasonSlot=-1;trace.wins=-1;trace.losses=-1;trace.ties=-1;
    if(!r.integer(record,0x88,custom,true))return trace.fail("IsTeamBuilder field unavailable");
    trace.custom=custom;
    if(!r.value(record+0x18,p.row))return trace.fail("program record identity unavailable");
    trace.row=p.row;
    if(year<0||year>100)return trace.fail("dynasty year outside supported range");
    p.id=team.id;p.year=year;p.prestige=team.prestige;p.atmosphere=team.atmosphere;p.ap=team.apRank;p.custom=custom!=0;p.identityKnown=true;
    if(!audience::valid(p))return trace.fail("program grades/rank/identity outside supported range");
    // Year-zero custom teams have no eligible completed seasons. Inherited
    // arrays must neither affect turnout nor become a prerequisite to capture.
    if(p.custom&&year==0){trace.historyCount=0;return true;}
    Record history(r);if(!r.related(record,0x184,history.p))return trace.fail("TeamSeasonStats relationship unavailable");
    if(!history.p){trace.historyCount=0;return audience::valid(p)||trace.fail("program grades/rank/identity outside supported range");} // A new program can have no stored seasons.
    A count=0;if(!api.count||!sustained::obtain(count,api.count,history.p))return trace.fail("season-history slot count unavailable");
    trace.historyCount=count>1000000?1000001:static_cast<int>(count);
    if(count>5)return trace.fail("season-history slot count exceeds verified layout");
    for(int i=0;i<3&&i+1<static_cast<int>(count);++i){
        if(p.custom&&i>=year)continue;
        trace.seasonSlot=i+1;trace.wins=-1;trace.losses=-1;trace.ties=-1;
        Record season(r);if(!r.element(history.p,i+1,season.p))return trace.fail("completed-season record unavailable");
        if(!season.p)continue;
        auto& s=p.history[i];if(!r.integer(season.p,57,trace.wins))return trace.fail("completed-season WINS field unavailable");s.wins=trace.wins;
        if(!r.integer(season.p,22,trace.losses))return trace.fail("completed-season LOSSES field unavailable");s.losses=trace.losses;
        if(!r.integer(season.p,53,trace.ties))return trace.fail("completed-season TIES field unavailable");s.ties=trace.ties;
        if(s.wins<0||s.wins>30||s.losses<0||s.losses>30||s.ties<0||s.ties>30||s.wins+s.losses+s.ties>30)return trace.fail("completed-season results outside supported range");
    }
    return audience::valid(p)||trace.fail("program grades/rank/identity outside supported range");
}
inline bool capture(A image,A manager,A gameRef,turnout::Seed& s,const Api* supplied=nullptr,CaptureTrace* diagnostics=nullptr){
    Reader r{image};Api api=supplied?*supplied:Api::engine(image);A game=0;
    CaptureTrace local;auto& trace=diagnostics?*diagnostics:local;trace={};trace.year=s.year;
    Record franchise(r),home(r),away(r);
    if(!activeFranchise(r,manager,franchise.p,trace))return false;
    if(!r.integer(franchise.p,0x1c,s.dynasty))return trace.fail("Franchise LeagueID field unavailable");
    if(!s.dynasty)return trace.fail("Franchise LeagueID is zero");
    if(!r.value(gameRef,game)||!game)return trace.fail("launch game record unavailable");
    if(s.spring){
        const bool homeSide=s.springSide==0;auto& actual=homeSide?s.home:s.away;
        Record school(r);trace.side=homeSide?"spring home actual school":"spring away actual school";
        if(!r.related(game,homeSide?0x2a:0x12,school.p)||!school.p||!team(r,school.p,actual)||!actual.prestigeKnown||
           !turnout::native::school(r,school.p,actual))return trace.fail("spring actual-school support/prestige unavailable");
        if(!program(r,school.p,actual,s.year,api,trace))return false;
        actual.program.ap=0; // No current competitive poll appeal for an exhibition.
        return true;
    }
    trace.side="home";
    if(!r.related(game,0x2a,home.p)||!home.p)return trace.fail("home team record unavailable");
    if(!program(r,home.p,s.home,s.year,api,trace))return false;
    if(!turnout::fcs(s.away.id)){
        trace={};trace.side="away";trace.year=s.year;
        if(!r.related(game,0x12,away.p)||!away.p)return trace.fail("away team record unavailable");
        if(!program(r,away.p,s.away,s.year,api,trace))return false;
    }
    return true;
}
}
