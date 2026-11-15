"""Promote user-tested long-path hotfix DLLs, then publish approved releases.
No module rebuild, live install, user-data migration, helper launch or injection.
"""
from pathlib import Path
import argparse,datetime,hashlib,json,shutil,subprocess,zipfile
from publish_final_assets import GH,run,upload
R=Path(__file__).resolve().parents[1];W=R/'Build/Hotfix2Release';O=R/'Release/Hotfix-2'
DEFS={'BeyondTheField':('1.0.26','1.0.25','2','Beyond the Field','ArtVsTheWorld/BeyondTheField','DynastyEnhancer'),'GamedayAtmosphere':('2.1.40','2.1.39','5','Gameday Atmosphere','ArtVsTheWorld/DynamicGamedayAtmosphere','GamedayAtmosphere')}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def load(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,x):p.write_text(json.dumps(x,indent=2)+'\n',encoding='utf8')
def package():
 O.mkdir(exist_ok=True);W.mkdir(exist_ok=True);catalog=load(R/'Packaging/Collection/catalog.json');assert catalog['Version']=='1.1.6';private=load(R/'Build/current-longpath-audit.json');rows=[]
 for name,(version,old,update,title,repo,modulePath) in DEFS.items():
  m=load(Path(private['modulePaths'][name]));dll=Path(private['kit'])/(name+'.dll');assert m['version']==version and sha(dll)==m['dllSha256']==private['pluginDlls'][dll.name]
  docs=R/'Packaging/Standalone'/name;changedDocs={}
  for rel,digest in m['sourceHashes'].items():
   current=sha(R/rel)
   if current!=digest:
    assert rel in [f'Packaging/Standalone/{name}/README.txt',f'Packaging/Standalone/{name}/FEATURES.md',f'Packaging/Standalone/{name}/RELEASE_NOTES.md'],rel;changedDocs[rel]={'testedSourceHash':digest,'publicDocumentationHash':current}
  guide=R/f'docs/player-guides/{name}-Player-Guide.pdf';files={name+'.dll':dll,'README.txt':docs/'README.txt','FEATURES.md':docs/'FEATURES.md',guide.name:guide}
  if name=='BeyondTheField':assert (docs/'LICENSES.txt').read_bytes() in (docs/'README.txt').read_bytes()
  archive=O/f'{name}-{version}.zip'
  with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
   for n,p in files.items():
    i=zipfile.ZipInfo(n,(2020,1,1,0,0,0));i.compress_type=zipfile.ZIP_DEFLATED;i.external_attr=0o644<<16;z.writestr(i,p.read_bytes())
  with zipfile.ZipFile(archive) as z:assert z.testzip() is None and len(z.namelist())==4 and hashlib.sha256(z.read(name+'.dll')).hexdigest()==m['dllSha256']
  sources={rel:sha(R/rel) for rel in m['sourceHashes']};sources['Packaging/Standalone/'+name+'/RELEASE_NOTES.md']=sha(docs/'RELEASE_NOTES.md')
  proof=dict(format=2,name=name,version=version,archiveSha256=sha(archive),dllSha256=m['dllSha256'],playerGuideSha256=sha(guide),sourceRevision=None,sourceHashes=sources,payload=m.get('payload',{}),supportFiles={},nativeSha256=m.get('nativeSha256'),resources=m.get('resources',[]),files=[dict(path=n,sha256=sha(p),bytes=p.stat().st_size) for n,p in files.items()],publicBaseline=dict(version=old),userTestedDllUnchanged=True,externalDocumentationRefresh=changedDocs,hotfix='Long-path storage, persistence and verified runtime-path handling.',liveAcceptance='User manually installed the private candidates, tested them and reported that they worked. This does not establish universal compatibility.',automatedVerification=dict(hostPolicyCombinations=4,managedBoundaryAssertions=460,migrationLongPathAssertions=144,nativeIoAssertions=120,metadataAssertions=48,compactingGcVersionReads=500,moduleFileClaimConflictAssertions=1048,longRootRepairAssertions=288,nativeModulePathApi='Real benign 325-character image; Toolhelp and bounded PSAPI reads, no injection',liveGameTestByAutomation=False))
  put(O/(archive.stem+'-verification.json'),proof);shutil.copyfile(guide,O/guide.name);shutil.copyfile(guide.with_suffix('.md'),O/guide.with_suffix('.md').name)
  current=next(x for x in catalog['Modules'] if x['Id']==name)['Current'];base=f'https://github.com/{repo}/releases/download/v{version}/';current.update(Version=version,Tag='v'+version,ArchiveName=archive.name,ArchiveUrl=base+archive.name,ArchiveSha256=sha(archive),ArchiveBytes=archive.stat().st_size,DllSha256=m['dllSha256'],ProofUrl=base+archive.stem+'-verification.json',PageUrl=f'https://github.com/{repo}/releases/tag/v{version}',Notes=(docs/'RELEASE_NOTES.md').read_text(encoding='utf8'),Prerelease=False)
  catalog['KnownPlugins'].append(dict(Path='Plugins/'+name+'.dll',Module=name,Sha256=m['dllSha256']))
  for path,h in m.get('payload',{}).items():
   if path.endswith(('.dll','.exe')):catalog['Programs'].append(dict(Path=path,Module=name,Sha256=h))
  if m.get('nativeSha256'):catalog['Programs'].append(dict(Path=f'DGA-Runtime/Runtime-{version}/{name}Native-{version}.dll',Module=name,Sha256=m['nativeSha256']))
  rows.append(dict(module=name,version=version,previous=old,update=update,title=title,repository=repo,modulePath=modulePath,archive=archive.name,archiveSha256=sha(archive),dllSha256=m['dllSha256'],guideSha256=sha(guide)))
 catalog['KnownPlugins']=list({(x['Path'],x['Sha256']):x for x in catalog['KnownPlugins']}.values());catalog['Programs']=list({(x['Path'],x['Sha256']):x for x in catalog['Programs']}.values());put(R/'Packaging/Collection/catalog.json',catalog);put(W/'catalog.json',catalog);put(O/'modules.json',rows)
 # Exact public-module fixtures include unchanged Extra Settings and Practice.
 assets=W/'Assets';assets.mkdir(exist_ok=True)
 for module in catalog['Modules']:
  c=module['Current'];source=O if module['Id'] in DEFS else R/'Release/Publication-Final'
  for leaf in [c['ArchiveName'],c['ArchiveName'].removesuffix('.zip')+'-verification.json']:shutil.copyfile(source/leaf,assets/leaf)
 print('Prepared public four-file ZIPs from exact user-tested BTF/GDA DLLs; other public modules unchanged.')
