param([string]$PackageRoot)
$ErrorActionPreference='Stop'
$packagePlugins=if(Test-Path -LiteralPath (Join-Path $PackageRoot 'DynamicGamedayAtmosphere.dll')){$PackageRoot}else{Join-Path $PackageRoot 'Plugins'}
$build=Join-Path (Split-Path -Parent $PSScriptRoot) 'Build'
$testRoot=Join-Path $build ('InstallerTest-'+[Guid]::NewGuid().ToString('N'))
$plugins=Join-Path $testRoot 'Plugins'
New-Item -ItemType Directory -Path $plugins -Force | Out-Null
Set-Content -LiteralPath (Join-Path $testRoot 'FrostyCore.dll') -Value 'test root marker'
$legacy=Join-Path $plugins 'DynamicAttendance.dll'
$current=Join-Path $plugins 'DynamicGamedayAtmosphere.dll'
Set-Content -LiteralPath $legacy -Value 'legacy test payload'
Set-Content -LiteralPath $current -Value 'previous test payload'
$installer=Join-Path (Split-Path -Parent $PSScriptRoot) 'Source\Install.ps1'
$audienceFile=Join-Path $plugins 'DynamicGamedayAtmosphere\History\audience-foundations.txt'
New-Item -ItemType Directory -Path (Split-Path -Parent $audienceFile) -Force | Out-Null
Set-Content -LiteralPath $audienceFile -Value 'DGA-AUDIENCE-1 0'
$audienceHash=(Get-FileHash -LiteralPath $audienceFile).Hash
& $installer -MmcRoot $testRoot -PackageRoot $PackageRoot
if(Test-Path -LiteralPath $legacy){throw 'Legacy adapter still active'}
if((Get-FileHash -LiteralPath $current).Hash -ne (Get-FileHash -LiteralPath (Join-Path $packagePlugins 'DynamicGamedayAtmosphere.dll')).Hash){throw 'Installed plugin mismatch'}
$backup=Join-Path $plugins 'DynamicGamedayAtmosphere\Backups'
if(@(Get-ChildItem -LiteralPath $backup -File).Count -ne 2){throw 'Missing old DLL backups'}
if((Get-FileHash -LiteralPath $audienceFile).Hash -ne $audienceHash){throw 'Audience foundations changed during installation'}
# A failure after moving the old adapter must put it back.
Set-Content -LiteralPath $legacy -Value 'rollback test payload'
$lock=[IO.File]::Open($current,[IO.FileMode]::Open,[IO.FileAccess]::Read,[IO.FileShare]::Read)
$failed=$false
try {& $installer -MmcRoot $testRoot -PackageRoot $PackageRoot}catch{$failed=$true}finally{$lock.Dispose()}
if(-not $failed){throw 'Expected locked destination to reject replacement'}
if(-not(Test-Path -LiteralPath $legacy)){throw 'Failed update did not restore legacy DLL'}
if((Get-Content -LiteralPath $legacy -Raw).Trim() -ne 'rollback test payload'){throw 'Wrong rollback content'}
if((Get-FileHash -LiteralPath $audienceFile).Hash -ne $audienceHash){throw 'Audience foundations changed during rollback'}
Write-Output 'PASS: rename installation, payload verification, old DLL retirement, backup and audience-foundation preservation, failed-update rollback.'
