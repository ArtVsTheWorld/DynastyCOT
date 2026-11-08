# Windows setup

The versioned DynastyCOT-Setup EXE is the primary user installer. It embeds the
verified package ZIP, so the user can open one file, choose an MMC Mod Manager folder, select
modules and click Install / update. Existing choices load from that folder.
Backups appear in a picker for Restore backup. No terminal is shown.
Versioned MMC_ModManager folders take precedence over a parent MMC launcher.
Several manager versions require choosing the specific folder.

The green window is branded DYNASTY COT by Ace. Separate actions install/update,
remove checked modules, or remove all COT. Removal disables selected switches and
removes verified external payloads; full removal also removes the shared host.
Settings, histories, profiles and backups remain. Changed payloads are retained
and reported. Receipt path allowlists exclude user-data directories. Known legacy
0.1.0/0.1.1 hashes support removal before receipts existed. Transactions make
removal undoable, and updates retain ownership of older versioned runtimes.

The application uses Windows Forms and the .NET Framework already required by
MMC. It invokes the same transactional installer using Windows' built-in
PowerShell, in a hidden process. It does not change machine execution policies
or require PowerShell 7. Module choices are recorded in the same backup journal
as installation or removal.

The EXE pins its embedded ZIP's SHA-256 before extraction. Paths and entries are
checked during extraction, and the existing installer verifies the complete
manifest before writing to the selected MMC folder. Extraction uses a unique
temporary directory that is removed when setup closes.

Build with Tools/build_installer.py after Tools/finalize_package.py. The normal
Build.ps1 -Package workflow now builds and checks the setup EXE too. Product
metadata comes from cot.product.json; the shared host and setup use that version.

Tests/test_setup.py drives the actual form buttons under a Windows message loop.
All actions are restricted to a marked fixture folder for this test. It checks
loading existing selections, saving new checkbox values, installing, populating
backups, selected/all removal, restoring each action, Mod Manager detection,
literal path quoting and rejecting an unsafe fixture.
Tests/test_installer.py also checks module-choice rollback and validation.

Build/current-setup.json and Release/setup-verification.json identify the exact
EXE, source hashes and embedded ZIP hash. The app can be opened for visual review
without changing an installation; selecting Install / update is a separate action.
