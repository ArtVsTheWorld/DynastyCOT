# Dynasty COT 1.0

**By Ace.** Four optional additions for College Football 27's offline Dynasty mode. Pick the features you want in one installer; MMC lists one plugin named **Dynasty COT**.

[Download](https://github.com/ArtVsTheWorld/DynastyCOT/releases) · [Release notes](docs/release/1.0.0.md) · [Help and rollback](docs/release/README.md)

**1.0 is available now.** [Download the Windows installer](https://github.com/ArtVsTheWorld/DynastyCOT/releases/download/v1.0.0/DynastyCOT-Setup-1.0.0.exe). Existing users can install over a supported earlier version; settings and history are kept.

## Beyond the Field

![Beyond the Field roster and player card](docs/images/beyond-the-field.png)

- **A livelier Social Media feed:** results, rankings, recruiting, awards and more, with reporters, team accounts, players and fans. Browse For You, National, conference, team and topic filters, or return to past editions. National shows up to 300 cards; extra wording choices add variety without increasing the weekly post budget.
- **More uncertainty when building your team:** separate recruiting, roster and team fog options let you see less exact information. Recruiting Notes keep a recruit's developing profile together.
- **Choose how ratings look:** use letter grades, descriptive words such as GOOD, or the other included scales. Rating colors are optional. Compact money labels turn long amounts into readable $K/$M values.
- **Player Analytics:** browse a selected team's weekly and season performance grades, snaps and player cards. These are mod-calculated, PFF-style grades from available Dynasty statistics, not official PFF grades. Only completed weeks contribute. National Analytics and the Player Stats shortcut are not included.

Beyond the Field fog, rating styles, colors and money formatting apply inside Dynasty; main-menu and Play Now displays stay native. Find the options under **Beyond the Field** in Dynasty settings. Social Media and fog start on; rating colors and dollar formatting start off. Published posts, accounts and history survive updates.

## Gameday Atmosphere

![Gameday Atmosphere visiting supporters](docs/images/gameday-atmosphere.png)

- **Crowds with a reason to be there:** attendance considers the program, results, opponent, venue and supported game context. Fans arrive, take breaks and leave as the game develops.
- **Traveling supporters:** recognizable visiting sections and small groups use fanbase demand, distance and matchup appeal. Strong home support resists takeovers. Winning visitors can still leave early, more slowly than a badly beaten home crowd.
- **More reactive sound and shake:** crowd energy builds before plays and reacts to important moments. Crowd fullness and the score affect background noise; stronger reactions take priority. Shake requests an accompanying crowd sound.
- **Your choice of atmosphere:** in-game controls cover attendance, sound, shake and replay options. Visiting Fan Strength starts at **100%**; lower or raise it to influence newly generated visiting-crowd plans. Saved plans and history stay intact.

Bands and verified military crowd groups retain their protection. Neutral-site crowd appearance stays native. Find the module under **User Settings > Gameday Atmosphere**.

The **basic offline Play Now** path adds crowd reactions, sound, shake and departures. With arrivals enabled, fresh games start at 75% of the available crowd, then fans continue arriving. Visiting-fan appearance stays native; Dynasty travel, season plans and saved crowd history are not used. Added replay changes skip touchdowns so the game controls their presentation. Additional Replays remains available for other eligible plays. Ace installed the final build, restarted MMC and the game, and reported the fixes working.

## Expanded Practice

![Expanded Practice drive](docs/images/expanded-practice.png)

- Keep normal practice, run full drives, or play a scrimmage with possession changes.
- Start from a chosen spot, switch sides and check session statistics.
- Drive modes track downs, field position and scores through the normal practice screen. Default Practice remains available.

Enter offline Dynasty practice or main-menu Practice and use the pause-menu Mode control.

## Extra Settings

![Extra Settings playcall and clock controls](docs/images/extra-settings.png)

- Set offensive and defensive playcall cooldowns and limits.
- Choose offensive and defensive play clocks independently, including extra durations and Off options.
- Each Dynasty keeps its own choices. New Dynasties start with offensive clock **On**, defensive clock **15 seconds**, and playcall restrictions off.

Open **Dynasty > League Settings > Game Settings**. Start a new game after changing gameplay settings.

## Install or update

1. Download the **DynastyCOT-Setup** EXE from [Releases](https://github.com/ArtVsTheWorld/DynastyCOT/releases). Keep your older installer if you may want to go back.
2. Close **the game and MMC Mod Manager**.
3. Open setup. It tries to find MMC automatically. If needed, click **Find exe** and select **MMCModManager.exe** inside the versioned `MMC_ModManager_v…` folder.
4. Check the modules you want and click **Install COT**. Open MMC and launch normally.

Use the installer when upgrading from older COT or supported standalone modules. It backs up replaced files and keeps settings and history. If an old file has been changed or is unrecognized, setup reports it for review rather than guessing. Do not keep old standalone plugins enabled alongside COT.

**Updates are your choice.** Use **Check for updates**, read the notes, then decide whether to download and install. Setup also supports selected-module removal, full removal and restoring a backup. It does not automatically force the newest version.

## Compatibility and data

Beyond the Field and Extra Settings target offline Dynasty. Expanded Practice also supports offline main-menu Practice, and Gameday Atmosphere has a basic offline Play Now path. AIO 1.4.1 and its TeamBuilder patch have targeted adapters; compatibility with every overhaul/version is not guaranteed. Unknown native programs remain rejected. See the [regression checklist](LIVE-ACCEPTANCE.md).

The module DLLs now use the new branding. Older internal IDs and data-folder names remain intentionally unchanged so existing settings, published history and crowd plans keep working. One `Plugins/DynastyCOT.dll` hosts the selected modules; native runtime DLLs live outside MMC's plugin scan. Logs stay inside module folders. Extra Settings and Expanded Practice retain their verified runtime helpers.

## Build and contribute

See the [source build and release guide](docs/release/README.md). Building requires Windows, a compatible local MMC SDK, MSVC, PowerShell, Python and Node, plus private, legally obtained native research inputs. SDKs, saves, histories and private research are excluded from this repository. A public checkout alone cannot run every native verification step.

Imported-source records are under `Baselines`; compatibility contracts are under `Compatibility`. Required third-party notices remain included. Automated verification is separate from live-game acceptance.
