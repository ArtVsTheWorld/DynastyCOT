param([Parameter(Mandatory=$true)][string]$MmcRoot,[string]$PackageRoot=$PSScriptRoot)
$ErrorActionPreference='Stop'
$pluginRoot=[IO.Path]::GetFullPath((Join-Path $MmcRoot 'Plugins'))
if(-not(Test-Path -LiteralPath (Join-Path $MmcRoot 'FrostyCore.dll'))){throw 'Choose the MMC Mod Manager folder containing FrostyCore.dll.'}
$packagePlugins=if(Test-Path -LiteralPath (Join-Path $PackageRoot 'DynamicGamedayAtmosphere.dll')){$PackageRoot}else{Join-Path $PackageRoot 'Plugins'}
$source=Join-Path $packagePlugins 'DynamicGamedayAtmosphere.dll'
if(-not(Test-Path -LiteralPath $source)){throw 'The package DLL is missing.'}
$version=[Reflection.AssemblyName]::GetAssemblyName($source).Version.ToString(3)
# Inspect metadata under .NET Framework; never execute the supplied adapter.
$repo=Split-Path -Parent $PSScriptRoot
$verifierFolder=Join-Path $repo 'Build\PackageVerifier'
New-Item -ItemType Directory -Path $verifierFolder -Force | Out-Null
$verifier=Join-Path $verifierFolder 'VerifyPackageResources.exe'
& (Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe') /nologo /reference:System.IO.Compression.dll /reference:System.IO.Compression.FileSystem.dll ('/out:'+$verifier) (Join-Path $repo 'Tools\VerifyPackageResources.cs')
if($LASTEXITCODE -ne 0){throw 'Adapter metadata verifier compilation failed.'}
$loaderMode=& $verifier --adapter $source
if($LASTEXITCODE -ne 0){throw 'Unsupported or incomplete package adapter.'}
if($loaderMode -eq 'external'){
$helperRelative='DynamicGamedayAtmosphere\Runtime-'+$version+'\LoadDynamicGamedayAtmosphere.exe'
$sourceHelper=Join-Path $packagePlugins $helperRelative
if(-not(Test-Path -LiteralPath $sourceHelper -PathType Leaf)){throw 'The matching runtime folder is missing. Copy both the DLL and its matching folder from the complete ZIP.'}
$helperTarget=[IO.Path]::GetFullPath((Join-Path $pluginRoot $helperRelative))
if(-not $helperTarget.StartsWith($pluginRoot+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw 'Invalid runtime destination.'}
New-Item -ItemType Directory -Path (Split-Path -Parent $helperTarget) -Force | Out-Null
Copy-Item -LiteralPath $sourceHelper -Destination $helperTarget -Force
if((Get-FileHash -LiteralPath $sourceHelper).Hash -ne (Get-FileHash -LiteralPath $helperTarget).Hash){throw 'The copied runtime loader did not verify.'}
}
$backup=Join-Path $pluginRoot 'DynamicGamedayAtmosphere\Backups'
New-Item -ItemType Directory -Path $backup -Force | Out-Null
$staged=Join-Path $pluginRoot 'DynamicGamedayAtmosphere.dll.pending'
Copy-Item -LiteralPath $source -Destination $staged -Force
if((Get-FileHash -LiteralPath $source).Hash -ne (Get-FileHash -LiteralPath $staged).Hash){throw 'The copied DLL did not verify.'}
$moved=@()
try {
    foreach($name in @('DynamicAttendance.dll','DynamicGamedayAtmosphere.dll')){
        $target=[IO.Path]::GetFullPath((Join-Path $pluginRoot $name))
        if(-not $target.StartsWith($pluginRoot+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw 'Invalid plugin destination.'}
        if(Test-Path -LiteralPath $target){
            $saved=Join-Path $backup ($name+'-'+[DateTime]::UtcNow.ToString('yyyyMMdd-HHmmssfff')+'.bak')
            Move-Item -LiteralPath $target -Destination $saved -ErrorAction Stop
            $moved+=@{Original=$target;Backup=$saved}
        }
    }
    Move-Item -LiteralPath $staged -Destination (Join-Path $pluginRoot 'DynamicGamedayAtmosphere.dll') -ErrorAction Stop
} catch {
    [array]::Reverse($moved)
    foreach($item in $moved){if(-not(Test-Path -LiteralPath $item.Original)){Move-Item -LiteralPath $item.Backup -Destination $item.Original}}
    if(Test-Path -LiteralPath $staged){Remove-Item -LiteralPath $staged}
    throw
}
Write-Output 'Installed Dynamic Gameday Atmosphere. Previous DLLs are backed up. Restart both MMC and the game. On first Update 3 startup, old DGA settings are backed up and reset to current defaults; subsequent custom settings are preserved.'
