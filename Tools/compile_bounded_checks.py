"""Focused self-process native fixture checks; never attaches to a game."""
from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'Build/ChampionshipLaunch/Focused';OUT.mkdir(exist_ok=True)
vs=Path(subprocess.check_output([r'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe','-latest','-products','*','-requires','Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-property','installationPath'],text=True).strip())
tool=sorted((vs/'VC/Tools/MSVC').iterdir())[-1];kits=Path(r'C:\Program Files (x86)\Windows Kits\10');win=sorted(x for x in (kits/'Include').iterdir() if (x/'ucrt').is_dir() and (kits/'Lib'/x.name/'um/x64').is_dir())[-1]
includes=[tool/'include',*(win/x for x in ['ucrt','shared','um'])];libs=[tool/'lib/x64',kits/'Lib'/win.name/'ucrt/x64',kits/'Lib'/win.name/'um/x64']
cl=[tool/'bin/Hostx64/x64/cl.exe','/nologo','/O2','/MT','/W4','/WX','/EHsc','/std:c++17','/utf-8',*(f'/I{x}' for x in includes)]
import sys
for name in sys.argv[1:]:
 source=Path(name).resolve();exe=OUT/(source.stem+'.exe')
 command=[*cl,source,'/Fe:'+str(exe),'/link',*(f'/LIBPATH:{x}' for x in libs),'/INCREMENTAL:NO']
 result=subprocess.run([str(x) for x in command],cwd=OUT,capture_output=True,text=True);print(result.stdout+result.stderr,flush=True)
 if result.returncode:raise SystemExit(result.returncode)
 print(exe)
