#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <intrin.h>
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <set>
#include <string>
#include <vector>

using A=uintptr_t;
extern "C" void CrowdTickGate();
extern "C" { void* TickOriginal=nullptr; }
namespace {
A image=0; HMODULE self=nullptr; std::filesystem::path folder;
std::mutex logMutex;
std::atomic<int> request{0};
std::atomic<unsigned long long> ticks{0};
std::vector<A> entities;
std::atomic<bool> enabled{false};
thread_local bool applying=false;
thread_local bool thinning=false;
thread_local unsigned int removalLimit=32768;
thread_local unsigned int calls=0,changed=0;
thread_local std::set<A> observed;
using ReadProperty=void*(__fastcall*)(void*,void*);
using SetVisibility=void(__fastcall*)(A,unsigned int,unsigned int,bool);
ReadProperty originalRead=nullptr;
SetVisibility originalSet=nullptr;
unsigned long long restoreAt=0;
float threshold=1.0f; bool hide=false;
bool signatures=false;
bool read(A p,void* out,size_t n);
struct Patch {A site=0;void* trampoline=nullptr;std::vector<unsigned char> original,replacement;};
std::vector<Patch> patches;
void jump(unsigned char* p,A target){p[0]=0xff;p[1]=0x25;memset(p+2,0,4);memcpy(p+6,&target,8);}
bool makePatch(A rva,void* detour,void** original,size_t length){
    Patch p;p.site=image+rva;p.original.resize(length);p.replacement.assign(length,0x90);
    if(!read(p.site,p.original.data(),length))return false;
    p.trampoline=VirtualAlloc(nullptr,length+14,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);if(!p.trampoline)return false;
    memcpy(p.trampoline,p.original.data(),length);jump(static_cast<unsigned char*>(p.trampoline)+length,p.site+length);
    DWORD old=0;if(!VirtualProtect(p.trampoline,length+14,PAGE_EXECUTE_READ,&old)){VirtualFree(p.trampoline,0,MEM_RELEASE);return false;}
    FlushInstructionCache(GetCurrentProcess(),p.trampoline,length+14);jump(p.replacement.data(),reinterpret_cast<A>(detour));
    *original=p.trampoline;patches.push_back(std::move(p));return true;
}
bool commitPatches(){
    // The three copied prologues are complete instructions without RIP-relative operands.
    std::vector<HANDLE> frozen;frozen.reserve(4096);bool okay=true;
    std::vector<DWORD> protection(patches.size());
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);if(snapshot==INVALID_HANDLE_VALUE)return false;
    THREADENTRY32 e{};e.dwSize=sizeof(e);
    if(Thread32First(snapshot,&e))do{
        if(e.th32OwnerProcessID!=GetCurrentProcessId()||e.th32ThreadID==GetCurrentThreadId())continue;
        HANDLE t=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT|THREAD_SET_CONTEXT,FALSE,e.th32ThreadID);
        if(!t){if(GetLastError()==ERROR_INVALID_PARAMETER)continue;okay=false;break;}
        if(SuspendThread(t)==DWORD(-1)){CloseHandle(t);okay=false;break;}frozen.push_back(t);
    }while(Thread32Next(snapshot,&e));
    CloseHandle(snapshot);
    if(okay)for(HANDLE t:frozen){
        CONTEXT c{};c.ContextFlags=CONTEXT_CONTROL;if(!GetThreadContext(t,&c)){okay=false;break;}
        for(const auto& p:patches)if(c.Rip>=p.site&&c.Rip<p.site+p.original.size()){
            c.Rip=reinterpret_cast<A>(p.trampoline)+c.Rip-p.site;if(!SetThreadContext(t,&c))okay=false;
        }
    }
    if(okay)for(const auto& p:patches){unsigned char actual[32]{};if(p.original.size()>sizeof(actual)||!read(p.site,actual,p.original.size())||memcmp(actual,p.original.data(),p.original.size()))okay=false;}
    // Make every target writable before any code is changed; roll back protections on failure.
    size_t prepared=0;
    if(okay)for(size_t i=0;i<patches.size();++i){if(!VirtualProtect(reinterpret_cast<void*>(patches[i].site),patches[i].original.size(),PAGE_EXECUTE_READWRITE,&protection[i])){okay=false;break;}++prepared;}
    if(okay)for(const auto& p:patches){memcpy(reinterpret_cast<void*>(p.site),p.replacement.data(),p.replacement.size());FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(p.site),p.replacement.size());}
    for(size_t i=0;i<prepared;++i){DWORD old=0;VirtualProtect(reinterpret_cast<void*>(patches[i].site),patches[i].original.size(),protection[i],&old);}
    for(HANDLE t:frozen){ResumeThread(t);CloseHandle(t);}return okay;
}

