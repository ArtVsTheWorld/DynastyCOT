# Historical Update 3 validation notes through 1.97

Historical evidence only. Build numbers, installation steps and pending statuses below describe their original checkpoints. Use [the current checklist](../DIAGNOSTICS.md) for testing.

# Update 3 testing and diagnostics

Working candidate **1.97.0**. Update 2 (1.54.0) remains the public release. This is the current checklist; earlier development notes describe older builds and are not release sign-off.

Candidate 1.97's seat-desirability model passed the live old/candidate preview and was accepted by the user. Exact counts and 90% selection were checked on the current layout/seed; original rank data, 60% crowd, replay controls and unchanged history were verified after restoration. The packaged DLL still needs its normal installed smoke check through arrivals, historical replay and resume/streamed views. Compare preferences within the same match seed for exact seats. See [1.97 review](Update3-1.97.md).

Candidate 1.95 first needs a qualified home defensive scoop-and-return compared with an interception return. Check the force-to-return blend, whistle fade and touchdown handoff. Recover your own fumble: no new return shake or revived gain buildup should start. Repeat a defensive return below the layered gate, then at a neutral venue. A prior force impact may finish independently of an own pickup. See [1.95 scope and regression coverage](Update3-1.95.md).

Candidate 1.94 first needs a qualified home-carrier showboat below the long-gain cutoff, then an away-carrier showboat for GEN_BOO. Check slow buildup, stronger-effect priority, no stale repeats and no reaction below the layered gate. Compare occupied-seat counts through arrivals in one venue, the 13/20-yard gain boundaries, a third/fourth-down goal-line stop and a Q2/Q4 high-leverage stop. Status reports occupied crowd and audio tier; request logs are not rendered/audible proof. See [1.94 evidence and scope](Update3-1.94.md).

Candidate 1.93 needs a qualified home-defense or neutral-site comparison: a five-plus-yard sack, five-plus-yard run loss, short sack, and an important third/fourth-down stop with contact flavor. Verify one prompt impact after the confirmed loss/stop, no standalone impact for a short sack, and no new impact on an ordinary third-down incompletion. Repeat below the layer gate and through a pause/replay interruption. The new `[defensive-motion]` entry records the shared request, not rendered-shake proof. Sound/replay sack classification stays independent. See [1.93 scope and regression coverage](Update3-1.93.md).

Candidate 1.92 needs a close ordinary game through halftime/Q3: verify fewer permanent exits, temporary crowd return in two waves and stable timing after a matching resume. Different games can have different schedules; repeated callbacks in one game cannot reroll them. The existing sparse-crowd, motion, final-play carry and replay checks below remain. See [1.92 comparisons and supplied-save limitations](Update3-1.92.md). The original save was read only through a private copy; no season advance or save write was performed.

Candidate 1.91 adds catalog-sized crowd motion (0.35x at 6,060; 1.10x at 61,888 authored positions) and bounded earned final-play carry into postgame. Compare small/large qualified venues; inspect `Crowd camera venue factor`; verify no new DGA event motion below the existing layer gate. Check a quick snap early in a lineup buildup, an ordinary earned final-play effect, an eruption, and immediate entry to the final menu. Native cannon/HFA motion remains separate. See the [code review and simulation coverage](Update3-1.91.md) for fixed traffic integration, custom-quarter recovery, sound continuity and native-boundary safeguards. Regression/simulation success is not live sound/camera acceptance.

