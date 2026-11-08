"""Build a single-file Windows setup around the verified private package ZIP."""
from pathlib import Path
import datetime,hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    meta=json.loads((ROOT/'Build/current-package.json').read_text())
    verification=json.loads(Path(meta['verification']).read_text())
    archive=Path(meta['archive']);version=verification['version']
    product=json.loads((ROOT/'cot.product.json').read_text(encoding='utf-8'))
    if product['version']!=version:raise RuntimeError('Stage and verify the current product version before building setup')
    if sha(archive)!=verification['archiveSha256']:raise RuntimeError('Verified package ZIP has changed')
    work=ROOT/'Build'/('Setup-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'))
    work.mkdir();info=work/'SetupInfo.cs'
    numeric=version.split('-')[0]+'.0'
    from create_update_metadata import main as create_update_metadata
    create_update_metadata()
    by_id={module['id']:module for module in product['modules']}
    order=['DynastyEnhancer','GamedayAtmosphere','ExtraDynastySettings','DrivePractice']
    names=[by_id[id]['displayName'] for id in order]
    descriptions=[by_id[id]['description'] for id in order]
    repository=product['distribution']['repository']
    info.write_text('using System.Reflection;\n[assembly:AssemblyTitle("Dynasty COT Setup")]\n'
        '[assembly:AssemblyProduct("Dynasty COT")]\n[assembly:AssemblyCompany("Ace")]\n'
        '[assembly:AssemblyVersion('+json.dumps(numeric)+')]\n[assembly:AssemblyFileVersion('+json.dumps(numeric)+')]\n'
        'namespace DynastyCOT.Setup { internal static class SetupInfo { internal const string Version='+json.dumps(version)+';'
        'internal const string PayloadSha256='+json.dumps(sha(archive))+';'
        'internal const string UpdateRepository='+json.dumps(repository)+';'
        'internal static readonly string[] ModuleNames=new string[]{'+','.join(map(json.dumps,names))+'};'
        'internal static readonly string[] ModuleDescriptions=new string[]{'+','.join(map(json.dumps,descriptions))+'}; }}\n')
    output=ROOT/'Release'/('DynastyCOT-Setup-'+version+'.exe')
    compiler=Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe')
    source=ROOT/'Packaging/Setup'
    subprocess.run([str(compiler),'/nologo','/target:winexe','/platform:x64','/optimize+',
        '/out:'+str(output),'/win32icon:'+str(source/'Assets/Crown.ico'),'/resource:'+str(source/'Assets/Crown.ico')+',DynastyCOT.Crown.ico','/resource:'+str(source/'Assets/Crown.png')+',DynastyCOT.Crown.png','/win32manifest:'+str(source/'app.manifest'),
        '/reference:System.Windows.Forms.dll','/reference:System.Drawing.dll','/reference:System.Web.Extensions.dll',
        '/reference:System.IO.Compression.dll','/resource:'+str(archive)+',DynastyCOT.Package.zip',
        str(info),*(str(p) for p in sorted(source.glob('*.cs')))],check=True)
    result=dict(exe=str(output),version=version,sha256=sha(output),payloadSha256=sha(archive),work=str(work),
        sources={p.relative_to(ROOT).as_posix():sha(p) for p in source.rglob('*') if p.is_file()})
    (ROOT/'Build/current-setup.json').write_text(json.dumps(result,indent=2)+'\n')
    assets=[archive,output,ROOT/'Release/cot-update.json']
    (ROOT/('Release/SHA256SUMS-'+version+'.txt')).write_text(''.join(sha(path)+'  '+path.name+'\n' for path in assets),encoding='utf-8')
    print('Built single-file setup: '+str(output))
if __name__=='__main__':main()
