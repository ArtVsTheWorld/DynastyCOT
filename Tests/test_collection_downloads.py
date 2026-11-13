"""Explicit public-network acceptance; downloads to a fixture, never installs."""
from pathlib import Path
import argparse,datetime,hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser();p.add_argument('--network',action='store_true',required=True);p.add_argument('--catalog',type=Path,default=ROOT/'Packaging/Collection/catalog.json');args=p.parse_args()
 meta=json.loads((ROOT/'Build/current-collection-setup.json').read_text());work=ROOT/'Build'/('CollectionNetwork-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'));work.mkdir();fixture=work/'Fixture';fixture.mkdir();(fixture/'.cot-fixture').write_text('isolated')
 assert hashlib.sha256(Path(meta['exe']).read_bytes()).hexdigest()==meta['sha256'],'Installer changed after build'
 exe=work/'CollectionPublishedDownloads.exe';compiler=Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe');src=ROOT/'Packaging/Collection'
 subprocess.run([str(compiler),'/nologo','/platform:x64','/optimize+','/warnaserror+','/out:'+str(exe),'/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll',str(Path(meta['work'])/'CollectionInfo.cs'),*(str(src/n) for n in ['Models.cs','Paths.cs','Installer.cs','Downloads.cs']),str(ROOT/'Core/UserStorage.cs'),str(ROOT/'Tests/CollectionPublishedDownloads.cs')],check=True)
 result=subprocess.run([str(exe),str(ROOT),str(fixture),str(args.catalog)],capture_output=True,text=True);(work/'results.log').write_text(result.stdout+result.stderr);print(result.stdout);assert result.returncode==0,result.stderr
 tests={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in [Path(__file__),ROOT/'Tests/CollectionPublishedDownloads.cs']}
 (ROOT/'Build/collection-network-verification.json').write_text(json.dumps(dict(catalogSha256=hashlib.sha256(args.catalog.read_bytes()).hexdigest(),testedPublicBaselines=True,version=meta['version'],exeSha256=meta['sha256'],anonymous=True,moduleCount=4,fixture=str(fixture),results=result.stdout,sources=meta['sources'],tests=tests,liveInstall=False,published=False),indent=2)+'\n',encoding='utf-8')
if __name__=='__main__':main()