Live1.89 Q1 testing confirmed a false sack label on an ordinary minus-two-yard run after the user-reported strip-sack/fumble. The normal log records Play2, away first down, selected type206, minus2 yards, sack confirmed and a sack-reaction request. The preceding home play has unavailable yardage and a takeaway route; its exact predicate timeline was not captured, so lingering native truth is a supported code-path explanation rather than a measured native reset trace. Native disassembly verifies that SackPredicate selects its DB play through tracker+CE0 and vtable+68h; the live guarded getter is mov eax,[rcx+10h]; ret. A read-only live snapshot after the reported sequence confirmed that layout and selected index2; it does not establish the incident-time index. Candidate1.90 tracks this independent identity, rejecting a carried predicate still tied to the old selected play, including after a temporary false value. A newly selected completed play admits genuine consecutive sacks. Guarded pointer/vtable/getter/index reads fail closed; native calls remain limited to the original presentation phases and cache also refreshes on selector changes. One hundred twenty-two regression cases cover strip/ordinary sacks, owner changes, changed hashes, phases, minus2/minus6 runs, unavailable resets, false resets on the same old selector and valid consecutive sacks. The production selector adapter has positive/bounds/read/signature/vtable/getter checks. Retest a strip-sack followed by an ordinary losing run, then genuine consecutive sacks; preserve `[play-summary]` reasons. A minus2 run must not produce a sack reaction; a measured five-yard loss can still earn its existing defensive reaction.

Candidate 1.80 closes a reproduced sparse-camera gap: a new live camera's first native HFA event previously passed through until the next worker update discovered the camera. First-event interception now learns the verified event IDs inside the borrowed native callback and preserves the requested native tier for later restoration, including when the channel row is created afterward. Compatible non-HFA broadcast/replay views no longer stop filtering supported live views. Regression checks cover these transitions, stale leases, ownership changes, unrelated channels and balanced references. The fix has not yet been verified in game.

Added DGA event motion still requires layered admission. Native HFA camera motion separately remains allowed at or above `NativeShakeMinimumFullnessPercent` (default 50% actual mapped fullness), even when layered admission fails. Cannon impacts remain an intentional independent exception. An integration regression verifies that a 60%-full small venue below its layered requirement cannot emit a fresh added TD/live/eruption cue, cannot replay a blocked event later, and immediately cancels an existing cue when admission is removed. These checks do not establish the cause of a particular visual shake without matching camera evidence.

Candidate 1.72 also sharpens forced-fumble onset without changing strength or detection requirements. A live feel check is pending.

Candidate 1.71 adds softer pre-punt buildup (half strength) and home-offense critical-down buildup (15% softer). These changes need an in-game feel check after installation.

Candidate 1.75 uses 75% neutral crowd strength for both teams, makes successful spins/jukes cheer-only, and pairs confirmed turnover-on-downs stops with small motion and a cheer. Earned live motion rises into TD/eruption strength over 300 ms without a fresh impact. It also retries a late deflection actor-team lookup, accepts continuously tracked knockout proof up to five seconds after the whistle, and records missing defensive proof in the normal log.

Candidate 1.77 verifies native IsInRealtimeSim and IsPlayCallOpen queries. Manual play-call capability is remembered only within the same match/session and cleared on Slow Sim, a missing query or a new game. Slow Sim and unknown UI retain original inset events, without added inset requests or missing-inset timeouts. Full-screen promotion remains independent of inset availability. Native replay-node activity blocks the timeout fallback, and an already-running native inset is acknowledged without dispatching another. Mode changes appear once in the normal log. Final live sequence verification is pending.

Major TD/takeaway presentation in verified manual play has a guarded candidate route: decline the replay-less quick TD shortcut only for a freshly confirmed major play, hold the authored inset request until the full native clip sequence finishes, then request a separate inset repeat. Native recording availability, sequence ordering and tradition/RTG routes require live verification. This is not a guarantee of native highlights when no recording exists. Kneels/spikes are excluded from DGA routine replay promotion; native postgame GreatPlayFinder fallback remains unresolved.

Cheer logs distinguish queued requests from registered native voices. A dropped request can receive one recovery within 600 ms on the same play/owner, preserving the two-voice budget; retiring native voices no longer count as active duplicates. Registration is not proof of audible sound.

Candidate 1.76 trims the in-game menu to Attendance (10 controls), Soundscape (12) and Broadcast (13), with toggles first and numeric controls below. Advanced INI values remain supported and are not rewritten by editing this smaller menu. No troubleshooting controls appear in the game menu.

