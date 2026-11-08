# Update 3: 2.0.31 camera and reliability review

This candidate replaces the separate camera baseline multipliers and partial zoom correction with one calculation based on the projection of each output view. A wide view receives more rotation and a close-up receives less. Horizontal and vertical motion are corrected separately; roll keeps its own strength. The existing 35-degree, 16:9 reference and the former Standard 1.10 multiplier form the common reference. This reference is a tuning choice, not a measurement of every Standard-camera frame.

The five camera sliders remain personal adjustments from 0 to 200. Existing percentages, including zero, are preserved. Default 100 means the common calibrated baseline. Occupied-seat strength, event eligibility, reaction timing, overlap accents, away/neutral percentages and the independent cannon envelope retain their existing rules. Actual camera angle, position and projection are not changed.

## Changes

- Normal correction is bounded at 4x on extreme wide views. Wide Player Camera Compensation extends the allowance to 6x. It does not add another multiplier to views already inside the normal bound.
- A failed projection read can use that same runtime/view/match/epoch's last good reading for 250 ms. An untrusted view without a fresh reading receives no added rotation. The cache has eight entries per calling thread. Valid zoom changes are corrected immediately, avoiding a lagging multiplier that could temporarily exaggerate a zoom.
- Camera cuts retain the reaction's oscillator and envelope. Pitch and yaw correction, vertical FOV, estimated screen displacement and projection fallback/limit counts are available in technical status. These describe the latest written output and do not prove which view was rendered.
- At default camera preferences, the render callback skips the controller/preset lookup. Earned reactions also skip three camera-owner/view-array reads that only lineup admission needs.
- Failed native reads discard any partially modified value in the main runtime, replay reader, camera runtime, play-call reader, skill reader and mixer reader.
- Unavailable-feature warnings and defensive-motion/showboat/2020-mode events are retained in ordinary technical logging. Failed log batches are counted in status. The readable log still omits routine technical details.
- Settings help and the build guide describe the current camera behavior and flat ZIP with its separately packaged loader. The settings help marker advances to 59; the settings-reset marker stays at 4.
- The installer regression now verifies both flat and legacy package layouts, including replacement failure and rollback in disposable folders.

## Review scope

The review combines a repository-wide file/dependency/test-registration scan, MSVC analysis of the active native runtime translation unit, the complete regression suite, and focused manual control-flow inspection. The manual inspection emphasizes attendance launch/resume/seating, result freshness and event ownership, audio/shake handoffs, camera/native-HFA ownership, replay selection, lifecycle cleanup, settings migration and packaging. Historical release notes remain historical. This is not a claim that every line was manually read or that passing tests prove the absence of bugs.

The earlier missing-TD-replay, stale sack, forced-fumble duplication, delayed-result and scoring-sound reports were checked against the current guarded paths and their existing regression fixtures. They are not treated as new live reproductions. This pass does not introduce a speculative replay rewrite or remove intentional native/DGA cheer overlap.

## Measurement and verification

The added projection test projects a fixed world point through the resulting rotated matrix and compares movement over 684 FOV/aspect/strength/preference/oscillator combinations inside the correction bounds. Separate cases check extreme limits, invalid readings, cache expiry and ownership. Runtime fixtures check gameplay/cutaway admission, source preservation, accessibility, pause/replay cancellation, preference zero, stale cues and native hook ownership. Settings regressions cover preserving custom camera and away/neutral percentages and repeated migration.

A local self-process camera fixture ran 23,000 output operations for each version, with two warm-up batches excluded. For default preferences, baseline median batch-average cost was 11.925 microseconds per output versus 8.736 for the candidate; the 95th-percentile batch averages were 12.791 and 9.499 microseconds. The preference fixture separately verifies nine native reads per default lookup before and zero after. These are controlled fixture measurements, not whole-game frame times, per-call tail percentiles or proof of fewer visible hitches.

Exact build/test/package results and hashes belong in the staged BUILD-INFO and private validation record. MSVC analysis reports intentional native-exception guards and local-name shadowing; its conditional-precedence warning was clarified without changing reaction selection.

## Live checks still needed

1. Compare the same gentle lineup effect across Standard, Zoom, All-22, Wide and Broadcast at 100. Check moving zoom as well as settled framing.
2. Compare a stronger reaction and its on-field touchdown celebration cut. Check for pulsing or a sudden strength change.
3. Check a mixed offense/defense preset choice and a player-lock/RTG view when available. Use the extended-wide toggle only if the normal bound proves too restrictive.
4. Confirm camera preference zero, away zero/50/100 and neutral 75 behave independently, and that preferences survive leaving/reopening Settings.
5. Check ordinary play, a touchdown replay sequence and pause/replay/postgame cleanup for regressions.

Actual crowd regeneration remains heavier than routine callbacks. Its cost is not removed by these camera optimizations. Native audio dispatch/registration is not audible-output proof; replay flags are not rendered-sequence proof. Camera movement must still be judged in game because framing, screen position, native motion and visual perception affect the result.

The candidate is staged privately. This work does not install or publish it.
