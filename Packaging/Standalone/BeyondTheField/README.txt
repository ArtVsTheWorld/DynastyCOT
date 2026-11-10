BEYOND THE FIELD by Ace - 1.0.7
Dynasty news, scouting uncertainty and player performance grades. COT is optional.

INSTALL
Close the game and MMC. Put BeyondTheField.dll directly in the versioned MMC
Mod Manager's Plugins folder, reopen MMC and launch normally. The DLL prepares
its matching support files; no Node installation or other module is required.
Use a writable MMC folder outside Program Files. The first startup takes longer.
Find options in Dynasty settings > Beyond the Field. Social Media is under
the Team tab; Player Analytics is a team-only player performance view.

UPDATING / OTHER PLUGINS
Move old DynastyEnhancer.dll and PlayerAnalytics.dll copies outside Plugins.
Keep their data folders. If keeping COT, disable its Beyond the Field module
before using this DLL. Other COT modules may stay enabled. Use matched current
standalone versions; do not mix different copies of the same feature.

SAVED DATA / REMOVAL
Your existing settings, accounts, published posts and history stay in place.
Legacy data-folder/compatibility IDs are intentionally retained so COT and this
plugin find the same data. New shipped filenames use Beyond the Field branding.
Do not delete COT-Runtime/DynastyEnhancer: it may contain your saved feed history.
To uninstall, close MMC/game and remove only BeyondTheField.dll. Keep data.
To switch its feature back to COT, reinstall COT to restore matching runtime
hashes, then enable its module. Reinstalling keeps settings/history.

LOGS
Package setup: Standalone-Runtime/DynastyEnhancer/Logs/package.log under MMC.
Native/generator: COT-Runtime/DynastyEnhancer under MMC.
Launcher: Plugins/DynastyEnhancer/Logs/BeyondTheField-launcher.log.

GPI (Gridiron Performance Index), @GPIndex, is the performance-grading
account. Its G icon and GPI GRADE column replace the previous branding. Grades
still use the same tracked Dynasty statistics. Archived account headers show
the current name without rewriting your saved posts, grades or history.
Position Changes projections still follow your player grade/fog setting.
Recruiting Fog offers Default (normal game display), Recruit Feedback and
Rank Only interest displays. Default is the initial choice; existing saved
choices are kept.
Feedback uses general standings instead of precise school bars/ranks. Blind
hides interest. Off-board, uncommitted prospects show their native scouting
progress icon in the Target column; their names are unchanged.

STARTUP: startup now waits up to 60 seconds for the exact native settings
code and two stable ownership checks. Unsupported hooks are never bypassed.
The 1.0.5 early-start failure was reproduced and recovered in the live session.
The Prospect List small-card standings were visually verified in a temporary
live test. Feedback and Blind now remove the misleading Projected Cutoff row;
Default and Rank Only retain it. Real lockout notices and recruiting rules stay
native. Ace installed 1.0.6, restarted MMC/game and reported it working. The 1.0.7
change sets the initial interest choice to Default and renames its menu label;
existing saved choices are kept. Other setups still need their own live checks.
Full Overview header stage-progress hiding remains a live-review item.
No test helpers or crash dumps are included.
See FEATURES.md. Other game/overhaul combinations still need their own checks.
Third-party notices are embedded and prepared under MMC/LICENSES.
