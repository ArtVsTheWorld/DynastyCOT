// Production root selectors operating entirely on THIS process's mock pages.
#include "../Source/NativeRuntime.cpp"
#include <cassert>
#include <iostream>
namespace {
std::vector<void*> pages;
template<class T>void put(A address,T value){std::memcpy(reinterpret_cast<void*>(address),&value,sizeof(value));}
void page(A address){A base=address&~A(0xffff);for(auto p:pages)if(reinterpret_cast<A>(p)==base)return;void* p=VirtualAlloc(reinterpret_cast<void*>(base),65536,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);assert(reinterpret_cast<A>(p)==base);pages.push_back(p);}
template<class T>void global(A rva,T value){page(image+rva);put(image+rva,value);}
float __fastcall firstLine(A){return 10.f;}
void stub(A rva){page(image+rva);unsigned char code[]={0x48,0xb8,0,0,0,0,0,0,0,0,0xff,0xe0};A target=reinterpret_cast<A>(firstLine);std::memcpy(code+2,&target,8);std::memcpy(reinterpret_cast<void*>(image+rva),code,sizeof(code));FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(image+rva),sizeof(code));}
bool nativeAnswer=true;unsigned eventCalls=0;A nestedComparison=0,nestedVariant=0;
bool __fastcall logic(A node,A){bool answer=nativeAnswer;if(nestedComparison){museComparisonValue(nestedComparison,nestedVariant);answer=at<unsigned char>(nestedVariant)!=0;}put(node+0x38,static_cast<unsigned char>(answer));put(node+0x40,4);return true;}
void __fastcall event(A){++eventCalls;}
struct Expressions {
    std::vector<unsigned char> bytes=std::vector<unsigned char>(65536);size_t next=0;
    A allocate(size_t size){A p=reinterpret_cast<A>(bytes.data()+next);next+=size;assert(next<bytes.size());return p;}
    A fn(const char* id){A n=allocate(0x50);put(n,image+attendance::bindings::MuseFunctionVtable);std::memcpy(reinterpret_cast<void*>(n+0x48),id,std::strlen(id)+1);return n;}
    A comparison(int op,const char* id,int type,int value){A n=allocate(0x30),constant=allocate(0x18);put(n,image+attendance::bindings::MuseComparisonVtable);put(n+8,fn(id));put(n+0x18,constant);put(n+0x28,op);put(constant,image+attendance::bindings::MuseConstantVtable);put(constant+8,value);put(constant+0x10,type);return n;}
    A compare(const char* id){return comparison(0,id,4,1);}
    A logic(std::initializer_list<A> children,int op=0){A n=allocate(0x50),entries=allocate(children.size()*16);put(n,image+attendance::bindings::MuseLogicVtable);put(n+8,entries);put(n+0x10,entries+children.size()*16);put(n+0x48,op);size_t i=0;for(A child:children)put(entries+16*i++,child);return n;}
    A root(A child){return logic({child});}
    A quickTouchdown(){return logic({logic({comparison(4,"178",0,19),comparison(5,"540",0,20),comparison(5,"540",0,20),comparison(5,"540",0,20),comparison(5,"540",0,20),comparison(5,"540",0,20),comparison(5,"540",0,20),comparison(5,"540",0,20),comparison(5,"540",0,20)},1),comparison(0,"157",4,0),comparison(0,"556",4,1)});}

};
}
int main(){
 image=0x500000000ull;
 alignas(16) unsigned char cfg[512]{},states[512]{},wrapper[64]{},match[1024]{},clock[512]{},time[128]{},recorder[0x1040]{};unsigned char paused=0;
 A mt=reinterpret_cast<A>(match),st=reinterpret_cast<A>(states),cl=reinterpret_cast<A>(clock),rec=reinterpret_cast<A>(recorder);
 global(0xd9eb808,14);global(0xf3fddbc,3);global(0xf63f9f0,reinterpret_cast<A>(cfg));global(attendance::bindings::MatchWrapper,reinterpret_cast<A>(wrapper));put(reinterpret_cast<A>(wrapper)+16,mt);
 global(attendance::bindings::GameStates,st);global(attendance::bindings::ClockManager,cl);global(attendance::bindings::ReplayManager,rec);global(attendance::bindings::PauseFlag,reinterpret_cast<A>(&paused));
 global(attendance::bindings::PlayStats,A(0));global(attendance::bindings::SelectedPlayer,-1);global(0xd9fba20,1119);global(0xd9fba24,1120);
 put(mt,image+attendance::bindings::MatchVtable);put(st,image+attendance::bindings::GameStatesVtable);put(st+0xa0,4);put(cl,image+A(0xb47eec8));put(cl+8,1);put(cl+0x28,reinterpret_cast<A>(time));put(reinterpret_cast<A>(time)+0x28,600.f);put(reinterpret_cast<A>(time)+0x2c,720.f);
 put(rec,image+attendance::bindings::ReplayVtable);put(rec+0x1030,20.0);put(mt+0x1d0,1);put(mt+0x1c8,0);put(mt+0x16c,0.f);stub(attendance::bindings::FirstDownLine);
 enabled=true;mode=0;extraReplays=true;bigPlayReplays=true;routineInsets=true;replayPresentationInstalled=true;replayPresentationFailed=false;pauseBinding=true;replayBinding=true;majorReplayBindings=true;playBindingsAvailable=true;originalMuseLogic=logic;originalReplayEventNode=event;
 unsigned checks=0;
 for(int nativeMode:{14,17}){
  global(0xd9eb808,nativeMode);assert(replayPresentationAllowed());++checks;
  cfg[0x84]=1;assert(!replayPresentationAllowed());cfg[0x84]=0;++checks;
  global(0xf3fddbc,4);assert(!replayPresentationAllowed());global(0xf3fddbc,3);++checks;
  for(int points:{0,3,6,7,8})for(bool recording:{false,true})for(bool quickTd:{false,true}){
   Expressions nodes;A quick=quickTd?nodes.quickTouchdown():nodes.root(nodes.compare("556"));
   replayPlayTracker.reset();replayPlayTracker.serial=1;replayPlayTracker.start.game.homeScore=0;replayPlayTracker.start.game.awayScore=0;replayPlayTracker.start.down=1;replayPlayTracker.start.offense=0;
   replayTrackerMatch=mt;replayTrackerEpoch=audioEpoch.load();replayRecordingSequence=1;replayRecordingStart=recording?10:-1;replayTouchdown.reset();put(mt+0x1d8,points);put(mt+0x1dc,0);
   nativeAnswer=true;assert(museLogicEvaluate(quick,0));
   bool requested=false; // Production never overrides a touchdown shortcut.

   assert((at<unsigned char>(quick+0x38)==0)==requested);++checks;
   // No native touchdown facts are ever forged, even for a confirmed score.
   A touchdown=nodes.root(nodes.fn("27"));nativeAnswer=false;assert(museLogicEvaluate(touchdown,0)&&at<unsigned char>(touchdown+0x38)==0);++checks;
   // Native authored inset events are never consumed for a touchdown.
   if(points>=6){A pip=nodes.allocate(0x60);put(pip,image+attendance::bindings::ReplayEventNodeVtable);put(pip+0x50,12);put(pip+0x1a,static_cast<unsigned short>(1));auto before=eventCalls;replayEventNode(pip);assert(eventCalls==before+1&&at<unsigned short>(pip+0x1a)==1);++checks;}
  }
 }
 // Exercise root admission -> real nested production comparison, so a
 // Dynasty-only root regression cannot be hidden by testing the child alone.
 for(int nativeMode:{14,17})for(int block=0;block<10;++block){
  Expressions nodes;A cmp=nodes.comparison(5,"22",0,40),root=nodes.logic({nodes.fn("1"),cmp}),variant=nodes.allocate(0x18);
  A gain=at<A>(cmp+8); // The function's cached native value, not a fabricated game stat.
  put(gain+8,20);put(gain+0x10,0);put(variant+8,4);put(variant,0);
  global(0xd9eb808,nativeMode);global(0xf3fddbc,3);cfg[0x84]=0;paused=0;put(st+0xa0,4);put(rec+0x38,static_cast<unsigned char>(0));enabled=true;extraReplays=true;bigPlayReplays=true;replayMinimumYards=15;
  replayPlayTracker.reset();replayPlayTracker.serial=1;replayTrackerMatch=mt;replayTrackerEpoch=audioEpoch.load();replayTouchdown.reset();replayTouchdown.observe(mt,audioEpoch.load(),1,0,0,0,0,true,false);put(mt+0x1d8,0);put(mt+0x1dc,0);
  if(block==1)cfg[0x84]=1;else if(block==2)global(0xf3fddbc,4);else if(block==3)paused=1;else if(block==4)put(st+0xa0,1);else if(block==5)extraReplays=false;else if(block==6)bigPlayReplays=false;else if(block==7)replayTouchdown.nativeTouchdown();else if(block==8)replayTouchdown.reset();else if(block==9)put(rec+0x38,static_cast<unsigned char>(1));
  originalMuseComparison=[](A,A output)->A{return output;};nestedComparison=cmp;nestedVariant=variant;nativeAnswer=false;
  assert(museLogicEvaluate(root,0));assert((at<unsigned char>(variant)!=0)==(block==0));++checks;nestedComparison=nestedVariant=0;
 }
 global(0xd9eb808,18);assert(!replayPresentationAllowed());++checks;
 for(void* p:pages)assert(VirtualFree(p,0,MEM_RELEASE));
 std::cout<<"PASS: "<<checks<<" production replay root checks: Dynasty/Play Now, online/practice rejection, recorded non-TD/TD routing, native touchdown facts, inset passthrough and no repeat after full playback.\n";
}
