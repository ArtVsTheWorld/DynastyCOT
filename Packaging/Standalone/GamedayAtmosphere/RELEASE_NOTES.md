# Gameday Atmosphere 2.1.9 - Update 4

Compared with the last published standalone release, **v2.0.35 (Update 3 Hotfix)**.

- **Traveling supporters:** new visitor sections and scattered groups use fanbase demand, distance and matchup appeal. Big fanbases handle long trips better; strong home crowds resist takeovers. Main visitor blocks mix in some home fans rather than becoming one solid block.
- **Visiting Fan Strength:** a 0-200% control, with 100% using the normal model. It affects new plans, while existing plans and protected groups stay intact.
- **More natural late crowds:** winning visitors can still leave during a blowout, at a slower rate than the losing home crowd.
- **Sound and shake:** added shake requests accompanying crowd sound. Scoring cheers receive priority, background roar weakens with a growing home deficit, and away supporters have their own reactions.
- **Basic offline Play Now:** arrivals, departures, sound and shake work without requiring Dynasty's travel/season calculations. With arrivals enabled, fresh games use a 75% opening crowd model; visiting-fan appearance remains native.
- **Touchdown presentation:** extra replay requests skip touchdowns so native touchdown presentation remains in control. Other eligible extra replays stay optional.
- **Simpler installation:** put `GamedayAtmosphere.dll` in Plugins. Matching runtime files are embedded; the external loader EXE is unnecessary. Existing settings/history are retained without a reset or retune.

The ZIP contains only the DLL, a short README and a complete code-checked feature list. Move the old `DynamicGamedayAtmosphere.dll` outside Plugins and keep its data folder. Use one Gameday provider; disable COT's Gameday module if using this standalone alongside its other features.

Validation: 90 packaging/storage/guard checks including four existing regression suites, plus all-four standalone coexistence fixtures. Ace reported the standalone packaging working. This does not certify every game build, stadium or overhaul. The crowd engine is the same 2.1.9 engine shipped with COT 1.0.2.

Source comparison: old tag `v2.0.35`, commit `193662a9af440ef9cdc6903171e1a30b8acb0164`; current maintained source is pinned in `source-release.json`. User-facing changes were checked against `AwayCrowd.h`, `VisitorPlans.h`, `SettingsMenuOptions.inc`, `NativeRuntime.cpp`, `ReactionAudioRuntime.h` and `MajorReplay.h`, not inferred from version numbers. Existing arrival, turnout, Fuller Crowds, halftime, 2020 Mode and general replay features are retained rather than presented as new.
