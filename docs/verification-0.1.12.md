# Player Analytics card fix — 0.1.12

Author: Ace. Scope: the missing Player Analytics card, plus its build and private installer. Standalone projects and the live installation are not modified.

## Evidence and cause

The supplied screen shows a populated, sorted National Analytics table alongside an empty native card. Read-only loaded-runtime logs contained row-key, selection and sort events, with no successful portrait events. Read-only inspection of the exact loaded native callback showed that the old Analytics binding emits general Dynasty information (`userTeamName`), rather than the roster-card response.

The real roster-card response already belongs to COT Fog. Its native implementation creates a response object and writes player fields such as first/last name, portrait, team colors, OVR, position, class, dimensions and hometown. The fix dispatches recognized Analytics keys through that existing owner. It uses the same native object factory and typed field setters. Both allocation helpers have exact instruction fingerprints; ownership, offline/Dynasty checks, Practice compatibility normalization and sibling-build/helper verification remain in place. Analytics no longer claims the unrelated slot or a second roster-card hook.

A key must belong to the current filtered/sorted Analytics page and its current snapshot card map. Unknown keys use the original native callback. Each recognized response starts with a fresh native object so that missing fields cannot inherit another player's information. Capture adds the verified native hometown field and retains existing portrait IDs/measurements. PFF-style grades and their calculation inputs are unchanged.

## Verification

The focused native test exercises the actual `rosterPlayerHook`, not only the in-memory card map. It checks native output-pointer/return semantics, typed fields, two different players, National selection, sorting, stale fields, out-of-scope/missing keys, closed/disabled/unverified states, native fallback and allocation/factory failures. The same cases also run in the Analytics-only fixture. Public tests use synthetic identities; captured game data stays in ignored private evidence.

Full native build, Node/core checks and package checks are recorded in the private `Build/AnalyticsCards` evidence. Final installation/update/rollback results accompany the versioned release evidence. Passing offline checks is not a claim that the replacement has been observed in the live game.

Completed checks: 1,027 focused Analytics integration assertions, the actual shared Fog card-entry tests, the full native build and existing Fog/currency/grade checks, 425 Node tests, all 16 module selections, nine binary ABI/build-pair checks, 116 installer cases, 34 real setup GUI/engine checks, 95 isolated 0.1.11 → 0.1.12 update/rollback cases, and 15 release metadata/provenance checks. The full failed stress-test rerun passes. Source privacy/whitespace checks pass; original standalone inputs and loaded installation DLL hashes remain unchanged.

The existing settings-persistence stress test intermittently encountered Windows access-denied during an isolated atomic commit, as it did during 0.1.11 verification. This is separate from card dispatch; production settings behavior is not changed or bypassed by this fix. Final test results record the complete rerun.

The 0.1.11 editorial pack is byte-identical. No published posts, identities, history, settings or save files are rewritten. No DLL is injected into the running game.

## Live acceptance

1. Close the game/MMC, install 0.1.12 and restart. Open Stats & Records → Player Analytics.
2. On National/Week 0, highlight the reported TE, then a CB on another team. Confirm portrait, name, team, position, OVR/grade, class and available dimensions/hometown follow the selection.
3. Sort grades both ways, change to a team and Season/another week, then move between players again. Check that no preceding player's card remains.
4. Check Fog and Rating Colors On/Off, then leave Analytics and inspect a normal roster card. The normal native card must retain its original behavior.
5. Review optional archetype/physical/mental details separately. The fix populates captured card fields; it does not invent unavailable ability data. Report any remaining empty fields and whether the basic card now follows the row.

Successful replacement-card rendering and exact game/AIO/TeamBuilder compatibility remain pending these checks.
