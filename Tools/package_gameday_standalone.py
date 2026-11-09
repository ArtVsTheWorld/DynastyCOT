"""Create the three-file private test ZIP only after exact-build verification."""
from pathlib import Path
import hashlib,json,subprocess,zipfile
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 meta=json.loads((ROOT/'Build/current-gameday-standalone.json').read_text(encoding='utf-8'))
 report=json.loads((ROOT/'Build/gameday-standalone-verification.json').read_text(encoding='utf-8'))
 if report['version']!=meta['version'] or report['dllSha256']!=meta['dllSha256'] or report['sourceHashes']!=meta['sourceHashes']:raise ValueError('Tests do not match current build')
 for relative,digest in meta['sourceHashes'].items():
  if sha(ROOT/relative)!=digest:raise ValueError('Source changed after tests: '+relative)
 stage=Path(meta['stage']);names=['GamedayAtmosphere.dll','README.txt','FEATURES.md']
 if sorted(x.name for x in stage.iterdir())!=sorted(names):raise ValueError('Standalone must contain exactly the DLL and two documents')
 if sha(stage/names[0])!=meta['dllSha256']:raise ValueError('DLL changed after tests')
 for name in names[1:]:
  if sha(stage/name)!=meta['sourceHashes']['Packaging/Standalone/GamedayAtmosphere/'+name]:raise ValueError('Packaged document differs from audited source')
 release=ROOT/'Release';release.mkdir(exist_ok=True);archive=release/('GamedayAtmosphere-'+meta['version']+'.zip')
 with zipfile.ZipFile(archive,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=9) as target:
  for name in names:target.write(stage/name,name)
 with zipfile.ZipFile(archive) as target:
  if target.namelist()!=names or target.testzip():raise ValueError('ZIP entry/integrity failure')
  for name in names:
   if target.read(name)!=(stage/name).read_bytes():raise ValueError('ZIP payload mismatch')
 evidence=dict(report,archive=archive.name,archiveSha256=sha(archive),files=[dict(path=name,sha256=sha(stage/name),bytes=(stage/name).stat().st_size) for name in names],sourceRevision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),published=False)
 (release/(archive.stem+'-verification.json')).write_text(json.dumps(evidence,indent=2)+'\n',encoding='utf-8')
 (release/(archive.stem+'-SHA256.txt')).write_text(sha(archive)+'  '+archive.name+'\n',encoding='utf-8')
 print('Packaged private test ZIP:',archive)
 print('SHA256:',sha(archive))
if __name__=='__main__':main()
