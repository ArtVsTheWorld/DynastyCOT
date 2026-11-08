param(
 [Parameter(Mandatory=$true)][string]$MmcRoot,
 [ValidateSet('All','DynastyEnhancer','GamedayAtmosphere','ExtraDynastySettings','DrivePractice')][string]$Module='All',
 [switch]$Package
)
$ErrorActionPreference='Stop'
$root=$PSScriptRoot
$MmcRoot=[IO.Path]::GetFullPath($MmcRoot)
foreach($name in @('FrostyCore.dll','FrostySdk.dll')){
 if(!(Test-Path -LiteralPath (Join-Path $MmcRoot $name) -PathType Leaf)){throw ('Missing MMC SDK assembly: '+$name)}
}
if($Package -and $Module -ne 'All'){throw 'A complete package requires all four module builds'}
& (Join-Path $root 'Test.ps1') -Suite Baseline
$buildId=[DateTime]::UtcNow.ToString('yyyyMMddHHmmssfff')
$selected=if($Module -eq 'All'){@('DrivePractice','DynastyEnhancer','GamedayAtmosphere','ExtraDynastySettings')}else{@($Module)}
$env:PYTHONDONTWRITEBYTECODE='1'
foreach($name in $selected){
 Write-Output ('Building COT source module: '+$name)
 Push-Location (Join-Path $root ('Modules/'+$name))
 try {
  switch($name){
   'DynastyEnhancer' {
    & (Join-Path $root 'Tools/Refresh-PracticeContract.ps1')
    if($LASTEXITCODE -ne 0){throw 'Practice compatibility contract failed'}
    & './Source/build.ps1' -NativeName ('DynastyEnhancerNative-cot-'+$buildId)
    if($LASTEXITCODE -ne 0){throw 'Enhancer native build failed'}
    & './Source/build-recruiting-fog.ps1' -Name ('DynastyEnhancerRecruitFog-p'+$buildId) -RecruitNotesUi -PlayerFogPerf
    if($LASTEXITCODE -ne 0){throw 'Enhancer Fog build failed'}
    $hashes=@{}
    foreach($file in Get-ChildItem -LiteralPath 'Source' -File | Where-Object {$_.Extension -in @('.cpp','.h','.ps1')}){
     $hashes['Modules/DynastyEnhancer/Source/'+$file.Name]=(Get-FileHash -LiteralPath $file.FullName).Hash.ToLowerInvariant()
    }
    $hashes['Compatibility/PracticeTextSetter.generated.h']=(Get-FileHash -LiteralPath (Join-Path $root 'Compatibility/PracticeTextSetter.generated.h')).Hash.ToLowerInvariant()
    $inputs=@{socialNative=('Modules/DynastyEnhancer/Build/DynastyEnhancerNative-cot-'+$buildId+'.dll');fogNative=('Modules/DynastyEnhancer/Build/DynastyEnhancerRecruitFog-p'+$buildId+'.dll');sourceHashes=$hashes}
    [IO.File]::WriteAllText((Join-Path $root 'cot.build-inputs.json'),($inputs|ConvertTo-Json -Depth 5),[Text.UTF8Encoding]::new($false))
   }
   'GamedayAtmosphere' { & './Source/Build.ps1' -Automatic }
   'ExtraDynastySettings' { & './Build.ps1' -MmcRoot $MmcRoot }
   'DrivePractice' { & './Build.ps1' -MmcRoot $MmcRoot }
  }
  if($LASTEXITCODE -ne 0){throw ($name+' build failed')}
 }finally{Pop-Location}
}
if($Package){
 & (Join-Path $root 'Test-Clocks.ps1') -MmcRoot $MmcRoot
 if($LASTEXITCODE -ne 0){throw 'Clock checks failed'}
 Push-Location (Join-Path $root 'Modules/ExtraDynastySettings')
 try{
  & pwsh -NoProfile -File './Test.ps1' -MmcRoot $MmcRoot *> (Join-Path $root 'Build/ClockChecks/sdk-regressions.log')
  if($LASTEXITCODE -ne 0){throw 'Settings SDK regressions failed; see Build/ClockChecks/sdk-regressions.log'}
 }finally{Pop-Location}
 & (Join-Path $root 'Test.ps1') -Suite All
 if($LASTEXITCODE -ne 0){throw 'Module checks failed'}
 & python (Join-Path $root 'Tools/export_compatibility.py')
 if($LASTEXITCODE -ne 0){throw 'Compatibility inventory failed'}
 & python (Join-Path $root 'Tools/build_package.py') --mmc-sdk $MmcRoot
 if($LASTEXITCODE -ne 0){throw 'COT host packaging failed'}
 & python (Join-Path $root 'Tools/verify_package.py') $MmcRoot
 if($LASTEXITCODE -ne 0){throw 'COT package checks failed'}
 & python (Join-Path $root 'Tests/test_installer.py')
 if($LASTEXITCODE -ne 0){throw 'COT installer fixture checks failed'}
 & python (Join-Path $root 'Tools/finalize_package.py') $MmcRoot
 if($LASTEXITCODE -ne 0){throw 'COT archive verification failed'}
 & python (Join-Path $root 'Tools/build_installer.py')
 if($LASTEXITCODE -ne 0){throw 'COT Windows setup build failed'}
 & python (Join-Path $root 'Tests/test_setup.py')
 if($LASTEXITCODE -ne 0){throw 'COT Windows setup checks failed'}
 & python (Join-Path $root 'Tests/test_updater.py')
 if($LASTEXITCODE -ne 0){throw 'COT manual updater and rollback checks failed'}
 & python (Join-Path $root 'Tests/test_release_metadata.py')
 if($LASTEXITCODE -ne 0){throw 'COT release metadata and public evidence checks failed'}
}
Write-Output 'PASS: requested COT development build completed. No live installation or publishing performed.'