Candidate 1.78 tunes the existing rainy home-win catch-up target only in the last half of Q4: a lead above 21 ramps to full severity at 35+. A non-stacking quarter-cap exception adds at most 10 points (proportionally less for low custom caps), with ranked and postseason floors preserved. Scores change pressure without an immediate deduction. A representative NIU-like Q4 simulation using 72.3% opening seating density moved from ~37% to 26.8% visible fullness. Live acceptance is pending.

The user accepted the settings menu's operation and appearance. Earlier 1.77 unskipped Play 44 (20-yard TD, Q3 3:07) showed neither replay; later 1.79 captures produced four user-confirmed full-screen-then-inset successes, including home scores after Super Sim in Q1 and Q3. The user now accepts this stability for Update 3 and ended blocker investigation. The intermittent failure remains an accepted limitation, not a proven fix. Slow Sim rendered replay confirmation remains pending. No native-camera event trace identified the earlier brief below-gate TD shake, though its DGA cue was inactive and no cannon was logged.

Candidate 1.79 added a bounded TD-selection diagnostic using the existing expression/event/replay-node hooks, without changing routing; 1.80 retains it. Before score confirmation it retains recognized native TD/takeaway/quick-route decisions in a fixed 128-row per-play buffer. Only confirmed TDs flush to normal `[replay-select]` logs. Entries include monotonic tick, opaque node identity, phase, native/final cached Boolean, whether modifications were allowed, armed kind/stage, start/current score, native result freshness and replay activity. Native inset-event interception and replay-node entry/completion are included. This identifies selector families, not the exact executing presentation package or audible/rendered output. New play/match/epoch state clears old evidence. A paired read-only replay-clock capture is needed for recording timelines. Successful captures do not establish an intermittent-routing fix.

## Starting turnout and resume continuity, candidate 1.89

Live 1.83 acceptance caught a fresh non-rivalry launch retaining EA peak turnout. Native 19:30 local time was confirmed by the user, so night classification was correct. Its only apply trigger was inside optional rivalry publication. Candidate 1.84 moves that callback to the shared block after ordinary/rivalry paths join and before native team setup/finalization. The displaced call is relocated without borrowing a register; the callback preserves volatile registers, XMM0-5 and flags. Prefix guards and the original offline/fresh-input/typed-option restrictions remain. An executable fixture checks both entry paths, once-only callback order, correct native arguments/return and stack restoration. This is automated evidence; successful live count override and resulting native seating are still pending.

Live 1.85 confirmed that the remaining blocker was mode14 becoming -1 at the shared callback; every other sampled guard passed. The native input cache was also empty, so the transition affected capture as well as application. Version 1.86 samples the existing mode field every100ms and inside the exact playable formula callback. A witnessed mode14 may bridge only -1 for ten seconds; -1 never renews it. Other modes, unreadable state, expiration and reversed time revoke the witness and captured inputs. Proof generations prevent recycling inputs after a mode switch. The count write still needs exact home/away/stadium/native-count identity, regular-season classification and fresh inputs. As requested, starting turnout no longer duplicates online/gameplay-config checks already handled by MMC. Native input capture now gets its own normal-log entry. Automated checks cover transition expiry, generation/identity isolation, unsupported modes and absent configuration; live count application is still pending.

Live 1.86 verified the transition and captured all138 schools. The remaining failure was the native broadcast capacity query at the shared pre-finalization point. Read-only inspection of the captured seed confirmed the same selected stadium561, EA66349 and capacity75005. Candidate1.87 binds that captured capacity only after exact launch identity checks and enforces1000-200000 capacity bounds; starting turnout has no capacity-RPC dependency. Native input logs include stadium/captured capacity. A pure calculation using that captured fixture yields peak75005 and kickoff64% of peak; this is a model check, not observed seating or count application. Live native count and rendered-density acceptance remain pending.

