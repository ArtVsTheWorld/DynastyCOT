# Update 3 — code comparison with Update 2

Baseline: **v1.54.0 / 1.54.0**, commit `9f326280fe6100a732b95d04f08c2e430d8192fa`  
Release: **v2.0.21 / 2.0.21**

The release notes compare those source versions. Development notes were checked against the final code; abandoned candidates are not presented as active features.

## Measured differences

| Item | Update 2 | Update 3 | Evidence |
| --- | --- | --- | --- |
| Native in-game DGA settings | No DGA settings tab | 38 controls: Attendance12, Soundscape12, Broadcast14 | [Menu catalog](../../Source/SettingsMenuOptions.inc) |
| Pause-menu fullness | No DGA fullness row | Measured Stadium Fullness beside Resume | [Pause menu](../../Source/PauseMenu.h) |
| Native reaction selector support | 5 allowed IDs:91,75,83,59,95 | 80 usable IDs across27 families;13–15 and23 excluded | [Update2 dispatcher](https://github.com/ArtVsTheWorld/DynamicGamedayAtmosphere/blob/v1.54.0/Source/ReactionAudioRuntime.h), [current catalog](../../Source/ReactionCatalog.h) |
| Named live-outcome kinds | 13, excluding None | 32, excluding None | [Live reactions](../../Source/LiveReactions.h) |
| Fresh regular-season peak | EA's original peak | DGA support/demand formula using six weighted inputs | [Turnout policy](../../Source/StartingAttendance.h) |
| School arrival groups | Four severe-late schools | Same four plus Utah | [Turnout policy](../../Source/StartingAttendance.h) |
| Named rivalry overrides | No named full-peak override set | Eight native rivalry events with100% expected peak and protected kickoff ranges | [Rivalry policy](../../Source/ShowcaseAttendance.h) |
| Crowd mix | Previous crowd/chant parameter paths and excitement quieting | One synchronized eight-group native-bus mix, occupied-seat/fullness curves, native excitement preserved | [Mix policy](../../Source/AttendanceAudio.h), [mixer](../../Source/AttendanceMixer.h) |
| Occupied-seat motion | No occupied-seat strength curve | Linear0.80x at6060 to1.25x at61888, separate from admission | [Motion policy](../../Source/VenueMotion.h) |
| Live-run motion | Earlier gain handling | Fixed11-to15-yard camera ramp with smoothing and peak audio handoff | [Detector](../../Source/LiveReactions.h), [camera](../../Source/DirectorShake.h), [native swell](../../Source/SwellAudioRuntime.h) |
| Optional empty-stadium mode | Absent | 2020 Mode with owned attendance/audio restoration | [Silence runtime](../../Source/StadiumSilenceRuntime.h) |
| Final-play carry | Winning eruptions | Other earned final-play sound/motion can finish too | [Final carry](../../Source/FinalReactionCarry.h) |

The named-outcome count is a code-organization measure, not a count of32 independent triggers. The selector count describes native sound choices, not unique recordings. Update2 already had arrivals, departures, layered atmosphere, Upset Watch, eruptions, saved crowds, additional replays and Super Sim support.

The ordinary run-cheer default was15 yards in Update2 and is15 in this release after intermediate retuning. The new item is the fixed11-to15 camera buildup, smoothing and stronger peak handoff, not a claim that the public default moved from20 to15.

## Values executed from the production headers

[Machine-readable output](Update3-Code-Facts.json) comes from [MeasureUpdate3.cpp](../../Tools/MeasureUpdate3.cpp), which includes and executes the current production policies. It does not open the game, MMC or any save.

- School-support weights:35% Stadium Atmosphere,25% Team Prestige,20% Program Tradition,10% Brand Exposure,5% Conference Prestige,5% AP rank
- All eight named rivalry cases returned100% expected peak even with synthetic low-support participants; team IDs do not select that policy
- Native return samples produced kickoff buildup25/peak35 yards and punt buildup30/peak40 yards
- Default run cheer15 yards and long air catch20 yards
- A close-up ratio that used to give50% shake now gives63.728% before the other independent crowd/neutral/envelope factors
- Normal wide-view compensation remains capped at1x; opt-in wide-player compensation remains separate
- At6060 mapped seats and25% fullness, background signal is50% of vanilla
- At61888 mapped seats and100% fullness, background signal isabout158.5% (+4dB), while collective reactions remain130% (+30%)

To reproduce from a configured MSVC Developer shell after building this version:

```powershell
cl /nologo /O2 /EHsc /std:c++17 Tools\MeasureUpdate3.cpp /Fe:Build\MeasureUpdate3.exe
Build\MeasureUpdate3.exe
```

## Validation and limits

The local release checks include138 native test programs,511 real configuration-reader cases,936 full-game policy simulations,14 Python tests,178 mapped binding guards and an isolated MMC host check. The measured42–9 home-win fixture ended35% full at night versus59% by day. That is a deterministic policy simulation, not a prediction for every stadium or score path.

Offline Dynasty and RTG were live-tested during development. The last small zoom increment and new steady-roar balance still need the next live check. Code/tests do not establish perceived loudness, FPS, every camera view or a rendered replay sequence. Native rivalry-event retention on arbitrary TeamBuilder replacements is conditional on the replacement preserving that metadata.
