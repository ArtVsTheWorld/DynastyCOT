# Update 3 — defensive fumble-return motion

Candidate 1.95.0 restores verified live defensive fumble returns to the interception-return camera path. The live detector already requires a tracked fumbler, fresh loose-ball or forced-fumble proof, and grounded held possession by the opposing side. No recovery is inferred from an official result alone.

Home defensive returns qualify at home venues; either defensive side can qualify at neutral venues. Existing layered/fullness, accessibility, pause/replay and simulation gates remain in force. Occupied-seat strength, neutral attenuation, live continuity, whistle fade, touchdown/eruption priority and bounded final-play carry use the same shared camera path as interception returns. A preceding forced-fumble impact blends into the live return without a new pickup impact.

Own-team recoveries emit no return event, never revive the old gain buildup and cannot restart an existing forced-fumble impact. A previously earned force may finish its original effect; the pickup does not cause it. Official-result-only recoveries remain excluded from completed-impact fallback. The disabled recovery-yardage fallback remains disabled.

Regression coverage compares live defensive fumble recoveries with interceptions through the production camera router and director across carrier side, neutral venue, layered admission and forced/unforced loose balls. It checks one event per held recovery, continuous motion, force-to-return handoff, whistle fade and cancellation. Own-team pickups are checked for both sides at home and neutral venues, with and without a forced-fumble marker. Existing official-result, prior-gain, audio, defensive routing, postgame carry and ownership regressions remain required.

Validation completed: the full automatic native/managed build and regression suite passed, including 828 full-game production-policy attendance simulations; all eight Python catalog/compatibility tests passed. Isolated MMC checks passed for ABI, metadata, resources, extraction, storage and logger fallback. No installed files were changed by those checks.

Installed-build visual acceptance remains pending: compare a defensive scoop-and-return with an interception return under qualifying atmosphere, then recover your own fumble and repeat below the layered gate. Distinguish an already-earned force impact from motion newly caused by a recovery. No attendance tuning, crowd variation, replay selection, installation or publication is part of this change.
