"""Require exact file/network/layout/released-migration evidence for handoff."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads((ROOT/p).read_text(encoding='utf-8'))
def main():
 build=read('Build/current-collection-setup.json');offline=read('Build/collection-verification.json');network=read('Build/collection-network-verification.json');migration=read('Build/released-migration-verification.json');exe=Path(build['exe']);version=build['version']
 assert sha(exe)==build['sha256']==offline['exeSha256']==network['exeSha256']==migration['exeSha256']
 for report in [offline,network,migration]:
  assert not report['liveInstall'] and not report.get('published',False)
  assert report['sources']==build['sources']
  for rel,digest in dict(report['sources'],**report['tests']).items():assert sha(ROOT/rel)==digest,'Changed source/test: '+rel
 count=int(re.search(r'PASS (\d+) ',offline['fileChecks'])[1]);assert count>=230
 assert '100/125/150' in offline['layout'] and network['anonymous'] and network['moduleCount']==4
 assert 'fixture install, independent GDA version change and rollback; BTF unchanged' in network['results']
 assert migration['installerVersion']==version and migration['actualReleasedInstallers'] and migration['actualReleasedStandalonePreparation'] and not migration['gameOrManagerLaunched']
 cases=migration['cases'];assert len(cases)==9 and {x['oldVersion'] for x in cases}=={'1.0.0','1.0.1','1.0.2'}
 for case in cases:assert all(case[k] for k in ['migration','coldSupportPreparation','rollbackHostIntegrity','repeatMigration','partialRemoveRestore','userDataPreserved'])
 catalog=read('Packaging/Collection/catalog.json')
 proof=dict(product='Dynasty COT collection installer',version=version,privateCandidate='-' in version,publicationStatus='built and verified before upload',installedIntoLiveMmc=False,sourceRevision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),exeSha256=sha(exe),exeBytes=exe.stat().st_size,catalogSha256=sha(ROOT/'Packaging/Collection/catalog.json'),sourceHashes=build['sources'],testHashes=dict(offline['tests'],**network['tests'],**migration['tests']),fileChecks=count,releasedMigrationChecks=migration['checks'],releasedMigrationCases=cases,layout=offline['layout'],anonymousNetwork='passed discovery/download/four-module fixture install, independent GDA older-version selection and rollback; BTF untouched',modules=[dict(id=m['Id'],version=m['Current']['Version'],dllSha256=m['Current']['DllSha256'],archiveSha256=m['Current']['ArchiveSha256'],repository=m['Repository']) for m in catalog['Modules']],nativeModuleChanges=False,installsDynastyCotDll=False,userAcceptance='Ace reported the collection installer install works; authorized publication after older-COT migration verification.',remainingLiveAcceptance=['Native game activation and rollback after older-COT migration on additional real installations','Other Windows permissions/scaling setups and supported-overhaul combinations'],manualUi='Observed MMC auto-detection of all four current standalones, compact main window, per-module release chooser and modern executable picker. Picker cancellation could not be completed through the computer-use parent-window bounds; test preview was closed without installing. Notes line breaks corrected with regression coverage.')
 (exe.with_name(exe.stem+'-verification.json')).write_text(json.dumps(proof,indent=2)+'\n',encoding='utf-8')
 (exe.with_name(exe.stem+'-SHA256.txt')).write_text(sha(exe)+'  '+exe.name+'\n',encoding='utf-8')
 text='''DYNASTY COT by Ace — collection installer '''+version+'''
Collection installer. No game-module code changes in this release.

This small installer manages the same public standalone DLLs. It does not install
DynastyCOT.dll. Check the modules you want, verify the detected MMC location and
choose Install / Update. Close MMC and the game before changing files.

It downloads only selected modules that need changing. Check for updates lets
you read module release notes and choose versions; it does not download ZIPs.
Use Remove for selected/full removal and Backups to restore prior versions.
Settings, accounts, saves and published history are kept. New program downloads
require Internet access. Existing collection backups restore without Internet.

Older COT migration: keep every enabled old-COT module selected, or explicitly
remove an unwanted module first. The old host and its matching program files
are backed up before it is replaced by standalone DLLs. Missing/unrecognized
old receipts/code stop for review. Keep all existing data and backup folders.

Passed: '''+str(count)+''' isolated file/update/failure/rollback checks; actual window
layout at 100/125/150 percent; real anonymous downloads for all four modules,
fixture install, independent GDA version change and rollback with BTF unchanged.
Also passed: '''+str(migration['checks'])+''' checks across 9 released old-COT migration cases
(1.0.0, 1.0.1, 1.0.2; all modules and partial selections), actual standalone support
file preparation, old host integrity after restore, repeated migration and partial
remove/restore. User reported the collection installer install works. These file
checks do not certify game activation or every other game/mod/Windows setup.

Close-MMC errors require a detected process and include its ID. Permission and
file-lock failures have separate explanations. Old standalone files alone do not
trigger that running-process error. Do not delete settings/history to fix it.

Pinned versions: Beyond the Field 1.0.7; Gameday Atmosphere 2.1.11;
Extra Settings 0.2.10; Expanded Practice 0.8.21.

Collection state/logs/backups: MMC/COT-Collection. Existing module data/log paths
stay unchanged. Full guide and checklist: docs/collection-installer.md in the
DynastyCOT source repository. Integrity metadata is beside this EXE.
'''
 (exe.with_name(exe.stem+'-README.txt')).write_text(text,encoding='utf-8')
 print('Verified collection handoff:',exe.name,'—',exe.stat().st_size,'bytes')
if __name__=='__main__':main()