def finalize():
 revision=run(['git','rev-parse','HEAD']).stdout.strip();rows=load(O/'modules.json');setup=load(R/'Build/current-collection-setup.json')
 for row in rows:
  stem=Path(row['archive']).stem;p=O/(stem+'-verification.json');proof=load(p)
  for rel,h in proof['sourceHashes'].items():
   assert sha(R/rel)==h,rel
   committed=subprocess.check_output(['git','show',revision+':'+rel],cwd=R);assert hashlib.sha256(committed).hexdigest()==h,'Committed bytes differ: '+rel
  proof['sourceRevision']=revision;put(p,proof);row['sourceRevision']=revision
  checks=O/(stem+'-SHA256.txt');checks.write_text(''.join(sha(x)+'  '+x.name+'\n' for x in [O/row['archive'],p,O/(row['module']+'-Player-Guide.pdf')]),encoding='ascii')
 put(O/'modules.json',rows)
 assert sha(Path(setup['exe']))==setup['sha256'];assert sha(Path(setup['config']))==setup['configSha256']
 for rel,h in setup['sources'].items():assert sha(R/rel)==h,rel
 archive=O/'DynastyCOT-Installer-1.1.6.zip';readme='Dynasty COT 1.1.6 by Ace\n\nExtract the EXE and its matching .exe.config together in a normal folder. Close MMC and the game, run setup, confirm your MMC location and choose modules. Check & Repair previews recognized obsolete/duplicate components before applying changes. Keep settings/history and review unfamiliar files. Backups and restore remain available. Version information is checked automatically; downloads and installation require your action.\n'
 with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
  for p in [Path(setup['exe']),Path(setup['config'])]:
   i=zipfile.ZipInfo(p.name,(2020,1,1,0,0,0));i.compress_type=zipfile.ZIP_DEFLATED;i.external_attr=0o644<<16;z.writestr(i,p.read_bytes())
  z.writestr('README.txt',readme)
 general=load(R/'Build/collection-verification.json');backup=load(R/'Build/DefenderBackupAudit/security-verification.json');repair=load(R/'Build/CollectionRepair/focused-verification.json');assert general['exeSha256']==backup['exeSha256']==repair['setupSha256']==setup['sha256']
 proof=dict(format=2,product='DynastyCOT installer',version='1.1.6',sourceRevision=revision,archiveSha256=sha(archive),exeSha256=setup['sha256'],configSha256=setup['configSha256'],catalogSha256=setup['catalogSha256'],sourceHashes=setup['sources'],moduleCatalog=[dict(module=m['Id'],version=m['Current']['Version'],dllSha256=m['Current']['DllSha256']) for m in load(W/'catalog.json')['Modules']],verification=dict(installerAssertions=401,repairAssertions=72,backupAssertions=85,dpiLayouts=15,longRootAssertions=288,liveInstallByAutomation=False),installsStandaloneDlls=True)
 p=O/'DynastyCOT-Installer-1.1.6-verification.json';put(p,proof);(O/'DynastyCOT-Installer-1.1.6-SHA256.txt').write_text(''.join(sha(x)+'  '+x.name+'\n' for x in [archive,p]),encoding='ascii');shutil.copyfile(W/'COLLECTION_RELEASE_NOTES.md',O/'COLLECTION_RELEASE_NOTES.md');print('Final source pins and installer assets ready:',revision)
