# Dynasty COT development

- Author is Ace. Work only inside this repository unless the user authorizes installation.
- The approved architecture is one MMC host with four optional modules, retaining existing runtime designs initially.
- User decisions: ALL offensive play clock modes and defensive clock durations are per Dynasty; use a validated LeagueID, never save filenames. New Dynasties default to offensive On and defensive 15 seconds.
- Do not import global clock choices into every Dynasty or persist values when no validated Dynasty is active.
- Dynasty Enhancer is unreleased: legacy Enhancer SETTINGS migration is unnecessary. History is separate and remains preserved.
- Standalone source repositories and installed plugins are reference inputs; do not modify them.
- Preserve offline/mode eligibility, instruction fingerprints, ownership checks and helper integrity checks. Do not bypass unknown hooks.
- Keep native runtime binaries outside MMC's recursive plugin scan in the final COT layout.
- Source baseline checks compare imported files with Baselines/source-files.json. Record deliberate source changes in Baselines/changes.json.
- Imported module build scripts still build standalone baselines. They do not create an installable COT product.
- SDKs, game captures, saves, generated history, logs and build outputs must not be committed or packaged.
- No test may patch the live game by default. Live acceptance is a separate, explicitly reported stage.
