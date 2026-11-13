"""Private source build from the frozen, reviewed BTF integration baseline."""
from pathlib import Path
import hashlib,json,sys
import build_standalones
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'Build/ReleaseReadiness-20261113'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 path=ROOT/'cot.build-inputs.json';inputs=json.loads(path.read_text(encoding='utf8'));review=[];built=json.loads((ROOT/"Build/current-standalone-BeyondTheField.json").read_text())
 for rel,digest in inputs['sourceHashes'].items():
  actual=sha(ROOT/rel)
  if actual==digest:continue
  frozen=OUT/'Before'/rel
  previous=json.loads((OUT/"reviewed-native-inputs.json").read_text()) if (OUT/"reviewed-native-inputs.json").exists() else []
  compiled=Path(built.get("work",""))/"Native"/rel
  assert (compiled.exists() and sha(compiled)==digest) or (frozen.exists() and sha(frozen)==digest) or any(x["file"]==rel and x["after"]==digest for x in previous) or (built.get("privateCandidate")=="1.0.17-scouting1" and built["sourceHashes"].get(rel)==digest),'Not part of the frozen native baseline: '+rel
  assert rel.startswith('Modules/DynastyEnhancer/Source/') or rel in ['Modules/DynastyEnhancer/Tests/RecruitInterestTests.cpp','Modules/DynastyEnhancer/Tests/ScoutingPolicyTests.cpp','Modules/DynastyEnhancer/Tests/ReleaseReadinessTests.cpp'],'Unexpected integration input change: '+rel
  review.append({'file':rel,'before':digest,'after':actual});inputs['sourceHashes'][rel]=actual
 for name in ['AthleticMeasurables.h','ScoutingEvidence.h','ScoutingTechniqueLibrary.h','NotesPresentation.h','ScoutingVariety.h']:
  rel='Modules/DynastyEnhancer/Source/'+name;inputs['sourceHashes'][rel]=sha(ROOT/rel)
 (OUT/'reviewed-native-inputs.json').write_text(json.dumps((json.loads((OUT/'reviewed-native-inputs.json').read_text()) if (OUT/'reviewed-native-inputs.json').exists() else [])+review,indent=2)+'\n')
 path.write_text(json.dumps(inputs,indent=2)+'\n',encoding='utf8')
 old=build_standalones.DEFS['BeyondTheField'];build_standalones.DEFS['BeyondTheField']=(*old[:2],'1.0.23',*old[3:])
 sys.argv=[sys.argv[0],'--mmc-sdk',str(ROOT.parent/'MMC_ModManager_v1.1.0.6'),'--module','BeyondTheField'];build_standalones.main()
 p=ROOT/'Build/current-standalone-BeyondTheField.json';m=json.loads(p.read_text());
 for rel in inputs['sourceHashes']:
  compiled=Path(m['work'])/'Native'/rel
  if compiled.exists() and rel.startswith('Modules/DynastyEnhancer/Source/') and not rel.endswith('RuntimeFeed.h'):assert sha(compiled)==sha(ROOT/rel),'Source changed during build: '+rel
 m['privateCandidate']='1.0.23-release1';m['installed']=m['published']=False;p.write_text(json.dumps(m,indent=2)+'\n')
if __name__=='__main__':main()
