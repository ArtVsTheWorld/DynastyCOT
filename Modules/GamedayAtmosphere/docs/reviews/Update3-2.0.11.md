# Update 3 final logging/storage cleanup â€” 2.0.11

## User-facing and technical diagnostics

- `Logs/status.txt`: compact current-game, attendance, atmosphere, audio/replay health and performance summary. Fullness is distinguished from the percentage of the planned crowd. Timings use milliseconds. The native final phase is labeled Final in this readable report. Process ID, audio-fullness and diagnostic-preview fields retain their diagnostic-tool labels.
- `Logs/technical-status.txt`: the existing full report, plus the raw presentation phase. Timing counters, identities, feature readiness, context gates, audio levels and settings remain available without enabling Debug.
- `Logs/DynamicGamedayAtmosphere.log`: readable events, results, attendance changes, major replay stages, warnings and errors. Routine queued/accepted voice chatter, airborne-pass samples, repeated roar start/stop detail and pending-result notices go to the technical log instead.
- `Logs/technical.log`: the original detailed event records at normal diagnostic coverage, unchanged by readable formatting. Debug still adds the optional deeper traces. Readable truncation/formatting never modifies this copy.
- `Logs/launcher.log`: existing bounded MMC startup/loading diagnostics.

The existing bounded producer queue remains singular. Human formatting and both event/status writes run only on the worker. Each event-log drain opens each file once per batch rather than once per entry, retaining the existing cap/reset-at-entry-boundary policy. Each event log has its own MaxFileMB cap; statuses are overwritten once per second. The first conversion copies a legacy detailed main log to technical.log before clearing the readable destination; failed copy preserves the original.

## Settings and file ownership

The final-release reset generation is now4. Update2, older and prerelease Update3 INIs (including generation3) receive current defaults once, with an exact byte-for-byte backup before atomic replacement. Later custom edits survive; future schemas/reset generations remain protected. Failed or locked backup/reset paths stop preparation safely rather than discarding the old INI. Users should close the game/MMC before updating and need not delete any INI.

Only DynamicGamedayAtmosphere.dll remains directly inside MMC/Plugins. Active settings, Logs, History, Diagnostics, Tools, versioned runtimes and Backups remain in Plugins/DynamicGamedayAtmosphere. Known loose loader/diagnostic executable names and strictly versioned DGA native runtimes are moved to that Backups folder after the game closes. Unrelated files, nonmatching names and the adapter DLL are left alone.

## 2020 Mode audio

A scoped live audition on runtime2.0.9 compared the normal empty-stadium mute with WeatherAndAmbience+6dB and the existing ReverbMaster return+3dB for up to four minutes. The user heard a more natural/open sound, with clear audio. Both added gains retired and the original audio interface was restored afterward. No installed DLL, INI or sliders were modified for that comparison.

The release profile applies those two native-bus gains only in unpaused field/presentation phases; it does not invent a new reverb algorithm, modify sends or apply a global dry gain. Commentary/referee/player/foley dry gains stay unchanged; any existing wet content follows the auditioned return adjustment. The native reverb itself still determines the room/tail.

The user also reported the separate drum cadence. 2020 Mode now mutes DrumCadence77 and PAAnnouncer32, alongside both bands and the collective crowd children. Rather than muting parent Crowd9 unconditionally, it mutes anticipation, Beds, Chants, Reactions, StudentSection, Swells and scripted CrowdNIS, preserving quiet Sweeteners/individual details at-3dB. Those are native individual fan samples used as sideline flavor, not newly identified player voices; existing actual field/player chatter remains available. Their balance could not be auditioned under2.0.9's parent mute and needs the staged-build smoke check.

Every group is checked against its exact native name/parent before use. If a collective crowd child cannot be verified or faults, a root Crowd mute remains the fallback and detail stays silent. Owned gain/mute handles are retired on disable/context loss; unrelated music, user sliders and normal-mode mixes remain unchanged. 2020 Mode stays disabled by default.

## Verification

Targeted log formatting/cap/migration and settings/helper-location tests pass. Native-shaped 2020 audio tests cover separate crowd-child/band/drum/PA mute, quiet detail, spatial gains, exact group identities, missing-child fallback, repeated updates, manager replacement, release faults and field-context retirement. The full automatic native/managed suite passed, including 828 attendance simulations and 497 native-reader configuration cases. All 14 Python tests and 178/178 mapped-image checks passed. The final native rebuild and focused production seating/startup-status, logging, 2020-mode and mixer tests passed. The isolated current-MMC ABI/resource/extraction/storage check passed against the final native binary. Package contents and embedded-resource hashes are verified at staging. No installed game/MMC/INI/save changes have been made by this work.
