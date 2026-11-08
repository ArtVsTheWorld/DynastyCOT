param([Parameter(Mandatory=$true)][string]$MmcRoot)
$ErrorActionPreference='Stop'
$root=$PSScriptRoot
$MmcRoot=[IO.Path]::GetFullPath($MmcRoot)
& (Join-Path $root 'Test.ps1') -Suite Baseline
pwsh -NoProfile -File (Join-Path $root 'Modules/ExtraDynastySettings/Build.ps1') -MmcRoot $MmcRoot
if($LASTEXITCODE -ne 0){throw 'Per-Dynasty clock build failed.'}
$out=Join-Path $root 'Build/ClockChecks'
New-Item -ItemType Directory -Path $out -Force | Out-Null
$csc=Join-Path $env:WINDIR 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
& $csc /nologo /platform:x64 /reference:System.Web.Extensions.dll ('/out:'+(Join-Path $out 'Profiles.exe')) (Join-Path $root 'Modules/ExtraDynastySettings/Plugin/DynastyClockProfiles.cs') (Join-Path $root 'Modules/ExtraDynastySettings/Plugin/ClockPreferences.cs') (Join-Path $root 'Tests/DynastyClockProfileChecks.cs')
if($LASTEXITCODE -ne 0){throw 'Clock profile checks did not compile.'}
$profileOutput=@(& (Join-Path $out 'Profiles.exe') (Join-Path $out 'Fixtures'))
if($LASTEXITCODE -ne 0){throw 'Clock profile checks failed.'}
$profileOutput | Write-Output
$match=[regex]::Match(($profileOutput -join ' '),'PASS: (\d+) per-Dynasty')
if(!$match.Success){throw 'Missing profile check result.'}
$profileCount=[int]$match.Groups[1].Value
pwsh -NoProfile -File (Join-Path $root 'Tools/Test-DynastyClocks.ps1') -MmcRoot $MmcRoot
if($LASTEXITCODE -ne 0){throw 'Emitted native clock checks failed.'}
$native=Get-Content -LiteralPath (Join-Path $out 'native-result.json') -Raw | ConvertFrom-Json
$helperExe=Join-Path $out 'ClockHelperChecks.exe'
& $csc /nologo /platform:x64 ('/out:'+$helperExe) (Join-Path $root 'Tests/ClockHelperChecks.cs')
if($LASTEXITCODE -ne 0){throw 'Clock helper checks did not compile.'}
& $helperExe (Join-Path $root 'Modules/ExtraDynastySettings/Release/OfflineDynastySettings-0.2.10/OfflineDynastySettings/OfflineDynastySettings.Persistence.exe') $out
if($LASTEXITCODE -ne 0){throw 'Clock helper checks failed.'}
$sources=@('Modules/ExtraDynastySettings/Plugin/DynastyClockProfiles.cs','Modules/ExtraDynastySettings/Plugin/DynastyClockNative.cs','Modules/ExtraDynastySettings/Shared/Clock/DynastyPlayClockAction.cs','Modules/ExtraDynastySettings/Shared/Clock/ExtendedClock.cs','Modules/ExtraDynastySettings/Plugin/CombinedClockRuntime.cs','Modules/ExtraDynastySettings/Plugin/DefensivePlayClock.cs','Modules/ExtraDynastySettings/Plugin/PersistenceHelper.cs')
$hashes=foreach($source in $sources){@{path=$source;sha256=(Get-FileHash -LiteralPath (Join-Path $root $source)).Hash.ToLowerInvariant()}}
$report=@{atUtc=[DateTime]::UtcNow.ToString('O');profileChecks=$profileCount;nativeChecks=$native.passed;helperChecks=10;gameAccess=$false;sources=$hashes;remaining=@('Live gameplay and save/load acceptance','Final shared COT host and installed layout validation')}
[IO.File]::WriteAllText((Join-Path $out 'verification.json'),($report | ConvertTo-Json -Depth 5),[Text.UTF8Encoding]::new($false))
Write-Output 'PASS: clock build, profile checks and emitted native checks. Live acceptance remains outstanding.'
