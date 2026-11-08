# Hot reload 1.1: COT integration recommendation

Audit scope: the user-supplied `hot-reload-template-1.1.zip`, SHA-256 `d3832957058be6fba477af4ef37d1f77ba401d8f6e66a8e11f66f0d99a8c6e05`. Reviewed its README, API/payload headers, hook declarations, resident stub, entry patcher and thread/stack walker. The archive contains 27 source/documentation files. No template code was installed, executed, incorporated into COT or distributed. Supplied test-result text is author evidence, not a test performed here.

## What the code supports

- A resident stub owns native entry detours; replaceable payloads provide handlers. A payload's hook list can add previously unseen hooks during a session. Removing a declaration leaves the installed detour in place and routes it to the original function.
- Existing hook names must retain their textual signature (whitespace is ignored) and target. Changed signatures or targets are refused with `RESTART NEEDED`; this is a restart boundary, not something to bypass.
- The source checks unique signature-pattern matches, rejects unsupported instruction relocation and short branches, compares entry bytes again immediately before writing, and suspends/enumerates other threads around the patch. Its own installed patch ranges cannot overlap.
- It closes a dispatcher gate, drains payload calls, invokes shutdown, initializes a new payload and switches handlers. Failed initialization attempts to restart the old payload. A stack/range guard can retain an old DLL instead of unloading code that may still be executing.
- Dynamic generic hooks have a finite ABI/capacity contract: up to 20 arguments and 64 dynamic hook slots per session. Aggregate/vector/other unusual ABI cases require independent verification; textual signature agreement is not ABI type proof.

Initial adoption of the resident stub still requires installing it and restarting once. Stub/host changes, changed hook ABI, changed targets and unsupported function prologues remain restart cases.

## Gaps that matter for COT

| Finding in supplied source | COT consequence |
| --- | --- |
| `file_sig` compares file size and modification time. Copy validation checks a stable copy and x64 DLL format; it is not a cryptographic trust check. `LoadLibraryExW` runs before payload exports/version are inspected. | Verify an approved manifest and payload digest before loading. Keep COT's explicit, user-controlled update flow. A developer file watcher must never become an automatic public updater. |
| `hr_patch_install_ex` compares against bytes observed when building that trampoline. This guards races, not a known stock-game fingerprint. Internal overlap detection identifies this stub's hooks. | Retain exact game-code fingerprints, expected vtable owner checks and known Practice normalization. Do not treat a stable foreign detour as original game code. |
| `shutdown_current` logs shutdown errors/crashes and continues; shutdown failure is not a hard veto on switching. A stack guard only detects threads it can inspect at that moment. | COT must refuse reload unless workers, UI callbacks, pending timers/jobs and all external references are quiescent. Retaining a DLL prevents some use-after-unload failures but does not prevent two generations from mutating shared state. |
| New hooks can be installed before the old payload is drained and before new initialization succeeds. Dropped hooks persist as passthrough. | A failed candidate can still change the resident hook inventory. Validate the complete proposed hook manifest first; report this separately from restoring the old payload. |
| The payload owns correct state-blob contents and thread cleanup. The stub cannot prove that saved pointers remain valid. | Keep saves, settings, account identities, publication ledgers and published history in their existing stores. Use versioned plain data for transient reload state; never transfer payload-code pointers or borrowed game objects. |
| COT currently pins native DLLs, owns vtable slots, clones native UI definitions and supplies callbacks/data retained by the game. Its existing stop paths deliberately refuse unsafe teardown. | Moving handlers into a payload requires a resident COT dispatcher and an inventory of every retained reference. Adding the template DLL next to current DLLs would create competing ownership, not safe reload. |
| No redistribution license is present in the supplied ZIP. | Obtain an explicit license/permission before copying template implementation into the public project. This does not block an audit or COT's current release. |

## Recommendation

Pursue a separate, opt-in development prototype only after the current candidate is reviewed. Keep one resident COT owner for existing hooks and use typed dispatch interfaces for replaceable logic. Begin with pure content/selection logic and immutable data; defer native UI definitions, presentation callbacks and game-data hooks until their lifetime rules are proven.

Require an explicit reload action, exact same-process Dynasty eligibility, verified payloads, compatible state schemas, a safe hub/menu exit, complete worker joins and a successful ownership/retention audit. Reject unknown owners and restart-required changes. Test add/remove/change hooks, in-flight callbacks, long-running originals, shutdown/init failure, stale external callbacks, interrupted copies, untrusted hashes, multiple modules and Dynasty switching in an isolated harness before any game trial.

The useful promise is **some compatible handler changes and new supported hooks without another relaunch after initial adoption**. It cannot promise that every COT change can reload live. COT 0.1.10 does not include this prototype.
