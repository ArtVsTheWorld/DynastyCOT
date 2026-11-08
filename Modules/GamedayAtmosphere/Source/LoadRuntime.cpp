#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <filesystem>
#include <iostream>
#include <string>
#include "LoaderDiagnostics.h"

uintptr_t moduleBase(DWORD pid,const std::wstring& filename,DWORD* failure=nullptr) {
    if(failure)*failure=0;
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);
    if(snapshot==INVALID_HANDLE_VALUE){if(failure)*failure=GetLastError();return 0;}
    MODULEENTRY32W entry{};entry.dwSize=sizeof(entry);uintptr_t result=0;
    BOOL found=Module32FirstW(snapshot,&entry);DWORD error=found?0:GetLastError();
    while(found){
        if(_wcsicmp(entry.szModule,filename.c_str())==0){result=reinterpret_cast<uintptr_t>(entry.modBaseAddr);break;}
        found=Module32NextW(snapshot,&entry);if(!found)error=GetLastError();
    }
    if(failure&&error!=ERROR_NO_MORE_FILES)*failure=error;
    CloseHandle(snapshot);return result;
}
int runtimeRequest(int argc,wchar_t** argv) {
    if(argc!=3&&argc!=4){std::wcerr<<L"Usage: LoadDynamicGamedayAtmosphere.exe PID runtime.dll [--stop]\n";return 2;}
    DWORD pid=static_cast<DWORD>(std::stoul(argv[1]));
    auto dll=std::filesystem::absolute(argv[2]);
    auto fail=[&](int result,const char* operation,DWORD error,const std::wstring& detail=std::wstring()){
        std::cerr<<attendance::loader::diagnostic(pid,operation,error,detail)<<"\n";return result;
    };
    if(!pid)return fail(2,"validate game process ID",ERROR_INVALID_PARAMETER);
    DWORD attributes=GetFileAttributesW(dll.c_str());
    if(attributes==INVALID_FILE_ATTRIBUTES)return fail(2,"read runtime DLL file attributes",GetLastError(),dll.wstring());
    if(attributes&FILE_ATTRIBUTE_DIRECTORY)return fail(2,"validate runtime DLL file",ERROR_DIRECTORY,dll.wstring());
    HANDLE game=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION|PROCESS_VM_WRITE|PROCESS_VM_READ,FALSE,pid);
    if(!game){
        DWORD error=GetLastError();fail(3,"OpenProcess for runtime loading (query/thread creation/memory allocation/read/write)",error);
        HANDLE query=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);DWORD probe=query?0:GetLastError();if(query)CloseHandle(query);
        fail(3,"read-only process query cross-check",probe,query?L"Query access works; combined runtime-load access was denied.":L"Even limited query access is unavailable.");return 3;
    }
    wchar_t gameName[32768]{};DWORD length=32768;
    if(!QueryFullProcessImageNameW(game,0,gameName,&length)){DWORD error=GetLastError();CloseHandle(game);return fail(3,"QueryFullProcessImageNameW",error);}
    if(_wcsicmp(std::filesystem::path(gameName).filename().c_str(),L"CollegeFB27.exe")!=0){CloseHandle(game);return fail(3,"validate target process",ERROR_INVALID_PARAMETER,L"Target is not CollegeFB27.exe.");}
    void* remote=nullptr;LPTHREAD_START_ROUTINE function=nullptr;
    if(argc==4) {
        if(std::wstring(argv[3])!=L"--stop"){CloseHandle(game);return fail(2,"validate stop argument",ERROR_INVALID_PARAMETER);}
        DWORD lookup=0;uintptr_t base=moduleBase(pid,dll.filename().wstring(),&lookup);
        if(lookup){CloseHandle(game);return fail(4,"enumerate game modules for runtime stop",lookup);}
        if(!base){CloseHandle(game);return fail(4,"locate runtime to stop",ERROR_MOD_NOT_FOUND,dll.filename().wstring());}
        HMODULE local=LoadLibraryExW(dll.c_str(),nullptr,DONT_RESOLVE_DLL_REFERENCES);
        if(!local){DWORD error=GetLastError();CloseHandle(game);return fail(4,"map local runtime for stop export",error,dll.wstring());}
        FARPROC stop=GetProcAddress(local,"DynamicGamedayAtmosphereStop");
        if(!stop){DWORD error=GetLastError();FreeLibrary(local);CloseHandle(game);return fail(4,"resolve runtime stop export",error);}
        function=reinterpret_cast<LPTHREAD_START_ROUTINE>(base+reinterpret_cast<uintptr_t>(stop)-reinterpret_cast<uintptr_t>(local));
        FreeLibrary(local);
    }else {
        DWORD lookup=0;auto existing=moduleBase(pid,dll.filename().wstring(),&lookup);
        if(lookup){CloseHandle(game);return fail(4,"enumerate game modules for loaded runtime",lookup);}
        if(existing){std::cout<<"Runtime already loaded\n";CloseHandle(game);return 0;}
        HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);
        if(snapshot==INVALID_HANDLE_VALUE){DWORD error=GetLastError();CloseHandle(game);return fail(4,"snapshot game modules for conflicting runtime",error);}
        MODULEENTRY32W entry{};entry.dwSize=sizeof(entry);bool conflict=false;
        BOOL found=Module32FirstW(snapshot,&entry);DWORD enumerationError=found?0:GetLastError();
        while(found){
            std::wstring name=entry.szModule;
            if(name.find(L"DynamicAttendanceNative")==0||name.find(L"DynamicAttendanceProbe")==0||name.find(L"DynamicGamedayAtmosphereNative")==0)conflict=true;
            found=Module32NextW(snapshot,&entry);if(!found)enumerationError=GetLastError();
        }CloseHandle(snapshot);
        if(enumerationError&&enumerationError!=ERROR_NO_MORE_FILES){CloseHandle(game);return fail(4,"enumerate game modules for conflicting runtime",enumerationError);}
        if(conflict){std::cerr<<"An attendance/atmosphere runtime is already loaded. Restart both MMC and the game before updating.\n";CloseHandle(game);return 9;}
        FARPROC load=GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"LoadLibraryW");
        if(!load){DWORD error=GetLastError();CloseHandle(game);return fail(4,"resolve local LoadLibraryW",error);}
        MEMORY_BASIC_INFORMATION mbi{};
        if(!VirtualQuery(reinterpret_cast<void*>(load),&mbi,sizeof(mbi))){DWORD error=GetLastError();CloseHandle(game);return fail(4,"locate LoadLibraryW owner",error);}
        wchar_t owner[32768]{};DWORD ownerLength=GetModuleFileNameW(static_cast<HMODULE>(mbi.AllocationBase),owner,32768);
        if(!ownerLength||ownerLength>=32768){DWORD error=ownerLength?ERROR_INSUFFICIENT_BUFFER:GetLastError();CloseHandle(game);return fail(4,"read LoadLibraryW owner filename",error);}
        auto ownerFile=std::filesystem::path(owner).filename().wstring();uintptr_t remoteOwner=moduleBase(pid,ownerFile,&lookup);
        if(!remoteOwner){CloseHandle(game);return fail(4,"locate loader owner in game",lookup?lookup:ERROR_MOD_NOT_FOUND,ownerFile);}
        function=reinterpret_cast<LPTHREAD_START_ROUTINE>(remoteOwner+reinterpret_cast<uintptr_t>(load)-reinterpret_cast<uintptr_t>(mbi.AllocationBase));
        size_t bytes=(dll.wstring().size()+1)*sizeof(wchar_t);
        remote=VirtualAllocEx(game,nullptr,bytes,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
        if(!remote){DWORD error=GetLastError();CloseHandle(game);return fail(5,"VirtualAllocEx for runtime path",error);}
        SIZE_T written=0;
        BOOL copied=WriteProcessMemory(game,remote,dll.c_str(),bytes,&written);DWORD error=copied?ERROR_PARTIAL_COPY:GetLastError();
        if(!copied||written!=bytes){VirtualFreeEx(game,remote,0,MEM_RELEASE);CloseHandle(game);return fail(5,"WriteProcessMemory for runtime path",error);}
    }
    HANDLE thread=CreateRemoteThread(game,nullptr,0,function,remote,0,nullptr);
    if(!thread){DWORD error=GetLastError();if(remote)VirtualFreeEx(game,remote,0,MEM_RELEASE);CloseHandle(game);return fail(6,"CreateRemoteThread for runtime operation",error);}
    DWORD wait=WaitForSingleObject(thread,15000);
    if(wait!=WAIT_OBJECT_0){DWORD error=wait==WAIT_FAILED?GetLastError():ERROR_TIMEOUT;CloseHandle(thread);CloseHandle(game);return fail(7,"wait for game runtime operation",error,L"Operation may still be pending; allocated path retained. Do not retry in this game process.");}
    DWORD result=0;BOOL completed=GetExitCodeThread(thread,&result);DWORD resultError=completed?0:GetLastError();CloseHandle(thread);
    if(remote&&!VirtualFreeEx(game,remote,0,MEM_RELEASE))fail(0,"release completed runtime-path allocation",GetLastError(),L"Cleanup warning; not a new load attempt.");CloseHandle(game);
    if(!completed)return fail(8,"read runtime operation result",resultError);
    if(argc==4){std::cout<<"Stop result: "<<result<<"\n";return result==0?0:8;}
    DWORD lookup=0;bool verified=moduleBase(pid,dll.filename().wstring(),&lookup)!=0;
    if(lookup)return fail(8,"enumerate game modules to verify runtime load",lookup);
    if(!verified)return fail(8,"verify runtime DLL loaded",0,L"Game load operation finished but DLL is absent. Thread result: "+std::to_wstring(result)+L". Check security history, DLL dependencies or native initialization failure.");
    std::cout<<"Loaded; see the plugin's Logs folder for activation status\n";return 0;
}
#ifndef GAMEDAY_LOADER_TESTING
int wmain(int argc,wchar_t** argv){try{return runtimeRequest(argc,argv);}catch(const std::exception& error){std::cerr<<"Loader failed while preparing request: "<<error.what()<<"\n";return 2;}}
#endif
