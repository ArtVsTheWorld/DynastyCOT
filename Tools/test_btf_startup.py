"""Bounded loader regressions. Fakes/self-process reads only; no game attachment."""
from pathlib import Path
import datetime,subprocess,json,hashlib
R=Path(__file__).resolve().parents[1]
def main():
 work=R/'Build'/('BtfStartup-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'));work.mkdir()
 src=R/'Modules/GamedayAtmosphere/Source';tests=src.parent/'Tests';logs=[]
 def run(cmd):
  p=subprocess.run(list(map(str,cmd)),cwd=R,capture_output=True,text=True);logs.append(p.stdout+p.stderr);(work/'tests.log').write_text('\n'.join(logs),encoding='utf8');assert p.returncode==0,p.stdout+p.stderr;print(p.stdout.strip())
 for name,files in [('ManagedRuntimeLoaderTests',['ManagedRuntimeLoader.cs','LaunchDiagnostics.cs']),('LauncherDiagnosticsTests',['LaunchDiagnostics.cs'])]:
  exe=work/(name+'.exe');run([r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe','/nologo','/platform:x64','/optimize+','/warnaserror+','/out:'+str(exe),tests/(name+'.cs'),*[src/f for f in files]]);run([exe])
 run(['python',R/'Modules/DynastyEnhancer/Tests/btf_managed_launcher_test.py'])
 evidence=dict(tests=['ManagedRuntimeLoaderTests','LauncherDiagnosticsTests','btf_managed_launcher_test'],work=str(work),gameOpened=False,liveInstall=False,snapshotReads=8,snapshotTotalDelayMs=700,sources={str(p.relative_to(R)):hashlib.sha256(p.read_bytes()).hexdigest() for p in [src/'ManagedRuntimeLoader.cs',R/'Modules/DynastyEnhancer/Source/MmcRecruitingPlugin.cs',tests/'ManagedRuntimeLoaderTests.cs']})
 (R/'Build/btf-startup-verification.json').write_text(json.dumps(evidence,indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
