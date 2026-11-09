#include "../Source/NativeRuntime.cpp"
#include <cassert>
#include <iostream>
int main(){
    unsigned checks=0;
    for(int m:{-1,0,13,14,15,16,17,18})for(int session:{0,1,2,3,4})for(int practice:{0,1,2}){
        assert(attendance::scope::offlineMatch(m,session,practice)==((m==14||m==17)&&session==3&&practice==0));++checks;
    }
    auto region=VirtualAlloc(nullptr,0x11000000,MEM_RESERVE,PAGE_NOACCESS);assert(region);image=reinterpret_cast<A>(region);
    auto commit=[&](A rva){assert(VirtualAlloc(reinterpret_cast<void*>((image+rva)&~A(0xfff)),4096,MEM_COMMIT,PAGE_READWRITE));};
    for(A rva:{A(0xd9eb808),A(0xf3fddbc),A(0xf63f9f0),A(0xd9fba20),attendance::bindings::MatchWrapper,attendance::bindings::GameStates,attendance::bindings::ClockManager})commit(rva);
    auto put=[](A p,auto value){std::memcpy(reinterpret_cast<void*>(p),&value,sizeof(value));};
    alignas(16) unsigned char configuration[512]{},wrapper[64]{},match[1024]{},states[512]{},clock[512]{},time[128]{};
    A cfg=reinterpret_cast<A>(configuration),w=reinterpret_cast<A>(wrapper),mt=reinterpret_cast<A>(match),st=reinterpret_cast<A>(states),cl=reinterpret_cast<A>(clock),tm=reinterpret_cast<A>(time);
    put(image+0xf63f9f0,cfg);put(image+attendance::bindings::MatchWrapper,w);put(w+16,mt);put(image+attendance::bindings::GameStates,st);put(image+attendance::bindings::ClockManager,cl);
    put(mt,image+A(0xb47e908));put(st,image+A(0xb47f790));put(cl,image+A(0xb47eec8));put(cl+0x28,tm);put(cl+8,1);put(st+0xa0,1);put(tm+0x28,600.f);put(tm+0x2c,720.f);
    for(int m:{13,14,17,18})for(int session:{2,3,4})for(int practice:{0,1,2}){
        put(image+0xd9eb808,m);put(image+0xf3fddbc,session);configuration[0x84]=static_cast<unsigned char>(practice);
        attendance::State state;A actual=0;assert(gameState(state,actual)==((m==14||m==17)&&session==3&&practice==0));++checks;
    }
    put(image+0xd9eb808,17);put(image+0xf3fddbc,3);configuration[0x84]=0;
    for(A invalid:{mt,st,cl}){
        A old=at<A>(invalid);put(invalid,A(1));attendance::State state;A actual=0;assert(!gameState(state,actual));put(invalid,old);++checks;
    }
    matchLifecycle.join(mt,false);configuration[0x84]=1;observeWorldExit();assert(matchLifecycle.leftWorld);++checks;
    matchLifecycle.join(mt,false);configuration[0x84]=0;observeWorldExit();assert(!matchLifecycle.leftWorld);++checks;
    put(image+0xd9fba20,1176);put(image+0xd9fba24,1175);EmptyWorld empty;
    assert(emptyWorld(empty));++checks;
    configuration[0x84]=1;assert(!emptyWorld(empty));++checks;configuration[0x84]=0;
    put(image+0xf3fddbc,4);assert(!emptyWorld(empty));++checks;
    put(image+0xd9eb808,18);assert(!emptyWorld(empty));++checks;
    put(image+0xd9eb808,14);put(image+0xf3fddbc,1);assert(emptyWorld(empty));++checks; // Existing visual-only scope retained.
    put(image+0xd9eb808,17);put(image+0xf3fddbc,3);
    // The production Play Now history paths must preserve stored Dynasty data.
    attendance::timeline::Game archived;archived.id=987;archived.resumeReady=true;
    archived.context.team0=101;archived.context.team1=202;
    attendance::timeline::Point point;point.time=123;point.loss=37;point.temporary=42;
    archived.points.push_back(point);attendanceArchive.games.push_back(archived);
    std::ostringstream beforeHistory;attendance::timeline::write(beforeHistory,attendanceArchive);
    attendanceArchive.revision=123;attendanceArchive.active=987;
    auto original=attendanceArchive.games.size();lastMatch=mt;bootstrapPending=true;
    attendance::State state;assert(gameState(state,mt));beginAttendanceHistory(state,false);
    assert(!bootstrapPending&&attendanceArchive.revision==123&&attendanceArchive.games.size()==original&&attendanceArchive.active==0);++checks;
    assert(std::atomic_load(&sessionCrowdPlan)->match==lastMatch);++checks;
    recordAttendance();assert(attendanceArchive.revision==123&&attendanceArchive.games.size()==original);++checks;
    std::ostringstream afterHistory;attendance::timeline::write(afterHistory,attendanceArchive);assert(beforeHistory.str()==afterHistory.str());++checks;
    assert(VirtualFree(region,0,MEM_RELEASE));
    std::cout<<"PASS: "<<checks<<" offline match admission, native type and Play Now history isolation checks; no live game access.\n";
}
