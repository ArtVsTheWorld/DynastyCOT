"""Prepare exact public release assets locally. Never installs or publishes."""
from pathlib import Path
import hashlib,json,zipfile,subprocess,shutil
R=Path(__file__).resolve().parents[1];O=R/'Release/Publication-Final';E=R/'Build/PublicationFinal'
NAMES=['BeyondTheField','GamedayAtmosphere','ExtraSettings','ExpandedPractice']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def put(p,v):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf8')
def main():
 O.mkdir(exist_ok=True);E.mkdir(exist_ok=True);catalog=json.loads((R/'Build/FinalReadiness/catalog.json').read_text());catalog['Version']='1.1.5';rows=[]
 revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=R,text=True).strip()
 for name in NAMES:
  m=json.loads((R/'Build'/('current-gameday-standalone.json' if name=='GamedayAtmosphere' else f'current-standalone-{name}.json')).read_text());docs=R/'Packaging/Standalone'/name
  for p,h in m['sourceHashes'].items():assert sha(R/p)==h,p
  assert sha(Path(m['dll']))==m['dllSha256'];guide=R/f'docs/player-guides/{name}-Player-Guide.pdf';source=guide.with_suffix('.md')
  files={name+'.dll':Path(m['dll']),'README.txt':docs/'README.txt','FEATURES.md':docs/'FEATURES.md',guide.name:guide}
  if name=='BeyondTheField':
   notices=(docs/'LICENSES.txt').read_text();assert notices in (docs/'README.txt').read_text();assert 'SDK, Node.js,' not in notices
  archive=O/f"{name}-{m['version']}.zip"
  with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
   for leaf,p in files.items():
    e=zipfile.ZipInfo(leaf,(2020,1,1,0,0,0));e.compress_type=zipfile.ZIP_DEFLATED;e.external_attr=0o644<<16;z.writestr(e,p.read_bytes())
  with zipfile.ZipFile(archive) as z:assert len(z.namelist())==4 and z.testzip() is None
  previous=json.loads(next((R/'Build/FinalReleasePass/PublicBaselines'/name).glob('*-verification.json')).read_text())
  proof=dict(format=2,name=name,version=m['version'],archiveSha256=sha(archive),dllSha256=m['dllSha256'],playerGuideSha256=sha(guide),sourceRevision=revision,sourceHashes=m['sourceHashes'],payload=m.get('payload',{}),supportFiles=m.get('supportFiles',{}),nativeSha256=m.get('nativeSha256'),nativeTests=m.get('nativeTests'),resources=m.get('resources',[]),files=[dict(path=k,sha256=sha(p),bytes=p.stat().st_size) for k,p in files.items()],publicBaseline=dict(version=previous['version'],sourceRevision=previous.get('sourceRevision'),archiveSha256=previous['archiveSha256']),liveAcceptance='Offline verification is separate from installed-menu, visual crowd, live frame-time and audible acceptance. Prior LSU-Ole Miss quieting listening remains confirmed; spring boos and new layout/gate acceptance remain pending.',installedIntoLiveMmc=False)
  put(O/(archive.stem+'-verification.json'),proof)
  (O/(archive.stem+'-SHA256.txt')).write_text(sha(archive)+'  '+archive.name+'\n'+sha(guide)+'  '+guide.name+'\n',encoding='ascii')
  shutil.copyfile(guide,O/guide.name);shutil.copyfile(source,O/source.name);shutil.copyfile(docs/'RELEASE_NOTES.md',O/f'{name}-RELEASE_NOTES.md')
  module=next(x for x in catalog['Modules'] if x['Id']==name);current=module['Current'];base='https://github.com/'+module['Repository']+'/releases/download/v'+m['version']+'/'
  current.update(Version=m['version'],Tag='v'+m['version'],ArchiveName=archive.name,ArchiveUrl=base+archive.name,ArchiveSha256=sha(archive),DllSha256=m['dllSha256'],ArchiveBytes=archive.stat().st_size,ProofUrl=base+archive.stem+'-verification.json',PageUrl='https://github.com/'+module['Repository']+'/releases/tag/v'+m['version'],Prerelease=False,Notes=(docs/'RELEASE_NOTES.md').read_text(),Compatibility='Use one provider of each module. Offline scopes, exact hook checks and shared AppData remain. Read the included player guide and release notes for supported modes and live acceptance limits.')
  catalog['KnownPlugins'].append(dict(Path='Plugins/'+name+'.dll',Module=name,Sha256=m['dllSha256']))
  rows.append(dict(module=name,version=m['version'],archive=archive.name,archiveSha256=sha(archive),dllSha256=m['dllSha256'],guideSha256=sha(guide),repository=module['Repository'],publicBaseline=previous['version']))
 catalog['KnownPlugins']=list({(x['Path'],x['Sha256']):x for x in catalog['KnownPlugins']}.values())
 put(R/'Packaging/Collection/catalog.json',catalog);put(E/'catalog.json',catalog);put(O/'modules.json',rows);print('Four-file release assets:',O)
if __name__=='__main__':main()
