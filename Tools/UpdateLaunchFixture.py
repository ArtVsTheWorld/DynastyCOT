from pathlib import Path
import json,hashlib
root=Path(r'C:\Apps\CFB 27\MMC\DynastyCOT');p=root/'Modules/ExtraDynastySettings/Tools/LaunchChecks.cs';s=p.read_text()
s=s.replace('Check(failed,"Locked preference silently acknowledged"); persistence.Poll();','Check(!failed,"Obsolete global monitor must not access or acknowledge locked global preferences"); persistence.Poll();')
s=s.replace('File.ReadAllText(ini).Trim()=="ExtendedSeconds=60","Preference save did not retry after lock released"','File.ReadAllText(ini).Trim()=="ExtendedSeconds=0","Obsolete global monitor changed the preference"')
s=s.replace('Check(failed,"Failed remote read did not reach retry logic");','Check(!failed,"Detached clock persistence must not leave a second global monitor reading remote state");')
s=s.replace('Check(retried && File.ReadAllText(storage).Contains','Check(!retried && File.ReadAllText(storage).Contains')
s=s.replace('"Clock save failure prevented defense saving"','"Detached clock ownership must not prevent playcall persistence"')
p.write_text(s,encoding='utf-8',newline='\n')
m=root/'Baselines/changes.json';doc=json.loads(m.read_text());rel=p.relative_to(root).as_posix()
doc['changes']=[x for x in doc['changes'] if x['path']!=rel];doc['changes'].append(dict(path=rel,sha256=hashlib.sha256(p.read_bytes()).hexdigest(),reason='Verify detached persistence ownership and absence of global clock writes'))
m.write_text(json.dumps(doc,indent=2)+'\n')
