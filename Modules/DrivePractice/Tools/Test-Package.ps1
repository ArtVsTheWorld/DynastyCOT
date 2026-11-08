param([string]$MmcRoot=$env:MMC_ROOT)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$folderName='SituationalPractice';$exeName='SituationalPractice.Runtime.exe'
$source=Get-Content (Join-Path $root 'Plugin/AssemblyInfo.cs') -Raw
if($source -notmatch 'PluginVersion\("([^"]+)"\)'){throw 'Version missing'}
$version=$Matches[1]
$zipPath=Join-Path $root "Release/Drive-Based-Practice-$version.zip"
Add-Type -AssemblyName System.IO.Compression.FileSystem
$zip=[IO.Compression.ZipFile]::OpenRead($zipPath)
$expected=@("$folderName.dll",'README.txt',"$folderName/$exeName","$folderName/SituationalPractice.Runtime.pdb")
try {
 $names=@($zip.Entries | ForEach-Object FullName)
 if($names.Count -ne $expected.Count -or (Compare-Object ($names|Sort-Object) ($expected|Sort-Object))){throw 'Unexpected ZIP contents (settings/history must never be shipped)'}
}finally{$zip.Dispose()}
$fixture=Join-Path $root ('Evidence/package-checks/'+[Guid]::NewGuid().ToString('N'))
$unpacked=Join-Path $fixture 'unpacked';$target=Join-Path $fixture 'Plugins'
[IO.Compression.ZipFile]::ExtractToDirectory($zipPath,$unpacked)
$kept=@("$folderName/settings.ini","$folderName/Diagnostics/history.log","$folderName/history/session.json",'OfflinePlaycallSettings/dynasties.json',"$folderName/DynastyPlayClock.ini","$folderName/OfflineDynastySettings.ini")
foreach($name in $kept){$p=Join-Path $target $name;New-Item -ItemType Directory -Path (Split-Path $p -Parent) -Force|Out-Null;[IO.File]::WriteAllText($p,'existing user data '+$name)}
. (Join-Path $PSScriptRoot 'Install-Package.ps1')
Merge-PluginFiles $unpacked $target
Merge-PluginFiles $unpacked $target
foreach($name in $kept){if([IO.File]::ReadAllText((Join-Path $target $name)) -ne ('existing user data '+$name)){throw "User file overwritten: $name"}}
if(Test-Path (Join-Path $target 'README.txt')){throw 'README should not be installed into Plugins'}
if((Get-FileHash (Join-Path $target "$folderName.dll")).Hash -ne (Get-FileHash (Join-Path $unpacked "$folderName.dll")).Hash){throw 'Plugin DLL not replaced'}
# Run the installed DLL's verifier against fixture files. No helper/game launch.
$compiler=Join-Path $env:WINDIR 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
$check=Join-Path $fixture 'RuntimePackageChecks.exe'
& $compiler /nologo /platform:x64 "/out:$check" (Join-Path $PSScriptRoot 'RuntimePackageChecks.cs')
if($LASTEXITCODE -ne 0){throw 'Package checker compilation failed'}
& $check $MmcRoot (Join-Path $target "$folderName.dll") $target $folderName $exeName
if($LASTEXITCODE -ne 0){throw 'Runtime validation failed'}
# Missing source files must be reported before any program file is copied.
Remove-Item -LiteralPath (Join-Path $unpacked "$folderName/$exeName")
$partial=Join-Path $fixture 'partial';$rejected=$false
try{Merge-PluginFiles $unpacked $partial}catch{$rejected=$_.Exception.Message -like '*Missing release file*'}
if(!$rejected -or (Test-Path $partial)){throw 'Incomplete package partially installed'}
Write-Host 'PASS ZIP allowlist, helper outside DLL, repeat folder merge, DLL replacement, settings/log/history preservation, missing-source preflight; fixture only.'
