"""Require exact file/network/layout evidence, then prepare the private handoff."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads((ROOT/p).read_text(encoding='utf-8'))
def main():
 build=read('Build/current-collection-setup.json');offline=read('Build/collection-verification.json');network=read('Build/collection-network-verification.json');exe=Path(build['exe']);version=build['version']
 assert sha(exe)==build['sha256']==offline['exeSha256']==network['exeSha256']
 for report in [offline,network]:
  assert not report['liveInstall'] and not report['published']
  assert report['sources']==build['sources']
  for rel,digest in dict(report['sources'],**report['tests']).items():assert sha(ROOT/rel)==digest,'Changed source/test: '+rel
 count=int(re.search(r'PASS (\d+) ',offline['fileChecks'])[1]);assert count>=224
 assert '100/125/150' in offline['layout'] and network['anonymous'] and network['moduleCount']==4
 assert 'fixture install, independent GDA version change and rollback; BTF unchanged' in network['results']
 catalog=read('Packaging/Collection/catalog.json')
 proof=dict(product='Dynasty COT collection installer',version=version,privateCandidate=True,published=False,installedIntoLiveMmc=False,sourceRevision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),exeSha256=sha(exe),exeBytes=exe.stat().st_size,catalogSha256=sha(ROOT/'Packaging/Collection/catalog.json'),sourceHashes=build['sources'],testHashes=dict(offline['tests'],**network['tests']),fileChecks=count,layout=offline['layout'],anonymousNetwork='passed discovery/download/four-module fixture install, independent GDA older-version selection and rollback; BTF untouched',modules=[dict(id=m['Id'],version=m['Current']['Version'],dllSha256=m['Current']['DllSha256'],archiveSha256=m['Current']['ArchiveSha256'],repository=m['Repository']) for m in catalog['Modules']],nativeModuleChanges=False,installsDynastyCotDll=False,remainingLiveAcceptance=['Real MMC/game migration and rollback with existing saves/settings/history','Other Windows permissions/scaling setups and supported-overhaul combinations'],manualUi='Observed MMC auto-detection of all four current standalones, compact main window, per-module release chooser and modern executable picker. Picker cancellation could not be completed through the computer-use parent-window bounds; test preview was closed without installing. Notes line breaks corrected with regression coverage.')
 (exe.with_name(exe.stem+'-verification.json')).write_text(json.dumps(proof,indent=2)+'\n',encoding='utf-8')
 (exe.with_name(exe.stem+'-SHA256.txt')).write_text(sha(exe)+'  '+exe.name+'\n',encoding='utf-8')
 text='''DYNASTY COT by Ace — collection installer '''+version+'''
PRIVATE TEST CANDIDATE. Not published. No live MMC installation was performed.

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

Passed: '''+str(count)+''' isolated migration/file/update/failure/rollback checks; actual window
layout at 100/125/150 percent; real anonymous downloads for all four modules,
fixture install, independent GDA version change and rollback with BTF unchanged.
Next: a real MMC/game migration/adoption and rollback acceptance check. Offline
tests and successful detection do not certify live-game or all-overhaul behavior.

Pinned versions: Beyond the Field 1.0.7; Gameday Atmosphere 2.1.11;
Extra Settings 0.2.10; Expanded Practice 0.8.21.

Collection state/logs/backups: MMC/COT-Collection. Existing module data/log paths
stay unchanged. Full guide and checklist: docs/collection-installer.md in the
DynastyCOT source repository. Integrity metadata is beside this EXE.
'''
 (exe.with_name(exe.stem+'-README.txt')).write_text(text,encoding='utf-8')
 print('Verified private collection handoff:',exe.name,'—',exe.stat().st_size,'bytes')
if __name__=='__main__':main()
