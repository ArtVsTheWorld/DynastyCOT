# PFF editorial and National capacity: 0.1.15

Baseline: 0.1.14 source commit `7209abc`. Integration workspace only; no standalone project, live installation or save was modified. The attachment was treated as editorial data: the implementation supplies its missing factual/ranking logic rather than treating proposal text as executable instructions.

## Imported copy

`pff-college-editorial-deliverables.zip` contains the revised pack, the five-family proposal and an editorial review. The revised pack supplies 48 base variants: 5 of the prior 32 bodies retained, 27 rewritten and 16 added. All 15 proposal templates are enabled through new guarded events, for **63 total PFF alternatives**. Existing IDs and evidence voice metadata remain. Proposal IDs remove the `proposed` namespace now that their factual paths exist. Em/en dash separators use ASCII punctuation to avoid unsupported native glyphs.

The account remains PFF / @PFF_College, library 88 / asset 125 (`bticl_win64_P_color`). Published text/accounts/history are not rewritten. The current same-edition freeze preserves older PFF posts on an upgrade; new copy and honors appear in new publications. No native award, player stat or save value is written.

## Qualification and selection

Rankings use the exact compiled grades supplied to Analytics. The model remains `0.2.0-production`: all 936 weekly and 8,237 season grades in the prior read-only sim fixture match 0.1.14 exactly. There is no second JS individual-grade formula.

Leader claims need more evidence than an individual spotlight. The Social native worker now supplies explicit FBS membership, played weekly teams and position completeness. Current native season participation is reconciled against all finalized game caches, including a completed current game, while ranking inputs themselves still exclude unreleased/current-week games. Missing/incompatible grade inputs, incomplete participation, identity/team/position mismatches, duplicate candidate rows, unplayed schedule entries or absent coverage metadata suppress affected claims. No expanding capture or grading work is added to the Analytics scrolling path.

- **Position Top 3:** one rotating eligible position per edition; at least three graded games and 200 snaps, or eight attempts for K/P. Actual roster positions remain distinct. FS is labeled Free Safety, SS Strong Safety, and edge/linebacker subpositions are explicit; a narrow board cannot masquerade as a broader-position leader list. Exactly three players/schools/grades are printed. Ties inside the board use shared rank labels; a tie crossing the third-place boundary omits that position's board. Workload/record identity only stabilize order inside a labeled tie, never break a grade tie into a false ranking.
- **Offensive/Defensive Player of the Week:** highest unique qualified weekly grade in the verified offensive/defensive field; minimum 20 snaps. Offense includes OL, defense includes front/linebacker/secondary groups; K/P are excluded from these two honors. An ambiguous highest-grade tie omits that honor rather than picking an arbitrary winner.
- **Offense/Defense of the Week:** snap-weighted mean of the unit's tracked individual grades, rounded to one decimal. Every played FBS team must have a usable unit for the comparison. At least eight players must have 20+ snaps. Offense requires a QB, five linemen and two skill players; defense requires two defensive linemen, one linebacker and three secondary players. Roster LT/RT labels need not match on-field assignments, and nickel fronts vary. All valid positive-snap unit contributors enter the weighting. A missing unit or tied highest displayed unit grade omits that unit honor; an incomplete team is never silently removed so a runner-up can win.

Weekly player and unit slots alternate by the released source week, falling back to the other supported honor if the preferred form is unavailable. A season board and the existing program/spotlight logic share the remaining slots. The total stays **at most four optional PFF roots**, within the unchanged fifteen-post routine team budget and important-news protections. A season board may mention a school that also earns a distinct weekly honor; all mentioned schools count toward their normal team budgets. Reactions are not automatically attached.

Top 3 boards first try full names/schools, then initials plus verified native school short names. No school abbreviations or statistics are invented. Ambiguous or still-overlong boards are omitted intact. Changed boards use leaderboard facts, not merely a new week label, for repetition history.

## National: 300 cards

Both native `FeedData.h` and JS `feed_visibility.mjs` now cap National at 300 displayed cards, including replies. Generation budgets and stored publications are unchanged. Category filtering occurs before selection. Official draft picks and important news retain priority, parents accompany selected children, and compact numbering reflects the displayed count. Sparse views are not padded. For You, conference/team scopes, one-edition-per-week behavior and the removal of National Analytics are unchanged.

