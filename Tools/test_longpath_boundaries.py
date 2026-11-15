from pathlib import Path
import subprocess,datetime,json,os
r=Path.cwd();w=r/'Build/LongPathAudit'/('Managed-'+datetime.datetime.now().strftime('%H%M%S'));w.mkdir();exe=w/'LongPathBoundaryChecks.exe';subprocess.run([r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe','/nologo','/platform:x64','/warnaserror+','/out:'+str(exe),str(r/'Core/UserStorage.cs'),str(r/'Tests/LongPathBoundaryChecks.cs')],check=True);result=[]
for legacy in ['true','false']:
 for block in ['true','false']:
  f=w/(legacy+'-'+block);f.mkdir();(f/'.cot-fixture').write_text('isolated');Path(str(exe)+'.config').write_text('<configuration><runtime><AppContextSwitchOverrides value="Switch.System.IO.UseLegacyPathHandling='+legacy+';Switch.System.IO.BlockLongPaths='+block+'" /></runtime></configuration>');p=subprocess.run([str(exe),str(f),str(exe)],capture_output=True,text=True);print(p.stdout+p.stderr);result.append(dict(legacy=legacy,block=block,code=p.returncode,result=p.stdout+p.stderr))
(r/'Build/LongPathAudit/boundary-results.json').write_text(json.dumps(dict(work=str(w),results=result),indent=2));assert all(x['code']==0 for x in result)
