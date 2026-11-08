# Dynasty COT 0.1.6-private: release-candidate verification

Public test prerelease by Ace. Live acceptance remains pending. This document describes one bounded candidate, not a stable compatibility certification.

## Audit and finite implementation plan

The comparison baseline is the unmodified 0.1.5-private source/package. It was frozen before candidate edits with SHA-256 manifests. The original 0.1.5 installer SHA-256 is `66756185f1c833317866a1ab932838a02de9c72710aa54fff7a2593920cf6c61`; ZIP SHA-256 is `9ac883e47cdba4bb9e20d8745c59741e63355800a01342ade99dc38910ba6bdc`.

31 DYNASTY-EPUFT checkpoint files (299,056,411 bytes) were copied and hash-verified for read-only audit. All three screenshots, the supplied Tankathon HTM and Ace's CFB Toolkit source reference were located. Original saves and the live installation are not build inputs or output destinations. Raw checkpoints, captures, histories and private research fixtures remain local and are excluded from public source and release assets.

The persisted MMC pack has AIO Dynasty 1.4.1 followed by TeamBuilder Compatibility 1.1.0; DARK v4 and the dollar-icon mod are also enabled. Persisted configuration is evidence of the selected pack, not proof of what the running game loaded.

One systematic content audit and one finite 464-alternative expansion were authorized: 192 casual, 48 historical, 80 seasonal, 96 upset/reply and 48 recruit game/film alternatives. Iteration is limited to correcting failed requirements/checks. The four modules remain independently selectable. Source ownership was split between Beyond the Field, installer/release infrastructure and lead integration; only the lead publishes.

## Read on the full-season tweet audit

The actual published run contained **32 editions, 960 default cards and 54,505 stored cards**. The full published year and the regenerated same-input comparison are separate evidence sets. All 562 baseline families were inventoried; the candidate has 585 families and 7,824 alternatives.

The feed covered the season well, but often sounded like a newswire instead of a college-football community. Only one exact default-feed body repeated (twice), and none used “changes the conversation.” The larger problems were repeated structures and angles, generic reactions, and attention bunching around recruiting. Signed spotlights (55) and commitments (40) were the two most displayed families. Beat reporters supplied 351 of 960 default cards; there were 107 displayed replies.

The WMU result is the clearest example: verified unranked W. Michigan won 31–26 at #14 Michigan. The existing result report was accurate, but a generic reaction undersold its significance. New winning/losing/rival/analyst perspectives respond to the verified score, ranking and program gap. An actual captured rivalry is required for rival-specific reactions. Existing published wording and account identities are retained.

My assessment: the most useful changes are contextual reactions, parent-aware filtering/replies, and access to stored news. The finite casual pool gives quieter editions more personality. Selection remains the largest content weakness, especially in National and repeated recruiting angles; the template count alone does not demonstrate success. See the [full audit](Modules/DynastyEnhancer/Docs/RC016-REVIEW.md) and [representative samples](Modules/DynastyEnhancer/Docs/RC016-SAMPLES.md).

## Findings and behavior

- Filtering previously operated on an already curated subset, so a broad category could miss eligible narrower-scope material. Reply matching could also lose a parent from another voice category.
- Category rendering could instantiate thousands of native cards (an inherited test rendered 9,300 Recruiting cards). Bounded pages retain stored content and provide incremental access through the existing week selector.
- Date-based holiday admission previously depended on an upcoming game exactly on the date, missing off-weeks. Date mapping must use supported calendar anchors, with no invented date when context is absent.
- The supplied draft snapshot contains projected/reference pick facts. The first requested offseason checkpoint is native OffSeason week 2, stage 3, CurrentYear 0 / season 2026. Draft reporting is separate from native outcomes and must retain that distinction.
- Visiting Fan Strength is an existing Gameday Atmosphere INI preference, default 100, range 0–200. It adjusts visitor odds only for newly generated plans; default preserves the model exactly. Existing fixture history, neutral native plans and protected crowd groups remain unchanged.

## Verification results

The complete source build and final-candidate checks passed. The following results use the finalized 0.1.6-private ZIP and installer; earlier baseline-only fixture results are not substituted.

| Area | Candidate evidence | Remaining acceptance |
|---|---|---|
| Beyond the Field content, voice and repetition | 406 Node checks passed in the final complete regression suite; final recorded comparison below | Native reading, timing and contextual judgment |
| Category/scope/reply/paging behavior | Native Social Media paging/filter tests passed; all 224 draft picks accessible | Mouse/controller focus and native view changes |
| Recruit ledger, trait uncertainty and isolation | Native trait availability verified; weak-noise overlap, record consistency, identity/isolation, immutable blobs and legacy-writer sidecar recovery checks passed | In-game season transitions |
| Calendar and first-offseason draft mapping | Actual/synthetic date, rollover, neighboring-week and first-offseason conservation checks passed; 257 reference slots map 224 native player outcomes | Native displayed labels and next-offseason behavior |
| Notes/board colors and compact dollar redraws | Complete integrated Fog/UI build passed, including 231 native/owned frame shapes, 13 AIO Notes and 19 currency siblings | Exact active overhaul UI, edits and reused widgets |
| Visitor slider | 201,000 seeded strength comparisons; exact default, bounds, neutral, INI reload and saved/new fixture cases passed | New-matchup visual comparison and monitor/UI navigation |
| Gameday native/managed regressions | Full source production build passed, including scope, ownership, audio, crowd protection and persistence checks | Live gameplay and exact overhaul combinations |
| Installer | Final candidate: 29 GUI/engine checks, 100/125/150% native renders, 1080/1152 working-area bounds, footer/actions visible and all module descriptions reachable | Other monitor DPI/accessibility configurations |
| Manual updater and rollback | Final candidate: 95 checks, cross-version 0.1.5 update/rollback; post-publication anonymous-path result is recorded in the release evidence asset | User's chosen live install/rollback |
| Release metadata/privacy | 13 isolated metadata/provenance/privacy checks passed; public source and sanitized export checks | Prepared local SDK/private-fixture inputs are required for rebuilding |

