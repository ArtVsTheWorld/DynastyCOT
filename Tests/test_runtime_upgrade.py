"""1.0 renamed-runtime migration, preserving data and transactional rollback.

Default tests the actual current stage against the retained 0.1.19 source ZIP.
--synthetic creates a clearly labeled renamed fixture stage for development;
it cannot establish source-built 1.0 package acceptance.
"""
from pathlib import Path
import argparse, datetime, hashlib, json, os, shutil, subprocess, zipfile

ROOT=Path(__file__).resolve().parents[1]
RENAMES={
 'COT-Runtime/DynastyEnhancer/DynastyEnhancerNative-v103.dll':'COT-Runtime/DynastyEnhancer/BeyondTheFieldNative-v103.dll',
 'COT-Runtime/DynastyEnhancer/DynastyEnhancerRecruitFog-v1.dll':'COT-Runtime/DynastyEnhancer/BeyondTheFieldRecruitFog-v1.dll',
 'COT-Runtime/DynastyEnhancer/LoadDynastyEnhancer.exe':None,
 'DGA-Runtime/Runtime-2.1.9/DynamicGamedayAtmosphereNative-2.1.9.dll':'DGA-Runtime/Runtime-2.1.9/GamedayAtmosphereNative-2.1.9.dll',
 'Plugins/OfflineDynastySettings/OfflineDynastySettings.Persistence.exe':'Plugins/OfflineDynastySettings/ExtraSettings.Persistence.exe',
 'Plugins/SituationalPractice/SituationalPractice.Runtime.exe':'Plugins/SituationalPractice/ExpandedPractice.Runtime.exe',
 'Plugins/SituationalPractice/SituationalPractice.Runtime.pdb':'Plugins/SituationalPractice/ExpandedPractice.Runtime.pdb'}
DATA={
 'Plugins/DynastyCOT/settings.json':b'{"format":1,"modules":{"DynastyEnhancer":true,"GamedayAtmosphere":false,"ExtraDynastySettings":true,"DrivePractice":false}}',
 'COT-Runtime/DynastyEnhancer/RuntimeHistory/101/published.json':b'published post IDs and wording',
 'COT-Runtime/DynastyEnhancer/RecruitResumeHistory/101.json':b'persistent recruit records',
 'COT-Runtime/DynastyEnhancer/DynastyEnhancementSettings/101.json':b'fog/colors/money choices',
 'Plugins/OfflinePlaycallSettings/dynasties.json':b'per-Dynasty clocks',
 'Plugins/DynamicGamedayAtmosphere/settings.ini':b'crowd preferences',
 'Plugins/DynamicGamedayAtmosphere/DynamicGamedayAtmosphere.ini':b'saved public GDA preferences',
 'Plugins/DynamicGamedayAtmosphere/History/crowd.json':b'saved crowd plan',
 'Plugins/SituationalPractice/session.json':b'practice preferences',
 'Plugins/OtherOverhaul.dll':b'unrelated overhaul'}
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def write(root,relative,data):
 p=root/relative;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data)
def snapshot(root):return {p.relative_to(root).as_posix():sha(p) for p in root.rglob('*') if p.is_file() and 'Backups' not in p.parts}
def refresh(package):
 path=package/'package-manifest.json';doc=json.loads(path.read_text())
 for item in doc['files']:item['sha256']=sha(package/item['path'])
 path.write_text(json.dumps(doc))

