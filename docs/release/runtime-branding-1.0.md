# Runtime branding and upgrade contract

The public release is called **Dynasty COT 1.0**. Its semantic/package and MMC version is **1.0.0**, plugin name **Dynasty COT**, author **Ace**.

| Module | Distributed runtime | Compatibility kept |
|---|---|---|
| Beyond the Field | `BeyondTheFieldNative-v103.dll`, `BeyondTheFieldRecruitFog-v1.dll` | Existing `DynastyEnhancer` module ID, data paths and verified Analytics ABI |
| Gameday Atmosphere | `GamedayAtmosphereNative-2.1.9.dll` | Existing crowd settings, fixture plans, history and native scope checks |
| Extra Settings | `ExtraSettings.Persistence.exe` | Existing per-Dynasty clock/playcall profiles and helper SHA verification |
| Expanded Practice | `ExpandedPractice.Runtime.exe` | Existing practice settings, hook ownership and helper SHA verification |

Only `Plugins/DynastyCOT.dll` is an MMC plugin. Native DLLs remain outside its recursive plugin scan. Build output DLLs also use the new names; old timestamped development pairs remain recognized only by the development sibling contract. Exported interface names, namespaces and native ownership markers are compatibility identifiers, not new filenames, and remain stable.

Beyond the Field and Gameday Atmosphere use the same tested managed Windows loader policy. Each request validates the exact game executable, expected full DLL path and matching Windows loader-module mapping. Loading another version of the same runtime role is rejected. Beyond the Field's social and fog DLLs may coexist; uncertain load completion does not trigger another attempt. Payload hashes remain verified before loading. No external Beyond the Field loader is distributed.

Extra Settings and Expanded Practice helpers do ongoing persistence/gameplay work. They are retained, not replaced with unverified new hooks. Renaming their installed filenames does not change their SHA-verified executable contents.

Upgrade retirement uses the installed receipt or a known payload hash. Every replacement/removal participates in the installer transaction and backup. Unknown active legacy plugins stop installation before mutation; unknown inactive runtime files are preserved and reported. User settings, published posts, accounts, recruit records, crowd histories and saves are not binary payloads and are not swept up by retirement.

The Beyond the Field launcher now writes inside `Plugins/DynastyEnhancer/Logs`. Other module diagnostics already use module-owned data/runtime subdirectories. Historical folder names intentionally stay stable to avoid breaking existing data.

Run `Tests/test_runtime_upgrade.py` against a real source-built 1.0 stage for the retained 0.1.19 upgrade, fault injection, preservation, uninstall and exact rollback checks. Its optional synthetic mode is development-only and cannot count as release acceptance.
