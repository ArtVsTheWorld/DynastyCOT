# Beyond the Field 1.0.3 - Position Changes fix

- Projected overall ratings on the Position Changes preview now follow your player grade style and Roster Fog setting, instead of revealing exact numbers.
- Position calculations, sorting, player counts and the normal selection highlight stay native. This changes the displayed text, not a player's ratings or position.
- The targeted fix retains the working NIL formatting, player-card Ratings fixes, capitalized descriptions and existing Back navigation.
- Your settings, saves, accounts, published posts and history stay in place. The Social Media generator crash fix from 1.0.2 is still included; content and weekly budgets are unchanged.

Close the game and MMC, then replace only `BeyondTheField.dll` in the versioned Mod Manager's Plugins folder. Keep the existing data folders. Matching support files are embedded and prepared automatically. Use one provider of Beyond the Field: disable the same module in COT if you use this standalone DLL.

Validation: the display correction was reproduced and confirmed in the live game on two players using a targeted adapter. The source-built native engine passed the existing menu/NIL/fog regressions, 10,123 focused projected-rating checks, 7,317 analytics-model checks, 2,491 analytics integration checks and native dispatch tests. The actual DLL passed 750 isolated standalone integration checks, including existing-data preservation and coexistence with the other three modules. Offline conversion work was reduced, but a noticeable live Back/reopen speedup was inconclusive. The packaged update still needs its own clean-launch performance check; offline tests do not certify every game or overhaul combination.

Compared with v1.0.2, this release changes the projected-position display adapter and embeds a newly source-built, matched native pair. The managed loader and user-data paths retain the established design. Roll back by closing MMC/game and restoring your saved 1.0.2 `BeyondTheField.dll`; keep all settings/history folders.

Gameday Atmosphere's experimental touchdown replay changes are not included in this release.
