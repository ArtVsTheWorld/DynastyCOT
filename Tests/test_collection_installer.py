"""Run actual collection file/update code only inside marked isolated fixtures."""
from pathlib import Path
import datetime,hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 meta=json.loads((ROOT/'Build/current-collection-setup.json').read_text());source=ROOT/'Packaging/Collection'
 assert sha(Path(meta['exe']))==meta['sha256'],'Installer changed after build'
 for rel,digest in meta['sources'].items():assert sha(ROOT/rel)==digest,'Rebuild changed installer: '+rel
 work=ROOT/'Build'/('CC-'+datetime.datetime.now(datetime.timezone.utc).strftime('%m%d%H%M%S'));work.mkdir()
 compiler=Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe');exe=work/'CollectionInstallerChecks.exe'
 args=[str(compiler),'/nologo','/platform:x64','/optimize+','/warnaserror+','/out:'+str(exe),'/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll',str(Path(meta['work'])/'CollectionInfo.cs'),*(str(source/name) for name in ['Models.cs','Paths.cs','Installer.cs','Downloads.cs']),str(ROOT/'Core/UserStorage.cs'),str(ROOT/'Tests/CollectionInstallerChecks.cs')]
 compile=subprocess.run(args,capture_output=True,text=True);(work/'compile.log').write_text(compile.stdout+compile.stderr);assert compile.returncode==0,compile.stdout+compile.stderr
 future=work/'FutureModule';future.mkdir();future_source=future/'FutureModule.cs';future_source.write_text('using System.Reflection;[assembly:AssemblyVersion("9.9.9.0")]public static class FutureModule {}')
 future_dll=future/'BeyondTheField.dll';subprocess.run([str(compiler),'/nologo','/target:library','/out:'+str(future_dll),str(future_source)],check=True)
 result=subprocess.run([str(exe),str(ROOT),str(work/'Fixtures'),str(future_dll),meta.get('catalog',str(source/'catalog.json')),meta.get('artifactRoot',str(ROOT/'Release'))],capture_output=True,text=True);(work/'results.log').write_text(result.stdout+result.stderr);assert result.returncode==0,result.stdout+result.stderr
 print(result.stdout.strip())
 root=work/'LayoutFixture';root.mkdir();(root/'.cot-fixture').write_text('isolated');(root/'MMCModManager.exe').write_text('fixture; never executed')
 layout=subprocess.run([meta['exe'],'--layout-check',str(root),str(work),str(work/'Layout')],capture_output=True,text=True);(work/'layout.log').write_text(layout.stdout+layout.stderr);assert layout.returncode==0,layout.stdout+layout.stderr
 print(layout.stdout.strip())
 report=dict(version=meta['version'],exeSha256=meta['sha256'],fixtureRoot=str(work),fileChecks=result.stdout.strip(),layout=layout.stdout.strip(),sources=meta['sources'],tests={p.relative_to(ROOT).as_posix():sha(p) for p in [Path(__file__),ROOT/'Tests/CollectionInstallerChecks.cs']},liveInstall=False,published=False)
 (ROOT/'Build/collection-verification.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
if __name__=='__main__':main()
