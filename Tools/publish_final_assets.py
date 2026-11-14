"""Reviewed publication workflow. Explicit phase flags; never installs game plugins."""
from pathlib import Path
import argparse,hashlib,json,subprocess
R=Path(__file__).resolve().parents[1];O=R/'Release/Publication-Final';E=R/'Build/PublicationFinal'
GH=Path(r'C:\Apps\CFB 27\Working\Teambuilder Import Plugin\PublishingTools\gh-2.103.3\bin\gh.exe')
PATHS={'BeyondTheField':'DynastyEnhancer','GamedayAtmosphere':'GamedayAtmosphere','ExtraSettings':'ExtraDynastySettings','ExpandedPractice':'DrivePractice'}
def run(args,cwd=R,check=True):
 p=subprocess.run(list(map(str,args)),cwd=cwd,capture_output=True,text=True,encoding='utf8');
 if check and p.returncode:raise RuntimeError(p.stdout+'\n'+p.stderr)
 return p
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def put(p,v):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf8')
def upload(repo,tag,title,notes,assets,target):
 existing=run([GH,'api',f'repos/{repo}/releases/tags/{tag}'],check=False)
 if existing.returncode==0:
  old=json.loads(existing.stdout);assert old['draft'],'Refusing to replace published release '+repo+' '+tag
 else:run([GH,'release','create',tag,'--repo',repo,'--draft','--target',target,'--title',title,'--notes-file',notes])
 run([GH,'release','upload',tag,'--repo',repo,*assets,'--clobber'])
 release=next(x for x in json.loads(run([GH,'api',f'repos/{repo}/releases?per_page=100']).stdout) if x['tag_name']==tag)
 for path in assets:
  a=next(x for x in release['assets'] if x['name']==path.name);assert a['size']==path.stat().st_size and a.get('digest')=='sha256:'+sha(path),path
 run([GH,'release','edit',tag,'--repo',repo,'--target',target,'--title',title,'--notes-file',notes,'--draft=false','--latest'])
 return f'https://github.com/{repo}/releases/tag/{tag}'
