DYNASTY COT by Ace
Public test prerelease 0.1.6-private (nonstable candidate)

This combines four independently optional modules behind one MMC plugin entry:
- Beyond the Field: a livelier Dynasty feed, fog, grades and clearer money labels.
- Gameday Atmosphere: crowds that arrive, react and travel with the matchup.
- Extra Settings: offline controls, including clocks saved for each Dynasty.
- Expanded Practice: full drives and scrimmages from the normal practice menu.

INSTALL / UPDATE
Double-click DynastyCOT-Setup-0.1.6-private.exe. The setup EXE includes the
whole package; there is nothing else to download or extract.
Close MMC and the game. Choose your MMC Mod Manager folder (usually
MMC_ModManager_vX.X.X.X, not its parent MMC folder), select the modules you
want enabled, then click Install 0.1.6-private. Current module choices are loaded.

The ZIP is an alternative for advanced use. After extracting it:
  .\Install.ps1 -MmcRoot 'C:\Your\MMC_ModManager_vX.X.X.X'

Use setup for the first migration. It verifies the package, backs up replaced
files, disables the four standalone adapters and activates COT last.
It preserves Gameday Atmosphere/playcall data and copies Beyond the Field's
RuntimeHistory and RecruitResumeHistory from the legacy Runtime-v103 path. Legacy settings
start fresh. Conflicting history copies stop installation for review.
Later updates keep COT's current history and do not re-import stale standalone copies.

MODULES
Check modules to enable them when installing/updating. Unchecked modules
stay installed but do not run. You can also edit Plugins/DynastyCOT/settings.json,
then restart MMC and the game. The four controls are DynastyEnhancer,
GamedayAtmosphere, ExtraDynastySettings and DrivePractice. The bundled Node
runtime needs no separate installation.

UNINSTALL
Choose Uninstall selected modules, check the modules to remove, then click
Uninstall selected. Or choose Uninstall all COT to remove the host and all
four modules. Close MMC and the game first. Settings, histories, profiles
and backups stay available for reinstalling or restoring a backup.
Selected removal disables that module in the shared host and removes its
external payloads. A changed/unrecognized payload is kept and reported.
Standalone mods are not automatically re-enabled. Restore backup can undo
an uninstall. Installing again restores payloads; choose which modules to enable.

OPTIONAL UPDATES
Open Updates > Check for updates. Choose an available version, including an
older or marked test release, and read its notes and compatibility information.
Download selected release verifies its size and SHA-256. Downloading does not
install; the Install button explicitly names the selected package version.
Use bundled returns to the version embedded in this installer. Nothing checks,
downloads or installs automatically. No GitHub credentials are included.
Project and release instructions: https://github.com/ArtVsTheWorld/DynastyCOT

CHANGES IN 0.1.6
- Toolkit-inspired green setup with separate Install, Updates, Backups and
  Activity pages, module descriptions, older-version choice and manual updates.
- Visiting Fan Strength (%) is an existing Gameday Atmosphere user preference:
  100 default, 0-200 range. It changes newly generated crowd plans; existing
  saved fixture plans and history remain unchanged.
- Beyond the Field has a bounded content/filtering/seasonal/native UI pass.
  See the versioned release notes and verification evidence for supported
  behavior, limitations and remaining live checkpoint acceptance.
- Public test prerelease source: ArtVsTheWorld/DynastyCOT. Internal module IDs,
  history paths, fingerprints and scope/ownership protections are retained.

CHANGES IN 0.1.5
- Rating Colors Off retains native theme/focus colors while grade labels fit.
- Exact AIO 1.4.1 Top Players overview adapter covers both initial Dynasty
  creation and Add Coach within an existing Dynasty.
- Large visiting followings resist distance more. Strong hosts mainly resist
  extra takeover demand; verified AP top-ten matchups have a capped demand boost.
- Added background roar weakens progressively with the home deficit. Earned
  scoring/reaction roars and neutral support keep their own behavior.
- Scoring cheers take priority over live accents, queued followups and competing
  layers, with one bounded delayed recovery for a dropped native selector.
