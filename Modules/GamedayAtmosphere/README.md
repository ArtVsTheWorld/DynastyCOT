# Gameday Atmosphere

By Ace · **Update 4 private development — 2.1.9** · College Football 27 for Windows · MMC

This is a private Update 4 test build. It adds protected bands and academy sections, visiting-fan appearances, separate home/away departures and stronger save-history ownership checks to the revised attendance system. Those changes still need live visual and save/load checks. Automatic celebration filtering and the option to prevent presentation skipping are still research work and are not enabled. This is not the final Update 4 release.

Crowds arrive, take halftime breaks and leave early. Crowd sound follows the fans in the seats, while big plays bring extra reactions, camera shake and replay opportunities.

[Download](https://github.com/ArtVsTheWorld/DynamicGamedayAtmosphere/releases/latest) · [Full feature list](FEATURES.md)

## Install or update

1. Close the game and MMC
2. Unzip the download
3. Copy **DynamicGamedayAtmosphere.dll** into MMC's **Plugins** folder, replacing the old DLL
4. Open MMC and launch the game normally

**That's the only file you need to copy.** You do not need to copy a runtime folder or the `.md` documents.

Keep your DGA settings and History folder. This build backs up older audience foundations as `audience-foundations.pre-2.1.4.txt` inside History and corrects established-school estimates once. It keeps TeamBuilder estimates and saved performance references. No INI deletion or game-save edit is needed.

New visiting-fan plans and crowd-history ownership links are saved automatically inside that same History folder. Older crowd history without a verified fixture link uses an attendance estimate when resumed; it is not assigned to a save just because the teams, score and clock happen to match. Once this build has recorded a resumable checkpoint, future matching loads can use its linked history.

```text
MMC/
  Plugins/
    DynamicGamedayAtmosphere.dll
```

DGA prepares its own runtime automatically. Keep your existing `Plugins/DynamicGamedayAtmosphere` folder: it contains your settings, logs and crowd history. Updating does not require deleting that folder or its INI.

Generated native runtime files now live in `MMC/DGA-Runtime`, outside the folder MMC scans for plugins. Older DGA runtime files are archived there automatically once the game is closed. An old native-DLL warning can appear during the first upgrade startup; close and reopen MMC after that cleanup. You should not need to reinstall between launches.

Protected cadets/midshipmen now count within the crowd target. For example, a 10,000-person target with 2,000 required spectators leaves room for 8,000 ordinary fans. Bands stay separate from fan fullness. Old saved/replay crowds keep their earlier temporary-seat rules until that interval finishes. The new budget still needs a live visual check.

Protected showcase rivalries can now get the gentler opening kickoff shake even when neither team is ranked. If visitors open on offense, the loud defensive opening lasts until their first down or punt; qualifying short stops and incompletions can each get the biggest cheer. Scores and possession changes end it too.

## Settings

Open **Gameday Atmosphere** from either:

- **Dynasty/RTG Central → User Settings**
- **Pause → Options → Settings**, at the far right after Auto-Subs

Most people can keep the defaults. Turn on the game's **Dynamic Camera Effects** if you want added shake. Broadcast includes separate shake sliders for Standard, Zoom, All-22, Wide and Broadcast cameras. The actual lens balances shake automatically, including modded cameras: tight views feel a little stronger than wide views. 100 uses the recommended balance, 50 halves it and 0 turns it off for that camera. Existing slider choices are kept.

Use **100 on the camera sliders** for the new balance. If you lowered them to 90 while testing the previous build, return them to 100 after updating; that reduction is now built into Standard/wider framing.

**Away Team Reaction Shake** is now On/Off and starts On. Its strength follows the visiting supporters still in the stands. Off keeps their sounds but removes their added shake. **Neutral-Site Crowd Shake (%)** still controls both teams at neutral stadiums and starts at 75.

**Existing Update 3 settings are kept.** Updating from Update 2 resets the old settings once and saves a backup automatically. You do not need to delete any INIs.

This candidate includes a menu fix for AIO Broadcast Beta1. It keeps DGA settings separate from the broadcast switcher. Both menus still need a live check together before this compatibility fix is considered confirmed.

## If something is wrong

Send the whole **`Plugins/DynamicGamedayAtmosphere/Logs`** folder and briefly describe what happened. For a problem during a game, include the teams and quarter/clock.

If startup fails, include `launcher.log` from that Logs folder. The new build handles loading itself; there is no separate loader EXE or runtime folder to install.

Offline Dynasty and Road to Glory have been tested on the game update checked October 30, 2026. The revised peak-attendance formula applies to fresh regular-season Dynasty games. See the full feature list for mode differences and replay limitations.

COT release candidate: Visiting Fan Strength (%) uses the normal model at 100, a minimum visiting share at 0, and stronger traveling demand up to 200. It is saved in the existing Gameday INI. It affects only newly generated fixture plans; saved plans and neutral venues remain unchanged. Protected groups remain protected.