def main():
 p=argparse.ArgumentParser();p.add_argument('--prepare-repos',action='store_true');p.add_argument('--publish-modules',action='store_true');p.add_argument('--publish-collection',action='store_true');a=p.parse_args()
 rows=json.loads((O/'modules.json').read_text());revision=run(['git','rev-parse','HEAD']).stdout.strip();results=[]
 for m in rows:
  name=m['module'];docs=R/'Packaging/Standalone'/name;repo=m['repository'];version=m['version'];folder=E/'Repos'/name
  if a.prepare_repos:
   if not folder.exists():folder.parent.mkdir(exist_ok=True);run(['git','clone','--quiet','--single-branch','--branch','main','https://github.com/'+repo+'.git',folder])
   assert repo in run(['git','remote','get-url','origin'],cwd=folder).stdout
   assert not run(['git','status','--porcelain'],cwd=folder).stdout,'Review existing clone edits before replacing docs'
   run(['git','pull','--ff-only','origin','main'],cwd=folder)
   title={'BeyondTheField':'Beyond the Field','GamedayAtmosphere':'Gameday Atmosphere','ExtraSettings':'Extra Settings','ExpandedPractice':'Expanded Practice'}[name]
   base=f'https://github.com/{repo}/releases/download/v{version}/'
   text=f'''# Ace’s {title}

**[Download {version} ZIP]({base}{m['archive']})** · [Illustrated player guide]({base}{name}-Player-Guide.pdf) · [All features](FEATURES.md) · [What changed](RELEASES.md)

Close MMC/game. Copy **{name}.dll** from the ZIP into the **Plugins** folder beside the MMCModManager.exe you actually use, replacing the prior copy. Keep the included README, feature list and illustrated guide. The ZIP has exactly those four files; required notices are in the README appendix where needed. Matching support prepares automatically outside Plugins.

Your settings, reports, accounts and published history stay under `%LOCALAPPDATA%\\Ace\\DynastyCOT`. Use Win+R to open that folder. The guide names the exact module logs and settings menus. To roll back, restore the prior DLL with MMC/game closed, or use the collection installer's Backups. Keep user data.

**Want one feature? Download its standalone ZIP. Want several? [Use the Dynasty COT installer](https://github.com/ArtVsTheWorld/DynastyCOT/releases/tag/v1.1.5).** The installer handles recognized older copies transactionally. Keep one provider of each feature.

Read the release notes for supported contexts and remaining live acceptance. Offline checks are separate from actual menu, crowd visual, performance and audible results. Optional trusted-build Defender help is in the guide; security exclusions are not required for everyone and are never added automatically.

## Source and verification

Development stays in the [shared COT workspace](https://github.com/ArtVsTheWorld/DynastyCOT/tree/{revision}); [current module source](https://github.com/ArtVsTheWorld/DynastyCOT/tree/{revision}/Modules/{PATHS[name]}). The older tree in this download/support repository is historical and is not the current build input. `source-release.json` pins the source revision and artifact hashes. Verification JSON and SHA-256 files are separate release assets; the editable guide is also available.
'''
   (folder/'README.md').write_text(text,encoding='utf8');(folder/'FEATURES.md').write_bytes((docs/'FEATURES.md').read_bytes());old=(folder/'RELEASES.md').read_text() if (folder/'RELEASES.md').exists() else '';notes=(docs/'RELEASE_NOTES.md').read_text();(folder/'RELEASES.md').write_text(notes+'\n\n---\n\n'+old,encoding='utf8')
   put(folder/'source-release.json',dict(format=2,version=version,repository='ArtVsTheWorld/DynastyCOT',revision=revision,modulePath='Modules/'+PATHS[name],archive=m['archive'],archiveSha256=m['archiveSha256'],dllSha256=m['dllSha256'],playerGuideSha256=m['guideSha256'],fourFileZip=True,publicBaseline=m['publicBaseline']))
   run(['git','config','user.name',run(['git','config','user.name']).stdout.strip()],cwd=folder);run(['git','config','user.email',run(['git','config','user.email']).stdout.strip()],cwd=folder)
   run(['git','add','README.md','FEATURES.md','RELEASES.md','source-release.json'],cwd=folder);run(['git','commit','-m',f'Release {title} {version} with illustrated guide and COT source pin'],cwd=folder)
  if a.publish_modules:
   assert not run(['git','status','--porcelain'],cwd=folder).stdout
   proof=json.loads((O/(Path(m['archive']).stem+'-verification.json')).read_text());assert proof['sourceRevision']==revision and proof['archiveSha256']==sha(O/m['archive'])
   run(['git','push','origin','HEAD:main'],cwd=folder);target=run(['git','rev-parse','HEAD'],cwd=folder).stdout.strip()
   assets=[O/m['archive'],O/(Path(m['archive']).stem+'-verification.json'),O/(Path(m['archive']).stem+'-SHA256.txt'),O/(name+'-Player-Guide.pdf'),O/(name+'-Player-Guide.md')]
   url=upload(repo,'v'+version,'Ace’s '+name+' '+version,docs/'RELEASE_NOTES.md',assets,target);results.append(dict(module=name,url=url,sourceRevision=revision,distributionRevision=target,archiveSha256=m['archiveSha256']));print(url,flush=True)
 if a.publish_modules:put(E/'published-modules.json',results)
 if a.publish_collection:
  setup=json.loads((R/'Build/current-collection-setup.json').read_text());assert sha(Path(setup['exe']))==setup['sha256']
  assets=[O/'DynastyCOT-Installer-1.1.5.zip',O/'DynastyCOT-Installer-1.1.5-verification.json',O/'DynastyCOT-Installer-1.1.5-SHA256.txt',O/'DynastyCOT-Player-Guides-1.1.5.zip']
  url=upload('ArtVsTheWorld/DynastyCOT','v1.1.5','Dynasty COT 1.1.5 - Four standalone modules, one optional installer',O/'COLLECTION_RELEASE_NOTES.md',assets,revision);put(E/'published-collection.json',dict(url=url,sourceRevision=revision,installerSha256=setup['sha256']));print(url,flush=True)
if __name__=='__main__':main()
