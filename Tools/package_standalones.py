"""Package exact tested standalone DLLs and their two user documents."""
from pathlib import Path
import hashlib,json,subprocess,zipfile
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 report=json.loads((ROOT/'Build/standalones-verification.json').read_text(encoding='utf-8'));release=ROOT/'Release';revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
 for name,tested in report['modules'].items():
  meta=json.loads((ROOT/f'Build/current-standalone-{name}.json').read_text(encoding='utf-8'));stage=Path(meta['stage']);names=[name+'.dll','README.txt','FEATURES.md']
  if meta['dllSha256']!=tested['dllSha256'] or meta['sourceHashes']!=tested['sourceHashes']:raise ValueError('Tests differ from build')
  for relative,digest in meta['sourceHashes'].items():
   if sha(ROOT/relative)!=digest:raise ValueError('Source changed: '+relative)
  if sorted(x.name for x in stage.iterdir())!=sorted(names):raise ValueError('Unexpected standalone entry')
  for file in names[1:]:
   if sha(stage/file)!=meta['sourceHashes'][f'Packaging/Standalone/{name}/{file}']:raise ValueError('Document differs')
  if sha(stage/names[0])!=meta['dllSha256']:raise ValueError('DLL differs')
  archive=release/f'{name}-{meta["version"]}.zip'
  with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
   for file in names:z.write(stage/file,file)
  with zipfile.ZipFile(archive) as z:
   if z.namelist()!=names or z.testzip():raise ValueError('Archive verification failed')
   for file in names:
    if z.read(file)!=(stage/file).read_bytes():raise ValueError('Archive bytes differ')
  evidence=dict(format=1,name=name,version=meta['version'],sourceRepository='https://github.com/ArtVsTheWorld/DynastyCOT',sourceRevision=revision,archiveSha256=sha(archive),dllSha256=meta['dllSha256'],sourceHashes=meta['sourceHashes'],payload=meta['payload'],nativeBranding=meta['branding'],integrationChecks=report['checks'],installedIntoLiveMmc=False,liveAcceptance=report.get('liveAcceptance','pending-new-standalone-wrapper'),files=names)
  (release/(archive.stem+'-verification.json')).write_text(json.dumps(evidence,indent=2)+'\n',encoding='utf-8')
  (release/(archive.stem+'-SHA256.txt')).write_text(sha(archive)+'  '+archive.name+'\n',encoding='utf-8')
  print('Verified three-file ZIP:',archive.name)
if __name__=='__main__':main()
