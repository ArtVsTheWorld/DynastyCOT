"""Source-build and offline-test the standalone Play Now replay hotfix.

No installation, game injection or publication. Stable INI/diagnostics remain
pinned to the last public bundle. Touchdown experiments are excluded entirely.
"""
from pathlib import Path
import argparse,datetime,hashlib,json,shutil,subprocess,zipfile
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--mmc-sdk',required=True);a=p.parse_args();sdk=Path(a.mmc_sdk).resolve()
 src=ROOT/'Modules/GamedayAtmosphere/Source';tests=src.parent/'Tests';version=json.loads((src/'Product.json').read_text())['version']
 if version!='2.1.11':raise ValueError('This reviewed hotfix builder targets 2.1.11')
 text=(src/'NativeRuntime.cpp').read_text()
 for token in ['TouchdownReplayRequest.h','GAMEDAY_TOUCHDOWN_REPLAY_TEST','touchdownReplayRequest','TDSettleProbe']:
  if token in text:raise ValueError('Experimental touchdown implementation present: '+token)
 work=ROOT/'Build'/('GamedayReplayRelease-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'));work.mkdir();stage=work/'Package';stage.mkdir();logs=[]
 def run(command):
  result=subprocess.run([str(x) for x in command],cwd=work,capture_output=True,text=True,encoding='utf-8',errors='replace');logs.append(result.stdout+result.stderr);(work/'build.log').write_text('\n'.join(logs),encoding='utf-8')
  if result.returncode:raise RuntimeError(logs[-1])
  print(result.stdout.strip().splitlines()[-1] if result.stdout.strip() else 'PASS command',flush=True)
 vs=Path(subprocess.check_output([r'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe','-latest','-products','*','-requires','Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-property','installationPath'],text=True).strip());tool=sorted((vs/'VC/Tools/MSVC').iterdir())[-1];kits=Path(r'C:\Program Files (x86)\Windows Kits\10');win=sorted(x for x in (kits/'Include').iterdir() if (x/'ucrt').is_dir() and (kits/'Lib'/x.name/'um/x64').is_dir())[-1]
 includes=[tool/'include',*(win/x for x in ['ucrt','shared','um']),work];libs=[tool/'lib/x64',kits/'Lib'/win.name/'ucrt/x64',kits/'Lib'/win.name/'um/x64']
 (work/'ProductVersion.h').write_text('#pragma once\n#define GAMEDAY_VERSION "'+version+'"\n#define GAMEDAY_SCHEMA 2\n',encoding='ascii')
 cl=[tool/'bin/Hostx64/x64/cl.exe','/nologo','/O2','/MT','/W4','/WX','/EHsc','/std:c++17','/utf-8',*(f'/I{x}' for x in includes)];link=['/link',*(f'/LIBPATH:{x}' for x in libs),'/INCREMENTAL:NO']
 tested=[]
 for name in ['ReplayEntryGateTests','ReplayPresentationTests','MajorReplayTests','ReplaySelectionTraceTests','ReplayUpdateHookTests','OfflineMatchScopeTests','SettingsMenuTests']:
  exe=work/(name+'.exe');run([*cl,tests/(name+'.cpp'),'/Fe:'+str(exe),*link]);run([exe]);tested.append(name)
 native=work/('GamedayAtmosphereNative-'+version+'.dll');run([*cl,'/DUNICODE','/D_UNICODE','/LD',src/'NativeRuntime.cpp','/Fe:'+str(native),*link])
 reference=ROOT/'Release/DynastyCOT-1.0.2.zip'
 with zipfile.ZipFile(reference) as z:baseline=json.loads(z.read('package-manifest.json'))
 expected={x['path']:x['sha256'] for x in baseline['embedded']};defaults=src/'DynamicGamedayAtmosphere.ini';diagnostics=src.parent/'Build/Release219/DgaDiagnostics.exe'
 for file,name in [(defaults,'Attendance.DynamicGamedayAtmosphere.ini'),(diagnostics,'Attendance.DgaDiagnostics.exe')]:
  if sha(file)!=expected['resource:'+name]:raise ValueError('Stable supporting resource changed')
 info=work/'BuildInfo.cs';info.write_text('namespace DynamicGamedayAtmosphere {internal static class BuildInfo {public const string Version="'+version+'";public const string AssemblyVersion="'+version+'.0";public const string NativeFile="'+native.name+'";}}',encoding='utf-8')
 resources=[(native,'Attendance.'+native.name),(defaults,'Attendance.DynamicGamedayAtmosphere.ini'),(diagnostics,'Attendance.DgaDiagnostics.exe')]
 csc=Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe');dll=stage/'GamedayAtmosphere.dll'
 managed=[src/'MmcPlugin.cs',info,*(src/x for x in ['LaunchDiagnostics.cs','PluginSettings.cs','PluginStorage.cs','ManagedRuntimeLoader.cs'])]
 run([csc,'/nologo','/target:library','/platform:x64','/optimize+','/define:DIRECT_RUNTIME_LOADING;STANDALONE_GAMEDAY','/out:'+str(dll),'/reference:'+str(sdk/'FrostyCore.dll'),'/reference:'+str(sdk/'FrostySdk.dll'),*(f'/resource:{file},{name}' for file,name in resources),*managed])
 docs=ROOT/'Packaging/Standalone/GamedayAtmosphere'
 for name in ['README.txt','FEATURES.md']:shutil.copyfile(docs/name,stage/name)
 # Track the complete native dependency tree, managed sources, release docs and checks.
 tracked=[x for x in src.rglob('*') if x.suffix in ['.h','.cpp','.cs','.inc','.json','.ini'] and x.name!='TouchdownReplayRequest.h']
 tracked += [src.parent/'../DynastyEnhancer/Vendor/json.hpp',Path(__file__),ROOT/'Tests/GamedayStandaloneFixture.cs',ROOT/'Tests/test_gameday_standalone.py',ROOT/'Tools/package_gameday_standalone.py',*(tests/(x+'.cpp') for x in tested),*(docs/x for x in ['README.txt','FEATURES.md','RELEASE_NOTES.md'])]
 result=dict(format=1,module='GamedayAtmosphere',version=version,runtimeVersion=version,stage=str(stage),work=str(work),dll=str(dll),dllSha256=sha(dll),nativeSourceBuilt=True,nativeSha256=sha(native),nativeTests=tested,referenceVersion='1.0.2',referenceArchiveSha256=sha(reference),resources=[dict(name=name,sha256=sha(file)) for file,name in resources],sourceHashes={x.resolve().relative_to(ROOT).as_posix():sha(x) for x in tracked},sdk=str(sdk),installed=False,published=False,experimentalTouchdownRouting=False,liveGameAcceptance='Play Now 20-yard non-touchdown completion showed full-screen and inset replays in private test; clean production package not live-installed. No touchdown fix or universal compatibility claim.')
 (ROOT/'Build/current-gameday-standalone.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');print('SOURCE-BUILT HOTFIX READY:',stage,flush=True)
if __name__=='__main__':main()
