"""Repeat production repair transactions beyond MAX_PATH under four MMC policies."""
from pathlib import Path
import json,datetime,subprocess,os,hashlib
R=Path(__file__).resolve().parents[1];m=json.loads((R/'Build/current-collection-setup.json').read_text());w=R/'Build/LongPathAudit'/('Installer-'+datetime.datetime.now().strftime('%H%M%S'));w.mkdir();src=R/'Packaging/Collection';csc=r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe';stubs=[]
for name,version in [('BeyondTheField','1.0.24'),('DynastyCOT','1.0.2'),('OldLoader','1.0.0'),('OtherPlugin','1.0.0')]:
 code=w/(name+'.cs');code.write_text('using System.Reflection;[assembly:AssemblyVersion("'+version+'.0")]public class Stub {public static void Main(){}}');dll=code.with_suffix('.exe' if name=='OldLoader' else '.dll');subprocess.run([csc,'/nologo','/target:'+('exe' if name=='OldLoader' else 'library'),'/out:'+str(dll),str(code)],check=True);stubs.append(dll)
code=(R/'Tests/CollectionRepairChecks.cs').read_text();code='using File=Ace.DynastyCOT.StorageFile;using Directory=Ace.DynastyCOT.StorageDirectory;using Path=Ace.DynastyCOT.StoragePath;\n'+code
# Test-only resource inspection, not production installer loading.
code=code.replace('System.Reflection.Assembly.LoadFile(payloads.Single(p=>p.Module.Id=="GamedayAtmosphere").Dll)','System.Reflection.Assembly.Load(File.ReadAllBytes(payloads.Single(p=>p.Module.Id=="GamedayAtmosphere").Dll))');p=w/'CollectionLongPathChecks.cs';p.write_text(code);exe=w/'CollectionLongPathChecks.exe';subprocess.run([csc,'/nologo','/platform:x64','/warnaserror+','/out:'+str(exe),'/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll',str(Path(m['work'])/'CollectionInfo.cs'),*[str(src/n) for n in ['Models.cs','Paths.cs','Installer.cs','RepairData.cs','Downloads.cs']],str(R/'Core/UserStorage.cs'),str(p)],check=True);results=[]
for legacy,block in [('true','true'),('true','false'),('false','true'),('false','false')]:
 f=w/(legacy+'-'+block)/('a'*100)/('b'*100)/('c'*70);os.makedirs('\\\\?\\'+str(f));Path(str(exe)+'.config').write_text('<configuration><runtime><AppContextSwitchOverrides value="Switch.System.IO.UseLegacyPathHandling='+legacy+';Switch.System.IO.BlockLongPaths='+block+'" /></runtime></configuration>');run=subprocess.run([str(exe),str(R),str(f),m['catalog'],m['artifactRoot'],*map(str,stubs)],capture_output=True,text=True);print(legacy,block,run.stdout+run.stderr,flush=True);results.append(dict(legacy=legacy,block=block,length=len(str(f)),code=run.returncode,result=run.stdout+run.stderr))
 if run.returncode:break
 # Independently verify published text and current Notes bytes via Unicode paths.
 assert Path('\\\\?\\'+str(f/'UserData/BeyondTheField/RuntimeHistory/42/editions/2026-1.json')).read_bytes()==b'published wording'
 assert Path('\\\\?\\'+str(f/'UserData/BeyondTheField/ScoutingReports/42/2026.json')).read_bytes()==b'newer notes'
(R/'Build/LongPathAudit/installer-results.json').write_text(json.dumps(dict(work=str(w),results=results,liveChanges=False),indent=2));assert len(results)==4 and all(v['code']==0 for v in results)
