param()
$ErrorActionPreference='Stop'
$testOut=Join-Path (Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $PSScriptRoot))) 'Build/RecruitInterestTests'
New-Item -ItemType Directory -Force -Path $testOut | Out-Null
$listProbeVs=& 'C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe' -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if(-not $listProbeVs){throw 'MSVC x64 tools are required'}
$listProbeTool=Get-ChildItem -LiteralPath (Join-Path $listProbeVs 'VC/Tools/MSVC') -Directory | Sort-Object Name -Descending | Select-Object -First 1
$listProbeKits='C:/Program Files (x86)/Windows Kits/10'
$listProbeSdk=Get-ChildItem -LiteralPath (Join-Path $listProbeKits 'Include') -Directory | Sort-Object Name -Descending | Select-Object -First 1
$listProbeFlags=@('/nologo','/O2','/MT','/W4','/WX','/EHsc','/std:c++17','/utf-8',('/I'+(Join-Path $listProbeTool.FullName 'include')),('/I'+(Join-Path $listProbeSdk.FullName 'ucrt')),('/I'+(Join-Path $listProbeSdk.FullName 'shared')),('/I'+(Join-Path $listProbeSdk.FullName 'um')))
$listProbeLib=@(('/LIBPATH:'+(Join-Path $listProbeTool.FullName 'lib/x64')),('/LIBPATH:'+(Join-Path $listProbeKits ('Lib/'+$listProbeSdk.Name+'/ucrt/x64'))),('/LIBPATH:'+(Join-Path $listProbeKits ('Lib/'+$listProbeSdk.Name+'/um/x64'))))

& (Join-Path $listProbeTool.FullName 'bin/Hostx64/x64/cl.exe') @listProbeFlags (Join-Path $PSScriptRoot 'RecruitInterestTests.cpp') ('/Fo:'+(Join-Path $testOut 'RecruitInterestTests.obj')) ('/Fe:'+(Join-Path $testOut 'RecruitInterestTests.exe')) /link @listProbeLib /INCREMENTAL:NO
if($LASTEXITCODE -ne 0){throw 'Recruit interest fixture compile failed'}
& (Join-Path $testOut 'RecruitInterestTests.exe')
if($LASTEXITCODE -ne 0){throw 'Recruit interest fixture failed'}






& (Join-Path $listProbeTool.FullName 'bin/Hostx64/x64/cl.exe') @listProbeFlags (Join-Path $PSScriptRoot 'RecruitInterestUiCompileTests.cpp') ('/Fo:'+(Join-Path $testOut 'RecruitInterestUiCompileTests.obj')) ('/Fe:'+(Join-Path $testOut 'RecruitInterestUiCompileTests.exe')) /link @listProbeLib /INCREMENTAL:NO
if($LASTEXITCODE -ne 0){throw 'Recruit UI native contract fixture compile failed'}
& (Join-Path $testOut 'RecruitInterestUiCompileTests.exe')
if($LASTEXITCODE -ne 0){throw 'Recruit UI native contract fixture failed'}
& (Join-Path $listProbeTool.FullName 'bin/Hostx64/x64/cl.exe') @listProbeFlags (Join-Path $PSScriptRoot 'RecruitingStartupTests.cpp') ('/Fo:'+(Join-Path $testOut 'RecruitingStartupTests.obj')) ('/Fe:'+(Join-Path $testOut 'RecruitingStartupTests.exe')) /link @listProbeLib /INCREMENTAL:NO
if($LASTEXITCODE -ne 0){throw 'Recruiting startup fixture compile failed'}
& (Join-Path $testOut 'RecruitingStartupTests.exe')
if($LASTEXITCODE -ne 0){throw 'Recruiting startup fixture failed'}
& python (Join-Path $PSScriptRoot 'recruit_interest_ui_test.py')
if($LASTEXITCODE -ne 0){throw 'Recruit interest UI checks failed'}