Native tests cover a 9,300-story candidate catalog, a busy 224-pick draft plus news, nested/reply-category cases, stable numbering and unchanged stored history. The 288 native/JS scope-category parity cases pass at the new limit.

## Verification and actual-data examples

- 449 Node tests pass, including all 48 revised bodies, all 15 proposal forms, exact qualification/ranking, ties, incomplete/foreign/duplicate data, weighted units, long-name fallback, position-label accuracy, deterministic refresh and 300-card draft/thread protection.
- Source build passes 7,317 grading/model and 2,491 Analytics integration checks, existing Fog/UI/currency/ownership suites and 288 native/JS scope-category parity cases. Native season/cache participation checks include a deliberately mismatched fixture that must suppress a leader claim. No live hooks are installed by these tests.
- Fifteen derived windows from the prior read-only Oklahoma capture (preseason, thirteen regular-season windows, current bowl window) render **56 PFF posts**, at most four per edition, with a maximum body length of 171 characters. The latest native participation corroboration is used for these derived windows; they are not fifteen independently captured checkpoints or a live UI test.
- Rendered families in that corpus: 12 Top 3 boards, 7 Offensive Player, 9 Defensive Player, 7 Offense, 5 Defense, 5 weekly standouts, 3 season spotlights and 8 movement posts. Low-grade season-watch eligibility is additionally covered by fixtures. Every proposed type is exercised by actual recorded-game inputs.

Examples from the current captured window:

> Three names leading the Free Safety grade board through Week 15:
> 1. Michael Boganowski (Oklahoma) 95.6
> 2. Jalen Thomeson (San Diego St.) 91.0
> 3. Zechariah Poyser (Miami) 85.4

> Tennessee takes our Week 15 offensive honors with a 75.4 team grade.

> Defensive Player of the Week: Ari Watford of Clemson. 88.9 in Week 15.

These examples prove event/render eligibility, not visibility after every busy full-feed budget. Existing important news may crowd out optional PFF content. Source files and raw captures remain unchanged by the audits; generated reports/captures stay in ignored `Build/RC015`.

Offline candidate ranking took about 16 ms on that 936-weekly/8,237-season-row input. A separate 400-root, 100-iteration JS selection fixture averaged about 0.29 ms at the 300-card limit. These are generation/selection timings, not native widget creation, end-to-end menu latency or live FPS. Showing 50 additional native cards can cost more to render; live scrolling acceptance is still required.

## Remaining acceptance

Use disposable DYNASTY-EPUFT checkpoint copies. Follow the [release checklist](release/0.1.15-private.md): new-publication PFF copy/honors, correct grades/position/week, stable reload/history and Dynasty isolation; busy National/category/draft/thread behavior at 300; team-only Analytics/Fog/Card-Back regression; live menu/FPS and exact game/AIO combination. No live compatibility claim follows from source or fixture checks.

## Final private package checks

- 157 manifest entries; exact packaged Analytics ABI/build pair, host resources/helpers and all 16 module selections pass.
- 116 installer-engine, 36 GUI/engine and 95 updater checks pass in isolated fixtures, including 0.1.14-to-0.1.15 install/rollback and data preservation.
- 15 release-metadata/provenance/privacy unit checks pass. Public-source boundary scan has zero forbidden files/credentials; all 983 standalone original-source hashes remain unchanged.
- Native source build ID: `20261108225801145`, Analytics ABI 2.
- Installer: `Release/DynastyCOT-Setup-0.1.15-private.exe`; SHA-256 `a915d60e0ff92e0860b9fcd5cf28bb3dd00cea228998140d88876da6a0ad1f0f`.
- ZIP: `Release/DynastyCOT-0.1.15-private.zip`; SHA-256 `c6bf1972903b0c7f7178356a728cc8e691184fdb56cbea36bb407b4fd627d9f4`.
- Integrity/provenance: `Release/SHA256SUMS-0.1.15-private.txt` and `release-evidence-0.1.15-private.json`. No live install, save write, push or publication.
