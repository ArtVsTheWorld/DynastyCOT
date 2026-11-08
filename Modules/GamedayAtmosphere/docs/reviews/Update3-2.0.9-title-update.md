# CFB27 title-update compatibility — candidate 2.0.9

## Scope

This candidate ports the preserved 2.0.8 source to the Windows game installed on October 30. It changes native addresses, RIP/call displacement signatures and their native-shaped test fixtures. Attendance formulas, arrival/departure policies, seat patterns, audio curves/palette, shake strengths, menu options, settings defaults/migration markers and replay policy remain unchanged.

The earlier source is tagged `v2.0.8-october13`, with the private draft release “Update 3 - October 13 Build.” preserved. The compatibility work is on `title-update-20261030`. This document identifies the inspected installation; it does not infer an official cross-platform patch number from the generic file-version string.

## Preserved evidence

| Identity | Before update | After update |
|---|---|---|
| EXE SHA256 | `4c5365a3903db783c8240d09f63d36199ec6b693b76e7f440b5ac925e855506d` | `8247e6dcc68b78672ac1c79e2deb41900925e6303a71c1e0ace02ef8602b3be2` |
| PE image bytes | 301,981,696 | 302,030,848 |
| PE timestamp | 1791226072 | 1792187070 |

Hashes identify the compared files; runtime admission still uses the specific native shape/ownership checks. No executable-hash lock was added.

The disk EXE is protected. A normal startup was used to obtain a private read-only snapshot of the loaded module with query/read-only process access 0x1010. No process writes, injected hooks, suspensions, remote function calls or protection changes were used for that inspection. The full image read had no failed ranges. An existing old readable snapshot matched the old PE timestamp/image size and all 121 original source byte-check preimages.

Private evidence, including the original disk EXE, lives under `Build/TitleUpdate-20261030`. The old readable reference remains recorded in `old-mapped-reference.json`. These files, game data and saves are excluded from source/release assets. A mapped snapshot is a comparison artifact, not a runnable unprotected EXE.

## Native binding work

- Native function candidates were located using neighboring Windows unwind-function length/layout identities, then checked against readable old/new instructions and source guards. Global references were decoded independently from matching callers; a uniform address shift was not used.
- 258 address identities were resolved across named bindings and inline consumers. That includes crowd masks/renderer identity, game/team/clock state, starting-attendance and native option/TDF interfaces, audio event/mixer/arena functions, camera/HFA interfaces, skill/actor/event data, replay state and lifecycle callbacks, and settings/menu dispatch.
- Existing instruction shapes and structure-field accesses were retained. Twenty-four byte-check fragments and the replay-update prefix needed relocated RIP/call bytes. Hook lengths and owned trampoline behavior remain unchanged.
- Every one of the 42 settings-provider dispatch slots matched its expected relocated function. A separate bounded read-only native-cache inspection found the exact Scriptpod header, category head and fallback bytes, with stable identities. No menu script was modified during inspection.
- The standalone mapped-image audit passed **178/178** existing byte/pointer checks on the updated loaded module. The audit covers source byte guards, static hook prefixes, explicit module pointer relationships and all 42 menu dispatch slots. It does not execute native code or certify heap layouts during gameplay.

An initial automatic address inventory also matched the cannon trigger hash to an unrelated numeric code location. Its independent golden-event regression caught that false positive. The hash was restored unchanged and excluded from relocation; the final full build passes. Source events, hash keys and user tuning are not reinterpreted as addresses.

## Checks

- Full automatic native/managed regression build with warnings treated as errors.
- 828 full-game attendance simulations and 497 native configuration-reader cases.
- Fourteen Python tests, including the new mapped-image audit's malformed/truncated/architecture/address-parser/base-relocation failure cases.
- New native tests tie the encoded replay-manager displacement to the named replay global and the launch-hook call to its native team-setup target, in addition to existing executable trampoline/stack/argument tests.
- Isolated MMC adapter ABI, metadata, resources, extraction and storage checks passed using the current SDK at the main MMC root. The old nested SDK path is no longer present; no installed MMC files were changed.
- Packaged native resource and staged adapter hashes are recorded in BUILD-INFO.json.

Private verification logs: `Build/Build-209-final.log`, `Build/PythonTests-209.log`, `Build/HostCheck-209-current-mmc.log` and the title-update evidence folder.

## First live check

Close the game and MMC. Replace the adapter in the current MMC `Plugins` folder with the staged 2.0.9 DLL, restart MMC and launch a fresh regular-season Dynasty game. Pause at opening kickoff. Confirm the runtime version and all feature-activation lines before playing. Then check several normal plays for clean sound/motion and no whistle hitch, plus a non-skipped TD/takeaway full-screen-to-inset replay. Save/resume, halftime, overtime and 2020 Mode remain part of final live acceptance.

The native functions have not been called by DGA inside the updated game in this turn. Audible reactions, rendered motion/replay sequencing, full in-match object lifecycles and FPS still require the user's live check. No installation, publication or replacement of the old draft release was performed.
