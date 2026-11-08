# Player Analytics: 0.1.14 audit and verification

This is an integration-only, source-built private candidate. The comparison baseline is 0.1.13 (`6cdf8de0`). The active Oklahoma Dynasty was read without writing to game memory, installed files or saves. Its native clock was 2027, BowlSeason1 Week 17 (native year index 1). Raw captures and generated audit feeds remain in ignored local Build/RC014, outside source/distribution.

## What the sim revealed

78,820 recorded player-game rows and 11,730 roster/Season-view players were audited. Among the same 4,841 players with at least 200 snaps, the old season grades averaged 60.49 with a standard deviation of only 3.70. Just 58 reached 70 and none reached 80. QB maximum was 67.7; OL maximum was 66.2; punters averaged about 50.4. The problem was systematic compression, not evidence that almost everyone played poorly.

The old model averaged already-shrunken weekly grades. That compressed season variance further, while frozen priors did not fit this sim well. OL upside relied on pancakes, which exist in native game and season records but appeared in only 421 of 13,409 OL game records (3.14%). K/P production was divided by all recorded participation, including unrelated special-team work, rather than the relevant attempts.

All consumed production fields were found in both native game and native season stats: 83,469 player/field comparisons. Totals reconcile when ordinary and kick/punt-return variants are combined. Native game and season representations of full/half sacks differ, but all 3,899 defensive season records reconcile using full sacks + half-sacks / 2. The model retains that weighting. Pancakes are deliberately excluded from grading, rather than falsely described as absent.

## What changed

- Season grades now use accumulated released game production, not an arithmetic average of weekly grades. This matches the tracked season-wide measures while excluding current-week games. No native stat or game result is written.
- Production is compared within position, native year and edition. Eligible populations use tied midpoint percentiles mapped to an approximately normal curve centered at 68 with an 11-point spread before reliability/uncertainty. This is relative production, not an independent film evaluation. Names, ratings and development traits do not determine grades.
- Cohorts require at least 24 qualified observations: 20 weekly / 200 season snaps, or 2 weekly / 8 season specialist attempts. Smaller samples shrink toward the center; sparse cohorts use bounded, newly calibrated production priors. Missing required stats stay N/A. Ties are not artificially spread into a full curve.
- OL uses recorded sacks allowed and participation with an exposure prior. It can grade high without a pancake, and low when sacks allowed increase. This cannot evaluate run blocking or pass-block win rate. CB/S use recorded catches allowed, but targets and coverage responsibilities remain unavailable; complete coverage grades are unsupported.
- Kickers use FG/PAT attempts and distance buckets; punters use punts, net yards, inside-20, touchbacks and blocks. Unrelated kickoff/holding participation no longer dilutes production.
- Coverage uncertainty is persistent and deterministic: one-star-or-lower programs have a maximum +/-6 season points, falling by one point per prestige star to +/-2 at five stars. Unknown prestige uses three-star uncertainty (+/-4). Weekly variation blends the persistent component with a smaller fixed weekly component. Prestige has no directional quality adjustment; identical-production tests confirm roughly three times the uncertainty SD at one versus five stars.

Updating this derived model recalculates available Analytics estimates, including past-week views. Existing published post wording/history, settings, account identities and native saves remain untouched.

## Same-input distribution comparison

The table uses matching qualified players in both builds (200 snaps; specialists 8 attempts in the new model). No qualified 200-snap grade was lost.

| Position | Players | Old mean | New mean | New SD | New range | New 80+ |
|---|---:|---:|---:|---:|---:|---:|
| QB | 171 | 56.8 | 68.2 | 10.3 | 41.2-98.9 | 22 |
| RB | 324 | 60.8 | 68.1 | 10.1 | 39.4-97.9 | 42 |
| WR | 523 | 59.9 | 68.3 | 10.3 | 39.4-99.8 | 70 |
| TE | 261 | 62.6 | 68.2 | 10.6 | 40.0-99.4 | 32 |
| OL | 935 | 59.3 | 68.1 | 10.5 | 37.4-100.0 | 126 |
| DL | 848 | 61.1 | 67.9 | 10.4 | 39.2-95.4 | 106 |
| LB | 700 | 60.8 | 68.4 | 10.2 | 39.5-100.0 | 88 |
| CB | 521 | 60.8 | 68.1 | 10.4 | 34.5-100.0 | 73 |
| S | 553 | 61.6 | 68.0 | 10.4 | 39.4-100.0 | 78 |
| K | 141 | 59.2 | 67.9 | 9.6 | 42.7-90.6 | 18 |
| P | 139 | 50.4 | 67.9 | 10.7 | 42.5-90.6 | 22 |

Across the same 4,841 200-snap players: 70+ rises from 58 to 2,054; 80+ rises from 0 to 638; 90+ becomes 83. The new mean is 68.12, SD 10.38. Bins below 50 / 50-59.9 / 60-69.9 / 70-79.9 / 80-89.9 / 90+ contain 181, 855, 1751, 1416, 555, 83 players. Every position has high and low grades, with most players near the middle.

The six most recent regular-season editions were also compared. Oklahoma has recorded games in Weeks 10-13 and none in 14-15; those missing team weeks were not padded with invented grades. Its Week 10-13 revised means are 69.6, 70.3, 68.6 and 69.0, with both strong and weak individual performances. Tiny late-week national cohorts retain sparse-sample handling rather than being forced into a full curve.

## PFF Social posts

