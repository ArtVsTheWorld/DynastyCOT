param([string]$ExtractedFolder,[string]$PluginsFolder)
$ErrorActionPreference='Stop'
function Merge-PluginFiles([string]$Source,[string]$Target) {
    # Copy only shipped program files. Never replace an entire runtime folder.
    $files=@('OfflineDynastySettings.dll','OfflineDynastySettings/OfflineDynastySettings.Persistence.exe')
    foreach($name in $files){if(!(Test-Path -LiteralPath (Join-Path $Source $name) -PathType Leaf)){throw "Missing release file: $name. Unzip the full download first."}}
    foreach($name in $files){
        $destination=Join-Path $Target $name
        New-Item -ItemType Directory -Path (Split-Path -Parent $destination) -Force | Out-Null
        Copy-Item -LiteralPath (Join-Path $Source $name) -Destination $destination -Force
    }
}
if($MyInvocation.InvocationName -ne '.') {
    if(!$ExtractedFolder -or !$PluginsFolder){throw 'Pass -ExtractedFolder with the unzipped release and -PluginsFolder with MMC Plugins.'}
    if(Get-Process CollegeFB27,MMCModManager -ErrorAction SilentlyContinue){throw 'Close the game and MMC before installing.'}
    Merge-PluginFiles $ExtractedFolder $PluginsFolder
    Write-Host 'Plugin DLL and runtime files merged. Existing settings, logs and history were kept. Launch normally.'
}
