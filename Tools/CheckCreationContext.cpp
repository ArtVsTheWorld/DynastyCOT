// Opt-in read-only diagnostic for the production creation-scope validator.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdlib>
#include <iostream>
#include "../Modules/DynastyEnhancer/Source/EnhancementCreationScope.h"
int main(int argc,char** argv){
 if(argc!=4){std::cerr<<"Usage: CheckCreationContext PID image-base provider-address\n";return 2;}
 const auto process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|PROCESS_VM_READ,FALSE,static_cast<DWORD>(std::strtoul(argv[1],nullptr,0)));
 if(!process)return 3;
 const auto read=[&](uintptr_t at,void* out,size_t size){SIZE_T n=0;return ReadProcessMemory(process,reinterpret_cast<const void*>(at),out,size,&n)&&n==size;};
 const auto scope=enhancements::creationScope(static_cast<uintptr_t>(std::strtoull(argv[2],nullptr,0)),static_cast<uintptr_t>(std::strtoull(argv[3],nullptr,0)),read);
 CloseHandle(process);
 if(!scope){std::cout<<"No currently validated initial offline creation page. No writes performed.\n";return 1;}
 std::cout<<"PASS: production initial offline creation scope; LeagueID "<<scope.league<<". Read-only; no hooks, writes or native calls.\n";return 0;
}