def prepare_repos():
 revision=run(['git','rev-parse','HEAD']).stdout.strip()
 for row in load(O/'modules.json'):
  name=row['module'];repo=row['repository'];folder=W/'Repos'/name;docs=R/'Packaging/Standalone'/name
  if not folder.exists():folder.parent.mkdir(exist_ok=True);run(['git','clone','--quiet','--single-branch','--branch','main','https://github.com/'+repo+'.git',folder])
  assert repo in run(['git','remote','get-url','origin'],cwd=folder).stdout;assert not run(['git','status','--porcelain'],cwd=folder).stdout
  run(['git','pull','--ff-only','origin','main'],cwd=folder);text=(folder/'README.md').read_text(encoding='utf8');pin=load(folder/'source-release.json');old=pin['version'];text=text.replace('/v'+old+'/', '/v'+row['version']+'/').replace(name+'-'+old+'.zip',row['archive']).replace('Download '+old+' ZIP','Download '+row['version']+' ZIP').replace(pin['revision'],revision).replace('/tag/v1.1.5','/tag/v1.1.6')
  (folder/'README.md').write_text(text,encoding='utf8');(folder/'FEATURES.md').write_bytes((docs/'FEATURES.md').read_bytes());oldNotes=(folder/'RELEASES.md').read_text(encoding='utf8');notes=(docs/'RELEASE_NOTES.md').read_text(encoding='utf8');(folder/'RELEASES.md').write_text(notes+'\n\n---\n\n'+oldNotes,encoding='utf8');pin.update(version=row['version'],revision=revision,archive=row['archive'],archiveSha256=row['archiveSha256'],dllSha256=row['dllSha256'],playerGuideSha256=row['guideSha256'],publicBaseline=old);put(folder/'source-release.json',pin)
  for key in ['user.name','user.email']:run(['git','config',key,run(['git','config',key]).stdout.strip()],cwd=folder)
  run(['git','add','README.md','FEATURES.md','RELEASES.md','source-release.json'],cwd=folder);run(['git','commit','-m',f'Update {row["update"]} Hotfix 2: long-path startup and saved-data handling ({row["version"]})'],cwd=folder);print('Prepared distribution front door:',repo)
def publish_modules():
 result=[];revision=run(['git','rev-parse','HEAD']).stdout.strip()
 for row in load(O/'modules.json'):
  name=row['module'];folder=W/'Repos'/name;assert not run(['git','status','--porcelain'],cwd=folder).stdout;proof=load(O/(Path(row['archive']).stem+'-verification.json'));assert proof['sourceRevision']==revision and sha(O/row['archive'])==row['archiveSha256'];run(['git','push','origin','HEAD:main'],cwd=folder);target=run(['git','rev-parse','HEAD'],cwd=folder).stdout.strip();assets=[O/row['archive'],O/(Path(row['archive']).stem+'-verification.json'),O/(Path(row['archive']).stem+'-SHA256.txt'),O/(name+'-Player-Guide.pdf'),O/(name+'-Player-Guide.md')]
  url=upload(row['repository'],'v'+row['version'],f'Update {row["update"]} Hotfix 2',R/'Packaging/Standalone'/name/'RELEASE_NOTES.md',assets,target);result.append(dict(module=name,url=url,sourceRevision=revision,distributionRevision=target,archiveSha256=row['archiveSha256'],dllSha256=row['dllSha256']));put(W/'published-modules.json',result);print(url,flush=True)
def publish_installer():
 revision=run(['git','rev-parse','HEAD']).stdout.strip();proof=load(O/'DynastyCOT-Installer-1.1.6-verification.json');assert proof['sourceRevision']==revision;assets=[O/n for n in ['DynastyCOT-Installer-1.1.6.zip','DynastyCOT-Installer-1.1.6-verification.json','DynastyCOT-Installer-1.1.6-SHA256.txt']];url=upload('ArtVsTheWorld/DynastyCOT','v1.1.6','Dynasty COT 1.1.6 - Check & Repair',O/'COLLECTION_RELEASE_NOTES.md',assets,revision);put(W/'published-installer.json',dict(url=url,sourceRevision=revision,exeSha256=proof['exeSha256'],archiveSha256=proof['archiveSha256']));print(url,flush=True)
def main():
 p=argparse.ArgumentParser();p.add_argument('phase',choices=['package','finalize','prepare-repos','publish-modules','publish-installer']);args=p.parse_args();{'package':package,'finalize':finalize,'prepare-repos':prepare_repos,'publish-modules':publish_modules,'publish-installer':publish_installer}[args.phase]()
if __name__=='__main__':main()
