"""Build one managed Gameday plugin carrying the released verified native engine.

No installation, engine execution, native rebuild or publishing is performed.
"""
from pathlib import Path
import argparse, datetime, hashlib, json, re, shutil, subprocess, zipfile
ROOT=Path(__file__).resolve().parents[1]

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    p=argparse.ArgumentParser();p.add_argument('--mmc-sdk',required=True);p.add_argument('--version',default='1.0.2-standalone-test');a=p.parse_args()
    if not re.fullmatch(r'\d+\.\d+\.\d+(?:-[A-Za-z0-9.-]+)?',a.version):raise ValueError('Invalid distribution version')
    sdk=Path(a.mmc_sdk).resolve();src=ROOT/'Modules/GamedayAtmosphere/Source';runtime=json.loads((src/'Product.json').read_text(encoding='utf-8'))['version']
    built=ROOT/'Modules/GamedayAtmosphere/Build'/('Release'+runtime.replace('.',''))
    reference=ROOT/'Release/DynastyCOT-1.0.2.zip'
    with zipfile.ZipFile(reference) as z:released=json.loads(z.read('package-manifest.json'))
    expected={x['path']:x['sha256'] for x in released['embedded']}
    resources=[(built/f'GamedayAtmosphereNative-{runtime}.dll',f'Attendance.GamedayAtmosphereNative-{runtime}.dll'),(src/'DynamicGamedayAtmosphere.ini','Attendance.DynamicGamedayAtmosphere.ini'),(built/'DgaDiagnostics.exe','Attendance.DgaDiagnostics.exe')]
    for file,name in resources:
        if sha(file)!=expected['resource:'+name]:raise ValueError('Runtime resource differs from tested COT 1.0.2: '+name)
    stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f')
    work=ROOT/'Build'/('GamedayStandalone-'+stamp);stage=work/'Package';generated=work/'Generated';stage.mkdir(parents=True);generated.mkdir()
    adapter=generated/'GamedayAdapter.cs';text=(src/'MmcPlugin.cs').read_text(encoding='utf-8')
    numeric=a.version.split('-')[0]+'.0'
    text=text.replace('DynamicGamedayAtmosphere.BuildInfo.AssemblyVersion',json.dumps(numeric)).replace('[assembly: PluginVersion(DynamicGamedayAtmosphere.BuildInfo.Version)]','[assembly: PluginVersion('+json.dumps(a.version)+')]')
    adapter.write_text(text,encoding='utf-8',newline='\n')
    sources=[ROOT/'Core/UserStorage.cs',adapter,built/'BuildInfo.cs',*(src/name for name in ['LaunchDiagnostics.cs','PluginSettings.cs','PluginStorage.cs','ManagedRuntimeLoader.cs'])]
    output=stage/'GamedayAtmosphere.dll';compiler=Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe')
    command=[str(compiler),'/nologo','/target:library','/platform:x64','/optimize+','/define:DIRECT_RUNTIME_LOADING;STANDALONE_GAMEDAY','/out:'+str(output),'/reference:'+str(sdk/'FrostyCore.dll'),'/reference:'+str(sdk/'FrostySdk.dll')]
    subprocess.run([*command,*(f'/resource:{file},{name}' for file,name in resources),*(str(x) for x in sources)],check=True)
    docs=ROOT/'Packaging/Standalone/GamedayAtmosphere'
    for name in ['README.txt','FEATURES.md']:shutil.copyfile(docs/name,stage/name)
    tracked=[ROOT/'Core/UserStorage.cs',src/'MmcPlugin.cs',src/'PluginStorage.cs',src/'PluginSettings.cs',src/'ManagedRuntimeLoader.cs',src/'LaunchDiagnostics.cs',docs/'README.txt',docs/'FEATURES.md',Path(__file__).resolve()]
    result=dict(format=1,module='GamedayAtmosphere',version=a.version,runtimeVersion=runtime,stage=str(stage),work=str(work),dll=str(output),dllSha256=sha(output),referenceVersion='1.0.2',referenceArchiveSha256=sha(reference),resources=[dict(name=name,sha256=sha(file)) for file,name in resources],sourceHashes={x.relative_to(ROOT).as_posix():sha(x) for x in tracked},sdk=str(sdk),installed=False,published=False)
    (ROOT/'Build/current-gameday-standalone.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
    print('Built one-DLL Gameday standalone stage:',stage)

if __name__=='__main__':main()