def main():
 parser=argparse.ArgumentParser();parser.add_argument('--synthetic',action='store_true');parser.add_argument('--gda-archive',type=Path,default=ROOT.parent.parent/'Working/DynamicGamedayAtmosphere/Release/DynamicGamedayAtmosphere-2.0.35.zip');args=parser.parse_args()
 base=ROOT/'Build'/('RuntimeUpgrade-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'));base.mkdir()
 old=base/'Previous019'
 with zipfile.ZipFile(ROOT/'Release/DynastyCOT-0.1.19-private.zip') as archive:archive.extractall(old)
 stage=Path(json.loads((ROOT/'Build/current-package.json').read_text())['stage'])
 if args.synthetic:
  stage=base/'Synthetic100';shutil.copytree(old,stage)
  manifest=json.loads((stage/'package-manifest.json').read_text());manifest['version']='1.0.0'
  files=[]
  for item in manifest['files']:
   if item['path'] in RENAMES:
    target=RENAMES[item['path']]
    if target is None:continue
    write(stage,target,(stage/item['path']).read_bytes());item['path']=target
   files.append(item)
  # Keep only manifest files so the package's unlisted-file guard is exercised.
  keep={item['path'] for item in files}|{'package-manifest.json'}
  for p in stage.rglob('*'):
   if p.is_file() and p.relative_to(stage).as_posix() not in keep:p.unlink()
  manifest['files']=files;(stage/'package-manifest.json').write_text(json.dumps(manifest))
  ownership=json.loads((stage/'Templates/owned-payloads.json').read_text())
  ownership['standalone']=json.loads((ROOT/'Packaging/StandalonePayloads.json').read_text())['files']
  for item in ownership['current']:
   if item['path'] in RENAMES:item['path']=RENAMES[item['path']]
  (stage/'Templates/owned-payloads.json').write_text(json.dumps(ownership))
  (stage/'Install.ps1').write_bytes((ROOT/'Packaging/Install.ps1').read_bytes());refresh(stage)
 manifest=json.loads((stage/'package-manifest.json').read_text());assert manifest['version']=='1.0.0','Stage 1.0.0 first, or use --synthetic for development only'
 assert 'BeyondTheFieldNative-v103.dll' in ''.join(x['path'] for x in manifest['files'])
 assert sorted(p.name for p in (stage/'Plugins').iterdir() if p.is_file())==['DynastyCOT.dll'],'Loose Plugins payload: only the shared DLL belongs at the root'
 extractor=base/'ExtractAtmosphereFixture.exe'
 subprocess.run([r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe','/nologo','/out:'+str(extractor),str(ROOT/'Tests/ExtractAtmosphereFixture.cs')],check=True)
 checks=0
 def check(value,message):
  nonlocal checks
  assert value,message;checks+=1
 def run(root,package=stage,success=True,extra=()):
  result=subprocess.run([os.environ.get('COT_TEST_POWERSHELL','pwsh'),'-NoProfile','-NonInteractive','-File',str(package/'Install.ps1'),'-MmcRoot',str(root),'-PackageRoot',str(package),'-Fixture','-FixtureRoot',str(base),*extra],capture_output=True,text=True)
  (base/f'{root.name}-{checks}.log').write_text(result.stdout+result.stderr)
  check((result.returncode==0)==success,result.stdout+result.stderr);return result.stdout+result.stderr
 def seed(name):
  root=base/name;root.mkdir();write(root,'.cot-fixture',b'isolated');write(root,'MMCModManager.exe',b'never executed')
  run(root,old);subprocess.run([str(extractor),str(root),str(base)],check=True)
  for path,data in DATA.items():write(root,path,data)
  return root
 def data_intact(root):
  for path,data in DATA.items():check((root/path).read_bytes()==data,'user data changed: '+path)
 root=seed('Upgrade019');before=snapshot(root);run(root)
 for previous,current in RENAMES.items():
  check(not (root/previous).exists(),'old payload remains: '+previous)
  if current and not current.startswith('DGA-Runtime'):check((root/current).is_file(),'renamed payload missing: '+current)
 data_intact(root)
 receipt=json.loads((root/'Plugins/DynastyCOT/installed-files.json').read_text())
 check(not set(RENAMES)&{x['path'] for x in receipt['files']},'retired paths remain in receipt')
 installed=snapshot(root);run(root);check(snapshot(root)==installed,'repeat upgrade changes files')
 backups=sorted((root/'Plugins/DynastyCOT/Backups').iterdir());upgrade_backup=next(p for p in backups if any(x['path'] in RENAMES and x['installed'] is None for x in json.loads((p/'transaction.json').read_text())['operations']))
 run(root,extra=('-Rollback',str(upgrade_backup)));check(snapshot(root)==before,'rename rollback does not restore 0.1.19 exactly')
 for fault in ['AfterPayload','AfterLegacyDisable','AfterAdapter']:
  root=seed('Fault-'+fault);before=snapshot(root);run(root,success=False,extra=('-FaultAtStage',fault));check(snapshot(root)==before,'partial rename failure did not restore: '+fault)
 root=seed('ChangedInactive');path=next(iter(RENAMES));write(root,path,b'changed detached DLL');before=sha(root/path);message=run(root)
 check(sha(root/path)==before and 'Kept changed or unknown inactive legacy payload' in message,'changed detached binary removed silently');data_intact(root)
 for path in ['Plugins/DynastyEnhancer.dll','Plugins/DynastyEnhancerNative-unknown.dll','Plugins/DynamicGamedayAtmosphere/Runtime-x/DynamicAttendanceNative-unknown.dll','Plugins/PlayerAnalytics.dll']:
  root=seed('ActiveUnknown-'+str(checks));write(root,path,b'unknown active owner');before=snapshot(root);message=run(root,success=False)
  check(snapshot(root)==before and 'Unrecognized active legacy payload kept' in message,'unknown active owner not rejected before mutation: '+path)
 # Explicit synthetic trust metadata tests standalone recognition without
 # redistributing old standalone binaries or weakening production hash guards.
 trusted=base/'TrustedStandaloneFixture';shutil.copytree(stage,trusted)
 path='Plugins/DynastyEnhancer.dll';known=b'fixture standalone adapter';ownership=json.loads((trusted/'Templates/owned-payloads.json').read_text())
 ownership.setdefault('standalone',[]).append(dict(path=path,module='DynastyEnhancer',sha256=hashlib.sha256(known).hexdigest()))
 (trusted/'Templates/owned-payloads.json').write_text(json.dumps(ownership));refresh(trusted)
 root=seed('KnownStandalone');write(root,path,known);before=snapshot(root);run(root,trusted);check(not (root/path).exists(),'known standalone not retired')
 backup=sorted((root/'Plugins/DynastyCOT/Backups').iterdir())[-1];run(root,extra=('-Rollback',str(backup)));check(snapshot(root)==before,'known standalone rollback differs')
 root=seed('ChangedStandalone');write(root,path,known+b'changed');before=snapshot(root);run(root,trusted,success=False);check(snapshot(root)==before,'changed standalone removed')
 # Actual public GDA 2.0.35: validate the tagged catalog ZIP hash, then extract
 # managed resources without starting any module/helper/native engine.
 public=json.loads((ROOT/'Packaging/StandalonePayloads.json').read_text())
 provenance=next(x for x in public['provenance'] if x['id']=='GDA-2.0.35')
 check(sha(args.gda_archive)==provenance['archiveSha256'],'GDA release ZIP does not match pinned public reference')
 with zipfile.ZipFile(args.gda_archive) as archive:
  adapter=archive.read('DynamicGamedayAtmosphere.dll');loader=archive.read('DynamicGamedayAtmosphere/Runtime-2.0.35/LoadDynamicGamedayAtmosphere.exe')
 resources=base/'GdaResources';write(resources,'.cot-fixture',b'isolated');write(resources,'Plugins/DynastyCOT.dll',adapter)
 subprocess.run([str(extractor),str(resources),str(base)],check=True)
 native=(resources/'DGA-Runtime/Runtime-2.0.35/DynamicGamedayAtmosphereNative-2.0.35.dll').read_bytes()
 diagnostics=(resources/'DGA-Runtime/Runtime-2.0.35/DgaDiagnostics.exe').read_bytes()
 actual={'DynamicGamedayAtmosphere.dll':adapter,'LoadDynamicGamedayAtmosphere.exe':loader,'DynamicGamedayAtmosphereNative-2.0.35.dll':native,'DgaDiagnostics.exe':diagnostics}
 rows=[x for x in public['files'] if x['release']=='GDA-2.0.35']
 for entry in rows:check(hashlib.sha256(actual[Path(entry['path']).name]).hexdigest()==entry['sha256'],'GDA resource hash differs: '+entry['path'])
 for fault in [None,'AfterLegacyDisable']:
  root=base/('Gda2035-'+str(fault));write(root,'.cot-fixture',b'isolated');write(root,'MMCModManager.exe',b'never executed')
  for relative,data in DATA.items():write(root,relative,data)
  for entry in rows:write(root,entry['path'],actual[Path(entry['path']).name])
  before=snapshot(root);run(root,success=fault is None,extra=('-FaultAtStage',fault) if fault else ())
  if fault:check(snapshot(root)==before,'GDA2035 failed update lost public files/settings')
  else:
   for entry in rows:check(not (root/entry['path']).exists(),'known public GDA payload remains: '+entry['path'])
   data_intact(root);backup=sorted((root/'Plugins/DynastyCOT/Backups').iterdir())[-1]
   run(root,extra=('-Rollback',str(backup)));check(snapshot(root)==before,'GDA2035 rollback differs')
 # Exact log migration is collision-safe and user-data rollback preserves edits.
 root=seed('LegacyLogs');write(root,'Plugins/DynastyEnhancer-launcher.log',b'old launcher bytes');write(root,'Plugins/DynastyEnhancer-launcher.log.1',b'old rotated bytes')
 write(root,'Plugins/DynastyEnhancer/Logs/legacy-launcher.log',b'already archived different bytes');write(root,'Plugins/unrelated.log',b'other log')
 before=snapshot(root);run(root)
 archive='Plugins/DynastyEnhancer/Logs/legacy-launcher-'+hashlib.sha256(b'old launcher bytes').hexdigest()+'.log'
 check((root/archive).read_bytes()==b'old launcher bytes','colliding legacy log lost')
 check((root/'Plugins/DynastyEnhancer/Logs/legacy-launcher.log').read_bytes()==b'already archived different bytes','existing legacy log overwritten')
 check((root/'Plugins/DynastyEnhancer/Logs/legacy-launcher.log.1').read_bytes()==b'old rotated bytes','rotated legacy log lost')
 check(not (root/'Plugins/DynastyEnhancer-launcher.log').exists() and not (root/'Plugins/DynastyEnhancer-launcher.log.1').exists(),'loose legacy logs retained')
 check((root/'Plugins/unrelated.log').read_bytes()==b'other log','unrelated log moved')
 backup=sorted((root/'Plugins/DynastyCOT/Backups').iterdir())[-1];run(root,extra=('-Rollback',str(backup)));check(snapshot(root)==before,'legacy log rollback differs')
 root=seed('UninstallAfterUpgrade');run(root);data_intact(root);before=snapshot(root)
 remove=dict(DynastyEnhancer=False,GamedayAtmosphere=False,ExtraDynastySettings=False,DrivePractice=True)
 run(root,extra=('-Action','UninstallSelected','-ModuleChoicesJson',json.dumps(remove)))
 check(not (root/'Plugins/SituationalPractice/ExpandedPractice.Runtime.exe').exists(),'selected branded practice helper retained')
 backup=sorted((root/'Plugins/DynastyCOT/Backups').iterdir())[-1];run(root,extra=('-Rollback',str(backup)));check(snapshot(root)==before,'selected removal rollback differs')
 run(root,extra=('-Action','UninstallAll'));check(not (root/'Plugins/DynastyCOT.dll').exists(),'full uninstall retained host')
 for path,data in DATA.items():
  if path!='Plugins/DynastyCOT/settings.json':check((root/path).read_bytes()==data,'full uninstall lost data: '+path)
 report=dict(checks=checks,fromVersion='0.1.19-private',toVersion='1.0.0',synthetic=args.synthetic,fixtureRoot=str(base),liveInstall=False,rollback=True,faultStages=3,hashProvenRetirement=True,unknownActiveRejectedBeforeMutation=True,dataPathsPreserved=True,publicGda2035=True,legacyLogMigration=True,pluginsRootOnlySharedDll=True)
 (ROOT/'Build/runtime-upgrade-verification.json').write_text(json.dumps(report,indent=2)+'\n')
 print('PASS:',checks,'runtime upgrade checks; synthetic=',args.synthetic,'no live installation')
if __name__=='__main__':main()
