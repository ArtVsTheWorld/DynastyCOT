# ZIP and manual installation

Use the setup EXE for the simplest upgrade. It checks the package, migrates supported data and records backups. Close MMC and the game first. Installing into Program Files normally requires right-clicking setup and choosing **Run as administrator**. The target must be the versioned folder containing `MMCModManager.exe`. COT also writes settings, logs and history while playing, so a complete MMC installation in a user-writable folder outside Program Files is preferable. Elevating setup alone does not grant normal game/MMC processes permission to write there.

## ZIP with the install script

1. Download the matching `DynastyCOT-VERSION.zip` from Releases and extract the entire ZIP into a separate folder.
2. Open PowerShell. For a Program Files target, open it as administrator.
3. Run the following, replacing both example paths:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "C:\Extracted-COT\Install.ps1" -MmcRoot "C:\Your\MMC_ModManager_vX.X.X.X"
```

This retains the normal checks, transaction backups and settings/history migration. It still requires write access. A script run stops on unrecognized standalone DLLs; use the new setup EXE to review and approve backing up those exact files. The execution-policy option applies to this PowerShell process, not a machine-wide policy change.

## Advanced file-copy installation

This is a basic installation method. It does not perform the script's automatic history migration, transaction logging or rollback. Prefer setup/script for an existing installation with data to migrate.

1. Close MMC and the game. Back up your existing COT/module folders and saves.
2. Move old standalone plugin DLLs belonging to these modules to a backup folder **outside Plugins**. Examples include `DynamicGamedayAtmosphere.dll`, `OfflineDynastySettings.dll`, `SituationalPractice.dll`, `DynastyEnhancer.dll` and `PlayerAnalytics.dll`. Also check for their old extracted native DLLs in module subfolders: MMC scans Plugins recursively. Keep settings, history and unrelated mods.
3. From the fully extracted COT ZIP, merge **Plugins**, **COT-Runtime** and **LICENSES** into the actual Mod Manager folder. Their contents must land beside `MMCModManager.exe`, with `Plugins/DynastyCOT.dll` in place. Keep the directory structure; native BTF DLLs belong in COT-Runtime, not Plugins.
4. For an existing COT installation, retain `Plugins/DynastyCOT/settings.json`. For a fresh installation you can copy `Templates/settings.json` from the ZIP to that location. Its four module switches default to On; existing saved choices remain if the existing file is kept.
5. Restart MMC, enable Dynasty COT and launch normally. Review `Plugins/DynastyCOT/launcher-status.txt` and module logs. Keep the original backups; manually reversing copied files does not reverse a Dynasty save.

Windows may ask for administrator approval when copying into Program Files. File copying does not bypass permissions. If access is denied, check the exact destination, folder permissions and Windows Security protection history; send the full installer Details text when asking for help.
