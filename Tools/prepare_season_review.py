"""Extract private season fixtures into ignored Build; never touch source captures."""
from pathlib import Path
import hashlib,json,zipfile
ROOT=Path(__file__).resolve().parents[1]
source=ROOT/'Build/SeasonTest-OSU-2026';out=ROOT/'Build/RC016/Inputs'
out.mkdir(parents=True,exist_ok=True)
stages={'PreSeason':0,'RegularSeason':1,'BowlSeason1':2,'BowlSeason2':3,'BowlSeason3':4,'NationalChampionship':5,'OffSeason':6}
inputs=[]
for archive in source.glob('20*.zip'):
    year,phase,week=archive.stem.split('-');dest=out/(archive.stem+'.json')
    with zipfile.ZipFile(archive) as z:
        names=[n for n in z.namelist() if n.endswith('runtime-raw.json')]
        assert len(names)==1
        payload=z.read(names[0]);dest.write_bytes(payload)
    inputs.append(dict(path=str(dest),sha256=hashlib.sha256(payload).hexdigest(),order=(int(year),stages[phase],int(week))))
inputs.sort(key=lambda x:x['order'])
(out.parent/'inputs.json').write_text(json.dumps(dict(baseline='eb78401',inputs=inputs),indent=2),encoding='utf-8')
print(f'Prepared {len(inputs)} frozen input captures')
