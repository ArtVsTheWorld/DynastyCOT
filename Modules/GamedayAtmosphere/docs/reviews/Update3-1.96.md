# Update 3 — lower-bowl variation

Candidate 1.96.0 narrows the crowd-variation work to the issue observed in the user's live Kyle Field session. On runtime 1.92, rendered counts were verified at 90%, 70%, 50% and 30% of the original crowd. The 90% view was convincing; medium/sparse views retained fairly uniform lower sidelines while visible thinning concentrated upstairs. The 90/70/50 views shared a wide composition; the 30% capture was closer and supports qualitative comparison. The original 57% crowd, automatic mode, replay controls and unchanged attendance-history file were verified after restoration.

## Scope

The existing immutable seating order remains the basis for arrivals, permanent exits, halftime absence, historical replays and resumed crowds. No NativeRuntime, timeline/history-format, arrival schedule, departure formula, renderer coordination, audio, camera or replay-selection logic is changed by this candidate's spatial work. The previously drafted separate phase orders and bowl-wide renderer budgets are deferred; that draft is preserved privately, not shipped.

At 90% retained crowd and above, the existing selection is preserved for identical authored positions, original empty-seat masks and match seed. Below that, a smooth blend reaches full adjustment at 50% retained crowd. The near-field spatial weight feathers out between the 25th and 70th distance/height percentiles. It relaxes some front protection and adds persistent broad/local pockets there. Existing groups and individual stragglers remain, and depth-based preference still retains more fans near the field overall. These percentiles are geometric proxies, not native deck/section IDs.

The change selects a different set of seats for the same target. It does not change the total number of spectators, invent new original fans, recolor clothing or move fans between seats. Native empty positions remain empty. A lower native peak can still impose its own sparse seating distribution; this adjustment acts on DGA's additional thinning. Shared nested masks avoid permanent refills and preserve the existing replay/halftime restoration mechanism.

## Offline comparison

Eight seeds on a synthetic 9,216-position bowl, with the same total at every comparison. “Front” is the closest 35% of positions by field-distance/height; “back” is the farthest 25%. These are illustrative model measurements, not Kyle Field captures or real attendee counts.

| Overall retained crowd | Front: current → candidate | Back: current → candidate |
| --- | --- | --- |
| 90% | 96.7% → 96.7% | 81.5% → 81.5% |
| 70% | 86.3% → 81.1% | 49.5% → 53.6% |
| 50% | 69.7% → 57.6% | 27.8% → 36.0% |
| 30% | 46.1% → 36.4% | 12.9% → 19.3% |

Front-area occupancy variation across 24 synthetic seating areas increases from 11.6 to 17.3 percentage points of standard deviation at 50% overall, and from 11.3 to 16.7 at 30%. These are outputs of the fixture, not prescribed section targets. The front still stays fuller than the back in the aggregate. A first, stronger relaxation flattened that preference too much and was reduced before staging.

The regression uses before-change 90/95/100% mask fingerprints, exact/nested seat-count checks, native-hole exclusions, repeatability, and broad multi-seed spatial bounds. Existing production visibility, original-baseline, arrivals, halftime, replay, resume and postgame tests remain required. Pattern preparation does the extra spatial work once per renderer; the per-fan visibility callback retains its existing constant-time rank lookup. No live overhead measurement is claimed.

Validation completed: full automatic native/managed build and regression suites passed, including 828 full-game production-policy simulations and the production visibility/arrival/replay/resume fixtures. All eight Python tests passed. Isolated MMC ABI, metadata, resources, extraction, storage and logger-fallback checks passed without modifying installed files. The staged synthetic comparison image was visually inspected; it is not installed-build acceptance.

Installed-build visual acceptance is pending. Reload with the staged adapter, use a paused wide replay view, compare 90/70/50/30, and restore the initialized crowd. Preserve the seed/history when an exact same-seat comparison is wanted. A fresh game's different seed can change area placement even at an unchanged fullness. No installation or publication is performed by staging.
