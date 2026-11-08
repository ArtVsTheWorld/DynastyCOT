# DYNASTY COT by Ace

Small additions to offline Dynasty, packaged as one MMC plugin with four independently selectable modules.

| Module | What it adds |
|---|---|
| Beyond the Field | Social posts, PFF-style player grades, scouting fog, letter grades, rating colors and clearer money. |
| Gameday Atmosphere | Crowds that arrive, react and travel with the matchup. |
| Extra Settings | More offline Dynasty controls, including clocks saved separately for each Dynasty. |
| Expanded Practice | Full drives and scrimmages from the normal practice menu. |

This project publishes **test prereleases**. The `-private` version suffix is retained from the development package format; the repository and its authorized prerelease downloads are public. Passing automated checks does not establish live-game or overhaul compatibility. See [live acceptance](LIVE-ACCEPTANCE.md) and the release's compatibility notes before testing.

The local, unpublished [0.1.14 candidate](docs/release/0.1.14-private.md) rebalances Player Analytics using released native production, adds prestige-based grading uncertainty and introduces the PFF Social account. See the [actual-sim audit and grading limits](docs/player-analytics-0.1.14.md). Analytics remains team-only; Social's National view keeps its 250-card limit. [Hot reload remains an audit/recommendation](docs/hot-reload-template-review.md), with no prototype included.

The 0.1.8 Social Media rendering fix remains in place; the unsupported list/sidebar replacement stays disabled.

The local, unpublished [0.1.7 candidate results](docs/verification/0.1.7-private.md) cover the compact installer and weekly feed changes, including remaining acceptance limits.

Read the [candidate verification](RC-VERIFICATION.md), [full-season tweet audit](Modules/DynastyEnhancer/Docs/RC016-REVIEW.md) and [before/after examples](Modules/DynastyEnhancer/Docs/RC016-SAMPLES.md) for what improved and what remains weak.

## Install, update and undo

Download the versioned setup EXE from [Releases](https://github.com/ArtVsTheWorld/DynastyCOT/releases). Close the game and MMC. Select the **MMC Mod Manager folder**, normally named `MMC_ModManager_vX.X.X.X`, choose your modules and install. The installer contains its matching package.

Updates are optional: explicitly check for updates, inspect a release's notes and compatibility information, then choose whether to download and install it. An older installed version remains usable. The installer supports selected-module removal, full removal and restoring a previous backup. Settings, account identities, published history and save files are retained. See [release and rollback documentation](docs/release/README.md).

The ZIP and `Install.ps1` are also provided for advanced use. Neither source builds nor automated tests install into a live MMC folder by default.

## Persistence and compatibility

Display names have changed; internal module IDs, paths and compatibility identities remain stable. One `Plugins/DynastyCOT.dll` adapter loads enabled modules. Its selections are in `Plugins/DynastyCOT/settings.json`. Native binaries stay outside MMC's recursive plugin scan. Existing external helpers retain their verified identities and established data locations.

All offensive and defensive clock choices belong to a validated unsigned LeagueID, never a save filename. New Dynasties default to offensive On and defensive 15 seconds. Prior global clock preferences are not imported into every Dynasty. Published news and recruit history are preserved separately from settings.

**Visiting Fan Strength (%)** is under Gameday Atmosphere's Attendance settings. `100` preserves the normal matchup/travel model. `0–200` reduces or increases visitor odds for newly generated fixture plans; `0` retains the model's minimum support and protected groups. Saved fixture plans, native neutral-site behavior and crowd history remain intact. Change this before generating a new matchup to compare settings.

The [compatibility contracts](Compatibility/README.md) preserve native scope checks, fingerprints, hook ownership and helper verification. Installer preservation of another mod's files is separate from gameplay compatibility.

## Build from source

Required: PowerShell 7, Python 3, Node 24.18.0, .NET Framework's compiler, MSVC/Windows SDK and a compatible local MMC SDK. Required third-party notices are retained under `Vendor` and module license directories.

```powershell
.\Test.ps1 -Suite Baseline
.\Test-Clocks.ps1 -MmcRoot 'C:\path\to\MMC-SDK'
.\Build.ps1 -MmcRoot 'C:\path\to\MMC-SDK' -Package
```

`Build.ps1` builds all four modules before packaging. It rebuilds Practice before generating its exact hook contract and compiling Beyond the Field, verifies native source hashes and helper identities, and stages outputs from source. It never borrows installed mod payloads. `cot.product.json` describes the product; `Baselines/source-files.json` and `changes.json` identify imported inputs and deliberate changes.

Some inherited UI generation and verification require private, legally obtained research fixtures and generated adapter inputs. These prerequisites are documented locally by the ignored `Baselines/private-fixtures.json`; they are not included in public source. Saves, captures, game assets, SDKs, generated histories and credentials must never be committed. A public checkout alone cannot run the full private-fixture verification. See [distribution documentation](docs/release/README.md) for reproducible release steps and the public/private boundary.

The AIO adapters target exact AIO 1.4.1 programs; unknown variants remain rejected. Generation tools are under `Modules/DynastyEnhancer/Tools`. Travel-state fallback generation uses `Data/Geography/state-points-2025.json`.
