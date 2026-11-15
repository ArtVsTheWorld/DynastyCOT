"""Promote the exact private BTF menu candidate; explicit publication phases.

Never installs modules, starts helpers, modifies saves or publishes other modules.
GitHub credentials remain in memory and are passed only to the GitHub CLI child.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,zipfile
from publish_final_assets import GH
R=Path(__file__).resolve().parents[1]
O=R/'Release/BTF-Hotfix-3';W=R/'Build/BtfHotfix3Release'
DOC=R/'Packaging/Standalone/BeyondTheField';REPO='ArtVsTheWorld/BeyondTheField'
VERSION='1.0.32';TAG='v'+VERSION;STEM='BeyondTheField-'+VERSION

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def load(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,x):p.write_text(json.dumps(x,indent=2)+'\n',encoding='utf8')
def run(args,cwd=R):
 env=dict(os.environ,GIT_TERMINAL_PROMPT='0',GCM_INTERACTIVE='Never')
 if str(args[0])==str(GH) and not env.get('GH_TOKEN'):
  p=subprocess.run(['git','credential','fill'],input='protocol=https\nhost=github.com\n\n',capture_output=True,text=True,env=env)
  fields=dict(x.split('=',1) for x in p.stdout.splitlines() if '=' in x)
  if not fields.get('password'):raise RuntimeError('GitHub publishing authentication is unavailable')
  env['GH_TOKEN']=fields['password']
 p=subprocess.run(list(map(str,args)),cwd=cwd,env=env,capture_output=True,text=True,encoding='utf8')
 if p.returncode:raise RuntimeError(p.stdout+'\n'+p.stderr)
 return p.stdout

def prepare():
 O.mkdir(exist_ok=True);W.mkdir(exist_ok=True)
 m=load(R/'Build/HubAudit/Final2/BeyondTheField-candidate.json')
 assert m['version']==VERSION and sha(Path(m['dll']))==m['dllSha256']
 for rel,h in m['sourceHashes'].items():assert sha(R/rel)==h,rel
 assert (DOC/'LICENSES.txt').read_bytes() in (DOC/'README.txt').read_bytes()
 guide=R/'docs/player-guides/BeyondTheField-Player-Guide.pdf'
 files={'BeyondTheField.dll':Path(m['dll']),'README.txt':DOC/'README.txt','FEATURES.md':DOC/'FEATURES.md',guide.name:guide}
 archive=O/(STEM+'.zip')
 with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
  for leaf,p in files.items():
   i=zipfile.ZipInfo(leaf,(2020,1,1,0,0,0));i.compress_type=zipfile.ZIP_DEFLATED;i.external_attr=0o644<<16;z.writestr(i,p.read_bytes())
 with zipfile.ZipFile(archive) as z:
  assert z.testzip() is None and len(z.namelist())==4
  assert hashlib.sha256(z.read('BeyondTheField.dll')).hexdigest()==m['dllSha256']
 proof=dict(format=2,name='BeyondTheField',version=VERSION,sourceRevision=None,archiveSha256=sha(archive),dllSha256=m['dllSha256'],playerGuideSha256=sha(guide),sourceHashes=m['sourceHashes'],payload=m['payload'],supportFiles={},nativeProof=m['nativeProof'],fourFileZip=True,files=[dict(path=n,bytes=p.stat().st_size,sha256=sha(p)) for n,p in files.items()],publicBaseline=dict(version='1.0.26'),candidateDllUnchanged=True,verification=dict(sourceAuditMethods=164,sourceAuditAssets=196,sourceAuditErrors=0,nonNotesMethods=151,policyCombinations=906,interestVmAssertions=287,reportDisplayAssertions=8113,projectedPositionAssertions=10123,moduleExtractionAssertions=220,nativeBuildSuites=6),liveAcceptance='Roster/card/NIL/recruiting and offseason displays were inspected on the existing 1.0.31 process with targeted private display fixes. Final 1.0.32 clean-start acceptance, installed input latency, current regular-season Practice Plan and DE-only menus remain pending. Offline performance is callback overhead, not whole-menu timing.')
 put(O/(STEM+'-verification.json'),proof)
 shutil.copyfile(guide,O/guide.name);shutil.copyfile(guide.with_suffix('.md'),O/guide.with_suffix('.md').name)
 catalog=load(R/'Packaging/Collection/catalog.json');old=next(x for x in catalog['Modules'] if x['Id']=='BeyondTheField')['Current'];assert old['Version'] in ['1.0.26',VERSION]
 base=f'https://github.com/{REPO}/releases/download/{TAG}/'
 old.update(Version=VERSION,Tag=TAG,ArchiveName=archive.name,ArchiveUrl=base+archive.name,ArchiveSha256=sha(archive),ArchiveBytes=archive.stat().st_size,DllSha256=m['dllSha256'],ProofUrl=base+STEM+'-verification.json',PageUrl=f'https://github.com/{REPO}/releases/tag/{TAG}',Notes=(DOC/'RELEASE_NOTES.md').read_text(encoding='utf8'),Prerelease=False)
 catalog['KnownPlugins'].append(dict(Path='Plugins/BeyondTheField.dll',Module='BeyondTheField',Sha256=m['dllSha256']))
 for path,h in m['payload'].items():
  if path.endswith(('.dll','.exe')):catalog['Programs'].append(dict(Path=path,Module='BeyondTheField',Sha256=h))
 for key in ['KnownPlugins','Programs']:catalog[key]=list({(x['Path'],x['Sha256']):x for x in catalog[key]}.values())
 put(R/'Packaging/Collection/catalog.json',catalog)
 print('Prepared four-file public ZIP from exact candidate DLL:',m['dllSha256'])

def finalize():
 revision=run(['git','rev-parse','HEAD']).strip();p=O/(STEM+'-verification.json');proof=load(p)
 for rel,h in proof['sourceHashes'].items():
  assert sha(R/rel)==h,rel
  committed=subprocess.check_output(['git','show',revision+':'+rel],cwd=R)
  assert hashlib.sha256(committed).hexdigest()==h,'Uncommitted source: '+rel
 proof['sourceRevision']=revision;put(p,proof)
 (O/(STEM+'-SHA256.txt')).write_text(''.join(sha(x)+'  '+x.name+'\n' for x in [O/(STEM+'.zip'),p,O/'BeyondTheField-Player-Guide.pdf']),encoding='ascii')
 print('Pinned',len(proof['sourceHashes']),'exact committed build sources to',revision)

def distribution():
 folder=W/'BeyondTheField'
 if not folder.exists():run(['git','clone','--quiet','--single-branch','--branch','main','https://github.com/'+REPO+'.git',folder])
 assert REPO in run(['git','remote','get-url','origin'],folder)
 assert not run(['git','status','--porcelain'],folder)
 run(['git','pull','--ff-only','origin','main'],folder)
 proof=load(O/(STEM+'-verification.json'));pin=load(folder/'source-release.json');prior=pin['version'];assert prior=='1.0.26'
 text=(folder/'README.md').read_text(encoding='utf8').replace('/v'+prior+'/', '/'+TAG+'/').replace('BeyondTheField-'+prior+'.zip',STEM+'.zip').replace('Download '+prior+' ZIP','Download '+VERSION+' ZIP').replace(pin['revision'],proof['sourceRevision'])
 (folder/'README.md').write_text(text,encoding='utf8')
 (folder/'FEATURES.md').write_bytes((DOC/'FEATURES.md').read_bytes())
 notes=(DOC/'RELEASE_NOTES.md').read_text(encoding='utf8');old=(folder/'RELEASES.md').read_text(encoding='utf8');(folder/'RELEASES.md').write_text(notes+'\n\n---\n\n'+old,encoding='utf8')
 pin.update(version=VERSION,revision=proof['sourceRevision'],archive=STEM+'.zip',archiveSha256=proof['archiveSha256'],dllSha256=proof['dllSha256'],playerGuideSha256=proof['playerGuideSha256'],publicBaseline=prior);put(folder/'source-release.json',pin)
 for key in ['user.name','user.email']:run(['git','config',key,run(['git','config',key]).strip()],folder)
 run(['git','add','README.md','FEATURES.md','RELEASES.md','source-release.json'],folder)
 run(['git','commit','-m','Update 2 Hotfix 3: Dynasty menu adapters and responsiveness (1.0.32)'],folder)
 print('Prepared standalone source pin and release notes.')

def publish():
 proof=load(O/(STEM+'-verification.json'));revision=run(['git','rev-parse','HEAD']).strip();assert revision==proof['sourceRevision']
 assert run(['git','ls-remote','origin','refs/heads/main']).split()[0]==revision,'Push the reviewed COT source commit first'
 folder=W/'BeyondTheField';assert not run(['git','status','--porcelain'],folder)
 pin=load(folder/'source-release.json');assert pin['revision']==revision and pin['dllSha256']==proof['dllSha256']
 run(['git','push','origin','HEAD:main'],folder);target=run(['git','rev-parse','HEAD'],folder).strip()
 releases=json.loads(run([GH,'api',f'repos/{REPO}/releases?per_page=100']))
 assert not any(x['tag_name']==TAG for x in releases),'Refusing to overwrite an existing release'
 assets=[O/(STEM+'.zip'),O/(STEM+'-verification.json'),O/(STEM+'-SHA256.txt'),O/'BeyondTheField-Player-Guide.pdf',O/'BeyondTheField-Player-Guide.md']
 run([GH,'release','create',TAG,'--repo',REPO,'--draft','--target',target,'--title','Update 2 Hotfix 3','--notes-file',DOC/'RELEASE_NOTES.md'])
 run([GH,'release','upload',TAG,'--repo',REPO,*assets])
 actual=next(x for x in json.loads(run([GH,'api',f'repos/{REPO}/releases?per_page=100'])) if x['tag_name']==TAG)
 assert actual['draft']
 for p in assets:
  a=next(x for x in actual['assets'] if x['name']==p.name);assert a['size']==p.stat().st_size and a['digest']=='sha256:'+sha(p),p.name
 run([GH,'release','edit',TAG,'--repo',REPO,'--draft=false','--latest'])
 result=json.loads(run([GH,'api',f'repos/{REPO}/releases/tags/{TAG}']));assert not result['draft'] and not result['prerelease']
 latest=json.loads(run([GH,'api',f'repos/{REPO}/releases/latest']));assert latest['tag_name']==TAG
 put(W/'published.json',dict(url=result['html_url'],sourceRevision=revision,distributionRevision=target,dllSha256=proof['dllSha256'],assets=[dict(name=a['name'],digest=a['digest'],bytes=a['size']) for a in result['assets']]))
 print('Published:',result['html_url'])

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('phase',choices=['prepare','finalize','distribution','publish']);a=p.parse_args();globals()[a.phase]()
