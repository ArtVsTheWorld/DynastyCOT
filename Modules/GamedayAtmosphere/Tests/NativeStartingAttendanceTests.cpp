#define WIN32_LEAN_AND_MEAN
#include "../Source/NativeStartingAttendance.h"
#include "../Source/AppearanceAdmission.h"
#include "../Source/AwayCrowd.h"
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
    A vt=0,descriptor=0;unsigned char schema[0xe0]{};A columns[512]{};
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
    t.vt=tableVt;t.descriptor=address(t.schema);for(unsigned i=0;i<512;++i)t.columns[i]=i;
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
    put<int>(image+0xbdd3070,0x180);put(image+0xbdd3078,reinterpret_cast<A>("TeamPrestige"));
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
    gt.related[0x15]=&stadium;assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&!seed.regular&&seed.bowlFixture&&grades(seed.home)&&grades(seed.away));noLeaks();
    for(int type:{1,2,3,4,5}){gt.ints[0x3e]=type;assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&seed.bowlFixture&&seed.bowlRelated&&!seed.practiceFixture&&!seed.regular&&seed.springType==type);noLeaks();}
    for(int type:{0,6,7,8,9,-1}){gt.ints[0x3e]=type;assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&!seed.bowlFixture&&seed.bowlRelated);noLeaks();}
    gt.ints[0x3e]=5;gt.ints[0x33]=1;assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&!seed.bowlFixture&&seed.practiceFixture);gt.ints[0x33]=0;
    assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&seed.bowlFixture&&seed.springType==5);noLeaks();
    auto postseasonSeed=seed;gt.related[0x15]=nullptr;gt.ints[0x3e]=1;
    gt.ints[36]=7;assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&seed.resumed&&seed.home.militaryKnown&&seed.home.military);noLeaks();gt.ints[36]=0;
    gt.ints[0x3d]=15;sameConference=true;assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&!seed.regular);noLeaks();sameConference=false;assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&seed.regular);noLeaks();gt.ints[0x3d]=8;
    put(tableVt+0x128,A(0));assert(!capture(image,address(manager),address(&reference),45000,seed,&api));put(tableVt+0x128,reinterpret_cast<A>(related));noLeaks();
    // Semantic field-name resolution: marker excluded, actual school resolved
    // independently of scoreboard slots, failed/ambiguous metadata stays native.
    std::vector<unsigned char> gameFields(512*0x58,0);auto gf=address(gameFields.data());
    for(int i=0;i<512;++i)gameFields[i*0x58+15]=15;
    std::memcpy(gameFields.data()+5*0x58,"SeasonGameNum",13);gameFields[5*0x58+15]=2;put<unsigned>(gf+5*0x58+0x40,2);
    put(address(gt.schema)+0x40,gf);put(address(gt.schema)+0x48,gf+gameFields.size());gt.ints[5]=4;gt.ints[0x3e]=0;
    put<int>(image+0xbdd9e50,0x3c);put(image+0xbdd9e58,reinterpret_cast<A>("SeasonGameNum"));
    put<int>(image+0xbdd1960,0xf);put(image+0xbdd1968,reinterpret_cast<A>("BlacklistUserIdLower"));
    gt.columns[0x3c]=5; // native game class key60 maps to database column5
    gt.columns[5]=6;gt.ints[6]=250; // class-key map intentionally differs from the named database column
    schoolTables[1].ints[34]=13; // actual school grades are not ready in spring
    ht.ints[0xd1]=1502;ht.related[0xa5]=nullptr;
    assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&seed.spring&&seed.springSide==1&&seed.springSchool==1193&&seed.springNumber==4);noLeaks();
    assert(seed.springNumberKnown&&seed.springRoleEvidence==6); // away tracking + fixture home venue
    at.related[0xa5]=nullptr;at.related[0x118]=&stadium;
    assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&seed.spring&&seed.springSide==1&&(seed.springRoleEvidence&8));noLeaks();
    at.related[0x118]=nullptr;assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&!seed.spring);noLeaks();
    at.related[0xa5]=&schoolRows[1];schoolTables[1].ints[34]=1;
    gt.columns[5]=5;
    gt.ints[5]=250;assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&!seed.spring);noLeaks();
    // Observed Extensions 0.3 BeginGame: game250, stand-in1502, native team
    // field15=1200000. A launched stamped row can use regular week type; the
    // stamp, real-school ownership and fresh Dynasty launch still qualify it.
    for(int type:{0,1})for(int stamp:{1100000,1200000}){
        gt.ints[0x3e]=type;ht.ints[0xf]=stamp;
        assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&seed.spring&&!seed.regular&&seed.springStamp==stamp&&seed.springNumber==250&&seed.springSide==1);noLeaks();
    }
    gt.ints[0x3e]=0;
    for(int stamp:{0,1099999,1100001,1199999,1200001}){ht.ints[0xf]=stamp;assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&!seed.spring);noLeaks();}
    ht.ints[0xf]=1200000;put(image+0xbdd1968,reinterpret_cast<A>("OtherField"));assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&!seed.spring);noLeaks();
    put(image+0xbdd1968,reinterpret_cast<A>("BlacklistUserIdLower"));put<int>(image+0xbdd9e50,59);assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&!seed.spring);noLeaks();
    put<int>(image+0xbdd9e50,60);ht.ints[0xf]=0;
    gt.ints[5]=4;gt.ints[0x33]=1;assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&!seed.spring);gt.ints[0x33]=0;
    ht.ints[0xd1]=1153;ht.related[0xa5]=&schoolRows[0];gt.ints[0x3e]=1;
    assert(capture(image,address(manager),address(&reference),45000,seed,&api)&&seed.regular&&!seed.spring);noLeaks();
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
    // Regression: 2.1.33 rejected the shared launch on gameBowlId before
    // publishing CrowdFixture, so appearance preparation never had an owner.
    put<int>(address(options)+0x14,45000);put<unsigned char>(address(options)+0x22,47);assert(!launch(image,launchInput,seed));
    assert(!launch(image,launchInput,postseasonSeed)); // Turnout API remains strict.
    assert(!launch(image,launchInput,seed,nullptr,false,LaunchUse::Supporters)); // Not a captured bowl fixture.
    assert(launch(image,launchInput,postseasonSeed,nullptr,false,LaunchUse::Supporters)&&launchInput.bowl==47);
    assert(!turnoutEligible(postseasonSeed,launchInput)&&!calculate(postseasonSeed,launchInput).applied);
    postseasonSeed.dynasty=42;postseasonSeed.serial=1;postseasonSeed.observed=100;postseasonSeed.dynastyVerifiedAt=99;postseasonSeed.dynastyGeneration=1;
    assert(capturedLaunchProof(postseasonSeed,launchInput,101,1)&&!transitionProof(postseasonSeed,launchInput,101,1));
    std::vector<unsigned char> untouched(options,options+sizeof(options));auto previousSets=sets;assert(!setAttendance(image,51000,&api)&&sets==previousSets&&!std::memcmp(options,untouched.data(),sizeof(options)));
    postseasonSeed.home.audienceFans=100000;postseasonSeed.away.audienceFans=75000;launchInput.neutral=true;
    CrowdFixture fixture{postseasonSeed,launchInput,true};
    using namespace attendance::visitors;
    assert(appearanceAdmission({},1153,1193,true)==AppearanceStage::NoFixture);
    assert(appearanceAdmission(fixture,1153,1193,true)==AppearanceStage::Prepared);
    assert(appearanceAdmission(fixture,1193,1153,true)==AppearanceStage::Ownership);
    assert(appearanceAdmission(fixture,1153,1193,false)==AppearanceStage::Disabled);
    auto supporterPlan=plan(fixture,1153,1193);assert(supporterPlan.valid&&!supporterPlan.native&&supporterPlan.layoutVersion==attendance::visitors::CurrentLayoutVersion&&supporterPlan.share>=.30f&&supporterPlan.share<=.70f);
    assert(!plan(fixture,1193,1153).valid);auto missing=fixture;missing.seed.away.audienceFans=0;assert(!plan(missing,1153,1193).valid);
    // Every relevant identity and descriptor guard still applies in supporter mode.
    for(int field=0;field<4;++field){auto wrong=postseasonSeed;if(field==0)++wrong.home.id;if(field==1)++wrong.away.id;if(field==2)++wrong.stadium;if(field==3)++wrong.ea;assert(!launch(image,launchInput,wrong,nullptr,false,LaunchUse::Supporters));}
    for(int other:{-2,0,15,17,99}){put<int>(image+0xd9eb808,other);assert(!launch(image,launchInput,postseasonSeed,nullptr,true,LaunchUse::Supporters));}put<int>(image+0xd9eb808,14);
    members[7].name=reinterpret_cast<A>("wrongBowlName");assert(!launch(image,launchInput,postseasonSeed,nullptr,false,LaunchUse::Supporters));members[7].name=reinterpret_cast<A>(names[7]);
    for(int bowlId:{0,47,126}){put<unsigned char>(address(options)+0x22,static_cast<unsigned char>(bowlId));assert(launch(image,launchInput,postseasonSeed,nullptr,false,LaunchUse::Supporters)&&!setAttendance(image,51000,&api)&&sets==previousSets);}
    for(int invalid:{128,255}){put<unsigned char>(address(options)+0x22,static_cast<unsigned char>(invalid));assert(!launch(image,launchInput,postseasonSeed,nullptr,false,LaunchUse::Supporters));}
    put<unsigned char>(address(options)+0x22,47);
    for(int invalidType:{0,6,7,8,9,-1,255}){auto invalid=postseasonSeed;invalid.springType=invalidType;assert(!launch(image,launchInput,invalid,nullptr,false,LaunchUse::Supporters));}
    for(int field=0;field<2;++field){auto invalid=postseasonSeed;if(field==0)invalid.bowlRelated=false;else invalid.practiceFixture=true;assert(!launch(image,launchInput,invalid,nullptr,false,LaunchUse::Supporters));}
    // Exact reported championship tuple: native type5, Week20, bowl47.
    ht.ints[0xd1]=1212;at.ints[0xd1]=1205;gt.ints[0x3e]=5;gt.ints[0x3d]=20;gt.related[0x15]=&stadium;st.ints[6]=615;st.ints[22]=65000;
    Seed observedChampionship;assert(capture(image,address(manager),address(&reference),65000,observedChampionship,&api)&&observedChampionship.bowlFixture&&observedChampionship.springType==5);
    put<unsigned short>(address(options)+0x30,1212);put<unsigned short>(address(options)+0x20,1205);put<unsigned short>(address(options)+0x46,615);put<int>(address(options)+0x14,65000);
    assert(launch(image,launchInput,observedChampionship,nullptr,false,LaunchUse::Supporters)&&!turnoutEligible(observedChampionship,launchInput)&&!setAttendance(image,51000,&api)&&sets==previousSets);
    noLeaks();put<unsigned short>(address(options)+0x30,1153);put<unsigned short>(address(options)+0x20,1193);put<unsigned short>(address(options)+0x46,553);put<int>(address(options)+0x14,45000);
    put<unsigned char>(address(options)+0x22,127);assert(launch(image,launchInput,seed)&&!seed.bowlFixture); // Regular route preserved.
    for(void* block:memory)assert(VirtualFree(block,0,MEM_RELEASE));
    std::puts("PASS: captured capacity/ownership, native descriptor/ABI/reference cleanup, verified competitive types1-5 including observed Week20 final type5/bowl47; practice, unknown types, missing Bowl proof and contradictory flags rejected; postseason turnout/setter blocked; regular and spring contracts preserved.");
}
