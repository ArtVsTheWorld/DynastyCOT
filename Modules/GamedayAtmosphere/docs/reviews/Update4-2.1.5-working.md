# Update 4 private working build: 2.1.5

This is development work, not a published release. The installed plugin and live
game have not been changed by this offline pass. Presentation protection and
automatic celebration filtering remain unfinished. Visitor-plan persistence,
native selector cleanup and fixture-linked crowd history are implemented and
checked offline; their live results still need verification.

## Changes in the working code

- Bands are excluded from DGA thinning. Military-tagged spectators and student
  sections belonging to verified military schools also stay. The academy
  sections in the checked Army, Navy and Air Force files use the student label.
  Ordinary students and standing-only fans are not automatically exempt.
- School identity for those protected sections is read independently of school
  grades or applying the revised peak formula. Unknown identity is not guessed
  from a team ID. 2020 Mode still empties the stadium.
- Opening kickoff shake is 25% gentler. The separate hostile defensive opening
  remains unchanged.
- Fumble recovery proof uses the verified ball carrier's team. Picking up an
  offense's own fumble cannot become a defensive takeaway merely because the
  possession label has not caught up. Defensive recoveries keep the return path.
- The visiting-fan candidate uses relative fanbase demand, native city distance,
  local opponents and rivalry. Existing native visitors count toward its target.
  Protected groups retain their outfits. Home and visiting supporters have
  separate permanent departure counts. Neutral appearance rules stay native.
- A verified crowd fixture is now separate from an applied turnout override.
  Switching revised peak attendance off does not, by itself, disable the
  visiting-fan appearance plan.
- Resumed crowd setup clears provisional departure counters before restoring
  saved home/away losses, including when the seating seed has not changed. An
  unmatched resume clears previous saved fractions. Protection limits remain.
- A weaker visiting sustained reaction cannot replace a stronger home reaction.
  Separate home and visitor cheers remain independent. With no remaining home
  fans, DGA does not add a home-supporter one-shot; native sounds are untouched.
- The major replay model waits for a stable, verified play-call destination
  before sending an inset request. Finishing a full-screen clip while the
  celebration camera is still active is insufficient. The delayed fallback has
  the same destination requirement. Slow Sim still gets no play-call inset.
- History recording avoids calculating a supporter ratio with no original
  supporter count. Existing version 1–5 histories remain readable; the private
  version 6 format records separate supporter departure fractions.
- The exact fixture's visitor target and selection seed persist in a bounded
  plan book under History. Signed native dynasty IDs, both program rows, season,
  week and stadium must match. Missing audience inputs on resume do not reroll an
  existing plan. Neutral crowds keep their native appearance rules.
- Team-selector bits are temporary, scoped to EA's generation callback. After
  generation they return to their original values; unrelated native flags are
  preserved. DGA keeps a separate, bounded generated-supporter record for its
  population model. Unchanged regeneration does not rebuild seating ranks.
- Crowd history is linked to that fixture's actual recorded game ID. Matching
  teams/score/clock in another dynasty is insufficient. Neutral and disabled
  visitor appearances can store a history-only ownership record. Old unlinked
  histories use an estimate. Rank/weather refreshes may differ for a known
  source; the fixed game context and clock/score must still match.
- A worker defers saving a new history link until its resumable crowd checkpoint
  has reached disk. Plan reuse and binding the same history ID do not dirty the
  book. No file writes occur inside the appearance or per-seat callbacks.

## Offline checks

`Build/Update4OwnedHistory215-build.log` records the completed current full build,
regression executables, settings/loader checks and 936 attendance simulations.
NativeAppearanceTests uses self-process memory to verify temporary selectors,
generated identity and retired-stream rejection. VisitorPlanTests covers
ownership, appearance regeneration, history-only records, legacy parsing,
different-dynasty rejection and disk-checkpoint ordering. These checks do not
establish visible shake, audible
mix balance, actual visitor outfit percentages or rendered replay playback.

## Research completed without touching the game process

The full crowd-label catalog is in `docs/CROWD-TYPES.md`. Native placement,
uniform and military-school evidence is retained in `Build/Update4NativeResearch`.

Three current celebration behavior trees and nine celebration presentation
packages have been exported and decoded from the game files for inspection.
The play-result tree contains both ordinary and custom celebration option nodes.
Their Dance/Spike/Signature choices are also assigned to manual directions, so
editing that shared table would change manual celebration choices too. The
automatic selection path needs a narrower control before any filter is enabled.

Sixteen skip-related presentation packages have also been exported and decoded.
Several are transition cleanup: they stop specific music channels, switch views
and return stadium audio. They do not contain the skip-button decision itself.
Blocking those whole packages could leave music or presentation state hanging.
The control must be found before the transition, with normal cleanup preserved.
A name such
as "QuickSkip" in earlier DGA code is not proof that a query represents the
user's skip button. No unverified native skip override or placeholder setting
has been added.

## Work before a release candidate

- Verify visitor outfits and fixture-linked attendance through a fresh-process
  Midgame Save load, including upgrading an older unlinked checkpoint
- Reconcile the visitor target against actual native occupied positions in
  sparse crowds; the current geometry assignment is not a measured outfit count
- Finish and verify presentation protection for both manual skipping and
  automatic shortening
- Filter verified automatic celebration choices while preserving manual input
- Visually test an academy section, visitor outfits and blowout retention
- Verify own-fumble pickup, defensive recovery/return and touchdown replay
  handoffs in game
- Update the final feature list and release notes around the verified result

The academy visual test and final sound/replay checks can wait until the user
returns. No unattended game input, live previews, installs or publication are
needed for the offline work above.
