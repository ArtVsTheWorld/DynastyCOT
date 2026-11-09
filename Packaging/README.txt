DYNASTY COT by Ace
Package version 1.0.2 - generator crash hotfix

INSTALL / UPDATE
Close MMC and the game. Run DynastyCOT-Setup-1.0.2.exe, confirm the actual
versioned MMC Mod Manager folder and choose your modules. For an installation
inside Program Files, right-click setup and choose Run as administrator.
Setup does not automatically change permissions or security settings.

Recognized older standalone copies are backed up and replaced automatically.
Unrecognized copies are listed for review. Yes backs up and replaces those
exact files; No leaves everything unchanged. Settings/history are retained.
You do not need to delete whole module folders. A file changed after review
must be reviewed again. Undo changes can restore the original files.

ZIP / MANUAL SCRIPT
Extract the entire ZIP into a separate folder. Open PowerShell; use Run as
administrator if the target Mod Manager is inside Program Files. Run:
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File "C:\Extracted-COT\Install.ps1" -MmcRoot "C:\Your\MMC_ModManager_vX.X.X.X"
Replace both example paths. This uses the same package checks, backups and
history/settings migration. The script stops on unknown standalone DLLs;
use the setup EXE for the reviewed backup-and-replace prompt.
See https://github.com/ArtVsTheWorld/DynastyCOT/blob/main/docs/manual-install.md
for advanced file-copy installation and its migration/rollback limitations.

ACCESS DENIED
Details identifies the blocked path. Confirm the selected Mod Manager folder
is writable. Program Files normally requires an elevated installer. Check
folder permissions and Windows Security protection history if needed.
Downloads working while Program Files fails strongly suggests a permissions
issue, but does not identify every possible security or file-lock cause.

FEATURES
All four modules are optional. Checked modules run; unchecked modules remain
installed but disabled. Existing choices are retained. Beyond the Field adds
Dynasty news, fog, grades, colors, money displays and team Player Analytics.
Gameday Atmosphere adds crowds, sound and shake, including basic Play Now.
Extra Settings keeps playclock/playcall choices per Dynasty. Expanded Practice
adds drives and scrimmages in offline Dynasty and main-menu Practice.
Native game-module/helper code is unchanged from the released 1.0 package.
The Social Media generator now resolves the actual player when restoring an
archived personal voice, preventing a null-player crash. Published wording,
account identities, settings and history are kept.

REMOVAL / ROLLBACK
Choose Remove selected features or Remove all COT. Settings, published history,
profiles and backups remain. Undo changes restores an installation backup;
rollback does not roll back your Dynasty save. Keep older installers and saves.

OPTIONAL UPDATES
Use Updates > Check for updates, inspect the notes, choose a version, download
and explicitly install it. Nothing downloads or installs automatically.

REPORTS
COT startup: Plugins/DynastyCOT/launcher-status.txt
Beyond the Field: COT-Runtime/DynastyEnhancer/*.jsonl and runtime-generator.log
Gameday: Plugins/DynamicGamedayAtmosphere/Logs/
Extra Settings: Plugins/OfflineDynastySettings/*.log
Practice: Plugins/SituationalPractice/launcher.log and SituationalPractice.log

This installer hotfix has private fixture validation. Affected-user acceptance
is pending. Native fingerprints, ownership and helper verification remain.
