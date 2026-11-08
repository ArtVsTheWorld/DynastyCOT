# Update 3 — occupied crowds and live showboating

Candidate 1.94.0, 29 October 2026. Includes 1.93's shared defensive-loss/stop motion route and the subsequently approved refinements. No attendance-formula, departure-policy or replay-selection changes.

## Occupied seats

Actual live mapped fullness multiplied by the complete authored seating layout supplies occupied mapped seats. Original empty positions, arrivals and applied departures enter that count; camera culling and historical replay fullness do not. Fullness and venue-based requirements still decide whether layered atmosphere qualifies. Occupied count sets strength after admission.

The camera retains the approved linear anchors: 0.35x at 6,060 occupied positions and 1.10x at 61,888, clamped at the ends. Neutral crowd motion retains its separate 0.75x factor. Native HFA filtering and cannon motion remain separate.

| Layout | Fullness | Occupied positions | Shake factor |
| --- | ---: | ---: | ---: |
| 10,000 | 100% | 10,000 | 0.403x |
| 20,000 | 50% | 10,000 | 0.403x |
| 60,000 | 50% | 30,000 | 0.672x |
| 60,000 | 85% | 51,000 | 0.954x |
| 60,000 | 100% | 60,000 | 1.075x |

These factors do not imply admission; a half-full venue may still fail its layer requirement.

DGA's added reaction-layer presence replaces its former square-root fullness factor with square-root occupied count relative to 61,888, capped at one. It is not multiplied by density a second time. Reaction-size tiers use occupied counts below 12,000, from 12,000 to below 36,000, and 36,000 or more. The existing requested tier remains an upper bound. One-shot dispatch retains the requested sound family while selecting its authored size variant; native equivalent voices, bounded followups and the two-voice budget remain respected. Negative sounds do not acquire a positive followup. Native excitement peaks, sparse background quieting and volume caps keep their existing owners.

Status reports occupied reaction crowd, audio tier and the live occupied-seat camera factor. Full-game trace CSVs now include occupied positions and the time-varying camera factor; the summary's existing size_factor column remains its full-capacity reference.

## Showboating

Read-only research verified the carrier's FootballPlayerEntity identity and its animation-audio tag slots. The specific EnumerationValueAsset name `PChat_Loco_Celebration` with value 2 supplies a live running-celebration start event. The reader checks the native type-getter signature, control/player/tag/value identities, packed actor key versus team/slot, held-ball states, exact enumeration name, and stable carrier/tag reads. It does not call a game function or install a new hook.

Tags are cached: a remembered celebration label can remain through later scenes and plays. The model requires a fresh tag arriving during continuous verified live possession. First attachment, reacquisition, actor replacement, old tags and a tag merely moving between channels are baseline observations. Below-gate events are consumed rather than queued. Pause/replay, simulation, stale reads and world changes clear the live lease. Clock plays, kickers and extra-point presentations cannot supply proof; verified receiving-side carriers on punt/kick returns can qualify.

Home showboating admits one slow buildup, rising over 2.5 seconds on a start-event lease bounded to 4.5 seconds, followed by normal camera/sound handoff and settling. It does not infer that the animation remains active for that whole lease. Existing stronger earned reactions keep priority. Away showboating requests the stock GEN_BOO family, using occupied-crowd sizing and existing duplicate/voice limits, without a new positive home shake or positive excitement lift. Both routes require layered atmosphere; home/away follows the designated carrier side.

Two bounded external captures were preserved privately with query/read-only process access. The second capture contains a fresh home running-celebration tag during a live interception return. The human confirmed a visible home showboat. Replaying its sampled carrier/tag data through the new model produced exactly one eligible home event; cached tags, ordinary movement and a later extra-point celebration did not trigger it. The replay assumes admission and does not establish rendered shake or audible output. First-event or identical cached-tag starts without a new observed transition deliberately fail closed.

## Stop and gain rules

- The dedicated goal-line motion requires a fourth-down stand inside the five. Third-down goal-line sound/context remains, but its label cannot independently request camera motion. A separate five-yard loss, confirmed contact stop or Q4 high-leverage condition can still qualify.
- High-leverage stop proof now requires Q4, retaining the existing one-score and final-two-minute limits. Q2 stops retain their ordinary sound/context without that special stop impact.
- Runs, quarterback scrambles and short catch-and-runs start motion buildup at 13 total yards and reach their default target/cheer at 20. The span is seven yards before the configurable gain cutoff. Long air-yard catches and kickoff/punt return thresholds are preserved.
- AudioBalanceVersion 45 upgrades only the former stock 15-yard gain cutoff to 20. Other custom thresholds are preserved; help and migration checks cover repeat launches and future versions.

## Verification and remaining live checks

The staged BUILD-INFO records the completed build, Python, simulation and isolated-host checks. New regression cases cover count invariance, bounded audio targets/families, live animation layout/freshness/ownership, negative dispatch, fourth-down/Q4 restrictions, and gain boundaries. Existing tests whose scenarios relied on the former gain cutoff were extended to reach the approved new boundary; fumble cancellation and fourth-down milestone assertions remain.

First installed-build check: a qualified home showboat before reaching the ordinary long-gain threshold, then an away showboat for boos. Check stronger-effect priority, no cached repeats, sparse-gate rejection, the 13/20-yard gain boundaries, third/fourth-down goal-line comparisons and Q2/Q4 stop comparisons. Observe the occupied crowd/factor through arrivals in one venue. Requests and dispatch logs do not prove that sound was heard or shake rendered. No installation or publication was performed.
