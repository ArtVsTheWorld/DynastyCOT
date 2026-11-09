"""Run the offline APT/ABI fixtures; no live patch helpers are invoked."""
from pathlib import Path
import json,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[1]
module=ROOT/'Modules/DynastyEnhancer';out=ROOT/'Build/RC016/UiTests';out.mkdir(parents=True,exist_ok=True)
results=[]
for test in sorted((module/'Tests').glob('*_test.py')):
    started=time.perf_counter()
    with (out/(test.stem+'.log')).open('w',encoding='utf-8') as log:
        p=subprocess.run([sys.executable,str(test)],cwd=module,stdout=log,stderr=subprocess.STDOUT)
    results.append(dict(test=test.name,exitCode=p.returncode,seconds=time.perf_counter()-started))
    print(('PASS ' if p.returncode==0 else 'FAIL ')+test.name,flush=True)
(out/'results.json').write_text(json.dumps(results,indent=2),encoding='utf-8')
sys.exit(any(r['exitCode'] for r in results))
