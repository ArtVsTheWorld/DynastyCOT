# Update 4 working plan

## Crowd-label research added November 5

[Crowd types and academy protection](CROWD-TYPES.md) records all 12 native
crowd labels. The three academy stadiums use student-tagged sections with
school-specific uniforms. The private 2.1.5 candidate protects military-tagged
spectators and students belonging to a verified military school, alongside the
band protection. This identity read is independent of school grades and applying
the revised peak formula. Ordinary students and standing-only spectators remain
subject to thinning. An academy-specific visual check is still required.

Working version: **2.1.3**, based on the published 2.0.35 hotfix. Feature milestones advance the middle number; fixes to a milestone use the final number. Existing settings and their reset markers are preserved. No Update 4 features are enabled merely by changing the version.

## First: make the attendance plan reach the correct seats

A supplied TeamBuilder report shows the revised launch calculation being skipped, followed by a fully occupied seating baseline. The native attendance reference is about 30% of capacity. The supplied save has poor school grades and an 0–4 record. A private calculation using the production policy and saved school cohort produces a much smaller expected crowd, not a sellout.

Treat these as separate observations: the launch plan was not applied, and native attendance does not match the measured seats. A changed stadium is relevant context, but is not yet a proven cause. Do not weaken Dynasty identity checks or multiply every crowd by the native attendance ratio: either can affect unsupported modes or thin already-correct crowds twice.

Next capture should record the actual setup-mode value, read success, witness age/generation, launch identity, plan application, stadium identity and first native crowd masks during one fresh TeamBuilder launch. Verify mode/identity through loading before changing the guard. Reconcile an authored/custom stadium with actual masks only after establishing which value owns visible occupancy. The existing retained-crowd floors are not the explanation for a full opening crowd.

## Keep the band in its seats

Current thinning ranks generic occupied positions and hides model parts. The verified mapping has positions and visibility masks, but no verified band-role classifier. A part bit identifies a render part; it is not a proven band-member ID.

Compare native renderer/asset/property identities for a band block and adjacent ordinary fans before changing masks. Prefer a stable role or asset marker over a hardcoded section location or team ID. If a marker is verified, exempt that role from DGA arrivals, temporary breaks and permanent departures while retaining native visibility. Do not create missing band members or override scripted band movements. Keep band counts separate from the general-fan occupancy denominator if the mapping includes them.

Live check: a paused wide replay showing the band and ordinary seats together, followed by an ordinary return to gameplay. Test both home and visiting bands, multiple venues, low crowd targets, replays and a new game before shipping.

## Mix home and visiting supporters

The current renderer control changes visibility, not fan allegiance or clothing palettes. Team colors are not enough to prove that an individual fan belongs to either team.

Identify native fan-role, appearance and palette data at renderer and instance level. Determine whether a palette can be selected per person, per group or only per renderer. Keep changes stable for the game and restore borrowed data at teardown. Use current native/TeamBuilder colors rather than school-ID color tables. Preserve band and student roles once identified.

Start with believable visiting blocks and smaller groups, then consider rivalry, opponent appeal and travel demand. Keep visual supporter shares separate from neutral-site classification and reaction ownership until those changes are explicitly designed and tested. A different shirt color must not silently reassign audio or gameplay home-field advantage.

## Calendar and school breaks

The supplied save exposes game month, day, weekday, season week and season year. Native live mappings still need verification. Use the scheduled game date, not the computer's date or a fixed week number across different seasons.

Proposed first tuning: a small early-season lift that does not duplicate the home-opener bonus, a modest Halloween effect, and a larger student-break effect only when the calendar basis is known. Actual school vacation calendars are not verified inputs. Do not describe a broad Thanksgiving window as an exact campus break schedule.

Keep major rivalries, senior day, postseason and compelling games from receiving a blind holiday penalty. Put new demand bonuses under a shared cap with existing event boosts. Tune these with simulations before selecting final percentages.

## Expectations and conference races

The save contains preseason and current AP ranks, conference wins/losses, conference standing, previous-year conference results and the remaining schedule. These are useful inputs, not proof of a mathematically clinched berth.

