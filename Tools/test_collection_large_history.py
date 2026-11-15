"""Exercise large synthetic history; all writes stay in marked Build fixtures."""
from pathlib import Path
import datetime,json,subprocess,hashlib
R=Path(__file__).resolve().parents[1]
m=json.loads((R/'Build/current-collection-setup.json').read_text())
w=R/'Build/CollectionRepair'/('LargeHistory-'+datetime.datetime.now().strftime('%H%M%S%f'));w.mkdir()
exe=w/'LargeHistoryChecks.exe';src=R/'Packaging/Collection'
subprocess.run([r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe','/nologo','/platform:x64','/optimize+','/warnaserror+','/out:'+str(exe),'/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll',str(Path(m['work'])/'CollectionInfo.cs'),*[str(src/(s+'.cs')) for s in ['Models','Paths','Installer','RepairData','Downloads']],str(R/'Core/UserStorage.cs'),str(R/'Tests/CollectionLargeHistoryChecks.cs')],check=True)
p=subprocess.run([str(exe),str(w),m['catalog'],m['artifactRoot']],capture_output=True,text=True);(w/'results.log').write_text(p.stdout+p.stderr);print(p.stdout+p.stderr);assert p.returncode==0
report=json.loads((w/'results.json').read_text());report.update(fixture=str(w),exeSha256=m['sha256'],sources=m['sources'],testSources={str(p.relative_to(R)):hashlib.sha256(p.read_bytes()).hexdigest() for p in [Path(__file__),R/'Tests/CollectionLargeHistoryChecks.cs']})
(R/'Build/CollectionRepair/large-history-verification.json').write_text(json.dumps(report,indent=2)+'\n')
