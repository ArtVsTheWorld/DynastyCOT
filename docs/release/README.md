# Dynasty COT release and updates

[Dynasty COT 1.0](https://github.com/ArtVsTheWorld/DynastyCOT/releases/tag/v1.0.0) is the first stable release. The older 0.1.6 public test prerelease remains available. See the [1.0 notes](1.0.0.md) and [verification report](verification-1.0.0.md).

**1.0.1** is an installer-only hotfix for old standalone conflicts and clearer write-access errors. It does not change the game modules. See the [hotfix notes](1.0.1.md) and [ZIP/manual guide](../manual-install.md).

**1.0.2** fixes the Social Media generator null-player crash while retaining the installer fixes. See the [generator hotfix notes](1.0.2.md).

## Choose your setup

Run `DynastyCOT-Setup-VERSION.exe` and choose the **MMC Mod Manager** folder containing `MMCModManager.exe`, usually `MMC_ModManager_vX.X.X.X`. It is different from the parent MMC folder. Close MMC and the game before changing an installation.

All four modules are optional:

| Module | What it adds |
| --- | --- |
| Beyond the Field | A livelier Dynasty feed, scouting fog, grades and clearer money displays. |
| Gameday Atmosphere | Crowds that arrive, react and travel with the matchup. |
| Extra Settings | More offline Dynasty controls, including clocks saved for each Dynasty. |
| Expanded Practice | Full drives and scrimmages from the normal practice menu. |

On **Install**, checked modules run. Unchecked modules are retained but disabled. To remove files, select **Remove selected features** and check what to remove, or choose **Remove all COT**. User settings, published history and transaction backups are retained. Native binaries remain outside MMC's recursive plugin scan; keep that layout intact.

## Manual updates

Setup makes no network request at startup. Open **Updates** and explicitly choose **Check for updates**. Pick a published version, including an older release or a clearly marked test release. Read its version, release notes and known compatibility information. **Download selected release** only downloads and verifies the ZIP; it does not install anything. The Install button names the selected package's version. Choose modules and explicitly press **Install VERSION** to apply it. **Use bundled VERSION** returns to the package embedded in that installer.

Downloads use anonymous HTTPS GitHub endpoints. No GitHub account, credential or embedded token is used. A private repository would prevent anonymous discovery/download; this project is public. GitHub API rate limits or network failures leave the installation unchanged. You can stay on your installed version or download a versioned installer from Releases instead.

The release's `cot-update.json` pins the ZIP's exact byte length and SHA-256, notes and compatibility text hashes. Setup also checks the selected version's package manifest and every payload hash before the existing transactional installer runs. These checks detect corruption and mismatched payloads; they are not an independent publisher signature. Trust the project/release origin. Release assets also include `SHA256SUMS-VERSION.txt` for manual verification.

## Restore a previous version

Open **Undo changes**, select an installation backup and restore it. Each install/update/removal records a transaction. Restore reverses that transaction's managed files. Later user edits to settings and published histories are kept. If a managed binary was changed outside COT since the transaction, restoration stops with a conflict; read **Activity** rather than replacing files manually. Keep an older versioned installer and a copy of your saves separately. Rollback never rolls back a Dynasty save.

To deliberately install an older available release, use the same explicit select/download/install flow. Keep its compatibility notes in mind. A module may reject newer data it does not understand; file rollback is not proof of backwards game compatibility.

## Build and verify from source

The repository contains COT source and required license notices. It excludes MMC SDK assemblies, game assets/captures, SDK dumps, saves, generated publication history, credentials, private research fixtures and compiled outputs. Those inputs must never be uploaded to a public runner.

On a prepared Windows x64 machine with the local MMC SDK, .NET Framework compiler, PowerShell 7, Python, Node and the module build-tool prerequisites, run:

```powershell
./Build.ps1 -MmcRoot 'C:\path\to\MMC_ModManager_vX.X.X.X' -Package
```

Build scripts check source baselines, scopes/ownership, helper fingerprints, package hashes and isolated installer transactions. Some existing native compatibility tests require private game-derived fixtures. A fresh public checkout does not contain those inputs and cannot claim those tests passed without them. Obtain the required local inputs through your own game/SDK installation; do not publish them.

The build stages source output, validates it, finalizes `DynastyCOT-VERSION.zip`, generates `cot-update.json`, embeds the exact verified ZIP in the versioned installer, writes checksums, then tests the actual setup window and updater in marked Build fixtures. Update metadata is generated **after** ZIP finalization to avoid a checksum cycle. None of these steps installs into live MMC.

Additional focused checks:

```powershell
python Tests/test_setup.py
python Tests/test_updater.py
# After the lead publishes the exact candidate: anonymous network reads,
# installation/rollback writes restricted to a new marked Build fixture.
python Tests/test_updater.py --published 0.1.6-private
```

`Build/setup-verification.json` records real WinForms button/engine checks and 100/125/150% layout captures. `Build/updater-verification.json` records local HTTP discovery/download/install/rollback and failure cases. The separate published check records `Build/updater-published-verification.json`. The window clamps to the current monitor working area with 16-pixel margins and a reduced effective minimum; the four module cards fit together without scrolling in 0.1.7. The 150% layout is also checked against simulated 1080/1152-pixel working areas, including footer visibility, update actions and visibility of every module without scrolling. Screenshots at simulated layout scales verify this window on the current machine; another monitor's DPI and accessibility settings remain useful user acceptance tests. Automated generation/menu fixtures are distinct from live menu FPS and gameplay acceptance.

## Publish one candidate

The lead reviews source/private-input exclusions, builds and verifies the payload, completes targeted clean-start acceptance, and commits/pushes the matching source and versioned release notes. A documentation-only release update may retain the exact tested payload bytes; record the implementation revision and later documentation revision separately. After the complete build, run `python Tools/create_release_evidence.py`. It refuses stale or baseline-only checks and exports allowlisted public counts/hashes, compatibility notes and the source revision without local paths or private fixtures. Release assets are the ZIP, setup EXE, `cot-update.json`, checksums, versioned release notes and `release-evidence-VERSION.json`. Keep older assets available. Publish tag `vVERSION` only after the clean-start acceptance for that exact package. Development candidates use prereleases; 1.0.0 is the first stable-version target. The checked-in workflow is manual-only and requires a prepared, access-controlled Windows self-hosted runner; it never publishes on a source push.

After upload, run the explicit published-fixture check above. No public website API call, startup success, isolated installer test or offline fixture establishes live compatibility. Document the remaining checkpoint acceptance separately. Do not replace a published version's payload under the same version tag; fix failures in a newly versioned candidate.

GitHub API behavior follows the [official Releases REST documentation](https://docs.github.com/en/rest/releases/releases). The updater lists published releases rather than relying on the stable-only latest-release endpoint, allowing explicit prerelease and older-version choices.

## Partial updater checks

`python Tests/test_updater.py --candidate-only` exercises the current payload's complete discovery/download/install/transaction-restore path without opening an older release. It writes a separate candidate-only report and does not prove cross-version rollback. The full evidence exporter rejects candidate-only or same-version reports as complete release verification. Do not use this mode to disguise a blocked earlier release; retain and report that blocker separately. For 0.1.7, Windows Defender quarantined a DLL while extracting the older 0.1.6 payload in the cross-version fixture. No protection changes or bypass were attempted.
