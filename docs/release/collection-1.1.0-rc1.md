# Dynasty COT collection — private 1.1.0-rc1

The collection now manages the same four standalone DLLs users can drag into Plugins. It no longer installs a DynastyCOT.dll host. The installer is about 472 KiB and downloads only selected modules that need changing. Module development remains in this workspace; the standalone repositories remain their individual download/support pages.

Pinned module releases: Beyond the Field 1.0.7, Gameday Atmosphere 2.1.11, Extra Settings 0.2.10 and Expanded Practice 0.8.21. Their public ZIPs and DLLs are reused byte-for-byte. No game-module code, native fingerprints, scope checks, hook ownership, helper verification, publication policies or data formats were changed for this collection candidate.

## Audit and implementation

The older installer installed one host and a shared runtime payload, so its update unit was the entire collection. The new download manager treats each module's repository/version/ZIP/DLL as an independent unit. Already-current standalone files are adopted without download or rewrite. Explicit update checks show per-module notes and version choices; no update service or automatic module downloads are installed.

Migration checks cover old-COT enabled choices, canonical/legacy/renamed plugin copies, matching program-file rollback snapshots, unrecognized-copy review, actual running processes and safe paths. Every enabled old-COT module must be accounted for before the host is removed. Unknown/missing receipts or program bytes stop for review. Real settings, saves, account identities, histories, crowd plans and module data folders are excluded from replacement/removal/restore. Existing old-COT backups are retained.

The compact main window has four rounded cards, detected MMC location, modern executable picker and one Install / Update action. Remove offers selected/full removal. Backups restores prior managed versions; Details exposes activity. The crown is the Windows app icon only.

## Verification

- **224 isolated checks passed:** fresh installs; adopting standalones; legacy/renamed duplicate retirement; old-COT conversion; enabled-module preservation; old runtime snapshots and restoration after an actual support-file change; selected/full uninstall and rollback; every commit-stage failure; locked files; exact-hash unknown-file review; stale review rejection; tampered backups/ZIPs/proofs; path traversal; Windows path casing; interrupted-operation recovery; a simulated future release and old-host rollback after its cache changes; DLL-version mismatches; cancellation; explicit downgrades; retained settings/history; no-module-ZIP update checks; and GDA-only download independence.
- **Actual native-window layout checks passed at 100%, 125% and 150%:** all module cards, descriptions and footer actions fit. Fonts and geometry were scaled in the tests. The executable contains only its catalog/icon resources, not a module bundle or host DLL.
- **Real anonymous GitHub path passed:** four public releases were discovered and downloaded, proofs/ZIP/DLL hashes checked, all four DLLs installed into an isolated fixture, GDA changed independently to an explicitly selected older release, then rolled back. BTF's DLL stayed unchanged.
- **Manual computer-use inspection:** actual MMC location and all four installed versions were correctly detected; main cards/footer, the release chooser and modern file picker were observed. The tool could not cancel the owned picker using its out-of-parent button bounds, so the self-built preview process was closed; no installation was initiated. Collapsed Unix release-note newlines were corrected and regression-tested.
- Public-source boundary and source/artifact hashes are checked separately. Integrity metadata is supplied beside the private EXE. No SDKs, game assets, saves, captures, generated histories, credentials or runtime logs are shipped in the installer/source changes.

These are installer/fixture results. They do not certify native activation or all overhaul/game combinations. No live installation, save overwrite, game-process patch or installer release publication was performed.

## Remaining acceptance before installer publication

1. Run the private installer against a real writable MMC installation and confirm current standalone adoption. Launch and check all selected modules; make sure there is no additional COT host provider.
2. Test a real older-COT conversion and migration rollback with its existing settings/history. The file fixtures pass, including matching external runtime restoration; the actual MMC/game path remains to be checked.
3. Check selected removal/restore and per-module update choices from another user's Windows setup. Actual permissions, display scaling and overhaul behavior require their own acceptance.

The README now presents COT as the collection front door, links all four individual repositories and explains that this new installer is still a private candidate. The existing 1.0.x releases retain their legacy layout and build path for reproduction/rollback.

See [installation, migration, logs and testing instructions](../collection-installer.md).
