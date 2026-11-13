"""Prepare private handoff files from verified builds; no install or publication."""
from pathlib import Path
import hashlib,json,shutil,statistics,subprocess,zipfile
R=Path(__file__).resolve().parents[1];O=R/'Release/FinalReadiness-Private';E=R/'Build/FinalReleasePass'
def read(p):return json.loads(p.read_text(encoding='utf8'))
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def put(n,v):(O/n).write_text(v if isinstance(v,str) else json.dumps(v,indent=2)+'\n',encoding='utf8')
def main():
 modules=read(O/'modules.json');setup=read(R/'Build/current-collection-setup.json');checks=read(R/'Build/collection-verification.json');migration=read(R/'Build/released-migration-verification.json');storage=read(R/'Build/user-storage-verification.json');signed=read(E/'signing-save-audit.json');pin=read(E/'source-pin-verification.json');pushes=read(E/'source-pushes.json');perf=read(E/'final-formatting-performance.json');ui=read(E/'FinalUIBenchmark/Results/ui-performance.json');revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=R,text=True).strip()
 assert checks['exeSha256']==setup['sha256']==sha(Path(setup['exe']))
 for p,h in setup['sources'].items():assert sha(R/p)==h,p
 engine=['Core/UserStorage.cs','Packaging/Collection/Models.cs','Packaging/Collection/Paths.cs','Packaging/Collection/Installer.cs','Packaging/Collection/Downloads.cs','Packaging/Collection/catalog.json']
 assert all(migration['sources'][p]==sha(R/p) for p in engine)
 assert signed['candidateDetected']==signed['signed']==4421 and signed['oldDetected']==4301 and not signed['problems']
 assert sha(Path('D:/Downloads/DYNASTY-R10YR1NSD'))==signed['saveSha256']
 for m in modules:
  assert storage['modules'][m['module']]['dllSha256']==m['dllSha256']
  p=read(O/(Path(m['archive']).stem+'-verification.json'));assert p['archiveSha256']==sha(O/m['archive']) and p['sourceRevision']==pin['revision']
  assert len((O/(m['module']+'-DISCORD.txt')).read_text())<2000
  assert all(not(x.startswith('- ') and x.endswith('.')) for x in (O/(m['module']+'-RELEASE_NOTES.md')).read_text().splitlines())
 for p in [Path(setup['exe']),Path(setup['config'])]:shutil.copyfile(p,O/p.name)
 timing={}
 for op in ['window','inspect','updates']:
  rows=[x for x in ui['samples'] if x['operation']==op];timing[op]=dict(firstMs=rows[0]['ms'],warmMedianMs=statistics.median(x['ms'] for x in rows[1:]),slowMs=max(x['ms'] for x in rows))
 put('PERFORMANCE.json',dict(finalFormatting=perf,installer=timing,installerPeakBytes=ui['memoryBytes'],liveMenuOrFpsMeasurement=False))
 proof=dict(format=1,private=True,published=False,installedIntoLiveMmc=False,moduleSourceRevision=pin['revision'],installerSourceRevision=revision,compiledInputsMatched=pin['inputs'],installer=dict(version=setup['version'],sha256=setup['sha256'],configSha256=setup['configSha256']),modules=modules,nodeTests=513,gdaNativeSuites=44,gdaManagedChecks=302,standaloneChecks=read(R/'Build/standalones-verification.json')['checks'],collection=checks['fileChecks'],layout=checks['layout'],oldCotMigrationChecks=migration['checks'],oldCotCases=len(migration['cases']),migrationEngineMatchesFinal=True,signing=dict(nativeSigned=4421,oldDetected=4301,newDetected=4421,recovered=120,conflicts=0,duplicates=0,originalSaveSha256=signed['saveSha256'],originalUnchanged=True),remainingImplementation=['Active non-owner coach selector and private Notes ownership'],remainingAcceptance=['Transfer-label fit','Installed Notes/fog/NIL/GPI/navigation','Fresh/saved spring and championship visuals','Audible spring boos','Physical mixed-monitor DPI and live performance'])
 put('VERIFICATION.json',proof)
 put('DynastyCOT-Setup-1.1.4-verification.json',dict(format=1,product='Dynasty COT',version=setup['version'],sourceRevision=revision,sourceHashes=setup['sources'],exeSha256=setup['sha256'],configSha256=setup['configSha256'],catalogSha256=setup['catalogSha256'],privateCandidate=True,published=False))
 put('REPOSITORIES.md','# Reviewed source\n\n'+f'[COT integration/installer](https://github.com/ArtVsTheWorld/DynastyCOT/commit/{revision})\n\n'+f'[Exact module source pin](https://github.com/ArtVsTheWorld/DynastyCOT/commit/{pin["revision"]}) — {pin["inputs"]} compiled inputs match Git bytes\n\n'+'\n'.join('- ['+p['repository']+']('+p['url']+') — candidate notes and source pin' for p in pushes)+'\n\nAll pushes use release-readiness-2026-11-13 review branches. Public releases/latest-download targets were not changed.\n')
 put('SIGNING-RESULTS.md',f'''# Signing detection

The supplied NSD save contains 4,421 Signed Recruit rows. The old interest-leader plus recruiting-board method detects 4,301. Committed-player ownership detects all 4,421, without omissions/extras against those Signed rows, duplicate players or conflicting owners. All 4,301 previously detected destinations agree.

New Mexico State has {len(signed['nmsu'])} signed players. Its three old misses are Augustine Goss, Brady Mott and Grant Leonard. Goss's interest leader is Colorado State; Mott's is North Carolina. Their committed-player membership identifies New Mexico State. Leonard has no unique interest leader.

The container reports 833.0; relevant table headers identify schema 486 and match the 486.6 member definitions. Team has 424 members, Recruit 15 and Player 288. Used board/target tables and SeasonInfo also match. Recruit.Player is Player; Team.CommittedPlayers is Player[]. Every signed player has exactly one owner. No validation was bypassed.

This checkpoint has no HardCommitted/SoftCommitted rows. Verbal/unsigned states, missing/conflicting/duplicate ownership, transfers and cleared rollover lists have regressions; actual other-stage and Year 2 saves were not supplied. A general full-save import still rejects an unrelated historical-series reference. The targeted signing audit validates every reference it uses without suppressing that separate failure.

Published signing facts/words stay frozen; corrections apply to unpublished/new reports. This adds reporting facts, not guaranteed extra posts. Signing decisions, influence, scholarships, rosters, budgets and native game data are unchanged.

Original checkpoint SHA-256: `{signed['saveSha256']}` — unchanged after read-only inspection
''')
 table='| Module | Candidate | Last public comparison |\n|---|---|---|\n'+''.join('| '+m['module']+' | '+m['version']+' | '+m['publicBaseline']+' |\n' for m in modules)
 put('REPORT.md',f'''# Private release-preparation handoff

No live installation, injection, restart, save edit, history clearing, automatic live migration or public-release publication was performed. Reviewed source and documentation are pushed on review branches in COT and all four standalone repositories.

{table}

Collection installer: **1.1.4**. It installs the same independent DLLs, without DynastyCOT.dll. Each module changed since its public baseline; versions were not bumped merely to synchronize them.

## Final fixes

- Transfer labels are compact: `TR (FR RS)` becomes `TR RSFR`; `TR JR` means a non-redshirt junior. Four years, redshirts, missing values and JUCO distinctions have guards. No sticky font/bounds changes are applied to reused widgets. Actual card fit still needs visual acceptance
- Notes hide technical provenance labels and use natural secondary-role wording. Stored statistics, backgrounds, assessments, accuracy categories and internal evidence are unchanged. Scouting/Blind protections remain
- Confirmed signing news follows committed-player ownership, recovering all 120 missed signees in Gris's save, including the three NMSU cases. See SIGNING-RESULTS.md
- ZIPs contain the DLL, short README, complete FEATURES and required BTF licenses. Support prepares automatically in shared AppData; no manual COT-Support arrangement or extracted GDA injector EXE is added
- Setup retains independent updates, compatible public-version selection, safe Markdown, selected/full uninstall and restore. Recognized old providers are backed up outside the plugin scan; unknown bytes still require exact review
- Visual inspection caught a collapsed module area at high scaling. The adaptive layout and stronger regression keep choices usable, with scrolling only in the module area when space requires it. Footer actions remain visible

## Verification

- 513 JavaScript tests pass; focused Notes/class tests and BTF native build fog/interest/startup suites pass
- 44 GDA native suites and 302 managed checks pass without live game calls
- {proof['standaloneChecks']} standalone integration checks and actual-assembly storage tests pass
- 299 collection file/migration/failure/rollback checks pass, including actual public DLL upgrades, independent updates, renamed duplicates, external-support GDA, unknown-file review, permission/lock/collision errors and selected/full removal/restore
- 2,733 checks across nine actual COT 1.0.0/1.0.1/1.0.2 migrations pass, including cold support preparation, exact old-host rollback, repeated migration and preserved data sentinels. Final DLLs were used; later installer-only layout changes leave all tested migration-engine and catalog hashes identical
- Anonymous public discovery/download/install/version-change/rollback passes against existing public releases. Candidate GitHub assets are not published; their future anonymous URLs cannot yet be accepted. The candidate's complete local-artifact install path passes
- 15 actual WinForms fixtures pass at emulated 100/125/150/175/200% and 1280x720, 1366x768, 1920x1080. Rendered 100% and 200% examples were inspected. Physical mixed-monitor movement remains pending
- {pin['inputs']} compiled input files match the exact module source commit. DLL identities/versions/Ace branding, payload hashes and ZIP allowlists are checked. No SDK, save, capture, generated history, credential, development log or test executable is shipped

## Storage and upgrades

Current settings, accounts, Notes, histories, logs and verified support use `%LOCALAPPDATA%\\Ace\\DynastyCOT\\<module>`, shared across MMC copies. Legacy originals remain; explicit AppData preferences win; conflicts are archived. Retry/rollback do not reset saved choices. Collection receipts/backups remain associated with their MMC installation.

Installing DLLs into Program Files still requires permission, but normal saving does not require Plugins write access. Write-denied directories, shared copies, interruptions/retry, public upgrades and both standalone/collection directions are tested. Close-process errors require an actual process/PID; permission, lock and collision failures have separate explanations.

## Performance

Five alternating self-process samples compare identical final formatting inputs against 1.0.22. Rendering 30,000 Notes takes median 144.3 → 152.2 ms, about 0.26 microseconds extra per report. One million transfer labels take 9.7 → 18.2 ms. Peak working set stays around 5 MiB. CPU timing has Windows sampling granularity. These are formatting costs, not live menu latency.

The preceding preserved 14-edition benchmark measured existing-history Week 12 at 9.77 → 6.90 seconds and mature history at 3.12 → 3.02 seconds, retaining current published output. Cold reconstruction intentionally does more work. Those are offline process timings, excluding native capture/settling/approval/UI. This final signing correction is not a new generation-speed claim.

Current installed logs show example generator pipelines near 2.2 seconds for an opening edition and 4.1–4.3 seconds for Week 2; they are not controlled candidate measurements. PERFORMANCE.json includes current installer cold/warm window, inspection and update-view timings. No new live FPS/menu benchmark was performed.

## Remaining work versus acceptance

**Implementation limitation:** full multi-coach active-school selection and school-private Notes ownership remain unresolved outside the verified Owner-school perspective. Georgia/Owner was confirmed in the three-coach save; that does not establish Pittsburgh/East Point switching. Do not advertise full multi-coach support. Public-history recovery remains included without inventing uncaptured private actions.

**Acceptance tests:** actual transfer-label fit, installed Notes/fog/NIL/GPI/navigation, fresh/saved spring/championship visuals, audible boos, physical mixed-DPI movement and live performance need the checklist. Dispatch/voice registration is not proof of audible playback. Old stored backgrounds deliberately are not rerolled.

Source branches are pushed. Public release assets and latest pointers remain unchanged. Keep the installer EXE and matching EXE.config together; this private kit does so. Per-module GitHub notes and Discord drafts are beside this report.
''')
 put('README-TEST-KIT.txt','DYNASTY COT private test kit by Ace\n\nExtract the entire ZIP. When ready, close MMC/game and run DynastyCOT-Setup-1.1.4.exe. Keep its .exe.config beside it. Local module ZIPs and verification files let setup use these private candidates without publication.\n\nAlternatively copy the DLL from an individual module ZIP into Plugins. Retain the previous DLL and all data/history. Start with TESTING-CHECKLIST.md. No automatic installation or public release was performed.\n')
 put('TESTING-CHECKLIST.md','''# Prioritized smoke test

1. Extract the complete kit and keep EXE/config/ZIPs/proofs together. Close MMC/game when you choose to install; back up the previous DLLs and retain AppData
2. In the normal Owner-school Dynasty, check transfer FR/SO/JR/SR and redshirts. Stage/interest text should stay separate through selection, scrolling and reopening
3. Check old and new partially/full-scouted Notes. Technical labels should be gone; old stats/assessments unchanged. Test Blind, measurables and interest choices
4. Check a new NSD publication against actual ownership, especially the three NMSU cases. Old posts must retain their words; not every signee gets a post
5. Check independent fog scales on roster/full card/depth chart/hub/Extensions reports, GPI completed weeks, edited NIL and Back navigation. Play Now/main menu remain native
6. Use a fresh spring and championship fixture to check actual-school colors, empty seats/bands, visitor mixing, halftime behavior and reload continuity. Old plans intentionally retain history
7. Listen for spring disappointment after qualifying starting-offense failures. Report actual audibility. Check visiting-TD quieting and ordinary 12+ yard non-TD replays with your broadcast mod
8. Save/reload a per-Dynasty clock choice. Test Drive/Scrimmage/Default Practice from Dynasty and main menu, including the supported Extensions bridge
9. In disposable copies, try one-module update, selected/full removal and restore. Other modules/data must stay intact. Permission errors must not falsely say MMC is open
10. Resize/move setup on your actual monitors. Non-owner multi-coach selection/private Notes remain implementation work, not a completed test
''')
 put('COT-RELEASE_NOTES.md','# Dynasty COT installer 1.1.4\n\nChanges since public 1.1.0\n\n- Manage the same four independent standalone modules\n- Choose installer-compatible public versions and read clean release notes\n- Keep module choices and buttons usable across tested scaling/window sizes\n- Upgrade recognized older installations with backups and rollback\n- Keep data and collection receipts in writable AppData\n- Distinguish permissions, locks, collisions and actual running processes\n\nKeep EXE/config together. Physical mixed-monitor movement still needs acceptance. Module candidates have separate release notes and limits.\n')
 put('COT-DISCORD.txt','**Dynasty COT installer 1.1.4**\n\n- Pick any combination of the four standalone modules\n- Check updates yourself, read the notes and select compatible versions\n- Cleaner layout keeps choices and actions usable at high scaling\n- Recognizes older installs, with backups and restore\n- Clearer permission/file-lock/running-process errors\n- Settings and history stay in shared AppData\n\nExtract the complete setup ZIP and keep its files together. Private testing only; public assets have not been replaced.\n')
 put('SHA256SUMS.txt',''.join(sha(p)+'  '+p.name+'\n' for p in sorted(O.iterdir()) if p.is_file() and p.name!='SHA256SUMS.txt'))
 kit=R/'Release/DynastyCOT-1.1.4-Private-TestKit.zip'
 with zipfile.ZipFile(kit,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
  for p in sorted(O.iterdir()):
   if p.is_file():z.write(p,p.name)
 with zipfile.ZipFile(kit) as z:assert z.testzip() is None
 kit.with_name(kit.stem+'-SHA256.txt').write_text(sha(kit)+'  '+kit.name+'\n')
 print('READY',kit,'SHA256',sha(kit));print('Installer timings:',timing)
if __name__=='__main__':main()
