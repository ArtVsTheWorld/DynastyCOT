# Building the plugin

Use Windows 10 or 11 with Git, Visual Studio 2022 or newer Build Tools (**Desktop development with C++** and a Windows SDK), and the .NET Framework C# compiler included with Windows. Building the final adapter also needs your local MMC installation.

From the repository folder:

```powershell
.\Source\Build.ps1 -Automatic
```

This builds the native runtime, optional separate-loader comparison tool, and all native, settings and diagnostic checks. Output goes to `Build/Release213` for build 2.1.3. The folder suffix is generated from `Source/Product.json`; use that version for later builds. There are no sibling-project or MinHook dependencies; the old build step compiled unused objects and has been removed.

To compile the MMC adapter and make a download:

```powershell
.\Source\Package.ps1 -MmcRoot 'C:\Your\MMC' -ReleaseName 'Update 4 Candidate'
```

Set the working version in `Source/Product.json` and build it first. Use a private candidate name while testing; the command above only packages the current working source. `MmcRoot` must contain `FrostyCore.dll` and `FrostySdk.dll`. Those files belong to MMC and are not included here. Packaging does not install anything unless you add `-Install`.

The ZIP opens directly to the adapter DLL, README and feature list. The adapter embeds its native runtime and handles loading directly. Copy only the DLL into Plugins; the Markdown files are optional reading. Runtime files are prepared automatically under MMC/DGA-Runtime, outside the recursive plugin scan. Settings, logs and history stay under Plugins/DynamicGamedayAtmosphere. Do not upload game files, MMC assemblies, dumps, saves or local runtime history.

## Extra checks

```powershell
python -m unittest discover -s Tests -p 'test_*.py'
.\Tools\VerifyRelease.ps1 -Archive '.\Release\DynamicGamedayAtmosphere-Update3.zip'
```

`Tests/HostChecks.cs` checks an adapter against an installed MMC SDK. `Tests/InstallTests.ps1` exercises installation/rollback in a disposable folder under Build. Neither replaces a live game test. The GitHub Windows workflow builds native code and runs tests without a game or MMC installation; it does not create or publish releases.

For the default single-DLL layout, `Tools/CheckMmcHost.ps1 -MmcRoot 'C:\Your\MMC'` compiles/tests the adapter in an isolated Build fixture, including runtime extraction outside Plugins, repeat-extraction discovery, settings and diagnostics. `-ExternalLoader` retains the isolated separate-loader comparison fixture. It does not install into MMC or create a release archive.

## Historical tags

`v1.20.1` and `v1.26.1` were imported from preserved source snapshots. Their original build scripts retain the original local tooling paths. The portable build setup is on `main`; actual game code at each tag is preserved for comparisons. Release archives are the original files, not rebuilds.

The first Git commits record the import date. They are not an invented history of every development session. Public names are Release, Update 1, Update 2, etc.; numbered builds identify source, runtime payloads and bug reports.
