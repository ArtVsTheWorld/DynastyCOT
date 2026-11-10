# Gameday Atmosphere 2.1.11 — Play Now replay hotfix

Compared with the previous public standalone release, **2.1.9**.

- Fixed a replay eligibility check that admitted Dynasty but left offline Play Now out. With **Additional Replays** and **Full-Screen Big Play Replays** enabled, qualifying non-touchdown big plays in Play Now can use the added replay coverage.
- Replay diagnostics now admit offline Play Now too, so those decisions can be checked when troubleshooting.
- Existing crowd behavior, sound, camera shake, replay thresholds and saved settings/history are retained.
- Touchdown presentation remains controlled by the game. The unsuccessful private touchdown experiment and temporary timing helpers are **not included**. This release does not guarantee a replay after every touchdown.

**Update:** close both MMC and the game, then replace `Plugins/GamedayAtmosphere.dll` with the DLL from the ZIP. Keep your settings/history folders. Restart both applications. Use one Gameday provider; disable COT's Gameday module when using this standalone for the feature. The ZIP contains the DLL, README.txt and FEATURES.md; matching support files are embedded.

**Verification:** the production replay-root tests exercise Dynasty and Play Now, nested gain comparisons, blocked scopes/options, and unchanged touchdown predicates/shortcuts/insets. Replay, scope, menu and isolated managed packaging/storage/duplicate-provider checks pass. Settings and history remain byte-for-byte intact in upgrade fixtures. A 20-yard non-touchdown Play Now completion showed both full-screen and inset replays in the private test; the clean production ZIP was not live-installed during this publishing step. Other game/mod combinations remain unverified.

Version 2.1.10 was reserved for private touchdown tests. Public 2.1.11 uses a distinct native filename/cache so it cannot reuse that private runtime. No change to native fingerprints, ownership checks or supported offline scope.

Rollback: close MMC and the game and restore the 2.1.9 plugin DLL. Keep the existing data folders. The prior release remains available on GitHub.
