# Update 3: 2.0.33 lens-based shake balance

The user confirmed that the latest camera feedback came from the installed 2.0.31 build. The uniform 25% reduction in the privately staged 2.0.32 candidate was not installed. This change therefore uses 2.0.31 as its strength reference rather than adding another reduction to 2.0.32.

## Behavior

The existing horizontal and vertical projection correction remains. A separate, bounded lens preference applies once to pitch, yaw and roll:

| Actual vertical lens | Change from 2.0.31 at the same settings |
| --- | --- |
| About 54.45 degrees or wider, matching the captured Standard lens | 15% weaker |
| About 37.90 degrees or tighter, matching the captured Zoom lens | 5% stronger |
| Between those lenses | Smooth transition between the two strengths |

The transition uses a smoothstep in vertical focal length, with zero slope at each endpoint. It adds only scalar arithmetic to the existing projection calculation, with no additional native reads, allocations, camera-name lookup or per-frame trigonometry. The normal optical safety bounds still apply to unusually wide views.

This is a lens preference, not a physical camera-height measurement. Modded cameras receive the same treatment from their actual projection. A narrow lens on a high camera also gets the tight-view preference. The five named sliders remain optional personal overrides, default to 100, and preserve saved settings. Different aspect ratios do not change the vertical-lens preference.

Event eligibility, occupied-seat scaling, reaction envelopes and handoffs, neutral/away percentages, accessibility controls and replay exclusions keep their existing behavior. No attendance or audio tuning is included. The output status now lists the framing factor separately from optical correction and the user's slider.

## Checks and limits

- Projection tests compare both unbiased and intentionally biased screen displacement across lens, aspect, effect strength, personal preference and oscillator phase.
- A fine lens sweep checks the range, direction, continuity, aspect independence and flat endpoints of the new curve.
- Runtime fixtures check actual matrix output against the requested broad, tight and intermediate strengths for every effect category and four personal preferences. This catches missing or repeated framing gain and changes to the source camera matrix.
- Existing invalid-projection leases, world ownership, pause/replay exclusions, native accessibility gates, zero controls and camera-position preservation remain covered.

The exact full-build, menu, simulation, Python, isolated MMC and package results are recorded in the private staging BUILD-INFO. Matrix projection comparisons and estimated screen movement are not rendered-pixel measurements or proof of equal perceived shake. This revision has not received a new live visual comparison. It is staged privately without installing, pushing or publishing it.
