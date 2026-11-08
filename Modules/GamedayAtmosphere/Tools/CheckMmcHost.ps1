param([Parameter(Mandatory=$true)][string]$MmcRoot,[switch]$DirectLoader,[switch]$ExternalLoader,[string]$BuildName='')
$ErrorActionPreference='Stop'
if($DirectLoader -and $ExternalLoader){throw 'Choose one private loader variant'}
if(-not $ExternalLoader){$DirectLoader=$true}
$root=Split-Path -Parent $PSScriptRoot;$source=Join-Path $root 'Source'
$product=Get-Content -LiteralPath (Join-Path $source 'Product.json') -Raw | ConvertFrom-Json
if(-not $BuildName){$BuildName='Release'+$product.version.Replace('.','')}
if($BuildName -notmatch '^[A-Za-z0-9-]+$'){throw 'Invalid build folder name'}
$build=Join-Path $root ('Build\'+$BuildName)
$fixture=Join-Path $root ('Build\HostCheck-'+[Guid]::NewGuid().ToString('N'));$plugins=Join-Path $fixture 'Plugins'
New-Item -ItemType Directory -Path $plugins -Force | Out-Null
$csc=Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
$native=$product.name+'Native-'+$product.version+'.dll'
$resources=@(('/resource:'+(Join-Path $build $native)+',Attendance.'+$native),('/resource:'+(Join-Path $build 'LoadDynamicGamedayAtmosphere.exe')+',Attendance.LoadDynamicGamedayAtmosphere.exe'),('/resource:'+(Join-Path $source 'DynamicGamedayAtmosphere.ini')+',Attendance.DynamicGamedayAtmosphere.ini'),('/resource:'+(Join-Path $build 'DgaDiagnostics.exe')+',Attendance.DgaDiagnostics.exe'))
$variant=@();$extraSource=@()
if($DirectLoader -or $ExternalLoader){$resources=@($resources | Where-Object {$_ -notlike '*,Attendance.LoadDynamicGamedayAtmosphere.exe'})}
if($DirectLoader){$variant=@('/define:DIRECT_RUNTIME_LOADING');$extraSource=@((Join-Path $source 'ManagedRuntimeLoader.cs'))}
if($ExternalLoader){
    $variant=@('/define:EXTERNAL_RUNTIME_LOADER')
    $loader=Join-Path $build 'LoadDynamicGamedayAtmosphere.exe'
    $identity=Join-Path $fixture 'ExternalLoaderInfo.cs'
    $hash=(Get-FileHash -LiteralPath $loader -Algorithm SHA256).Hash
    Set-Content -LiteralPath $identity -Encoding ascii -Value ('namespace DynamicGamedayAtmosphere { internal static class ExternalLoaderInfo { internal const string Sha256="'+$hash+'"; } }')
    $extraSource=@($identity)
    $externalFolder=Join-Path $fixture ('DGA-Runtime\Runtime-'+$product.version)
    New-Item -ItemType Directory -Path $externalFolder -Force | Out-Null
    Copy-Item -LiteralPath $loader -Destination (Join-Path $externalFolder 'LoadDynamicGamedayAtmosphere.exe')
}
& $csc /nologo /target:library /platform:x64 @variant ('/out:'+(Join-Path $plugins 'DynamicGamedayAtmosphere.dll')) ('/reference:'+(Join-Path $MmcRoot 'FrostyCore.dll')) ('/reference:'+(Join-Path $MmcRoot 'FrostySdk.dll')) @resources (Join-Path $source 'MmcPlugin.cs') (Join-Path $source 'LaunchDiagnostics.cs') (Join-Path $source 'PluginSettings.cs') (Join-Path $source 'PluginStorage.cs') @extraSource (Join-Path $build 'BuildInfo.cs')
if($LASTEXITCODE -ne 0){throw 'Adapter compilation failed'}
& $csc /nologo ('/out:'+(Join-Path $fixture 'HostChecks.exe')) ('/reference:'+(Join-Path $MmcRoot 'FrostySdk.dll')) (Join-Path $root 'Tests\HostChecks.cs')
if($LASTEXITCODE -ne 0){throw 'Host check compilation failed'}
& (Join-Path $fixture 'HostChecks.exe') $MmcRoot $plugins
if($LASTEXITCODE -ne 0){throw 'Host ABI/resource/storage checks failed'}
if(Get-ChildItem -LiteralPath $plugins -File | Where-Object {$_.Extension -in @('.ini','.log')}){throw 'A DGA INI/log escaped its owned data folder'}
Write-Output ('PASS: isolated adapter verified at '+$plugins+'. No installed files or release archive changed.')