Live 1.87 verified the same fresh night launch: native count75005, capacity75005, measured kickoff64%, then89% at Q1 8:20 with11% still arriving and no departures. A Q1 Midgame Save reload at8:15 incorrectly chose a loading-created record at80%, then advanced to84%. Original gameplay history ended only3.098 seconds from the restored clock, outside the old2.01-second matching window.

Candidate1.88 excludes provisional loading recordings from resume selection. New history stores an explicit readiness flag (format5); formats1-4 remain readable and require recorded progression between midgame samples before they can supply resumed attendance. A live phase3 sample or a verified restoration establishes a new source. Matching remains tied to team/score/quarter/length/ranks/venue/weather/postseason, also checking week/year when both plans supply them; the bounded clock allowance is5.01 seconds. Equal-clock ties use the latest crowd. A confirmed pause checkpoints its current crowd, and a restored arrival curve cannot catch up while the game clock is unchanged. The preserved failure archive now restores89% in an offline regression. No installed-build success is claimed yet.

Live1.88 did not enter restoration after a process restart: paused Q1 8:15 measured80%,20% still arriving, zero departures and ordinary-game status. The fresh Midgame Save DLL status had matching PID29900, active game0 and applied_game0, with an explicit applied restore at the same clock/score. DGA's parser required positive game numbers. Candidate1.89 accepts0 as a valid generation and gives the tracker a separate initialized flag, retaining one-shot restoration, ordinary-game adoption and foreign/stale-signal rejection. The stored original89% source is still present; the end-to-end regression now checks zero-based recognition before matching that source.

First test: close game and MMC, install1.90, reload the existing Q1 Midgame Save and pause immediately. At Q1 8:15 expect89% full,11% still arriving, zero departures and the75005 peak; status must report restored crowd history, and the resume log should name the original source clock near498.098 seconds. Stay paused briefly to verify no drift. Then play beyond6:00 remaining and reach play selection; arrivals should finish without refilling permanent departures. Do not delete history before this test. A separate daytime/lower-demand fresh launch, other school arrival groups and postseason/native-peak checks remain pending.

Before release, install the staged candidate with both game and MMC closed and start a fresh regular-season Dynasty game. This feature cannot safely retrofit an already generated seating baseline. Check the normal `[starting-attendance]` log and status for EA/revised counts and local kickoff. A missing fresh-input message means EA fallback, not a successful override.

- Match logged My School atmosphere/tradition/brand/conference grades to the Dynasty school UI (A+=0, F=12). Rank replaces championship contender; campus lifestyle is absent. Confirm 138 valid FBS schools on the stock save, excluding FCS/placeholders. Cohort evaluation occurs only during a fresh playable launch.
- Check ordinary daytime 50-60%, night 55-65% of peak, Utah/severe-late baseline 45% day/50% night, weather-only minimum 35%, usual marquee ceiling 70% and exceptional hard cap 80%. Only top-fifth support, at least 95% expected peak and a Top10 pair or Top25 rivalry with night/home-opener incentive can use the exceptional band; severe-late homes cannot. Verify percentage-of-peak versus percentage-of-capacity and Q1 deadlines 50/30/75/20% for normal/marquee/severe/postseason.
- Compare the same day/night fixture: EA count should be identical while DGA adds 4 points before bounds; 19:00 local is the inclusive threshold. A Pacific schedule shown in Eastern time must use its converted local kickoff.
- Compare dry/light/medium/heavy weather at an outdoor venue; verify the count reduction and visible seat baseline, then verify a fully roofed venue does not receive the precipitation deduction.
- Test one increase beyond EA's sparse turnout and one reduction below a native sellout. Capture normal/replay/streamed/postgame views and confirm actual seat density rather than treating the requested count as rendered proof.
- Repeated copies/reloads of the same fixture retain variation; a different game with reused pointers must acquire its own matching inputs. An unknown weather value, stale sample or changed signature keeps EA behavior.
- Verify that regular neutral games use both teams symmetrically and that rivalry remains a bounded bonus. Check a conference championship, bowl/CFP and practice/preseason load for unchanged EA peak turnout, postseason openings 70-80% and championship openings 80-90%.
- Check a Q1 Midgame Save before arrivals finish, including 0-0, then reload and confirm the same peak, kickoff fraction, Q1 deadline and remaining arrivals. Check late attachment, quitting/reloading the same matchup, old history and the turnout switch off; no invented opener or rebuilt native crowd. History format 4 preserves the plan and reads formats 1-3.
- Listen for a modest sound-only welcome in a sparse fresh opening. At high demand (expected peak at least 90%), verify lighter ordinary first-drive buildup from 55% actual density; below 50% there is no crowd-driven shake. Marquee, normal size and accessibility gates otherwise remain. Check a close ranked/Top50 rivalry at halftime, then a blowout: extra patience should apply only while close. General sparse sound/departure tuning is otherwise unchanged.

