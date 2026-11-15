"""Compile focused private installer tests; all writes stay in marked fixtures."""
from pathlib import Path
import json,subprocess,datetime,hashlib
R=Path(__file__).resolve().parents[1]
def main():
 setup=json.loads((R/'Build/current-collection-setup.json').read_text(encoding='utf8'));work=R/'Build/CollectionRepair'/('Focused-'+datetime.datetime.now(datetime.timezone.utc).strftime('%H%M%S%f'));work.mkdir();compiler=Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe');stubs=[]
 for name,version in [('BeyondTheField','1.0.24'),('DynastyCOT','1.0.2'),('OldLoader','1.0.0'),('OtherPlugin','1.0.0')]:
  source=work/(name+'.cs');source.write_text('using System.Reflection;[assembly:AssemblyVersion("'+version+'.0")]public class Stub {public static void Main(){}}',encoding='ascii');dll=work/(name+('.exe' if name=='OldLoader' else '.dll'));subprocess.run([str(compiler),'/nologo','/target:'+('exe' if name=='OldLoader' else 'library'),'/out:'+str(dll),str(source)],check=True);stubs.append(dll)
 src=R/'Packaging/Collection';exe=work/'CollectionRepairChecks.exe';subprocess.run([str(compiler),'/nologo','/platform:x64','/optimize+','/warnaserror+','/out:'+str(exe),'/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll',str(Path(setup['work'])/'CollectionInfo.cs'),*[str(src/f) for f in ['Models.cs','Paths.cs','Installer.cs','RepairData.cs','Downloads.cs']],str(R/'Core/UserStorage.cs'),str(R/'Tests/CollectionRepairChecks.cs')],check=True)
 p=subprocess.run([str(exe),str(R),str(work/'Fixtures'),setup['catalog'],setup['artifactRoot'],*map(str,stubs)],capture_output=True,text=True);(work/'results.log').write_text(p.stdout+p.stderr,encoding='utf8');print(p.stdout+p.stderr);assert p.returncode==0
 (R/'Build/CollectionRepair/focused-verification.json').write_text(json.dumps(dict(result=p.stdout.strip(),fixture=str(work),liveInstall=False,setupSha256=hashlib.sha256(Path(setup['exe']).read_bytes()).hexdigest()),indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
