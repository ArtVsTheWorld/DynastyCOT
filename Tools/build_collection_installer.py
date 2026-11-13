"""Build the lightweight collection manager. Modules remain exact standalone DLLs."""
from pathlib import Path
import argparse,datetime,hashlib,json,subprocess,shutil
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--catalog',type=Path);parser.add_argument('--artifact-root',type=Path);args=parser.parse_args()
 source=ROOT/'Packaging/Collection';catalog=args.catalog.resolve() if args.catalog else source/'catalog.json';data=json.loads(catalog.read_text(encoding='utf-8'));version=data['Version']
 work=ROOT/'Build'/('CollectionSetup-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'));work.mkdir()
 info=work/'CollectionInfo.cs';info.write_text('using System.Reflection;\n[assembly:System.Runtime.Versioning.TargetFramework(".NETFramework,Version=v4.8")]\n[assembly:AssemblyTitle("Dynasty COT Collection")]\n[assembly:AssemblyCompany("Ace")]\n[assembly:AssemblyVersion("'+version.split('-')[0]+'.0")]\nnamespace DynastyCOT.Collection {internal static class CollectionInfo {internal const string ConfigSha256='+json.dumps(sha(source/'app.config'))+';internal const string Version='+json.dumps(version)+';internal const string CatalogSha256='+json.dumps(sha(catalog))+';}}',encoding='utf-8')
 output=ROOT/f'Release/DynastyCOT-Setup-{version}.exe';compiler=Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe');ui=ROOT/'Packaging/Setup'
 subprocess.run([str(compiler),'/nologo','/target:winexe','/platform:x64','/optimize+','/warnaserror+','/out:'+str(output),'/win32icon:'+str(ui/'Assets/Crown.ico'),'/win32manifest:'+str(ui/'app.manifest'),'/reference:System.Windows.Forms.dll','/reference:System.Drawing.dll','/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll','/resource:'+str(catalog)+',Collection.catalog.json','/resource:'+str(ui/'Assets/Crown.ico')+',Collection.icon.ico',str(info),str(ROOT/'Core/UserStorage.cs'),str(ui/'ModernControls.cs'),*(str(p) for p in sorted(source.glob('*.cs')))],check=True)
 config=output.with_suffix(output.suffix+'.config');shutil.copyfile(source/'app.config',config)
 sources=[source/'app.config',ROOT/'Core/UserStorage.cs',*source.glob('*.cs'),catalog,ui/'ModernControls.cs',ui/'app.manifest',ui/'Assets/Crown.ico',Path(__file__)]
 report=dict(artifactRoot=str(args.artifact_root.resolve() if args.artifact_root else ROOT/'Release'),config=str(config),configSha256=sha(config),version=version,exe=str(output),sha256=sha(output),work=str(work),catalog=str(catalog),catalogSha256=sha(catalog),sources={p.relative_to(ROOT).as_posix():sha(p) for p in sources},embeddedModuleDlls=False,installsStandaloneDlls=True,installedIntoLiveMmc=False,published=False)
 (ROOT/'Build/current-collection-setup.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
 (output.with_name(output.stem+'-SHA256.txt')).write_text(sha(output)+'  '+output.name+'\n',encoding='utf-8')
 print('Built lightweight installer:',output)
if __name__=='__main__':main()
