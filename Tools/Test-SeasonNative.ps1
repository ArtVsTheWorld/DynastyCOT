param([switch]$ChecksOnly)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$out=Join-Path $root 'Build/RC016/Native'
New-Item -ItemType Directory -Force $out | Out-Null
$vs=& 'C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe' -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$vc=Get-ChildItem -LiteralPath (Join-Path $vs 'VC/Tools/MSVC') -Directory | Sort-Object Name -Descending | Select-Object -First 1
$kits='C:/Program Files (x86)/Windows Kits/10'
$sdk=Get-ChildItem -LiteralPath (Join-Path $kits 'Include') -Directory | Sort-Object Name -Descending | Select-Object -First 1
$flags=@('/nologo','/O2','/MT','/W4','/WX','/EHsc','/std:c++17','/utf-8',('/I'+(Join-Path $vc.FullName 'include')))
foreach($part in @('ucrt','shared','um','winrt')){$flags+=('/I'+(Join-Path $sdk.FullName $part))}
$libs=@(('/LIBPATH:'+(Join-Path $vc.FullName 'lib/x64')))
foreach($part in @('ucrt','um')){$libs+=('/LIBPATH:'+(Join-Path $kits ('Lib/'+$sdk.Name+'/'+$part+'/x64')))}
Push-Location $out
try{
 if($ChecksOnly){
  $source=Join-Path $root 'Modules/DynastyEnhancer/Source'
  foreach($name in @('AnalyticsModelTests','draft_results_frame_tests','AnalyticsIntegrationTests','RecruitResumeTools')){
   $file=if($name -eq 'RecruitResumeTools'){Join-Path $source ($name+'.cpp')}else{Join-Path $root ('Modules/DynastyEnhancer/Tests/'+$name+'.cpp')}
   & (Join-Path $vc.FullName 'bin/Hostx64/x64/cl.exe') @flags ('/I'+$source) $file ('/Fe:'+ $name+'.exe') /link @libs /INCREMENTAL:NO
   if($LASTEXITCODE -ne 0){throw ($name+' compile failed')}
   if($name -eq 'RecruitResumeTools'){& (Join-Path $out ($name+'.exe')) --self-test}else{& (Join-Path $out ($name+'.exe'))}
   if($LASTEXITCODE -ne 0){throw ($name+' failed')}
  }
  return
 }
 foreach($mode in @('before','after')){
  $source=if($mode -eq 'before'){Join-Path $root 'Build/RC016/Baseline/Modules/DynastyEnhancer/Source'}else{Join-Path $root 'Modules/DynastyEnhancer/Source'}
  & (Join-Path $vc.FullName 'bin/Hostx64/x64/cl.exe') @flags ('/I'+$source) (Join-Path $root 'Modules/DynastyEnhancer/Tests/SeasonCaptureReplay.cpp') ('/Fe:'+ $mode+'.exe') /link @libs /INCREMENTAL:NO
  if($LASTEXITCODE -ne 0){throw 'Season replay compile failed'}
 }
 # Alternate A/B order; parsing and diagnostics are outside each timed region.
 foreach($fixture in @('2026-PreSeason-0','2026-RegularSeason-12','2026-NationalChampionship-20')){
  foreach($mode in @('before','after')){
   & (Join-Path $out ($mode+'.exe')) (Join-Path $root ('Build/RC016/Inputs/'+$fixture+'.json')) (Join-Path $out ($fixture+'-'+$mode+'.json'))
   if($LASTEXITCODE -ne 0){throw 'Season replay failed'}
  }
 }
}finally{Pop-Location}
