# 0.1.6 test prerelease: live acceptance pending

The candidate is built and checked in isolated fixtures. No live installation, save overwrite or live-game compatibility certification is part of this build. Successful startup, bytecode emulation and offline timings do not prove native menu responsiveness or gameplay compatibility.

Use copies of the original DYNASTY-EPUFT checkpoints. Save any test progress under a new name. Filenames are labels, not authoritative native stages; manual saves and their autosaves can be at different stages. Prefer the named manual checkpoints for the steps below.

## First: install and persistence

- [ ] Close the game and MMC. Run the versioned installer, select the actual MMC Mod Manager folder, inspect all four module descriptions and install your chosen modules. Confirm it reports 0.1.6-private.
- [ ] Load a copy of `DYNASTY-EPUFTPRESEASON`. Check module activation logs, retained account identities, previous published posts and settings. Open Extra Settings and Beyond the Field from the normal Dynasty settings menu.
- [ ] In a second Dynasty, use different fog/clock choices, then switch back and reload. Choices and publications must remain isolated. Check initial offline creation and Add Coach paths separately.

## Priority 1: reported UI defects with the active overhaul combination

Persisted MMC pack audit found AIO Dynasty 1.4.1 followed by TeamBuilder Compatibility 1.1.0, DARK v4 and the dollar-icon mod. Record your actual game build and current order before testing. Other combinations need their own acceptance.

- [ ] `DYNASTY-EPUFTRIVALRYWK13` (or `WK4` / `WK7`): open the recruiting board and Recruit Notes. Toggle Rating Colors On/Off, change selection, scroll and return. Enabled colors should appear; Off must retain native selected/unselected colors. Fog must still obey scouting visibility.
- [ ] Open Owen Lineberger's recruiting card (verified 3-star kicker prospect in the Rivalry, CCG, Bowl 1, EOS and first-draft checkpoints), then several other cards. Check the dollar wallet beside recruiting hours, initial entry and after editing the scholarship. Only the money label should change; hours and raw editing/sorting values must remain native.
- [ ] `DYNASTY-EPUFTEOSRECAP` (native OffSeason 0), then `OFFSZNREC1DRAFT`: open roster NIL allocation, change Current NIL repeatedly, sort, switch player, leave/re-enter and toggle Dollar Values. Compact dollar text must survive deferred redraws and widget reuse without changing native budget arithmetic.
- [ ] `DYNASTY-EPUFTCCGWEEK` (159 captured offers) or `BOWLWK1` (489): inspect coordinator/coach offers before and after editing. K/M labels must stay inside their native bounds. Check long names and selection contrast.
- [ ] Regression: Weekly Advance players-at-risk grades remain readable when selected with Rating Colors Off; Select Your Team Top Players show the requested grades in initial creation and Add Coach; broad topbar dollar amounts retain their intended layout.

## Priority 2: feed correctness and continuity

- [ ] `PRESEASON`, `WK0`, `WK4`, `WK7`, `WK10CFPPOLL`: inspect every LB/RB category in National, Conference and Team scopes. An empty view should mean no eligible posts, not a limit applied before filtering. Compare the displayed count/page label with additional pages in the existing week selector.
- [ ] Switch categories/scopes/pages, open replies, sort and return. Confirm parent context is retained, navigation focus stays usable and text is not clipped. Check a quiet edition and a busy recruiting/result edition.
- [ ] Read WMU's unranked 31–26 win over #14 Michigan where supported by the checkpoint. New-generation review should have distinct winner, loser and analyst perspectives. Existing published posts intentionally retain their original wording.
- [ ] Reopen/refresh the same edition, restart and reload. New publications and recruit modeled-game records must stay stable; previously published wording/authors must not change.
- [ ] `RIVALRYWK13`, `BOWLWK1`, `NCWK`, `OFFSZNWK4`, `NSD2`, `TRAINING`, `CONFERENCESCUTS`, `PRESZN2027`: inspect seasonal and offseason coverage, major commitments/results/awards, conference wording and year rollover. Verify no duplicate holiday burst when reopening. Calendar events require a supported date anchor; missing dates should not invent a holiday.
- [ ] Recruit game/film posts must be identified as modeled where simulated, fit the position and playing season, and never reveal a development trait. Native ratings, traits and statistics must remain unchanged.

## Priority 3: draft reference

- [ ] Use a copy of `DYNASTY-EPUFTOFFSZNREC1DRAFT`: this is CurrentYear 0 / 2026, native OffSeason week 2, stage 3, corresponding to the requested displayed first-offseason week. Compare the first-offseason reporting order with the supplied Tankathon reference. The supplied file is a reference snapshot, not a certified final historical order.
- [ ] Verify player draft outcomes/rounds and native game data remain unchanged. Check the next offseason uses its normal behavior. Existing published reports must not be rewritten.

## Priority 4: visitor slider and native gameplay

- [ ] At Attendance settings, set Visiting Fan Strength to 50, 100 and 200; close/reopen and restart to verify persistence. Generate a different fresh fixture for each comparison. Existing saved fixture plans should retain their prior visitors even after changing this preference.
- [ ] Compare small/far, nearby rivals, major visitor/weak host and two strong programs. Check neutral-site behavior, bands/cadets/protected groups and the retained travel model. 0 is minimum support, not removal of protected spectators.
- [ ] Smoke-test pre-play shake with a matching roar, score cheers, deficits, pause/replay/Slow Sim and successive games. Check normal practice and full-drive/scrimmage switching, plus per-Dynasty offensive/defensive clocks.

## Performance and rollback

- [ ] On the same checkpoint and menu route, record cold entry and warm re-entry separately. Compare feed category/page changes, roster, recruiting board and Notes with features enabled/disabled. Record frame times or FPS and stalls. Offline generation/callback timings are not live FPS measurements.
- [ ] With MMC/game closed, use explicit Check for updates, inspect notes/compatibility, then deliberately choose whether to download/install. Staying on the current version must remain possible.
- [ ] Remove one module, restore it, remove all and restore a chosen backup. Confirm later settings/history and other mods survive. Never use the original checkpoints for destructive tests.

For a failure record the checkpoint, native year/stage, screen, enabled modules, game/overhaul versions, expected/actual behavior and relevant logs. Narrow it with the relevant module alone. Unknown fingerprints and foreign hook ownership must continue to fail safely.
