"""Compare identical seating inputs against the captured pre-edit source."""
from pathlib import Path
import hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[1]
def main():
 baseline=Path((ROOT/'Build/current-gameday-perf-baseline.txt').read_text());work=baseline/'Benchmark';work.mkdir(exist_ok=True);src=ROOT/'Modules/GamedayAtmosphere/Source'
 proof=json.loads((baseline/'baseline.json').read_text())
 for file,digest in proof['sourceHashes'].items():assert hashlib.sha256((baseline/file).read_bytes()).hexdigest()==digest
 s=(baseline/'SeatPattern.h').read_text().replace('namespace attendance::seats {','namespace attendance::baselineSeats {\nusing attendance::seats::random;')
 for name in ['SeatHash.h','CrowdVisibility.h']:s=s.replace('"'+name+'"','"'+(src/name).as_posix()+'"')
 (work/'LegacySeatPattern.h').write_text(s)
 s=(baseline/'VisitorSeating.h').read_text().replace('namespace attendance::visitors::seating {','namespace attendance::visitors::legacySeating {')
 for name in ['SeatPattern.h','CrowdAgents.h']:s=s.replace('"'+name+'"','"'+(src/name).as_posix()+'"')
 (work/'LegacyVisitorSeating.h').write_text(s)
 vs=Path(subprocess.check_output([r'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe','-latest','-products','*','-requires','Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-property','installationPath'],text=True).strip());tool=sorted((vs/'VC/Tools/MSVC').iterdir())[-1];kits=Path(r'C:\Program Files (x86)\Windows Kits\10');win=sorted(p for p in (kits/'Include').iterdir() if (p/'ucrt').is_dir() and (kits/'Lib'/p.name/'um/x64').is_dir())[-1]
 exe=work/'GamedayPerformanceTests.exe';test=ROOT/'Modules/GamedayAtmosphere/Tests/GamedayPerformanceTests.cpp'
 cmd=[tool/'bin/Hostx64/x64/cl.exe','/nologo','/O2','/MT','/W4','/WX','/EHsc','/std:c++17','/utf-8',*(f'/I{x}' for x in [tool/'include',win/'ucrt',win/'shared',win/'um',work]),test,'/Fe:'+str(exe),'/link',*(f'/LIBPATH:{x}' for x in [tool/'lib/x64',kits/'Lib'/win.name/'ucrt/x64',kits/'Lib'/win.name/'um/x64'])]
 result=subprocess.run([str(x) for x in cmd],cwd=work,capture_output=True,text=True);(work/'compile.log').write_text(result.stdout+result.stderr);assert result.returncode==0,result.stdout+result.stderr
 result=subprocess.run([str(exe)],cwd=work,capture_output=True,text=True);(work/'results.log').write_text(result.stdout+result.stderr);assert result.returncode==0,result.stdout+result.stderr
 rows=[]
 for line in result.stdout.splitlines():
  if line.startswith('BENCH|'):
   _,name,before,after,reduction=line.split('|');rows.append(dict(name=name,beforeMs=float(before),afterMs=float(after),reductionPercent=float(reduction)))
 evidence=dict(baseline=str(baseline),baselineSourceHashes=proof['sourceHashes'],sourceHashes={str((src/x).relative_to(ROOT)).replace('\\','/'):hashlib.sha256((src/x).read_bytes()).hexdigest() for x in ['SeatPattern.h','VisitorSeating.h']},measurements=rows,result=result.stdout,gameAccess=False,liveFpsMeasured=False)
 (ROOT/'Build/gameday-performance-verification.json').write_text(json.dumps(evidence,indent=2)+'\n');print(result.stdout)
if __name__=='__main__':main()
