# Dynasty COT 1.0 verification

## Published payload and source

The release uses the exact source-built package Ace installed and tested after restarting MMC and the game. No binary was rebuilt for the documentation/publication update.

- Implementation revision: `901bba4bdf7920a9af9b7f548f6ded07abe2f231`.
- ZIP SHA-256: `61671b9441ae893dbc54cc8ecb6201d415e44926f99e033c464ab4ce54ad8fb7`.
- Installer SHA-256: `f047b4decb53e431ade455a63f89d8ed54b76446fe7ec32821c0934addc68af7`.
- Contents: 159 manifested files plus the manifest. One MMC entry: **Dynasty COT**, author **Ace**, version **1.0.0**.
- Earlier unpublished 1.0 builds remain local for comparison/rollback. The older public test prerelease remains on GitHub.

## Automated verification

| Check | Result |
|---|---|
| Original standalone sources | 983 original source hashes unchanged |
| Native builds | All four modules source-built; no installed payloads borrowed |
| Module selections and paths | 106 configuration/planner/path checks, all 16 module combinations |
| MMC registration, helpers and resources | 20 assembly checks |
| Packaged Analytics ABI/build pair | 11 checks |
| Final money/Analytics adapters | 48 real-runtime currency-class checks and 21 production return-dispatch checks |
| Dynasty presentation scope | 1,669 real options/store/scope checks; saved preferences preserved |
| Offline modes and touchdown policy | 665 production admission/type/history/replay-exclusion checks |
| Expanded Practice | 154 mode/persistence/scoring checks; 7,445 actual x64 hook executions in each practice context |
| Transactional installer | 116 clean/update/removal, preservation, tamper/path and rollback checks |
| Legacy runtime migration | 135 checks against actual 0.1.19 and released Gameday 2.0.35 payloads |
| Same-version update | 123 actual previous-1.0 to final-1.0 checks, including failure rollback and later user-data preservation |
| Setup window/engine | 36 actual GUI/engine checks, selected/full removal, folder detection and rollback |
| Layout | 100/125/150% and bounded 1080/1152 working areas; final 150%/1080 capture reviewed, footer visible |
| Optional updater | 95 discovery/download/install/rollback and failure-case checks in isolated fixtures |
| Release metadata/privacy | 18 checks; private fixtures, paths and credentials excluded from public evidence |
| Social Media | 461 Node tests passed in the final package validation |

The full native suites and affected final reruns passed, including rating publication/focus/badge fitting, NIL edits/redraw, fog/color isolation, Analytics, crowd/travel/protected groups, sound/replays, per-Dynasty clocks and practice ownership. Automated tests do not establish universal live-game compatibility.

## Targeted live confirmation

After installing the final package and restarting both MMC and the game, Ace reported: "Fixes look to be working" and authorized publication. Read-only checks then matched the installed host, both Beyond the Field DLLs, Gameday native and both helpers to the exact six release hashes. Both Beyond the Field natives and Gameday native were loaded in the fresh process, with no failed/unavailable events in the two current Beyond the Field activation traces.

Earlier targeted tests confirmed main-menu Drive Practice/Scrimmage/Default behavior, successful Analytics-card Back navigation, retained NIL formatting, fitted uppercase grades, selection highlight and no numeric bucket flashes. A neutral/rivalry Texas-Oklahoma Play Now smoke test confirmed roar/cheers and shake, with Dynasty crowd-history hashes unchanged. Native scope reads confirmed that leaving Dynasty clears its active career flag and LeagueID even though both frontend screens can report mode -1/session 3.

The latest user confirmation is targeted acceptance, not independent observation of every checklist item. The final runtime leaves touchdown replay/celebration presentation native, retains other eligible additions, starts fresh Play Now arrival plans at 75%, and scopes effective fog/color/dollar options to an owned active Dynasty without editing saved choices.

## Practical limits and continued review

- No new live FPS/menu benchmark is claimed. The earlier Play Now trace had a 226 ms one-time seat-preparation maximum and 250 ms crowd-apply maximum; sound/camera callbacks stayed below 4 ms in that sample.
- Broader home/neutral venues, scoring returns, long sessions, save/load and other game/overhaul versions remain useful regression coverage. AIO 1.4.1/TeamBuilder have targeted support, not blanket certification.
- The earlier Play Now trace rejected an in-game card adapter bank despite an expected body hash. Final scope avoids first-time attachment outside Dynasty; its in-Dynasty-game coverage remains a review item. No fingerprint was bypassed.
- Analytics is team-only. Its nonworking Player Stats shortcut stays hidden.
- Public anonymous updater verification is performed after publication in an isolated fixture; it never installs into live MMC.

Use the [regression checklist](../../LIVE-ACCEPTANCE.md), [mode/scope investigation](1.0-offline-practice-playnow.md), [1.0 Discord post](dynasty-cot-1.0-discord.md), and [Gameday Update 4 post](gameday-update4-discord.md). Both Discord drafts stay below 2,000 characters. The [Gameday comparison](gameday-update4-code-review.md) uses the actual released 2.0.35 tag.
