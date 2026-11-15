"""Long-path regression across .NET host modes. Physical, marked fixtures only."""
from pathlib import Path
import subprocess,json,datetime,os,hashlib,_winapi
R=Path(__file__).resolve().parents[1];W=R/'Build'/('LongPathChecks-'+datetime.datetime.now().strftime('%H%M%S'));W.mkdir();exe=W/'UserStorageLongPathChecks.exe';compiler=Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe')
subprocess.run([str(compiler),'/nologo','/platform:x64','/warnaserror+','/out:'+str(exe),str(R/'Core/UserStorage.cs'),str(R/'Tests/UserStorageLongPathChecks.cs')],check=True)
results=[]
for legacy in ['false','true']:
 for block in ['false','true']:
  fixture=W/(legacy+'-'+block);fixture.mkdir();(fixture/'.cot-fixture').write_text('isolated');outside=W/('Outside-'+legacy+'-'+block);outside.mkdir();(outside/'file.json').write_text('unrelated outside target');link=fixture/'Junction';subprocess.run(['cmd.exe','/d','/c','mklink','/J',str(link),str(outside)],check=True,capture_output=True)
  longParent=fixture/('l'*150)/('k'*150);os.makedirs('\\\\?\\'+str(longParent));longLink=longParent/'LongJunction';_winapi.CreateJunction(str(outside),'\\\\?\\'+str(longLink))
  Path(str(exe)+'.config').write_text('<configuration><runtime><AppContextSwitchOverrides value="Switch.System.IO.UseLegacyPathHandling='+legacy+';Switch.System.IO.BlockLongPaths='+block+'" /></runtime></configuration>')
  p=subprocess.run([str(exe),str(fixture),str(link),str(longLink)],capture_output=True,text=True);(W/(legacy+'-'+block+'.log')).write_text(p.stdout+p.stderr);print(p.stdout+p.stderr);assert p.returncode==0
  # Independent Python/Unicode-native verification, not the C# IO helper.
  sources=[]
  for base,ds,fs in os.walk('\\\\?\\'+str(fixture),followlinks=False):
   ds[:]=[d for d in ds if not os.path.isjunction(os.path.join(base,d))]
   for name in fs:
    path=Path(base)/name
    if name.startswith('snapshot-'):
     data=path.read_bytes();assert data==b'original published words';sources.append(dict(path=str(path),sha256=hashlib.sha256(data).hexdigest()))
  assert len(sources)==2
  os.rmdir(link);os.rmdir('\\\\?\\'+str(longLink));assert (outside/'file.json').read_text()=='unrelated outside target'
  results.append(dict(legacy=legacy,block=block,result=p.stdout.strip(),independentSnapshotChecks=sources))
(R/'Build/btf-longpath-verification.json').write_text(json.dumps(dict(work=str(W),results=results,sources={str(p.relative_to(R)):hashlib.sha256(p.read_bytes()).hexdigest() for p in [R/'Core/UserStorage.cs',R/'Tests/UserStorageLongPathChecks.cs',Path(__file__)]},liveChanges=False,hostConfigurationChanged=False,published=False),indent=2)+'\n')
