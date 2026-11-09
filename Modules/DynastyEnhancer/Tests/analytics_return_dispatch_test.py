"""Compile the production return-dispatch branch with real native frame filters.
No game attachment, hooks, assets or native writes occur in this fixture.
"""
from pathlib import Path
import subprocess,tempfile,os
ROOT=Path(__file__).resolve().parents[3]
source=(ROOT/'Modules/DynastyEnhancer/Source/RecruitingFog.cpp').read_text()
start=source.index('    if(nativeanalytics::cardReturnPending)for(auto* bank:')
end=source.index('    static const notesui::FrameDispatch',start)
branch=source[start:end]
assert branch.index('acceptsFrame(liveHeader)')<branch.index('nativeanalytics::available()')<branch.index('beforeInvoke(')
assert 'bank->ownsInvocation(object,0,read)' in branch
fixture=r"""
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <cstring>
#include <stdexcept>
#include "RecruitNotesUiRuntime.h"
#include "AnalyticsActionsUiPrograms.h"
using namespace recruitingfog;
int checks=0,probes=0,resumes=0,admissions=0,failures=0;bool eligible=true;
void check(bool x){++checks;if(!x)throw std::runtime_error("Return dispatch fixture failed at check "+std::to_string(checks));}
namespace nativeanalytics {bool cardReturnPending=true;bool available(){++probes;return eligible;}void resumeCard(){++resumes;}}
namespace enhancements {struct Options{unsigned scale=1,teamScale=1;bool colors=false,roster=true,team=true,dollars=false;};Options options(){return {};}}
bool read(uintptr_t,void*,size_t){return false;}
void note(std::initializer_list<std::pair<std::string,std::string>>){++failures;}
struct Bank {
 notesui::Runtime filter;int index=0;bool owned=false,reject=false;
 explicit Bank(std::vector<notesui::Program> p):filter(std::move(p)){filter.prepare();}
 bool acceptsFrame(const unsigned char* h){return filter.acceptsFrame(h);}
 template<class R>int beforeInvoke(void*,R,bool,bool,unsigned,bool,bool,bool,bool,unsigned){++admissions;if(reject)throw std::runtime_error("Exact body/owner refused");return index;}
 template<class R>bool ownsInvocation(void*,unsigned,R){return owned;}
};
Bank analyticsMainUi(analyticsactions::hubMainPrograms()),analyticsCurrentUi(analyticsactions::currentMainPrograms());
void dispatch(const unsigned char* liveHeader){void* object=nullptr;
__BRANCH__
}
std::array<unsigned char,48> header(const notesui::Program& p){std::array<unsigned char,48> h{};memcpy(h.data()+8,&p.argc,4);unsigned short r=static_cast<unsigned short>(p.registers),f=static_cast<unsigned short>(p.flags);memcpy(h.data()+12,&r,2);memcpy(h.data()+14,&f,2);memcpy(h.data()+24,&p.size,4);return h;}
void reset(){probes=resumes=admissions=failures=0;eligible=true;nativeanalytics::cardReturnPending=true;analyticsMainUi.index=analyticsCurrentUi.index=0;analyticsMainUi.owned=analyticsCurrentUi.owned=false;analyticsMainUi.reject=analyticsCurrentUi.reject=false;}
int main(){try{
 auto a=analyticsactions::hubMainPrograms(),b=analyticsactions::currentMainPrograms();
 reset();auto unrelated=header(a[0]);unsigned size=37;memcpy(unrelated.data()+24,&size,4);for(int n=0;n<10000;++n)dispatch(unrelated.data());check(probes==0&&admissions==0&&resumes==0);
 for(const auto& p:{a[0],b[0]}){
 auto h=header(p);reset();eligible=false;dispatch(h.data());check(probes==1&&admissions==0&&resumes==0);
 reset();nativeanalytics::cardReturnPending=false;dispatch(h.data());check(probes==0&&admissions==0&&resumes==0);
 reset();dispatch(h.data());check(probes==1&&admissions==1&&resumes==1);
 reset();analyticsMainUi.index=analyticsCurrentUi.index=1;dispatch(h.data());check(resumes==0&&admissions>0&&probes>0);
 reset();analyticsMainUi.index=analyticsCurrentUi.index=-1;analyticsMainUi.owned=analyticsCurrentUi.owned=true;dispatch(h.data());check(resumes==1&&probes==1);
 reset();analyticsMainUi.reject=analyticsCurrentUi.reject=true;dispatch(h.data());check(resumes==0&&failures>0&&admissions>0);
 for(auto offset:{8,12,14,24}){reset();auto mutated=h;mutated[offset]^=0x7f;dispatch(mutated.data());check(probes==0&&admissions==0&&resumes==0);}
 }
 std::cout<<"PASS: "<<checks<<" production return-dispatch checks; real vanilla/AIO native frame filters; 10000 unrelated card scripts skip identity walks; stale scope/foreign body guarded.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
""".replace('__BRANCH__',branch)
vs=subprocess.check_output([r'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe','-latest','-products','*','-requires','Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-property','installationPath'],text=True).strip()
tool=max((Path(vs)/'VC/Tools/MSVC').iterdir());kits=Path(r'C:\Program Files (x86)\Windows Kits\10');sdk=max((kits/'Include').iterdir())
with tempfile.TemporaryDirectory(prefix='analytics-return-',dir=ROOT/'Build') as temp:
 d=Path(temp);cpp=d/'checks.cpp';cpp.write_text(fixture);exe=d/'checks.exe'
 flags=['/nologo','/O2','/MT','/W4','/WX','/EHsc','/std:c++17','/utf-8','/I'+str(tool/'include'),'/I'+str(ROOT/'Modules/DynastyEnhancer/Source')]+['/I'+str(sdk/p) for p in ['ucrt','shared','um','winrt']]
 libs=['/LIBPATH:'+str(tool/'lib/x64'),'/LIBPATH:'+str(kits/'Lib'/sdk.name/'ucrt/x64'),'/LIBPATH:'+str(kits/'Lib'/sdk.name/'um/x64'),'/INCREMENTAL:NO']
 result=subprocess.run([str(tool/'bin/Hostx64/x64/cl.exe'),*flags,str(cpp),'/Fe:'+str(exe),'/link',*libs],cwd=d,capture_output=True,text=True)
 if result.returncode:raise RuntimeError(result.stdout+result.stderr)
 subprocess.run([str(exe)],check=True)
