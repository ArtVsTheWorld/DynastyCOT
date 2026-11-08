# Dynasty COT 0.1.11 editorial verification

Private source-built test candidate, compared with 0.1.10 (`f40a80fe27a3cc23cbd53d255325c9e563b375f4`). No live installation or publication. Author: Ace.

## Audit and finite implementation

Reviewed the supplied checkpoint-09 proposal against its exact exported source hash `9882f7258b296b2e1de87c640989852e40c2e04a6720ca129682a48c6698a1fd`: 585 families, 7,824 existing templates, 6,129 proposed replacements, zero proposed additions. Import checks require the original ID, family and wording to match; a stale proposal fails rather than patching another template.

The review retained 1,378 proposed originals where voice, humor, historical wording, observation timing, projection provenance or meaningful status details were stronger. It changes 4,755 existing bodies, including unit corrections to proposal-unchanged entries. Metadata, guards and existing IDs stay in place. For example, the player's opponent is `vs. opponent`, never a team attribution; complete result sentences stay separate; losses after a Q3 lead do not imply a 15-minute quarter or a regulation finish. Catch-up commitments retain their observation interval. Per-game and pass-attempt denominators are explicit. Coach/transfer/undrafted and named-poll distinctions survive shortening.

Exactly 150 new variants: six apiece in 20 primary families (120), plus 30 compact factual alternatives and parent-engaged replies across ten supplemental families. The latter cover dense statistical/NIL/matchup copy, upset celebration/frustration/rivalry responses and restrained player replies. No second expansion was performed.

Family choice used the current native displayed windows from the recorded replay, plus historical full-year audit evidence. The high-volume National families differ from the personalized feed. Counts below are displayed cards, not stored totals; eligibility averages/minima are final rendered candidates per audit callback, including both rendering passes, not raw template counts or unique events.

| Primary family | For You / National baseline cards | Mean usable alternatives before → after | Minimum before → after |
|---|---:|---:|---:|
| `narrative.game` | 22 / 943 | 29.9 → 35.9 | 25 → 31 |
| `cycle.signed-spotlight` | 52 / 795 | 32.0 → 38.0 | 32 → 38 |
| `roster.commit` | 35 / 592 | 41.0 → 47.0 | 41 → 47 |
| `team.win` | 13 / 480 | 62.0 → 68.0 | 62 → 68 |
| `world.award` | 38 / 358 | 40.0 → 46.0 | 40 → 46 |
| `cycle.recruit-fit` | 13 / 358 | 28.0 → 34.0 | 28 → 34 |
| `team.narrow_win` | 2 / 256 | 10.0 → 16.0 | 10 → 16 |
| `cycle.team-welcome` | 38 / 227 | 38.0 → 44.0 | 38 → 44 |
| `draft.pick` | 0 / 213 | 28.0 → 34.0 | 28 → 34 |
| `preseason.honor.beat` | 2 / 203 | 28.0 → 34.0 | 28 → 34 |
| `history.team-counter` | 0 / 186 | 28.0 → 34.0 | 28 → 34 |
| `team.rivalry_win` | 4 / 166 | 10.0 → 16.0 | 10 → 16 |
| `team.poll_up` | 12 / 154 | 58.0 → 64.0 | 58 → 64 |
| `transactions.coach-deal` | 0 / 139 | 10.0 → 16.0 | 10 → 16 |
| `launch.kickoff` | 1 / 138 | 10.0 → 16.0 | 10 → 16 |
| `portal.unsigned-local` | 39 / 0 | 34.0 → 40.0 | 34 → 40 |
| `injury.report` | 34 / 49 | 42.0 → 48.0 | 42 → 48 |
| `transactions.signed` | 26 / 20 | 10.0 → 16.0 | 10 → 16 |
| `buzz.team-pregame` | 20 / 34 | 10.0 → 16.0 | 10 → 16 |
| `reply.player.award` | 14 / 6 | 10.0 → 16.0 | 10 → 16 |

The selection audit found retired 30/25/60/100-card assumptions in repetition memory/refinement, although the native UI shows complete local scopes and up to 250 National cards. The candidate now uses that actual native selection contract. A weak-key cache reuses immutable archive projections; near-shape matching reuses structural matches while reading current weights. Native/JavaScript parity is tested over sparse/busy/draft/orphan/reply/fractional-priority fixtures, all four scopes and all twelve types (288 comparisons). The native list implementation and National limit are preserved.

## Same-input comparison

31 recorded DYNASTY-EPUFT editions: preseason, 17 consecutive regular-season editions (0–16), all three bowl stages, the title stage, offseason 0–7 and 9. Offseason 8 is absent from the source and was not reconstructed. Six additional explicit synthetic fixtures cover WMU 31–26 over #14 Michigan, missing context, long names, recruit records, a title result and offseason draft reporting. These fixtures are labeled test inputs, never historical reporting.

The replay uses retained event facts. Missing old stat tables, roster membership and brand grades are not invented. It is a new-publication replay, not a claim to reproduce every original live menu visit. The fixture/input hashes and detailed outputs remain in ignored private Build files.

Near-duplicate metric: the normalized template structure is identical, or its word four-grams overlap by at least 70% of the larger set, against earlier cards in that view and four preceding editions. Placeholders replace names/numbers: changing teams, players or scores cannot create diversity. Views overlap and must not be summed into a single independent corpus.

