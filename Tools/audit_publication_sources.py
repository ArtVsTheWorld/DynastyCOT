"""Verify public build provenance without reading user histories or installed plugins."""
from pathlib import Path
import hashlib,json,subprocess
R=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 tracked=set(subprocess.check_output(['git','ls-files'],cwd=R,text=True).splitlines());missing=[];count=0
 for name in ['BeyondTheField','GamedayAtmosphere','ExtraSettings','ExpandedPractice']:
  m=json.loads((R/'Build'/('current-gameday-standalone.json' if name=='GamedayAtmosphere' else f'current-standalone-{name}.json')).read_text())
  assert sha(Path(m['dll']))==m['dllSha256']
  for p,h in m['sourceHashes'].items():
   assert sha(R/p)==h,'Source drift: '+p
   # Local build-input manifest pins external SDK/native adapter inputs. Its
   # digest is evidence; publishing the private paths/captures is not allowed.
   if p not in tracked and p != 'cot.build-inputs.json':missing.append(p)
   count+=1
 assert not missing,'Build inputs missing from reviewed Git index: '+str(sorted(set(missing)))
 staged=subprocess.check_output(['git','diff','--cached','--name-only'],cwd=R,text=True).splitlines()
 allowed_images={f'docs/player-guides/assets/{n}.png' for n in ['BeyondTheField','GamedayAtmosphere','ExtraSettings','ExpandedPractice']}
 for p in staged:
  assert not any(x in p.lower().split('/') for x in ['build','release','saves','captures','research','runtimehistory','recruitresumehistory']),p
  assert Path(p).suffix.lower() not in ['.dll','.exe','.sav','.fbmod','.fbproject','.ast','.raw','.gz','.pdf'],p
  if Path(p).suffix.lower() in ['.png','.jpg','.jpeg']:assert p in allowed_images,p
 print('PASS',count,'source hashes; public sources tracked; private local manifest hash only; reviewed staged files:',len(staged))
if __name__=='__main__':main()