The shipped tuning record uses Initial 20, Home 60, Away 0.33 and Rivalry 30 (the schema default for Rivalry is 20). Examples are explicit synthetic fixtures using those values, not predictions for a loaded save. The night/time and launch boundary were inspected live read-only; the revised count override and 1.83 cohesion changes are not yet user-verified.

## Confirmed so far

- Candidate 1.71 passed the full native/managed checks, including 511 real native-reader configuration cases, plus 7 Python tests and isolated MMC adapter checks. The Wake sparse-peak regression, below-gate sound route, kneel/spike exclusion, broken-tackle motion and away-takeaway cleanup have automated coverage, along with softer punt/home-offense critical buildup and unscaled result handoffs. Installation/rollback is checked in a disposable fixture; the real upgrade and full-game checks below still need user verification.
- The user verified the basic tab/fullness toggle on 1.61 and accepted the curated menu's operation and appearance on 1.77. Individual Reset/decimal-edit cases were not separately enumerated; do not treat them as independently verified.
- Texas cannon movement was confirmed in game. The 1.67 log records native cannon starts with 15%, 20%, 25% and 30% settings. All nine authored cannon selectors have offline coverage; other stadiums have not been manually verified.
- Airborne home passes were captured; the 1.67 log records new verified airborne passes. An earlier away pass also logged anticipation. Audible timing on both offenses still needs listening on the final build.
- Build 1.67 correctly identified an away defender forcing a home fumble and withheld a positive home impact. A natural home-defense force with its positive sound/motion remains to be checked.
- Truck, hurdle, knockout and deflection signals were observed in game. Capturing a signal or logging a request alone does not establish the final audible/visible effect.
- The native Dynamic Camera Effects setting was verified read-only. The revised default RTG camera projection has automated coverage; its natural eruption strength still needs a visual check.

## Texas/Ole Miss test, October 27

Runtime 1.68 resumed from Midgame Save and finished Ole Miss 45, Texas 37. Attendance restored at 100% without fresh opening arrivals and finished at 89%. Both offenses logged airborne-pass anticipation (19 verified flights in the resumed process). Standard touchdowns, replay restoration, Super Sim/halftime attendance transitions and final sound/camera cleanup were observed in logs. These entries establish requests/state, not perceived output.

The user identified the final logged "defensive sack" as Ole Miss kneeling. This is a false positive, not successful sack coverage. Candidate 1.70 excludes native knee/clock-stop play types from buildup and result impacts, and makes regular broken tackles sound-only. Verified away takeaways also clear previous home-positive motion immediately instead of retaining a short fade. The new sound-only route addresses sparse-game reaction gaps; verify these fixes and the sound-only output live before release. The earlier process crash remains unresolved; no new error was logged during the resumed game. Positive home forced-fumble and pass-breakup output, and other rare-event checks below, remain outstanding.


