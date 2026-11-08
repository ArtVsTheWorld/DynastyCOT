# Update 3 — retention, halftime timing and supplied-save comparisons

Candidate 1.92.0, 29 October 2026. Implements approved items 1 and 2. Section-by-section thinning/color variation remains the next pass. Peak/kickoff turnout, crowd-motion strengths, replay handling and blowout endpoint policies were not retuned here.

## Close games

Ordinary close-game halftime permanent-departure targets are now 3–5% of peak, with marquee close games at 1–3%. Margins of 9–16 ease continuously toward the existing 10% target at 16 points; 17+ targets are preserved. Routine close-game departure pressure is reduced, blending back into the former rate by a 16-point margin. These are targets and pressure changes within the existing model, not hard crowd assignments or extra score-triggered deductions. Floors, protection, safe windows and no-refill behavior remain.

In the same representative score path and 61,888-position layout:

| Scenario | 1.91 final permanent departures | 1.92 | Final visible fullness in 1.92 |
| --- | ---: | ---: | ---: |
| Middle-support close | 13% | 8% | 79.0% |
| Elite ordinary close | 13% | 8% | 92.0% |
| Elite night rivalry | 7% | 5% | 95.0% |
| Neutral rivalry | 6% | 4% | 96.0% |
| Rainy middle-support home rout | 72% | 72% | 22.6% |

Departures use the peak crowd as denominator; visible fullness uses capacity. Close-game halftime permanent loss in the ordinary example drops from 9% to 4%. Postseason permanent-departure policy is preserved.

## Halftime timing

Seeded games begin the temporary departure ramp during the last 20–30% of Q2. Eighty percent of temporary seat-breakers follow a main return wave ending 15–25% into Q3; the remaining 20% follow a modestly delayed wave ending 25–35% into Q3. The seed fixes these timings for the whole game; callbacks and score changes do not roll them again. Timings scale with quarter length and use the already-archived variation seed. Unseeded legacy history keeps its former timetable. The resume estimator and halftime permanent conversion use the same return envelope.

At 12-minute quarters, this means the departure ramp starts around 3:36–2:24 before halftime, most returns finish 1:48–3:00 into Q3, and the small later wave finishes 3:00–4:12 into Q3. This changes whole-crowd timing only; it does not introduce the future section-pattern work or new user settings.

## Item 3: duration of a settled result

Matched synthetic paths now reach the same 42–7 result early versus late:

| Same final result | Settled early: final fullness | Developed late: final fullness |
| --- | ---: | ---: |
| Home loses by 35 | 21.5% | 36.1% |
| Home wins by 35 in rain | 22.6% | 31.5% |

The existing model already distinguishes duration. These comparisons are evidence for reviewing the remaining aggressiveness, especially the late rainy winner; no additional blowout cut, floor or persistence rule was added without reviewing those results. They are model outputs, not measured spectator behavior.

## Supplied Dynasty save

A private copy of `DYNASTY-DGA183-AUTOSAVE` was parsed read-only using the locally installed parser/schema. Original and copied SHA256 matched. No save-writing API was called and the original file was not modified.

- 2026 season, week 1 snapshot; 143 teams, including 138 FBS school-grade records.
- 892 regular-season fixtures with FBS home teams, including 127 FCS opponents and 140 rivalry fixtures.
- 11 neutral venues inferred from explicit saved venue references differing from home/alternate venue references; zero venue references use the home default.
- 790 future/undetermined weather values remain unknown. Only 102 fixtures have a usable saved-weather condition.
- 1,886 scheduled launch scenarios: clear-day and clear-night brackets for all 892 fixtures, plus saved-weather daytime scenarios for the 102 known conditions.

The schedule-weighted, variation-neutral clear-day peak averages 79.3% capacity (median 87.9%); the clear-night bracket averages 82.2% (median 91.6%). These are controlled brackets, not a claim that the whole schedule is played at night. Current ranks, grades and record inputs are frozen at the save snapshot, not forecasts of later weeks. The opener bonus follows prior scheduled true home games, so it is not wrongly applied to every fixture.

Saved menu kickoff times have an unresolved timezone relationship to the verified stadium-local launch option. Actual night classification is therefore not guessed from those times. Static venue capacity/roof/layout records are external to this save. Results are capacity percentages with the unverified venue-hash variation bracketed ±3 points before caps; saved-weather scenarios assume an outdoor venue. Rendered occupancy and live sound/camera behavior still require a few representative in-game checks, not playing every scheduled matchup.

Raw saves, team tables and the local parser remain private in the owned build folder. Staged review data contains only numeric schedule inputs/results and aggregate findings.

## Verification

The full native/managed build, targeted retention/arrival/history/runtime tests, eight Python checks and isolated MMC adapter checks passed. New tests cover stable timing across 256 seeds and four quarter lengths, bounded/monotonic temporary absences, two-wave returns, halftime conversion ownership, rewind rejection, legacy timing and revised close-game targets. The production runtime fixture verifies cleanup at the seeded return deadline rather than assuming every game ends its break at a fixed clock. Simulations cover 828 full-game combinations, 69 detailed traces, 128 halftime seeds and the supplied-save schedule scenarios. No live visual, audio or replay acceptance is implied by these checks.
