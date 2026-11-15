# Dynasty COT development

- Author is Ace. Work only inside this repository unless the user authorizes installation.
- The approved architecture is a collection hub/optional installer for the four independently selectable standalone DLLs. New collection installs do not install DynastyCOT.dll. Preserve old host migration/rollback and the modules' existing runtime contracts.
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

## Release-note style

- All four COT standalone modules use Ace's approved Update-style notes: an `Update N` heading, bold feature-category headings and short bullets focused on what players notice.
- Use simple, natural language. Group related changes; use nested bullets for choices, ranges or reveal stages. Describe important changes since the last public release, rather than pasting the full feature guide or leading with implementation details.
- User reaffirmed this style for every future release. Continue Hotfix numbering within the existing Update heading (for example, `Update 2 Hotfix 2` and `Update 5 Hotfix 2`); keep bold categories and short player-facing bullets.
- Check claims against the code and distinguish unconfirmed live behavior. Use the user-authored Beyond the Field Update 2 and Gameday Atmosphere Update 5 GitHub release descriptions as style references.

## Private test handoffs

- User preference: deliver the four standalone plugin DLLs directly together in one folder, with direct file links. Do not package private test kits as ZIPs containing other ZIPs. Keep installer, reports and integrity metadata separate from the four-DLL test folder.