## Measured repetition result

The same-event comparison covers 91 recorded editions across three Dynasties (31, 31 and 29 consecutive publication stages). The metric compares each visible card with four preceding editions and earlier cards in the same view, counting equal normalized template skeletons or at least 70% overlap of unique four-word phrases. Names and numbers do not create diversity.

| View | Before | After | Reduction |
|---|---:|---:|---:|
| For You | 240 / 2,729 | 187 / 2,729 | 22.1% |
| Team | 111 / 1,358 | 61 / 1,358 | 45.0% |
| Conference | 933 / 4,889 | 646 / 4,889 | 30.8% |
| National | 2,376 / 8,193 | 2,131 / 8,193 | 10.3% |

**The 50% target was not achieved.** National still has 831 duplicate cases with an audited unused alternative; 1,300 had none in that view's lookback. Adjacent near-duplicates changed 2→1 in For You, 1→3 in Team, 1→0 in Conference and 21→8 in National. The Team adjacency regression is another known selection weakness. Essential primary reports are retained; optional personal reactions are assessed separately. This replay uses retained event facts; it does not reconstruct missing historical raw inputs or prove every new event family displayed. Calendar, recruit simulation, minimal-data and long-name cases have separate checks.

Required-news conservation was checked separately from optional personal reactions. Both engines retained **50,428 / 50,428 required events and all 3,612 primary award reports**. An initially overbroad checker flagged optional player thank-you posts; exact same-player/award/school/season analyst and official counterparts were retained. Three additional regression tests cover that distinction. No runtime coverage code was changed to make the audit pass.

## Performance limits

A same-input visitor-planning comparison used 64 matchup fixtures, 500,000 plans per trial, seven measured trials after warmup and the same optimized compiler. Median baseline was 0.285985 microseconds/plan, candidate 0.302089; result checksums matched exactly. The ranges overlapped during background development work. This is a small offline cost per generated plan, not a live-frame measurement or proof of menu responsiveness.

The final story-renderer comparison ran sequentially after other heavy checks stopped, against identical 91-edition retained-event inputs:

| Renderer timing | 0.1.5 baseline | Candidate |
|---|---:|---:|
| Mean per edition | 174.9 ms | 277.6 ms |
| Median | 129.5 ms | 197.3 ms |
| 95th percentile | 398.0 ms | 640.4 ms |
| Maximum | 435.5 ms | 668.8 ms |

**Mean rendering cost increased 58.7%.** This excludes the exhaustive alternative audit, capture/file I/O, native widget construction and live FPS. The added variety work costs CPU; it is not described as a generation-speed improvement. The busy native category fixture separately drops from 9,300 simultaneous cards to 100 per page (98.9% fewer), retaining access to the remaining eligible cards.

Social generation and menu-fixture timings are distinct from native FPS. Previous PresentMon probes did not produce usable live frame-time evidence. No claim of unchanged live menu FPS is made.

## Distribution and safety boundary

The installer contains the exact verified source-built ZIP. Updates require separate explicit check, selection, download and install actions. GitHub origin, size/SHA-256, package manifest and payload verification precede transactional installation. Checksums detect corruption; they are not an independent publisher signature. Older versions and transaction backups remain available; rollback retains later user data and refuses managed-file conflicts.

Internal IDs, data locations, native scope checks, fingerprints, hook ownership and helper verification are retained. Generated native UI adapters contain authored custom programs and zero placeholders; native bodies are verified and copied at runtime. Raw game captures and private fixtures are excluded. Required third-party notices are retained.

See the [prioritized DYNASTY-EPUFT checklist](LIVE-ACCEPTANCE.md) and [update/rollback instructions](docs/release/README.md). 31 checkpoint originals and copies, 201 live plugin/runtime/config payloads and both 0.1.5 release assets were rehashed unchanged. The finalizer also verified 978 original standalone source files unchanged. No source/test startup pass is described as live-game compatibility.

## How to test new publications safely

Checkpoint copies retain their LeagueID and therefore share the already published history. Reopening the completed 2026 year intentionally retains its old wording and draft board. UI/filter/paging fixes still apply to those stored posts. Use `DYNASTY-EPUFTPRESZN2027` and advance into unpublished editions to inspect new voices, seasonal posts and recruit records. Testing the supplied first-offseason order requires an unpublished 2026/CurrentYear0 first offseason, such as a fresh test Dynasty with a new LeagueID. Do not erase existing history as a testing shortcut.

The named manual checkpoints are best for UI regression: `RIVALRYWK13` for recruiting Notes/board and Owen Lineberger, `CCGWEEK` or `BOWLWK1` for coach offers, `EOSRECAP` for NIL editing, and `OFFSZNREC1DRAFT` for preserved native draft outcomes. All remain live acceptance work.

## Build prerequisites and remaining external review

The release was built locally from source with the prepared MMC SDK and private research fixtures. The manual GitHub workflow is provided for a prepared, access-controlled Windows runner; a public checkout alone cannot run the omitted private-fixture checks. No SDK, capture, save, generated publication archive or credential is shipped in public source. The candidate's remaining live-game/UI/performance checks are explicit user acceptance work, not successful offline results.
