"""Push candidate documentation/source pins on review branches, never publish releases."""
from pathlib import Path
import argparse,json,subprocess,shutil
ROOT=Path(__file__).resolve().parents[1]
REPOS={'BeyondTheField':('Dynasty-Enhancer','BeyondTheField','DynastyEnhancer'),'GamedayAtmosphere':('DynamicGamedayAtmosphere','DynamicGamedayAtmosphere','GamedayAtmosphere'),'ExtraSettings':('offline-dynasty-settings','offline-dynasty-settings','ExtraDynastySettings'),'ExpandedPractice':('drive-based-practice','drive-based-practice','DrivePractice')}
def run(args,cwd=ROOT):return subprocess.check_output(args,cwd=cwd,text=True,stderr=subprocess.STDOUT).strip()
def main():
 p=argparse.ArgumentParser();p.add_argument('--push',action='store_true');a=p.parse_args();source=run(['git','rev-parse','HEAD']);branch='release-readiness-2026-11-13';results=[]
 for m in json.loads((ROOT/'Release/FinalReadiness-Private/modules.json').read_text()):
  name=m['module'];local,repo,modulePath=REPOS[name];folder=ROOT/'Build/StandalonePublishing'/local;docs=ROOT/'Packaging/Standalone'/name
  assert 'ArtVsTheWorld/'+repo in run(['git','remote','get-url','origin'],folder)
  assert not run(['git','status','--porcelain'],folder),'Publishing clone has unreviewed changes'
  if run(['git','branch','--show-current'],folder)!=branch:run(['git','switch','-c',branch],folder)
  run(['git','fetch','origin','tag','v'+m['publicBaseline']],folder)
  old=json.loads((folder/'source-release.json').read_text());baseline=run(['git','rev-parse','v'+m['publicBaseline']+'^{commit}'],folder)
  (folder/'README.md').write_text(f'''# {name} by Ace

An independent part of [Dynasty COT](https://github.com/ArtVsTheWorld/DynastyCOT). You do not need the other modules.

[Download published versions](https://github.com/ArtVsTheWorld/{repo}/releases) · [Full feature list](FEATURES.md) · [Release history](RELEASES.md)

**This branch prepares private candidate {m['version']}. Its release assets are not published yet.** The last public version used for comparison is {m['publicBaseline']}.

## Install or update

1. Close MMC and the game and back up your previous DLL
2. Download the chosen public ZIP and copy {name}.dll into the versioned Mod Manager's Plugins folder
3. Replace the old copy and launch MMC normally; matching support files prepare automatically

Use one provider per module. The COT installer can retire recognized older copies with backups. Keep all settings/history folders. To roll back, close MMC/game and restore your older DLL.

Current candidates store settings/history/logs in `%LOCALAPPDATA%\\Ace\\DynastyCOT`, shared across MMC copies. Installation into protected folders still requires permission to replace DLLs.

Want one feature? Download its standalone ZIP. Want several? Use the [optional COT installer](https://github.com/ArtVsTheWorld/DynastyCOT/releases).

## Source and support

Maintained source and compatibility checks are together in [Dynasty COT at {source[:8]}](https://github.com/ArtVsTheWorld/DynastyCOT/tree/{source}/Modules/{modulePath}). This repository's older source tree is historical; use that pinned integration source for current development.

See RELEASE_NOTES.md for the candidate's changes and limits. Automated checks do not establish universal live-game compatibility. Keep your previous working DLL and report your game/mod versions with logs when requesting support.
''',encoding='utf8')
  for leaf in ['FEATURES.md','RELEASE_NOTES.md','DISCORD.txt']:
   shutil.copyfile(docs/leaf,folder/leaf)
  if (docs/'LICENSES.txt').exists():shutil.copyfile(docs/'LICENSES.txt',folder/'LICENSES.txt')
  (folder/'source-release.json').write_text(json.dumps(dict(format=1,title=name,version=m['version'],status='private-candidate-not-published',repository='https://github.com/ArtVsTheWorld/DynastyCOT',revision=source,modulePath='Modules/'+modulePath,baselineTag='v'+m['publicBaseline'],baselineCommit=baseline,archive=m['archive'],archiveSha256=m['archiveSha256'],dllSha256=m['dllSha256'],lastPublishedVersion=m['publicBaseline'],sourceTreeHere='historical; use pinned integration workspace'),indent=2)+'\n',encoding='utf8')
  released=folder/'RELEASES.md';prior=released.read_text(encoding='utf8') if released.exists() else ''
  released.write_text('## Private candidate '+m['version']+' (not published)\n\n'+(docs/'RELEASE_NOTES.md').read_text(encoding='utf8')+'\n---\n\n'+prior,encoding='utf8')
  files=['README.md','FEATURES.md','RELEASE_NOTES.md','DISCORD.txt','RELEASES.md','source-release.json']+(['LICENSES.txt'] if (docs/'LICENSES.txt').exists() else [])
  run(['git','add','--',*files],folder);run(['git','commit','-m','Prepare '+name+' '+m['version']+' source pin and release notes'],folder)
  commit=run(['git','rev-parse','HEAD'],folder)
  if a.push:print(run(['git','push','-u','origin',branch],folder),flush=True)
  results.append(dict(module=name,repository='ArtVsTheWorld/'+repo,commit=commit,branch=branch,url='https://github.com/ArtVsTheWorld/'+repo+'/commit/'+commit,sourceRevision=source,releasePublished=False))
 (ROOT/'Build/FinalReleasePass/source-pushes.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results,indent=2))
if __name__=='__main__':main()
