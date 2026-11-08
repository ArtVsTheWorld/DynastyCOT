#define GAMEDAY_LOADER_TESTING
#include "../Source/LoadRuntime.cpp"
#include <cassert>
#include <sstream>
int main(){
    using attendance::loader::diagnostic;
    auto text=diagnostic(123,"CreateRemoteThread for runtime operation",5,L"C:\\Mods\\DGA\\runtime.dll");
    assert(text.find("PID 123")!=std::string::npos&&text.find("Windows error 5 (0x00000005)")!=std::string::npos&&text.find("CreateRemoteThread")!=std::string::npos&&text.find("runtime.dll")!=std::string::npos);
    assert(text.find('\n')==std::string::npos&&text.find('\r')==std::string::npos);
    assert(attendance::loader::utf8(L"\u00c9\u4e2d")=="\xc3\x89\xe4\xb8\xad");
    wchar_t own[32768]{};assert(GetModuleFileNameW(nullptr,own,32768));DWORD failure=999;
    auto base=moduleBase(GetCurrentProcessId(),std::filesystem::path(own).filename().wstring(),&failure);assert(base&&failure==0);
    assert(!moduleBase(GetCurrentProcessId(),L"Dga-deliberately-missing-module.dll",&failure)&&failure==0);
    assert(!moduleBase(MAXDWORD,L"Dga-deliberately-missing-module.dll",&failure)&&failure!=0);
    // Exercise the real loader's early diagnostic without requesting a game
    // handle, creating a thread or loading any DLL.
    wchar_t command[]=L"loader",pid[]=L"123",missing[]=L"C:\\Dga-deliberately-missing-folder\\runtime.dll";wchar_t* args[]{command,pid,missing};
    std::ostringstream output;auto original=std::cerr.rdbuf(output.rdbuf());int result=runtimeRequest(3,args);std::cerr.rdbuf(original);
    assert(result==2&&output.str().find("read runtime DLL file attributes")!=std::string::npos&&output.str().find("Windows error")!=std::string::npos&&output.str().find("runtime.dll")!=std::string::npos);
    std::cout<<"PASS: native loader operation/PID/path/Win32 diagnostics, UTF-8 paths, read-only own-process module enumeration, missing-versus-denied distinction and early missing-file failure. No game opened.\n";
}