void log(const char* event,const std::string& message) {
    std::lock_guard<std::mutex> guard(logMutex);
    std::ofstream f(folder/L"probe.log",std::ios::app);
    f<<GetTickCount64()<<" pid="<<GetCurrentProcessId()<<" "<<event<<" "<<message<<"\n";
}
std::string hex(A a){char s[32]{};sprintf_s(s,"0x%llx",static_cast<unsigned long long>(a));return s;}
bool read(A p,void* out,size_t n){SIZE_T done=0;return p>=0x10000&&ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(p),out,n,&done)&&done==n;}
template<class T>T at(A p){T v{};read(p,&v,sizeof(v));return v;}
bool entityValid(A p){return at<A>(p)==image+0xb485368&&at<A>(at<A>(p+0x38)+8)==image+0xf50ab30;}
bool bytes(A rva,const std::vector<unsigned char>& expected){
    std::vector<unsigned char> got(expected.size());if(!read(image+rva,got.data(),got.size()))return false;
    if(got==expected)return true;
    // Continue this same game's verified v2 probe through its existing absolute entry gates.
    unsigned char entry[14]{};if(!read(image+rva,entry,sizeof(entry))||memcmp(entry,"\xff\x25\0\0\0\0",6))return false;
    A target=0;memcpy(&target,entry+6,8);MEMORY_BASIC_INFORMATION m{};
    if(!VirtualQuery(reinterpret_cast<void*>(target),&m,sizeof(m)))return false;
    wchar_t path[32768]{};if(!GetModuleFileNameW(static_cast<HMODULE>(m.AllocationBase),path,32768))return false;
    return std::filesystem::path(path).filename()==L"DynamicAttendanceProbe-v2.dll";
}

