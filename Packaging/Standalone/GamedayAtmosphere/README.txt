GAMEDAY ATMOSPHERE by Ace - 2.1.9 / Update 4
Based on the crowd engine in Dynasty COT 1.0.2. COT is not required.

INSTALL
1. Close the game and MMC Mod Manager.
2. Put GamedayAtmosphere.dll in the versioned Mod Manager's Plugins folder.
3. Restart MMC and launch the game. Open User Settings > Gameday Atmosphere.

The DLL prepares its own support files. There are no extra downloads.
Use a writable Mod Manager folder outside Program Files so it can save data.

UPDATING / COT
Replace the older Gameday plugin DLL; keep its settings/history folders.
If the old file is DynamicGamedayAtmosphere.dll, move only that DLL outside
Plugins before starting MMC. Old support files may be archived automatically;
reopen MMC if its first upgrade scan still reports an old DLL warning.

Use one Gameday provider. If keeping COT for other features, turn its Gameday
module off. Matching current COT and standalone copies share a startup guard
so they do not start two Gameday runtimes. Keep versions matched.

SETTINGS / HISTORY
Existing INIs are kept without a reset or retune. Current COT and standalone
use the same Plugins/DynamicGamedayAtmosphere data folder and DGA-Runtime.
Logs are in Plugins/DynamicGamedayAtmosphere/Logs. Back up older profiles
before switching back to an older bundle, whose migration rules may differ.

REMOVE
Close MMC/game and remove GamedayAtmosphere.dll. Keep shared data/runtime
folders if using COT or preserving your settings/history for later.

See FEATURES.md for the full feature list and mode limits. The standalone
packaging was reported working by Ace; other setups still need their own checks.
