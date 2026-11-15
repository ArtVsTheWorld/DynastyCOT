"""Run the real collection archive/install/rollback tests on isolated hotfix assets."""
from pathlib import Path
import datetime,json,subprocess,shutil,hashlib
R=Path(__file__).resolve().parents[1];E=R/'Build/LoaderHotfix'
def main():
 work=E/('Checks-'+datetime.datetime.now(datetime.timezone.utc).strftime('%H%M%S%f'));work.mkdir();assets=work/'Assets';assets.mkdir();catalog=json.loads((E/'catalog.json').read_text());setup=json.loads((R/'Build/current-collection-setup.json').read_text());src=R/'Packaging/Collection'
 for m in catalog['Modules']:
  current=m['Current'];folder=R/('Release/Loader-Hotfix' if m['Id'] in ['BeyondTheField','GamedayAtmosphere'] else 'Release/Publication-Final')
  for leaf in [current['ArchiveName'],current['ArchiveName'].removesuffix('.zip')+'-verification.json']:shutil.copyfile(folder/leaf,assets/leaf)
  assert hashlib.sha256((assets/current['ArchiveName']).read_bytes()).hexdigest()==current['ArchiveSha256']
 compiler=Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe');future=work/'Future.cs';future.write_text('using System.Reflection;[assembly:AssemblyVersion("9.9.9.0")]public static class FutureModule {}')
 futureDll=work/'BeyondTheField.dll';subprocess.run([str(compiler),'/nologo','/target:library','/out:'+str(futureDll),str(future)],check=True)
 exe=work/'CollectionInstallerChecks.exe';subprocess.run([str(compiler),'/nologo','/platform:x64','/optimize+','/warnaserror+','/out:'+str(exe),'/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll',str(Path(setup['work'])/'CollectionInfo.cs'),*[str(src/f) for f in ['Models.cs','Paths.cs','Installer.cs','RepairData.cs','Downloads.cs']],str(R/'Core/UserStorage.cs'),str(R/'Tests/CollectionInstallerChecks.cs')],check=True)
 result=subprocess.run([str(exe),str(R),str(work/'Fixtures'),str(futureDll),str(E/'catalog.json'),str(assets)],capture_output=True,text=True);(work/'results.log').write_text(result.stdout+result.stderr,encoding='utf8');assert result.returncode==0,result.stdout+result.stderr;print(result.stdout)
 (E/'archive-verification.json').write_text(json.dumps(dict(results=result.stdout.strip(),catalogSha256=hashlib.sha256((E/'catalog.json').read_bytes()).hexdigest(),fixture=str(work),liveInstall=False),indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