- Confirmed TDs with fresh same-snap recording data can request one inset recovery
  at stable play selection if native replay playback never appeared. Native
  celebrations and active replays retain ownership. Live retest pending.
- Kicker animation playback is not added: native clip/actor binding research
  remains incomplete. No guessed animation or input-dispatch hook is enabled.

CHANGES IN 0.1.4
- Expanded factual Social Media perspectives, persistent account voices and
  replies that respond to the parent's details. Four-edition wording/structure
  memory improves variety across accounts, families and feed views.
- Unchanged narratives wait for a supported development. Essential news stays
  covered; already published wording and history remain preserved.
- Native menu presentation caching, indexed fog/colors/dollars dispatch,
  reused generation lookups and compact new archive records reduce repeated work.
- Shake drives its accompanying roar/swell without waiting for an active-audio
  receipt. Existing timing/intensity and mute/pause/replay guards remain.
- Main visiting sections are 25% smaller with home-fan pockets. Strong home
  programs reduce away travel demand; neutral-site behavior remains native.
- Offline review: 63.5% structural repetition reduction across 60 editions;
  generation averaged 5.5% faster. These are not live menu/FPS measurements.

CHANGES IN 0.1.3
- Weekly Advance / My School Update players-at-risk OVRs have an independent
  adapter, so a neighboring renderer cannot block their grade display.
- Dynasty Enhancements also appears on initial offline Dynasty creation.
  Its choices use the LeagueID allocated by that creation, not a global preset.
- Fresh defaults: Social Media Feed, Recruiting Fog, Roster Fog and Team Fog On;
  player and team grade scales A+-; Rating Colors and Dollar Values Off.
  Existing saved choices remain in effect.
- Compact money labels include $, retaining their native slot-fitting bounds.
  AIO's full topbar dollar amounts remain unchanged.

PREVIOUS FEEDBACK CHANGES
- Clock rows work in the validated offline Dynasty hub as well as gameplay.
- AIO 1.4.1 keeps its full topbar dollar values; tight labels use K/M.
  Player-card Class & NIL text fits inside its slot beside Height/Weight.
- An exact AIO Highest NIL Players adapter applies the roster grade options.
- Recruit account names include available 1-5 star ratings; handles/history stay.
- Drive/Scrimmage switches from the line reopen the playcall picker.
- Crowd motion and sound are paired (revised in 0.1.4 as described above).
- Travel uses city distance, then a weaker state estimate, a modest same-state
  bonus when cities are missing, or neutral x1.00 when location is unknown.
- Green setup with separate install, selected uninstall and full uninstall actions.

CLOCKS
Offensive On / Off At Playcall / Off / 60 Seconds and defensive
3 / 5 / 15 / 20 / 30 / 45 / 60 Seconds / Off save separately for each Dynasty.
New Dynasties default to offensive On and defensive 15 Seconds.
Previous global preferences are not assigned to every Dynasty.

PRACTICE
Normal Default Practice remains the initial mode. Pause > Mode selects
Drive Practice or Scrimmage. No initialization play is required.

ROLLBACK
Open setup, select the same MMC Mod Manager folder, open Backups, choose a
transaction and click Restore selected backup. Close MMC and the game first. Advanced command:
  .\Install.ps1 -MmcRoot 'C:\Your\MMC_ModManager_vX.X.X.X' -Rollback '<printed backup directory>'
Later user edits to settings/history are retained. A changed managed file
requires review before manual rollback. Keep backups and module data folders.

STATUS / REPORTS
Plugins/DynastyCOT/launcher-status.txt reports requested module startup.
Actual native activation is recorded in component logs. Startup success
alone does not prove that game hooks activated. Keep settings/logs for reports.

LIVE VALIDATION STILL REQUIRED
This package has offline build, code, integrity and fixture checks.
It has not been installed into the live game for acceptance.
See LIVE-ACCEPTANCE.md for the checklist. These feedback changes need a
cold launch and in-game testing with AIO 1.4.1 + the TeamBuilder patch.
No blanket compatibility with every Dynasty overhaul is claimed.

Do not delete generated history to update the product.
