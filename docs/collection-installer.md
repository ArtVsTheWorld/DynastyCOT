# Dynasty COT collection installer

## Private candidate handoff

Keep setup, its matching `.exe.config`, module ZIPs and verification JSON files together. Select your normal `MMCModManager.exe`, check the modules you want, and choose **Install / Update**. This private kit supplies Beyond the Field 1.0.24, Gameday Atmosphere 2.1.38, Extra Settings 0.2.12 and Expanded Practice 0.8.24. The public version picker lists separately published builds. Close MMC and the game before changing DLLs. For a standalone installation, use the instructions in the chosen ZIP. For an installer-managed rollback, open **Backups** and restore the matching operation.

See the [private installer instructions](player-guides/Private-Installer-Readme.txt). Each module has a printable guide and editable Markdown under [player-guides](player-guides/). In the distributed private kit, open `Player-Guides`. The PDFs cover settings, examples, mode limits and optional Windows Security troubleshooting.

The [1.1.0 installer](https://github.com/ArtVsTheWorld/DynastyCOT/releases/tag/v1.1.0) is the optional collection manager. It manages the same four standalone DLLs; it does not install a new DynastyCOT.dll or change any module's native hooks/helpers. The individual ZIPs remain usable without this installer.

## What the user sees

- Four compact module cards, their installed/selected versions and plain descriptions.
- A detected MMC location and modern executable picker for MMCModManager.exe.
- One **Install / Update** button. Checked modules are considered; unchecked modules stay as they are. Already-current files are kept without redownloading them.
- **Check for updates** lists versions and notes from each module's own repository. Choose a version for one or more modules, then choose Done and Install / Update. No startup checking, automatic module download, forced update, token or account login.
- **Remove** offers selected or full module removal. **Backups** restores a previous operation. Essential progress/errors remain visible; diagnostic logs are kept behind the scenes. Settings/history are kept.

The installer is a small online download manager, not another all-in-one payload. Fetching a new version requires Internet access; removal and restoring an existing collection backup do not. ZIP downloads require the right repository/version, GitHub's SHA-256 asset digest, matching release integrity metadata, exact size and the approved DLL/README/feature-list layout, with an optional license notice, the matching DLL hash and its managed assembly identity. The initial four versions are pinned in the shipped catalog. Catalog resources are verified before use. HTTPS redirects are limited to GitHub release hosts; downloaded programs are never executed by setup.

## Switching from standalone downloads

An existing current DLL is adopted without rewriting it. Older copies, renamed copies with a recognized module identity and recognized legacy/native DLLs are detected in Plugins, including subfolders. The selected module ends with one canonical DLL directly under Plugins. Only the selected module is updated; the other three do not need to be redownloaded.

Recognized copies are backed up before replacement. Unrecognized copies require approval for their exact path/hash. If a file changes after review, setup stops. It does not guess at unrelated plugins or rewrite module settings, account records, published wording, saves, generated histories or crowd plans.

## Switching from older COT

Download the new collection installer once from the COT release page. The old 1.0.x updater only understands bundle updates and cannot perform this conversion itself.

The old host supplies several modules through one DLL. Setup reads its existing module choices and preselects enabled modules. All enabled modules must be selected for conversion; otherwise setup stops and explains which ones are missing. Use Remove first if you intentionally want to retire one.

Conversion backs up the old host and its known matching program files and required license notices, then installs the exact selected standalone DLLs and removes the old host from the plugin scan. External program-file snapshots matter: the standalone DLLs prepare matching support files at their next startup. A collection rollback can restore the older host's matching files as well as the host DLL. Program hashes from verified future module releases are retained in backup journals, including rolled-back transactions, so a later module update does not require a new installer just to restore an old migration. Real user-data paths are excluded from that restore.

Missing/malformed old receipts, unrecognized old runtime bytes and linked paths stop migration for review; they are not silently overwritten. Repair an incomplete older installation before converting it. A known older Gameday cache that has not yet been extracted is allowed: the original absence is recorded, and the verified old host can recreate it after rollback. Old COT backups are retained in Plugins/DynastyCOT/Backups. To use an older bundle's own restore flow, first restore the collection migration backup so the standalones and old host are not mixed.

## Backups, restore and logs

Current candidates keep collection-managed state under `%LOCALAPPDATA%/Ace/DynastyCOT/Collection/<MMC identity>/`, outside the plugin scan. Earlier `COT-Collection` receipts/backups are copied without deleting originals. The corresponding contents are:

- `installed.json`: module versions/hashes recorded by the manager.
- `Backups/<transaction>/`: journal and verified `.bak` copies.
- `Logs/setup.log`: completed collection operations.
- `Modules/<module>/`: README/features for modules fetched by setup.

Downloaded ZIPs and verified staging DLLs are cached under `%LOCALAPPDATA%/DynastyCOT/Downloads`. They are not active plugins. Existing module logs and data remain in their current paths; see the standalone READMEs.

Close MMC and the game before file changes. A running-process warning is based on actual MMC/game processes, not old plugin files. Installing/replacing DLLs requires access to Plugins, including elevation when Windows requires it. Current candidates save module data and support in shared AppData, so normal saving does not require write access to Plugins. Setup does not change Windows permissions. A real running-process error names MMC or the game and gives its process ID. Access-denied errors explain permissions; sharing violations explain locked files. Old standalone files are migration inputs, not proof that MMC is open. Do not delete data folders to clear an error.

Every changed file has before/after hashes and a verified backup before mutation. Replacements/journal writes are atomic. Failed operations restore previous managed files. An interrupted install blocks new changes until its backup is restored. Restore validates every path, backup hash and current file before writing; later edits or out-of-order backups stop the operation. Restore newer collection backups first. An explicitly selected downgrade requires confirmation.

## Testing checklist

1. Fresh writable MMC fixture: select one module, then several. Confirm only those canonical DLLs appear directly in Plugins, with no DynastyCOT.dll.
2. Existing standalone install: all four installed versions should be detected. Adopting current versions should not redownload them. Use drag-and-drop again afterward; no extra host should become necessary.
3. Older COT: check the enabled module preselection and migration explanation. Convert, launch and verify modules/settings/history. Restore that migration and confirm the older host has its matching runtime and no duplicate standalone DLLs.
4. Check updates, inspect per-module notes, keep an older version and update just Gameday Atmosphere. Confirm Beyond the Field is not downloaded or rewritten.
5. Remove one module and restore it. Remove all and restore. Confirm settings/accounts/history remain, then verify native activation in MMC/game.
6. Check scaling, the file picker, permission errors and the process warning on another user's setup. Offline tests and successful detection do not establish live-game compatibility.

Verification metadata is supplied beside the installer. The released 1.0.0, 1.0.1 and 1.0.2 installers passed nine isolated migration cases (2,589 checks), including real standalone support-file preparation, old-host integrity after rollback, repeated migration and unchanged user-data fixtures. Another 230 collection checks, actual-window scaling checks and anonymous public module downloads/install/version-change/rollback passed. Ace reported the collection installer installs successfully. Native game activation after migration/rollback on additional installations, other users' permissions/scaling and overhaul combinations remain live-review items.

## Build and maintain

1. Build/test/publish each standalone module from this workspace with the existing module tooling. Each standalone repository continues to pin this source revision.
2. Update the four pinned versions in `Tools/build_collection_catalog.py`, using the verified ZIPs/evidence in Release. Run that tool to produce the catalog of exact artifacts and program-only migration hashes. No SDKs, assets, saves or generated history belong in the public source/catalog.
3. Run `python Tools/build_collection_installer.py` to compile the lightweight EXE. It embeds the catalog and app icon, not the module DLLs.
4. Run `python Tests/test_released_cot_migration.py` against the locally retained verified 1.0.0/1.0.1/1.0.2 packages and SDK references. It launches no game, manager, generator or helper. Run `python Tests/test_collection_installer.py` for isolated filesystem, failure/rollback, selection and actual-window layout checks. Run `python Tests/test_collection_downloads.py --network` explicitly for anonymous public discovery/download and fixture-only install/version-change/rollback.
5. Run `python Tools/finalize_collection_installer.py` after all reports pass for the exact EXE/source hashes. Review the source/artifact hashes and live acceptance before publishing the installer. The legacy bundle build remains available to reproduce older 1.0.x releases; it is not the new collection install path.

## Candidate acceptance limits

Only installer-compatible public releases appear in the version picker; a release needs the expected ZIP layout, module identity, GitHub digest and matching integrity metadata. Checking does not install anything. Release notes render safe Markdown without a browser.

Single-owner Dynasty operation is the verified BTF scope. Multi-coach active non-owner selection and private Notes ownership remain implementation limitations, not completed tests. The release-preparation report distinguishes fixture checks from installed-game acceptance.

## Current download contents

A current standalone ZIP contains its DLL, README.txt, FEATURES.md and module-specific Player-Guide.pdf. Required notices are in the README appendix where needed. The updated installer retains earlier accepted archive layouts, verifies guide-bearing releases and copies their PDF outside Plugins with the other module documents. Rollback restores the prior guide or its prior absence. Verification JSON and checksum assets remain outside module ZIPs.
