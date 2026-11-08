OFFLINE DYNASTY SETTINGS 0.2.10
By Ace

PURPOSE
Adds game settings and team switching to offline Dynasty.

INSTALL / UPDATE
1. Close the game and MMC.
2. Unzip the download.
3. Copy BOTH OfflineDynastySettings.dll and the OfflineDynastySettings folder beside it
   into MMC's Plugins folder.
4. Merge folders, replace matching files, then launch normally.

Do not delete the existing folder. Your settings, logs and history stay there.
README.txt is for reading; it does not need to go into Plugins.

SETTINGS
Open Dynasty > League Settings > GAME SETTINGS. Cooldowns and limits default
to Off, Offensive Play Clock to On, and Defensive Play Clock to 15 Seconds.
Existing choices are kept. During a game, team switching is under Pause > Options.

FEATURE LIST
- Separate offense/defense cooldowns (0-10 plays) and limits (0-20 uses).
  Zero disables a restriction. These save per Dynasty; change before a new game.
- Offensive clock: On, Off At Playcall, Off, or 60 Seconds.
- Defensive clock: 3, 5, 15, 20, 30, 45, 60 Seconds, or Off.
  Clock choices apply across offline Dynasties and work independently.
- Controller Select: confirm the other team, finish your current play, then switch
  at the next play-selection screen. CPU takes your old team; no timeout needed.
  Supports one controller with full-team control.

TROUBLESHOOTING
Missing helper or hash mismatch? Re-copy the DLL AND folder from the same ZIP.
The helper EXE must stay in Plugins/OfflineDynastySettings; do not run it yourself.
Logs are in that folder. Include them and the error message in bug reports.
Security software can still block these files; separate packaging is not a guarantee.
