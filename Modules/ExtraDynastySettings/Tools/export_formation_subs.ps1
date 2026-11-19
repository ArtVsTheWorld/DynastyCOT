param([Parameter(Mandatory=$true)][string]$Dll,[Parameter(Mandatory=$true)][string]$NativeBody,[Parameter(Mandatory=$true)][string]$Output,[Parameter(Mandatory=$true)][string]$MmcRoot,[long]$CodeAddress=0x160000000)
$ErrorActionPreference='Stop'
[void][Reflection.Assembly]::LoadFrom((Join-Path $MmcRoot 'FrostySdk.dll'))
[void][Reflection.Assembly]::LoadFrom((Join-Path $MmcRoot 'FrostyCore.dll'))
$assembly=[Reflection.Assembly]::LoadFrom([IO.Path]::GetFullPath($Dll))
$type=$assembly.GetType('DynastyPlayClock.DynastyPlayClockAction',$true)
$flags=[Reflection.BindingFlags]'NonPublic,Static'
[IO.Directory]::CreateDirectory([IO.Path]::GetFullPath($Output)) | Out-Null
$image=[long]0x140000000;$remote=$CodeAddress
foreach($kind in 0..5){
    $bytes=$type.GetMethod('BuildFormationSubsBridge',$flags).Invoke($null,[object[]]@([int]$kind,$image,$remote))
    [IO.File]::WriteAllBytes((Join-Path $Output "Bridge$kind.bin"),$bytes)
    "Bridge $kind`: $($bytes.Length) bytes"
}
$body=[IO.File]::ReadAllBytes([IO.Path]::GetFullPath($NativeBody))
foreach($selector in 2,12){
    $address=$remote+$(if($selector -eq 2){0x3000}else{0x3400})
    $bytes=$type.GetMethod('CloneTypedFormationItem',$flags).Invoke($null,[object[]]@($body,$image,$address,[int]$selector))
    [IO.File]::WriteAllBytes((Join-Path $Output "Item$selector.bin"),$bytes)
}
$find=$type.GetMethod('BuildFormationSubsFind',$flags).Invoke($null,@())
[IO.File]::WriteAllBytes((Join-Path $Output 'Find.bin'),$find)
$lookupPath=Join-Path ([IO.Path]::GetDirectoryName([IO.Path]::GetFullPath($NativeBody))) 'a507b9199506fc4bb54cf724a6f8b4b4c0d32fc076680a960b2a8dcf9ec718b9.bin'
$lookupBody=[IO.File]::ReadAllBytes($lookupPath)
$lookup=$type.GetMethod('CloneFormationLookup',$flags)
$lookupBytes=$lookup.Invoke($null,[object[]]@($lookupBody,$image))
[IO.File]::WriteAllBytes((Join-Path $Output 'Lookup.bin'),$lookupBytes)
foreach($case in 'length','opcode','target'){
    [byte[]]$bad=$lookupBody.Clone()
    if($case -eq 'length'){$bad=New-Object byte[] 159}
    elseif($case -eq 'opcode'){$bad[0x8d]=0x90}
    else{$bad[0x8e]=$bad[0x8e] -bxor 1}
    $rejected=$false;try{[void]$lookup.Invoke($null,[object[]]@($bad,$image))}catch{$rejected=$true}
    if(!$rejected){throw "Changed lookup clone accepted: $case"}
}
# The compiled plugin must not carry a legacy file gate of any kind.
if($null -ne $assembly.GetType('OfflineDynastySettings.FormationSubsPreferences',$false)){
    throw 'Legacy Formation Subs preferences type is still present'
}
$dllBytes=[IO.File]::ReadAllBytes([IO.Path]::GetFullPath($Dll))
foreach($encoding in [Text.Encoding]::ASCII,[Text.Encoding]::Unicode){
    $strings=$encoding.GetString($dllBytes)
    foreach($legacy in 'FormationSubs.ini','KickoffAndKickReturn'){
        if($strings.Contains($legacy)){throw "Compiled plugin still references $legacy"}
    }
}
# Exercise the launch gate in this test process. An invalid image base makes
# the first native read fail before allocating or publishing any hook.
$fixture=Join-Path ([IO.Path]::GetFullPath($Output)) ('data-' + [Guid]::NewGuid())
$storeType=$assembly.GetType('Ace.DynastyCOT.UserStore',$true)
$storage=$assembly.GetType('Ace.DynastyCOT.UserStorage',$true)
$storage.GetField('current',$flags).SetValue($null,$storeType.GetConstructor([type[]]@([string])).Invoke([object[]]@([string]$fixture)))
$settings=Join-Path $fixture 'ExtraSettings'
[IO.Directory]::CreateDirectory($settings) | Out-Null
$path=Join-Path $settings 'FormationSubs.ini'
$process=[Diagnostics.Process]::GetCurrentProcess()
try {
    $start=$process.StartTime.ToUniversalTime().Ticks
    $type.GetField('installedPid',$flags).SetValue($null,[int]$process.Id)
    $type.GetField('installedStart',$flags).SetValue($null,[long]$start)
    $type.GetField('installedBase',$flags).SetValue($null,[long]::MinValue)
    $unavailable=$type.GetField('formationSubsUnavailableStart',$flags)
    $install=$type.GetMethod('InstallFormationSubs',$flags)
    $settingsFiles=$assembly.GetType('OfflineDynastySettings.SettingsFiles',$true)
    $log=$settingsFiles.GetMethod('PathFor',$flags).Invoke($null,[object[]]@('', 'OfflineDynastySettings.log'))
    foreach($case in 'missing','KickoffAndKickReturn=Off','invalid text','KickoffAndKickReturn=On'){
        if($case -ne 'missing'){[IO.File]::WriteAllText($path,$case)}
        $unavailable.SetValue($null,[long]0)
        $before=if([IO.File]::Exists($log)){[IO.File]::ReadAllText($log).Length}else{0}
        $install.Invoke($null,[object[]]@([int]$process.Id))
        $line=[IO.File]::ReadAllText($log).Substring($before)
        if($unavailable.GetValue($null) -ne $start -or !$line.Contains('compatibility mismatch')){
            throw "Legacy preference affected the native preflight: $case"
        }
        if($case -eq 'missing'){
            if([IO.File]::Exists($path)){throw 'Missing legacy file was created'}
        }elseif([IO.File]::ReadAllText($path) -ne $case){
            throw "Legacy preference was modified: $case"
        }
    }
}finally{
    $process.Dispose()
}
"PASS missing, Off, invalid and On configurations reach the native guard and legacy data stays intact."
$clone=$type.GetMethod('CloneTypedFormationItem',$flags)
foreach($case in 'length','selector','type-call','helper-call'){
    [byte[]]$bad=$body.Clone();$selector=2
    if($case -eq 'length'){$bad=New-Object byte[] 214}
    elseif($case -eq 'selector'){$selector=3}
    elseif($case -eq 'type-call'){$bad[0x2e]=0x90}
    else{$bad[0x4d]=0x90}
    $rejected=$false
    try{[void]$clone.Invoke($null,[object[]]@($bad,$image,($remote+0x3000),[int]$selector))}catch{$rejected=$true}
    if(!$rejected){throw "Changed clone contract accepted: $case"}
}
"PASS built plugin has no legacy Formation Subs configuration gate."
"PASS invalid clone length/type and changed instruction-site rejection."
foreach($name in @('Queue','Boundary','Apply','Eligibility')){
    $bytes=$type.GetMethod("BuildController$name",$flags).Invoke($null,[object[]]@([long]0x140000000,[long]0x210000000,[long]0x210000c00))
    [IO.File]::WriteAllBytes((Join-Path $Output "$name.bin"),$bytes)
}

foreach($name in 'FormationPreview','ReturnSetMembership','PreviewFloat','FormationGroup','GroupedFormationSet','FormationSetName','TypedSetMembership'){
    $method=$type.GetMethod("Build$name",$flags)
    $args=if($name -eq 'PreviewFloat'){[object[]]@($image)}else{[object[]]@($image,$remote)}
    $bytes=$method.Invoke($null,$args)
    [IO.File]::WriteAllBytes((Join-Path $Output "$name.bin"),$bytes)
    "$name`: $($bytes.Length) bytes"
}
foreach($name in 'PreviewTreeFind','PreviewString'){
    $bytes=$type.GetMethod("Build$name",$flags).Invoke($null,@())
    [IO.File]::WriteAllBytes((Join-Path $Output "$name.bin"),$bytes)
}

[IO.File]::WriteAllBytes((Join-Path $Output "KickoffName.bin"),[Text.Encoding]::ASCII.GetBytes("Kickoff`0"))
[IO.File]::WriteAllBytes((Join-Path $Output "ReturnName.bin"),[Text.Encoding]::ASCII.GetBytes("Kickoff Return`0"))
