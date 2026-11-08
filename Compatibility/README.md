# Compatibility maintenance

The generated [inventory](inventory.json) collects validation ranges, extracted patch spans, vtable slots, native address references and source lines for hook ownership and UI contracts. Regenerate with `python Tools/export_compatibility.py`; use `--check` to detect stale sources. Symbolic and dynamically located UI patches still require review of the cited code. The inventory does not certify a game build or overhaul.

Practice patches 15 bytes at RVA `0x25a3e60`, while both Enhancer engines fingerprint 32 bytes there. COT verifies three exact Practice entry jumps, complete emitted bodies and image/data relocations before normalizing the first 15 bytes in a local comparison buffer. It never rewrites the other hook. The remaining 17 bytes must still match. The contract passed 4,949 ownership/mutation checks, including relocated images, unreadable pages, changed tails and every body byte.

`Tools/Refresh-PracticeContract.ps1` exports actual compiled helper code, regenerates the contract and runs C++ checks. Rebuild both Enhancer engines after a contract change. Packaging rejects a Practice helper with a different SHA-256.

Extra Settings and Enhancer share the settings path through distinct ownership layers: the settings text detour and list/writer vtable. The emitted writer-chain fixture checks all 32 clock pairs and Dynasty isolation. Practice's controller bridge recognizes the exact Extra Settings bridge; unknown ownership remains an activation failure. Atmosphere uses a separate named pause-tile output.

Enhancer's creation-page extension uses the same list/writer hooks. The
read-only EnhancementCreationScope guard requires the native local response,
offline getter fingerprint, initial-creation flag, inactive career service and a
stable allocated LeagueID. It does not make a global preset or write native game
settings. Tests reject loaded, online, inactive, unknown-mode and changing-ID
contexts. Weekly Advance and its neighboring core screens use independent UI
banks, so failure in an unrelated callback cannot block an otherwise valid one.

For an overhaul report, retain the COT package version, exact overhaul version/order, module switches, launcher status and component activation logs. Compare both patch spans and full validation ranges; reproduce the smallest failing combination. Add a narrowly validated adapter for a specific build or disable the affected feature. Do not bypass fingerprint checks or infer gameplay compatibility from launcher startup.
