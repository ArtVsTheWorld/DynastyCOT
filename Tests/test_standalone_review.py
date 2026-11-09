"""Actual old/new window errors and reviewed migration; private fixtures only."""
from pathlib import Path
import base64, datetime, hashlib, json, os, subprocess, zipfile
ROOT=Path(__file__).resolve().parents[1]

def main():
    base=ROOT/'Build'/('StandaloneReview-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'))
    root=base/"MMC's review fixture";root.mkdir(parents=True)
    files={'.cot-fixture':b'isolated','MMCModManager.exe':b'never executed',
           'Plugins/DynamicGamedayAtmosphere.dll':b'unknown older GDA bytes, never executed',
           'Plugins/OfflineDynastySettings.dll':b'unknown older settings bytes, never executed',
           'Plugins/DynamicGamedayAtmosphere/Runtime-2.0.21/DynamicGamedayAtmosphereNative-2.0.21.dll':b'older extracted native bytes, never executed',
           'Plugins/OtherOverhaul.dll':b'unrelated untouched bytes',
           'Plugins/DynamicGamedayAtmosphere/settings.ini':b'user crowd settings',
           'Plugins/DynastyEnhancer/RuntimeHistory/77/posts.json':b'published words'}
    for name,data in files.items():
        p=root/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data)
    stage=Path(json.loads((ROOT/'Build/current-package.json').read_text())['stage'])
    archive=base/'fixture-payload.zip'
    with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=1) as z:
        for p in sorted(stage.rglob('*')):
            if p.is_file():z.write(p,p.relative_to(stage).as_posix())
    product=json.loads((ROOT/'cot.product.json').read_text(encoding='utf-8'));modules=product['modules']
    info=base/'SetupInfo.cs';info.write_text('namespace DynastyCOT.Setup {internal static class SetupInfo {internal const string Version='+json.dumps(product['version'])+';internal const string PayloadSha256='+json.dumps(hashlib.sha256(archive.read_bytes()).hexdigest())+';internal const string UpdateRepository="ArtVsTheWorld/DynastyCOT";internal static readonly string[] ModuleNames=new string[]{'+','.join(json.dumps(x['displayName']) for x in modules)+'};internal static readonly string[] ModuleDescriptions=new string[]{'+','.join(json.dumps(x['description']) for x in modules)+'};}}',encoding='utf-8')
    source=ROOT/'Packaging/Setup';exe=base/'StandaloneReviewChecks.exe'
    command=[r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe','/nologo','/platform:x64','/main:DynastyCOT.Setup.StandaloneReviewChecks','/out:'+str(exe),'/reference:System.Windows.Forms.dll','/reference:System.Drawing.dll','/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll','/resource:'+str(archive)+',DynastyCOT.Package.zip','/resource:'+str(source/'Assets/Crown.ico')+',DynastyCOT.Crown.ico','/resource:'+str(source/'Assets/Crown.png')+',DynastyCOT.Crown.png',str(info),str(ROOT/'Tests/StandaloneReviewChecks.cs'),*(str(p) for p in source.glob('*.cs'))]
    subprocess.run(command,check=True)
    old=ROOT/'Release/DynastyCOT-Setup-1.0.0.exe';report=base/'result.json'
    result=subprocess.run([str(exe),str(root),str(base),str(old),str(report)],capture_output=True,text=True,timeout=240)
    (base/'checks.log').write_text(result.stdout+result.stderr,encoding='utf-8')
    assert result.returncode==0,result.stdout+result.stderr
    doc=json.loads(report.read_text());doc.update(stage=str(stage),fixtureRoot=str(base),version=product['version'],sourceHashes={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in [ROOT/'Packaging/Install.ps1',*source.glob('*.cs')]})
    # The same reviewed bytes must also survive all injected transaction failures.
    def snapshot(folder):
        return {p.relative_to(folder).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in folder.rglob('*') if p.is_file() and 'Backups' not in p.parts}
    def quote(value):return "'"+str(value).replace("'","''")+"'"
    reviewed=[dict(path=k,module='ExtraDynastySettings' if 'OfflineDynastySettings.dll' in k else 'GamedayAtmosphere',sha256=hashlib.sha256(v).hexdigest()) for k,v in files.items() if k.endswith('.dll') and 'OtherOverhaul' not in k]
    for fault in ['AfterPayload','AfterLegacyDisable','AfterAdapter']:
        target=base/('Fault-'+fault);target.mkdir()
        for name,data in files.items():
            dest=target/name;dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(data)
        before=snapshot(target)
        call='& '+quote(stage/'Install.ps1')+' -MmcRoot '+quote(target)+' -PackageRoot '+quote(stage)+' -Fixture -FixtureRoot '+quote(base)+' -ReviewedStandaloneJson '+quote(json.dumps(reviewed))+' -FaultAtStage '+fault
        command="$ErrorActionPreference='Stop';$ProgressPreference='SilentlyContinue';try { "+call+";exit 0 } catch { [Console]::Error.WriteLine($_.Exception.Message);exit 1 }"
        environment={k:v for k,v in os.environ.items() if k.lower()!='psmodulepath'}
        run=subprocess.run([r'C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe','-NoProfile','-NonInteractive','-ExecutionPolicy','Bypass','-EncodedCommand',base64.b64encode(command.encode('utf-16le')).decode()],capture_output=True,text=True,timeout=90,env=environment)
        assert run.returncode!=0 and 'previous managed files were restored' in run.stderr,run.stdout+run.stderr
        assert before==snapshot(target),'Reviewed standalone fault rollback differs: '+fault
    doc['faultRollbackStages']=3
    (ROOT/'Build/standalone-review-verification.json').write_text(json.dumps(doc,indent=2)+'\n',encoding='utf-8');print(result.stdout.strip())

if __name__=='__main__':main()
