"""Publish verified distribution front doors; development stays in DynastyCOT.

Requires explicit --publish. Does not rebuild, install, change saves or force-push.
"""
from pathlib import Path
import argparse,hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[1]
GH=Path(r'C:\Apps\CFB 27\Working\Teambuilder Import Plugin\PublishingTools\gh-2.103.3\bin\gh.exe')
MODULES={
 'BeyondTheField':dict(repo='BeyondTheField',local='Dynasty-Enhancer',title='Beyond the Field',version='1.0.2',baseline=None,preview=True,path='DynastyEnhancer',summary='Dynasty Social Media, scouting fog, clearer ratings/money labels and team Player Analytics.'),
 'GamedayAtmosphere':dict(repo='DynamicGamedayAtmosphere',local='DynamicGamedayAtmosphere',title='Gameday Atmosphere',version='2.1.9',baseline='v2.0.35',preview=False,path='GamedayAtmosphere',summary='Crowd arrivals and exits, traveling supporters, reactive sound and camera shake.'),
 'ExtraSettings':dict(repo='offline-dynasty-settings',local='offline-dynasty-settings',title='Extra Settings',version='0.2.10',baseline='v0.2.1',preview=True,path='ExtraDynastySettings',summary='Separate offensive/defensive playcall controls and clocks saved for each Dynasty.'),
 'ExpandedPractice':dict(repo='drive-based-practice',local='drive-based-practice',title='Expanded Practice',version='0.8.21',baseline='v0.8.18',preview=True,path='DrivePractice',summary='Default Practice, full drives and scrimmages in offline Dynasty or main-menu Practice.'),
}
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def run(command,cwd=ROOT,check=True):
 result=subprocess.run([str(x) for x in command],cwd=cwd,capture_output=True,text=True,encoding='utf-8')
 if check and result.returncode:raise RuntimeError(result.stdout+'\n'+result.stderr)
 return result
