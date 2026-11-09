param(
 [Parameter(Mandatory=$true)][string]$MmcRoot,
 [string]$PackageRoot=$PSScriptRoot,
 [switch]$Fixture,
 [string]$FixtureRoot='',
 [ValidateSet('None','AfterPayload','AfterLegacyDisable','AfterAdapter')][string]$FaultAtStage='None',
 [string]$Rollback='',
 [string]$ModuleChoicesJson='',
 [ValidateSet('Install','UninstallSelected','UninstallAll')][string]$Action='Install'
)
$ErrorActionPreference='Stop'
function Resolve-Owned([string]$Root,[string]$Relative){
 if([String]::IsNullOrWhiteSpace($Relative) -or [IO.Path]::IsPathRooted($Relative) -or $Relative.Contains(':') -or ($Relative -split '[/\\]') -contains '..'){throw 'Unsafe COT path'}
 $prefix=[IO.Path]::GetFullPath($Root).TrimEnd('\','/')+[IO.Path]::DirectorySeparatorChar
 $target=[IO.Path]::GetFullPath((Join-Path $prefix $Relative))
 if(!$target.StartsWith($prefix,[StringComparison]::OrdinalIgnoreCase)){throw 'COT path escapes selected root'}
 return $target
}
function Assert-NoLink([string]$Root,[string]$Path){
 $prefix=[IO.Path]::GetFullPath($Root).TrimEnd('\','/')+'\'
 $current=$Path
 while($current.StartsWith($prefix,[StringComparison]::OrdinalIgnoreCase)){
  if(Test-Path -LiteralPath $current){
   if((Get-Item -LiteralPath $current -Force).Attributes -band [IO.FileAttributes]::ReparsePoint){throw ('Linked path is not an installation target: '+$current)}
  }
  $current=Split-Path -Parent $current
 }
}
function Hash([string]$Path){return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()}
function Owned-Module([string]$Path){
 # Runtime data is deliberately outside this allowlist, even in a receipt.
 if($Path -eq 'Plugins/DynastyCOT.dll' -or $Path -match '^LICENSES/[^/]+[.]txt$'){return 'Core'}
 if($Path -match '^COT-Runtime/Shared/node[.]exe$' -or $Path -match '^COT-Runtime/DynastyEnhancer/((DynastyEnhancer|BeyondTheField)Native-v103[.]dll|(DynastyEnhancer|BeyondTheField)RecruitFog-v1[.]dll|LoadDynastyEnhancer[.]exe|dynasty-enhancer[.]json|Tools/[^/]+[.]mjs|Content/Runtime/[^/]+[.]json)$'){return 'DynastyEnhancer'}
 if($Path -match '^DGA-Runtime/Runtime-\d+[.]\d+[.]\d+/((DynamicGamedayAtmosphere|GamedayAtmosphere)Native-\d+[.]\d+[.]\d+[.]dll|DgaDiagnostics[.]exe)$'){return 'GamedayAtmosphere'}
 if($Path -in @('Plugins/OfflineDynastySettings/OfflineDynastySettings.Persistence.exe','Plugins/OfflineDynastySettings/ExtraSettings.Persistence.exe')){return 'ExtraDynastySettings'}
 if($Path -match '^Plugins/SituationalPractice/(SituationalPractice|ExpandedPractice)[.]Runtime[.](exe|pdb)$'){return 'DrivePractice'}
 return $null
}
function Standalone-Module([string]$Path){
 if($Path -eq 'Plugins/DynastyEnhancer.dll' -or $Path -match '^Plugins/(DynastyEnhancer/Runtime-[^/]+/)?(DynastyEnhancerNative[^/]*|DynastyEnhancerRecruitFog[^/]*)[.]dll$'){return 'DynastyEnhancer'}
 if($Path -in @('Plugins/DynamicGamedayAtmosphere.dll','Plugins/LoadDynamicGamedayAtmosphere.exe','Plugins/DgaDiagnostics.exe') -or $Path -match '^Plugins/(DynamicGamedayAtmosphere/Runtime-[^/]+/)?(DynamicGamedayAtmosphereNative[^/]*|DynamicAttendanceNative[^/]*)[.]dll$' -or $Path -match '^Plugins/DynamicGamedayAtmosphere/(Runtime-[^/]+/LoadDynamicGamedayAtmosphere[.]exe|Tools/DgaDiagnostics[.]exe)$'){return 'GamedayAtmosphere'}
 if($Path -eq 'Plugins/OfflineDynastySettings.dll' -or $Path -eq 'Plugins/OfflineDynastySettings/OfflineDynastySettings.Persistence.exe'){return 'ExtraDynastySettings'}
 if($Path -eq 'Plugins/SituationalPractice.dll' -or $Path -match '^Plugins/SituationalPractice/SituationalPractice[.]Runtime[.](exe|pdb)$'){return 'DrivePractice'}
 if($Path -match '^Plugins/(PlayerAnalytics[.]dll|(PlayerAnalytics/Runtime-v0[.]1/)?PlayerAnalyticsNative-v0[.]1[.]dll)$'){return 'DynastyEnhancer'}
 return $null
}
$MmcRoot=[IO.Path]::GetFullPath($MmcRoot).TrimEnd('\','/')
if(!(Test-Path -LiteralPath (Join-Path $MmcRoot 'MMCModManager.exe') -PathType Leaf)){throw 'Select the folder containing MMCModManager.exe'}
if($Fixture){
 if(!$FixtureRoot){throw 'Fixture mode requires its isolated parent'}
 $FixtureRoot=[IO.Path]::GetFullPath($FixtureRoot).TrimEnd('\','/')
 if(!$MmcRoot.StartsWith($FixtureRoot+'\',[StringComparison]::OrdinalIgnoreCase) -or !(Test-Path -LiteralPath (Join-Path $MmcRoot '.cot-fixture'))){throw 'Not an isolated COT fixture'}
}else{
 if($FaultAtStage -ne 'None'){throw 'Fault injection is fixture-only'}
 if(Get-Process MMCModManager,CollegeFB27 -ErrorAction SilentlyContinue){throw 'Close MMC and the game before installing or rolling back'}
}
$utf8=New-Object Text.UTF8Encoding($false)
$backupBase=Resolve-Owned $MmcRoot 'Plugins/DynastyCOT/Backups'
function Write-AtomicJournal([string]$Path,$Value){
 $temporary=$Path+'.tmp'
 $text=$Value|ConvertTo-Json -Depth 7
 try{
  $stream=New-Object IO.FileStream($temporary,[IO.FileMode]::Create,[IO.FileAccess]::Write,[IO.FileShare]::None)
  try{$bytes=$utf8.GetBytes($text);$stream.Write($bytes,0,$bytes.Length);$stream.Flush($true)}finally{$stream.Dispose()}
  if(Test-Path -LiteralPath $Path){[IO.File]::Replace($temporary,$Path,($Path+'.previous'))}else{[IO.File]::Move($temporary,$Path)}
 }finally{if(Test-Path -LiteralPath $temporary){Remove-Item -LiteralPath $temporary -Force}}
}
function Restore-Transaction([string]$Directory,[switch]$Automatic){
 $Directory=[IO.Path]::GetFullPath($Directory)
 if(!$Directory.StartsWith($backupBase+'\',[StringComparison]::OrdinalIgnoreCase)){throw 'Rollback is outside COT backups'}
 Assert-NoLink $MmcRoot $Directory
 $journalPath=Join-Path $Directory 'transaction.json'
 $journal=Get-Content -LiteralPath $journalPath -Raw | ConvertFrom-Json
 if($journal.format -ne 1 -or $journal.root -ne $MmcRoot -or $journal.status -eq 'rolled_back'){throw 'Invalid or already restored COT transaction'}
 $records=@($journal.operations)
 # Check the complete restore before changing anything.
 foreach($op in $records){
  $target=Resolve-Owned $MmcRoot $op.path;Assert-NoLink $MmcRoot $target
  if($op.existed){
   $saved=Resolve-Owned $Directory $op.backup
   if(!(Test-Path -LiteralPath $saved -PathType Leaf) -or (Hash $saved) -ne $op.before){throw 'Rollback backup differs'}
  }
 if(!$Automatic -and !$op.userData -and (Test-Path -LiteralPath $target) -and (!$op.installed -or (Hash $target) -ne $op.installed)){throw ('Managed file changed after installation: '+$op.path)}
 }
 [Array]::Reverse($records)
 foreach($op in $records){
  $target=Resolve-Owned $MmcRoot $op.path
  if(!$Automatic -and $op.userData -and (Test-Path -LiteralPath $target) -and (Hash $target) -ne $op.installed){continue}
  if($op.existed){
   New-Item -ItemType Directory -Path (Split-Path -Parent $target) -Force | Out-Null
   Copy-Item -LiteralPath (Resolve-Owned $Directory $op.backup) -Destination $target -Force
   if((Hash $target) -ne $op.before){throw 'Rollback restore failed verification'}
  }elseif(Test-Path -LiteralPath $target -PathType Leaf){Remove-Item -LiteralPath $target -Force}
 }
 $journal.status='rolled_back'
 Write-AtomicJournal $journalPath $journal
}
$hasher=[Security.Cryptography.SHA256]::Create()
try{$key=[BitConverter]::ToString($hasher.ComputeHash($utf8.GetBytes($MmcRoot.ToUpperInvariant()))).Replace('-','')}finally{$hasher.Dispose()}
$mutex=New-Object Threading.Mutex($false,('Local\DynastyCOT.Install.'+$key))
try{$owned=$mutex.WaitOne(0)}catch [Threading.AbandonedMutexException]{$owned=$true}
if(!$owned){$mutex.Dispose();throw 'Another COT installer/rollback owns this MMC folder'}
try{
if($Rollback){
 Restore-Transaction $Rollback
 Write-Output 'COT rollback verified. Existing user data and later user edits were preserved.'
 return
}
$PackageRoot=[IO.Path]::GetFullPath($PackageRoot)
$manifest=Get-Content -LiteralPath (Join-Path $PackageRoot 'package-manifest.json') -Raw | ConvertFrom-Json
if($manifest.format -ne 1 -or $manifest.product -ne 'DynastyCOT'){throw 'Unsupported COT package'}
$seen=@{};$payload=@();$ownedFiles=@()
foreach($file in $manifest.files){
 if($seen.ContainsKey($file.path.ToLowerInvariant())){throw 'Duplicate package path'}
 $seen[$file.path.ToLowerInvariant()]=$true
 $source=Resolve-Owned $PackageRoot $file.path;Assert-NoLink $PackageRoot $source
 if(!(Test-Path -LiteralPath $source -PathType Leaf) -or (Hash $source) -ne $file.sha256){throw ('Package integrity failed: '+$file.path)}
 if($file.path -match '^(COT-Runtime/|LICENSES/)' -or $file.path -in @('Plugins/DynastyCOT.dll','Plugins/OfflineDynastySettings/OfflineDynastySettings.Persistence.exe','Plugins/OfflineDynastySettings/ExtraSettings.Persistence.exe','Plugins/SituationalPractice/SituationalPractice.Runtime.exe','Plugins/SituationalPractice/SituationalPractice.Runtime.pdb','Plugins/SituationalPractice/ExpandedPractice.Runtime.exe','Plugins/SituationalPractice/ExpandedPractice.Runtime.pdb')){
  $target=Resolve-Owned $MmcRoot $file.path;Assert-NoLink $MmcRoot $target
  $payload+=@{source=$source;path=$file.path;hash=$file.sha256;userData=$false}
  $owner=Owned-Module $file.path
  if($owner){$ownedFiles+=[pscustomobject][ordered]@{path=$file.path;sha256=$file.sha256;module=$owner}}
 }
}
foreach($file in Get-ChildItem -LiteralPath $PackageRoot -Recurse -File){
 $relative=$file.FullName.Substring($PackageRoot.Length+1).Replace('\','/').ToLowerInvariant()
 if($relative -ne 'package-manifest.json' -and !$seen.ContainsKey($relative)){throw ('Unmanifested package file: '+$relative)}
}
if(!($payload.path -contains 'Plugins/DynastyCOT.dll')){throw 'COT host missing from package'}
$template=Resolve-Owned $PackageRoot 'Templates/settings.json'
$config=Resolve-Owned $MmcRoot 'Plugins/DynastyCOT/settings.json'
$ownership=Get-Content -LiteralPath (Join-Path $PackageRoot 'Templates/owned-payloads.json') -Raw|ConvertFrom-Json
if($ownership.format -ne 1){throw 'Unsupported COT ownership metadata'}
foreach($file in $ownership.current){
 if((Owned-Module $file.path) -ne $file.module -or $file.sha256 -notmatch '^[0-9a-f]{64}$'){throw 'Invalid COT ownership metadata'}
 $ownedFiles+=[pscustomobject][ordered]@{path=$file.path;sha256=$file.sha256;module=$file.module}
}
$receiptPath='Plugins/DynastyCOT/installed-files.json'
$installedReceipt=$null
$receipt=Resolve-Owned $MmcRoot $receiptPath
if(Test-Path -LiteralPath $receipt -PathType Leaf){
 Assert-NoLink $MmcRoot $receipt
 $installedReceipt=Get-Content -LiteralPath $receipt -Raw|ConvertFrom-Json
 if($installedReceipt.format -ne 1){throw 'Unsupported installed COT receipt'}
 foreach($file in $installedReceipt.files){
  if(!(Owned-Module $file.path) -or (Owned-Module $file.path) -ne $file.module -or $file.sha256 -notmatch '^[0-9a-f]{64}$'){throw 'Unsafe installed COT receipt'}
 }
}
$currentHost=Resolve-Owned $MmcRoot 'Plugins/DynastyCOT.dll'
$knownHosts=@($payload|Where-Object {$_.path -eq 'Plugins/DynastyCOT.dll'}|ForEach-Object {$_.hash})+@($ownership.legacy|Where-Object {$_.path -eq 'Plugins/DynastyCOT.dll'}|ForEach-Object {$_.sha256})
$knownHosts+=@($installedReceipt.files|Where-Object {$_.path -eq 'Plugins/DynastyCOT.dll'}|ForEach-Object {$_.sha256})
$updatingCot=(Test-Path -LiteralPath $currentHost -PathType Leaf) -and $knownHosts -contains (Hash $currentHost)
$ids=@('DynastyEnhancer','GamedayAtmosphere','ExtraDynastySettings','DrivePractice')
if($ModuleChoicesJson){
 $choices=$ModuleChoicesJson | ConvertFrom-Json
 if($choices -isnot [pscustomobject] -or @($choices.PSObject.Properties).Count -ne 4){throw 'Choose all four COT module switches'}
 foreach($property in $choices.PSObject.Properties){
  if($ids -cnotcontains $property.Name -or $property.Value -isnot [bool]){throw 'Invalid COT module switch'}
 }
 $content=@{format=1;modules=$choices} | ConvertTo-Json -Depth 4
 $sha=[Security.Cryptography.SHA256]::Create()
 try{$contentHash=[BitConverter]::ToString($sha.ComputeHash($utf8.GetBytes($content))).Replace('-','').ToLowerInvariant()}finally{$sha.Dispose()}
 if($Action -eq 'Install'){$payload+=@{content=$content;path='Plugins/DynastyCOT/settings.json';hash=$contentHash;userData=$true}}
}elseif(!(Test-Path -LiteralPath $config)){$payload+=@{source=$template;path='Plugins/DynastyCOT/settings.json';hash=(Hash $template);userData=$true}}
if($Action -eq 'UninstallSelected' -and (!$ModuleChoicesJson -or !(@($choices.PSObject.Properties | Where-Object {$_.Value}).Count))){throw 'Select at least one module to uninstall'}
# Import history only. Unreleased Enhancer SETTINGS intentionally start fresh.
foreach($name in $(if($Action -eq 'Install' -and !$updatingCot){@('RuntimeHistory','RecruitResumeHistory')}else{@()})){
 $old=Resolve-Owned $MmcRoot ('Plugins/DynastyEnhancer/Runtime-v103/'+$name)
 if(Test-Path -LiteralPath $old -PathType Container){
  foreach($file in Get-ChildItem -LiteralPath $old -Recurse -File){
   Assert-NoLink $MmcRoot $file.FullName
   $relative=$file.FullName.Substring($old.Length+1).Replace('\','/')
   $path='COT-Runtime/DynastyEnhancer/'+$name+'/'+$relative
   $target=Resolve-Owned $MmcRoot $path;Assert-NoLink $MmcRoot $target
   $hash=Hash $file.FullName
   if(Test-Path -LiteralPath $target){
    if((Hash $target) -ne $hash){throw ('Conflicting Enhancer history preserved; resolve before install: '+$path)}
   }else{$payload+=@{source=$file.FullName;path=$path;hash=$hash;userData=$true}}
  }
 }
}
$retire=@();$retainedLegacy=@();$standaloneRetire=@();$logRetire=@()
if($Action -eq 'Install'){
 $analyticsEnabled=$true
 $effectiveChoices=if($ModuleChoicesJson){$choices}elseif(Test-Path -LiteralPath $config){(Get-Content -LiteralPath $config -Raw | ConvertFrom-Json).modules}else{$null}
 if($effectiveChoices -and $effectiveChoices.PSObject.Properties['DynastyEnhancer']){$analyticsEnabled=[bool]$effectiveChoices.DynastyEnhancer}
 $standalone=@($ownership.standalone|Where-Object {$null -ne $_})
 foreach($file in $standalone){if((Standalone-Module $file.path) -ne $file.module -or $file.sha256 -notmatch '^[0-9a-f]{64}$'){throw 'Unsafe standalone ownership metadata'}}
 $known=@($ownership.legacy)+@($installedReceipt.files)+$standalone
 $currentPaths=@{};foreach($file in $ownedFiles){$currentPaths[$file.path]=$true}
 # Only the explicit 1.0 binary-name migration is retired. Data paths never move.
 $renames=@{
  'COT-Runtime/DynastyEnhancer/DynastyEnhancerNative-v103.dll'='COT-Runtime/DynastyEnhancer/BeyondTheFieldNative-v103.dll'
  'COT-Runtime/DynastyEnhancer/DynastyEnhancerRecruitFog-v1.dll'='COT-Runtime/DynastyEnhancer/BeyondTheFieldRecruitFog-v1.dll'
  'COT-Runtime/DynastyEnhancer/LoadDynastyEnhancer.exe'=''
  'DGA-Runtime/Runtime-2.1.9/DynamicGamedayAtmosphereNative-2.1.9.dll'='DGA-Runtime/Runtime-2.1.9/GamedayAtmosphereNative-2.1.9.dll'
  'Plugins/OfflineDynastySettings/OfflineDynastySettings.Persistence.exe'='Plugins/OfflineDynastySettings/ExtraSettings.Persistence.exe'
  'Plugins/SituationalPractice/SituationalPractice.Runtime.exe'='Plugins/SituationalPractice/ExpandedPractice.Runtime.exe'
  'Plugins/SituationalPractice/SituationalPractice.Runtime.pdb'='Plugins/SituationalPractice/ExpandedPractice.Runtime.pdb'
 }
 foreach($path in $renames.Keys){
  if($currentPaths.ContainsKey($path)){continue}
  $replacement=$renames[$path];if($replacement -and !$currentPaths.ContainsKey($replacement)){continue}
  $target=Resolve-Owned $MmcRoot $path;Assert-NoLink $MmcRoot $target
  if(!(Test-Path -LiteralPath $target -PathType Leaf)){continue}
  $observed=Hash $target;$owner=Owned-Module $path
  $proven=@($known|Where-Object {$_.path -eq $path -and $_.module -eq $owner -and $_.sha256 -eq $observed})
  if($proven.Count){$retire+=$path}else{$retainedLegacy+=$path}
 }
 # Standalone trust is separate from the COT uninstall receipt namespace.
 $plugins=Join-Path $MmcRoot 'Plugins'
 if(Test-Path -LiteralPath $plugins){foreach($file in Get-ChildItem -LiteralPath $plugins -Recurse -File | Where-Object {$_.Extension -in @('.dll','.exe')}){
  $path=$file.FullName.Substring($MmcRoot.Length+1).Replace('\','/');$owner=Standalone-Module $path
  if(!$owner -or (!$analyticsEnabled -and $path -match '^Plugins/(PlayerAnalytics[.]dll|(.*/)?PlayerAnalyticsNative-v0[.]1[.]dll)$')){continue}
  Assert-NoLink $MmcRoot $file.FullName;$observed=Hash $file.FullName
  $proven=@($standalone|Where-Object {$_.path -eq $path -and $_.module -eq $owner -and $_.sha256 -eq $observed})
  # Identical native bytes may have shipped in COT's detached runtime folder.
  if(!$proven.Count -and $path -ne 'Plugins/PlayerAnalytics.dll' -and $path -match 'Native[^/]*[.]dll$'){
   $proven=@((@($ownedFiles)+$known)|Where-Object {$_.module -eq $owner -and $_.path -match '[.]dll$' -and $_.sha256 -eq $observed})
  }
  if(!$proven.Count){
   if($file.Extension -eq '.exe'){$retainedLegacy+=$path;continue}
   throw ('Unrecognized active legacy payload kept; no installation changes made: '+$path+'. Close MMC/game and remove or verify that standalone mod before retrying.')
  }
  $standaloneRetire+=$path
 }}
 # Exact legacy log names are user data, never swept by an extension wildcard.
 foreach($suffix in @('','.1')){
  $path='Plugins/DynastyEnhancer-launcher.log'+$suffix;$source=Resolve-Owned $MmcRoot $path;Assert-NoLink $MmcRoot $source
  if(!(Test-Path -LiteralPath $source -PathType Leaf)){continue}
  $hash=Hash $source;$destination='Plugins/DynastyEnhancer/Logs/legacy-launcher.log'+$suffix
  $target=Resolve-Owned $MmcRoot $destination;Assert-NoLink $MmcRoot $target
  if((Test-Path -LiteralPath $target -PathType Leaf) -and (Hash $target) -ne $hash){
   $destination='Plugins/DynastyEnhancer/Logs/legacy-launcher-'+$hash+'.log'+$suffix
   $target=Resolve-Owned $MmcRoot $destination;Assert-NoLink $MmcRoot $target
   if((Test-Path -LiteralPath $target -PathType Leaf) -and (Hash $target) -ne $hash){throw ('Archived legacy launcher log changed; no installation changes made: '+$destination)}
  }
  $payload+=@{source=$source;path=$destination;hash=$hash;userData=$true};$logRetire+=$path
 }
}
$directory=Join-Path $backupBase ([DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfff')+'-'+[Guid]::NewGuid().ToString('N'))
Assert-NoLink $MmcRoot $directory
New-Item -ItemType Directory -Path $directory -Force | Out-Null
$operations=New-Object 'Collections.Generic.List[object]'
$journal=@{format=1;root=$MmcRoot;status='installing';action=$Action;operations=@()}
function Save-Journal {
 $journal.operations=$operations.ToArray()
 Write-AtomicJournal (Join-Path $directory 'transaction.json') $journal
}
function Record-Target([string]$Relative,[bool]$UserData){
 $target=Resolve-Owned $MmcRoot $Relative;Assert-NoLink $MmcRoot $target
 $exists=Test-Path -LiteralPath $target -PathType Leaf
 $backup=([Guid]::NewGuid().ToString('N')+'.bak');$before=$null
 if($exists){$before=Hash $target;Copy-Item -LiteralPath $target -Destination (Join-Path $directory $backup)}
 $op=@{path=$Relative;userData=$UserData;existed=$exists;backup=$backup;before=$before;installed=$null}
 $operations.Add($op);Save-Journal
 return $op
}
function Write-Payload($File){
 $target=Resolve-Owned $MmcRoot $File.path
 if((Test-Path -LiteralPath $target -PathType Leaf) -and (Hash $target) -eq $File.hash){return}
 $op=Record-Target $File.path $File.userData
 New-Item -ItemType Directory -Path (Split-Path -Parent $target) -Force | Out-Null
 if($File.ContainsKey('content')){[IO.File]::WriteAllText($target,$File.content,$utf8)}
 else{Copy-Item -LiteralPath $File.source -Destination $target -Force}
 if((Hash $target) -ne $File.hash){throw ('Installed file differs: '+$File.path)}
 $op.installed=$File.hash;Save-Journal
}
function Fault([string]$Stage){if($FaultAtStage -eq $Stage){throw ('Injected fixture failure: '+$Stage)}}
function Json-Payload([string]$Path,$Value,[bool]$UserData){
 $text=$Value|ConvertTo-Json -Depth 8
 $sha=[Security.Cryptography.SHA256]::Create()
 try{$hash=[BitConverter]::ToString($sha.ComputeHash($utf8.GetBytes($text))).Replace('-','').ToLowerInvariant()}finally{$sha.Dispose()}
 return @{content=$text;path=$Path;hash=$hash;userData=$UserData}
}
try{
 if($Action -ne 'Install'){
  $remove=@{};foreach($id in $ids){$remove[$id]=$Action -eq 'UninstallAll' -or $choices.$id}
  $remove['Core']=$Action -eq 'UninstallAll'
  $known=@($ownedFiles)+@($ownership.legacy)
  if($installedReceipt){$known+=@($installedReceipt.files)}
  $byPath=@{}
  foreach($file in $known){
   $owner=Owned-Module $file.path
   if(!$owner -or $owner -ne $file.module -or $file.sha256 -notmatch '^[0-9a-f]{64}$'){throw 'Unsafe installed COT receipt'}
   if(!$byPath.ContainsKey($file.path)){$byPath[$file.path]=@()}
   $byPath[$file.path]+=$file
  }
  # Finish preflight before removing anything. Never remove a changed file.
  $deletions=@();$kept=@()
  foreach($path in $byPath.Keys){
   $target=Resolve-Owned $MmcRoot $path;Assert-NoLink $MmcRoot $target
   if(!(Test-Path -LiteralPath $target -PathType Leaf)){continue}
   $owner=Owned-Module $path;$observed=Hash $target
   if(!$remove[$owner]){continue}
   if(@($byPath[$path].sha256) -notcontains $observed){
    if($path -eq 'Plugins/DynastyCOT.dll'){throw 'COT host changed since installation; no files were removed. Reinstall this build before uninstalling.'}
    $kept+=$path;continue
   }
   $deletions+=$path
  }
  $settings=Get-Content -LiteralPath $template -Raw|ConvertFrom-Json
  if(Test-Path -LiteralPath $config){Assert-NoLink $MmcRoot $config;$settings=Get-Content -LiteralPath $config -Raw|ConvertFrom-Json}
  if($settings.format -ne 1 -or !$settings.modules){throw 'Existing COT settings are invalid'}
  foreach($id in $ids){if($remove[$id]){$settings.modules|Add-Member -NotePropertyName $id -NotePropertyValue $false -Force}}
  Write-Payload (Json-Payload 'Plugins/DynastyCOT/settings.json' $settings $true)
  # The host is removed first for a complete uninstall, after preflight succeeds.
  foreach($path in @($deletions|Sort-Object @{Expression={if($_ -eq 'Plugins/DynastyCOT.dll'){0}else{1}}}, {$_})){
   Record-Target $path $false|Out-Null
   Remove-Item -LiteralPath (Resolve-Owned $MmcRoot $path) -Force
  }
  Fault 'AfterPayload'
  $remaining=@($known|Where-Object {!( $deletions -contains $_.path )}|Sort-Object path,sha256 -Unique)
  Write-Payload (Json-Payload $receiptPath ([ordered]@{format=1;version=$manifest.version;files=$remaining}) $false)
  Fault 'AfterAdapter'
  $journal.status='committed';Save-Journal
  Write-Output ('COT removal verified. Backup: '+$directory)
  Write-Output 'Settings, histories and backups were kept. Standalone mods are not re-enabled; use Restore backup to undo removal.'
  foreach($path in $kept){Write-Output ('Kept changed file (module disabled): '+$path)}
  return
 }
 # Retain ownership of older versioned runtimes across later updates.
 # They may still be on disk after the module moves to a new runtime version.
 if($installedReceipt){$ownedFiles+=@($installedReceipt.files|Where-Object {$retire -notcontains $_.path})}
 $ownedFiles=@($ownedFiles|Sort-Object path,sha256 -Unique)
 $payload+=Json-Payload $receiptPath ([ordered]@{format=1;version=$manifest.version;files=$ownedFiles}) $false
 foreach($file in $payload | Where-Object {$_.path -ne 'Plugins/DynastyCOT.dll'}){Write-Payload $file}
 Fault 'AfterPayload'
 # Every deletion was preflighted by exact path and receipt/known hash.
 foreach($path in @((@($retire)+@($standaloneRetire))|Sort-Object -Unique)){
  Record-Target $path $false | Out-Null
  Remove-Item -LiteralPath (Resolve-Owned $MmcRoot $path) -Force
 }
 foreach($path in $logRetire){Record-Target $path $true | Out-Null;Remove-Item -LiteralPath (Resolve-Owned $MmcRoot $path) -Force}
 Fault 'AfterLegacyDisable'
 Write-Payload ($payload | Where-Object {$_.path -eq 'Plugins/DynastyCOT.dll'})
 Fault 'AfterAdapter'
 foreach($file in $payload){if((Hash (Resolve-Owned $MmcRoot $file.path)) -ne $file.hash){throw 'Final installation verification failed'}}
 $journal.status='committed';Save-Journal
 Write-Output ('COT installation verified. Backup: '+$directory)
 Write-Output 'Module switches: Plugins/DynastyCOT/settings.json. Restart MMC and the game after changes.'
 foreach($path in @($retainedLegacy|Sort-Object -Unique)){Write-Output ('Kept changed or unknown inactive legacy payload: '+$path)}
}catch{
 $failure=$_
 try{Restore-Transaction $directory -Automatic}catch{throw ('Install failed: '+$failure.Exception.Message+'; rollback needs review: '+$_.Exception.Message+'; backup: '+$directory)}
 throw ('Install failed and previous managed files were restored: '+$failure.Exception.Message)
}

}finally{$mutex.ReleaseMutex();$mutex.Dispose()}