Freeze an expectations baseline before observing the season's results. Where a preseason snapshot is unavailable, label any reconstructed baseline as an estimate. Compare results with schedule difficulty and preseason strength rather than simply rewarding every .500 team. A low-expectation 3–3 team can gain interest without being treated as an elite program.

Add a bounded late-season conference-race boost when standings and remaining games support it. Verify each conference's championship format and tiebreakers before claiming a team can clinch or has been eliminated. Otherwise use a conservative race-interest category. Keep this separate from live comeback retention and existing current-AP support so the same success is not counted repeatedly.

## Other useful additions

- Per-game national coverage and Game of the Week interest: prior research found saved broadcast fields and launch descriptors, but live inputs and enum values still need validation before use
- Clear launch diagnostics showing whether turnout is DGA-planned or native fallback, and flagging disagreement between planned attendance and visible seats
- A simulation matrix combining weak/strong programs, changing venues, school breaks, rivalries, conference races and Default/Fuller profiles

## Delivery order

1. Diagnose and reproduce the TeamBuilder launch/seat discrepancy
2. Verify band classification and keep the band stable
3. Verify appearance controls and audition visiting supporter blocks
4. Add calendar and expectation inputs, then simulate before live tuning

Research exports, supplied saves and user logs stay in private Build folders. Do not install or publish a development build without a separate test/release step. No current live game, installed plugin, settings or save is changed by this research setup.

## Audience model implemented in development source

Peak attendance now estimates spectators before comparing with venue capacity. The fixed program foundation uses a 2025 attendance/five-year reference for known native schools, or a robust peer estimate for TeamBuilder/unmapped programs. The approved weights are prestige 40%, atmosphere 30%, three completed seasons 20%, and AP 10%. The most recent completed season receives 50% of the results weight, then 30% and 20%.

Only real in-dynasty completed seasons count for TeamBuilder; missing history is omitted rather than imputed as .500. The initial peer history supplies an expectation reference for future results, not fabricated records. Foundations are keyed by native Franchise.LeagueID and program row/identity, stored under the plugin's own History folder, and reloaded before native hooks are installed. Repeated launches do not compound growth. Audience differences are computed from current attributes against the same frozen reference. Advancing a year alone adds nothing.

Existing matchup event ranges, shared cap, same-state/service-academy/senior/opening/night rules, hard-to-fill discounts, protected showcase overrides, Fuller option, and arrival functions are retained. Ordinary form/opponent strengths use a fixed audience reference; weather and small variation apply to fans. The old capacity-based breakout target is replaced by bounded temporary fan interest, and the old peak-percentage floor cannot create phantom spectators. The independent departure floor still does not refill crowds. Native postseason, unsupported modes and Midgame Saves retain their established paths.

The implementation follows StatManager.LeagueHistoryManager -> LeagueHistoryManager.Dynasty to obtain the active Franchise.LeagueID, verifies that Franchise.League and StatManager.LeagueData identify the same record, then uses the borrowed-record ABI for TeamSeasonStats. This replaces the 2.1.0/2.1.1 named-table/FIRST lookup that live diagnostics proved unavailable. New enum labels and ordinals are checked before use. Missing identity, unsupported history layout or corrupt foundation storage preserves EA turnout. Foundation writes occur on the worker, not audio/camera/crowd callbacks. Native bindings passed offline byte/enum evidence and isolated ABI fixtures; a live fresh-launch/restart/second-dynasty check remains required.

Production-policy simulations cover six-season improvement, flat support, decline, stadium expansion and small/large venues, with book serialization/reload each year and no future-season record leakage. Grade/rank trajectories are explicit hypothetical inputs, not predictions of EA progression. Full private artifacts stay under Build/Update4Implementation.

## Opening and sound adjustments in development source

The near-sellout first-possession discount now additionally requires a big game, any rivalry, or at least 13,000 stored positions. It still needs a verified 90% peak plan. Actual big games retain their existing 50% opening route. Ordinary small venues use their normal size/night requirement, not an automatic reduced gate from a high turnout forecast. First-drive tracking, normal night adjustments and the sparse sound-only welcome are retained.

