EXTRA SETTINGS by Ace - 0.2.10 standalone release
More offline Dynasty clocks and playcall controls. COT is optional.

INSTALL
Close the game and MMC. Put ExtraSettings.dll directly in the versioned MMC
Mod Manager's Plugins folder. Reopen MMC and launch normally. The DLL prepares
its verified settings-saving helper; no extra download is needed.
Use a writable MMC folder outside Program Files.
Open Dynasty > League Settings > Game Settings to change the options.

UPDATING / COT
Move the old OfflineDynastySettings.dll, DynastyPlayClock.dll and
OfflinePlaycall.dll outside Plugins before starting MMC. Keep their data.
If using COT for other features, disable its Extra Settings module first.
Do not load two copies of these settings hooks. Use matched current modules.

SAVED DATA / REMOVE
Choices remain in Plugins/OfflineDynastySettings, including DynastyClocks.json.
Each Dynasty keeps its own clocks; new Dynasties start On / 15 seconds.
Close MMC/game and remove only ExtraSettings.dll to uninstall. Keep the data
folder for future use. Do not delete it just to update.
To switch back to COT, reinstall its matching payload, then enable its module.

LOGS
Plugins/OfflineDynastySettings/Diagnostics and its existing module log files.
Package setup: Standalone-Runtime/ExtraDynastySettings/Logs/package.log.

See FEATURES.md. Ace reported the standalone release working on November 9, 2026.
Other game/overhaul combinations still need their own checks.
