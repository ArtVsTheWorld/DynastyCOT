# Beyond the Field 1.0.7 — Recruiting feedback and GPI

Changes since the published 1.0.3 release:

- **Choose how recruit interest looks.** Default keeps the normal game display and is the initial choice. Recruit Feedback replaces exact interest bars/rank numbers with broad standings: Favorite, Contender, In the Mix, Outside Looking In and Long Shot. Schools are grouped by standing, then alphabetically within a group; multiple schools can share a tier. Existing movement arrows remain. Rank Only hides the bars but keeps real rankings. Recruiting Fog must be enabled; Blind hides interest altogether.
- **Find prospects you already scouted.** Off-board, uncommitted prospects show their scouting-progress icon in the Target column. It updates after further scouting/removal without needing to flip pages. Names, active-board markers and commitment logos stay intact. This quality-of-life feature is always on.
- **Cleaner recruiting cards.** Full Overview labels align under Influence; Prospect List cards show the same broad standings instead of Interest Unknown. Recruit Feedback and Blind hide the misleading Projected Cutoff divider. Default/Rank Only retain it. Real lockout notices and the game's recruiting rules remain unchanged.
- **GPI branding.** Player Analytics and its fictional Social Media account now use Gridiron Performance Index (GPI), @GPIndex and the G icon. Grade calculations, completed-week restrictions and weekly feed budgets are unchanged. Existing account identities and published history are preserved.
- **More reliable startup.** BTF waits briefly for the exact supported native settings code instead of permanently stopping during early game initialization. Unsupported code/hook owners still fail safely.
- Keeps the Position Changes fix, edited Current NIL formatting, player-card fog/colors, capitalized grades and faster Back navigation. Settings, saves, accounts and published history stay in place. Saved interest choices are preserved; Default applies when no choice has been saved.

**Install/update:** close the game and MMC, then replace only BeyondTheField.dll in the versioned Mod Manager's Plugins folder. Keep all data folders. Use one BTF provider; disable bundled BTF in COT if using this standalone. Matching support files are embedded. To roll back, close MMC/game and restore your saved older DLL; retain settings/history.

**Verification:** Ace installed 1.0.6, restarted MMC/game and reported it working. 1.0.7 adds the Default preference/label change, verified with settings/menu/persistence regressions. Recruiting response, emitted UI, startup, native composition, existing-data and standalone/COT coexistence checks passed; integrity evidence ships beside the ZIP. Successful tests do not certify every game/overhaul combination.

**Known limits:** the full Overview header's stage-progress hiding remains a live-review item. An earlier Windows graphics timeout/game hang has unresolved causality; this release does not claim to fix it. Save/reload, multi-coach and other overhaul combinations need their own acceptance checks. No touchdown replay experiment is included.