The Wake/Virginia test exposed a separate native-quieting issue: after an early halftime seat break reduced actual fullness to 48.4%, runtime 1.68 reduced a native 1000 excitement sample to 168.4. Candidate 1.70 uses a fullness/strength blend for the bed ceiling and lets native reaction peaks bypass that ceiling while retaining crowd-presence scaling. The captured 18,829-position fixture is covered in the quieting and borrowed-entity runtime tests. Audible output still needs a live retest.

## Florida/Georgia test, October 28

Runtime 1.70 finished Georgia 42–Florida 23 at a neutral rivalry venue. Attendance started at 65%, reached 100%, and finished at 87% with 13% permanent departures. Both offenses logged airborne anticipation. Florida Play 27's 76-yard TD had the desired whole-play full-screen replay and separate inset repeat, confirmed by the user. Georgia Play 44's 22-yard TD went straight to PAT selection without either replay, also confirmed. Play 75's turnover on downs had cheer dispatch but no motion. The final interception (Play 87, eight seconds left) had visible motion but no audible cheer despite a primary queue dispatch; no native voice acceptance was recorded in 1.70.

The user also saw the final knee repeated in postgame. Three short replay windows appear after the knee. Offline postgame packages request different GreatPlayFinder offsets (2, 1, 0 for three highlights), so this is consistent with a native selection/recording fallback, not proof of why it selected the knee. Candidate 1.75 excludes clock-management plays from DGA routine promotion; native highlight variety remains unverified. The temporary watcher was stopped and evidence preserved under Build/LiveWatch/FloridaGeorgia-20261028.

## Overnight slow-sim test, October 28

Runtime 1.75 completed a 62–31 home win. The external read-only observer captured 73.4 minutes including postgame, 21,726 state samples, 3,737 native cache records and 43 native playback windows. The dedicated DGA worker averaged about 0.49% of one CPU core; this excludes shared game-thread/audio/camera callback time and does not establish total mod overhead or FPS. The observer averaged about 0.95% of one core. Whole-game private memory fluctuated after loading rather than showing a steady increase; it cannot be attributed to DGA.

The user clarified that this was Slow Sim, which has no play-call screen and therefore no inset destination. Missing inset acknowledgements are expected for this mode, not evidence of missing visible replays. The user confirmed pick-six Plays 61 and 90 each played full-screen twice, with mascot/upset-home-crowd cutaways and no player card. The native Cutaway_TurnoverTD_p asset contains two replay choices, consistent with that pattern; the executing package was not captured. Both sixty-second lifecycle timeouts are real logged controller observations, but the controller must check inset availability before expecting or requesting one. Manual-game full-screen-then-separate-inset ordering still needs verification. The user also confirmed Play 98 touchdown shake was fine; recording ended before its replay, so that replay is ungraded. That capture predates the 1.77 mode guard; it remains evidence of the older controller behavior.

All 42 queued live reactions and 9 queued play-result cheers logged native registration; no listener was present to establish audibility. At 49% mapped fullness, a sampled native 1000 excitement peak reached 979.5 output, supporting preservation of peaks above the calmer background ceiling. Final crowd fullness was 45%, giving additional sparse-crowd coverage in a dry home blowout. The home-loss beat-the-traffic trigger was not exercised. Private evidence and separate verified facts/hypotheses are preserved in Build/Overnight-20261028/findings.json, with improvement avenues for typed gameplay events and Update 4 presentation diagnostics. The capture stopped after its planned five-minute postgame window.

## Before the two test games

1. Close MMC and the game. Copy the candidate plugin DLL into MMC's Plugins folder, then restart both.
2. Confirm the launch/runtime log identifies 1.80.0 and activation succeeds. A startup/version line alone is not activation proof.
3. On first Update 3 startup, verify the old INI is backed up and the new file has current defaults. Edit one setting, restart once more, and confirm it stays changed without another reset.
4. Use the normal log setting, Debug=0. Cannon entries should now appear without technical logging.

## Full-game test plan

Use one ranked game in a cannon stadium and one unranked home team hosting a Top 25 visitor. Play the full games normally; record game quarter/clock and result when a reaction is missing or surprising. Rare plays are opportunities, not requirements to manufacture during every game.

