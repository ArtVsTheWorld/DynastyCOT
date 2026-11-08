param([string]$MmcRoot=$env:MMC_ROOT)
$ErrorActionPreference='Stop'
if(!$MmcRoot){throw 'Pass -MmcRoot with your MMC folder, or set MMC_ROOT.'}
foreach($name in @('FrostyCore.dll','FrostySdk.dll')){if(!(Test-Path -LiteralPath (Join-Path $MmcRoot $name))){throw "Missing $name in MmcRoot"}}
$projectRoot=Split-Path -Parent $PSScriptRoot
$versionSource=Get-Content -LiteralPath (Join-Path $PSScriptRoot 'AssemblyInfo.cs') -Raw
if($versionSource -notmatch 'PluginVersion\("([^"]+)"\)'){throw 'Plugin version missing'}
$version=$Matches[1]
$release=Join-Path $projectRoot ("Release\SituationalPractice-"+$version)
$compiler=Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
New-Item -ItemType Directory -Path $release -Force | Out-Null
$runtimeFolder=Join-Path $release 'SituationalPractice'
New-Item -ItemType Directory -Path $runtimeFolder -Force | Out-Null
$runtime=Join-Path $runtimeFolder 'SituationalPractice.Runtime.exe'
$runtimePdb=Join-Path $runtimeFolder 'SituationalPractice.Runtime.pdb'
& $compiler /nologo /target:winexe /platform:x64 /debug:pdbonly "/pdb:$runtimePdb" /reference:System.IO.Compression.dll "/out:$runtime" (Join-Path $PSScriptRoot 'DriveEngine.cs') (Join-Path $PSScriptRoot 'NativeRuntime.cs') (Join-Path $PSScriptRoot 'NativeCompatibility.cs') (Join-Path $PSScriptRoot 'DynastyControllerCompatibility.cs') (Join-Path $PSScriptRoot 'NativeDrivePolicy.cs') (Join-Path $PSScriptRoot 'PracticeRunner.cs') (Join-Path $PSScriptRoot 'SessionStats.cs') (Join-Path $PSScriptRoot 'DefenseChoice.cs') (Join-Path $PSScriptRoot 'Diagnostics.cs')
if($LASTEXITCODE -ne 0){throw 'Runtime compilation failed'}
$hashSource=Join-Path $projectRoot 'Evidence/ExpectedRuntime.cs'
New-Item -ItemType Directory -Path (Split-Path -Parent $hashSource) -Force | Out-Null
$helperHash=(Get-FileHash -LiteralPath $runtime -Algorithm SHA256).Hash.ToLowerInvariant()
Set-Content -LiteralPath $hashSource -Encoding Ascii -Value ('namespace SituationalPractice { internal static class ExpectedRuntime { internal const string Hash="'+$helperHash+'"; } }')
& $compiler /nologo /target:library /platform:x64 /reference:System.IO.Compression.dll "/out:$(Join-Path $release 'SituationalPractice.dll')"  "/reference:$(Join-Path $MmcRoot 'FrostyCore.dll')" "/reference:$(Join-Path $MmcRoot 'FrostySdk.dll')" (Join-Path $PSScriptRoot 'AssemblyInfo.cs') (Join-Path $PSScriptRoot 'PracticeAction.cs') (Join-Path $PSScriptRoot 'RuntimeFiles.cs') $hashSource (Join-Path $PSScriptRoot 'Diagnostics.cs')
if($LASTEXITCODE -ne 0){throw 'MMC plugin compilation failed'}