Opening-drive status now distinguishes the normal size/night requirement from an actual special opening discount. The published status classification is captured with the live admission decision, rather than inferred later from a potentially different matchup. The protected showcase list now contains 24 native rivalry events. The 16 added events use the existing 70–80% kickoff range and full peak; the original eight retain their individual ranges. Native rivalry-event identifiers select these rules, including TeamBuilder replacements that retain the event; team slots never select them.

## Audience-foundation audit — approved and implemented in 2.1.4

The November 4 audit under `Build/AudienceAudit-20261104` compares 134 calibrated native programs against the 2025 final/five-year attendance data and the partial 2026 Week 9 table. All 138 saved programs are simulated across seven fixture conditions and baseline/improving/declining states, using their actual saved venue capacities. The production audience/calibration/turnout source hashes remain unchanged during this research.

The candidate removes the one-time peer adjustment from known native schools' own 50/50 attendance reference. Nebraska's foundation changes from about 73,353 to 86,861 spectators. Existing 40/30/20/10 dynasty response and exponent remain; TeamBuilder and unmapped peer estimates stay unchanged. The candidate also softens negative opponent appeal by up to 75%, using a smooth retention proxy derived from the lower of 2025/five-year attendance relative to the frozen reported 2025 capacity. Protection fades as current audience falls below its reference. There is no blue-blood list, new peak percentage floor or current-venue-based loyalty growth. Custom replacements do not inherit a native school's retention proxy.

Across the frozen ordinary-game cohort, mean projected peak changes from 76.59% to 77.03%, while sellouts fall from 28 to 15. Nebraska's ordinary FCS example changes from 72.9% to 96.3%; the hypothetical sustained-decline FCS case still falls to 57.6%. Audience-only mean fullness error against the annual data changes from 4.80 to 3.12 points for 2025 and 6.75 to 5.53 for 2026. These are baseline checks, not per-game prediction accuracy. The proposed retention band and relief strength are tuning choices, not fitted opponent effects; annual averages cannot establish exact opponent elasticity.

The report, full 2,898-row scenario CSV, foundation audit, native rivalry evidence and reproducible harness are in that folder. The user approved the proposal. 2.1.4 now uses raw native attendance references, derives turnout retention from a frozen evidence table, and applies opponent relief with the correct home/neutral audience ownership. Rivalry bonuses and current form retain their strengths. Production results match the approved candidate across all 2,898 scenarios.

Format-1 foundation files migrate to format 2 using each known native school's old-to-new calibration ratio once. Frozen feature references, identities and custom/peer entries are preserved. A copy of the original file is required before the worker can write the upgrade. Unreadable files and backup failures preserve EA turnout. Format-2 reloads cannot repeat the correction. No installed file or game save has been modified during development.

The arrival target and visible catch-up now have separate notices; completion waits for settled temporary gaps and a successful visibility update. Permanent exits are excluded from the remaining-arrivals readout. The added hostile-opening response requests the largest generic cheer for a positive result on the actual first away defensive snap, including incompletions and gains of two yards or less. It coalesces with an existing official-result cheer, excludes conversions/scores and stronger takeaway routes, and adds no shake or gain. Fullness, ownership, pause/replay, simulation and neutral exclusions remain.

## Band and home/away color research started

Private evidence is under `Build/BandColorResearch-20261104`. The current native agent-variation asset and SDK agree on eight musician types, IDs 1–8, distinct from default spectators, students, military and always-standing supporters. Authored point-cloud maps contain per-seat AgentType, HomeAwayNeutral/HomeAwayUltra/HomeAwayFiftyFifty and ten color-design maps. Four checked assets identify 274 band positions at Kennesaw, 121 at Nebraska, 151 at Michigan and 377 at Texas A&M. These are authored positions, not verified visible headcounts.

In the user's paused Texas Tech replay, read-only inspection matched all 35,306 live seat coordinates exactly to the authored point cloud, which marks 314 band positions. Local matching-build symbols located the renderer without installing new hooks or writing memory. No packed live musician/affiliation field has yet been proven by consumer code; a sampled correlation search found no supported simple byte/nibble mapping. No band exemption, density preview or recoloring has been applied.

