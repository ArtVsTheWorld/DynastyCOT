# Dynasty COT by Ace

**Want one feature? Download its standalone ZIP. Want several? Use the Dynasty COT installer.**

Dynasty COT is the home for four optional plugins. The installer manages the same standalone DLLs, so you can switch installation methods without starting a new Dynasty or losing settings and published posts.

[Download the collection installer 1.1.5](https://github.com/ArtVsTheWorld/DynastyCOT/releases/tag/v1.1.5)

Already using COT? Download installer **1.1.5** first to manage these new ZIPs with player guides. Older compatible releases remain available in its update picker. Manual DLL installation works as before.

| Module | Download | Player guide | All features |
|---|---|---|---|
| Beyond the Field | [Version 1.0.24 ZIP](https://github.com/ArtVsTheWorld/BeyondTheField/releases/download/v1.0.24/BeyondTheField-1.0.24.zip) | [Illustrated PDF](https://github.com/ArtVsTheWorld/BeyondTheField/releases/download/v1.0.24/BeyondTheField-Player-Guide.pdf) | [Feature list](Packaging/Standalone/BeyondTheField/FEATURES.md) |
| Gameday Atmosphere | [Version 2.1.38 ZIP](https://github.com/ArtVsTheWorld/DynamicGamedayAtmosphere/releases/download/v2.1.38/GamedayAtmosphere-2.1.38.zip) | [Illustrated PDF](https://github.com/ArtVsTheWorld/DynamicGamedayAtmosphere/releases/download/v2.1.38/GamedayAtmosphere-Player-Guide.pdf) | [Feature list](Packaging/Standalone/GamedayAtmosphere/FEATURES.md) |
| Extra Settings | [Version 0.2.12 ZIP](https://github.com/ArtVsTheWorld/offline-dynasty-settings/releases/download/v0.2.12/ExtraSettings-0.2.12.zip) | [Illustrated PDF](https://github.com/ArtVsTheWorld/offline-dynasty-settings/releases/download/v0.2.12/ExtraSettings-Player-Guide.pdf) | [Feature list](Packaging/Standalone/ExtraSettings/FEATURES.md) |
| Expanded Practice | [Version 0.8.24 ZIP](https://github.com/ArtVsTheWorld/drive-based-practice/releases/download/v0.8.24/ExpandedPractice-0.8.24.zip) | [Illustrated PDF](https://github.com/ArtVsTheWorld/drive-based-practice/releases/download/v0.8.24/ExpandedPractice-Player-Guide.pdf) | [Feature list](Packaging/Standalone/ExpandedPractice/FEATURES.md) |

## Beyond the Field

![Beyond the Field roster](docs/images/beyond-the-field.png)

Follow Dynasty results, recruiting, rankings, awards and more in Social Media. Choose separate recruiting, roster and team rating styles; hide precise interest or use broad recruiting feedback. Recruit Notes bring scouting observations and eligible modeled backgrounds together. Optional estimated workouts, rating colors, compact money labels and GPI player performance grades add more ways to evaluate your program.

Options are under **Dynasty Settings > Beyond the Field**. GPI grades are calculated by the mod from available Dynasty statistics. They are not official professional grades. Existing published posts and saved reports remain unchanged. See the full guide for scouting gates, supported displays and the remaining non-Owner multi-coach limitation.

## Gameday Atmosphere

![Gameday Atmosphere crowd](docs/images/gameday-atmosphere.png)

Crowds arrive, take seat breaks and leave as the game develops. Dynasty turnout and visiting support consider available program and matchup information. New home-site plans use recognizable visiting sections across nearby seating blocks and small scattered parties. Sound and camera effects react to supported plays; visiting touchdowns can quiet the home background.

Choose attendance, sound, camera and replay options in **User Settings > Gameday Atmosphere**, or the supported pause-menu settings. Basic offline Play Now and supported Dynasty Extensions spring games have their own behavior. Saved crowds retain their plans until you explicitly regenerate an eligible fixture. Added positive visiting cheers need at least 20% current modeled visiting support at home sites; native audio is separate. Touchdown replay presentation stays with the game. New section visuals, frame-time behavior and spring boo audibility still need broader live acceptance.

## Expanded Practice

![Expanded Practice](docs/images/expanded-practice.png)

Use normal Practice, full drives or a scrimmage with possession changes. Choose the starting spot, switch sides and inspect session statistics. Open offline Dynasty or main-menu Practice and use **Pause > Mode**. The guide explains the specifically supported Dynasty Extensions combination and remaining restrictions.

## Extra Settings

![Extra Settings](docs/images/extra-settings.png)

Choose separate offensive and defensive playcall limits, cooldowns and play clocks in offline Dynasty. Each Dynasty keeps its clock choices. Find the extra rows in the normal game-settings menu. The guide shows all choices and defaults.

## Install or update

1. Close MMC and the game. Open the versioned folder containing the **MMCModManager.exe you actually use**.
2. For one module, open its ZIP and copy **only its DLL** into that folder's **Plugins** folder. Keep the included PDF, README and feature list for reference.
3. For several modules or migration from older COT/standalones, extract the complete installer ZIP, run setup, confirm the detected MMC location and select your modules. It backs up recognized conflicting copies; do not delete settings/history to fix duplicates.
4. Restart MMC and launch normally. MMC displays the four names with an **Ace’s** prefix. A listed plugin and an active in-game feature are different states.

The installer does not install a separate DynastyCOT.dll. Updates are optional and per module: check, read the notes, choose a version and install it yourself. Older installer-compatible versions stay selectable. Existing three-file and license-bearing ZIP layouts remain supported by the updated installer.

## Keep your data and undo changes

Press **Win+R**, paste `%LOCALAPPDATA%\Ace\DynastyCOT` and press Enter. Each module's guide names its settings and logs there. Module data is shared across MMC copies; current plugins do not require a writable Plugins folder for their settings/history.

To undo a manual update, close MMC/game and restore your previous DLL. For installer changes use **Backups > Restore**, restoring newer transactions first. Keep saves, settings, accounts, Notes and published history. Do not roll back a Dynasty save merely to undo plugin installation.

One provider of each module is enough. Exact supported native layouts and compatible wrappers are checked; unknown layouts can leave a feature inactive safely. [Installation, updates and compatibility](docs/standalones.md).

## Help and verification

Send the module/version, what happened, a screenshot of the affected screen, and the named logs in its guide. Optional trusted-build Defender help is in the PDF; exclusions are not required for every user and setup never changes security settings for you.

Release notes describe changes since the last public release. Feature lists cover the complete current additions. Verification JSON and SHA-256 files are separate release assets. Offline tests do not prove every game, overhaul, Windows setup, sound or visual result.

All four modules are maintained in this source workspace. Their standalone repositories are individual download and support pages, with exact source pins. Game files, SDKs, saves, user histories and private research fixtures are not distributed as source.
