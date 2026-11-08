param()
$ErrorActionPreference='Stop'
$moduleRoot=Split-Path -Parent $PSScriptRoot
$repoRoot=Split-Path -Parent (Split-Path -Parent $moduleRoot)
$out=Join-Path $repoRoot 'Build/AnalyticsCards/NativeTests'
New-Item -ItemType Directory -Force -Path $out | Out-Null
$vs=& 'C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe' -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if(!$vs){throw 'MSVC x64 tools required'}
$vc=Get-ChildItem -LiteralPath (Join-Path $vs 'VC/Tools/MSVC') -Directory | Sort-Object Name -Descending | Select-Object -First 1
$kits='C:/Program Files (x86)/Windows Kits/10'
$sdk=Get-ChildItem -LiteralPath (Join-Path $kits 'Include') -Directory | Sort-Object Name -Descending | Select-Object -First 1
$compiler=Join-Path $vc.FullName 'bin/Hostx64/x64/cl.exe'
$flags=@('/nologo','/O2','/MT','/W4','/WX','/EHsc','/std:c++17','/utf-8','/DRECRUIT_NOTES_UI','/DPLAYER_FOG_PERF',('/I'+(Join-Path $vc.FullName 'include')))
foreach($part in @('ucrt','shared','um','winrt')){$flags+=('/I'+(Join-Path $sdk.FullName $part))}
$libs=@(('/LIBPATH:'+(Join-Path $vc.FullName 'lib/x64')))
foreach($part in @('ucrt','um')){$libs+=('/LIBPATH:'+(Join-Path $kits ('Lib/'+$sdk.Name+'/'+$part+'/x64')))}
Push-Location $out
try{
 foreach($name in @('AnalyticsIntegrationTests','RecruitingFogTests')){
  & $compiler @flags (Join-Path $PSScriptRoot ($name+'.cpp')) ('/Fe:'+$name+'.exe') /link @libs /INCREMENTAL:NO
  if($LASTEXITCODE -ne 0){throw ($name+' build failed')}
  & (Join-Path $out ($name+'.exe'))
  if($LASTEXITCODE -ne 0){throw ($name+' failed')}
 }
}finally{Pop-Location}
Write-Output 'PASS: Analytics card/owner checks; no hooks installed and no live game access.'
