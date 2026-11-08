# Beyond the Field — 0.1.6 candidate review

This is one bounded review and one authored expansion. It is not a claim of live-game compatibility or completion of the 50% repetition target.

## What was reviewed

The published season was read separately from regenerated comparisons. The completed checkpoint Dynasty has **32 saved editions from 2026 preseason through 2027 preseason**, **960 For You cards**, and **54,505 stored cards** across its editions. Those stored cards are not all on screen at once. All existing published archives remained read-only during the audit.

The review inventoried **every one of the 562 merged baseline post/reply families**, including required facts, voice/role conditions, distinct structural forms, repeated endings, and observed eligibility. The candidate has 585 families and 7,824 alternatives, including exactly **464 newly authored alternatives**. The family inventory and raw season evidence are private local verification artifacts, excluded from the public repository and package.

The same-input comparison covers **91 recorded editions across three separate Dynasties**: 31, 31, and 29 consecutive recorded publication stages, including preseason, regular season, bowls, championship, and offseason. This replays retained event facts; it does not invent missing historical raw snapshots. Upcoming-game, some carryover, and other families requiring missing archived inputs cannot be reconstructed by that corpus. A separate fictional comparison exercises 31 consecutive stages for each of three program brand grades, including sparse editions. Calendar, simulated recruiting, long-name, and minimal-data cases have separate checks.

## Main whole-year findings

1. **The feed already has substantial news coverage.** Results, awards, commitments, injuries, portal movement, and offseason continuity all appear. The central weakness is how facts are framed and selected, rather than a lack of raw templates.
2. **Recruiting attention bunches up.** Signed spotlights and commitments are the two most frequently displayed families. Several kinds of welcome/fit/decision posts can describe one recruitment without contributing much new perspective.
3. **Important upsets need contrasting reactions.** The stored Week 2 event confirms unranked W. Michigan beating **#14 Michigan, 31–26**. The original coverage recognizes the result, but a generic fan response such as “That's a result worth sitting with for a minute” undersells it. The candidate adds losing-side frustration, an actual rival's reaction when a captured rivalry exists, and scrutiny of the verified program-strength disparity.
4. **Accounts have distinct identities but can still share a voice.** Beat reporters supplied 351 of 960 displayed cards; analysts 162, official accounts 159, fans 122, players 79, recruits 51, model 21, CFP 12, draft 3. There were 107 displayed replies. Persistent account profiles exist; the expansion gives them more substantive opinions and interests to choose from.
5. **Exact repetition is less common than structural repetition.** Only one exact default-feed body repeated across the reviewed season, twice. There were zero displayed occurrences of “changes the conversation.” Similar openings, short report-plus-generic-reaction structures, and repeated angles remain the more important problem. Removing a catchphrase alone would not solve it.
6. **Stored and displayed content were being confused.** Every reviewed edition had National candidates, but a dedicated category could falsely appear empty because replies were considered only after their parent's category matched. For You categories were also searching a pool already reduced by default curation. Narrower scopes can legitimately contain local news absent nationally; that intentional difference remains.
7. **Dedicated categories could build thousands of native cards at once.** The old native regression fixture rendered 9,300 Recruiting cards simultaneously. That is unnecessary menu work, even though the stored catalog is valid.

## The 20 most displayed families

Counts are from the actual 960-card published season, not regenerated candidates. Alternatives are the merged baseline → candidate counts; a larger count is not itself evidence of improved display diversity.

| Family | Displayed | Alternatives |
|---|---:|---:|
| cycle.signed-spotlight | 55 | 32 → 32 |
| roster.commit | 40 | 41 → 41 |
| world.upset | 38 | 40 → 48 |
| world.award | 37 | 40 → 40 |
| team.upcoming | 30 | 41 → 41 |
| injury.report | 22 | 42 → 42 |
| reply.fan.upset | 21 | 61 → 69 |
| cycle.team-welcome | 18 | 38 → 38 |
| model.line | 18 | 38 → 38 |
| late.rivalry-team | 16 | 41 → 41 |
| roster.underdog | 16 | 10 → 10 |
| portal.unsigned-local | 15 | 34 → 34 |
| matchup.receiver-secondary-recap | 14 | 10 → 10 |
| roster.unsigned | 14 | 10 → 10 |
| carry.training | 13 | 10 → 10 |
| narrative.game | 13 | 30 → 30 |
| buzz.team-pregame | 12 | 10 → 10 |
| portal.entrant | 12 | 40 → 40 |
| reply.fan.award | 12 | 20 → 28 |
| roster.interest | 12 | 40 → 40 |

Common event families already have 30–61 alternatives; several more specialized families have only ten. Both eligible-alternative exhaustion and selection mistakes occur. The private inventory records per-family minimum/maximum/average eligible counts, exhausted calls, and unobserved families. “No replayable historical input” is not recorded as a passed content check.

## One finite expansion

