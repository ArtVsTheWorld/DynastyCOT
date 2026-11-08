$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot '..\Source\BuildTools.ps1')
$fixture=Join-Path ([IO.Path]::GetTempPath()) ('DgaBuildTools-'+[Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path (Join-Path $fixture 'Include\wdf'),(Join-Path $fixture 'Include\10.0.99999.0') -Force | Out-Null
foreach($version in @('10.0.9500.0','10.0.26100.0')){
 foreach($relative in @('um\Windows.h','ucrt\stdio.h','shared\sdkddkver.h')){
  $path=Join-Path $fixture ('Include\'+$version+'\'+$relative)
  New-Item -ItemType Directory -Path (Split-Path -Parent $path) -Force | Out-Null
  Set-Content -LiteralPath $path -Value 'test fixture'
 }
 foreach($relative in @('um\x64\kernel32.lib','ucrt\x64\ucrt.lib')){
  $path=Join-Path $fixture ('Lib\'+$version+'\'+$relative)
  New-Item -ItemType Directory -Path (Split-Path -Parent $path) -Force | Out-Null
  Set-Content -LiteralPath $path -Value 'test fixture'
 }
}
$selected=Get-DgaWindowsSdk -KitsRoot $fixture
if($selected.Name -ne '10.0.26100.0'){throw 'SDK discovery must skip driver/partial SDK folders and sort complete versions numerically.'}
$empty=Join-Path $fixture 'Missing';$failed=$false
try {Get-DgaWindowsSdk -KitsRoot $empty | Out-Null}catch{$failed=$true}
if(-not $failed){throw 'Missing SDK must fail clearly.'}
Write-Output 'PASS: Windows SDK discovery ignores WDK/partial installs, sorts numeric versions and reports missing SDKs.'
