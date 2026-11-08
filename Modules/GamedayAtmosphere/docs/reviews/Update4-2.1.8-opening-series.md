# Opening kickoff and first defensive series: private 2.1.8

## Sunshine Showdown observation

The read-only 2.1.6 launch check found native rivalry event 53, game-supplied
ranks 66/73, a 77% kickoff plan and 76.7% mapped fullness. Intro atmosphere
started, but no opening-kickoff activation was logged; the kickoff result's
camera cue was inactive. The separate first-away-offensive-play welcome
activated later. Logs alone do not establish every rendered camera frame.

The source explains the gate: the dedicated opening kickoff required the ranked
big-game classification. Protected attendance did not grant that classification.
The event was therefore recognized for turnout but rejected for kickoff motion.

## Changes

- Capture the existing native rivalry event read as transient match context.
  All 24 protected native showcase events can qualify for opening kickoff,
  including unranked teams and TeamBuilder replacements retaining the event.
  Team IDs and exceptional turnout percentages do not choose the rule.
- Keep ranked big-game classification unchanged for other mechanics. The
  kickoff-specific gate preserves fresh-opening proof, score/clock/formation,
  opening and camera settings, crowd fullness and native world ownership.
- Retain the 4.5–6% gentle kickoff range, occupied-seat/camera/neutral scaling
  and existing whistle fade. No global shake-strength increase is added.
- When visitors have the first offensive possession at a home stadium, the
  hostile atmosphere now follows their first series of downs. It ends on a
  first down or actual punt, plus scores, takeaways, turnover on downs,
  possession changes and field-goal attempts. Selecting a special-teams call
  alone does not prove a punt.
- The hostile bed/roar/swell continues through live play and between snaps.
  Each qualifying pre-snap call keeps the existing hostile camera range;
  punt buildup retains its half-strength adjustment. The sustained channel
  cycles within its bounded loop rather than expiring after one play.
- Each fresh verified incompletion or stop of two yards or less can get the
  largest generic result cheer once. Longer gains do not end a series unless
  they earn a first down. First-down conversions do not get a home-success
  cheer, and stronger scoring/takeaway routes retain ownership.
- Prefer the official first-down result. A confirmed next on-field lineup's
  down/yard-line progression provides fallback when that result was missing
  or late. Transitional play selection alone cannot supply that fallback.
- Timeouts, ordinary penalty scenes and pause/replay suspension do not create
  another opening or erase knowledge of the series. An automatic first down
  still ends it. Resume, simulation and a later drive cannot invent a new one.
- The series can cross Q1/Q2 on short quarters, but cannot continue through
  halftime. Neutral sites keep their existing one first-offensive-play welcome
  per team rather than adopting the home-stadium hostile series.

The Opening Atmosphere help and feature list are updated. A new technical status
line identifies whether the hostile opening series is active. The previous
protected-seat shared budget and AIO menu compatibility remain included.

## Checks and limits

OpeningSeriesTests wires intro proof, play tracking, lineup admission, kickoff
audio and the motion model across all 24 event IDs. It also covers repeated
defensive result responses, long non-converting gains, explicit and missing-result
first downs, punt selection versus actual punt, score/possession endings,
penalty/timeout/pause continuity, long sustained output, quarter transitions,
special-teams classification, and neutral/resume/simulation exclusions.

Existing hostile-opening, first-drive, ranked-rivalry, kickoff-sound, camera,
reaction and save/crowd tests remain part of the full build. Tests use private
objects; no live game input, process preview or installation is performed.
Evidence is under Build/Update4Opening218. The new behavior still needs a fresh
live kickoff and opening defensive-series check after installing 2.1.8.
The package remains private and is not published.

The full build completed with 162 PASS lines and 936 full-game simulations.
After the last opening-toggle and series-lifecycle guard changes, the final
native payload and ten relevant executables were compiled/checked again.
Actual stock/AIO routing and isolated MMC/resource/package checks passed.
Package and source hashes are in Build/Update4Opening218/final-verification.json.
