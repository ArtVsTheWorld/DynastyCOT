# Update 3 final cleanup — 2.0.10

## Live evidence and scope

The October 30 title-update candidate 2.0.9 completed a fresh Dynasty game, home28–away21. No current-session binding/runtime faults were observed. Actual mapped fullness rose from51% to94.4%, ended83.1%, and temporary halftime seats returned without refilling permanent departures. All48 native mixer overrides were retired at the final menu. The user confirmed unskipped touchdowns/takeaways consistently showed full-screen playback followed by a separate inset; hitches and duplicate reactions were not particularly noticed.

This cleanup retains native/DGA cheer overlap, intentional fourth-down-to-long-gain milestones, scoring upgrades, palette intensity rules, settings/defaults, and replay selection policy. It does not install or publish anything.

## Findings and changes

- **Forecast mistaken for guaranteed rain.** The launch option held precipitation severity2, dynamic weather enabled,20% rain chance; live weather and the user reported dry. Dynamic severity is not proof of rain at kickoff. A validated static-weather setting retains the existing severity deduction. Dynamic or unverified forecast settings do not receive an unconditional launch penalty. Native weather is unchanged; actual live weather continues to influence departures. This conservative policy also forgoes an initial weather penalty when a dynamic forecast eventually becomes wet; it does not recalculate peak attendance midgame.
- **Fourth-down fallback repeated an earned conversion.** Live crossing produced one reaction; official-result fallback treated the prior conversion as a supplemental accent and admitted it again. Fallback now respects the live conversion latch and the already recorded conversion for that play. A later long gain, takeaway, touchdown or distinct earned trigger remains eligible. This is DGA self-deduplication, not native-cheer suppression.
- **Away deep catch/gain overlap.** A main situational disappointment at the catch could precede a symmetric gain threshold on a later tick. Shared play coverage prevents a second equivalent ordinary negative counterpart. A verified later score can still upgrade once; a new play resets coverage.
- **Repeated seating work.** Successful native part resolutions are cached only within a single forced native traversal, bounded to32 identities and discarded at its end/fault. Natural callbacks and subsequent passes resolve fresh. Seat depths are computed once before sorting, with the same numeric values, stable tie rules and final pattern. Seat preparation runs once per dispatch, rather than once per controller. Renderer/mask identity, original empties, temporary/permanent ownership, retirement checks and historical replay targets remain authoritative.
- **Diagnostics.** Opening-drive admission is labeled separately from the normal critical-down gate. Eruption logs no longer promise an extra50% volume boost; the native bus mix still respects its configured total cap. Postgame carry is labeled eligible/retained separately from actual playing sound/camera states. Separate seat-preparation and mask-measurement timing counters isolate future spikes.

## Performance evidence and limits

The monitored 2.0.9 game recorded crowd-apply max152.595ms, with36/38 actual updates at least16ms; four replay-crowd checks exceeded16ms, max20.238ms. Forced native passes peaked12.845ms. These are wall-clock measurements, not FPS or proof of a visible hitch.

A private fixture runs the actual production capture/apply code against34393 authored seats,20 changing attendance targets and native callback substitutes. The initial comparison was126.0ms versus108.4ms for first capture/preparation; average subsequent updates3.03ms versus2.89ms. Host timing varies; this synthetic comparison is not a live-game speed claim. The cache regression proves103179 repeated resolution requests use three native lookups with identical part results. Full regression coverage must also pass before staging. Remaining genuine native traversals and initial section-pattern construction still have a cost.

## Verification

The full automatic native/managed regression build passed with warnings treated as errors, including 828 attendance simulations and 497 native configuration-reader cases. All 14 Python tests and 178/178 mapped-image byte/pointer checks passed. Direct old/new comparison preserved the exact rank order for 1,024, 8,000 and 34,393 seats at two seeds each. Targeted tests cover static/dynamic/unverified weather, conversion fallback deduplication with independent recovery eligibility, cross-tick negative coverage with a score upgrade, and bounded per-pass part-cache reset/fault behavior.

The isolated current-MMC ABI, metadata, embedded resources, extraction and storage check also passed. Review confirmed that native-cheer overlap logic, semantic palette, bindings, settings defaults and menu catalogue are unchanged. No additional release blocker was found in these checks.

## Final live smoke check

Install with game/MMC closed. Load a fresh Dynasty game and check the logged weather policy against actual conditions. Run several plays through an arrival update, a fourth-down conversion and an unskipped TD/takeaway; inspect the new timing breakdown. No new lengthy replay capture is required unless the smoke check fails. Prior 2.0.9 rendered replay acceptance remains recorded; it does not certify the newly staged binary.