| View | Displayed cards, each version | Near-duplicate cards before → after | Reduction | Adjacent near-duplicates before → after |
|---|---:|---:|---:|---:|
| For You | 1,044 | 92 → 59 | 35.9% | 0 → 0 |
| My Team | 424 | 22 → 8 | 63.6% | 0 → 0 |
| Conference | 1,404 | 329 → 229 | 30.4% | 1 → 0 |
| National | 7,750 | 6,176 → 5,917 | 4.2% | 920 → 1 |

**Limits:** the broad National multi-edition metric improves modestly; it does not meet a 50% reduction. Adjacent repetition improves sharply. Required results, awards, signings and draft selections can exhaust factual alternatives even when dozens are eligible. The candidate preserves that news rather than dropping it or inventing angles. Across the 31 National windows, 994 displayed cards use one of the 150 additions. All six supplemental fixtures have zero measured near-duplicates, but this small result is not a substitute for the recorded-year metric.

Generated posts stay exactly 48,772 in both versions; primary stories stay 44,387. All 31 views retain identical displayed counts. Stored posts are 48,722 → 48,724: two already-generated replies no longer collapse under exact-wording deduplication. There are no extra generation slots, replies-per-parent allowances or higher team/National caps. The 15 routine roots per team, recruit-statline exemption, important-news protections, all 224 draft slots, one edition per week and existing categories/scopes remain intact.

## Checks and factual safeguards

- No baseline protected event lost; primary award conservation checked in every edition. Identity/handle/role mismatches among compared displayed IDs: zero.
- Tests verify source-backed units, attribution, timing, sparse guards, long names, 280 Unicode-codepoint and 512 UTF-8-byte bounds, no unresolved placeholders, thread ancestry, relevant upset replies and differentiated celebration/frustration/analysis/rival positions. Overlong individual values still suppress an impossible variant rather than truncating a name or inventing facts.
- Existing account/style rules remain in force. General rewriting does not overwrite personality-tagged copy. Fans retain jokes and pettiness; restrained player alternatives are available without converting every account to the same vocabulary.
- Same-week published cards, their wording, identities and ancestry survive content upgrades. Tests cover stable refresh, deterministic fresh generation and Dynasty isolation. No save/profile/history migration or mutation is introduced.
- Recruit game records remain persistent, mod-generated records, separate from native data. Their native type label is **COT HS Game**; Social Media's setting description explains the source. Stored generated posts retain `recordSource`. No native rating, trait or record writes are added. There is no technical prefix forced into every body.
- Existing native container rendering is retained. The disabled experimental list/sidebar path stays disabled. Hook ownership, exact fingerprints, scope checks, sibling-build identity and helper verification remain on the existing paths.

In the personalized corpus, roles retain different lengths and cadences. Length alone does not prove personality; the private examples also show different claims and emotional positions, and persistent account character/analyst-interest tests still pass.

| Role | Cards | Mean words before → after | After range |
|---|---:|---:|---:|
| beat | 445 | 17.2 → 16.1 | 6–37 |
| analyst | 132 | 21.9 → 18.3 | 7–29 |
| official | 159 | 12.3 → 12.1 | 4–22 |
| fan | 139 | 13.9 → 13.5 | 7–22 |
| player | 68 | 9.5 → 9.9 | 5–20 |
| recruit | 86 | 11.4 → 11.4 | 3–21 |

## Performance

Final warm ABBA comparison: two 31-edition runs per engine, identical input, author-input copying outside timing; no exhaustive audit callbacks. Capture, process startup, disk IO and live menu/FPS time are excluded. Before mean: 7.654 seconds/year; after: 8.214 seconds/year (+7.3%). Full generation includes more accurate visible-window refinement. This is a measured generation cost, not evidence of a live menu slowdown or speedup. Per-run median/p95/max values are in the private readable report.

An initial native settings test encountered Win32 access-denied during an isolated atomic-file commit. The unchanged test passed on direct rerun; final build/check results are recorded separately. No permission bypass, live settings edit or production retry was added.

## Live acceptance still required

Use copies of DYNASTY-EPUFT checkpoints, preserving their originals. Start with offseason signing/draft and busy regular-season editions; verify National scroll/category latency, compact numbering and complete 224-pick Transactions coverage. Check the WMU-style upset voices and parent replies. At the preseason checkpoint, check poll/honor wording and account identities. At a recruiting checkpoint, check COT HS Game labeling, Recruit Notes/colors and monetary edits. Reopen old editions and confirm old text; refresh a newly published edition twice and confirm stability. Measure actual menu entry/scroll timing and FPS separately. Exact AIO/TeamBuilder/game-build compatibility remains pending live acceptance.

## Completed candidate checks

- 425 Node tests pass, including editorial, persistence, filtering, budgets, reply context, dates, grades/currency and Analytics model checks.
- Native source build passes: 288 native/JavaScript scope/category parity comparisons, 974 Analytics integration assertions, and the existing native Social Media, Fog, currency, grade, settings and ABI checks.
- Package verification passes: 154 payload files, 983 original standalone source files verified unchanged, all 16 module selections, 17 actual host/resource/path checks, nine Analytics binary ABI/build-pair/license checks, and helper/tamper/traversal checks. Deliberate changes to imported COT copies are declared in the baseline ledger.
- 116 installer cases pass. The versioned single-file installer passes 34 real GUI/engine checks, including bounded layouts/scaling, module selection, uninstall and rollback.
- 95 updater checks pass for exact 0.1.10 → 0.1.11 discovery/download/install/rollback using isolated fixture endpoints. Corrupt payloads, failure recovery and retained data are covered. This is not a claim of a published GitHub release.
- 15 release metadata/provenance/privacy checks pass. Public-source boundary scan and source whitespace checks pass. Live installation and publishing remain false.