| Check | Expected result |
| --- | --- |
| Sparse native peaks | Around 48-52% fullness, native big-play excitement can rise above the calmer background ceiling. No abrupt peak collapse at 50%; empty-crowd scaling, six-second background blend and volume caps remain. |
| Sound-only route | Below the layered gate, confirmed gains/defensive plays/scores get one native reaction; skills and airborne passes can get softer swells. No added crowd shake, passive roar, follow-up cheer, eruption or extra volume. Native HFA shake has its separate fullness cutoff; cannon impacts are independent. Native disappointment applies to home mistakes and away scores. |
| Cannon | One numbered [cannon] entry per admitted native blast, reporting variant and configured strength. Native sound is not duplicated. A request is not proof of visible shake. |
| Pass anticipation | Both offenses' eligible deep throws swell while airborne, then hand off to the catch, breakup or incompletion. No pre-catch added cheer/shake or extra volume bonus. |
| Run/return to score | Live excitement flows into a touchdown/eruption without a quiet dip, duplicate initial hit or restarted lower reaction. |
| Kickoff/punt returns | Swell/movement builds from 30 yards to the existing peak at 40. A pickup after a fumble does not restart gain shake. |
| Skills | Regular broken tackles use sound only, without starting or restarting shake. Successful hurdles use a cheer; trucks use the big-hit crowd family. Spins and jukes use a cheer without shake or an extra skill swell. The other small skill buildup is gentle and does not repeat on one collision. |
| Home defense | Confirmed third/fourth-down knockouts and pass breakups get the small paired reaction. A routine incompletion alone does not qualify. Hit-stick motion requires a completed third/fourth-down stop; other-down hits keep sound only. |
| Forced fumble | A verified home-defense force can react once on any down. A verified live defensive return uses interception-style motion under the layered gate; a result-only or own-team recovery adds no new impact. An away-defense force/return at a home venue gets no positive home motion. |
| Touchdowns/eruption | Routine touchdowns stay routine. The opening TD exception covers only the first two tracked plays, including kickoff. A 60+ yard close-game TD gets a modest TD accent, not an eruption from distance alone. A genuinely sealing/winning event can still take priority. |
| Upset Watch | A credible home bid against a Top 25 visitor can gain sound from Q2 at its separate fullness gate, never below 50%. It does not grant packed-house status or raise volume caps. |
| Rainy home win | In ordinary home games, a wet 22+ lead builds extra departure pressure in the last half of Q4; 35+ reaches full severity. No instant per-score loss, dry/close/early changes, neutral/postseason expansion or refill. Verify a finish near 25–30% visible fullness for a comparable starting density/story; this is not a universal forced target. |
| Beat the traffic | A 17+ home deficit accelerates exits in the last third of Q4; 28+ can begin gently in the last half. Hopeless 28+ losses may use up to 10 extra original-crowd points above the normal quarter allowance. Floors, safe update windows and postseason limits remain. |
| Settings | Edit/reopen a toggle, slider, decimal value and Fan Patience; check Reset, Back and keyboard/controller tab navigation. Stock game settings must still work. The basic tab/fullness toggle is already verified. |
| Camera/sound gates | Dynamic Camera Effects Off and DGA Camera Shake Off stop added movement. Cannon strength 0 keeps native cannon audio. Pause and recorded replay stay steady; normal effects resume without stale celebrations. |
| Neutral camera | Both teams receive crowd movement at 75% of normal strength, including their own first offensive snap. Verify long gain/TD handoffs and the 0.9-second snap fade. Native cannons retain their independent strength. |
| Sack freshness | A sack followed by an incomplete pass cannot carry the sack shake onto the incompletion. Genuine sacks still react; test with pass-anticipation sound disabled too. |
| Clock management | Kneels and clock-stopping spikes produce no added pre-snap or sack/stop shake. Genuine sacks and fake-spike plays remain eligible. |
| Major replays | In manual play, a confirmed TD/takeaway plays its full native sequence before a separate inset repeat. In Slow Sim, keep native full-screen playback without expecting an inset. Switch back to manual and verify no old inset returns. Check ordinary TD, traditions, picks, forced fumbles and PAT transitions without skipping; preserve `[major-replay]` entries. Missing recordings must fail safely. |
| Cheer acceptance | `[reaction-audio]` distinguishes queueing, voice registration and one bounded recovery. Listen for the actual third/fourth-down and interception cheer; a log alone cannot confirm audibility. |
| Session cleanup | Try a timeout, replay, Super Sim and a second game. No stuck loops, old reaction IDs, crowd refills or repeated errors should carry over. |

