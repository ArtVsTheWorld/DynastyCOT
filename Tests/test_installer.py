"""Transactional installer tests. Only fixture roots under Build are selected."""
from pathlib import Path
import json,hashlib,subprocess,datetime,shutil,sys,os,zipfile
ROOT=Path(__file__).resolve().parents[1]
META=json.loads((ROOT/'Build/current-package.json').read_text())
PKG=Path(META['stage'])
BASE=ROOT/'Build'/('InstallerChecks-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'))
BASE.mkdir()
extractor=BASE/'ExtractAtmosphereFixture.exe'
subprocess.run([r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe','/nologo','/platform:x64','/out:'+str(extractor),str(ROOT/'Tests/ExtractAtmosphereFixture.cs')],check=True)
checks=0
def check(ok,message):
 global checks
 assert ok,message
 checks+=1
def write(root,path,data):
 p=root/path;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data)
def snapshot(root):
 return {p.relative_to(root).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in root.rglob('*')
         if p.is_file() and 'Backups' not in p.parts}
def seed(name,existing=True):
 root=BASE/name;root.mkdir();write(root,'MMCModManager.exe',b'COT fixture, never executed');write(root,'.cot-fixture',b'isolated')
 write(root,'Plugins/OtherOverhaul.dll',b'unrelated overhaul plugin')
 write(root,'Plugins/OtherOverhaul/settings.json',b'unrelated settings')
 if existing:
  for name in ['DynastyEnhancer.dll','DynamicGamedayAtmosphere.dll','OfflineDynastySettings.dll','SituationalPractice.dll']:
   write(root,'Plugins/'+name,('old '+name).encode())
  write(root,'Plugins/DynastyEnhancer/Runtime-v103/DynastyEnhancerNative-v103.dll',b'old native payload')
  write(root,'Plugins/DynamicGamedayAtmosphere/Runtime-1.54.0/DynamicGamedayAtmosphereNative-1.54.0.dll',b'old atmosphere payload')
  write(root,'Plugins/DynastyCOT.dll',b'previous COT adapter')
  write(root,'COT-Runtime/DynastyEnhancer/DynastyEnhancerNative-v103.dll',b'previous COT runtime')
  write(root,'Plugins/DynastyEnhancer/Runtime-v103/RuntimeHistory/101/history.json',b'news history')
  write(root,'Plugins/DynastyEnhancer/Runtime-v103/RecruitResumeHistory/101.json',b'recruit history')
  write(root,'Plugins/DynastyEnhancer/Runtime-v103/DynastyEnhancementSettings/101.json',b'old unreleased settings')
  write(root,'Plugins/DynamicGamedayAtmosphere/History/audience.txt',b'audience history')
  write(root,'Plugins/OfflinePlaycallSettings/dynasties.json',b'playcall profiles')
  write(root,'Plugins/DynastyCOT/settings.json',b'{"format":1,"modules":{"DrivePractice":false}}')
 return root
def run(root,*extra,success=True,package=PKG):
 command=[os.environ.get('COT_TEST_POWERSHELL','pwsh'),'-NoProfile','-NonInteractive','-ExecutionPolicy','Bypass','-File',str(package/'Install.ps1'),'-MmcRoot',str(root),
          '-PackageRoot',str(package),'-Fixture','-FixtureRoot',str(BASE),*extra]
 environment=os.environ.copy()
 if Path(command[0]).name.lower()=='powershell.exe':environment={k:v for k,v in environment.items() if k.casefold()!='psmodulepath'}
 result=subprocess.run(command,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,env=environment)
 (BASE/(root.name+'-'+str(checks)+'.log')).write_text(result.stdout)
 check((result.returncode==0)==success,root.name+': '+result.stdout)
 return result.stdout
# Fresh install, module defaults and exact host.
fresh=seed('Fresh',False);run(fresh)
manifest=json.loads((PKG/'package-manifest.json').read_text())
host=next(f for f in manifest['files'] if f['path']=='Plugins/DynastyCOT.dll')
check(hashlib.sha256((fresh/host['path']).read_bytes()).hexdigest()==host['sha256'],'fresh host differs')
check(all(json.loads((fresh/'Plugins/DynastyCOT/settings.json').read_text())['modules'].values()),'fresh module defaults')
receipt=json.loads((fresh/'Plugins/DynastyCOT/installed-files.json').read_text())
check({'Plugins/DynastyCOT.dll','COT-Runtime/Shared/node.exe','Plugins/OfflineDynastySettings/OfflineDynastySettings.Persistence.exe','Plugins/SituationalPractice/SituationalPractice.Runtime.exe'}.issubset({f['path'] for f in receipt['files']}),'receipt omitted module payloads')
# Migration, user choices, history and repeat update.
upgrade=seed('Upgrade');before=snapshot(upgrade);run(upgrade)
for name in ['DynastyEnhancer.dll','DynamicGamedayAtmosphere.dll','OfflineDynastySettings.dll','SituationalPractice.dll']:
 check(not (upgrade/'Plugins'/name).exists(),'standalone loader retained')
check(sorted(p.name for p in (upgrade/'Plugins').rglob('*.dll'))==['DynastyCOT.dll','OtherOverhaul.dll'],'legacy native DLLs remain in recursive scan')
check((upgrade/'COT-Runtime/DynastyEnhancer/RuntimeHistory/101/history.json').read_bytes()==b'news history','news not migrated')
check((upgrade/'COT-Runtime/DynastyEnhancer/RecruitResumeHistory/101.json').read_bytes()==b'recruit history','recruits not migrated')
check(not (upgrade/'COT-Runtime/DynastyEnhancer/DynastyEnhancementSettings/101.json').exists(),'legacy Enhancer settings imported')
check((upgrade/'Plugins/DynastyCOT/settings.json').read_bytes()==b'{"format":1,"modules":{"DrivePractice":false}}','module choices reset')
for path in ['Plugins/DynamicGamedayAtmosphere/History/audience.txt','Plugins/OfflinePlaycallSettings/dynasties.json','Plugins/OtherOverhaul.dll','Plugins/OtherOverhaul/settings.json']:
 check(snapshot(upgrade)[path]==before[path],'existing module data changed')
installed=snapshot(upgrade);run(upgrade);check(snapshot(upgrade)==installed,'repeat installation changed user files')
write(upgrade,'COT-Runtime/DynastyEnhancer/RuntimeHistory/101/history.json',b'newer COT history')
before=snapshot(upgrade);run(upgrade);check(snapshot(upgrade)==before,'update conflicted with or reset newer COT history')
# Integrated Analytics retires only its established loader/runtime, in the
# existing transaction. Keep its exported data and unrelated similarly named DLLs.
root=seed('AnalyticsMigration')
analytics_paths=['Plugins/PlayerAnalytics.dll','Plugins/PlayerAnalytics/Runtime-v0.1/PlayerAnalyticsNative-v0.1.dll']
for path in analytics_paths:write(root,path,b'previous standalone Analytics')
write(root,'Plugins/PlayerAnalytics/Runtime-v0.1/analytics-data.json',b'preserved Analytics data')
write(root,'Plugins/PlayerAnalytics/Runtime-v0.1/PlayerAnalyticsExperimental.dll',b'unrelated experimental plugin')
before=snapshot(root);run(root)
for path in analytics_paths:check(not (root/path).exists(),'standalone Analytics hook owner retained')
check((root/'Plugins/PlayerAnalytics/Runtime-v0.1/analytics-data.json').read_bytes()==b'preserved Analytics data','Analytics data deleted')
check((root/'Plugins/PlayerAnalytics/Runtime-v0.1/PlayerAnalyticsExperimental.dll').exists(),'unknown Analytics DLL removed')
backup=sorted((root/'Plugins/DynastyCOT/Backups').iterdir())[0];run(root,'-Rollback',str(backup));check(snapshot(root)==before,'Analytics migration rollback changed old data/owners')
root=seed('AnalyticsDisabled');write(root,'Plugins/PlayerAnalytics.dll',b'standalone owner')
off_choices={'DynastyEnhancer':False,'GamedayAtmosphere':True,'ExtraDynastySettings':True,'DrivePractice':True}
run(root,'-ModuleChoicesJson',json.dumps(off_choices));check((root/'Plugins/PlayerAnalytics.dll').read_bytes()==b'standalone owner','Beyond the Field Off retired standalone Analytics')
root=seed('AnalyticsFailure');write(root,'Plugins/PlayerAnalytics.dll',b'standalone owner');before=snapshot(root);run(root,'-FaultAtStage','AfterLegacyDisable',success=False);check(snapshot(root)==before,'failed Analytics migration lost its old loader')
# Failures at three commit boundaries restore every original managed/data file.
for fault in ['AfterPayload','AfterLegacyDisable','AfterAdapter']:
 root=seed('Failure-'+fault);before=snapshot(root);run(root,'-FaultAtStage',fault,success=False)
 check(snapshot(root)==before,'failed transaction did not restore '+fault)
# Conflicting history is detected before changing adapters.
root=seed('Conflict');write(root,'COT-Runtime/DynastyEnhancer/RuntimeHistory/101/history.json',b'newer history');before=snapshot(root)
run(root,success=False);check(snapshot(root)==before,'conflicting history was overwritten')
# Manual rollback preserves later user edits and newly observed history.
root=seed('Rollback');before=snapshot(root);run(root)
backup=sorted((root/'Plugins/DynastyCOT/Backups').iterdir())[0]
write(root,'COT-Runtime/DynastyEnhancer/RuntimeHistory/101/history.json',b'later observed news')
write(root,'COT-Runtime/DynastyEnhancer/RuntimeHistory/101/later.json',b'new edition')
run(root,'-Rollback',str(backup))
after=snapshot(root)
for key,value in before.items():check(after.get(key)==value,'manual rollback did not restore '+key)
check((root/'COT-Runtime/DynastyEnhancer/RuntimeHistory/101/history.json').read_bytes()==b'later observed news','later history edit lost')
check((root/'COT-Runtime/DynastyEnhancer/RuntimeHistory/101/later.json').read_bytes()==b'new edition','new history lost')
# A changed managed file makes manual rollback stop before touching the fixture.
root=seed('ForeignReplacement');run(root)
backup=sorted((root/'Plugins/DynastyCOT/Backups').iterdir())[0]
write(root,'Plugins/DynastyCOT.dll',b'later managed replacement');before=snapshot(root)
run(root,'-Rollback',str(backup),success=False);check(snapshot(root)==before,'rollback changed a foreign replacement')
# A user-edited config created by a clean install survives manual rollback.
root=seed('ConfigRollback',False);run(root)
backup=sorted((root/'Plugins/DynastyCOT/Backups').iterdir())[0]
write(root,'Plugins/DynastyCOT/settings.json',b'{"format":1,"modules":{"DrivePractice":false}}')
run(root,'-Rollback',str(backup));check((root/'Plugins/DynastyCOT/settings.json').read_bytes()==b'{"format":1,"modules":{"DrivePractice":false}}','later module choice lost')
# GUI module selections are part of the same backed-up transaction.
choices={'DynastyEnhancer':True,'GamedayAtmosphere':False,'ExtraDynastySettings':True,'DrivePractice':True}
root=seed('GuiChoices');before=snapshot(root)
run(root,'-ModuleChoicesJson',json.dumps(choices,separators=(',',':')))
check(json.loads((root/'Plugins/DynastyCOT/settings.json').read_text())['modules']==choices,'GUI module choices not saved')
backup=sorted((root/'Plugins/DynastyCOT/Backups').iterdir())[0]
run(root,'-Rollback',str(backup));check(snapshot(root)==before,'GUI choices rollback differs')
root=seed('GuiChoicesFailure');before=snapshot(root)
run(root,'-ModuleChoicesJson',json.dumps(choices,separators=(',',':')),'-FaultAtStage','AfterPayload',success=False)
check(snapshot(root)==before,'failed GUI install changed prior module choices')
root=seed('InvalidGuiChoices');before=snapshot(root)
bad_choices=dict(choices,DrivePractice='true')
run(root,'-ModuleChoicesJson',json.dumps(bad_choices,separators=(',',':')),success=False)
check(snapshot(root)==before,'invalid GUI module choices changed target')
# Partial/full removal, undo and unknown-file preservation.
remove={id:False for id in choices};remove['DrivePractice']=True
root=seed('RemoveSelected');run(root);before=snapshot(root)
run(root,'-Action','UninstallSelected','-ModuleChoicesJson',json.dumps(remove))
check(not (root/'Plugins/SituationalPractice/SituationalPractice.Runtime.exe').exists(),'selected helper retained')
check((root/'Plugins/DynastyCOT.dll').exists() and (root/'Plugins/OfflineDynastySettings/OfflineDynastySettings.Persistence.exe').exists(),'other modules removed')
check(not json.loads((root/'Plugins/DynastyCOT/settings.json').read_text())['modules']['DrivePractice'],'removed module not disabled')
check((root/'COT-Runtime/DynastyEnhancer/RuntimeHistory/101/history.json').read_bytes()==b'news history','removal lost history')
backup=sorted((root/'Plugins/DynastyCOT/Backups').iterdir())[-1]
run(root,'-Rollback',str(backup));check(snapshot(root)==before,'partial removal rollback differs')
for fault in ['AfterPayload','AfterAdapter']:
 run(root,'-Action','UninstallAll','-FaultAtStage',fault,success=False)
 check(snapshot(root)==before,'failed removal did not restore '+fault)
# Extract the actual embedded payload without starting any engines.
ownership=json.loads((PKG/'Templates/owned-payloads.json').read_text())
subprocess.run([str(extractor),str(root),str(BASE)],check=True)
for embedded in ownership['current']:
 check(hashlib.sha256((root/embedded['path']).read_bytes()).hexdigest()==embedded['sha256'],'embedded fixture differs')
# A foreign replacement must survive; its module is still disabled.
write(root,'Plugins/SituationalPractice/SituationalPractice.Runtime.exe',b'changed helper')
write(root,'COT-Runtime/DynastyEnhancer/RuntimeHistory/101/later.json',b'later history')
write(root,'DGA-Runtime/Runtime-2.1.9/unknown.txt',b'unrelated runtime neighbor')
run(root,'-Action','UninstallAll')
check(not (root/'Plugins/DynastyCOT.dll').exists(),'full removal kept host')
check(not (root/'COT-Runtime/Shared/node.exe').exists(),'full removal kept bundled node')
check(all(not (root/f['path']).exists() for f in ownership['current']),'full removal kept extracted Atmosphere payload')
check((root/'Plugins/SituationalPractice/SituationalPractice.Runtime.exe').read_bytes()==b'changed helper','removal deleted changed helper')
check((root/'Plugins/OtherOverhaul.dll').read_bytes()==b'unrelated overhaul plugin','removal changed another mod')
check((root/'DGA-Runtime/Runtime-2.1.9/unknown.txt').read_bytes()==b'unrelated runtime neighbor','removal swept neighbor')
check(not any(json.loads((root/'Plugins/DynastyCOT/settings.json').read_text())['modules'].values()),'full removal left modules on')
check((root/'COT-Runtime/DynastyEnhancer/RuntimeHistory/101/later.json').read_bytes()==b'later history','removal lost later history')
run(root);check((root/'Plugins/DynastyCOT.dll').exists(),'reinstall after removal failed')
check(not any(json.loads((root/'Plugins/DynastyCOT/settings.json').read_text())['modules'].values()),'reinstall reset user module choices')
root=seed('EmptyRemoval',False);before=snapshot(root)
run(root,'-Action','UninstallSelected','-ModuleChoicesJson',json.dumps({id:False for id in choices}),success=False)
check(snapshot(root)==before,'empty removal changed target')
# A corrupt receipt cannot turn history or other mods into owned payloads.
root=seed('UnsafeReceipt');run(root)
write(root,'Plugins/DynastyCOT/installed-files.json',json.dumps(dict(format=1,files=[dict(path='Plugins/OtherOverhaul.dll',module='Core',sha256='0'*64)])).encode())
before=snapshot(root);run(root,'-Action','UninstallAll',success=False);check(snapshot(root)==before,'unsafe receipt changed target')
# The previous public build predates receipts. Remove exact known 0.1.1
# payloads, including its different embedded Atmosphere build, with no install first.
root=seed('LegacyRemoval',False)
known={(f['path'],f['sha256']) for f in ownership['legacy']}
with zipfile.ZipFile(ROOT/'Release/DynastyCOT-0.1.1-private.zip') as old:
 for f in json.loads(old.read('package-manifest.json'))['files']:
  if (f['path'],f['sha256']) in known:write(root,f['path'],old.read(f['path']))
subprocess.run([str(extractor),str(root),str(BASE)],check=True)
write(root,'COT-Runtime/DynastyEnhancer/RuntimeHistory/101/history.json',b'old edition')
run(root,'-Action','UninstallAll')
check(not (root/'Plugins/DynastyCOT.dll').exists() and not (root/'COT-Runtime/Shared/node.exe').exists(),'legacy no-receipt removal failed')
check(not any((root/'DGA-Runtime').rglob('*.dll')),'legacy embedded runtime retained')
check((root/'COT-Runtime/DynastyEnhancer/RuntimeHistory/101/history.json').read_bytes()==b'old edition','legacy history lost')
# Tamper and traversal reject before target writes.
bad=BASE/'BadPackage';shutil.copytree(PKG,bad);helper=bad/'Plugins/OfflineDynastySettings/OfflineDynastySettings.Persistence.exe'
original=helper.read_bytes();helper.write_bytes(original+b'bad')
root=seed('Tamper');before=snapshot(root);run(root,success=False,package=bad);check(snapshot(root)==before,'tampered package changed target')
helper.write_bytes(original)
path=bad/'package-manifest.json';doc=json.loads(path.read_text());doc['files'][0]['path']='../outside';path.write_text(json.dumps(doc))
root=seed('Traversal');before=snapshot(root);run(root,success=False,package=bad);check(snapshot(root)==before,'traversal changed target')
report=dict(atUtc=datetime.datetime.now(datetime.timezone.utc).isoformat(),checks=checks,fixtureRoot=str(BASE),packageStage=str(PKG),manifestSha256=hashlib.sha256((PKG/'package-manifest.json').read_bytes()).hexdigest(),
 cleanInstall=True,update=True,migration=True,automaticRollbackStages=3,manualRollback=True,
 laterHistoryPreserved=True,tamperRejection=True,traversalRejection=True,guiModuleChoices=True,
 powershell=os.environ.get('COT_TEST_POWERSHELL','pwsh'),selectedUninstall=True,fullUninstall=True,uninstallRollback=True,legacyNoReceiptRemoval=True,embeddedPayloadRemoval=True,liveInstall=False)
(ROOT/'Build/installer-verification.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS:',checks,'installer checks; clean/update, migration, selected/all uninstall including legacy/extracted payloads, rollback, changed-file preservation and tamper/path rejection. Fixtures only.')
