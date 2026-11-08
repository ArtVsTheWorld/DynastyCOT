#define WIN32_LEAN_AND_MEAN
#include "../Source/NativeStartingAttendance.h"
#include <cassert>
#include <cstring>
#include <map>
#include <vector>
#include <cstdio>
using namespace attendance::turnout;
using namespace attendance::turnout::native;
template<class T>void put(A p,T v){std::memcpy(reinterpret_cast<void*>(p),&v,sizeof(v));}
A address(void* p){return reinterpret_cast<A>(p);}
struct Table;
struct Row {A vt=0,unused=0;Table* table=nullptr;int index=0;int refs=0;};
struct Table {
    A vt=0;unsigned char schema[0xe0]{};A columns[512]{};
    std::map<int,int> ints;std::map<int,float> reals;std::map<int,Row*> related;
};
A image=0,recordVt=0,tableVt=0;int enums=0,sets=0;bool sameConference=false;
A __fastcall schema(A p){return address(reinterpret_cast<Row*>(p)->table->schema);}
void __fastcall release(A p){--reinterpret_cast<Row*>(p)->refs;}
int __fastcall integer(A table,int row,A column){assert(row==0);return reinterpret_cast<Table*>(table)->ints[static_cast<int>(column)];}
int __fastcall enumeration(A table,int row,A column){++enums;return integer(table,row,column);}
bool __fastcall boolean(A table,int row,A column){return integer(table,row,column)!=0;}
float __fastcall realNumber(A table,int row,A column){assert(row==0);return reinterpret_cast<Table*>(table)->reals[static_cast<int>(column)];}
A __fastcall related(A table,A* out,int row,A column){
    assert(row==0);Row* record=reinterpret_cast<Table*>(table)->related[static_cast<int>(column)];if(record)++record->refs;*out=reinterpret_cast<A>(record);return address(out);
}
A __fastcall query(A object,A* out,unsigned id){assert(id==0x41525259u);++reinterpret_cast<Row*>(object)->refs;*out=object;return address(out);}
A __fastcall count(A){return 139;}
bool __fastcall same(A wrapper,A* home,A* away){assert(wrapper&&*home&&*away);return sameConference;}
bool __fastcall set(A service,unsigned key,A reference){
    assert(key==attendance::bindings::AttendanceOption);
    auto ref=reinterpret_cast<A*>(reference);assert(ref[0]==image+UnsignedType);int value=0;std::memcpy(&value,reinterpret_cast<void*>(ref[1]),4);put(service+0x14,value);++sets;return true;
}
void init(Table& t,Row& r){
    t.vt=tableVt;for(unsigned i=0;i<512;++i)t.columns[i]=i;
    put(address(t.schema)+0xd0,address(t.columns));put<short>(address(t.schema)+0xd8,512);
    r.vt=recordVt;r.table=&t;
}
void initTeam(Table& t,int id){t.ints={{0xd1,id},{0x182,50},{0x9e,17},{0x191,70},{0x180,6},{0x7a,2},{0x77,1},{0xec,1},{0xe9,2}};}
struct Member {A type,name;unsigned char unused[16];unsigned flags;unsigned reserved;unsigned short offset;unsigned char tail[6];};
static_assert(sizeof(Member)==48);
int main(){
    image=reinterpret_cast<A>(GetModuleHandleW(nullptr));std::vector<void*> memory;
    auto page=[&](A p){A base=p&~A(0xffff);for(void* existing:memory)if(address(existing)==base)return;void* block=VirtualAlloc(reinterpret_cast<void*>(base),0x10000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);assert(block==reinterpret_cast<void*>(base));memory.push_back(block);};
    recordVt=image+0xb100000;tableVt=recordVt+0x100;page(recordVt);
    put(recordVt+0x38,reinterpret_cast<A>(schema));put(recordVt+8,reinterpret_cast<A>(release));put(recordVt+0x28,reinterpret_cast<A>(query));
    put(tableVt+0x100,reinterpret_cast<A>(integer));put(tableVt+0x108,reinterpret_cast<A>(enumeration));put(tableVt+0xf8,reinterpret_cast<A>(boolean));put(tableVt+0x128,reinterpret_cast<A>(related));
    Table gt,ht,at,st,lt,seasont,arrt;Row game,home,away,stadium,league,season,array;
    init(gt,game);init(ht,home);init(at,away);init(st,stadium);init(lt,league);init(seasont,season);init(arrt,array);
    initTeam(ht,1153);initTeam(at,1193);st.ints={{6,553},{22,60000}};
    gt.ints={{0x3e,1},{0x33,0},{0x3d,8},{0x3f,2026},{36,0},{12,0}};
    gt.related={{0x2a,&home},{0x12,&away},{0x40,&stadium},{0x15,nullptr}};ht.related={{0x118,&stadium},{0xb,nullptr}};lt.related={{0xd,&array}};seasont.ints={{0x2c,15}};
    unsigned char manager[0x240]{};put(address(manager),image+StatManagerVtable);put(address(manager)+0x230,address(&league));put(address(manager)+0x228,address(&season));A reference=address(&game);
    Table hct,act;Row hc,ac;init(hct,hc);init(act,ac);hct.ints[24]=act.ints[24]=43;ht.related[0x17]=&hc;at.related[0x17]=&ac;
    // Deliberately give Texas ordinal43, independent of alphabetic ordering.
    unsigned char fields[25*0x58]{},enumType[0x50]{},enumNames[2*0x58]{};
    put(address(hct.schema)+0x40,address(fields));put(address(hct.schema)+0x48,address(fields)+sizeof(fields));
    put<unsigned>(address(fields)+24*0x58+0x40,3);put(address(fields)+24*0x58+0x50,address(enumType));
    put(address(enumType)+0x40,address(enumNames));put(address(enumType)+0x48,address(enumNames)+sizeof(enumNames));
    std::memcpy(enumNames,"Texas",5);enumNames[15]=10;put<int>(address(enumNames)+0x48,43);
    std::memcpy(enumNames+0x58,"Virginia",8);enumNames[0x58+15]=7;put<int>(address(enumNames)+0x58+0x48,7);
    Reader reader{image};auto texas=reader.stateCenter(address(&hc),24,43),virginia=reader.stateCenter(address(&hc),24,7);
    assert(std::isfinite(texas.latitude)&&texas.longitude<-95&&virginia.longitude>-85);
    assert(!std::isfinite(reader.stateCenter(address(&hc),24,8).latitude));
    put<unsigned>(address(fields)+24*0x58+0x40,2);assert(!std::isfinite(reader.stateCenter(address(&hc),24,43).latitude));
    put<unsigned>(address(fields)+24*0x58+0x40,3);
    page(image+0xbde66a0);page(image+0xbdd20b0);
    put<int>(image+0xbde66a0,17);put(image+0xbde66a8,reinterpret_cast<A>("Latitude"));
    put<int>(image+0xbde66b0,18);put(image+0xbde66b8,reinterpret_cast<A>("Longitude"));
    put<int>(image+0xbdd20b0,0x84);put(image+0xbdd20b8,reinterpret_cast<A>("IsMilitary"));
    put(tableVt+0x110,reinterpret_cast<A>(realNumber));hct.reals={{17,41.4f},{18,-74.f}};act.reals={{17,37.8f},{18,-122.3f}};ht.ints[0x84]=1;
    Api api{count,same,set};Seed seed;
    std::vector<Table> memberTables(137),schoolTables(138);std::vector<Row> memberRows(137),schoolRows(138);
    for(int i=0;i<138;++i){
        init(schoolTables[i],schoolRows[i]);schoolTables[i].ints={{34,i%13},{23,i%13},{5,i%13},{19,i%9}};
        if(i==0){ht.related[0xa5]=&schoolRows[i];arrt.related[i]=&home;}
        else if(i==1){at.related[0xa5]=&schoolRows[i];arrt.related[i]=&away;}
        else{init(memberTables[i-2],memberRows[i-2]);initTeam(memberTables[i-2],200+i);memberTables[i-2].ints[0x182]=i+1;memberTables[i-2].related[0xa5]=&schoolRows[i];arrt.related[i]=&memberRows[i-2];}
    }
    init(memberTables[136],memberRows[136]);initTeam(memberTables[136],1500);memberTables[136].ints[0x182]=256;arrt.related[138]=&memberRows[136];
    assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&seed.regular&&!seed.resumed&&seed.schools==138&&seed.home.id==1153&&seed.home.homeGames==3&&seed.home.wins==3&&seed.home.losses==3&&enums>0&&seed.home.atmosphere==0&&seed.home.standing>=0);
    auto noLeaks=[&](){for(Row* r:{&game,&home,&away,&stadium,&league,&season,&array})assert(r->refs==0);for(auto& r:memberRows)assert(r.refs==0);for(auto& r:schoolRows)assert(r.refs==0);};noLeaks();
    assert(seed.home.state==43&&seed.away.state==43&&hc.refs==0&&ac.refs==0);
    assert(seed.home.stateLatitude==texas.latitude&&seed.home.stateLongitude==texas.longitude&&!std::isfinite(seed.away.stateLatitude));
    assert(seed.home.latitude==41.4f&&seed.home.longitude==-74.f&&seed.away.longitude==-122.3f);
    assert(seed.home.militaryKnown&&seed.home.military&&seed.away.militaryKnown&&!seed.away.military);
    ht.related[0x17]=nullptr;assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&seed.home.state==-1);ht.related[0x17]=&hc;
    assert(seed.home.apRank==17&&seed.home.rank==50&&seed.home.prestige==6);
    ht.ints[0x9e]=0;assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&seed.home.apRank==0);noLeaks();ht.ints[0x9e]=17;
    ht.ints[0x9e]=-1;assert(!capture(image,address(manager),address(&reference),45000,seed,&api));noLeaks();ht.ints[0x9e]=17;
    schoolTables[0].ints[5]=13;assert(!capture(image,address(manager),address(&reference),45000,seed,&api));noLeaks();schoolTables[0].ints[5]=0;
    ht.related[0xa5]=nullptr;assert(!capture(image,address(manager),address(&reference),45000,seed,&api));noLeaks();
    Seed schools;assert(crowdIdentity(image,address(manager),address(&reference),schools)&&schools.home.id==1153&&schools.home.militaryKnown&&schools.home.military&&schools.away.militaryKnown&&!schools.away.military);noLeaks();
    ht.related[0xa5]=&schoolRows[0];
    gt.ints[0x3e]=2;assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&!seed.regular&&seed.home.militaryKnown&&seed.home.military);noLeaks();gt.ints[0x3e]=1;
    gt.related[0x15]=&stadium;assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&!seed.regular);noLeaks();gt.related[0x15]=nullptr;
    gt.ints[36]=7;assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&seed.resumed&&seed.home.militaryKnown&&seed.home.military);noLeaks();gt.ints[36]=0;
    gt.ints[0x3d]=15;sameConference=true;assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&!seed.regular);noLeaks();sameConference=false;assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&seed.regular);noLeaks();gt.ints[0x3d]=8;
    put(tableVt+0x128,A(0));assert(!capture(image,address(manager),address(&reference),45000,seed,&api));put(tableVt+0x128,reinterpret_cast<A>(related));noLeaks();
    // Real descriptor layout and typed setter contract; no writes to native records.
    for(A p:{image+OptionService,image+OptionType,image+OptionVtable,image+0xd9eb808,image+0xf63f9f0})page(p);
    unsigned char options[0x200]{},config[0x100]{};put(image+OptionService,address(options));put(address(options),image+OptionVtable);put(image+OptionVtable+0x80,image+0x90a1670);
    put<int>(image+0xd9eb808,14);put(image+0xf63f9f0,address(config));
    const char* names[]={"gameHomeTeamId","gameAwayTeamId","gameStadiumId","gameAttendance","gameTimeOfDay","gameWeatherPrecipitation","gameRivalryId","gameBowlId","gameFlagIsSeniorDay","gameWeatherDynamicEnabled"};
    unsigned keys[]={0x9e8d2700,0x9e1d2700,0xce7a6400,0x9e1d2500,0x9f4be400,0x9f7c2700,0x9f2e6900,0x9e2b2900,0x9e6ce400,0x9f793700};
    unsigned short offsets[]={0x30,0x20,0x46,0x14,0x3c,0x49,0x32,0x22,0x25,0x51};Member members[10]{};
    for(unsigned i=0;i<10;++i){members[i].name=reinterpret_cast<A>(names[i]);members[i].flags=keys[i];members[i].offset=offsets[i];}
    put(image+OptionType+0x30,address(members));put<unsigned short>(image+OptionType+0x38,10);
    put<unsigned short>(address(options)+0x30,1153);put<unsigned short>(address(options)+0x20,1193);put<unsigned short>(address(options)+0x46,553);put<int>(address(options)+0x14,45000);put<unsigned short>(address(options)+0x3c,1140);put<unsigned char>(address(options)+0x49,0);put<unsigned short>(address(options)+0x32,1023);put<unsigned char>(address(options)+0x22,127);
    Launch launchInput;assert(launch(image,launchInput,seed)&&launchInput.home==1153&&launchInput.capacity==60000&&launchInput.minutes==1140&&!launchInput.rivalry);
    put<unsigned short>(address(options)+0x32,119);
    assert(launch(image,launchInput,seed)&&launchInput.rivalry&&launchInput.rivalryEvent==119);
    put<unsigned short>(address(options)+0x32,1023);
    // Reported dry dynamic game had severity2 with a20% rain chance.
    // Severity is not a guaranteed current-weather observation in that mode.
    put<unsigned char>(address(options)+0x49,2);put<unsigned char>(address(options)+0x51,1);
    assert(launch(image,launchInput,seed)&&launchInput.weatherKnown&&launchInput.dynamicWeather&&launchInput.configuredPrecipitation==2&&launchInput.precipitation==0);
    auto dynamicResult=calculate(seed,launchInput);assert(dynamicResult.applied&&dynamicResult.weather==0);
    put<unsigned char>(address(options)+0x51,0);
    assert(launch(image,launchInput,seed)&&launchInput.weatherKnown&&!launchInput.dynamicWeather&&launchInput.precipitation==2);
    assert(calculate(seed,launchInput).weather==-9);
    members[9].name=reinterpret_cast<A>("unverifiedWeatherFlag");
    assert(launch(image,launchInput,seed)&&!launchInput.weatherKnown&&launchInput.precipitation==0);
    members[9].name=reinterpret_cast<A>(names[9]);put<unsigned char>(address(options)+0x51,2);
    assert(launch(image,launchInput,seed)&&!launchInput.weatherKnown&&launchInput.precipitation==0);
    put<unsigned char>(address(options)+0x49,4);assert(!launch(image,launchInput,seed));
    put<unsigned char>(address(options)+0x49,0);put<unsigned char>(address(options)+0x51,0);
    put<unsigned char>(address(options)+0x25,1);assert(launch(image,launchInput,seed)&&launchInput.seniorKnown&&launchInput.senior);
    members[8].name=reinterpret_cast<A>("unavailableSeniorFlag");assert(launch(image,launchInput,seed)&&!launchInput.seniorKnown&&!launchInput.senior);members[8].name=reinterpret_cast<A>("gameFlagIsSeniorDay");put<unsigned char>(address(options)+0x25,0);
    // Correct captured capacity binds without executing a native capacity RPC.
    auto captured=seed;
    for(int cap:{1000,75005,200000}){captured.capacity=cap;assert(launch(image,launchInput,captured)&&launchInput.capacity==cap);}
    for(int cap:{-1,0,999,200001}){captured=seed;captured.capacity=cap;const char* why=nullptr;assert(!launch(image,launchInput,captured,&why)&&!std::strcmp(why,"captured stadium capacity/launch identity mismatch"));}
    for(int field=0;field<4;++field){captured=seed;if(field==0)++captured.home.id;if(field==1)++captured.away.id;if(field==2)++captured.stadium;if(field==3)++captured.ea;assert(!launch(image,launchInput,captured));}
    assert(launch(image,launchInput,seed)&&launchInput.capacity==60000);
    // The -1 sentinel is allowed only when the caller provides fresh matched
    // launch proof. MMC handles online restrictions; gameplay config is unused.
    put(image+0xf63f9f0,A(0));assert(launch(image,launchInput,seed));
    put<int>(image+0xd9eb808,-1);assert(!launch(image,launchInput,seed));assert(launch(image,launchInput,seed,nullptr,true));
    for(int other:{-2,0,15,17,99}){put<int>(image+0xd9eb808,other);assert(!launch(image,launchInput,seed,nullptr,true)&&!setAttendance(image,51000,&api,true));}
    put<int>(image+0xd9eb808,14);put(image+0xf63f9f0,address(config));
    const char* failure=nullptr;members[2].name=reinterpret_cast<A>("wrongStadiumName");assert(!launch(image,launchInput,seed,&failure)&&!std::strcmp(failure,"gameStadiumId"));members[2].name=reinterpret_cast<A>(names[2]);
    unsigned char before[sizeof(options)];std::memcpy(before,options,sizeof(options));assert(setAttendance(image,50000,&api)&&sets==1);put<int>(address(before)+0x14,50000);assert(!std::memcmp(before,options,sizeof(options)));
    assert(gt.ints[9]==0&&seed.ea==45000); // The simulation/save attendance record never receives the launch override.
    members[3].flags|=1;assert(!setAttendance(image,51000,&api)&&sets==1);members[3].flags&=~1u;
    put<int>(image+0xd9eb808,-1);assert(!setAttendance(image,51000,&api)&&sets==1);assert(setAttendance(image,51000,&api,true)&&sets==2);put<int>(image+0xd9eb808,14);
    put<int>(image+0xd9eb808,17);assert(!launch(image,launchInput,seed)&&!setAttendance(image,51000,&api));put<int>(image+0xd9eb808,14);
    put<unsigned char>(address(options)+0x22,47);assert(!launch(image,launchInput,seed));
    for(void* block:memory)assert(VirtualFree(block,0,MEM_RELEASE));
    std::puts("PASS: captured stadium capacity with exact launch ownership/bounds and no capacity RPC, Dynasty setup, scoped -1 transition, other-mode rejection and no gameplay-config dependency, exact option failure names; native record/schema/int/enum/boolean ABI, owned-reference cleanup, regular/postseason/conference/resume classification, validated launch descriptors and isolated typed option write.");
}
