> Historical implementation notes. Current candidate evidence is in [RC-VERIFICATION.md](RC-VERIFICATION.md).

# Dynasty COT private development package

The first private package has one registered MMC adapter and four optional modules. It is built from isolated source snapshots and built payloads. Standalone projects and installed plugins remain read-only references. This checkpoint does not claim live gameplay acceptance.

## Implemented

- Per-Dynasty clock JSON uses canonical nonzero unsigned LeagueID, excluding UInt32.MaxValue. Fresh defaults are offense On and defense 15. Legacy global INIs are ignored.
- Native game-thread callbacks publish atomic packed records with a nonblocking lock and sequence counter. Readers validate offline Dynasty scope and retain native behavior in practice/unknown scope. The detached read-only helper persists stable snapshots and retries failed writes without acknowledging them.
- One COT host verifies and starts only enabled modules. Module failures are isolated. Startup status explicitly differs from native activation.
- Source-built Enhancer scripts/content and bundled licensed Node replace installed-file package inputs. Fog startup remains independent of Node availability. Generated adapter changes remain in the build directory.
- Native DLLs stay outside recursive plugin discovery. Atmosphere retains its supported DGA-Runtime owner path. Existing EXE helper integrity checks and user-data layouts remain.
- The installer validates payloads, backs up owned files, preserves history/data/module choices, retires replaced adapters/native payloads, and activates COT last. Three injected failures restore the fixture. Manual rollback preserves later user edits.
- Enhancer starts fresh settings when migrating from its unreleased standalone. RuntimeHistory and RecruitResumeHistory are copied separately on first migration; conflicts abort before mutation. Recognized COT upgrades retain its newer history without re-importing stale standalone copies.
- Strict generated recognition resolves the known Practice text-setter / Enhancer fingerprint overlap. Unknown detours remain rejected. Shared compatibility references are generated under Compatibility.

## Automated evidence

Build/ClockChecks/verification.json records 86 profile checks, 60 emitted x64 checks and 10 detached-helper checks, with source hashes. Build/ClockChecks/sdk-regressions.log includes 79 actual writer/menu/storage checks, native consumers, lifecycle, diagnostics and controller fixtures. Those execute mock code without opening the game.

The native Practice contract passed 4,949 checks. COT core checks cover all 16 enable selections, failure isolation and paths. Package checks inspect the actual compiled assembly for one registration, resources, exact helpers and tamper/path rejection. Installer evidence is tied to the exact package manifest and isolated fixture root. Final package/archive evidence is written under Release and Build.

Original native baselines were reproduced for all four components. Current Enhancer native outputs are selected by cot.build-inputs.json and rebuilt with the generated Practice ownership contract. Packaging rejects stale native-source hashes or a different Practice helper.

## Limits and next acceptance

The 0.1.2 private build adds green DYNASTY COT by Ace branding, versioned MMC Mod
Manager discovery, and separate selected/full uninstall actions. File receipts
and legacy public payload hashes protect history and other mods during removal.
The EXE passed 29 button-driven UI/engine checks, including removal/restore round
trips and literal path handling. Final installer counts and exact manifest hashes
are in the versioned release verification. Visual review covered all three actions.

Feedback fixes restore clock rows in the validated offline Dynasty hub, reopen
play selection when switching custom practice modes from the line, add recruit
account star ratings, require fresh audible crowd voices for shake, and use a
weaker state-distance fallback. Same-state unknown-city trips get a modest bonus;
unknown locations stay neutral. Native practice checks total 5,717; the separate
exact Practice/Enhancer ownership contract remains 4,949 checks.

AIO 1.4.1 UI adapters normalize only decoded loader-relocated pointer operands
before checking the complete original callback hash. Nineteen read-only live
callback samples matched the offline source after exact normalization. Full topbar
dollar values remain AIO's; compact labels and Class & NIL bounds handle its
preformatted strings. A separate source-pinned Highest NIL Players row adapter
uses the existing roster grade options. All 378 Enhancer Node tests passed.

The 0.1.3 feedback build separates Weekly Advance / My School Update from
neighboring core-asset callbacks, retaining each method's full validation.
The user-opened panel's 793-byte body matched the supported fingerprint in a
read-only sample. A regression verifies that an unsupported neighboring callback
cannot block an independently validated adapter. No header/body gate is relaxed.

Initial offline creation already allocates a LeagueID while the career service
is inactive. The Enhancements menu now recognizes the exact local response type,
offline-method fingerprint, creation flag, native response state and stable
identity before binding the ordinary per-Dynasty profile. The initialized page
uses that same profile after creation. New-profile defaults are Social, Recruiting,
Roster and Team Fog On; both scales A+-; Colors and Dollars Off. Existing saved
choices are retained. Compact dollar labels include $, with unchanged slot bounds,
source budgets, editing values and default-Off behavior.

Shared host and setup versions are generated from cot.product.json. Final archive
verification records the state of original installed references rather than
requiring them as package inputs; an old reference can be missing after a user
reorganizes MMC without preventing a source-built package.

[LIVE-ACCEPTANCE.md](LIVE-ACCEPTANCE.md) lists all live gameplay, save/load, UI and overhaul checks as pending. No blanket overhaul compatibility is claimed. DGA 2.1.9 private Update 4, AIO 1.4.1 currency and AIO Broadcast Beta1 menu integration still need combined live checks.

Enhancer's inherited UI test suite requires ignored research fixtures and retains development-only absolute test-tool imports. They are not package inputs. The static compatibility inventory includes symbolic/dynamic source contracts that require human review during game updates.

No live installation or publishing was performed. A future authorized live acceptance session is the next stage after this private package.
