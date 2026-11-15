"""Final managed GDA rebuild with source-built native/runtime lineage."""
from pathlib import Path
import json,datetime,hashlib,subprocess,shutil
R=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
m=json.loads((R/'Build/current-gameday-standalone.json').read_text());assert m['version']=='2.1.40';old=Path(m['work']).resolve();src=R/'Modules/GamedayAtmosphere/Source'
# Every compiled native input is still the exact tested source. Core .cs is
# managed only and intentionally being recompiled here.
for rel,h in m['sourceHashes'].items():
 if Path(rel).suffix in ['.cpp','.h','.inc'] and (R/rel).exists():assert sha(R/rel)==h,rel
w=R/'Build/LongPathAudit'/('GamedayAtmosphere-'+datetime.datetime.now().strftime('%Y%m%dT%H%M%S'));w.mkdir();stage=w/'Package';shutil.copytree(old/'Package',stage)
native=old/'GamedayAtmosphereNative-2.1.40.dll';assert sha(native)==m['nativeSha256'];diagnostics=w/'DgaDiagnostics.exe';sites=src.parent/'Build/Release219/CompatibilitySites.txt';csc=r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe'
subprocess.run([csc,'/nologo','/target:winexe','/platform:x64','/optimize+','/warnaserror+','/reference:System.Windows.Forms.dll','/reference:System.Drawing.dll','/reference:System.Runtime.Serialization.dll','/resource:'+str(sites)+',Dga.CompatibilitySites','/out:'+str(diagnostics),str(R/'Core/UserStorage.cs'),str(src/'DgaDiagnostics.cs'),str(src/'GameCompatibilityReport.cs')],check=True)
info=w/'BuildInfo.cs';shutil.copyfile(old/'BuildInfo.cs',info);resources=[(native,'Attendance.'+native.name),(src/'DynamicGamedayAtmosphere.ini','Attendance.DynamicGamedayAtmosphere.ini'),(diagnostics,'Attendance.DgaDiagnostics.exe')];sources=[R/'Core/UserStorage.cs',src/'MmcPlugin.cs',info,*[src/n for n in ['LaunchDiagnostics.cs','PluginSettings.cs','PluginStorage.cs','ManagedRuntimeLoader.cs']]];dll=stage/'GamedayAtmosphere.dll';sdk=Path(m['sdk']);subprocess.run([csc,'/nologo','/target:library','/platform:x64','/optimize+','/warnaserror+','/define:DIRECT_RUNTIME_LOADING;STANDALONE_GAMEDAY','/out:'+str(dll),'/reference:'+str(sdk/'FrostyCore.dll'),'/reference:'+str(sdk/'FrostySdk.dll'),*(f'/resource:{p},{n}' for p,n in resources),*map(str,sources)],check=True)
m.update(work=str(w),stage=str(stage),dll=str(dll),dllSha256=sha(dll),resources=[dict(name=n,sha256=sha(p)) for p,n in resources],sourceHashes={rel:sha(R/rel) for rel in m['sourceHashes']},nativeBuildLineage=str(old),nativeSourceBuilt=True,diagnosticsSourceBuilt=True,managedSourceBuilt=True,privateCandidate='longpath1',sourceBuildTool=sha(Path(__file__)));m['sourceHashes']['Core/NativeFileIO.h']=sha(R/'Core/NativeFileIO.h');(R/'Build/LongPathAudit/GamedayAtmosphere-candidate.json').write_text(json.dumps(m,indent=2));print('BUILT GDA 2.1.40 longpath1',dll)
