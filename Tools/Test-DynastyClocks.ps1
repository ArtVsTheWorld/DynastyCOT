param([Parameter(Mandatory=$true)][string]$MmcRoot)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$out=Join-Path $root 'Build/ClockChecks'
New-Item -ItemType Directory -Path $out -Force | Out-Null
[Reflection.Assembly]::LoadFrom((Join-Path $MmcRoot 'FrostyCore.dll')) | Out-Null
[Reflection.Assembly]::LoadFrom((Join-Path $MmcRoot 'FrostySdk.dll')) | Out-Null
$assembly=[Reflection.Assembly]::LoadFrom((Join-Path $root 'Modules/ExtraDynastySettings/Release/OfflineDynastySettings-0.2.10/OfflineDynastySettings.dll'))
$type=$assembly.GetType('DynastyPlayClock.DynastyPlayClockAction',$true)
$flags=[Reflection.BindingFlags]'Static,NonPublic'
foreach($pair in @(@('off-read','ExportDynastyRead',$false),@('def-read','ExportDynastyRead',$true),@('off-write','ExportDynastyWrite',$false),@('def-write','ExportDynastyWrite',$true))){
    $bytes=$type.GetMethod($pair[1],$flags).Invoke($null,[object[]]@([long]0x140000000,[long]0x200000000,[bool]$pair[2]))
    [IO.File]::WriteAllBytes((Join-Path $out ($pair[0]+'.bin')),[byte[]]$bytes)
}
foreach($method in @('BuildListUpdated','BuildWriterUpdated','BuildRuntimeUpdated','BuildEnumUpdated','BuildDurationUpdated','BuildCapUpdated','BuildCpuUpdated','BuildTransitionUpdated')){
    $bytes=$type.GetMethod($method,$flags).Invoke($null,[object[]]@([long]0x140000000,[long]0x200000000))
    if($bytes.Length -ge 1024){throw ($method+' exceeds its native code slot: '+$bytes.Length)}
    [IO.File]::WriteAllBytes((Join-Path $out ($method+'.bin')),[byte[]]$bytes)
    Write-Output ($method+': '+$bytes.Length+' bytes')
}
$descriptor=$type.GetMethod('BuildData',$flags).Invoke($null,[object[]]@([long]0x200000000))
[IO.File]::WriteAllBytes((Join-Path $out 'BuildData.bin'),[byte[]]$descriptor)
& python (Join-Path $root 'Tests/test_dynasty_clock_native.py')
if($LASTEXITCODE -ne 0){throw 'Native clock tests failed'}
