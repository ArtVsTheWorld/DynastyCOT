# Reaction audio catalogue — 2.0.8

The October 30 listening tests covered size selectors 1–3 in 28 native families: **84 tested, 80 audible, 4 excluded**. Size 0 was not auditioned and is not dispatched by this integration. Different selectors can share recordings; this is not a count of unique audio assets. The evidence is the user’s listening reports, not native queue/voice registration.

`Source/ReactionCatalog.h` is the runtime allowlist; [the JSON catalogue](REACTION-AUDIO-CATALOG.json) records the same tested IDs and their intended roles. The raw private audition records remain under `Build/FamilyAudioAudition-20261030` and `Build/NegativeAudioAudition-20261030`.

| Family | Audible selectors | Excluded | Existing use |
|---|---|---|---|
| Ballcarrier action + | 1, 2, 3 | — | Verified skill moves |
| Ballcarrier action – | 5, 6, 7 | — | Away skill counterpart |
| Big hit + | 9, 10, 11 | — | Truck / hit-stick; failed negative big-hit family is not used |
| Big hit – | None | 13, 14, 15 | Excluded: not audible |
| Catch + | 17, 18, 19 | — | Verified long catch/completion |
| Catch – | 21, 22 | 23 | Deep visitor catch / home missed catch |
| Field goal + | 25, 26, 27 | — | Existing made / opponent missed kick |
| Field goal – | 29, 30, 31 | — | Existing home missed / away made kick |
| Fumble/takeaway + | 33, 34, 35 | — | Forced fumble, defensive recovery; takeaway variety |
| Fumble/takeaway – | 37, 38, 39 | — | Home lost fumble / away defensive recovery |
| Incompletion + | 41, 42, 43 | — | Breakup / defensive stop |
| Incompletion – | 45, 46, 47 | — | Failed conversion / away defensive stop |
| Interception + | 49, 50, 51 | — | Interception; alternates with takeaway family |
| Interception – | 53, 54, 55 | — | Home interception |
| Sack + | 57, 58, 59 | — | Verified sack |
| Sack – | 61, 62, 63 | — | Away defensive sack |
| Receiving touchdown + | 65, 66, 67 | — | Verified passing touchdown |
| Receiving touchdown – | 69, 70, 71 | — | Away passing touchdown |
| Rushing touchdown + | 73, 74, 75 | — | Verified rushing touchdown |
| Rushing touchdown – | 77, 78, 79 | — | Away rushing touchdown |
| Yardage gain + | 81, 82, 83 | — | Run, scramble, catch-and-run and return gain |
| Yardage gain – | 85, 86, 87 | — | Away gain counterpart |
| Generic cheer | 89, 90, 91 | — | Existing result and lineup variety |
| Disappointment / aww | 93, 94, 95 | — | Existing home letdowns / fallback |
| Surprise / oh-ooh | 97, 98, 99 | — | Existing confirmed breakup / knockout variety; neutral emotional character |
| Boo | 101, 102, 103 | — | Existing away live showboat |
| Applause-labelled cheer | 105, 106, 107 | — | Existing generic/result/lineup variety; heard as cheers |
| Anticipation | 109, 110, 111 | — | Existing special lineup-cheer opportunity |

## Selection and strength

Family choice follows the verified event. Stable play/event/epoch identity chooses variety only among compatible families: for example, interception/takeaway, generic cheers, or confirmed breakup/surprise. A catch does not randomly become a sack reaction. Negative big-hit sounds fall back to audible disappointment; negative catch’s strongest requested selector falls back to 22.

The family’s ascending IDs represent native intensity/duration variants. The requested event tier is clamped by both occupied seats and fullness:

| Occupied mapped seats | Population ceiling |
|---|---|
| Under 12,000 |1|
| 12,000–35,999|2|
| 36,000 or more|3|

Below 30% fullness the ceiling is 1; from 30% to below 60% it is 2. Ordinary events normally request tier 2. Scores/takeaways, 20+ yard gains/catches and critical situations can request tier 3, subject to those crowd ceilings. A routine skill’s tier 3 needs a critical situation; a small crowd cannot become a stadium-sized reaction solely because the play is important. Actual selector levels are discrete native choices; the existing continuous native-bus gain curve is unchanged.

At neutral sites, both teams retain positive support. Stadium-wide home disappointment/boos are suppressed. Crowd-size selection uses the measured whole crowd; no unmeasured fan-affiliation fraction is assumed. The existing neutral shake multiplier remains 75%.

This pass creates no new play detectors or shake triggers. Away equivalents reuse existing verified samples and supply one sound-only counterpart, with a separate score upgrade when appropriate. They do not feed energy, crowd departures, motion or layered admission. Existing setting gates, pause/replay/sim rejection, freshness and once-per-event timing remain. DGA events may still play over native cheers; bounded queue recovery remains, without the old overlapping-cheer suppression.

One optional existing follow-up layer remains the validated touchdown cheer family. It is not a new trigger or an independently timed barrage. No background, chant, commentary, field, band or user-slider gain was retuned by this integration.
