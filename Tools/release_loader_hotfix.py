"""Package/publish the two approved loader hotfixes; never touch installed plugins."""
from pathlib import Path
import argparse,hashlib,json,zipfile,shutil,subprocess
from publish_final_assets import GH,run,upload
R=Path(__file__).resolve().parents[1];O=R/'Release/Loader-Hotfix';E=R/'Build/LoaderHotfix'
MODULES={'BeyondTheField':('1.0.25','1.0.24','2','Beyond the Field','ArtVsTheWorld/BeyondTheField','DynastyEnhancer'),
         'GamedayAtmosphere':('2.1.39','2.1.38','5','Gameday Atmosphere','ArtVsTheWorld/DynamicGamedayAtmosphere','GamedayAtmosphere')}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return p.read_text(encoding='utf8')
def put(p,v):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf8')
def main():
 p=argparse.ArgumentParser();p.add_argument('--package',action='store_true');p.add_argument('--prepare-repos',action='store_true');p.add_argument('--publish',action='store_true');a=p.parse_args();O.mkdir(exist_ok=True);E.mkdir(exist_ok=True)
 revision=run(['git','rev-parse','HEAD']).stdout.strip();catalog=json.loads(read(R/'Packaging/Collection/catalog.json'));rows=[]
 for name,(version,old,update,title,repo,modulePath) in MODULES.items():
  docs=R/'Packaging/Standalone'/name;mfile=R/'Build'/('current-gameday-standalone.json' if name=='GamedayAtmosphere' else f'current-standalone-{name}.json');m=json.loads(read(mfile));assert m['version']==version and sha(Path(m['dll']))==m['dllSha256']
  if a.package:
   # BTF is the exact source-built DLL that the user just installed and tested.
   # Only external docs change; neither is embedded in its DLL payload.
   if name=='BeyondTheField':
    tested=json.loads(read(R/'Release/BtfStartup-Private/BeyondTheField-1.0.25-startup1-verification.json'));assert m['dllSha256']==tested['dllSha256']
   changedDocs={}
   for path,digest in list(m['sourceHashes'].items()):
    now=sha(R/path)
    if now!=digest:
     assert name=='BeyondTheField' and path in [f'Packaging/Standalone/{name}/README.txt',f'Packaging/Standalone/{name}/FEATURES.md'],path
     changedDocs[path]={'before':digest,'after':now};m['sourceHashes'][path]=now
   if changedDocs:m['externalDocumentationRefresh']=changedDocs;put(mfile,m)
   for path,digest in m['sourceHashes'].items():assert sha(R/path)==digest,path
   guide=R/f'docs/player-guides/{name}-Player-Guide.pdf'
   files={name+'.dll':Path(m['dll']),'README.txt':docs/'README.txt','FEATURES.md':docs/'FEATURES.md',guide.name:guide}
   if name=='BeyondTheField':assert (docs/'LICENSES.txt').read_bytes() in (docs/'README.txt').read_bytes()
   archive=O/f'{name}-{version}.zip'
   with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
    for leaf,path in files.items():
     entry=zipfile.ZipInfo(leaf,(2020,1,1,0,0,0));entry.compress_type=zipfile.ZIP_DEFLATED;entry.external_attr=0o644<<16;z.writestr(entry,path.read_bytes())
   with zipfile.ZipFile(archive) as z:assert len(z.namelist())==4 and z.testzip() is None
   baseline=json.loads(read(R/f'Release/Publication-Final/{name}-{old}-verification.json'))
   proof=dict(format=2,name=name,version=version,archiveSha256=sha(archive),dllSha256=m['dllSha256'],playerGuideSha256=sha(guide),sourceRevision=revision,sourceHashes=m['sourceHashes'],payload=m.get('payload',{}),supportFiles=m.get('supportFiles',{}),nativeSha256=m.get('nativeSha256'),nativeTests=m.get('nativeTests'),resources=m.get('resources',[]),files=[dict(path=k,sha256=sha(v),bytes=v.stat().st_size) for k,v in files.items()],publicBaseline=dict(version=old,archiveSha256=baseline['archiveSha256']),hotfix='Bounded read-only Windows error 24 module-list retries; native load itself is never repeated.',missingPathReports='Separate unresolved issue; logs not yet supplied.',liveAcceptance='BTF exact private DLL completed a clean user-installed launch with both runtimes and a current feed ready. Retry branch is verified in isolated tests. GDA live acceptance remains pending.',installedIntoLiveMmc=False)
   proof['loaderTests']={'readOnlySnapshotMaximumAttempts':8,'maximumSnapshotDelayMs':700,'nativeLoadNeverRetried':True,'sourceTestsSha256':sha(R/'Modules/GamedayAtmosphere/Tests/ManagedRuntimeLoaderTests.cs')}
   if (E/'archive-verification.json').exists():proof['archiveFixtureResult']=json.loads(read(E/'archive-verification.json'))['results']
   for key,path in [('gdaManagedChecks','Build/gameday-standalone-verification.json'),('standaloneChecks','Build/standalones-verification.json')]:
    if (R/path).exists():proof[key]=json.loads(read(R/path))['checks']
   put(O/(archive.stem+'-verification.json'),proof);(O/(archive.stem+'-SHA256.txt')).write_text(sha(archive)+'  '+archive.name+'\n'+sha(guide)+'  '+guide.name+'\n',encoding='ascii')
   shutil.copyfile(guide,O/guide.name);shutil.copyfile(guide.with_suffix('.md'),O/guide.with_suffix('.md').name)
   module=next(x for x in catalog['Modules'] if x['Id']==name);current=module['Current'];base=f'https://github.com/{repo}/releases/download/v{version}/'
   current.update(Version=version,Tag='v'+version,ArchiveName=archive.name,ArchiveUrl=base+archive.name,ArchiveSha256=sha(archive),DllSha256=m['dllSha256'],ArchiveBytes=archive.stat().st_size,ProofUrl=base+archive.stem+'-verification.json',PageUrl=f'https://github.com/{repo}/releases/tag/v{version}',Prerelease=False,Notes=read(docs/'RELEASE_NOTES.md'))
   catalog['KnownPlugins'].append(dict(Path='Plugins/'+name+'.dll',Module=name,Sha256=m['dllSha256']))
   rows.append(dict(module=name,version=version,previous=old,update=update,title=title,repository=repo,archive=archive.name,archiveSha256=sha(archive),dllSha256=m['dllSha256'],guideSha256=sha(guide),sourceRevision=revision))
  folder=E/'Repos'/name
  if a.prepare_repos:
   if not folder.exists():folder.parent.mkdir(exist_ok=True);run(['git','clone','--quiet','--single-branch','--branch','main','https://github.com/'+repo+'.git',folder])
   assert not run(['git','status','--porcelain'],cwd=folder).stdout;run(['git','pull','--ff-only','origin','main'],cwd=folder)
   row=next(x for x in json.loads(read(O/'modules.json')) if x['module']==name);assert row['sourceRevision']==revision
   text=read(folder/'README.md');text=text.replace('/v'+old+'/', '/v'+version+'/').replace(name+'-'+old+'.zip',name+'-'+version+'.zip').replace('Download '+old+' ZIP','Download '+version+' ZIP')
   previousPin=json.loads(read(folder/'source-release.json'));text=text.replace(previousPin['revision'],revision)
   (folder/'README.md').write_text(text,encoding='utf8');(folder/'FEATURES.md').write_bytes((docs/'FEATURES.md').read_bytes());history=read(folder/'RELEASES.md');(folder/'RELEASES.md').write_text(read(docs/'RELEASE_NOTES.md')+'\n\n---\n\n'+history,encoding='utf8')
   previousPin.update(version=version,revision=revision,archive=row['archive'],archiveSha256=row['archiveSha256'],dllSha256=row['dllSha256'],playerGuideSha256=row['guideSha256'],publicBaseline=old);put(folder/'source-release.json',previousPin)
   run(['git','config','user.name',run(['git','config','user.name']).stdout.strip()],cwd=folder);run(['git','config','user.email',run(['git','config','user.email']).stdout.strip()],cwd=folder)
   run(['git','add','README.md','FEATURES.md','RELEASES.md','source-release.json'],cwd=folder);run(['git','commit','-m',f'Update {update} Hotfix: reliable {title} startup ({version})'],cwd=folder)
  if a.publish:
   row=next(x for x in json.loads(read(O/'modules.json')) if x['module']==name);assert row['sourceRevision']==revision
   proof=json.loads(read(O/f'{name}-{version}-verification.json'));assert proof['sourceRevision']==revision and sha(O/row['archive'])==row['archiveSha256']
   assert not run(['git','status','--porcelain'],cwd=folder).stdout
   run(['git','push','origin','HEAD:main'],cwd=folder);target=run(['git','rev-parse','HEAD'],cwd=folder).stdout.strip()
   assets=[O/row['archive'],O/f'{name}-{version}-verification.json',O/f'{name}-{version}-SHA256.txt',O/f'{name}-Player-Guide.pdf',O/f'{name}-Player-Guide.md']
   url=upload(repo,'v'+version,f'Update {update} Hotfix',docs/'RELEASE_NOTES.md',assets,target);print(url,flush=True)
 if a.package:
  put(O/'modules.json',rows);put(E/'catalog.json',catalog);print('Prepared exact four-file hotfix assets:',O)
if __name__=='__main__':main()