Next, trace how authored AgentType and affiliation maps reach the live agent records, verify band visibility against the fixed replay view, then audition a reversible band-preservation candidate and a modest mixed-color candidate separately. Protect only verified musicians, retain native hidden/culling state and replay/restoration ownership, and keep bands out of fan-demand inflation. Color-design patterns and school palettes are separate from actual home/away affiliation; neutral mixing parameters must not be described as direct away-fan percentages until tested. Stadium traditions, TeamBuilder palettes and neutral-site balance need their own visual checks before these features are enabled.

Routine semantic reaction selection now starts at the mildest verified variant; important downs use the medium variant, and major gains/results retain strong variants. Removed the shared 75%-of-selections forced-major preference. Ordinary third-down stop result sounds use situation-based intensity; fourth-down/conversion/goal-line stops retain major sounds. Trucks, skill highlights, takeaways, scores, eruptions, pass anticipation and opening welcome/lineup audio keep their existing specific policies. Away showboating retains strong boos, still subject to the occupied-seat ceiling. The 80 usable selectors remain reachable. No trigger, crowd-mix curve, native voice or camera strength is removed or globally reduced.

These are source-level development changes only; listening verification is outstanding and no build has been installed or published.

## First live audience test and diagnostic follow-up

The East Point versus FCS test on 2.1.0 did not apply the fan-count formula. Existing matchup capture succeeded, but the added dynasty/history capture failed and preserved EA turnout. Actual mapped fullness rose from 16% to 29% as arrivals completed; that is fallback coverage, not validation of the new model. No audience foundation was created. Retained gameplay logs end early in Q2.

2.1.1 reports the exact failed read, including home/away scope, dynasty year, program row, TeamBuilder flag and history length when available. Database/table/FIRST lookup failures, unavailable fields and invalid inputs have distinct messages. Status distinguishes verified field labels from a successful native read and an applied turnout plan. Year 0 TeamBuilder capture no longer requires the inherited history array, because none of those seasons are eligible. Native programs and later custom seasons retain their history checks. Formula weights, dynasty identity validation and eligible-history validation are unchanged. The failure's exact live cause remains unconfirmed; this is a private diagnostic candidate, not a verified attendance fix.

The next test only requires launching a fresh regular-season Dynasty matchup and pausing at opening kickoff. Check the new capture result before asking for gameplay, then verify foundation persistence and visible attendance once the native read succeeds.

## Single-DLL install and direct runtime loading, 2.1.2

The package now needs only DynamicGamedayAtmosphere.dll copied into Plugins. The managed adapter embeds the native runtime and handles its loading directly; no separate loader EXE or matching runtime folder must be copied. Existing process readiness, once-per-process claims, target identity, module/path verification, loader-owner relocation, conflict detection, pending-operation lifetime and failure diagnostics remain.

Generated native files live under MMC/DGA-Runtime/Runtime-version, outside MMC's recursive Plugins DLL scan. Native path resolution explicitly maps that location back to Plugins/DynamicGamedayAtmosphere, retaining INIs, logs, attendance history, audience foundations, diagnostics and other-plugin bridge paths. Exact old DGA runtime/helper names are archived outside the scan only after the game is closed. Other files are left in place. The first upgrade scan can still log an old DLL before cleanup; a later restart should not rediscover it.

This is a private Update 4 candidate. Direct loading and restart behavior still need live verification. No reduction in antivirus detections is claimed; the underlying Windows loading operations remain. The separate-loader control can still be built explicitly for comparison.

## Active dynasty identity lookup, 2.1.3

The 2.1.1 East Point-FCS live check reached the new audience path but its database lookup returned no Franchise table. 2.1.3 removes that query. It follows relationships owned by the launch's verified statistics manager, through its league-history manager to the active dynasty. The owning league must match the statistics manager's league by underlying table and row, so separate interface wrappers remain valid while another loaded dynasty is rejected. No filename, team ID, timestamp or default ID is used as a substitute for native dynasty identity.

New relationship field names/ordinals are checked against the verified title-update image. Missing links, invalid LeagueID and league mismatches preserve EA attendance and report the failing step. Borrowed records are released on every success/failure path. Foundation format, weights, event/arrival policy and settings reset markers are unchanged. The single-DLL direct loader remains the default. A successful new-game launch and persisted audience foundation still require live verification after installation.
