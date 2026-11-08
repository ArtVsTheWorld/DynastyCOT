# Update 3 — regular-season night and scoring engagement

Candidate 1.98.0 implements the requested night atmosphere/departure additions and lower-scoring versus higher-scoring departure pressure. It also consolidates the current testing checklist; older validation notes are explicitly archived. No installation or publication is performed.

## Scope

For a verified stadium-local night kickoff (19:00 or later), regular-season atmosphere admission requirements are multiplied by 0.90 with an absolute measured-fullness floor of 50%. An 80% threshold becomes 72%; 55% becomes 50%. The same policy is used in lineup sound, pre-snap camera admission, opening/critical/special routes and Upset Watch, and the admitted play retains its effective requirement into live/result reactions. The introductory big-game allowance already at 50% stays there.

This is admission, not an extra intensity bonus. Native packed-house classification, packed-context premiums, occupied-seat camera/audio strength, sound voice/volume budgets, native HFA's separate cutoff and cannon motion are unchanged. A night-only admission below the physical packed threshold does not acquire the packed-house label. Bowl, CFP and championship requirements remain unchanged. Unknown layout and stale/unsupported context still fail closed.

Night/scoring departure modifiers are regular-season only. They begin at zero after halftime, rise smoothly through Q3 and reach full strength at Q4. Night adds up to 20% pressure. Scoring pace uses total points divided by elapsed regulation fraction, normalized for custom quarters: a projected full-game total at or below 24 adds up to 15%, at 48 it adds none, and at 72 or above it reduces pressure by up to 10%. Smooth interpolation avoids sudden thresholds. The score margin limits how strongly this matters: one-score games receive only a quarter of the modifiers, rising to the full amount at a 21-point margin.

The factors multiply existing ordinary pressure and the speed of approach to existing blowout/traffic targets. They do not multiply the number of remaining fans or directly assign attendance. Target endpoints, quarter allowances, halftime schedules, crowd floors, safe visual windows, upset/comeback ownership, no-refill and score-event deduplication remain with their existing policies. The separate end-Q3 group keeps its existing score curve and seven-point cap. Both observed endpoints must permit pressure before a time interval is accrued; new scoring context is not charged retroactively across simulation jumps. Overtime and final presentation receive no new departure multiplier. Neutral regular-season night/scoring context is symmetric; existing home-only traffic rules remain home-only.

A mature noncompetitive low-scoring night can therefore request up to 1.38x pressure (1.15 x 1.20), but close-game changes remain much smaller. Actual visible differences depend on the existing target, floors, protection, safe windows and rounding; 20% more pressure does not mean 20 more percentage points depart.

## Diagnostics and testing

Status now reports effective night requirements and the combined departure engagement multiplier. The gate report uses the rivalry, bowl, local night and restored crowd-plan context rather than a reduced synthetic status snapshot. Installed-build live acceptance is pending. Compare the same matchup/weather/score path by day and night; include a sparse crowd, close defensive game, close shootout and decided loss. Retest halftime, resume, manual/Slow Sim and postseason isolation using the current checklist.

The new NightEngagementTests cover size-dependent admission boundaries, the hard 50% floor, native packed identity, live-play threshold inheritance, unknown layouts, postseason isolation, normalized scoring pace, bounded/close modifiers, comeback protection and full-game day/night/scoring ordering. Existing departure tests retain their no-backdated-score assertion while allowing the newly requested modest scoring-pace change in routine pressure.

The seating candidate accepted in 1.97 is retained. Crowd-animation work was dropped after its A/B showed no noticeable improvement, and no animation code or replay-offset setting is included.

## Separate recommendations, not changes in this candidate

Expanded marquee rivalry classification versus the legacy permanent-conversion/premium rules remains an explicit policy decision. Native HFA camera admission remains independent of DGA layered admission. Old raw prestige/toughest-stadium fields remain capture/validation dependencies despite the revised formula using atmosphere/tradition/brand/rank/conference grades. This candidate does not remove or silently retune those policies; program reputation itself remains represented in the approved support weights.

Validation completed: full automatic native/managed build and regression suites, all eight Python tests, 828 production-policy full-game simulations, and isolated MMC ABI/resources/extraction/storage/logging checks passed. The 828 existing scenario endpoints retain their previous final fullness, consistent with changing pressure/approach timing while preserving targets and floors. The separate matched-score NightEngagementTests also verify day/night/scoring ordering. Live acceptance is pending.