void* __fastcall property(void* reader,void* type) {
    if(applying){
        A caller=reinterpret_cast<A>(_ReturnAddress())-image;
        if(caller==0x3635248)return &threshold;
        if(caller==0x363521f)return &hide;
    }
    return originalRead(reader,type);
}
void __fastcall visibility(A crowd,unsigned int part,unsigned int agent,bool hidden) {
    if(applying){
        ++calls;
        if(observed.insert(crowd).second){
            A renderer=at<A>(crowd+0x9f0),vt=at<A>(renderer);
            log("crowd",hex(crowd)+" renderer="+hex(renderer)+" vtable="+hex(vt-image)+" setter="+hex(at<A>(vt+0xe0)-image));
        }
        // Stable selection across all body/prop parts of the same fan.
        unsigned int h=agent*747796405u+2891336453u;h=((h>>((h>>28)+4))^h)*277803737u;h=(h>>22)^h;
        if(thinning&&!hidden&&(h&0xffffu)<removalLimit){hidden=true;++changed;}
    }
    originalSet(crowd,part,agent,hidden);
}
void apply(bool thin) {
    unsigned total=0,culled=0; unsigned valid=0;
    for(A entity:entities){
        if(!entityValid(entity)){log("skipped","Crowd entity lifetime changed; no native call");continue;}
        calls=changed=0;applying=true;thinning=thin;
        reinterpret_cast<void(__fastcall*)(A)>(image+0x3635190)(entity);
        applying=false;thinning=false;total+=calls;culled+=changed;++valid;
    }
    log(thin?"probe-applied":"restored","entities="+std::to_string(valid)+" setter_calls="+std::to_string(total)+" hidden_overrides="+std::to_string(culled));
}
bool guardedApply(bool thin) {
    __try {apply(thin);return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){applying=false;thinning=false;return false;}
}
void findEntities() {
    const A needle=image+0xb485368;std::vector<unsigned char> buffer(0x100007);
    for(A addr=0x10000;addr<0x100000000;){
        MEMORY_BASIC_INFORMATION m{};if(!VirtualQuery(reinterpret_cast<void*>(addr),&m,sizeof(m)))break;
        A start=reinterpret_cast<A>(m.BaseAddress),end=start+m.RegionSize;if(end<=addr)break;addr=end;
        if(m.State!=MEM_COMMIT||m.Type!=MEM_PRIVATE||(m.Protect&(PAGE_GUARD|PAGE_NOACCESS))||!(m.Protect&0xee))continue;
        end=(std::min)(end,A(0x100000000));
        for(A p=start;p<end;p+=0x100000){
            size_t n=(std::min)(size_t(0x100007),size_t(end-p));if(!read(p,buffer.data(),n))continue;
            for(size_t i=0;i+8<=n;i+=8){A value=0;memcpy(&value,buffer.data()+i,8);if(value==needle&&entityValid(p+i))entities.push_back(p+i);}
        }
    }
    std::sort(entities.begin(),entities.end());entities.erase(std::unique(entities.begin(),entities.end()),entities.end());
    for(A e:entities)log("entity",hex(e)+" asset="+hex(at<A>(e+0x38)));
}
bool install() {
    if(at<unsigned int>(image+0x3c)>0x1000)return false;
    if(!bytes(0x3601860,{0x4c,0x8b,0xdc,0x55,0x53,0x56,0x41,0x55,0x49,0x8d,0x6b,0xa1,0x48,0x81,0xec,0xd8,0,0,0})||
       !bytes(0x5b2d1c0,{0x48,0x89,0x5c,0x24,0x10,0x57,0x48,0x83,0xec,0x20})||
       !bytes(0x64b3930,{0x48,0x89,0x5c,0x24,0x08,0x48,0x89,0x6c,0x24,0x10})||
       !bytes(0x3635190,{0x40,0x55,0x56,0x41,0x57}))return false;
    signatures=true;
    findEntities();if(entities.empty()||entities.size()>8)return false;
    if(!makePatch(0x3601860,reinterpret_cast<void*>(CrowdTickGate),&TickOriginal,19)||
       !makePatch(0x5b2d1c0,reinterpret_cast<void*>(property),reinterpret_cast<void**>(&originalRead),16)||
       !makePatch(0x64b3930,reinterpret_cast<void*>(visibility),reinterpret_cast<void**>(&originalSet),15))return false;
    if(!commitPatches())return false;
    enabled=true;return true;
}
DWORD WINAPI worker(void*) {
    wchar_t path[32768]{};GetModuleFileNameW(self,path,32768);folder=std::filesystem::path(path).parent_path();image=reinterpret_cast<A>(GetModuleHandleW(nullptr));
    log("starting","Controlled live crowd probe v3");
    if(!install()){log("inactive",signatures?"No compatible crowd entities or hook installation failed":"Required native bytes differ; no hooks installed");return 1;}
    log("ready","Commands: observe, probe (50%/12 seconds), empty (100% until restore), restore, stop. Changes apply on a game tick.");
    unsigned long long last=0;
    while(enabled){
        std::string command;{std::ifstream f(folder/L"control.txt");std::getline(f,command);}
        if(!command.empty()){
            {std::ofstream clear(folder/L"control.txt",std::ios::trunc);}
            if(command=="observe")request=1;
            else if(command=="probe")request=2;
            else if(command=="empty")request=4;
            else if(command=="restore"||command=="stop")request=3;
            log("requested",command);
        }
        auto now=GetTickCount64();if(now-last>5000){log("heartbeat","ticks="+std::to_string(ticks.load())+" pending="+std::to_string(request.load()));last=now;}
        Sleep(100);
    }
    return 0;
}
}
extern "C" void CrowdTick() {
    ++ticks;
    if(!enabled)return;
    int command=request.exchange(0);
    if(restoreAt&&GetTickCount64()>=restoreAt){command=3;restoreAt=0;}
    if(command){
        bool thin=command==2||command==4;
        removalLimit=command==4?65536u:32768u;
        if(!guardedApply(thin)){enabled=false;log("fault","Native apply rejected; restart required before further testing");return;}
        if(command==2)restoreAt=GetTickCount64()+12000;
        if(command==4)restoreAt=0;
        if(command==3)restoreAt=0;
    }
}
BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,LPVOID) {
    if(reason==DLL_PROCESS_ATTACH){self=instance;DisableThreadLibraryCalls(instance);HANDLE t=CreateThread(nullptr,0,worker,nullptr,0,nullptr);if(t)CloseHandle(t);}
    return TRUE;
}
