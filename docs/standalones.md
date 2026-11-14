# Standalone installation, updates and compatibility

Want one feature? Download its standalone ZIP. Want several? Use the [collection installer 1.1.5](https://github.com/ArtVsTheWorld/DynastyCOT/releases/tag/v1.1.5).

Each current module ZIP has exactly four files: its existing DLL, README.txt, FEATURES.md and a module-specific Player-Guide.pdf. Copy only the DLL into the Plugins folder beside your MMCModManager.exe. Read the included guide for settings, examples, limitations and help. Third-party notices are retained in the README appendix where needed. DLL filenames and compatibility IDs do not change when MMC displays the Ace’s prefix.

The installer manages these same DLLs. It downloads only selected changes, verifies them, backs up recognized older copies and supports selected/full removal and restore. Updates and older-version choices require user action. The current archive reader also accepts previously supported three-file ZIPs, license-bearing ZIPs and verified older external-support layouts. Unknown DLLs require review instead of being silently deleted.

## Existing installations

Close MMC/game before changing files. Use setup for an older DynastyCOT.dll host: preserve each enabled old module during the conversion or explicitly remove it first. Recognized old DLL names, loaders and matching program files are retired transactionally outside the recursive Plugins scan. If migration fails, inspect its rollback result and named path; do not delete settings/history.

Settings, account identities, published posts, Notes and crowd history live under `%LOCALAPPDATA%\Ace\DynastyCOT` and remain shared across MMC copies. Press Win+R and paste that path. Old data locations are read-only migration inputs; original copies are retained. Runtime code is prepared and verified outside Plugins. Some modules use supporting background processes, but users still install one plugin DLL.

Manual rollback restores the earlier DLL with MMC/game closed. Installer rollback restores the matching transaction using Backups, newer transactions first. A failed or interrupted restore blocks new operations until recovery completes. These operations do not restore or rewrite Dynasty saves.

## Compatibility and limits

- Keep one provider per feature. Combining an old COT host with the same standalone module can create a duplicate; the module and installer detect recognized providers.
- BTF fog/ratings/money display changes stay in supported Dynasty scope. Play Now/main-menu displays remain native. Independent interest/workout choices work with Recruiting Fog Off; Blind concealment still applies. Full private recruiting context for non-Owner multi-coach views remains unsupported beyond the documented perspective.
- GDA preserves protected groups, native empty seats and saved plans. New layout-5 visiting sections require a new eligible plan or explicitly requested regeneration and fresh generation. Spring single-school and neutral-site layouts stay separate. Exact crowd appearance, actual frame time and spring boos require live acceptance beyond offline checks. The controlled LSU-Ole Miss away-TD background reduction was heard and confirmed.
- Expanded Practice and Extra Settings keep native mode checks and verified hook ownership. Supported Dynasty Extensions wrappers are narrowly identified; unknown versions are not automatically chained. Other third-party runtime compatibility is not established merely because DLLs load.
- Native support/SDK versions and third-party game-asset overrides can change which adapters are supported. Logs distinguish disabled/waiting states from unknown layouts or ownership failures. Do not remove integrity checks to silence a warning.

Refer to each module's current release notes, full FEATURES.md and PDF guide. No local test run establishes universal live compatibility.
