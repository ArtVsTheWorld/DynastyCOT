# 2.1.9 private visitor calibration and Utah–BYU research

## Implemented

- Visitor targets use continuous fanbase-relative odds, with a shorter-distance travel curve, same-state and rivalry increases, seeded fixture variation and a 1–70% launch range. Neutral appearances remain native. Later departures may produce a higher visiting share.
- Outfit assignment accepts an original occupied-seat snapshot. Its denominator includes the native visiting spectators and excludes musicians and empty positions. Unoccupied positions keep their existing appearance bits. Required groups keep their native allegiance and uniforms.
- Native generation uses a completed, owned original visibility snapshot when one exists. It never uses thinned, replay or current-camera masks as the original audience. A new generation log records whether that snapshot was actually available. **The first generation may precede visibility capture; exact sparse-crowd outfit ratios remain a live acceptance item.** There is no claim that the helper-level denominator tests establish the engine's callback order.
- Existing saved fixture targets and outfit seeds remain frozen. New fixture plans use the revised calibration. The persistence reader accepts targets through 70% without discarding older fixture/history ownership.
- Verified protected rivalries retain their event-specific opening range and 30%-through-Q1 arrival deadline when the richer Dynasty launch capture is unavailable. This fallback does not replace EA's peak turnout or authorize a write without verified audience data.
- Launch diagnostics record native setup-mode transitions and the witness state at rejected input captures. This pass does not extend the ten-second loading witness or remove identity/mode guards without evidence.

## Calibration

The production formula was run across 1,000 seeds for each of eight synthetic matchup conditions. These are controlled scenarios using fanbase audience counts, not estimates of particular schools in the user's save. Detailed old/new ranges and visitor counts among 20,000 spectators are in `Build/Update4Visitors219/visitor-calibration.csv`.

| Condition | New range |
| --- | --- |
| Distant, limited traveling support | 1.51–1.84% |
| Ordinary comparable fanbases | 5.32–6.42% |
| Nearby comparable fanbases | 10.73–12.81% |
| Strong traveling fanbase | 12.06–14.35% |
| Large fanbase at a less-supported host | 23.99–27.83% |
| Exceptional nearby rivalry takeover | 58.45–64.79% |
| Extreme fanbase imbalance | 70% |
| Comparable in-state rivalry | 16.42–20.45% |

## Live evidence: Utah home / BYU away

Bound read-only capture: runtime 2.1.8, PID 27116, home 1218 / away 1116. Native rivalry event 27 was separately verified through the option descriptors. The user confirmed launch from Dynasty.

- At 16:01:30 local the playable input observation rejected its Dynasty witness. No matching revised attendance/crowd fixture was captured. EA supplied 51,444 / 51,444 attendance/capacity; DGA's ordinary Utah late-arrival/snow fallback started at 43%. This was not a low peak-demand result. Why the witness was rejected is not established by the old log; the new diagnostics are needed on a fresh launch.
- The visitor appearance plan was unavailable for that launch. Remaining supporters were measured at about 2.1% away, rather than the new rivalry target. Current supporter percentages use actual remaining spectators.
- Play 16 at 16:08:18: away TD, native TD predicates true, no replay playback before the PAT.
- Play 22 at 16:09:53: home TD, DGA major-play arm confirmed, no replay playback before the PAT. The user confirmed no manual skip. No recognized TD-selector trace was emitted for this play; the old tracing only classifies eligible top-level selectors, so absence of that trace is not proof no native TD condition was evaluated.
- Play 34 at 16:15:10: home TD followed by native replay playback at about 16:15:31 and later phase-5 playback. Flags alone do not establish the complete rendered full-screen/inset sequence.
- Play 43 at 16:18:44: official 48-yard home run, full crowd, layered effects eligible. The long-run reaction was emitted at the whistle, with zero measured live reaction distance. Score difference did not reject admission. Missing live carrier/coordinate proof needs a dedicated sample; the old capture did not collect those inputs. No speculative camera change was made.
- Play 44 at 16:19:25: home field goal. User saw the kicker close-up, no celebration, then a replay. This was an ordinary kick, not a verified last-minute go-ahead kick; it does not establish how the clutch-kick route behaves.

## Presentation and kicker research

Evidence is preserved under `Build/Update4NativeResearch/UtahBYU219`.

- `SkipPressAnyButton` resolves through the native UI service to RVA `0x7aa3530`, which reads `*(image+0xe357240)+0x2a`. Offline writer inspection shows this byte is copied from configuration. It is **not** a record of an actual skip-button press. Its zero value cannot prove no skip occurred or establish a working manual-skip blocker.
- The bounded listener/node scan found no verified live instances in its coverage. It was incomplete by design; that is not evidence those listeners are absent.
- Native skip packages also perform audio/view cleanup. None were blocked. No presentation-skip setting has been enabled.
- Six current presentation resources were exported and decoded read-only. The package named `postplay_GEN_fieldgoalToLead_p` selects coach-reaction scenes, rather than a kicker celebration. The postgame field-goal flow also contains team/coach reactions. Legacy kicker-team celebration clips exist as candidates; their actor binding and suitability in CFB are not verified.
- No animation dispatch, input interception, live setting change or additional game hook was applied during this research.

## Validation and remaining checks

- Full production build: 163 PASS lines, including 8,000 visitor scenarios and 936 full-game attendance simulations
- Actual native appearance helpers checked with sparse occupied masks, protected groups, retired streams and selector restoration using self-process fixtures
- Wider fan-budget/save/replay, settings, loader and reaction checks passed
- Final native recompilation and targeted production-path checks are recorded separately in the final-check log

Next live check: install the private candidate with MMC/game closed, return to the Dynasty hub, launch Utah–BYU fresh and pause at kickoff. Confirm event-based opening attendance, read the launch-mode transitions and determine whether original occupied visibility exists during first outfit generation. Presentation protection, missed-TD routing, the 48-yard run sample and clutch-kicker actor binding remain research/follow-ups; they are not advertised as fixed by this candidate.

## Later placement adjustment, 2026-11-06

The user requested slightly more mixing and companions for scattered visitors. Working source now assigns roughly 70% of selectable visitors to the main pocket and 30% to scattered parties of two to four. Neighbor selection uses physical coordinates and a bounded 3-D grid, favoring nearby seats on the same row. Overall visitor targets, protected spectators, band exclusions and occupied-seat budgets are preserved. Missing neighbors can leave a singleton; empty seats are never filled just to supply company.

Sixty synthetic bowl layouts passed exact-count and companion checks (minimum 81.5% of scattered visitors had a nearby companion). Seven affected test programs, including 8,000 calibration scenarios and production appearance/fan-budget fixtures, passed. The full native runtime compiled with warnings treated as errors. See Build/Update4VisitorParties/verification.json. This is not live visual acceptance.

The existing private 2.1.9 ZIP predates this placement change and was not replaced. Installed runtime remains 2.1.8. Fresh PID29940 captured Dynasty inputs correctly and reused an existing 11% visitor fixture plan. At the later snapshot the crowd was 86% full with 14% still arriving, mixer healthy, no new fault-tagged events, and replay additions disabled. One 5-yard official result expired after about 74 seconds. The prior rapid-toggle crash and TD replay fault remain unresolved; see BeforeRestart219/Crash27116/findings.md.
