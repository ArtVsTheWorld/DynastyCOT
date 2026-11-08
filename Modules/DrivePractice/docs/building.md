# Building

Use 64-bit Windows with .NET Framework 4.x and a matching MMC Mod Manager install.
The build uses MMC's `FrostyCore.dll` and `FrostySdk.dll`. They stay on your computer
and are not included in this repository. No extra DLL goes in the release ZIP.

```powershell
.\Build.ps1 -MmcRoot "C:\Path\To\MMC"
.\Test.ps1
.\Package.ps1
```

You can set `MMC_ROOT` instead of passing `-MmcRoot`. Build output goes into
`Release/`; test output goes into `Evidence/`. Both folders are ignored by Git.

The package command checks the DLL version and makes a ZIP containing only the
plugin DLL, its matching runtime folder and a short `README.txt`. It does not install or launch anything.

`Test.ps1` checks drive rules, CPU play choices, Stats, Scrimmage, first-snap
counting, first-down line ownership and diagnostics.
These tests use local test data and do not open or patch the game.

For generated native hooks and compatibility checks:

```powershell
python -m pip install -r requirements-dev.txt
.\Test.ps1 -NativeChecks -BaselineImage "private/working.pe.bin"
```

Without `-BaselineImage`, the native hook tests still run with mocks, but the
region compatibility fixture checks are skipped. Fixtures are generated locally
from a verified capture and are never committed. GitHub runs the SDK-free managed
checks and checks that the native manifest matches the source.

The helper EXE is separately shipped and verified against a SHA-256 compiled into the plugin.
No executable resource is extracted. Install by merging the runtime folder, never
replacing/deleting it: it also contains settings, logs and diagnostic history.
`Tools/Install-Package.ps1` copies only the release program files; README stays outside Plugins.

After building and packaging, run `.\Tools\Test-Package.ps1 -MmcRoot <MMC-folder>`
to check the exact ZIP, both helper failure cases and fixture-only install/upgrade preservation.
This test does not install into MMC or launch the game.
