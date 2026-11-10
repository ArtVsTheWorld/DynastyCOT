# Standalone downloads and compatibility

Want one feature? Download its standalone ZIP. Want the collection? Download Dynasty COT. All are built from this integration workspace; the standalone repositories are download/help front doors with exact source pins, not separate development forks.

| Module | Standalone repository | Current standalone |
|---|---|---|
| Beyond the Field | [BeyondTheField](https://github.com/ArtVsTheWorld/BeyondTheField) | 1.0.7 stable |
| Gameday Atmosphere | [DynamicGamedayAtmosphere](https://github.com/ArtVsTheWorld/DynamicGamedayAtmosphere) | 2.1.11 / Update 4 |
| Extra Settings | [offline-dynasty-settings](https://github.com/ArtVsTheWorld/offline-dynasty-settings) | 0.2.10 stable |
| Expanded Practice | [drive-based-practice](https://github.com/ArtVsTheWorld/drive-based-practice) | 0.8.21 stable |

Each ZIP contains exactly its branded plugin DLL, README.txt and FEATURES.md. Required program files and notices are embedded and verified when prepared. Native DLLs stay outside MMC's recursive Plugins scan; Extra Settings/Practice still use their verified helper processes. One-DLL downloads do not mean no generated support files.

## Install, update and undo

1. Close MMC and the game. Use a writable versioned Mod Manager folder, preferably outside Program Files.
2. Put the selected DLLs directly in `Plugins`. Move old duplicate plugin DLLs outside Plugins. Keep all data folders.
3. Restart MMC and launch. The first startup prepares support files; later starts retain matching cached files.
4. Read package preparation messages in `Standalone-Runtime/<internal module ID>/Logs/package.log`, then check the module's native/helper log for actual game activation. Gameday retains its existing launcher log under `Plugins/DynamicGamedayAtmosphere/Logs`.

To update, replace the DLL while MMC/game are closed. To undo a standalone update, restore the previous matching DLL and restart; generated program files can be restored/reprepared from that DLL, while user data remains. Back up user data before downgrading across older schema versions. Do not restore an old save over a newer one merely to undo plugin installation.

Existing internal data paths intentionally remain unchanged:

- Beyond the Field: `COT-Runtime/DynastyEnhancer` and `Plugins/DynastyEnhancer/Logs`. `RuntimeHistory`, `RecruitResumeHistory` and `DynastyEnhancementSettings` inside the runtime are user data. The new config filename is `beyond-the-field.json`; new DLL filenames use BeyondTheField.
- Gameday: `Plugins/DynamicGamedayAtmosphere`; native cache under `DGA-Runtime`.
- Extra Settings: `Plugins/OfflineDynastySettings`, including `DynastyClocks.json` and the playcall profiles.
- Expanded Practice: `Plugins/SituationalPractice`, including its reports/session data.

These old IDs/folder names are compatibility aliases, not extra copies of the plugin. Branding does not justify losing settings, accounts or published history.

## Potential compatibility issues

| Combination | What to watch for |
|---|---|
| All four matching current standalones | Fixture loading, distinct startup guards, payload paths and retained COT hook contracts pass. Ace reported all four standalone releases verified working on November 9, 2026. This does not certify every other setup. |
| Beyond the Field + Expanded Practice | BTF fingerprints code that Practice patches. It recognizes only the exact supported practice hook bodies/relocations. An older/modified Practice helper can make BTF reject activation. Keep this matched release set together. |
| Beyond the Field + Extra Settings | Both extend Dynasty settings through different native layers. Current menu/writer contracts are retained. Another mod rewriting those callbacks or a mismatched old version can block one module. |
| Extra Settings + Expanded Practice | They share the supported controller bridge. Exact known ownership is recognized; unknown bridge changes are rejected. This is not general compatibility with every controller/gameplay plugin. |
| Gameday + the other standalones | No new package-path/guard collision found. Its named pause output remains separate. Crowd effects intentionally do not activate in Practice. Individual live game/overhaul layouts still need checking. |
| COT + a standalone copy of the same feature | Use one provider. Disable that feature in COT before starting the standalone. The three new wrappers reject active COT ownership before extraction/startup; Gameday shares its existing per-MMC guard with COT. |
| Legacy standalone DLLs | Move old `DynastyEnhancer.dll`, `PlayerAnalytics.dll`, `DynamicGamedayAtmosphere.dll`, `OfflineDynastySettings.dll`, `DynastyPlayClock.dll`, `OfflinePlaycall.dll` or `SituationalPractice.dll` out of Plugins as appropriate. Keep settings/history. Renaming an old DLL is not a safe substitute. |
| Different release generations | Shared runtime caches, native ABI/build IDs and helper hashes must match. Switching providers/versions requires closing MMC/game. Reinstall COT before switching the same feature back to its bundled provider. |
| COT installer/removal | Released COT 1.0.2 does not auto-remove these new branded standalone DLLs. Fixture tests show installing/removing COT preserves them and user data; standalones reprepare a removed cache on next startup. A standalone remains separately installed until its DLL is removed. |
| Program Files, security tools, locked files | A one-DLL download still needs writable cache/log/data folders. A denied write or quarantined helper remains an activation issue. Unknown native owners are never bypassed to make startup appear successful. |

Targeted AIO 1.4.1/TeamBuilder support carries forward; compatibility with every overhaul/version is not established. The generation hotfix remains included. No live game was modified while building these distributions.

## Verification and release comparisons

- 660 all-four package/integration assertions: production assembly extraction, resource hashes, independent/duplicate guards, legacy/COT conflicts, traversal rejection, helper leases, data preservation, cache repair and real COT install/uninstall/re-extraction fixtures.
- 90 Gameday checks, including four existing managed regression suites. Ace reported the earlier equivalent standalone packaging working; 2.1.9 changes its distribution labels/docs, not the native engine.
- 465 BTF generator/content tests and the source-built native Social Media fixture. Its native packaging change is the config filename; the existing data directory and analytics build contract stay shared.
- 5,009 exact Practice ownership/relocation/mutation checks; default Practice regression suites passed without game access.
- 86 per-Dynasty clock-profile checks and 54 preference/launch checks.

Gameday notes compare source against v2.0.35; Extra Settings against v0.2.1; Expanded Practice against v0.8.18. Beyond the Field had no previous published release, so its notes distinguish the first standalone release from its old private prototype. Per-module RELEASE_NOTES.md files identify the compared commits and source areas; release integrity files pin the actual compiled payloads and source revision.

## Prioritized live checks

1. Install the selected standalone set with the game/MMC closed. Confirm each module's actual native/helper activation log; do not infer compatibility from MMC listing the DLLs.
2. In an existing Dynasty, confirm the same published feed/history, fog choices, clocks and practice data. BTF Social Media, Analytics and edited NIL values should still work.
3. Test BTF with Extra Settings: open creation and existing-Dynasty settings, edit a clock, reload, and verify another Dynasty stays independent.
4. Test Expanded Practice with Extra Settings/BTF present: main-menu and Dynasty Practice, all modes, side switching and returning to native practice.
5. Confirm Dynasty and basic Play Now crowd/noise/shake behavior, plus native touchdown presentation. Crowd effects in Practice should remain absent.

Ace confirmed all four standalone releases verified working on November 9, 2026. The three preview releases were promoted to stable without changing their tested ZIP/DLL bytes. Enclosed build-time preview labels remain historical; current release notes and repository status record this acceptance.
