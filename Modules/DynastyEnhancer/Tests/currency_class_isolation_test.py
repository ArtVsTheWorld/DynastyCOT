"""Real native UI Runtime exercised with linked/unlinked synthetic APT classes."""
from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
s=(ROOT/'Modules/DynastyEnhancer/Source/RecruitingFog.cpp').read_text()
assert '&nilToggleUi,&currencyRecruitingOfferUi,&currencyRecruitingHoursUi' in s
assert 'currencyRecruitingUi(currencyclassui::recruitingPrograms(0))' in s
assert 'runtime==&currencyRecruitingUi||runtime==&currencyRecruitingOfferUi||runtime==&currencyRecruitingHoursUi' in s
fixture=r"""
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <cstring>
#include "RecruitNotesUiRuntime.h"
#include "CurrencyClassBanks.h"
using namespace recruitingfog;using namespace notesui;
int checks=0;void check(bool ok){++checks;if(!ok)throw std::runtime_error("Currency isolation check "+std::to_string(checks));}
struct Fixture {
 std::vector<Program> programs=currencytopbarui::recruitingPrograms();
 std::vector<unsigned char> image=std::vector<unsigned char>(0x40000);std::array<uintptr_t,100> pool{};
 Fixture(){memcpy(image.data(),"Apt Data:1:",11);for(auto& p:programs){
 auto* h=image.data()+p.offset;memcpy(h+8,&p.argc,4);unsigned short r=static_cast<unsigned short>(p.registers),f=static_cast<unsigned short>(p.flags);memcpy(h+12,&r,2);memcpy(h+14,&f,2);memcpy(h+24,&p.size,4);
 std::vector<unsigned char> body(p.size);for(auto ptr:p.sourcePointers)memcpy(body.data()+ptr.offset,&ptr.value,8);
 for(auto n:p.nestedDefinitions){memcpy(body.data()+n.offset,&n.name,8);memcpy(body.data()+n.offset+16,&n.params,8);unsigned c=n.emptyPool?0:0x98765432u;uintptr_t v=n.emptyPool?0:0x12345678u;memcpy(body.data()+n.offset+32,&c,4);memcpy(body.data()+n.offset+40,&v,8);}
 p.hash=bodyHash(body);memcpy(h+48,body.data(),body.size());unlink(p.offset);
 }}
 void unlink(uintptr_t off){unsigned n=0x98765432u;uintptr_t v=0x12345678u;memcpy(image.data()+off+32,&n,4);memcpy(image.data()+off+40,&v,8);}
 void link(uintptr_t off){unsigned n=100;auto v=reinterpret_cast<uintptr_t>(pool.data());memcpy(image.data()+off+32,&n,4);memcpy(image.data()+off+40,&v,8);}
 bool read(uintptr_t a,void* out,size_t bytes){auto b=reinterpret_cast<uintptr_t>(image.data()),p=reinterpret_cast<uintptr_t>(pool.data());if((a>=b&&a+bytes>=a&&a+bytes<=b+image.size())||(a>=p&&a+bytes>=a&&a+bytes<=p+sizeof(pool))){memcpy(out,reinterpret_cast<void*>(a),bytes);return true;}return false;}
};
int main(){try{
 const auto original=currencytopbarui::recruitingPrograms();for(size_t i=0;i<3;++i){auto split=currencyclassui::recruitingPrograms(i);check(split.size()==1&&split[0].hash==original[i].hash&&split[0].code==original[i].code&&split[0].copyBytes==original[i].copyBytes&&split[0].offset==original[i].offset);}
 bool outOfRange=false;try{currencyclassui::recruitingPrograms(3);}catch(const std::exception&){outOfRange=true;}check(outOfRange);
 for(size_t selected=0;selected<3;++selected){
 Fixture f;f.link(f.programs[selected].offset);auto read=[&](uintptr_t a,void* o,size_t n){return f.read(a,o,n);};
 alignas(uintptr_t)std::array<unsigned char,0x60> object{};uintptr_t native=reinterpret_cast<uintptr_t>(f.image.data())+f.programs[selected].offset;memcpy(object.data()+0x58,&native,8);
 Runtime oldAggregate(f.programs);oldAggregate.prepare();bool rejected=false;try{oldAggregate.beforeInvoke(object.data(),read,false,true,1,false,true,true,true);}catch(const std::exception&){rejected=true;}check(rejected&&oldAggregate.ownerCount()==0);
 Runtime isolated({f.programs[selected]});isolated.prepare();check(isolated.beforeInvoke(object.data(),read,false,true,1,false,true,true,true)==0);uintptr_t on=0;memcpy(&on,object.data()+0x58,8);check(on!=native&&isolated.ownerCount()==1);
 auto bytes=isolated.retainedByteCount();check(isolated.beforeInvoke(object.data(),read,false,true,1,false,true,true,true)==-1&&isolated.retainedByteCount()==bytes);
 check(isolated.beforeInvoke(object.data(),read,false,true,1,false,true,true,false)==0);uintptr_t off=0;memcpy(&off,object.data()+0x58,8);check(off!=native&&off!=on);bytes=isolated.retainedByteCount();check(isolated.beforeInvoke(object.data(),read,false,true,1,false,true,true,false)==-1&&isolated.retainedByteCount()==bytes);
 check(isolated.beforeInvoke(object.data(),read,false,false)==0);uintptr_t restored=0;memcpy(&restored,object.data()+0x58,8);check(restored==native);
 for(size_t other=0;other<3;++other)if(other!=selected){Runtime unrelated({f.programs[other]});unrelated.prepare();check(unrelated.beforeInvoke(object.data(),read)==-1&&unrelated.ownerCount()==0);check(field<unsigned>(f.image.data()+f.programs[other].offset,32)==0x98765432u);}
 f.image[f.programs[selected].offset+48+f.programs[selected].size-1]^=1;Runtime tampered({f.programs[selected]});tampered.prepare();rejected=false;try{tampered.beforeInvoke(object.data(),read);}catch(const std::exception&){rejected=true;}check(rejected&&tampered.ownerCount()==0);
 }
 // Failure of one linked class cannot poison a separate linked class bank.
 Fixture f;f.link(f.programs[0].offset);f.link(f.programs[2].offset);f.image[f.programs[0].offset+48]^=1;
 auto read=[&](uintptr_t a,void* o,size_t n){return f.read(a,o,n);};alignas(uintptr_t)std::array<unsigned char,0x60> object{};
 Runtime broken({f.programs[0]}),healthy({f.programs[2]});broken.prepare();healthy.prepare();uintptr_t key=reinterpret_cast<uintptr_t>(f.image.data())+f.programs[0].offset;memcpy(object.data()+0x58,&key,8);bool rejected=false;try{broken.beforeInvoke(object.data(),read);}catch(const std::exception&){rejected=true;}check(rejected);
 key=reinterpret_cast<uintptr_t>(f.image.data())+f.programs[2].offset;memcpy(object.data()+0x58,&key,8);check(healthy.beforeInvoke(object.data(),read)==0&&healthy.ownerCount()==1);
 std::cout<<"PASS: "<<checks<<" real Runtime recruiting currency isolation checks; linked class works with sentinel siblings; Off/reuse/restore/tamper/class failures remain guarded.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
"""
vs=subprocess.check_output([r'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe','-latest','-products','*','-requires','Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-property','installationPath'],text=True).strip()
tool=max((Path(vs)/'VC/Tools/MSVC').iterdir());kits=Path(r'C:\Program Files (x86)\Windows Kits\10');sdk=max((kits/'Include').iterdir())
with tempfile.TemporaryDirectory(prefix='currency-class-',dir=ROOT/'Build') as temp:
 d=Path(temp);cpp=d/'checks.cpp';cpp.write_text(fixture);exe=d/'checks.exe'
 flags=['/nologo','/O2','/MT','/W4','/WX','/EHsc','/std:c++17','/utf-8','/I'+str(tool/'include'),'/I'+str(ROOT/'Modules/DynastyEnhancer/Source')]+['/I'+str(sdk/p) for p in ['ucrt','shared','um','winrt']]
 libs=['/LIBPATH:'+str(tool/'lib/x64'),'/LIBPATH:'+str(kits/'Lib'/sdk.name/'ucrt/x64'),'/LIBPATH:'+str(kits/'Lib'/sdk.name/'um/x64'),'/INCREMENTAL:NO']
 result=subprocess.run([str(tool/'bin/Hostx64/x64/cl.exe'),*flags,str(cpp),'/Fe:'+str(exe),'/link',*libs],cwd=d,capture_output=True,text=True)
 if result.returncode:raise RuntimeError(result.stdout+result.stderr)
 subprocess.run([str(exe)],check=True)
