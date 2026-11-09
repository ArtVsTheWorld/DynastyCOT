"""Compile the real presentation scope/options/store; use private self-process fixtures."""
from pathlib import Path
import subprocess, datetime
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'Build'/('PresentationScope-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S%f'))
OUT.mkdir()
src=OUT/'checks.cpp'
src.write_text(r'''
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cassert>
#include <chrono>
#include <iostream>
#include "EnhancementOptionsRuntime.h"
#include "DynastyPresentationScope.h"
int main(){
 using namespace enhancements;unsigned checks=0;
 auto region=VirtualAlloc(nullptr,0x11000000,MEM_RESERVE,PAGE_NOACCESS);assert(region);
 const auto image=reinterpret_cast<uintptr_t>(region);
 auto commit=[&](uintptr_t rva){assert(VirtualAlloc(reinterpret_cast<void*>((image+rva)&~uintptr_t(4095)),4096,MEM_COMMIT,PAGE_READWRITE));};
 for(auto rva:{0xd9eb808u,0xf3fddbcu,0xfa519b8u,0xf55c998u})commit(rva);
 auto put=[](uintptr_t p,auto value){std::memcpy(reinterpret_cast<void*>(p),&value,sizeof(value));};
 alignas(16) unsigned char service[0x800]{};auto career=reinterpret_cast<uintptr_t>(service);
 put(image+0xfa519b8,career);put(career,image+0xbaa8998);put(image+0xf55c998,uint32_t(1001));
 requireDynastyPresentation(image);currentOptionsLeague=1001;
 for(int mode:{-1,0,13,14,17,18})for(int session:{0,1,2,3,4})for(int active:{0,1}){
  put(image+0xd9eb808,mode);put(image+0xf3fddbc,session);service[0x7b1]=static_cast<unsigned char>(active);
  assert(nativePresentationScope()==presentationScope(mode,session,true,active!=0,1001,1001));++checks;
 }
 put(image+0xd9eb808,17);put(image+0xf3fddbc,3);service[0x7b1]=1;
 for(unsigned bits=0;bits<=4095;++bits)if(Options::validBits(bits)){
  currentOptions=bits;assert(options().packed()==(bits&1));assert(configuredOptions().packed()==bits&&currentOptions.load()==bits);++checks;
 }
 put(image+0xd9eb808,14);currentOptions=defaultOptions;
 assert(options().packed()==defaultOptions);++checks;
 currentOptionsLeague=1002;assert(!options().roster&&!options().team&&!options().colors&&!options().dollars);++checks;
 currentOptionsLeague=1001;assert(options().packed()==defaultOptions);++checks;
 put(career,uintptr_t(1));assert(!nativePresentationScope());++checks;put(career,image+0xbaa8998);
 put(image+0xfa519b8,uintptr_t(1));assert(!nativePresentationScope());++checks;put(image+0xfa519b8,career);
 // Saved settings remain writable during the initial creation settings page,
 // even when presentation is native. Switching profiles never overwrites A.
 auto folder=std::filesystem::temp_directory_path()/("cot-scope-"+std::to_string(GetCurrentProcessId()));
 OptionStore store;store.configure(folder);store.bind("1001");assert(store.change(3,3)&&store.change(4,1)&&store.change(6,1));store.flush();
 const auto savedA=configuredOptions().packed();put(image+0xd9eb808,-1);service[0x7b1]=0;
 assert(!options().roster&&!options().colors&&!options().dollars&&configuredOptions().packed()==savedA);++checks;
 store.bind("1002");assert(store.change(2,0));store.flush();store.bind("1001");assert(configuredOptions().packed()==savedA);++checks;
 std::ifstream f(folder/"DynastyEnhancementSettings/1001.json");nlohmann::json doc;f>>doc;
 assert(doc["playerGradeScale"]==3&&doc["ratingColors"]==true&&doc["dollarValues"]==true);++checks;
 put(image+0xd9eb808,14);service[0x7b1]=1;assert(options().packed()==savedA);++checks;
 auto start=std::chrono::steady_clock::now();volatile unsigned sink=0;for(int i=0;i<100000;++i)sink=options().packed();
 auto ns=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-start).count();
 std::cout<<"PASS: "<<checks<<" real native presentation admission, outside-mode passthrough, profile persistence and creation-setting checks; "<<double(ns)/100000<<" ns/scope lookup (self-process fixture, not menu latency). "<<sink<<"\n";
 presentationGuard=nullptr;assert(VirtualFree(region,0,MEM_RELEASE));
}
''')
# Use the same MSVC/SDK flags as the production native build, without running its engine.
script=(ROOT/'Source/build.ps1').read_text();prefix=script[:script.index('Push-Location $analyticsOut')]
prefix=prefix.replace('$PSScriptRoot',"'"+str(ROOT/'Source').replace('\\','/')+"'")
cmd=prefix+f'''
& $analyticsCl @analyticsCommon ('/I'+(Join-Path $analyticsRoot 'Source')) '{src}' ('/Fe:{OUT/'checks.exe'}') ('/Fo:{OUT/'checks.obj'}') /link @analyticsLib /INCREMENTAL:NO
if($LASTEXITCODE -ne 0){{throw 'Scope fixture compile failed'}}
& '{OUT/'checks.exe'}'
if($LASTEXITCODE -ne 0){{throw 'Scope fixture failed'}}
'''
ps=OUT/'test.ps1';ps.write_text(cmd)
subprocess.run(['pwsh','-NoProfile','-File',str(ps)],check=True)
fog=(ROOT/'Source/RecruitingFog.cpp').read_text()
assert fog.index('requireDynastyPresentation(base)')<fog.index('enhancementSettings.configure(folder)')
menu=fog[fog.index('void __fastcall enhancementListHook'):fog.index('void installEnhancementSettings')]
assert 'enhancements::configuredOptions()' in menu and 'enhancements::options()' not in menu
print('PASS: production guard installed before hooks; creation settings read the retained profile.')
