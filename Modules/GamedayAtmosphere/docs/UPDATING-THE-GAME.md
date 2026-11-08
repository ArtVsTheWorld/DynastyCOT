# Updating after an EA patch

Keep the last working release's tag and download intact so users can go back if needed.

1. Make a branch, such as `game-update-october`.
2. Collect the new game build details and an inactive log. The log identifies failed byte checks and unavailable controls. A DLL loading successfully does not mean its features activated.
3. Compare the new executable against the previous build. The inspection tool records the PE layout and bytes at existing named addresses without starting or modifying the game:

   ```powershell
   python .\Tools\InspectGameBuild.py --exe 'C:\Your\CollegeFB27.exe' --out .\Build\new-game-layout.json
   ```

   This is an inspection report, not an automatic offset finder or compatibility certificate. One matching instruction does not establish that every layout is unchanged.

   Protected disk code is not the readable runtime code. Preserve a private mapped-module snapshot after normal startup for comparisons, alongside its actual loaded base and EXE fingerprint. It is not a runnable unprotected EXE. Do not publish game snapshots or change disk protection for this workflow.

   Existing source byte/pointer checks can be audited without executing the game code:

   ```powershell
   python .\Tools\AuditMappedGame.py --snapshot .\Build\private-mapped-image.bin --base 0x140000000 --out .\Build\mapped-checks.json
   ```

   Use the actual loaded base, not an assumed base if ASLR changed it. This tool does not locate new functions, update offsets, inspect active heap objects or prove full gameplay compatibility. It rejects ordinary disk EXEs and reports individual failed existing checks. Native constants can be hashes rather than addresses; preserve independent golden inputs when relocating.

4. Check bindings and callers together:

   | Files | What to check |
   | --- | --- |
   | `Source/GameBindings.h` | Named native addresses, vtables and event entry points |
   | `Source/NativeRuntime.cpp` | Required/optional byte checks, hook lengths, inline addresses, object layouts and lifecycle callbacks |
   | `Source/ReplayPresentation.h`, `ReplayUpdateHook.h` | Replay expressions and safe displaced instructions |
   | `Source/SeatMapping.h`, `SeatBaseline.h`, `RenderedOccupancy.h` | Position/mask layout, original empty seats, renderer ownership and density |
   | `Source/SwellAudioRuntime.h`, `ReactionAudioRuntime.h`, `CrowdIntensityRuntime.h`, `AttendanceMixer.h` | Sound lifetime, events, voice limits and mix routing |
   | `Source/DirectorShakeRuntime.h`, `SparseHfaCamera.h` | Current views, projection/zoom, accessibility and camera ownership |

5. Change only verified addresses/layouts. Preserve byte checks, foreign-hook refusal and optional-feature failure handling. Removing a guard to make activation succeed can turn a clean failure into a crash. There is no EXE hash requirement; a report hash only identifies the inspected file.
6. Add a regression for the failure, run the full build and check settings migration. CI checks code and models; it cannot verify patched game objects or sound.
7. Test pregame/kickoff, consecutive games, arrivals, halftime, Normal/Fast Super Sim, a resumed scenario, manual/automatic replay, sound, live reactions, a TD/eruption, zoomed cameras and Return to Hub. Include sparse/packed venues and the usual other plugins.
8. Increment `Source/Product.json`. Change default settings/migration only if needed. Keep old runtime filenames distinct.
9. Update features, release notes and release history. Build and verify the ZIP. Test the DLL from that ZIP, not another loose build.

## Publishing

Create an annotated version tag on the tested commit. Create a GitHub release with the public update name, attach the verified ZIP and checksum, then mark it latest. Do not replace an old binary under an unchanged version. CI builds are not public releases.

Tie reports to the working build, game version and MMC version. Include matchup, mode, quarter/clock, settings, status and logs. Dispatch success alone does not establish audibility.
