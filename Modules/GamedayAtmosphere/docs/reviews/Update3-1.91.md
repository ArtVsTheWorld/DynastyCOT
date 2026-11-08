# Update 3 review — candidate 1.91.0

29 October 2026. Reviewed on `update-3-diagnostics`, starting from 1.90 commit `437a8eba5ea541427b3d29490c94dcfdd2fd0379`. Candidate only; not installed or published by the agent.

## Requested additions

**Venue-scaled crowd motion:** linear, clamped from 0.35× at 6,060 to 1.10× at 61,888 **authored seating positions**. Uses the same catalog endpoints as fullness qualification, with the complete owned rendered-layout denominator. Official capacity, current occupied seats and a single camera's draw are not substituted. The factor applies once to lineup/critical/opening, live gain/defensive impact, TD/safety/blocked kick/clincher and eruption targets before envelopes and transitions. Neutral crowd motion then receives its existing 0.75×. Unknown whole-layout ownership cannot create motion. Fullness gates are unchanged. The independent native cannon impact and EA HFA camera policy remain separate.

Examples for a configured 15% target: 6,060 positions → 5.25%; 30,000 → 10.07%; Michigan catalog 51,424 → 14.39%; 61,888 → 16.50%. These are target strengths, before event envelopes and view compensation.

**Final-play carry:** only a fresh, already-earned reaction from the same match, epoch and play can continue into final presentation. Existing camera envelope timers and oscillation phase continue. An open-ended live reaction gets a bounded settling tail. Ordinary carry is bounded by the configured 1–8-second fade; scoring carry has an 11-second maximum after final. Existing earlier expiries still win, and long custom camera holds receive a final fade rather than an abrupt cutoff. No extra one-shot cheer, event, replay or score is created. Unconfirmed run buildup and preplay shake do not become postgame reactions. Eruptions keep their existing stronger carry and take precedence. Pause/interactive replay, changed match/epoch/play, possession-loss cleanup, disabled sound/camera and the postgame menu cancel the applicable carry.

## Bugs fixed during the review

| Finding | Correction and evidence |
|---|---|
| The earlier Q4 traffic policy was still blocked by two old last-quarter-of-Q4 checks in its production caller. Testing only `LateTraffic` had missed the integration conflict. | Removed the stale target and extra-quarter routing gates. The policy itself still controls last-third / gentle last-half entry, deficits, neutral/postseason limits and recovery. Added direct production-target/cap regression cases. |
| Traffic recovery used fixed 12-minute-reference possession costs against unnormalized custom-quarter clocks. | Normalize remaining time to the same reference before estimating recovery. The quarter-length simulation sweep now gives identical final fullness at fixed normalized hidden-window cadence for all tested lengths. |
| A quick snap could copy the low raw roar-attack weight while preplay excitement was requesting its early excited-voice floor. | Share the audible envelope when carrying into the snap. Regression cases snap 1–2,000 ms into the attack and require continuity. This reproduces a possible drop in DGA's requested level; it does **not** prove this caused the user's earlier recorded silence or that a cheer is audible. |
| Opening the final menu before the first unpaused final snapshot could leave the menu-unseen flag unset, permitting a later upset-win eruption on highlight/unpause. | Latch menu completion even when it is the first observed phase-7 snapshot. Test verifies no new eruption or sound intent afterward. |
| A native menu read that faults partway through `memcpy` could return a partially written value. | Explicit zero/default return in the exception path. Real Windows page-boundary test covers a readable first half and inaccessible second half. |
| Static analysis could not prove that each generated trampoline fitted the independently selected allocation length. Two startup/diagnostic paths also put ~64 KiB path buffers on the stack. | Construct trampoline bytes first and allocate/copy their exact size, retaining byte checks and write/execute protection transitions. Put path buffers on the heap and reject missing/truncated paths. The old constants were consistent; this is strengthened allocation proof, not evidence of an observed overrun. Current analysis no longer reports those buffer or stack warnings. |

## Review scope and retained invariants

The pass traced the production data flow through starting-turnout capture/calculation/publication, launch identity/capacity and loading transitions; rendered occupancy and original seat masks; arrivals, halftime, departures, history and resume; atmosphere admission; result/live/skill/ownership audio and motion; replay modes and major replay routing; callback exclusion, settings, startup, migration, storage and diagnostics. Existing native/managed integration suites exercise the adapters and callback paths in addition to pure policy tests. This is a substantial review and verification pass, not a proof that every possible native-game behavior is bug-free.

