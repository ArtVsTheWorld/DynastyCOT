# Update 3: remaining-path performance audit and candidate 2.0.6

## Result and scope

No additional issue comparable to the reproduced Additional Replays whistle hitch was established in this audit. Candidate 2.0.5 passed the user's BC-Cincinnati retest through arrivals completion with no reported hitches; the user also verified a touchdown's full-screen then separate inset. Preserved counters recorded selector-work intervals up to 1.309 ms, no selector intervals reaching 4 ms, and no new attendance/crowd-apply >=16 ms hits after the kickoff baseline. This is evidence for that reproduction, not a whole-game FPS guarantee.

Review covered the native callback entry points and their attendance, reaction, audio, camera, skill, replay, settings and history helpers, plus the worker and MMC launch/storage code. Candidate 2.0.6 makes three small hot-path cleanups and adds timing to the previously unmeasured sound/camera callbacks. It retains the verified replay fix. No attendance formula, qualification rule, sound gain, shake envelope, polling cadence or presentation threshold was retuned.

## Changes

1. The native intensity callback sampled audio ownership/density, then wantedSwell sampled it again immediately. Pass the first snapshot into the intent helper. It lives only within this callback. Audio-disabled/restored contexts also skip unnecessary native-world reads while retaining their match/epoch and density information for cleanup. Active contexts still verify current native match/world ownership.
2. The camera output callback called prepare, then tick called prepare a second time. A sound-ready output now validates once inside tick immediately before applying motion. A sound-not-ready output still calls prepare to register a valid request and start the existing sound/motion handshake. Pause, replay, accessibility, world/view, freshness and output-only-write checks remain.
3. The legacy sound callback checked the world before discovering that an event was not a sound-start event. Reject unrelated parameter/stop events first. Actual cannon starts still pass the existing named-patch, native playback, ownership, audibility and context checks.

The normal audio-mix callback deliberately obtains its own fresh mix context after native swell dispatches. It does not share a cached world across those native calls. No game-object pointer or live ownership result is cached across callbacks.

## Audit decisions

| Path | Finding and decision |
| --- | --- |
| Replay selection/routing | The 2.0.5 fix removed unrelated play sampling and per-field system calls; major sampling now follows recognized selectors. Retained. No new routing rules. |
| Sound callbacks | Removed the immediate duplicate intensity-context read. Eight bus levels already use one native preset, cached verified templates, unchanged-key suppression and fade throttling. Stable mixes do not repeatedly recreate their presets. |
| Camera output | Removed duplicate validation without bypassing the sound lease. Keep current borrowed-view/projection/world checks and native accessibility handling. |
| Cannon observation | Added the early event-type rejection. No new effects or broader cannon eligibility. |
| Live plays, skills, showboating and defensive proof | Reads/lookup chains are bounded. Actor ownership is resolved for fresh events. Keep the existing 16/50 ms atmosphere cadence and identity rechecks; slowing these would trade away responsiveness/correctness. |
| Native fan visibility | Layout/mask access is already direct and guarded; baseline/position reads are bulk operations. Ordering is cached per renderer. Keep per-part ownership and locking: native streaming/replacement makes a broad cache or unlocked shortcut risky. |
| Seat ordering | Sorting/layout generation happens on initialization or replacement, rather than every play. The large setup maximum was present before kickoff and did not recur as a post-play stall in the retest. Leave the approved pattern and reconstruction rules intact. |
| Attendance math/context | Heavier decisions are throttled and applied at safe windows. Cohort/support setup is a fresh-launch operation, not a repeated per-fan calculation. Keep short transition observations before the heavier decision gate. |
| Replay/resume history | Normal recording probes are throttled and equal-density points deduplicated; replay lookup uses binary search. Archive size is bounded. The worker copies a changed revision under the lock, then serializes/flushes outside it. No disk I/O inside the timeline lock. Worst-case long-session lock contention has not been measured live. |
| Logging | Gameplay callbacks enqueue bounded messages; the worker drains them. The live retest had no queue drops. No per-play synchronous file flush was found on these callbacks. |
| Settings/menu | Worker checks timestamps/size and avoids reparsing unchanged settings every second. Native menu choices are loaded for the opened menu; user edits write settings. No menu file enumeration or INI parsing on the whistle path. |
| Entity discovery | Heap discovery is a worker fallback on a five-second check when controllers need refreshing, not an unconditional scan after plays. Native controller callbacks provide the normal discovery path. Leave the recovery fallback in place. |
| MMC launcher/storage | Runs outside the game; once-per-process load claims, readiness/error backoff and one-second polling prevent repeated load attempts. No launch/migration loop runs on the game render thread. |

Long-lived immutable native mixer presets must outlive native references/fades. This audit does not introduce speculative eviction. Similarly, a future asynchronous seating rebuild or broader native-read conversion needs its own lifetime and streaming tests; neither is required by the present hitch evidence.

## Private benchmark

The same private fixtures compare the 2.0.5 source with the candidate. The original intensity callback is stubbed; the camera uses a fixed verified private world and output buffer. Both keep identical expected context and matrix results. Values below are totals across repeated calls, not frame times or FPS:

| Fixture | 2.0.5, three-run range | Candidate, three-run range |
| --- | --- | --- |
| 10,000 intensity callback calls | 70.6-74.3 ms | 47.3-56.1 ms |
| 2,000 active camera output calls | 36.0-40.0 ms | 20.5-28.0 ms |
| 10,000 audio-disabled context requests | 11.7-14.1 ms | 0.13-0.22 ms |

The first two costs are only several microseconds per callback. These changes reduce steady overhead; they are not another demonstrated hitch fix or a claimed FPS increase. Private sources and logs are under Build/Audit206, excluded from the repository.

## Verification and release check

Production-wrapper tests cover one context sample per intensity update, exact density outputs, disabled context cleanup identity without native-world sampling, stale match/epoch rejection, camera request priming before audio is ready, sound-ready motion, immutable source view, and pause/accessibility rejection. Existing replay, native sound, audio mix, camera, streaming crowd, Midgame Save and attendance simulation checks remain required.

Completed checks: full automatic native/managed regression suite, 490 native configuration-reader cases, 828 production-policy full-game simulations, eight Python catalog/compatibility checks and the isolated MMC ABI/metadata/resource/extraction/storage check all passed. The targeted production-wrapper tests and before/after callback fixtures also passed. The candidate has not been installed or published by the agent.

New status counters measure DGA work after the original mixer/intensity/cheer/view callbacks and the native HFA/cannon observers. Native helper queries/dispatches within DGA work are included; the forwarded original update is excluded. These are callback wall-time maxima, not isolated CPU/FPS measurements, and no timer was added per spectator.

Before using 2.0.6 as the final release candidate, run a short smoke test: ordinary plays with Additional Replays enabled, one audible earned reaction with its shake, a snap fade, a pause/replay return and a sparse crowd mix if convenient. Check the new sound/camera counters. The broader 2.0.5 whistle-hitch result remains preserved; do not infer that the new candidate was live-tested from its automated checks.
