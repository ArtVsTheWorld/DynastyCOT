# Update 3 candidate 2.0.5: Additional Replays whistle hitch

## Evidence

The 2.0.4 live test did not clear the hitch. It continued immediately after the whistle after arrivals had completed. Across later plays, crowd-apply and forced-native-pass counts stayed unchanged; the replay-crowd maximum stayed near 1 ms. One reported no-hitch pass around Q1 3:27 is preserved as a comparison, but its exact summary identity is unverified.

The user then restarted the game for each side of an A/B: Additional Replays OFF produced zero hitches, while ON hitched after almost every play. The rest of DGA remained enabled. This isolates the enabled replay feature much more strongly than the earlier full-plugin comparison. It does not yet isolate a single instruction or distinguish all adapter work from the native presentation work it requests.

## Changes

The logic-evaluation hook previously sampled the complete play/mode state on every outer presentation expression, including expressions unrelated to replay selection. It also classified major selector shapes twice and repeated world eligibility checks. Selector and expression-field reads used ReadProcessMemory on this process for many small reads.

Sample a play synchronously only for a fully recognized major replay selector. Normal presentation/replay callbacks continue observing the complete play lifecycle. Classify the major selector once and check eligibility once per root. Unrelated replay event nodes forward without sampling play state. Disabled, restored or failed replay additions forward directly to the original evaluator/event functions.

For the replay evaluator's small same-process context/expression reads, use a bounded SEH-protected copy instead of a system call per field. Reads remain limited to 256 bytes, reject invalid addresses/access faults, and retain exact native type/shape checks. There is no cross-evaluation graph cache. Boolean results, yardage, challenge state and graph mutations are read fresh; no actor pointers survive an evaluation.

Keep existing replay rules: full-screen cutoffs, defensive routing, touchdown/takeaway sequencing, manual play versus Slow Sim, player skipping and injury/native presentation priority. The 2.0.4 equal-target crowd optimization remains. Attendance, audio and shake policies are unchanged.

## Measurement and checks

Add replay selector/routing self-work timing that excludes each forwarded native Eval/GetValue or event/node update. Accumulate self-work between presentation/replay callbacks so many small checks cannot hide behind a low individual maximum. These intervals are not a guaranteed render frame or FPS measurement. Native root Eval, replay event and playback-node timing include nested DGA hooks and must not be described as isolated EA cost. The major-play observer includes its native play/mode queries.

Production-wrapper tests exercise disabled pass-through with invalid node identities, hundreds of unrelated roots without synchronous play sampling, a recognized major root, immediate recognition of a mutated graph, native pause/replay/phase/config guards, fresh nested inset variants and scope cleanup. Local-copy tests include inaccessible pages and a read spanning an inaccessible boundary. Existing expression, major-sequence, mode, attendance, audio and camera regressions remain required. A fake-clock test proves native-call exclusion without sleep-based timing assertions.

The full automatic native/managed regression suite, 490 native configuration-reader cases, 828 full-game attendance simulations and eight Python checks passed. After the final routing timers were added, the final native payload was rebuilt and the production-wrapper/scene suite rerun successfully. The isolated MMC host verified that payload, metadata, resource extraction and storage.

A private expression fixture ran 30,000 classifications with identical semantic results using the two read paths. Across three runs, ReadProcessMemory took 166.9-172.0 ms and bounded local copies took 2.6-2.8 ms. This measures the expression-reading mechanism only, with private native-shaped objects. It is not a game stall, frame-rate measurement or prediction of the live speedup; actual expressions per post-play selection and native engine work still require the live counters.

## Live acceptance remains pending

With game/MMC closed, install the staged adapter and re-enable Additional Replays. In a comparable game, play five to ten ordinary plays; include short runs and incompletions, not only highlights. Report whistle hitching and let several inset replays finish. Then verify a touchdown or takeaway still gets its expected presentation when native flow permits it. If hitching remains, preserve the new selector/observer/native timing counters before choosing another change.

The enabled-path inefficiency is established in source and covered by regression tests. Its removal is a candidate fix, not live proof of hitch clearance. Additional Replays OFF remains the verified workaround. No installed game/MMC files, INI or save were changed by the agent.

## Subsequent live result

The user installed 2.0.5 with Additional Replays enabled and reported zero hitches during the initial approximately ten plays and after passing arrivals completion. The user then scored a touchdown, saw the full-screen replay followed by the separate inset, and intentionally exited the test game. Play16's native stages corroborate that sequence; arrivals completion is also logged. The final capture contains43 play summaries and no fault entries. Selector-work intervals peaked at1.309ms with none reaching4ms; attendance/crowd-apply >=16ms counts remained at the pre-kickoff baseline. The focused reproduction is passed. This does not certify all games or FPS; evidence is preserved privately under Build/LiveWatch/BCCincinnati-Hitches-20261030/Retest205.
