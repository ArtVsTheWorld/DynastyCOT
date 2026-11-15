"""Build and inspect real fixture windows; no live installer operations."""
from pathlib import Path
import datetime,json,subprocess,shutil
R=Path(__file__).resolve().parents[1]
m=json.loads((R/'Build/current-collection-setup.json').read_text())
w=R/'Build/CollectionRepair'/('Windows-'+datetime.datetime.now().strftime('%H%M%S%f'));w.mkdir()
c=Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe');src=R/'Packaging/Collection';ico=R/'Packaging/Setup/Assets/Crown.ico'
exe=w/'WindowChecks.exe'
subprocess.run([str(c),'/nologo','/platform:x64','/optimize+','/warnaserror+','/out:'+str(exe),'/main:CollectionRepairWindowChecks','/reference:System.Windows.Forms.dll','/reference:System.Drawing.dll','/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll','/resource:'+m['catalog']+',Collection.catalog.json','/resource:'+str(ico)+',Collection.icon.ico',str(Path(m['work'])/'CollectionInfo.cs'),str(R/'Core/UserStorage.cs'),str(R/'Packaging/Setup/ModernControls.cs'),*map(str,src.glob('*.cs')),str(R/'Tests/CollectionRepairWindowChecks.cs')],check=True)
shutil.copyfile(R/'Packaging/Collection/app.config',str(exe)+'.config')
p=subprocess.run([str(exe),str(w),m['artifactRoot'],str(ico)],capture_output=True,text=True,timeout=120);(w/'results.log').write_text(p.stdout+p.stderr);print(p.stdout+p.stderr);assert p.returncode==0
(R/'Build/CollectionRepair/window-verification.json').write_text(json.dumps(dict(result=p.stdout.strip(),fixture=str(w),liveChanges=False),indent=2)+'\n')