| Pool | Alternatives | Distribution |
|---|---:|---|
| Casual opinions | 192 | Eight topics ×24: uniforms, traditions, tailgating, campus, conference pride, rivalries, football arguments, optimism. 96 fan,48 player/recruit,24 analyst,24 local/official. |
| Historical references | 48 | Twelve verified pre-2026 moments ×4:24 fan,12 analyst,12 reporter. Primary-source provenance is in `Content/Runtime/archive-provenance.json`. |
| Seasonal | 80 | Halloween, Rivalry Week, Thanksgiving, Christmas, New Year:16 each, with eight fan/four official/four personal alternatives. |
| Contextual reactions | 96 | Twelve upset/reply families ×8, including winning/losing/rival perspectives and cautious follow-up. |
| Recruit game/film | 48 | Six position groups ×8. Kicker/punter alternatives have separate guards. |

Casual and archive content is low priority and bounded to a small number of candidates; it shares the existing default editorial budget. Busy 30-card default feeds do not receive extra cards. Sparse editions can use previously empty space. Facts remain required template inputs; missing context does not permit invented interviews, injuries, transactions, or statistics.

Recruit development traits are available in the verified native Player schema and actual prospective-player records. The normal Social Media capture now reads that field transiently. No native high-school game-stat table is exposed by the verified adapter, so these posts are visibly labeled **“Modeled HS game/film”**. A separate immutable record stores each modeled appearance by Dynasty, season, prospect identity, and week. Nothing writes native ratings, traits, or game stats. Trait effects are at most three percentage points on posting probability and 1.8% on performance, with independent large noise and overlapping ranges. Unknown traits use the neutral model. Tests check broad overlap, identity recycling, statistical consistency, no trait leak, and repeatability.

The modeled ledger is retained in the index and an atomic separately scoped `history.recruit-games.json` sidecar. An older release may drop unknown index fields; the sidecar survives and restores these records after a later upgrade. Existing history paths and immutable feed blobs are retained.

## Selection, filtering and performance changes

- Repetition memory distinguishes actually displayed cards from the much larger hidden CPU pool. Hidden candidates no longer consume every recently seen structure merely because thousands were generated.
- Near-shape checks span families and authors, using an inverted four-word phrase index. A bounded second pass refines wording for likely visible events, keeping all stored news and existing published wording.
- Fan/player/recruit category matches can surface a matching reply with its real parent as context. Parent matches retain their conversation.
- For You category filtering uses its eligible scope pool before the display limit, rather than the already curated 30-card window.
- Native pages retain 30 For You /100 National /60 Conference /25 Team routine-card limits, including categories. Existing guaranteed commitment/user-draft exceptions remain. All 224 draft selections are accessible in pick order across pages.
- Additional pages use the existing publication dropdown. Only the selected edition/category/scope's current/previous/next page entries are added: at most three extra dropdown rows, not a weeks ×pages ×scopes product.
- Card stamps distinguish shown count, eligible count, and page. An empty category says no matching stories in that edition, rather than claiming no stored news exists.

**The 50% structural near-duplicate target was not reached.** The final sequential quiescent comparison produced:

| Default view | Before duplicates/cards | After duplicates/cards | Reduction in duplicate count |
|---|---:|---:|---:|
| For You |240/2,729|187/2,729|22.1%|
| My Team |111/1,358|61/1,358|45.0%|
| Conference |933/4,889|646/4,889|30.8%|
| National |2,376/8,193|2,131/8,193|10.3%|

Metric: each visible card is compared with the preceding four editions plus earlier cards in the same edition/view. A duplicate has the same normalized template skeleton or at least70% overlap of unique four-word phrases, measured against the larger phrase set. Slots, names, and numbers cannot create diversity. The original metric was retained throughout the corrections.

There are still avoidable choices:831 National duplicates in that measured run had an audited unused alternative;1,300 did not have such an alternative in that view's lookback. Remaining duplicate counts include important factual reports that must stay visible. The bounded task stops with these limitations documented instead of adding templates indefinitely or deleting news to improve a score.

The measurement above is a recorded-fact replay, separate from the actual published season and from live UI performance. **Generation is slower in this candidate.** The final sequential run over identical91-edition inputs, excluding exhaustive alternative-audit callbacks, measured:

| Generation per edition | Baseline | Candidate |
|---|---:|---:|
| Mean |174.9ms|277.6ms|
| Median |129.5ms|197.3ms|
| 95th percentile |398.0ms|640.4ms|
| Maximum |435.5ms|668.8ms|

Mean generation time increased58.7%, about103ms per edition in this corpus. The visible-event refinement and broader repetition checks have a real CPU cost. These measurements exclude live capture, file IO, process startup, and game menu/FPS behavior. The native allocation improvement is separately tested:9,300 eligible Recruiting entries now instantiate100 cards on the National page, with the rest accessible. Fewer native cards do not erase the generation regression. Actual menu opening time, controller responsiveness, and FPS still require a live AIO-combination review.