- Peak turnout remains a fresh regular-season Dynasty launch decision. Saved/simulated native season records, postseason peak attendance and unsupported inputs retain EA behavior. The supported loading witness was retained; no redundant online gate was added.
- Grade weights remain atmosphere 40%, tradition 20%, brand 15%, current rank 15%, conference prestige 10%. Percentile ties, incomplete grades, valid ranks, local-night detection, indoor weather exemptions, deterministic fixture variation, floors/caps and neutral symmetry retain coverage.
- Starting attendance is a share of peak. Visible density is peak occupancy times currently seated share. A low-demand crowd cannot unlock layers merely by reaching 100% of its smaller peak. The 35% kickoff-share minimum, weather-only below-40 exception, severe-school timing, exceptional 80% cap and postseason opening bands are retained.
- Original empty seats remain empty; only owned temporary absences can return. Permanent exits are monotonic, respect current floors/quarter caps, and are settled at safe windows. An improved score does not refill fans who already left.
- Matched Q1 saves keep their original launch plan and exact saved visible crowd at the same paused clock. History remains bounded and backward-readable. Replay snapshots and Super Sim retain their separate clocks and ownership.
- Sound-only reactions below layers do not acquire a fresh camera cue. Ordinary broken tackles and successful spins/jukes do not start shake. A separately earned gain/TD tail can continue. Sparse-crowd native big-play peaks remain independent of bed calming. Native HFA motion above its separate 50% filter threshold can still occur below DGA's stronger layer gate.
- Possession-loss cleanup, fresh sack-selector proof, released-pass exclusion, kneel/spike suppression, fumble/breakup/hit ownership and once-only event identities retain dedicated tests. No unverified spectacular-catch signal was added.
- The requested replay sequence remains mode-aware: Slow Sim has no play-call-screen inset destination. Native recording availability, authored camera packages, injury interruptions and the accepted intermittent TD-selection limitation are not claimed fixed by this build.
- Shared callbacks retain the exclusion gate and model mutexes. The new carry state uses the same lock. Startup/diagnostic allocation changes introduce no per-frame heap buffer or extra live engine calls.

## Verification and simulations

- Full automatic native/managed build and regression suites pass, including 504 configuration-reader audit cases. Final native rebuild and affected integration/model suites pass after the menu-read and first-menu-snapshot hardening.
- Eight Python tests pass. The isolated MMC adapter passes ABI, metadata, embedded resource/extraction and storage/logging checks; installed files are not touched.
- MSVC `/analyze` was run on the production translation unit. Remaining findings are reviewed broad SEH boundary handlers and helper parameter shadowing; memory/stack findings above were addressed. This is not a runtime race detector or a substitute for the game's live behavior.
- 57 detailed traces: 19 score/demand profiles × 3 authored venue sizes. A 684-case full-game sweep spans 60/300/720/1200-second quarters and hidden updates every 10/30/120 normalized game seconds. The optional captured 138-school cohort adds 75,624 peak-formula cases.
- The harness uses production turnout, arrival, departure, halftime/conversion, fullness/atmosphere admission and quieting policies. It asserts finite bounds, monotonic exits, applicable floors, deterministic results and no added camera qualification below absolute 50% fullness.
- Fixed-cadence quarter-length final-fullness spread: 0.00 capacity points in the tested scenarios. Hidden-window cadence spread: at most 1.62 capacity points. Visual timing and halftime conversion can legitimately depend on hidden opportunities.

The review report contains all curves, qualification windows, cohort distributions, raw CSVs and assumptions. Native seating-mask generation is approximated by peak/capacity. Profiles select generic support percentiles; actual school identities are not the tuning target. Postseason examples explicitly assume an EA-supplied 100% peak. Qualification windows are standardized ordinary-down / third-and-10 probes, not predicted plays or proof of sound/shake. No fabricated injury, penalty, turnover or late-score event is injected into the synthetic score paths.

## Tuning observations and outstanding live work

1. Representative close ordinary games end 13% below their own peak; the elite night rivalry ends 7% below. Lowest support can be only 12.5% visibly full at kickoff (25% peak × 50% share), and cannot reach layered thresholds. These are current policy consequences for user review, not silently changed tuning.
2. Rainy decided games thin substantially. The middle-support home rout finishes ~22.6% capacity; a ranked elite home winner retains its 35%-of-peak floor, finishing ~33.7% capacity in the example. A severe rainy home loss can reach its existing 15%-of-peak floor. Review these curves before further attendance tuning.
3. Night detection and attendance/opening bonuses are active. A separate night-only departure adjustment or blanket ~10% atmosphere-threshold reduction is **not** active. These remain explicit future tuning decisions rather than changes hidden in this bug-fix pass.
4. First live check: compare a qualifying small and large venue on 1.91 with ordinary and earned reactions; verify the status venue factor and smoother snap. Confirm a sparse below-layer stadium still emits no new DGA crowd shake. Native HFA/cannon effects need separate attribution.
5. End a game while an ordinary earned effect is still active; verify sound/motion settle into final presentation and release at the menu. Also check an eruption still keeps its existing longer carry, and that an immediate final-menu entry cannot restart it.
6. Retest the prior strip-sack → ordinary losing run sequence, plus genuine consecutive sacks, because the 1.90 selector fix is included here but has not received installed-build acceptance. Sparse reaction audibility and Slow Sim rendered replay confirmation remain useful checks.

There is no new live performance measurement, audible-output proof, rendered camera/replay proof, or complete native function trace from this task. No game/MMC/INI/save/history installation or release publication was performed.
