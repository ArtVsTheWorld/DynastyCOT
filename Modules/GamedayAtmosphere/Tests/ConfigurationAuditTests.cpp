#include "../Source/NativeRuntime.cpp"
#include <cassert>
#include <iostream>
#include "ConfigurationOptions.inc"
int main(){
    auto root=std::filesystem::temp_directory_path()/(L"DGA-config-audit-"+std::to_wstring(GetCurrentProcessId()));
    pluginsFolder=root/L"Plugins";dataFolder=pluginsFolder/L"DynamicGamedayAtmosphere";logsFolder=dataFolder/L"Logs";attendance::ensureFolder(logsFolder);
    auto path=dataFolder/L"DynamicGamedayAtmosphere.ini";auto stamp=std::filesystem::file_time_type::clock::now();int serial=0;
    auto write=[&](const NumericOption& option,const std::wstring& value){std::wofstream f(path,std::ios::trunc);f<<L"["<<option.section<<L"]\n"<<option.key<<L"="<<value<<L"\n";f.close();std::filesystem::last_write_time(path,stamp+std::chrono::seconds(++serial));readSettings();};
    unsigned cases=0;
    for(const auto& option:options){
        for(double input:{option.factory,option.low,option.high,option.low-17,option.high+17}){
            write(option,std::to_wstring(input)); // Integer options use their documented whole-number spelling below.
            bool integer=std::floor(option.factory)==option.factory&&std::floor(option.low)==option.low&&std::floor(option.high)==option.high;
            if(integer)write(option,std::to_wstring(static_cast<long long>(input)));
            double expected=(std::clamp)(input,option.low,option.high)*option.scale;
            assert(std::fabs(option.get()-expected)<.002);++cases;
        }
        write(option,L"not-a-number");assert(std::fabs(option.get()-option.factory*option.scale)<.002);++cases;
        write(option,L"nan");assert(std::fabs(option.get()-option.factory*option.scale)<.002);++cases;
    }
    auto integer=attendance::config::integer(L"1garbage",5,0,10);assert(integer.value==5&&integer.status==attendance::config::Status::Invalid);
    assert(attendance::config::integer(L"99999999999999999999999999",5,0,10).value==5);
    assert(attendance::config::real(L"inf",4.5f,1,8).value==4.5f);
    assert(attendance::config::real(L"0 ; off",4.5f,0,8).value==0);
    assert(attendance::config::integer(L" 2 # comment",5,0,10).value==2);
    NumericOption profile{L"Dynamic Gameday Attendance",L"Profile",0,0,0,1,[]()->double{return static_cast<double>(std::atomic_load(&settings)->profile);}};
    for(const auto& name:{L"Auto",L"Default",L"Reduced",L"BadWeather",L"BadProfile"}){write(profile,name);int expected=!std::wcscmp(name,L"Default")?1:!std::wcscmp(name,L"Reduced")?2:!std::wcscmp(name,L"BadWeather")?3:0;assert(profile.get()==expected);}
    while(queuedLogs.size())drainLogs();
    std::ifstream log(logsFolder/L"DynamicGamedayAtmosphere.log");std::string text((std::istreambuf_iterator<char>(log)),{});assert(text.find("configuration")!=std::string::npos&&text.find("using")!=std::string::npos);
    std::cout<<"PASS: "<<cases<<" real native-reader configuration cases (every numeric public option), defaults/min/max/disabled/out-of-range/invalid, strict parsing, comments and meaningful configuration diagnostics.\n";
}
