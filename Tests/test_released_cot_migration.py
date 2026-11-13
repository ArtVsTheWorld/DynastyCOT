"""Run real released Install.ps1 files, then collection migration/cold prepare/undo.

All destinations are MMC copies below Build with fixture markers. No manager,
game, watcher, helper or generator is launched. Existing installations are read
only for the manager EXE and SDK references, never modified.
"""
from pathlib import Path
import argparse,datetime,hashlib,json,os,subprocess,sys,zipfile
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'Tools'))
from verify_package import verify
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--version',action='append',choices=['1.0.0','1.0.1','1.0.2']);a=p.parse_args();versions=a.version or ['1.0.0','1.0.1','1.0.2']
 meta=json.loads((ROOT/'Build/current-collection-setup.json').read_text());assert sha(Path(meta['exe']))==meta['sha256']
 for rel,digest in meta['sources'].items():assert sha(ROOT/rel)==digest,rel
 sdk=Path(r'C:\Apps\CFB 27\MMC\MMC_ModManager_v1.1.0.6');work=ROOT/'Build'/('ReleasedMigration-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'));work.mkdir();logs=[];cases=[];checks=0
 def run(cmd):
  result=subprocess.run([str(x) for x in cmd],capture_output=True,text=True,encoding='utf-8',errors='replace',env={k:v for k,v in os.environ.items() if k.lower()!='psmodulepath'});logs.append(result.stdout+result.stderr);(work/'results.log').write_text('\n'.join(logs),encoding='utf-8');assert result.returncode==0,result.stdout+result.stderr;return result.stdout
 source=ROOT/'Packaging/Collection';exe=work/'CollectionReleasedMigration.exe';compiler=Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe')
 run([compiler,'/nologo','/platform:x64','/optimize+','/warnaserror+','/out:'+str(exe),'/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll',Path(meta['work'])/'CollectionInfo.cs',*[source/name for name in ['Models.cs','Paths.cs','Installer.cs','Downloads.cs']],ROOT/'Core/UserStorage.cs',ROOT/'Tests/CollectionReleasedMigration.cs'])
 def helper(mode,root):
  nonlocal checks
  text=run([exe,mode,ROOT,root,work,sdk,meta.get('artifactRoot',str(ROOT/'Release'))]);checks+=int(text.split()[1]);print(text.strip(),flush=True)
 ids=['DynastyEnhancer','GamedayAtmosphere','ExtraDynastySettings','DrivePractice']
 for version in versions:
  archive=ROOT/f'Release/DynastyCOT-{version}.zip';proof=json.loads((ROOT/f'Release/verification-{version}.json').read_text());assert sha(archive)==proof['archiveSha256'];package=work/('Bundle-'+version)
  with zipfile.ZipFile(archive) as z:
   assert not z.testzip();assert all(not x.startswith(('/','\\')) and '..' not in Path(x).parts for x in z.namelist());z.extractall(package)
  verify(package)
  for label,enabled in [('all',ids),('gda-only',['GamedayAtmosphere']),('btf-settings',['DynastyEnhancer','ExtraDynastySettings'])]:
   root=work/(version+'-'+label)/'MMC_ModManager_vFixture';root.mkdir(parents=True);(root/'.cot-fixture').write_text('isolated');
   for name in ['MMCModManager.exe','FrostyCore.dll','FrostySdk.dll']:(root/name).write_bytes((sdk/name).read_bytes())
   choices={id:id in enabled for id in ids}
   run(['powershell.exe','-NoProfile','-ExecutionPolicy','Bypass','-File',package/'Install.ps1','-MmcRoot',root,'-PackageRoot',package,'-Fixture','-FixtureRoot',work,'-ModuleChoicesJson',json.dumps(choices),'-Action','Install'])
   sentinels={
    'COT-Runtime/DynastyEnhancer/DynastyEnhancementSettings/424242.json':json.dumps(dict(format=2,league=424242,socialMedia=True,recruitingFog=True,rosterFog=True,playerGradeScale=3,teamGradeScale=1,ratingColors=True,teamFog=True,dollarValues=True,recruitInterestDisplay=2)).encode(),
    'COT-Runtime/DynastyEnhancer/RuntimeHistory/424242/accounts.json':b'{"accounts":{"fixture-author":{"id":"retained-id","handle":"@SameAccount"}}}',
    'COT-Runtime/DynastyEnhancer/RuntimeHistory/424242/editions/2026-11.json':b'{"posts":[{"id":"published-fixture","text":"Published words must remain unchanged."}]}',
    'COT-Runtime/DynastyEnhancer/RecruitResumeHistory/424242.json':b'{"records":[{"recruit":42,"week":11,"yards":115}]}',
    'Plugins/OfflineDynastySettings/DynastyClocks.json':b'{"format":1,"leagues":{"424242":{"offense":"on","defense":15}}}',
    'Plugins/DynamicGamedayAtmosphere/DynamicGamedayAtmosphere.ini':b'[Atmosphere]\r\nVisitingFanStrength=85\r\n',
    'Plugins/DynamicGamedayAtmosphere/History/424242.json':b'{"fixture":{"awayShare":0.09}}',
    'Plugins/SituationalPractice/session.json':b'{"fixture":"saved-session"}',
    'Plugins/DynastyCOT/Backups/existing-user-backup.txt':b'previous user backup',
    'PrivateSaves/DYNASTY-MIGRATION-SENTINEL.sav':b'fixture checkpoint, never a real game save',
    'Plugins/OtherPlugin.dll':b'unrelated plugin marker, never executed'}
   for rel,data in sentinels.items():file=root/rel;file.parent.mkdir(parents=True,exist_ok=True);file.write_bytes(data)
   sentinels['Plugins/DynastyCOT/settings.json']=(root/'Plugins/DynastyCOT/settings.json').read_bytes()
   def intact():
    nonlocal checks
    for rel,data in sentinels.items():assert (root/rel).read_bytes()==data,'User data changed: '+rel;checks+=1
   helper('verify-host',root);helper('migrate',root);intact();helper('prepare',root);intact();helper('restore',root);intact();helper('verify-host',root)
   # A second migration must remain usable after materialization and rollback.
   helper('migrate',root);helper('prepare',root);helper('remove-restore',root);intact()
   cases.append(dict(oldVersion=version,choices=choices,migration=True,coldSupportPreparation=True,rollbackHostIntegrity=True,repeatMigration=True,partialRemoveRestore=True,userDataPreserved=True))
 report=dict(installerVersion=meta['version'],exeSha256=meta['sha256'],sources=meta['sources'],tests={p.relative_to(ROOT).as_posix():sha(p) for p in [Path(__file__),ROOT/'Tests/CollectionReleasedMigration.cs']},checks=checks,cases=cases,work=str(work),actualReleasedInstallers=True,actualReleasedStandalonePreparation=True,gameOrManagerLaunched=False,liveInstall=False)
 (ROOT/'Build/released-migration-verification.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print('PASS',checks,'checks across',len(cases),'released old-COT migration cases; no live installation.',flush=True)
if __name__=='__main__':main()
