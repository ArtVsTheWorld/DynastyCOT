# Update 3: 2.0.32 camera calibration

The user accepted the 2.0.31 Standard preview at a 75% camera preference, then accepted Zoom at that same preference as close enough to Standard. Build 2.0.32 folds that factor into the shared camera gain: 1.10 becomes 0.825. At a factory camera preference of 100, the new build reproduces the accepted preview strength.

This is one common strength adjustment. Event strengths relative to each other, occupied-seat scaling, reaction timing, overlap accents, away and neutral percentages, cannon eligibility, camera projection correction and accessibility controls retain their existing rules. Existing personal camera percentages are preserved. All five factory sliders remain at 100. The shared gain applies to DGA's added motion, including cannon movement; it does not change the game's own camera effects.

## Live evidence

The accepted checks ran on 2.0.31, PID 26012, at the on-field lineup in the same game. The preview used a fixed raw ordinary target of 5.0 and the supported camera preference of 75. The helper confirmed actual output writes with the 0.750 preference, then restored the complete INI to its original bytes.

- Standard: captured vertical FOV 54.449 degrees; pitch/yaw correction 1.632x; user accepted the gentle baseline.
- Zoom: captured vertical FOV 37.896 degrees; pitch/yaw correction 1.089x; user accepted the comparison as close enough.
- Both reported a peak small-angle estimate of 0.074% of frame height and 0.029% of frame width. These are output estimates, not measurements of visible pixels.

The Standard capture contains about five seconds of confirmed application and Zoom about fourteen. The user chose a short Standard calibration and Zoom check instead of an event-by-event or five-camera audition. All-22, Wide, Broadcast, dynamic cuts and individual event strengths are therefore not newly live-verified by these checks.

## Regression coverage

The projection regression retains its FOV/aspect/strength/preference comparisons and adds 36 effect/FOV equivalence checks. They compare the new common gain at preference 100 with the prior common gain at preference 75. Runtime expectations also cover the revised output, with the existing zero, ownership, projection lease, accessibility and pause/replay checks retained.

Exact build, simulation, configuration, managed, MMC and package results are recorded in the staged BUILD-INFO. The 2.0.31 reliability fixes and camera normalization remain included. No extra performance improvement is claimed for changing this gain constant.

The build is staged privately. It is not installed or published by this work.
