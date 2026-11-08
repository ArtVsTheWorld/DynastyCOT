# Protected spectator budget: private 2.1.7

## Current-game check before implementation

Read-only evidence was preserved in
Build/ProtectedFanBudget217/LiveBaseline-20261106-PID28104.
The installed runtime was 2.1.6, PID 28104, home 1100 / away 1166.
Mapped supporters were 18,223 home plus 1,259 away: 6.46% visitors versus
a 5.6% appearance target. These are owned supporter selections, not a visual
jersey census. The difference between geometry assignment and actual occupied
positions remains an appearance-validation follow-up.

Arrivals completed at 79.5% mapped fullness. The runtime identified 1,710
protected academy spectators and 105 band positions. The mixer was healthy;
recent away-gain and touchdown reaction voices registered. There were no
current-session warning/error records or failed log writes. Two crowd applies
exceeded 16 ms (maximum 93.273 ms, seat preparation maximum 82.615 ms), and
one mixer sample reached 36.037 ms. These maxima do not establish perceived
hitches or frame rate. Replay flags and audio registration do not establish
visible playback or audible output. No game process or installed files were
changed during this work.

## Change

Bands remain outside fan counts. Required military/academy spectators remain
inside those counts and occupy the total attendance budget first. The same
budget applies to first-quarter arrivals, temporary halftime gaps and permanent
departures. It spans unique, verified seating layouts, so a separately rendered
protected section cannot add spectators above the target. Native empty seats,
visibility/culling and protected uniforms remain untouched. 2020 Mode retains
its explicit empty-stadium override.

The target is bounded by available ordinary spectators and the actual required
spectators already in the baseline. It introduces no peak percentage floor and
does not add required students on top of a fanbase estimate. Existing home/away
score pressure determines permanent exits. Temporary absences fill only the
remaining total absence budget, respecting those exits. Deterministic capped
highest-averages allocation spreads each side's count across layouts. Increasing
permanent targets cannot refill another layout through a rounding change.
Each layout retains its existing desirability/section rank order.

Layout identity uses the same position fingerprint as baseline preservation,
not a native address. Duplicate streamed renderers share one allocation.
Conflicting counts or missing geometry fall back to the earlier guarded path.
The budget is prepared on actual crowd-update passes; no per-frame native
reads or new file writes are added to fan/audio/camera callbacks.

## History compatibility

History format 7 appends a policy marker to each point and reads formats 1–6.
Old clips retain their previous temporary-seat accounting. An old Midgame Save
with temporary absences uses that accounting through the current interval and
then adopts the shared budget. New history stores the applied budget's target
alongside applied supporter losses, rather than pairing old visible selections
with a newer target still waiting for a safe break. Fixture/source-game
ownership, appearance seeds and existing save safeguards remain.

The marker also survives writing a legacy point to a new archive. Corrupt or
unknown markers fail without replacing the valid archive. Replay allocations
do not modify the live permanent-exit state. A fresh-process equivalent uses
saved supporter fractions, the fixture's counts and deterministic layout keys.

## Verification

ProtectedFanBudgetTests checks exact target sums, a separate all-protected
section, physical headcount limits, side constraints, halftime returns, nested
permanent removal, ordering independence, history 7 round trips, history 6
compatibility and corrupt input rejection.

ProtectedFanBudgetRuntimeTests runs production preparation and selection
functions in self-process fixtures. It covers arrivals, halftime and returns,
permanent departures, duplicate layouts, bands, original native holes, replay
seeks and live restoration, fresh-process-equivalent restore and legacy-policy
transition. No live game or external process is used.

Full regression, final native compilation, AIO routing integration and package
results are recorded under Build/ProtectedFanBudget217 and Build/Update4FanBudget217.
The new budget still requires an in-game visual check after installation.
The build remains private; no installation or publication is performed.

The full build completed with 161 PASS lines and 936 production-policy
full-game simulations. The final native payload and nine relevant test
executables were compiled again after the last applied-snapshot and layout
identity changes. Actual stock/AIO routing and isolated MMC package checks also
passed. Hashes and the exact package result are in
Build/ProtectedFanBudget217/final-verification.json.

A final read-only 2.1.6 check reached Q3 11:57, home 14–away 10: 13,765 home
and 920 away supporters (6.26% visitors), 59.9% fullness, 4% permanent exits
and a 23-point temporary halftime absence target. The mixer remained healthy
with no new warning/fault entries. This is a baseline for the existing build,
not a live verification of the new 2.1.7 budget.
