# Troubleshooting Update 3

Current working build: **2.0.32**, staged for private testing. See the [README](../README.md), [full feature list](../FEATURES.md) and [release notes](releases/Update3.md). Earlier development observations remain in `docs/reviews`; their build-specific pending checks are historical, not a description of current release behavior.

## Check the installed build

Close the game and MMC before replacing the DLL, then restart both. Keep one copy of DGA installed. Open `Plugins/DynamicGamedayAtmosphere/Logs/status.txt` and check the version, game process and activation state. A version line alone does not establish that every optional feature activated.

Update 3 refreshes old settings once and backs up the exact old INI. Do not delete the History or Backups folders. New custom settings survive later launches.

## Report a problem

Send the whole Logs folder. Include teams, mode, quarter/clock, down, play result, whether you skipped presentation, and what you heard or saw. A recording with a timestamp helps. No files upload automatically.

- `status.txt` and `DynamicGamedayAtmosphere.log` are the readable summaries.
- `technical-status.txt` and `technical.log` retain detailed settings, native proof, IDs, replay stages and timing counters under normal logging.
- `launcher.log` records startup and loading failures.
- A requested sound, registered voice or camera cue does not prove audible or visible output.
- Timing maxima include the labeled callback work; they are not FPS measurements.

## Useful comparisons

For attendance, use a fresh regular-season Dynasty launch for peak-formula checks. A loaded game cannot regenerate its original seating. Compare expected peak, kickoff share and actual fullness separately. Saves and replays should restore their own recorded crowd without refilling permanent departures.

For sound, compare the same stadium and similar plays at different fullness. DGA may overlap native cheers; that is intended. The occupied-seat mix has no separate chant cut and no native excitement limiter. To disable DGA's audio hooks completely, turn off Crowd Sound Effects and restart the game/MMC.

For camera movement, check the game's Dynamic Camera Effects setting and DGA's Added Camera Shake switch. Enough seats and a qualifying play/situation are still required. Cannons have a separate setting. Gameplay HFA remains native. Camera movement now adjusts to the actual horizontal and vertical projection. All five camera sliders use the shared Standard/Zoom-calibrated baseline at 100. The normal extreme-wide correction bound is 4×; Wide Player Camera Compensation extends it to 6× and remains off by default. Native HFA camera shake yields while DGA is applying added movement, then restores its latest tier. The native-filter minimum of zero retains its opt-out.

For replays, note manual versus Slow Sim, any injury/penalty scene and whether playback was skipped. Manual major-play routing waits for full-screen playback before requesting an inset; Slow Sim has no inset destination. Native recordings and presentation still decide the visible sequence.

For 2020 Mode, check empty stands, no collective crowd/bands/drums/PA, retained quiet detail and clean field/commentary sound. The accepted ambience/reverb treatment is included; final individual-detail balance remains a live-check item on this release. Switching off should restore the current normal crowd and retire the audio overrides.

## Validation record

The 2.0.9 full-game title-update check passed activation and user-observed unskipped full-screen/inset playback. The repeated-whistle-hitch fix was previously confirmed by restarted OFF/ON comparisons and later live play. Subsequent cleanup, 2020 audio and logging changes have automated coverage. A passing fixture is not a claim that every rare play or presentation route has been seen live.

The build runs 141 native test programs, the managed checks, 936 game simulations, 560 configuration-reader cases, 14 Python tests and an isolated MMC adapter check. The exact packaged DLL and matching separate loader are checked in an isolated MMC host before staging. Private captures, EXEs and saves are not included in the repository or release.
