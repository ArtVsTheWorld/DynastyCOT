"""Private release-preparation artifacts only. Never installs, injects or publishes."""
from pathlib import Path
import hashlib,json,subprocess,zipfile,shutil
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'Release/FinalReadiness-Private';OUT.mkdir(exist_ok=True)
NAMES=['BeyondTheField','GamedayAtmosphere','ExtraSettings','ExpandedPractice']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip();results=[]
 for name in NAMES:
  meta=json.loads((ROOT/'Build'/('current-gameday-standalone.json' if name=='GamedayAtmosphere' else 'current-standalone-'+name+'.json')).read_text());docs=ROOT/'Packaging/Standalone'/name
  assert sha(Path(meta['dll']))==meta['dllSha256']
  for rel,digest in meta['sourceHashes'].items():assert sha(ROOT/rel)==digest,'Source no longer matches compiled candidate: '+rel
  archive=OUT/(name+'-'+meta['version']+'.zip');files={name+'.dll':Path(meta['dll']),**{n:docs/n for n in ['README.txt','FEATURES.md']}}
  if (docs/'LICENSES.txt').exists():files['LICENSES.txt']=docs/'LICENSES.txt'
  with zipfile.ZipFile(archive,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=9) as z:
   for leaf,p in files.items():
    entry=zipfile.ZipInfo(leaf,(2020,1,1,0,0,0));entry.compress_type=zipfile.ZIP_DEFLATED;entry.external_attr=0o644<<16;z.writestr(entry,p.read_bytes())
  with zipfile.ZipFile(archive) as z:
   assert z.testzip() is None and set(z.namelist())==set(files)
   for leaf,p in files.items():assert hashlib.sha256(z.read(leaf)).hexdigest()==sha(p)
  old=json.loads(next((ROOT/'Build/FinalReleasePass/PublicBaselines'/name).glob('*-verification.json')).read_text())
  priorHashes=old.get('sourceHashes',{});changed=[p for p,h in meta['sourceHashes'].items() if priorHashes.get(p)!=h];removed=[p for p in priorHashes if p not in meta['sourceHashes']]
  proof={k:meta[k] for k in ['name','module','version','dllSha256','sourceHashes','payload','resources','supportFiles','nativeSourceBuilt','nativeSha256','nativeTests','branding'] if k in meta}
  proof.update(format=1,name=name,archive=archive.name,archiveSha256=sha(archive),archiveBytes=archive.stat().st_size,sourceRevision=revision,privateCandidate=True,installed=False,published=False,files=[dict(path=n,sha256=sha(p),bytes=p.stat().st_size) for n,p in files.items()],comparison=dict(publicVersion=old['version'],publicSourceRevision=old.get('sourceRevision'),publicArchiveSha256=old['archiveSha256'],changedOrAddedSourceFiles=changed,removedOrNoLongerTrackedSourceFiles=removed),acceptance='Automated verification is separate from installed-menu, crowd visual, live performance and audible listening acceptance')
  (OUT/(archive.stem+'-verification.json')).write_text(json.dumps(proof,indent=2)+'\n',encoding='utf8')
  (OUT/(archive.stem+'-SHA256.txt')).write_text(sha(archive)+'  '+archive.name+'\n',encoding='utf8')
  for n in ['RELEASE_NOTES.md','DISCORD.txt']:shutil.copyfile(docs/n,OUT/(name+'-'+n))
  results.append(dict(module=name,version=meta['version'],archive=archive.name,archiveSha256=sha(archive),dllSha256=meta['dllSha256'],sourceRevision=revision,publicBaseline=old['version'],changedSourceFiles=len(changed)))
 (OUT/'modules.json').write_text(json.dumps(results,indent=2)+'\n',encoding='utf8');print(json.dumps(results,indent=2))
if __name__=='__main__':main()
