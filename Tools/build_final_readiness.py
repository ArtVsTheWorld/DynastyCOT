"""Private source build; no install, game access or publication."""
from pathlib import Path
import argparse,hashlib,json,sys
import build_standalones
R=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--module',required=True,choices=build_standalones.DEFS);a=p.parse_args()
 if a.module=='BeyondTheField':
  path=R/'cot.build-inputs.json';inputs=json.loads(path.read_text());old=json.loads((R/'Build/current-standalone-BeyondTheField.json').read_text());review=[]
  for rel,h in inputs['sourceHashes'].items():
   now=sha(R/rel)
   if now==h:continue
   assert rel.startswith('Modules/DynastyEnhancer/Source/'),rel
   assert old['sourceHashes'].get(rel)==h,'Input not linked to previous source-built candidate: '+rel
   review.append(dict(file=rel,before=h,after=now));inputs['sourceHashes'][rel]=now
  for leaf in ['RuntimeDiagnostics.h']:
   rel='Modules/DynastyEnhancer/Source/'+leaf;inputs['sourceHashes'][rel]=sha(R/rel)
  path.write_text(json.dumps(inputs,indent=2)+'\n')
  (R/'Build/FinalReadiness/reviewed-btf-inputs.json').write_text(json.dumps(review,indent=2)+'\n')
 sys.argv=[sys.argv[0],'--mmc-sdk',str(R.parent/'MMC_ModManager_v1.1.0.6'),'--module',a.module];build_standalones.main()
 path=R/f'Build/current-standalone-{a.module}.json';m=json.loads(path.read_text());m.update(privateCandidate=m['version']+'-final1',installed=False,injected=False,published=False);path.write_text(json.dumps(m,indent=2)+'\n')
if __name__=='__main__':main()