## Additional checks before claiming full coverage

- A real home blocked field goal, including its sound/impact and possible return/score handoff; a normal missed kick must not count as a block.
- A game-sealing takeaway in the default RTG player camera, if RTG is included in the advertised support. Offline Dynasty remains the verified mode; do not imply all modes are guaranteed.
- Another stadium's native cannon, ideally using a different variant from Texas. All nine paths are covered offline, but that is not nine manually played venues.
- A late blowout with another quarter length, plus a close game that should retain fans.
- Exactly five yards lost versus four without sack proof, and a fourth-down sack after possession flips.
- Confirm any initially failing case after a fix on the same final release candidate. Keep unresolved limitations explicit.

## Files and logs

Only DynamicGamedayAtmosphere.dll stays directly in Plugins. DGA owns:

```text
Plugins/DynamicGamedayAtmosphere/
  DynamicGamedayAtmosphere.ini
  Logs/launcher.log
  Logs/DynamicGamedayAtmosphere.log
  Logs/status.txt
  Backups/
  History/
  Diagnostics/
  Tools/DgaDiagnostics.exe
  Runtime-1.78.0/
```

The one-time Update 3 reset saves the exact original INI in Backups before replacing it. SettingsResetVersion=3 records the release-family reset and must stay unchanged in routine builds. Later custom settings persist. The launcher waits for a running game to close before moving/resetting older settings. Locked settings or unavailable backups stop preparation without discarding the original. Game saves, native settings and unrelated plugins are untouched.

Normal logs include play summaries, live reactions, pass-flight, cannon, departures, configuration and activation errors. Cannon starts have a process-local sequence number so separate blasts are not swallowed by identical-message suppression. Native callback duplicates remain bounded by the existing observer, and no per-frame cannon log is added. Entries say an impact was requested; native accessibility, camera context and higher-priority motion can still prevent or cover visible output.

Use Debug=1 temporarily for native voice diagnostics, then restore 0. A native instance or dispatch request does not guarantee audible output. For a missed reaction, save the complete session logs and note the play's quarter/clock, side, down, result and expected effect. Status shows fullness and admission gates; 100% of original attendance can still mean a partly full stadium.

## Controlled previews and compatibility reports

After a verified offline match loads, open Tools/DgaDiagnostics.exe. Choose a preset, Sound and/or Camera movement and a duration, return to the unpaused lineup without snapping, and use F8 to start or F9 to stop. Disable the play clock manually if needed. Presets end automatically; a snap, pause, replay or world change cancels them. Camera-only previews are intentionally silent. Previews compare output routes; they do not prove automatic play detection or eligibility.

The same tool can save a read-only game compatibility report even when DGA cannot activate. Reports contain build metadata, module names and bounded samples at existing bindings, not game binaries, saves or full memory dumps. Nothing is uploaded automatically. A report is diagnostic evidence, not a compatibility certificate.

Launcher failures name the operation, PID/path and Windows error where available. Readiness failures back off at 5, 15, 30 and 60 seconds; unchanged errors are summarized. Each game process gets one actual runtime-load attempt. A required native mismatch stays inactive rather than installing speculative hooks.

See [building and verification](../BUILDING.md), [candidate release notes](../releases/Update3.md) and the [README troubleshooting steps](../../README.md#problems).