Required coverage is conserved: **50,428/50,428 required event reports** and **3,612/3,612 primary award reports** are generated and published in both engines'91-edition corpus. The initial audit incorrectly counted optional player award thank-yous as additional required award news. All five candidate omissions were optional acknowledgments; each corresponding award retained both its same-player/same-award analyst report and official announcement. The audit now distinguishes that exact case, while checking every primary award report and treating a player's award post without an exact counterpart as required. No runtime selection rule was loosened and no award report was removed to repair the audit. Regression tests cover multiple distinct awards for one player and intentional trimming of a personal acknowledgment under a full flavor budget.

## Dates, draft and native UI evidence

Calendar holidays use the Dynasty year and valid captured month/day ranges, not fixed bowl week guesses. The captured bowl `DayOfWeek` labels can be stale; disagreement is diagnosed rather than rejecting valid dates. Bowl editions can span multiple calendar weeks. Regular editions may also use the previous edition's last supported game date to cover holidays between games, provided dates are ordered and the complete neighboring interval is no more than nine days. Unknown, overlapping, or distant neighbors do not invent an anchor. A 2027 Sunday Halloween between Saturday October30 and Tuesday November2 is covered by a dedicated regression fixture. Supported schedule anchors map Halloween to October31, Thanksgiving to November26 in2026, Christmas to December25, and New Year to December31/January1 with a single approaching-year identity. Displayed Week13 is explicitly Rivalry Week. Missing/invalid date context suppresses date-dependent posts; same-week refresh and prior-publication checks prevent duplicate bursts.

On the reviewed schedule, Christmas falls in native BowlSeason1/week17 and New Year in BowlSeason2/week18. These are results of that schedule's dates, not hard-coded stage numbers. The Christmas checkpoint was audited directly; the New Year mapping was exercised using the same captured future schedule anchors and synthetic stage context, not claimed as a live observation at that stage.

The supplied Tankathon HTML contains **257 ordered picks**. Its SHA-256 is `334b409bad28e1c6b69acc6c5823e9315f51556b402a5799a336baae9c729494`. It is a supplied2027 reference, **not asserted to be a finalized historical draft order**. The supported game path exposes224 players in seven32-player native rounds. Reporting uses the first32 supplied team entries per round, preserving trades/repeated destinations;33 extra slots do not create players. Native player inclusion and native round outcomes remain unchanged. An already published board remains frozen. Later offseasons retain the existing deterministic model.

The first requested offseason is verified as CurrentSeasonYear2026 /CurrentYear0 /native OffSeason week2 /stage3, corresponding to the user's displayed recruiting Offseason Week1. That narrow gate is tested. The change affects COT's modeled draft destinations/presentation/reporting, not native NFL simulation or native player records.

Read-only installed logs and exact source inspection established two AIO causes: the Notes bank's navigation callbacks moved; and currency compatibility omitted unchanged callback siblings even when their ordinary whole-asset bank could no longer validate. Independent AIO Notes/color programs and complete currency sibling banks retain exact frame/body/pointer checks. Scholarship editing now also repairs its monetary label after the native input cascade. Coach-offer text clones its own native font with a width bound; numerical budgets and adjacent hours are retained.

The final module Node suite passes406/406 tests. Automated checks cover first render, numeric edits, deferred resets, reused widgets, Off gates, exact AIO callback operands, source relocation at three asset bases, history isolation, calendar rollover, draft conservation, full accessible paging, deterministic refresh, primary award conservation, and text bounds. They do **not** establish that every live AIO screen currently binds the expected widget or that all menu colors visually match native focus states.

## Reproduction and remaining acceptance

From the module directory: `node --test Tests/*.test.mjs` and `python Tests/rc016_native_ui_test.py`. Native builds invoke the UI regression gate. `Source/build.ps1` runs native Social Media compatibility/paging checks. `Source/build-recruiting-fog.ps1` runs native fog/UI checks with its existing prototype-name contract. These commands never patch the live game by default.

Private same-input comparison commands, from the integration root, sequentially after all other heavy work:

```powershell
node --max-old-space-size=8192 Modules/DynastyEnhancer/Tests/SocialVariety/replay.mjs Build/RC016/Baseline/Modules/DynastyEnhancer Build/RC016/BeyondTheField/recorded.json Build/RC016/BeyondTheField/before-quiescent.json
node --max-old-space-size=8192 Modules/DynastyEnhancer/Tests/SocialVariety/replay.mjs Modules/DynastyEnhancer Build/RC016/BeyondTheField/recorded.json Build/RC016/BeyondTheField/after-quiescent.json
```

Prioritized live acceptance: AIO recruiting Notes/board colors; Current NIL after keyboard/controller edits and delayed redraw; offer wallet beside hours; coach-offer amounts within bounds; page navigation across categories/scopes/editions; Christmas/New Year and refresh; already published wording after upgrade; modeled recruit identity/history after rollback; menu opening time and FPS with equivalent saves/options.

Remaining content weaknesses include generic reactions still eligible in older families, some repeated offseason recruiting angles, awkward school-name/nickname fallbacks when an archive lacks aliases, and National structures that exhaust their useful alternatives. See [the readable sample](RC016-SAMPLES.md). No additional content expansion is hidden in this handoff.