def main():
 p=argparse.ArgumentParser();p.add_argument('--publish',action='store_true');p.add_argument('--module',choices=MODULES,action='append');a=p.parse_args()
 revision=run(['git','rev-parse','HEAD']).stdout.strip()
 if run(['git','status','--porcelain','--untracked-files=normal']).stdout.strip():raise ValueError('Commit reviewed integration sources before preparing public source pins')
 results=[]
 for name in a.module or MODULES:
  module=MODULES[name];repo='ArtVsTheWorld/'+module['repo'];folder=ROOT/'Build/StandalonePublishing'/module['local'];docs=ROOT/'Packaging/Standalone'/name;version=module['version'];tag='v'+version
  archive=ROOT/'Release'/f'{name}-{version}.zip';evidence=archive.with_name(archive.stem+'-verification.json');checksums=archive.with_name(archive.stem+'-SHA256.txt')
  proof=json.loads(evidence.read_text(encoding='utf-8'))
  if proof['archiveSha256']!=sha(archive) or proof['sourceRevision']!=revision:raise ValueError('Release evidence differs from exact source/artifact')
  if repo not in run(['git','remote','get-url','origin'],cwd=folder).stdout:raise ValueError('Wrong publishing remote')
  existing=run([GH,'release','view',tag,'--repo',repo,'--json','isDraft'],check=False)
  resume=existing.returncode==0
  if resume and not json.loads(existing.stdout)['isDraft']:raise ValueError('Published release already exists; refusing to replace it: '+repo+' '+tag)
  if resume:
   previous=json.loads((folder/'source-release.json').read_text(encoding='utf-8'))
   if previous['archiveSha256']!=sha(archive) or previous['dllSha256']!=proof['dllSha256']:raise ValueError('Draft belongs to a different payload; do not overwrite it')
  baseline=run(['git','rev-parse',module['baseline']+'^{commit}'],cwd=folder).stdout.strip() if module['baseline'] else None
  download=f'https://github.com/{repo}/releases/download/{tag}/{archive.name}'
  status='**Standalone preview:** the new wrapper still needs clean-launch live acceptance.' if module['preview'] else 'Update 4. Ace reported the standalone packaging working; other setups still need their own checks.'
  readme=f'''# {module['title']}

**By Ace.** {module['summary']}

**[Download {version} ZIP]({download})** · [Full feature list](FEATURES.md) · [What's new](RELEASES.md)

{status}

## Install or update

1. Close the game and MMC Mod Manager.
2. Put **{name}.dll** directly in the versioned Mod Manager's **Plugins** folder. The ZIP contains that DLL, README.txt and FEATURES.md. Matching support files are embedded and prepared automatically.
3. Move older duplicate plugin DLLs outside Plugins, keeping settings/history folders. Restart MMC and launch normally. Use a writable MMC folder outside Program Files.

Read README.txt in the ZIP for this module's menu, old filenames, logs and removal steps. Use one provider of each feature. If keeping COT for other modules, disable this feature in COT before using the standalone. You can install any combination of matching current standalone modules.

**Want one feature? Download its standalone ZIP. Want the collection? [Download Dynasty COT](https://github.com/ArtVsTheWorld/DynastyCOT/releases).** See the [compatibility and saved-data guide](https://github.com/ArtVsTheWorld/DynastyCOT/blob/{revision}/docs/standalones.md).

## Source and verification

This repository is the standalone download/help front door. Current releases are built from the [single maintained COT workspace](https://github.com/ArtVsTheWorld/DynastyCOT/tree/{revision}), including [this module's source](https://github.com/ArtVsTheWorld/DynastyCOT/tree/{revision}/Modules/{module['path']}) and shared build/packaging code. `source-release.json` pins the exact source revision, baseline and artifact hashes.

The older source tree retained in this repository is historical; it is **not** the build input for this standalone release. Fixes are maintained in COT rather than separately editing four forks. Release assets include integrity metadata; successful offline checks do not certify every game/overhaul combination.
'''
  (folder/'README.md').write_text(readme,encoding='utf-8');(folder/'FEATURES.md').write_bytes((docs/'FEATURES.md').read_bytes())
  notes=(docs/'RELEASE_NOTES.md').read_text(encoding='utf-8');old=(folder/'RELEASES.md').read_text(encoding='utf-8') if (folder/'RELEASES.md').exists() else ''
  (folder/'RELEASES.md').write_text(old if old.startswith(notes) else notes+('\n\n---\n\n## Earlier release notes\n\n'+old if old else ''),encoding='utf-8')
  source=dict(format=1,title=module['title'],version=version,repository='https://github.com/ArtVsTheWorld/DynastyCOT',revision=revision,modulePath='Modules/'+module['path'],baselineTag=module['baseline'],baselineCommit=baseline,firstPublishedStandalone=baseline is None,archive=archive.name,archiveSha256=sha(archive),dllSha256=proof['dllSha256'],threeFileZip=True,preview=module['preview'],sourceTreeHere='historical; use pinned integration workspace')
  (folder/'source-release.json').write_text(json.dumps(source,indent=2)+'\n',encoding='utf-8')
  if not a.publish:print('Prepared',repo,tag);continue
  run(['git','add','README.md','FEATURES.md','RELEASES.md','source-release.json'],cwd=folder)
  if run(['git','diff','--cached','--quiet'],cwd=folder,check=False).returncode:run(['git','commit','-m',f'Publish {module["title"]} {version} from the shared COT workspace'],cwd=folder)
  commit=run(['git','rev-parse','HEAD'],cwd=folder).stdout.strip();run(['git','push','origin','HEAD:main'],cwd=folder)
  if name=='BeyondTheField' and json.loads(run([GH,'api','repos/'+repo]).stdout)['private']:run([GH,'repo','edit',repo,'--visibility','public','--accept-visibility-change-consequences'])
  run([GH,'repo','edit',repo,'--description',module['summary']+' By Ace; built from DynastyCOT.'])
  command=[GH,'release','create',tag,'--repo',repo,'--draft','--target',commit,'--title',module['title']+' '+version+(' - standalone preview' if module['preview'] else ' - Update 4'),'--notes-file',str(docs/'RELEASE_NOTES.md')]
  if module['preview']:command.append('--prerelease')
  if resume:run([GH,'release','edit',tag,'--repo',repo,'--target',commit,'--notes-file',str(docs/'RELEASE_NOTES.md')])
  else:run(command)
  run([GH,'release','upload',tag,str(archive),str(evidence),str(checksums),'--repo',repo,'--clobber'])
  actual=json.loads(run([GH,'release','view',tag,'--repo',repo,'--json','assets']).stdout)
  uploaded={x['name']:x for x in actual['assets']}
  for asset in [archive,evidence,checksums]:
   if asset.name not in uploaded or uploaded[asset.name]['size']!=asset.stat().st_size:raise ValueError('Upload incomplete')
  # GitHub stores server SHA256 digests on assets; verify them before publishing.
  # Draft releases need not have a real Git tag yet. The authenticated list
  # includes drafts; the by-tag endpoint can return 404 before publication.
  api=next(x for x in json.loads(run([GH,'api',f'repos/{repo}/releases?per_page=100']).stdout) if x['tag_name']==tag)
  for item in api['assets']:
   path=ROOT/'Release'/item['name']
   if item.get('digest')!='sha256:'+sha(path):raise ValueError('Server digest mismatch: '+item['name'])
  run([GH,'release','edit',tag,'--repo',repo,'--draft=false',*(['--latest=false'] if module['preview'] else ['--latest'])])
  results.append(dict(module=name,repository=repo,tag=tag,release='https://github.com/'+repo+'/releases/tag/'+tag,archiveSha256=sha(archive),sourceRevision=revision,distributionCommit=commit,preview=module['preview']))
  print('Published',results[-1]['release'],flush=True)
 (ROOT/'Build/standalone-publication.json').write_text(json.dumps(results,indent=2)+'\n',encoding='utf-8')
if __name__=='__main__':main()
