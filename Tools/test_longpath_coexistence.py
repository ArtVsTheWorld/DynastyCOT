"""Actual prepared module files under long shared data paths, no callbacks/game."""
from pathlib import Path
import json,subprocess,datetime,shutil,os
R=Path(__file__).resolve().parents[1];w=R/'Build/LongPathAudit'/('Coexistence-'+datetime.datetime.now().strftime('%H%M%S'));w.mkdir();csc=r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe';sdk=r'C:\Apps\CFB 27\MMC\MMC_ModManager_v1.1.0.6';m=json.loads((R/'Build/LongPathAudit/candidates.json').read_text());gda=json.loads((R/'Build/LongPathAudit/GamedayAtmosphere-candidate.json').read_text());candidates={x['name']:x for x in m};candidates['GamedayAtmosphere']=gda
code=(R/'Tests/StandaloneModulesFixture.cs').read_text();code='using File=Ace.DynastyCOT.StorageFile;using Directory=Ace.DynastyCOT.StorageDirectory;using Path=Ace.DynastyCOT.StoragePath;\n'+code
old='Path.Combine(Path.GetDirectoryName(root),Path.GetFileName(root)+"-Profile","Ace","DynastyCOT")';new='Path.Combine(Path.GetDirectoryName(root),Path.GetFileName(root)+"-Profile",new string(\'a\',100),new string(\'b\',100),"Ace","DynastyCOT")';assert old in code;code=code.replace(old,new)
# The GDA extraction containment assertion deliberately uses its broader profile
# prefix and remains unchanged. Only the chosen test UserStore gets deeper.
p=w/'StandaloneLongPathChecks.cs';p.write_text(code);exe=w/'StandaloneLongPathChecks.exe';subprocess.run([csc,'/nologo','/platform:x64','/warnaserror+','/out:'+str(exe),'/reference:System.Web.Extensions.dll',str(R/'Core/UserStorage.cs'),str(p)],check=True);results=[]
for legacy,block in [('true','true'),('true','false'),('false','true'),('false','false')]:
 for mode in ['files','claim','conflict']:
  fixture=w/(legacy+'-'+block+'-'+mode);(fixture/'Plugins').mkdir(parents=True);(fixture/'.cot-fixture').write_text('isolated')
  for name,meta in candidates.items():shutil.copyfile(meta['dll'],fixture/'Plugins'/(name+'.dll'))
  Path(str(exe)+'.config').write_text('<configuration><runtime><AppContextSwitchOverrides value="Switch.System.IO.UseLegacyPathHandling='+legacy+';Switch.System.IO.BlockLongPaths='+block+'" /></runtime></configuration>');run=subprocess.run([str(exe),mode,str(fixture),str(w),sdk],capture_output=True,text=True);print(legacy,block,mode,run.stdout+run.stderr,flush=True);results.append(dict(legacy=legacy,block=block,mode=mode,code=run.returncode,result=run.stdout+run.stderr))
  if run.returncode:break
 if results[-1]['code']:break
(R/'Build/LongPathAudit/coexistence-results.json').write_text(json.dumps(dict(work=str(w),results=results,candidates={n:v['dllSha256'] for n,v in candidates.items()},noCallbacks=True,liveChanges=False),indent=2));assert len(results)==12 and all(x['code']==0 for x in results)
