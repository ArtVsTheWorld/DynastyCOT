# Dynasty COT 1.0 verification

The complete source build and isolated package/installer/updater checks passed. No live MMC installation or binary release publication was performed.

| Check | Result |
|---|---|
| Imported standalone source preservation | 983 original source hashes unchanged |
| Native source build | All four modules rebuilt; no installed payloads borrowed |
| MMC registration and helpers | 20 assembly checks; one entry, **Dynasty COT**, author **Ace** |
| Module selections | All 16 combinations; 106 configuration/planner/path checks |
| Analytics packaged ABI/build pairing | 11 checks, including new filename and PE export-module branding |
| Installer | 116 checks: clean/update, migration, selected/full uninstall, preservation, tamper and path rejection, automatic/manual rollback |
| 1.0 runtime migration | 135 checks using the actual source-built stage, retained 0.1.19 package and verified released Gameday 2.0.35 payloads |
| Migration failures | All three injected transaction failure stages restore the prior state; unknown active legacy DLLs stop before writes |
| History and logs | Settings, accounts/publications, recruit records and crowd history fixtures retained; legacy launcher log collision/rollback checks pass |
| Windows setup | 36 actual GUI/engine checks; folder detection, selected/full uninstall, quoted paths and unsafe-fixture rejection |
| Layout | Five rendered cases: 100/125/150%, plus bounded 1080/1152 working areas; descriptions fit without scrolling, footer text/buttons remain visible |
| Manual visual review | Final installer Install captures reviewed at 100% and 150%/1080; no clipped text/buttons or in-app logo |
| Optional updater | 95 checks using 0.1.19 → 1.0.0 discovery/download/install/rollback fixtures, including failure cases |
| Release evidence | 18 provenance/privacy checks; synthetic or stale migration reports cannot stand in for the actual candidate |
| Public source | Explicitly supplied README screenshots allowlisted; SDKs, saves, histories, private research, credentials and generated binaries excluded |

The full native suites also passed, including rating publication/focus/badge fitting, NIL editing/redraw, fog/color isolation, Analytics model/integration, crowd/travel/protected groups, sound/replay policies, per-Dynasty clocks and practice ownership. The build contains 159 manifested files plus its manifest.

Versioned assets: `DynastyCOT-1.0.0.zip`, `DynastyCOT-Setup-1.0.0.exe`, manual-update metadata, release notes, sanitized release evidence and SHA-256 checksums. The ZIP SHA-256 is `416a9b28cdf2e214c19a56005a6007caf705eb71b5d98d4b2295f7014f6a3812`.

## Remaining release acceptance

The 1.0 filename/direct-loader changes have not been loaded into a clean live game in this task. Passing native simulations, source tests or MMC startup is not live compatibility proof. Existing session-level UI successes apply to the preceding live adapters, not a cold install of these exact assets.

1. Close MMC/game, install this package, restart both, and confirm one **Dynasty COT** entry by **Ace** and native activation for the selected modules.
2. Check retained settings and published feed history, edit Current NIL repeatedly, then check uppercase fitted grades, bucket selection, no numeric flash and fast Back navigation.
3. Check team Analytics, completed-week availability, player cards/fog and a new weekly Social Media edition.
4. Run one Gameday fixture for crowd appearance, cheer/shake pairing, departures and replay transitions. Check Extra Settings and both custom practice modes.
5. Confirm module logs stay in subfolders. Use setup's **Undo changes** if rollback is needed.

No new live FPS/menu measurements or universal overhaul compatibility are claimed. Anonymous discovery/download of this particular release remains untested until its assets are published; the isolated updater path passes. Public binary publication remains held for the clean-start acceptance above.

The [Update 4 Discord draft](gameday-update4-discord.md) is 1,692 characters. Its [source comparison](gameday-update4-code-review.md) uses the actual 2.0.35 tag and identifies new/current implementation files.
