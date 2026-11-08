# Update 3 candidate 2.0.7: overtime coverage and 2020 Mode

## Reported overtime stop

LSU–Clemson Play 27 was a confirmed third-down defensive stop at 88% fullness in overtime. The diagnostic log did not contain fresh knockout/deflection proof. The motion selector previously chose only a fourth-down goal-line stop, measured five-yard loss, Q4 high-leverage stop, fresh contact-flavored stop or turnover on downs. A generic third-down OT stop fell through. The new fallback requests one generic stop impact without claiming a knockout was verified. Regulation behavior remains unchanged.

The old blanket `resumed` exclusion also suppressed fresh defensive results throughout a resumed session. The shared selector now requires a newly accepted tracked result rather than rejecting every resumed game. Raw resume snapshots, replay, sim, gaps, wrong sequence/owner, scores, clock plays and previously delivered motion remain excluded.

## Overtime behavior

- Confirmed OT two-point successes/stops and made/missed field goals have explicit motion profiles. Special results retain once-per-play ownership. Made tries/field goals use the conversion profile; stops/missed opponent field goals use the stop profile. Existing larger scoring/eruption cues keep priority.
- Existing two-point energy events now reach the positive-result audio route. Independently verified OT stops/tries/field goals bypass the ordinary four-second cheer cooldown and can sound over a previous eruption; duplicate samples still cannot resend a result. Home failed tries receive disappointed audio. Visitor positives do not receive a home-celebration shake. Neutral sites support either side.
- Eligible crowd-shake settings receive 1.10x once, retaining occupied-seat scaling, neutral 0.75x and existing bounds. Cannon and native HFA use separate policies.
- A low-priority, sound-only OT floor bridges kick/try/play-selection gaps. Earned result/return/eruption intents retain priority. It introduces no fake cheer, score, new volume premium or relaxed absolute density floor.

## 2020 Mode

The Attendance toggle defaults to off. It requests zero visible crowd independently of the normal Attendance/Soundscape switches, across game modes using the verified shared native stadium layout. Normal formulas remain Dynasty-only. A scene/match identity check precedes the override; no online-mode predicate was added.

The normal peak, arrival/departure state, native empty seats and replay history remain intact underneath. The effective render target is empty, including native renderer replacements and recorded replays. Disabling the mode reconstructs the exact current normal/replay seating target, avoiding monotonic-departure retention of forced empty masks. Stable empty scenes do not trigger repeated full stadium passes. Pause-menu fullness shows zero. The native reference attendance number is intentionally preserved rather than written to zero.

Read-only native mixer enumeration verified Crowd group 9 / parent 2, BandAway 74 / parent 5 and BandHome 75 / parent 5. Three independent owned native presets apply explicit mute with −60 dB gain. Exact names and parents must match. Commentary, field audio, BroadcastMusic, PA and other music groups are untouched. Turning the mode off/context teardown retires these layers through their owning callback; failed retirement can retry. MMC's restore/stop protocol also releases the independent override. No user volume slider is changed. Native cannon/other stadium effects are outside the crowd/band groups.

The native audio callback remains installed even when Soundscape starts disabled, enabling live 2020 Mode and Soundscape toggles. Normal inactive paths retire their additions. Settings help revision 49 preserves values; menu controls are Attendance 11 / Soundscape 12 / Broadcast 13, all below 15. No settings-reset marker or save schema was changed.

## Verification and remaining live checks

Regression coverage includes real tracker-to-motion/audio OT results, the supplied stop without specialized proof, ownership/neutral cases, fresh resumed results, cooldown/eruption overlap, layered visitor-score disappointment and noncompounding motion. Native-shaped production visibility tests cover zero masks, replay and renderer replacement, stable-pass suppression, disabled normal settings and exact current-target restoration. Mixer tests cover independent groups, wrong-name rejection, stable handles, manager replacement and retry after retirement fault. The full automatic native/managed suite passed, including 497 configuration-reader cases and 828 full-game attendance simulations; all 8 Python checks passed. Following the small final stop/restore-export cleanup, the native DLL and production visibility/configuration/menu-read tests were rebuilt and rerun. The final isolated MMC adapter must pass before staging; exact logs, results and hashes belong in the staged BUILD-INFO.

These tests establish model decisions and native-shaped integration. They do not prove audible output, rendered empty stands, frame rate or live OT timing. Install the staged adapter with game/MMC closed, then test:

1. With 2020 Mode off, compare a short normal sequence for clean sound and no whistle hitch. Keep Additional Replays on; allow a TD full-screen/inset sequence.
2. At qualifying OT fullness, stop an opponent on third down without relying on a knockout flag. Expect one stop impact/cheer. Then check a missed opponent FG and successful/failed two-point attempts, including neutral-side symmetry and no positive home shake for an away success.
3. At a normal game, toggle 2020 Mode on: empty stands, zero fullness readout, silent crowd and both bands, clean commentary/field sounds. Run a play and watch a replay. Toggle off: the current crowd and sound return, without treating zero attendance as permanent departures.
4. Repeat the override in Play Now and a postseason/neutral game if readily available. Practice already has its own native crowd policy. Unknown native layouts should fail closed rather than muting unrelated groups or altering saved attendance.

The live-verified 2.0.5 whistle-hitch fix and 2.0.6 hot-path optimizations are retained. This candidate does not install or publish itself.
