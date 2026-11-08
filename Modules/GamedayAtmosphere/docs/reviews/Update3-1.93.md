# Update 3 — shared defensive motion

Candidate 1.93.0, 29 October 2026. Implements the approved motion-trigger simplification. Attendance tuning, replay selection, motion strength/attack profiles and atmosphere qualification are unchanged.

## Rules

- A verified loss of at least five yards supplies the standalone defensive-loss impact, whether a sack or run. Exactly five qualifies (the existing 0.01-yard measurement tolerance remains). Native sack truth no longer participates in this camera decision.
- A short sack or sack with unavailable yardage retains sound and replay classification. It can still shake through a separately qualified goal-line, high-leverage or contact-confirmed critical-down stop.
- Third/fourth-down stops use one completed-result decision. Knockout, breakup and hit-stick evidence supplies contact flavor; it cannot start motion before the actual stop. Goal-line and high-leverage profiles retain priority. A routine third-down incompletion does not become a new motion trigger.
- One defensive loss/stop request owns the play. Late flavor can join a confirmed result within its original five-second window, without refreshing that window. It cannot restart an already delivered impact.
- Interceptions, forced fumbles, blocked kicks and scores keep their separate ownership/priority. Home-defense and neutral-site rules, fullness gates, pause/replay/sim cleanup, clock-play exclusions and venue scaling remain.

Audio still uses native/official classifications. The camera receives a separate request ID, preventing a delayed sack label or another audio event from bypassing the common motion decision. Normal runtime logs add `[defensive-motion]` with the selected event and play number. This records a request, not proof that the player saw or heard it.

## Verification

`DefensiveMotionTests` covers 480 loss/sack/down/owner/neutral/phase combinations, tracker-to-camera integration, exact-five boundaries, short and unknown-yardage sacks, early/late contact, accepted-penalty first downs, score correction, prior impact ownership, special teams, knees/spikes, gate removal, interruptions, resume and duplicate suppression. Existing sack-carryover, defensive-accent/loss, takeaway, scoring handoff and final-play tests remain in the full build. Full build and isolated MMC host results are recorded in the staged BUILD-INFO file.

The full runtime fixture exposed a pre-existing fixed-clock assumption: its Q2 check could occur after a seeded halftime break had already begun. The fixture now checks just before the actual per-game departure boundary, preserving its permanent-departure assertion. Twenty repeated runtime-fixture runs passed after this correction; attendance code was not changed.

## Live acceptance

Compare a five-plus-yard sack and run loss in a qualified stadium; both should get the same loss impact without waiting for the sack label. Compare a short sack, a third/fourth-down breakup or hit-stick stop, an ordinary third-down incompletion and a turnover on downs. Repeat a representative event below the layer gate and across a pause/replay. Confirm that larger forced-fumble/scoring reactions still win. Do not infer a missed visible effect solely from absent logs.

This removes dependence on the later sack predicate; it does not claim an earlier authoritative result or measured in-game latency improvement. No additional native calls or hooks were introduced.
