#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../Source/NativeAudience.h"
#include <cassert>
#include <map>
#include <cstring>
#include <iostream>
using namespace attendance;using namespace attendance::turnout::native;
template<class T>void put(A p,T v){std::memcpy(reinterpret_cast<void*>(p),&v,sizeof(v));}
struct Table;
struct Row {A vt=0,unused=0;Table* table=nullptr;int index=0,refs=0;};
struct Table {A vt=0;unsigned char schema[0xe0]{};A columns[512]{};std::map<int,int> ints;std::map<int,Row*> refs;};
int borrowed=0;A historySlots=5;
A __fastcall schema(A p){return reinterpret_cast<A>(reinterpret_cast<Row*>(p)->table->schema);}
void __fastcall release(A p){--reinterpret_cast<Row*>(p)->refs;--borrowed;}
int __fastcall integer(A p,int,A c){return reinterpret_cast<Table*>(p)->ints[static_cast<int>(c)];}
bool __fastcall boolean(A p,int row,A c){return integer(p,row,c)!=0;}
A __fastcall related(A p,A* out,int,A c){auto r=reinterpret_cast<Table*>(p)->refs[static_cast<int>(c)];if(r){++r->refs;++borrowed;}*out=reinterpret_cast<A>(r);return reinterpret_cast<A>(out);}
A __fastcall count(A){return historySlots;}
int main(){
    A image=reinterpret_cast<A>(GetModuleHandleW(nullptr));std::vector<void*> allocations;
    auto page=[&](A p){A aligned=p&~A(0xffff);for(auto v:allocations)if(reinterpret_cast<A>(v)==aligned)return;auto v=VirtualAlloc(reinterpret_cast<void*>(aligned),0x10000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);assert(v==reinterpret_cast<void*>(aligned));allocations.push_back(v);};
    A rv=image+0xb100000,tv=rv+0x100,mv=image+StatManagerVtable;page(rv);page(mv);
    put(rv+0x38,reinterpret_cast<A>(schema));put(rv+8,reinterpret_cast<A>(release));put(mv+0x38,reinterpret_cast<A>(schema));
    put(tv+0x100,reinterpret_cast<A>(integer));put(tv+0xf8,reinterpret_cast<A>(boolean));put(tv+0x128,reinterpret_cast<A>(related));
    auto init=[&](Table& t,Row& r){t.vt=tv;r.vt=rv;r.table=&t;for(unsigned i=0;i<512;++i)t.columns[i]=i;put(reinterpret_cast<A>(t.schema)+0xd0,reinterpret_cast<A>(t.columns));put<short>(reinterpret_cast<A>(t.schema)+0xd8,512);};
    Table ft,mt,lht,lt,gt,ht,at,history,stats;Row fr,manager,lhr,lr,clone,gr,hr,ar,hist,sr;
    init(ft,fr);init(mt,manager);manager.vt=mv;init(lht,lhr);init(lt,lr);init(lt,clone);init(gt,gr);init(ht,hr);init(at,ar);init(history,hist);init(stats,sr);
    mt.refs[1]=&lhr;mt.refs[0]=&lr;lht.refs[0]=&fr;ft.refs[0x1b]=&lr;lr.index=66;clone.index=66;
    ft.ints[0x1c]=-1649480587;ht.ints[0x88]=1;at.ints[0x88]=0;hr.index=7;ar.index=9;
    gt.refs[0x2a]=&hr;gt.refs[0x12]=&ar;ht.refs[0x184]=&hist;at.refs[0x184]=nullptr;
    history.refs[1]=&sr;history.refs[2]=&sr;history.refs[3]=&sr;stats.ints={{57,8},{22,4},{53,0}};
    A game=reinterpret_cast<A>(&gr);
    turnout::Seed s;s.year=0;s.home.id=1801;s.home.prestige=3;s.home.atmosphere=8;s.home.apRank=71;s.away.id=1251;s.away.prestige=0;s.away.atmosphere=12;s.away.apRank=138;
    Api api{count,nullptr,nullptr};audiencenative::CaptureTrace trace;
    auto capture=[&](){return audiencenative::capture(image,reinterpret_cast<A>(&manager),reinterpret_cast<A>(&game),s,&api,&trace);};
    auto failure=[&](const char* reason,const char* side){assert(!capture()&&borrowed==0);assert(trace.failure&&!std::strcmp(trace.failure,reason)&&!std::strcmp(trace.side,side));assert(trace.describe().find(reason)!=std::string::npos);};
    assert(capture());assert(borrowed==0&&!trace.failure&&trace.year==0&&trace.historyCount==0);
    assert(s.dynasty==-1649480587&&s.home.program.custom&&s.home.program.row==7&&audience::recent(s.home.program)==-1);
    int dynastyIdentity=0;assert(audiencenative::identity(image,reinterpret_cast<A>(&manager),dynastyIdentity)&&dynastyIdentity==-1649480587&&borrowed==0);
    ht.columns[0x184]=A(-1);assert(capture()&&borrowed==0); // Inherited history is not required at TeamBuilder Year 0.
    ht.ints[0x88]=0;failure("TeamSeasonStats relationship unavailable","home");ht.ints[0x88]=1;
    s.year=1;failure("TeamSeasonStats relationship unavailable","home");ht.columns[0x184]=0x184;
    s.year=1;assert(capture());assert(borrowed==0&&std::abs(audience::recent(s.home.program)-2.f/3)<.001f&&s.home.program.history[1].wins==0);
    s.year=3;assert(capture());assert(borrowed==0&&s.home.program.history[2].wins==8);
    stats.ints[57]=99;failure("completed-season results outside supported range","home");stats.ints[57]=8;
    mt.refs[1]=nullptr;failure("statistics manager's league-history relationship unavailable","dynasty");mt.refs[1]=&lhr;
    lht.refs[0]=nullptr;failure("league-history manager's active dynasty relationship unavailable","dynasty");lht.refs[0]=&fr;
    mt.refs[0]=nullptr;failure("statistics manager's active league unavailable","dynasty");mt.refs[0]=&lr;
    ft.refs[0x1b]=nullptr;failure("dynasty's owning league unavailable","dynasty");ft.refs[0x1b]=&clone;
    assert(capture()&&borrowed==0); // Different wrappers for the same record are valid.
    clone.index=67;failure("dynasty and launch statistics manager refer to different leagues","dynasty");assert(!audiencenative::identity(image,reinterpret_cast<A>(&manager),dynastyIdentity)&&dynastyIdentity==0&&borrowed==0);clone.index=66;
    clone.table=&ft;failure("dynasty and launch statistics manager refer to different leagues","dynasty");clone.table=&lt;
    clone.index=-1;failure("dynasty and launch statistics manager refer to different leagues","dynasty");clone.index=66;ft.refs[0x1b]=&lr;
    ft.ints[0x1c]=0;failure("Franchise LeagueID is zero","dynasty");ft.ints[0x1c]=-1649480587;
    ft.columns[0x1c]=A(-1);failure("Franchise LeagueID field unavailable","dynasty");ft.columns[0x1c]=0x1c;
    s.year=2026;failure("dynasty year outside supported range","home");assert(trace.year==2026&&trace.row==7&&trace.custom==1);s.year=3;
    historySlots=6;failure("season-history slot count exceeds verified layout","home");assert(trace.historyCount==6);historySlots=5;
    ht.columns[0x88]=A(-1);failure("IsTeamBuilder field unavailable","home");ht.columns[0x88]=0x88;
    stats.columns[22]=A(-1);failure("completed-season LOSSES field unavailable","home");stats.columns[22]=22;
    at.columns[0x88]=A(-1);failure("IsTeamBuilder field unavailable","away");at.columns[0x88]=0x88;
    s.away.id=1501;gt.refs[0x12]=nullptr;assert(capture()&&!trace.failure&&borrowed==0&&!std::strcmp(trace.side,"home"));
    assert(trace.row==7&&trace.custom==1&&trace.historyCount==5);s.away.id=1251;gt.refs[0x12]=&ar;
    manager.vt=rv;failure("launch statistics manager identity unavailable","dynasty");manager.vt=mv;
    assert(capture()&&!trace.failure&&borrowed==0); // Failure state cannot leak into a subsequent successful launch.
    struct Label {A address;int key;const char* name;};Label labels[]={{0xbdd30b8,0x184,"TeamSeasonStats"},{0xbdc5208,0,"LeagueData"},{0xbdc5218,1,"LeagueHistoryManager"},{0xbde1278,0,"Dynasty"},{0xbded4e8,0x1b,"League"},{0xbdd20f8,0x88,"IsTeamBuilder"},{0xbded4f8,0x1c,"LeagueID"},{0xbdeeca8,57,"WINS"},{0xbdeea78,22,"LOSSES"},{0xbdeec68,53,"TIES"}};
    for(auto l:labels){page(image+l.address);put(image+l.address,reinterpret_cast<A>(l.name));put(image+l.address-8,l.key);}
    assert(audiencenative::bindingsReady(image));
    for(auto l:labels){put<int>(image+l.address-8,999);assert(!audiencenative::bindingsReady(image));put<int>(image+l.address-8,l.key);}
    assert(audiencenative::bindingsReady(image));
    for(auto p:allocations)VirtualFree(p,0,MEM_RELEASE);
    std::cout<<"PASS: active statistics/history/dynasty relationships without a named-table/FIRST lookup, owning-league identity checks, wrapper equality, enum ordinals, TeamBuilder/history provenance, staged diagnostics, FCS exclusion and balanced borrowed references on all failures.\n";
}
