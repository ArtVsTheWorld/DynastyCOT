# Gameday Atmosphere standalone packaging test

Distribution: `1.0.2-standalone-test`. Native engine: `2.1.9`, identical to Dynasty COT 1.0.2. This is a local test artifact, not a published Gameday release. COT 1.0.2 remains published at its existing source tag.

The ZIP contains exactly `GamedayAtmosphere.dll`, `README.txt` and `FEATURES.md`. Native code, the defaults and the optional diagnostics tool are embedded. The plugin verifies and prepares its own runtime outside MMC's recursive Plugins scan. No external loader EXE or SDK is distributed.

## Settings and coexistence

- Existing canonical settings are retained byte-for-byte. Old flat INIs are copied unchanged; originals are retained. No automatic reset or retune occurs through the new startup path. New installs receive released defaults. Future configuration schemas retain their native refusal path.
- Existing data stays in `Plugins/DynamicGamedayAtmosphere`. The old internal data name is deliberate. Logs remain under `Logs`, diagnostics under `Tools`, and crowd history under `History`.
- Runtime files stay in `DGA-Runtime/Runtime-2.1.9`. Exact old helper/native names are backed up outside Plugins after the game is closed; unrelated files are untouched.
- Current COT and standalone use the same per-MMC-process named mutex. Only one starts the Gameday watcher. The fixture tests exercise both ownership orders without starting any watcher or loading native code.
- Choose one provider: disable COT's Gameday module if using its other modules beside this DLL. Move the old `DynamicGamedayAtmosphere.dll` outside Plugins before using the new branded DLL; keep its data folders. Legacy DLL detection provides this guidance.
- The released COT 1.0.2 installer does not identify the new standalone DLL as one of its old standalone DLL names. It leaves this independently installed DLL alone; the shared startup guard prevents duplicate runtime loading. Installing COT does not automatically select which provider the user prefers.
- A COT uninstall can remove a shared native cache it owns. Tests verify standalone re-extraction and unchanged INI/history afterward. Older COT migration/reset behavior still applies if switching an old profile back to an older bundle; back it up first.

## Feature audit

The complete user-facing list is in `Packaging/Standalone/GamedayAtmosphere/FEATURES.md`. Claims were checked against these integration sources, rather than an older standalone feature list:

| Feature group | Source evidence |
|---|---|
| Turnout, arrivals, profile controls and halftime | `Source/SettingsMenuOptions.inc`, `AttendanceRules.h`, `ArrivalRules.h`, `NativeRuntime.cpp` starting-attendance path |
| Visiting groups, strength, travel fallback and host resistance | `Visitor*`, `AwayCrowd*` rules/fixtures, native visitor-plan integration, appended visitor-strength setting |
| Early exits, winning visitors, continuity and protected groups | departure/late-traffic rules, native audience/history integration, `AwayCrowdIntegrationTests.cpp`, `BandProtectionTests.cpp` |
| Roar, deficit reduction, cheers, reactions, shake and camera controls | soundscape/broadcast menu definitions, `NativeRuntime.cpp`, crowd audio/reaction/director rule headers and fixtures |
| Extra replays with touchdown exclusion | native replay integration, `MajorReplayTests.cpp`, `ReplayPresentationTests.cpp` |
| 2020 Mode and pause fullness | native audio/pause integration, `Tuning2020Tests.cpp`, `PauseMenuTests.cpp` |
| Basic offline Play Now and scope limits | native `selected.openingOverride=75` Play Now path, `OfflineMatchScopeTests.cpp` and offline scope guards |

No native behavior changed in this test package. Native/defaults/diagnostics SHA256 values match the already source-built, published COT 1.0.2 bundle exactly.

## Reproduce and verify

From the integration workspace, with the local MMC SDK available:

```powershell
python Tools/build_gameday_standalone.py --mmc-sdk 'C:\path\to\MMC_ModManager_vX.X.X.X'
python Tests/test_gameday_standalone.py
python Tools/package_gameday_standalone.py
```

The automated pass includes 90 checks, including the existing Settings, PluginStorage, RuntimeStorage and ManagedRuntimeLoader regression suites. It checks the production managed assembly, embedded resource parity, extraction/hash repair, repeated starts, legacy DLL guidance, canonical/old/future settings preservation, history, both guard orders, duplicate assemblies, and a real COT 1.0.2 installer install/uninstall round trip in a marked isolated folder. No installed MMC files were modified; no watcher, game injection or native engine execution occurred.

`Release/GamedayAtmosphere-1.0.2-standalone-test-verification.json` ties checks to DLL/source/archive hashes. Integrity metadata is outside the ZIP, keeping the download itself to three files.

## Remaining live acceptance

1. Close MMC/game, back up the existing Gameday data folder, use one provider, and put only the DLL into Plugins. Confirm one Gameday runtime starts and no duplicate warning loop appears.
2. Confirm existing settings and prior Dynasty crowd history remain available. Change one control, restart and confirm it persists.
3. Load a fresh Dynasty game: inspect turnout, visitor mix, arrivals, halftime breaks, late departures, roar and shake with Dynamic Camera Effects on.
4. Load basic offline Play Now: with arrivals on, confirm its 75% opening model, later arrivals, sound and shake. Confirm no Dynasty travel history is written for this match.
5. Confirm native touchdown presentation still plays; added non-touchdown replays remain optional.

Startup/offline checks do not establish live compatibility with every game build, stadium or overhaul. Other three standalone module downloads are not part of this packaging test.
