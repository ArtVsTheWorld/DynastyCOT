$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$out=Join-Path $root 'Build/Compatibility'
New-Item -ItemType Directory -Path $out -Force | Out-Null
$source=Get-Content -LiteralPath (Join-Path $root 'Modules/DrivePractice/Plugin/AssemblyInfo.cs') -Raw
$version=[regex]::Match($source,'PluginVersion\("([^"]+)"\)').Groups[1].Value
$helper=Join-Path $root ('Modules/DrivePractice/Release/SituationalPractice-'+$version+'/SituationalPractice/SituationalPractice.Runtime.exe')
$csc=Join-Path $env:WINDIR 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
$exe=Join-Path $out 'ExportPracticeContract.exe'
& $csc /nologo /platform:x64 /reference:System.Web.Extensions.dll ('/out:'+$exe) (Join-Path $root 'Tools/ExportPracticeContract.cs')
if($LASTEXITCODE -ne 0){throw 'Practice contract exporter did not compile'}
& $exe $helper (Join-Path $out 'practice-contract.json')
if($LASTEXITCODE -ne 0){throw 'Practice contract export failed'}
& python (Join-Path $root 'Tools/generate_practice_contract.py')
if($LASTEXITCODE -ne 0){throw 'Practice contract generation failed'}
& (Join-Path $root 'Tools/Test-PracticeContract.ps1')
if($LASTEXITCODE -ne 0){throw 'Practice contract checks failed'}
