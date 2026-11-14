"""Assemble allowlisted public assets and integrity evidence; never install or publish."""
from pathlib import Path
import hashlib,json,zipfile,subprocess
R=Path(__file__).resolve().parents[1];O=R/'Release/Publication-Final'
def read(p):return json.loads(p.read_text(encoding='utf8'))
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def put(p,v):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf8')
def archive(path,files):
 with zipfile.ZipFile(path,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
  for name,data in files.items():
   e=zipfile.ZipInfo(name,(2020,1,1,0,0,0));e.compress_type=zipfile.ZIP_DEFLATED;e.external_attr=0o644<<16;z.writestr(e,data)
 with zipfile.ZipFile(path) as z:assert z.testzip() is None and set(z.namelist())==set(files)
def main():
 setup=read(R/'Build/current-collection-setup.json');checks=read(R/'Build/collection-verification.json');migration=read(R/'Build/released-migration-verification.json');managed=read(R/'Build/gameday-standalone-verification.json');modules=read(R/'Build/standalones-verification.json')
 assert setup['sha256']==sha(Path(setup['exe']))==checks['exeSha256']==migration['exeSha256']
 for p,h in setup['sources'].items():assert sha(R/p)==h,p
 rows=read(O/'modules.json');revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=R,text=True).strip()
 for m in rows:
  proof=read(O/(Path(m['archive']).stem+'-verification.json'));assert proof['sourceRevision']==revision and proof['archiveSha256']==sha(O/m['archive'])
 installer=O/'DynastyCOT-Installer-1.1.5.zip'
 instructions='DYNASTY COT by Ace\n\nExtract both program files together. Close MMC and the game. Run DynastyCOT-Setup-1.1.5.exe, confirm the detected MMC location, choose modules and select Install/Update. Downloads happen only when requested. Backups lets you restore an older installation. User settings and history stay in AppData.\n\nUse installer 1.1.5 for the new guide-bearing releases. Older compatible versions remain available in Updates. Individual DLL installation still works.\n\nhttps://github.com/ArtVsTheWorld/DynastyCOT\n'
 archive(installer,{Path(setup['exe']).name:Path(setup['exe']).read_bytes(),Path(setup['config']).name:Path(setup['config']).read_bytes(),'README.txt':instructions.encode('utf8')})
 guides=O/'DynastyCOT-Player-Guides-1.1.5.zip'
 archive(guides,{m['module']+'-Player-Guide.pdf':(O/(m['module']+'-Player-Guide.pdf')).read_bytes() for m in rows})
 # No captures, SDKs, user paths, histories, private fixture inputs or raw logs in public proof.
 proof=dict(format=2,version='1.1.5',sourceRevision=revision,archiveSha256=sha(installer),exeSha256=setup['sha256'],configSha256=sha(Path(setup['config'])),catalogSha256=setup['catalogSha256'],sourceHashes=setup['sources'],guidesArchiveSha256=sha(guides),modules=rows,verification=dict(gdaNativeSuites=50,gdaManagedChecks=managed['checks'],standaloneChecks=modules['checks'],installerChecks=checks['fileChecks'],dpiLayout=checks['layout'],oldCotMigrationChecks=migration['checks'],oldCotMigrationCases=len(migration['cases']),pdfPagesVisuallyInspected=24),liveInstallPerformed=False,liveAcceptance='Offline checks do not establish installed-menu responsiveness, new crowd layout appearance, live frame time or spring boo audibility. Previously confirmed listening is identified in the module notes.')
 put(O/'DynastyCOT-Installer-1.1.5-verification.json',proof)
 (O/'DynastyCOT-Installer-1.1.5-SHA256.txt').write_text('\n'.join(sha(p)+'  '+p.name for p in [installer,guides])+'\n',encoding='ascii')
 (O/'COLLECTION_RELEASE_NOTES.md').write_text('''# Dynasty COT 1.1.5

Changes since public collection release 1.1.0.

- Four optional standalone modules now appear in MMC as **Ace’s Beyond the Field**, **Ace’s Gameday Atmosphere**, **Ace’s Extra Settings** and **Ace’s Expanded Practice**. DLL names and settings/history identities stay the same.
- Each standalone ZIP has just its DLL, a short README, the complete feature list and an illustrated player guide. Required notices remain in the README where needed. Verification and editable guides are separate release downloads.
- The collection installs the same standalone DLLs. Choose each module and compatible version independently. New guide-bearing ZIPs and older supported formats both work; guides stay outside Plugins and follow uninstall/rollback.
- An explicitly selected MMC executable remains the target. Interrupted restores are recoverable and block conflicting operations. Known old providers migrate transactionally; settings, accounts and published posts are kept.
- The collection front page links to every module, guide and complete feature list. Module release notes explain changes since their previous public releases.

**Existing COT users: download installer 1.1.5 first.** Older installers may not understand the new four-file ZIPs. Extract the installer and its `.exe.config` together, close MMC/game, then run it. Manual users still copy only the selected DLL into Plugins. Keep one provider per feature.

Included versions: Beyond the Field **1.0.24**, Gameday Atmosphere **2.1.38**, Extra Settings **0.2.12**, Expanded Practice **0.8.24**. Check their separate releases for gameplay changes and supported contexts.

Offline verification covered 50 GDA native suites, managed/standalone integration, new/old archives, migration, uninstall and rollback, plus 15 layout checks and 24 visually inspected PDF pages. Public download/install/rollback is checked in isolated fixtures before this collection is published. No live MMC installation or save/history changes were performed for this publication pass.

New crowd-section visuals, live frame-time/menu behavior, spring boo audibility and unsupported third-party combinations still require live acceptance. Full private recruiting context in non-Owner multi-coach views remains limited. Successful startup is not a universal compatibility claim.
''',encoding='utf8')
 print('Public collection assets prepared:',installer)
if __name__=='__main__':main()