PFF / @PFF_College uses the requested native bticl_win64_P_color image, verified as library 88 / asset 125. Existing account IDs are unchanged. A dedicated persistent PFF identity avoids repurposing an older analyst account.

32 alternatives cover four families, eight each: weekly standouts, season spotlights, lower-grade watches and grade movement. Posts consume the exact compiled Analytics model; there is no second JS grade formula. Grades require corroborated player/team identity, sufficient released samples and matching Dynasty/year. Movement requires more graded games in the same model, team and position group. A model upgrade, transfer or position change is not reported as a performance jump.

At most four optional roots per edition, one per team/player. Weekly standouts require a strong grade. Unchanged samples are suppressed, standalone posts replace generic praise threads, and already published PFF wording/grade/account is frozen for same-edition refreshes. Important news, the fifteen-root routine team budget, exempt recruit statlines/replies and Social National's 250-card limit remain unchanged.

Examples from as-of windows of the actual recorded games (not fabricated game incidents):

> Week 1 grade spotlight: Oklahoma S Michael Boganowski. 90.1 across 68 recorded snaps.

> Tracking Oklahoma LT Ziyare Addison: the season estimate is up to 83.0, from 78.9.

> Oklahoma QB Keelon Russell: season grade 93.2, built from 467 recorded snaps.

Fifteen as-of windows were rendered: preseason, thirteen consecutive regular-season windows and the current bowl window. These are derived from the captured games, not fifteen separately captured checkpoints. They produced 56 eligible PFF roots using 22 variants; preseason had none. Lower-grade watches, sparse data, long names, offseason exclusion and all 32 variants are additionally exercised in fixtures. Every candidate rendered with explicit required fields inside the native 280-character / 512-byte limits. PFF text contains COT production estimates, not real PFF reporting, film study or proprietary grades; no affiliation is implied.

A fresh-observation full-feed stress fixture from the current award/signing-heavy capture stored 4,400 stories both with and without PFF enabled. This recreates a first observation offline; it is not a readout of the game's already published history. Its important-news coverage already consumes the available team budgets, so no PFF roots survive that particular full-feed selection. The PFF-only window audit proves eligibility/wording, not visibility in a saturated full feed. This is deliberate: PFF must not bypass the protected-news policy or add posts above the existing routine budget.

## Performance and checks

- Source build: 7,317 model/distribution checks, 2,489 native Analytics integration checks, existing Fog/UI/currency/ownership suites and 288 native/JS Social scope-category parity checks passed. No test installed hooks in the game.
- 440 Node tests passed, including correct native icon/account, deterministic refresh, history freezing, Dynasty/year/release guards, compact persisted observations, genuine movement, poor-week eligibility, twelve-edition reply suppression and all-template long-name/native-length checks.
- Offline read-only audit found zero rejected native linkages. Missing required fields, unplayed/practice games, duplicate references, native ratings/traits, unknown stages and current-week availability retain their tests.

On identical captured inputs, median grading-dataset generation was 4.81s before and 7.62s after (three runs each). The fuller model has a measurable background rebuild cost. Median cached Season team-page construction fell from 84.8ms to 18.1ms (100 calls each). The existing unchanged-capture cache avoids unnecessary rebuilds; grading remains on native worker paths, not per-row scrolling callbacks.

The full Social control/candidate generation medians were approximately 1.96s / 2.10s on the same 4,400-story input. This is an offline feature comparison with the PFF summary omitted/present, not a 0.1.13 live UI benchmark. Native capture, file IO, asynchronous card loading, end-to-end menu latency and FPS are separate measurements. No fresh live responsiveness or overhaul-compatibility pass is claimed.

Private audit evidence is local under Build/RC014: before.json, after.json, comparison.json, sack-units.json, edition-summaries.json, pff-render-audit.json, source-build.log and node-tests.log. Captures and generated feeds are excluded from the source and payload. Final archive: 156 manifest entries; exact packaged native ABI/build-pair and all 16 module combinations passed. 116 installer engine, 36 GUI/engine and 95 updater checks passed in isolated fixtures, including 0.1.13-to-0.1.14 installation/rollback. All 983 standalone original source hashes remain unchanged. Final asset hashes are recorded in release-evidence-0.1.14-private.json and SHA256SUMS-0.1.14-private.txt.

## Live acceptance still needed

1. Install the private candidate after closing MMC/game and restart. Check a disposable copy of a DYNASTY-EPUFT checkpoint, plus the Oklahoma sim if desired. Current week must be absent and Season must include only earlier released games.
2. Inspect high/low OL, QB, defensive and specialist samples. Check sorting, team/week switches, full Card/Back identity and Fog. National Analytics remains removed; the nonworking Stats shortcut remains hidden.
3. Advance one normal weekly checkpoint and review PFF under Analysis / For You / team/conference scopes when the budget permits. Compare each shown number with the corresponding Analytics week/Season, then reopen/reload and check stable wording.
4. Test another Dynasty for isolation; inspect existing published posts/settings; measure actual menu responsiveness and FPS. A successful source build does not certify the installed game or an overhaul combination.

## Private candidate artifacts

- Installer: `Release/DynastyCOT-Setup-0.1.14-private.exe`. SHA-256: `545066d450719c881c7fb2984cea21c80db797d105e237acca5c2602788f77a4`.
- ZIP: `Release/DynastyCOT-0.1.14-private.zip`. SHA-256: `8d42876e1d433d8c8b53d1328d8813dd976bcec8e45b3fda2963fcb76de6cfc1`.
- Matching native build ID: `20261108220556353`, Analytics ABI 2.
- No live install, save write, push or publication occurred.
